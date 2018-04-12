/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2018
 *
 *  @file  JSWrap.h
 *  @brief Wrap a C++ function and convert it to an integer that can be called from Javascript
 *
 *  To wrap a function, call:
 *
 *     `uint32_t fun_id = emp::JSWrap(FunctionToBeWrapped, "JS_Function_Name");``
 *
 *  To manually callback a function from Javascript, first set `emp_i.cb_args` to an array of
 *  function arguments, then call `empCppCallback( fun_id );`   This all happens automatically
 *  if you use the `emp.Callback(fun_id, args...)` function from Javascript.
 *
 *  The JS_Function_Name string is optional, but if you use it, the appropriate function will
 *  be automatically generated in Javascript by JSWrap, in the emp class.
 *
 *  For example, if you have:
 *
 *     `int AddPair(int x, int y) { return x + y; }``
 *
 *  You can wrap it with:
 *
 *     `size_t fun_id = emp::JSWrap(AddPair, "AddPair");`
 *
 *  And then in Javascript, you can simply call it as:
 *
 *     `emp.AddPair(4, 5); // will return 9.`
 *
 *
 *  @todo Add a JSWrap that takes an object and method and does the bind automatically.
 *  @todo Build a non-enscripten version; it should still be callable from the C++ side, but
 *        mostly to be able to test programs without Emscripten.
 *
 */

#ifndef EMP_JSWRAP_H
#define EMP_JSWRAP_H


#include <array>
#include <functional>
#include <tuple>

#include "../meta/meta.h"

#include "../base/assert.h"
#include "../base/vector.h"

#include "../tools/functions.h"
#include "../tools/mem_track.h"
#include "../tools/tuple_struct.h"
#include "../tools/tuple_utils.h"

#include "init.h"
#include "js_utils.h"

#ifdef EMSCRIPTEN
extern "C" {
  extern int EMP_GetCBArgCount();  // Get the number of arguments associated with a callback.
}
#else
// When NOT in Emscripten, need a stub for this function.
int EMP_GetCBArgCount() { return -1; }
#endif

namespace emp {

  /// Helper functions to individually LOAD ARGUMENTS from JS based on expected type.
  template <int ARG_ID> static void LoadArg(int16_t & arg_var) {
    arg_var = (int16_t) EM_ASM_INT({ return emp_i.cb_args[$0]; }, ARG_ID);
  }

  template <int ARG_ID> static void LoadArg(int32_t & arg_var) {
    arg_var = (int32_t) EM_ASM_INT({ return emp_i.cb_args[$0]; }, ARG_ID);
  }

  template <int ARG_ID> static void LoadArg(int64_t & arg_var) {
    arg_var = (int64_t) EM_ASM_DOUBLE({ return emp_i.cb_args[$0]; }, ARG_ID);
  }

  template <int ARG_ID> static void LoadArg(uint16_t & arg_var) {
    arg_var = (uint16_t) EM_ASM_INT({ return emp_i.cb_args[$0]; }, ARG_ID);
  }

  template <int ARG_ID> static void LoadArg(uint32_t & arg_var) {
    arg_var = (uint32_t) EM_ASM_INT({ return emp_i.cb_args[$0]; }, ARG_ID);
  }

  template <int ARG_ID> static void LoadArg(uint64_t & arg_var) {
    arg_var = (uint64_t) EM_ASM_DOUBLE({ return emp_i.cb_args[$0]; }, ARG_ID);
  }

  template <int ARG_ID> static void LoadArg(bool & arg_var) {
    arg_var = (bool) EM_ASM_INT({ return emp_i.cb_args[$0]; }, ARG_ID);
  }

  template <int ARG_ID> static void LoadArg(char & arg_var) {
    arg_var = (char) EM_ASM_INT({ return emp_i.cb_args[$0]; }, ARG_ID);
  }

  template <int ARG_ID> static void LoadArg(double & arg_var) {
    arg_var = EM_ASM_DOUBLE({ return emp_i.cb_args[$0]; }, ARG_ID);
  }

  template <int ARG_ID> static void LoadArg(float & arg_var) {
    arg_var = (float) EM_ASM_DOUBLE({ return emp_i.cb_args[$0]; }, ARG_ID);
  }

  template <int ARG_ID> static void LoadArg(std::string & arg_var) {
    char * tmp_var = (char *) EM_ASM_INT({
        return allocate(intArrayFromString(emp_i.cb_args[$0]), 'i8', ALLOC_STACK);
      }, ARG_ID);
    arg_var = tmp_var;   // @CAO Do we need to free the memory in tmp_var?
  }

  template <int ARG_ID, size_t SIZE, typename T> static void LoadArg(emp::array<T, SIZE> & arg_var){
    EM_ASM_ARGS({emp_i.__outgoing_array = emp_i.cb_args[$0];}, ARG_ID);
    pass_array_to_cpp(arg_var);
  }

  template <int ARG_ID, typename T> static void LoadArg(emp::vector<T> & arg_var){
    EM_ASM_ARGS({emp_i.__outgoing_array = emp_i.cb_args[$0];}, ARG_ID);
    pass_vector_to_cpp(arg_var);
  }

  //Helper functions to load arguments from inside Javascript objects by name.
  template <int ARG_ID> static void LoadArg(int16_t & arg_var, std::string var) {
    arg_var = (int16_t) EM_ASM_INT({
      return emp_i.curr_obj[Pointer_stringify($0)];
    }, var.c_str());
  }

  template <int ARG_ID> static void LoadArg(int32_t & arg_var, std::string var) {
    arg_var = (int32_t) EM_ASM_INT({
      return emp_i.curr_obj[Pointer_stringify($0)];
    }, var.c_str());
  }

  template <int ARG_ID> static void LoadArg(int64_t & arg_var, std::string var) {
    arg_var = (int64_t) EM_ASM_DOUBLE({
      return emp_i.curr_obj[Pointer_stringify($0)];
    }, var.c_str());
  }

  template <int ARG_ID> static void LoadArg(uint16_t & arg_var, std::string var) {
    arg_var = (uint16_t) EM_ASM_INT({
      return emp_i.curr_obj[Pointer_stringify($0)];
    }, var.c_str());
  }

  template <int ARG_ID> static void LoadArg(uint32_t & arg_var, std::string var) {
    arg_var = (uint32_t) EM_ASM_INT({
      return emp_i.curr_obj[Pointer_stringify($0)];
    }, var.c_str());
  }

  template <int ARG_ID> static void LoadArg(uint64_t & arg_var, std::string var) {
    arg_var = (uint64_t) EM_ASM_DOUBLE({
      return emp_i.curr_obj[Pointer_stringify($0)];
    }, var.c_str());
  }

  template <int ARG_ID> static void LoadArg(bool & arg_var, std::string var) {
    arg_var = (bool) EM_ASM_INT({
      return emp_i.curr_obj[Pointer_stringify($0)];
    }, var.c_str());
  }

  template <int ARG_ID> static void LoadArg(char & arg_var, std::string var) {
    arg_var = (char) EM_ASM_INT({
      return emp_i.curr_obj[Pointer_stringify($0)];
    }, var.c_str());
  }

  template <int ARG_ID> static void LoadArg(double & arg_var, std::string var) {
    arg_var = EM_ASM_DOUBLE({
      return emp_i.curr_obj[Pointer_stringify($0)];
    }, var.c_str());
  }

  template <int ARG_ID> static void LoadArg(float & arg_var, std::string var) {
    arg_var = (float) EM_ASM_DOUBLE({
      return emp_i.curr_obj[Pointer_stringify($0)];
    }, var.c_str());
  }

  template <int ARG_ID> static void LoadArg(std::string & arg_var, std::string var) {
    char * tmp_var = (char *) EM_ASM_INT({
      if (emp_i.curr_obj[Pointer_stringify($0)] == null){
        emp_i.curr_obj[Pointer_stringify($0)] = "undefined";
      }
      return allocate(intArrayFromString(emp_i.curr_obj[Pointer_stringify($0)]),
                      'i8', ALLOC_STACK);
    }, var.c_str());
    arg_var = tmp_var;   // Free memory here?
  }

  template <typename JSON_TYPE, int ARG_ID, int FIELD>
  struct LoadTuple;

  /// This needs to go before LoadTuple is defined, in case there are nested tuple structs
  template <int ARG_ID, typename JSON_TYPE> static
  typename std::enable_if<JSON_TYPE::n_fields != -1, void>::type
  LoadArg(JSON_TYPE & arg_var, std::string var) {
    //std::cout << "Loading " << var << " ARGNID: " << ARG_ID << std::endl;
    //LoadArg<ARG_ID>(std::get<ARG_ID>(arg_var.emp__tuple_body));
    EM_ASM_ARGS({
      emp_i.object_queue.push(emp_i.curr_obj);
      emp_i.curr_obj = emp_i.curr_obj[Pointer_stringify($0)];
    }, var.c_str());
    LoadTuple<JSON_TYPE, ARG_ID, JSON_TYPE::n_fields> load_tuple = LoadTuple<JSON_TYPE, ARG_ID, JSON_TYPE::n_fields>();
    load_tuple.LoadJSDataArg(arg_var);
  }

  template <typename JSON_TYPE, int ARG_ID, int FIELD>
  struct LoadTuple {
    static void LoadJSDataArg(JSON_TYPE & arg_var) {
    //std::cout << "LoadingJS " << arg_var.var_names[FIELD-1] << " FIeLd: " << FIELD-1 << std::endl;
      LoadArg<ARG_ID>(std::get<FIELD-1>(arg_var.emp__tuple_body), arg_var.var_names[FIELD-1]);
      LoadTuple<JSON_TYPE, ARG_ID, FIELD-1> load_tuple = LoadTuple<JSON_TYPE, ARG_ID, FIELD-1>();
      load_tuple.LoadJSDataArg(arg_var);
    }
  };

  template <typename JSON_TYPE, int ARG_ID>
  struct LoadTuple<JSON_TYPE, ARG_ID, 0> {
    static void LoadJSDataArg(JSON_TYPE & arg_var) {
        EM_ASM({emp_i.curr_obj = emp_i.object_queue.pop();});
    }
  };


  template <int ARG_ID, typename JSON_TYPE> static
  typename std::enable_if<JSON_TYPE::n_fields != -1, void>::type
  LoadArg(JSON_TYPE & arg_var) {
    //std::cout << "Loading ARGNID: " << ARG_ID << std::endl;
    EM_ASM_ARGS({
      emp_i.object_queue = [];
      emp_i.curr_obj = emp_i.cb_args[$0];
    }, ARG_ID);
    LoadTuple<JSON_TYPE, ARG_ID, JSON_TYPE::n_fields> load_tuple = LoadTuple<JSON_TYPE, ARG_ID, JSON_TYPE::n_fields>();
    load_tuple.LoadJSDataArg(arg_var);
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

  template <typename T, size_t N>
  static void StoreReturn(const emp::array<T, N> & ret_var) {
    pass_array_to_javascript(ret_var);
    EM_ASM({ emp_i.cb_return = emp_i.__incoming_array; });
  }

  /// If the return type has a personalized function to handle the return, use it!
  template <typename RETURN_TYPE>
  static emp::sfinae_decoy<void, decltype(&RETURN_TYPE::StoreAsReturn)>
  StoreReturn(const RETURN_TYPE & ret_var) {
    ret_var.template StoreAsReturn();
  }

  /// Helper functions to store values inside JSON objects
  static void StoreReturn(const int & ret_var, std::string var) {
    EM_ASM_ARGS({ emp_i.curr_obj[Pointer_stringify($1)] = $0; }, ret_var, var.c_str());
  }

  static void StoreReturn(const double & ret_var, std::string var) {
    EM_ASM_ARGS({ emp_i.curr_obj[Pointer_stringify($1)] = $0; }, ret_var, var.c_str());
  }

  static void StoreReturn(const std::string & ret_var, std::string var) {
    EM_ASM_ARGS({ emp_i.curr_obj[Pointer_stringify($1)] = Pointer_stringify($0); }
                                                    , ret_var.c_str(), var.c_str());
  }

  template <typename T, size_t N>
  static void StoreReturn(const emp::array<T, N> & ret_var, std::string var) {
    pass_array_to_javascript(ret_var);
    EM_ASM_ARGS({ emp_i.curr_obj[Pointer_stringify($0)] = emp_i.__incoming_array;}, var.c_str());
  }

  template <typename JSON_TYPE, int FIELD>
  struct StoreTuple;

  // Tuple struct
  template <typename RETURN_TYPE>
  static typename std::enable_if<RETURN_TYPE::n_fields != -1, void>::type
  StoreReturn(const RETURN_TYPE & ret_var) {
    EM_ASM({
      emp_i.cb_return = {};
      emp_i.object_queue = [];
      emp_i.curr_obj = emp_i.cb_return;
    });

    StoreTuple<RETURN_TYPE, RETURN_TYPE::n_fields> store_tuple = StoreTuple<RETURN_TYPE, RETURN_TYPE::n_fields>();
    store_tuple.StoreJSDataArg(ret_var);
  }

  // Nested tuple struct
  template <typename RETURN_TYPE>
  static emp::sfinae_decoy<void, decltype(RETURN_TYPE::n_fields)>
  StoreReturn(const RETURN_TYPE & ret_var, std::string var) {
    EM_ASM_ARGS({
      emp_i.curr_obj[Pointer_stringify($0)] = {};
      emp_i.object_queue.push(emp_i.curr_obj);
      emp_i.curr_obj = emp_i.curr_obj[Pointer_stringify($0)];
    }, var.c_str());

    StoreTuple<RETURN_TYPE, RETURN_TYPE::n_fields> store_tuple = StoreTuple<RETURN_TYPE, RETURN_TYPE::n_fields>();
    store_tuple.StoreJSDataArg(ret_var);
  }

  template <typename JSON_TYPE, int FIELD>
  struct StoreTuple {
    static void StoreJSDataArg(const JSON_TYPE & ret_var) {
      StoreReturn(std::get<FIELD-1>(ret_var.emp__tuple_body), ret_var.var_names[FIELD-1]);
      StoreTuple<JSON_TYPE, FIELD-1> store_tuple = StoreTuple<JSON_TYPE, FIELD-1>();
      store_tuple.StoreJSDataArg(ret_var);
    }
  };

  template <typename JSON_TYPE>
  struct StoreTuple<JSON_TYPE, 0> {
    static void StoreJSDataArg(const JSON_TYPE & ret_var) {
      EM_ASM({emp_i.curr_obj = emp_i.object_queue.pop();});
    }
  };

  // The following code is in the "internal" namespace since it's used only to implement the
  // details of the JSWrap function.

  namespace internal {

    template <typename T, int ARG_ID>
    void LoadArg_impl(emp::sfinae_decoy<bool, decltype(&T::template LoadFromArg<ARG_ID>)>,
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
        emp_assert(EMP_GetCBArgCount() < 0 || EMP_GetCBArgCount() >= num_args, EMP_GetCBArgCount(), num_args);

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
        emp_assert(EMP_GetCBArgCount() < 0 || EMP_GetCBArgCount() >= num_args, EMP_GetCBArgCount(), num_args);

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
      static emp::vector<JSWrap_Callback_Base *> callback_array(1, nullptr);
      return callback_array;
    }

  } // End internal namespace

  // The following JSWrap functions take a target function and return an integer id that
  // indexes into a callback array.

  // The first version assumes that we already have it enclosed in an std::function, while
  // the second version assumes we have a raw function pointer and wraps it for us.

  template <typename RET_TYPE, typename... ARG_TYPES>
  size_t JSWrap(const std::function<RET_TYPE(ARG_TYPES...)> & in_fun,
                const std::string & fun_name="",
                bool dispose_on_use=false)
  {
    // We should never create disposible functions with names!
    emp_assert(fun_name == "" || dispose_on_use == false);

    auto * new_cb =
      new emp::internal::JSWrap_Callback<RET_TYPE, ARG_TYPES...>(in_fun, dispose_on_use);
    auto & callback_array = internal::CallbackArray();
    size_t out_id = callback_array.size();
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

  template <typename RETURN_TYPE, typename... ARG_TYPES>
  size_t JSWrap( RETURN_TYPE (*in_fun) (ARG_TYPES...),
                 const std::string & fun_name="", bool dispose_on_use=false )
  {
    std::function<RETURN_TYPE(ARG_TYPES...)> fun_ptr(in_fun);
    return JSWrap(fun_ptr, fun_name, dispose_on_use);
  }

  /// @endcond

  template <typename FUN_TYPE>
  size_t JSWrap(const FUN_TYPE & in_fun, const std::string & fun_name="", bool dispose_on_use=false)
  {
    return JSWrap(to_function(in_fun), fun_name, dispose_on_use);
  }


  /// If we want a quick, unnammed, disposable function, use JSWrapOnce
  template <typename FUN_TYPE>
  size_t JSWrapOnce(FUN_TYPE && in_fun) { return JSWrap(std::forward<FUN_TYPE>(in_fun), "", true); }


  /// Cleanup a function pointer when finished with it.
  void JSDelete( size_t fun_id ) {
    emp_assert(fun_id > 0);  // Make sure this isn't a null pointer!
    // @CAO -- Should make sure to clean up named functions on JS side if they exist.
    auto & callback_array = internal::CallbackArray();
    delete callback_array[fun_id];
    callback_array[fun_id] = nullptr;
  }

  /// @cond SIMPLIFY
}

/// Once you use JSWrap to create an ID, you can call the wrapped function from Javascript
/// by supplying CPPCallback with the id and all args.

extern "C" void empCppCallback(size_t cb_id)
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

/// @endcond

#endif
