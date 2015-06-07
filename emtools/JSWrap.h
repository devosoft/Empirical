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

#include <functional>
#include <tuple>

#include "../tools/assert.h"
#include "../tools/functions.h"

extern "C" {
  extern int EMP_GetCBArgCount();  // Get the number of arguments associated with a callback.
};


namespace emp {
  
  // The following code is in the "internal" namespace since it's used only to implement the
  // details of the JSWrap function.

  namespace internal {

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

      template <int ARG_ID>
      static void LoadArg(std::string & arg_var) {
        char * tmp_var = (char *) EM_ASM_INT({
            return allocate(intArrayFromString(emp_i.cb_args[$0]), 'i8', ALLOC_STACK);
          }, ARG_ID);
        arg_var = tmp_var;   // @CAO Do we need to free the memory in tmp_var?
      }

      template <class ARG_TYPE, int ARG_ID>
      static void LoadArg(ARG_TYPE & arg_var) {
        arg_var.template LoadFromArg<ARG_ID>();
      }


      // Helper functions to individually STORE RETURN VALUES to JS

      static void StoreReturn(const int & ret_var) {
        EM_ASM_ARGS({ emp_i.cb_return = $0; }, ret_var);
      }

      static void StoreReturn(const double & ret_var) {
        EM_ASM_ARGS({ emp_i.cb_return = $0; }, ret_var);
      }

      static void StoreReturn(const std::string & ret_var) {
        EM_ASM_ARGS({ emp_i.cb_return = Pointer_stringify($0); }, ret_var.c_str());
      }

      // A pair of helper functions that systematically load ALL arguments from JS.
      template <typename TUPLE_TYPE, int ARGS_LEFT>
      struct Collect_impl {
        static void CollectArgs(TUPLE_TYPE & tuple) {
          LoadArg<ARGS_LEFT-1>(std::get<ARGS_LEFT-1>(tuple) );      // Load an arg
          Collect_impl<TUPLE_TYPE, ARGS_LEFT-1>::CollectArgs(tuple); // Recurse to load next arg
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
      JSWrap_Callback(const std::function<RET_TYPE(ARG_TYPES...)> & in_fun,
                      bool in_disposable=false)
        : JSWrap_Callback_Base(in_disposable), fun(in_fun) { ; }
      ~JSWrap_Callback() { ; }
      
      // This function is called from Javascript.  Arguments should be collected and then used
      // to call the target function.
      void DoCallback() {
        const int num_args = sizeof...(ARG_TYPES);  

        // Make sure that we are returning the correct number of arguments.  If this
        // assert fails, it means that we've failed to set the correct number of arguments
        // in emp.cb_args, and need to realign.
        emp_assert(EMP_GetCBArgCount() == num_args);
        
        // Collect the values of the arguments
        std::tuple<ARG_TYPES...> args;           // Argument values to call function with.
        Collect_impl<std::tuple<ARG_TYPES...>, num_args>::CollectArgs(args);
        
        // And finally, do the actual callback.
        // emp::ApplyTuple(fun, args);

        RET_TYPE return_val;
        emp::ApplyTuple([&return_val, this](ARG_TYPES... in_args){ return_val = fun(in_args...); }, args);

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
      JSWrap_Callback(const std::function<void(ARG_TYPES...)> & in_fun, bool in_disposable=false)
        : JSWrap_Callback_Base(in_disposable), fun(in_fun) { ; }
      ~JSWrap_Callback() { ; }
      
      // This function is called from Javascript.  Arguments should be collected and then used
      // to call the target function.
      void DoCallback() {
        const int num_args = sizeof...(ARG_TYPES);  

        // Make sure that we are returning the correct number of arguments.  If this
        // assert fails, it means that we've failed to set the correct number of arguments
        // in emp.cb_args, and need to realign.
        emp_assert(EMP_GetCBArgCount() == num_args);
        
        // Collect the values of the arguments
        std::tuple<ARG_TYPES...> args;           // Argument values to call function with.
        Collect_impl<std::tuple<ARG_TYPES...>, num_args>::CollectArgs(args);
        
        // And finally, do the actual callback.
        // emp::ApplyTuple(fun, args);

        emp::ApplyTuple(fun, args);

        // And save a return value for JS.
        StoreReturn(0);
      }
    };


  };
  
  // The following JSWrap functions take a target function and return an integer id.
  // The first version assumes that we already have it enclosed in an std::function, while
  // the second version assumes we have a raw function pointer and wraps it for us.
  
  template <typename RET_TYPE, typename... ARG_TYPES>
  uint32_t JSWrap(const std::function<RET_TYPE(ARG_TYPES...)> & in_fun,
                  const std::string & fun_name="",
                  bool dispose_on_use=false)
  {
    // We should never create disposible functions with names!
    emp_assert(fun_name == "" || dispose_on_use == false);
    
    auto * new_cb = new emp::internal::JSWrap_Callback<RET_TYPE, ARG_TYPES...>(in_fun, dispose_on_use);
    
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
        }, (uint32_t) new_cb, fun_name.c_str());
    }
    
    return (uint32_t) new_cb;
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
    // @CAO -- make sure we're not trying to delete a named function; JS side will still exist.
    delete (emp::internal::JSWrap_Callback_Base *) (long long) fun_id;
  }
};




// Once you use JSWrap to create an ID, you can call the wrapped function from Javascript
// by supplying CPPCallback with the id and all args.

extern "C" void empCppCallback(uint32_t cb_ptr)
{
  // Convert the uint passed in from 32 bits to 64 and THEN convert it to a pointer.
  auto * cb_obj = (emp::internal::JSWrap_Callback_Base *) (long long) cb_ptr;

  // Run DoCallback() on the generic base class type, which is virtual and will call 
  // the correct template automatically.
  cb_obj->DoCallback();
  
  // If we have indicated that this callback is single use, delete it now.
  if (cb_obj->IsDisposable()) {
    delete cb_obj;
  }
}


#endif
