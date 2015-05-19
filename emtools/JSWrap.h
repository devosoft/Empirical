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
  
  namespace internal {

    template<typename TUPLE_TYPE, int ARG_ID,  int FIRST_ARG, typename... OTHER_ARGS>
    void JSWrap_CollectArgs(TUPLE_TYPE & tuple) {
        std::get<ARG_ID>(tuple) = EM_ASM_INT({ return emp_data.callback_args[$0]; }, ARG_ID);
        JSWrap_CollectArgs<TUPLE_TYPE, ARG_ID+1, OTHER_ARGS...>(tuple);
    }

    template<typename TUPLE_TYPE, int ARG_ID>
    void JSWrap_CollectArgs(TUPLE_TYPE & tuple) {
      // @CAO Add a static assert to make sure the ARG_ID is the tuple size?
      (void) tuple;
    }
    

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
      virtual void DoCallback() {
        const int num_args = sizeof...(ARG_TYPES);  
        
        // Make sure that we are returning the correct number of arguments.
        emp_assert(EMP_GetCBArgCount() == num_args);
        
        // Collect the values of the arguments
        JSWrap_CollectArgs<std::tuple<ARG_TYPES...>, 0, ARG_TYPES...>(args);
        
        // And finally, do the actual callback.
        ApplyTuple(fun, args);
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

