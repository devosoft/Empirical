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

      // Helper functions to load individual arguments from JS based on expected type.
      static void LoadArg(int arg_id, int & arg_var) {
        arg_var = EM_ASM_INT({ return emp_i.cb_args[$0]; }, arg_id);
      }
      static void LoadArg(int arg_id, double & arg_var) {
        arg_var = EM_ASM_DOUBLE({ return emp_i.cb_args[$0]; }, arg_id);
      }

      // A pair of helper functions that systematically load ALL arguments from JS.
      template <typename TUPLE_TYPE, int ARGS_LEFT>
      struct Collect_impl {
        static void CollectArgs(TUPLE_TYPE & tuple) {
          LoadArg( ARGS_LEFT-1, std::get<ARGS_LEFT-1>(tuple) );      // Load an arg
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

    template <typename... ARG_TYPES>
    class JSWrap_Callback : public JSWrap_Callback_Base {
    private:
      std::function<void(ARG_TYPES...)> fun;   // Function to be wrapped
      std::tuple<ARG_TYPES...> args;           // Argument values to call function with.

    public:
      JSWrap_Callback(std::function<void(ARG_TYPES...)> & in_fun, bool in_disposable=false)
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
        Collect_impl<std::tuple<ARG_TYPES...>, num_args>::CollectArgs(args);
        
        // And finally, do the actual callback.
        emp::ApplyTuple(fun, args);
      }
    };


  };
  
  // The following JSWrap functions take a target function and return an integer id.
  // The first version assumes that we already have it enclosed in an std::function, while
  // the second version assumes we have a raw function pointer and wraps it for us.
  
  template <typename... ARG_TYPES>
  uint32_t JSWrap(std::function<void(ARG_TYPES...)> & in_fun,
                  const std::string & fun_name="",
                  bool dispose_on_use=false)
  {
    auto * new_cb = new emp::internal::JSWrap_Callback<ARG_TYPES...>(in_fun, dispose_on_use);
    
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
          };
        }, (uint32_t) new_cb, fun_name.c_str());
    }
    
    return (uint32_t) new_cb;
  }

  template <typename FUN_TYPE>
  uint32_t JSWrap(FUN_TYPE & in_fun, const std::string & fun_name="", bool dispose_on_use=false)
  {
    std::function<FUN_TYPE> fun_ptr(in_fun);
    return JSWrap(fun_ptr, fun_name, dispose_on_use);
  }


  // If we want a quick, unnammed, disposable function, use JSWrapOnce
  template <typename T>
  uint32_t JSWrapOnce(T && in_fun) { return JSWrap(in_fun, "", true); }
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
