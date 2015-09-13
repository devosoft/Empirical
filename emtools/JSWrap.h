#ifndef EMP_JSWRAP_H
#define EMP_JSWRAP_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Wrap a C++ function and convert it to an integer that can be called from Javascript
//
//  To wrap a function, call:
//     uint32_t fun_id = emp::JSWrap(FunctionToBeWrapped, "JS_Function_Name");
//
//  To manually callback a function from Javascript, first set emp_i.cb_args to an array of
//  function arguments, then call empCppCallback( fun_id );   This all happens automatically
//  if you use the emp.Callback(fun_id, args...) function from Javascript.
//
//  The JS_Function_Name string is optional, but if you use it, the appropriate function will
//  be automatically generated in Javascript by JSWrap, in the emp class.
//
//  For example, if you have:
//     int AddPair(int x, int y) { return x + y; }
//
//  You can wrap it with:
//     uint32_t fun_id = emp::JSWrap(AddPair, "AddPair");
//
//  And then in Javascript, you can simply call it as:
//     emp.AddPair(4, 5); // will return 9.
//
//
//  Development notes:
//  * Make sure JSWrap can take function objects, lambdas, or just function names.
//  * Add a JSWrap that takes an object and method and does the bind automatically.
//
//  Recent changes:
//  * Made JSWrap compatible with Javascript objects with multiple properties. 
//    In order do so, you must define the properties of the object as a tuple 
//    struct in js_object_struct.h. - @ELD
//

#include <functional>
#include <tuple>
#include <array>

#include "../tools/assert.h"
#include "../tools/functions.h"
#include "../tools/mem_track.h"
#include "../tools/vector.h"
#include "../tools/tuple_struct.h"
#include "js_object_struct.h"

#ifdef EMSCRIPTEN
extern "C" {
  extern int EMP_GetCBArgCount();  // Get the number of arguments associated with a callback.
}
#else
// When NOT in Emscripten, need a stub for this function.
int EMP_GetCBArgCount() { return -1; }
#endif


namespace emp {
  
  // ----- LoadArg -----
  // Helper functions to individually LOAD ARGUMENTS from JS based on expected type.
  template <int ARG_ID> static void LoadArg(int & arg_var) {
    arg_var = EM_ASM_INT({ return emp_i.cb_args[$0]; }, ARG_ID);
  }
  
  template <int ARG_ID> static void LoadArg(bool & arg_var) {
    arg_var = EM_ASM_INT({ return emp_i.cb_args[$0]; }, ARG_ID);
  }
  
  template <int ARG_ID> static void LoadArg(char & arg_var) {
    arg_var = EM_ASM_INT({ return emp_i.cb_args[$0]; }, ARG_ID);
  }
  
  template <int ARG_ID> static void LoadArg(double & arg_var) {
    arg_var = EM_ASM_DOUBLE({ return emp_i.cb_args[$0]; }, ARG_ID);
  }
  
  template <int ARG_ID> static void LoadArg(float & arg_var) {
    arg_var = EM_ASM_DOUBLE({ return emp_i.cb_args[$0]; }, ARG_ID);
  }
  
  template <int ARG_ID> static void LoadArg(std::string & arg_var) {
    char * tmp_var = (char *) EM_ASM_INT({
        return allocate(intArrayFromString(emp_i.cb_args[$0]), 'i8', ALLOC_STACK);
      }, ARG_ID);
    arg_var = tmp_var;   // @CAO Do we need to free the memory in tmp_var?
  }

  //Helper functions to load arguments from inside Javascript objects by name.
  template <int ARG_ID> static void LoadArg(int & arg_var, std::string var) {
    arg_var = EM_ASM_INT({ return emp_i.cb_args[$0][Pointer_stringify($1)]; 
      }, ARG_ID, var.c_str());
  }

  template <int ARG_ID> static void LoadArg(bool & arg_var, std::string var) {
    arg_var = EM_ASM_INT({ return emp_i.cb_args[$0][Pointer_stringify($1)]; 
      }, ARG_ID, var.c_str());
  }
  
  template <int ARG_ID> static void LoadArg(char & arg_var, std::string var) {
    arg_var = EM_ASM_INT({ return emp_i.cb_args[$0][Pointer_stringify($1)]; 
      }, ARG_ID, var.c_str());
  }
  
  template <int ARG_ID> static void LoadArg(double & arg_var, std::string var) {
    arg_var = EM_ASM_DOUBLE({ return emp_i.cb_args[$0][Pointer_stringify($1)]; 
      }, ARG_ID, var.c_str());
  }
  
  template <int ARG_ID> static void LoadArg(float & arg_var, std::string var) {
    arg_var = EM_ASM_DOUBLE({ return emp_i.cb_args[$0][Pointer_stringify($1)]; 
      }, ARG_ID, var.c_str());
  }
  
  template <int ARG_ID> static void LoadArg(std::string & arg_var, std::string var) {
    char * tmp_var = (char *) EM_ASM_INT({
        return allocate(intArrayFromString(
		emp_i.cb_args[$0][Pointer_stringify($1)]), 'i8', ALLOC_STACK);
      }, ARG_ID, var.c_str());
    arg_var = tmp_var;   // Free memory here?
  }

  //Helper macros to make sure that every piece of a tuple_struct gets loaded
  #define LOAD_TUPLE_ARG_1 LoadArg<ARG_ID>(std::get<0>(arg_var.emp__tuple_body)\
                        , arg_var.var_names[0]);
  #define LOAD_TUPLE_ARG_2 LOAD_TUPLE_ARG_1 LoadArg<ARG_ID>(std::get<1>(\
			arg_var.emp__tuple_body),arg_var.var_names[1]);
  #define LOAD_TUPLE_ARG_3 LOAD_TUPLE_ARG_2 LoadArg<ARG_ID>(std::get<2>(\
			arg_var.emp__tuple_body),arg_var.var_names[2]);
  #define LOAD_TUPLE_ARG_4 LOAD_TUPLE_ARG_3 LoadArg<ARG_ID>(std::get<3>(\
			arg_var.emp__tuple_body),arg_var.var_names[3]);
  #define LOAD_TUPLE_ARG_5 LOAD_TUPLE_ARG_4 LoadArg<ARG_ID>(std::get<4>(\
			arg_var.emp__tuple_body),arg_var.var_names[4]);
  #define LOAD_TUPLE_ARG_6 LOAD_TUPLE_ARG_5 LoadArg<ARG_ID>(std::get<5>(\
			arg_var.emp__tuple_body),arg_var.var_names[5]);
  #define LOAD_TUPLE_ARG_7 LOAD_TUPLE_ARG_6 LoadArg<ARG_ID>(std::get<6>(\
			arg_var.emp__tuple_body),arg_var.var_names[6]);
  #define LOAD_TUPLE_ARG_8 LOAD_TUPLE_ARG_7 LoadArg<ARG_ID>(std::get<7>(\
			arg_var.emp__tuple_body),arg_var.var_names[7]);
  #define LOAD_TUPLE_ARG_9 LOAD_TUPLE_ARG_8 LoadArg<ARG_ID>(std::get<8>(\
			arg_var.emp__tuple_body),arg_var.var_names[8]);
  #define LOAD_TUPLE_ARG_10 LOAD_TUPLE_ARG_9 LoadArg<ARG_ID>(std::get<9>(\
			arg_var.emp__tuple_body),arg_var.var_names[9]);
  #define LOAD_TUPLE_ARG_11 LOAD_TUPLE_ARG_10 LoadArg<ARG_ID>(std::get<10>(\
			arg_var.emp__tuple_body),arg_var.var_names[10]);
  #define LOAD_TUPLE_ARG_12 LOAD_TUPLE_ARG_11 LoadArg<ARG_ID>(std::get<11>(\
			arg_var.emp__tuple_body),arg_var.var_names[11]);
  #define LOAD_TUPLE_ARG_13 LOAD_TUPLE_ARG_12 LoadArg<ARG_ID>(std::get<12>(\
			arg_var.emp__tuple_body),arg_var.var_names[12]);
  #define LOAD_TUPLE_ARG_14 LOAD_TUPLE_ARG_13 LoadArg<ARG_ID>(std::get<13>(\
			arg_var.emp__tuple_body),arg_var.var_names[13]);
  #define LOAD_TUPLE_ARG_15 LOAD_TUPLE_ARG_14 LoadArg<ARG_ID>(std::get<14>(\
			arg_var.emp__tuple_body),arg_var.var_names[14]);
  #define LOAD_TUPLE_ARG_16 LOAD_TUPLE_ARG_15 LoadArg<ARG_ID>(std::get<15>(\
			arg_var.emp__tuple_body),arg_var.var_names[15]);

  //Macro to load the right set of args into the tuple
  //This requires that the user define DATA_OBJECT_SIZE with the struct,
  //which is kind of ugly. Is there a way around it with templating?
  #define LOAD_ARGS_FROM_TUPLE(s) EMP_MERGE(LOAD_TUPLE_ARG_,  s)
  
  //Load a Javascript-style object of the form described in js_object_struct.h
  //into a JSDataObject struct. 
  template <int ARG_ID> static void LoadArg(JSDataObject & arg_var) {
    LOAD_ARGS_FROM_TUPLE(DATA_OBJECT_SIZE)
  }


  // ----- StoreReturn -----
  // Helper functions to individually store return values to JS

  // static void StoreReturn(const bool & ret_var) {
  //   EM_ASM_ARGS({ emp_i.cb_return = $0; }, ret_var);
  // }

  static void StoreReturn(const int & ret_var) {
    EM_ASM_ARGS({ emp_i.cb_return = $0; }, ret_var);
  }

  static void StoreReturn(const double & ret_var) {
    EM_ASM_ARGS({ emp_i.cb_return = $0; }, ret_var);
  }

  static void StoreReturn(const std::string & ret_var) {
    EM_ASM_ARGS({ emp_i.cb_return = Pointer_stringify($0); }, ret_var.c_str());
  }

  // If the return type has a personalized function to handle the return, use it!
  template <typename RETURN_TYPE>
  static typename emp::sfinae_decoy<void, decltype(&RETURN_TYPE::StoreAsReturn)>::type
  StoreReturn(const RETURN_TYPE & ret_var) {
    ret_var.template StoreAsReturn();
  }


  
  // The following code is in the "internal" namespace since it's used only to implement the
  // details of the JSWrap function.

  namespace internal {

    template <typename T, int ARG_ID>
    void LoadArg_impl(typename emp::sfinae_decoy<bool, decltype(&T::template LoadFromArg<ARG_ID>)>::type,
                      T & target) {
      target.template LoadFromArg<ARG_ID>();
    }
    template <typename T, int ARG_ID>
    void LoadArg_impl(int, T & target) {
      LoadArg<ARG_ID>(target);
    }

    // JSWrap_Callback_Base provides a base class for the wrappers around functions.
    // Specifically, it creates a virtual DoCallback() member function that can be called
    // to trigger a specific wrapped function.

    class JSWrap_Callback_Base {
    protected:
      bool is_disposable;  // Should this callback be deleted automatically after first use?

    public:
      JSWrap_Callback_Base(bool in_disposable=false) : is_disposable(in_disposable) { ; }
      virtual ~JSWrap_Callback_Base() { ; }
      
      bool IsDisposable() const { return is_disposable; }
      void SetDisposable() { is_disposable = true; }
      
      // Base class to be called from Javascript (after storing args) to do a callback.
      virtual void DoCallback() = 0;


      // A pair of helper functions that systematically load ALL arguments from JS.
      template <typename TUPLE_TYPE, int ARGS_LEFT>
      struct Collect_impl {
        static void CollectArgs(TUPLE_TYPE & tuple) {
          LoadArg_impl<typename std::tuple_element<ARGS_LEFT-1,TUPLE_TYPE>::type, ARGS_LEFT-1>( true, std::get<ARGS_LEFT-1>(tuple) );  // Load an arg
          Collect_impl<TUPLE_TYPE, ARGS_LEFT-1>::CollectArgs(tuple);        // Recurse to next arg
        }
      };
      
      template <typename TUPLE_TYPE>
      struct Collect_impl<TUPLE_TYPE, 0> {
        static void CollectArgs(TUPLE_TYPE & tuple) { (void) tuple; } // End load recursion.
      };

    };


    // The derived form of JSWrap_Callback knows the specific argument types of the function
    // needed, keeps track of the function poninter, and has a tuple in which the arguments
    // can be loaded before a call is made.

    template <typename RET_TYPE, typename... ARG_TYPES>
    class JSWrap_Callback : public JSWrap_Callback_Base {
    private:
      std::function<RET_TYPE(ARG_TYPES...)> fun;   // Function to be wrapped

    public:
      JSWrap_Callback(const std::function<RET_TYPE(ARG_TYPES...)> & in_fun, bool disposable=false)
        : JSWrap_Callback_Base(disposable), fun(in_fun)
      {
        EMP_TRACK_CONSTRUCT(JSWrap_Callback);
      }
      ~JSWrap_Callback() { EMP_TRACK_DESTRUCT(JSWrap_Callback); }
      
      // This function is called from Javascript.  Arguments should be collected and then used
      // to call the target function.
      void DoCallback() {
        const int num_args = sizeof...(ARG_TYPES);  

        // Make sure that we are returning the correct number of arguments.  If this
        // assert fails, it means that we've failed to set the correct number of arguments
        // in emp.cb_args, and need to realign.
        emp_assert(EMP_GetCBArgCount < 0 || EMP_GetCBArgCount() == num_args);
        
        // Collect the values of the arguments in a tuple
        using args_t = std::tuple< typename std::decay<ARG_TYPES>::type... >;
        args_t args;
        Collect_impl<args_t, num_args>::CollectArgs(args);

        // And finally, do the actual callback.

        RET_TYPE return_val;
        emp::ApplyTuple([&return_val, this](ARG_TYPES... in_args){
            return_val = fun(in_args...);
          }, args);

        // And save the return value for JS.
        StoreReturn(return_val);
      }
    };

    // A specialized version of the class that handles functions with void returns.

    template <typename... ARG_TYPES>
    class JSWrap_Callback<void, ARG_TYPES...> : public JSWrap_Callback_Base {
    private:
      std::function<void(ARG_TYPES...)> fun;   // Function to be wrapped

    public:
      JSWrap_Callback(const std::function<void(ARG_TYPES...)> & in_fun, bool disposable=false)
        : JSWrap_Callback_Base(disposable), fun(in_fun)
      { EMP_TRACK_CONSTRUCT(JSWrap_Callback_VOID); }
      ~JSWrap_Callback() { EMP_TRACK_DESTRUCT(JSWrap_Callback_VOID); }

      
      // This function is called from Javascript.  Arguments should be collected and then used
      // to call the target function.
      void DoCallback() {
        const int num_args = sizeof...(ARG_TYPES);  

        // Make sure that we are returning the correct number of arguments.  If this
        // assert fails, it means that we've failed to set the correct number of arguments
        // in emp.cb_args, and need to realign.
        emp_assert(EMP_GetCBArgCount < 0 || EMP_GetCBArgCount() == num_args);
        
        // Collect the values of the arguments in a tuple 
        using args_t = std::tuple< typename std::decay<ARG_TYPES>::type... >;
        args_t args;
        Collect_impl<args_t, num_args>::CollectArgs(args);
        
        // And finally, do the actual callback.
        emp::ApplyTuple(fun, args);

        // And save a return value for JS.
        StoreReturn(0);
      }
    };

    
    // The following function returns a static callback array; callback ID's all index into
    // this array.
    static emp::vector<JSWrap_Callback_Base *> & CallbackArray() {
      static emp::vector<JSWrap_Callback_Base *> callback_array;
      return callback_array;
    }

  } // End internal namespace
  
  // The following JSWrap functions take a target function and return an integer id that
  // indexes into a callback array.

  // The first version assumes that we already have it enclosed in an std::function, while
  // the second version assumes we have a raw function pointer and wraps it for us.
  
  template <typename RET_TYPE, typename... ARG_TYPES>
  uint32_t JSWrap(const std::function<RET_TYPE(ARG_TYPES...)> & in_fun,
                  const std::string & fun_name="",
                  bool dispose_on_use=false)
  {
    // We should never create disposible functions with names!
    emp_assert(fun_name == "" || dispose_on_use == false);
    
    auto * new_cb =
      new emp::internal::JSWrap_Callback<RET_TYPE, ARG_TYPES...>(in_fun, dispose_on_use);
    auto & callback_array = internal::CallbackArray();
    uint32_t out_id = (int) callback_array.size();
    callback_array.push_back(new_cb);
    
    if (fun_name != "") {
      EM_ASM_ARGS({
          var fun_name = Pointer_stringify($1);
          emp[fun_name] = function() {
            emp_i.cb_args = [];
            for (var i = 0; i < arguments.length; i++) {
              emp_i.cb_args[i] = arguments[i];
            }
            
            // Callback to the original function.
            empCppCallback($0);

            // Return the resulting value
            return emp_i.cb_return;
          };
        }, out_id, fun_name.c_str());
    }
    
    return out_id;
  }

  // uint32_t JSWrap(const std::function<void()> & in_fun,
  //                 const std::string & fun_name="",
  //                 bool dispose_on_use=false)
  // {
  //   return 0;
  // }

  template <typename RETURN_TYPE, typename... ARG_TYPES>
  uint32_t JSWrap( RETURN_TYPE (*in_fun) (ARG_TYPES...),
                   const std::string & fun_name="", bool dispose_on_use=false)
  {
    std::function<RETURN_TYPE(ARG_TYPES...)> fun_ptr(in_fun);
    return JSWrap(fun_ptr, fun_name, dispose_on_use);
  }

  // template <typename FUN_TYPE>
  // uint32_t JSWrap(const FUN_TYPE & in_fun, const std::string & fun_name="", bool dispose_on_use=false)
  // {
  //   std::function<FUN_TYPE> fun_ptr(in_fun);
  //   return JSWrap(fun_ptr, fun_name, dispose_on_use);
  // }



  // If we want a quick, unnammed, disposable function, use JSWrapOnce
  template <typename FUN_TYPE>
  uint32_t JSWrapOnce(FUN_TYPE && in_fun) { return JSWrap(std::forward<FUN_TYPE>(in_fun), "", true); }


  // Cleanup a function pointer when finished with it.
  void JSDelete( uint32_t fun_id ) {
    emp_assert(fun_id > 0);  // Make sure this isn't a null pointer!
    // @CAO -- Should make sure to clean up named functions on JS side if they exist.
    auto & callback_array = internal::CallbackArray();
    delete callback_array[fun_id];
    callback_array[fun_id] = nullptr;
  }

}




// Once you use JSWrap to create an ID, you can call the wrapped function from Javascript
// by supplying CPPCallback with the id and all args.

extern "C" void empCppCallback(uint32_t cb_id)
{
  // Convert the uint passed in from 32 bits to 64 and THEN convert it to a pointer.
  auto * cb_obj = emp::internal::CallbackArray()[cb_id];

  // Run DoCallback() on the generic base class type, which is virtual and will call 
  // the correct template automatically.
  cb_obj->DoCallback();
  
  // If we have indicated that this callback is single use, delete it now.
  if (cb_obj->IsDisposable()) {
    delete cb_obj;
    emp::internal::CallbackArray()[cb_id] = nullptr;
  }
}


#endif
