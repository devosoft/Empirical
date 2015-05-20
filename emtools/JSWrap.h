#ifndef EMP_JSWRAP_H
#define EMP_JSWRAP_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Wrap a C++ function and convert it to an integer that can be called from Javascript
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
    // Specifically, it allows a virtual DoCallback() member function that can be called
    // to trigger the wrapped function.

    class JSWrap_Callback_Base {
    protected:
      bool is_disposable;

    public:
      JSWrap_Callback_Base(bool in_disposable=false) : is_disposable(in_disposable) { ; }
      virtual ~JSWrap_Callback_Base() { ; }
      
      bool IsDisposable() const { return is_disposable; }
      void SetDisposable() { is_disposable = true; }
      
      // Base class to be called from Javascript (after storing args) to do a callback.
      virtual void DoCallback() = 0;

      static void LoadArg(int arg_id, int & arg_var) {
        arg_var = EM_ASM_INT({ return emp_data.callback_args[$0]; }, arg_id);
      }
      static void LoadArg(int arg_id, double & arg_var) {
        arg_var = EM_ASM_DOUBLE({ return emp_data.callback_args[$0]; }, arg_id);
      }

    };


    // The CollectArgs methods below takes a tuple and trasfer all of the arguments saved
    // in JaveScript into the tuple.

    template <typename TUPLE_TYPE, int ARGS_LEFT>
    struct JSWrap_Collect_impl {
      static void CollectArgs(TUPLE_TYPE & tuple) {
        JSWrap_Callback_Base::LoadArg( ARGS_LEFT-1, std::get<ARGS_LEFT-1>(tuple) );
        JSWrap_Collect_impl<TUPLE_TYPE, ARGS_LEFT-1>::CollectArgs(tuple);
      }
    };

    template <typename TUPLE_TYPE>
    struct JSWrap_Collect_impl<TUPLE_TYPE, 0> {
      static void CollectArgs(TUPLE_TYPE & tuple) {
        (void) tuple;
      }
    };


    template <typename... ARG_TYPES>
    class JSWrap_Callback : public JSWrap_Callback_Base {
    private:
      std::function<void(ARG_TYPES...)> fun;
      std::tuple<ARG_TYPES...> args;

    public:
      JSWrap_Callback(std::function<void(ARG_TYPES...)> in_fun, bool in_disposable=false)
        : JSWrap_Callback_Base(in_disposable) { ; }
      ~JSWrap_Callback() { ; }
      
      // This function is called from Javascript.  Arguments should be collected and then used
      // to call the target function.
      void DoCallback() {
        const int num_args = sizeof...(ARG_TYPES);  
        
        // Make sure that we are returning the correct number of arguments.
        emp_assert(EMP_GetCBArgCount() == num_args);
        
        // Collect the values of the arguments
        JSWrap_Collect_impl<std::tuple<ARG_TYPES...>, num_args>::CollectArgs(args);
        
        // And finally, do the actual callback.
        emp::ApplyTuple(fun, args);
      }

    };

  };
  
  // The following function (JSWrap) takes a target function and returns an integer id.
  // To call the target function from Javascript, supply CPPCallback with the id and all args.
  
  template <typename... ARG_TYPES>
  unsigned int JSWrap(std::function<void(ARG_TYPES...)> in_fun, bool dispose_on_use=false)
  {
    auto * new_cb = new emp::internal::JSWrap_Callback<ARG_TYPES...>(in_fun, dispose_on_use);
    return (unsigned int) new_cb;
  }

};


extern "C" void empCppCallback(unsigned int cb_ptr)
{
  auto * cb_obj = (emp::internal::JSWrap_Callback_Base *) (long long) cb_ptr;
  cb_obj->DoCallback();
  
  if (cb_obj->IsDisposable()) {
    delete cb_obj;
  }
}


#endif

