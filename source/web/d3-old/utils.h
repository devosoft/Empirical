//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
// This file contains macros used to build Empirical's C++ wrapper for D3

#ifndef __UTILS_H__
#define __UTILS_H__

#include <map>
#include <string>

#include "../../base/macros.h"
#include "../init.h"
#include "../js_utils.h"
#include "../JSWrap.h"

// Helper macros

/// A more descriptive wrapper for the check to see if something is a function in Javascript
#define IS_JS_FUNCTION(FN) (typeof FN === "function")

/// Expects func_string to be a pre-defined variable that holds a string that may or may
/// not be a function and checks to see if it is a function defined within the current window.
/// If it is, it stores that function in the variable func_string.
#define CONVERT_FUNCSTRING_TO_FUNCTION_IF_IN_WINDOW()          \
  do {                                                         \
    if IS_JS_FUNCTION(window[func_string]) {                   \
      func_string = window[func_string];                       \
    }                                                          \
  } while (0);

//Check if func_string is in a single namespace
#define CONVERT_FUNCSTRING_TO_FUNCTION_IF_IN_NAMESPACE_1(A1)   \
  if IS_JS_FUNCTION(window[A1][func_string]){                  \
    func_string = window[A1][func_string];                     \
  }

//Check 2 namespaces
#define CONVERT_FUNCSTRING_TO_FUNCTION_IF_IN_NAMESPACE_2(A1, A2) \
  CONVERT_FUNCSTRING_TO_FUNCTION_IF_IN_NAMESPACE_1(A1)           \
  else CONVERT_FUNCSTRING_TO_FUNCTION_IF_IN_NAMESPACE_1(A2)

//Check 3 namespaces
#define CONVERT_FUNCSTRING_TO_FUNCTION_IF_IN_NAMESPACE_3(A1, A2, A3)   \
  CONVERT_FUNCSTRING_TO_FUNCTION_IF_IN_NAMESPACE_2(A1, A2)             \
  else CONVERT_FUNCSTRING_TO_FUNCTION_IF_IN_NAMESPACE_1(A3)

/// Expects func_string to be a pre-defined variable that holds a string that may or may
/// not be a function and checks to see if it is a function defined within and of the
/// given namespaces. If it is, it stores that function in the variable func_string.
/// Namespaces are checked in the order given and the first function found is used.
#define CONVERT_FUNCSTRING_TO_FUNCTION_IF_IN_NAMESPACE(...)            \
    do {EMP_ASSEMBLE_MACRO(CONVERT_FUNCSTRING_TO_FUNCTION_IF_IN_NAMESPACE_, __VA_ARGS__);} while (0);

/// Expects func_string to be a pre-defined variable that holds a string that may or may
/// not be a function and checks to see if it is a function defined within and of the
/// given namespaces or the current window. If it is, it stores that function in the
/// variable func_string. Namespaces are checked in the order given and the first function
/// found is used.
#define CONVERT_FUNCSTRING_TO_FUNCTION_IF_IN_NAMESPACE_OR_WINDOW(...)  \
  CONVERT_FUNCSTRING_TO_FUNCTION_IF_IN_NAMESPACE( __VA_ARGS__)         \
  CONVERT_FUNCSTRING_TO_FUNCTION_IF_IN_WINDOW()

//Functions that accept callback functions

/// Call a Javascript function that accepts either a string indicating the name of a
/// callback function or a normal string
#define D3_CALLBACK_FUNCTION_1_ARG(FUNC, CALLBACK)                             \
  EM_ASM_ARGS({                                                                \
      var func_string = UTF8ToString($0);                                 \
      CONVERT_FUNCSTRING_TO_FUNCTION_IF_IN_NAMESPACE_OR_WINDOW("d3", "emp");   \
      emp.__new_object = FUNC(func_string);                                    \
    }, CALLBACK);

/// Same as D3_CALLBACK_FUNCTION_1_ARG, but accepts an additional argument, ARG1, that goes
/// before the callback function in the call to FUNC
#define D3_CALLBACK_FUNCTION_2_ARGS(FUNC, CALLBACK, ARG1)                      \
  EM_ASM_ARGS({                                                                \
      var arg1 = UTF8ToString($0);                                        \
      var func_string = UTF8ToString($1);                                 \
      CONVERT_FUNCSTRING_TO_FUNCTION_IF_IN_NAMESPACE_OR_WINDOW("d3", "emp");   \
      emp.__new_object = FUNC(arg1, func_string);                              \
    }, ARG1, CALLBACK);

//Methods that accept callback functions (intended to be used within methods for d3 objects)

//Layer of indirection so macro gets expanded
#define D3_CALLBACK_METHOD_2_ARGS_IMPL(MACRO, FUNC, ARG1, ARG2)     \
  EM_ASM_ARGS({                                                     \
      var arg1 = UTF8ToString($1);                             \
      var func_string = UTF8ToString($2);                      \
      MACRO;                                                        \
      emp.__new_object = js.objects[$0].FUNC(arg1, func_string);    \
  }, this->id, ARG1, ARG2);

//This macro finds a function specified by ARG2 in either the d3, emp, or
//window namespace and feeds it to FUNC, which is called on the current
//d3 object. ARG1 is the first argument to FUNC. If ARG2 is not found to
//be a function, it will be passed to FUNC as a string.
#define D3_CALLBACK_METHOD_2_ARGS(FUNC, ARG1, ARG2)          \
  D3_CALLBACK_METHOD_2_ARGS_IMPL(CONVERT_FUNCSTRING_TO_FUNCTION_IF_IN_NAMESPACE_OR_WINDOW("d3", "emp"), FUNC, ARG1, ARG2);

//Layer of indirection so macro gets expanded
#define D3_CALLBACK_METHOD_1_ARG_IMPL(MACRO, FUNC, ARG1)    \
EM_ASM_ARGS({                                               \
    var func_string = UTF8ToString($1);                \
    MACRO;                                                  \
    emp.__new_object = js.objects[$0].FUNC(func_string);    \
}, this->id, ARG1);

/// The same as D3_CALLBACK_METHOD_2_ARGS except for FUNCs
/// that only accept a single argument (the string that might or might not
/// be a function).
#define D3_CALLBACK_METHOD_1_ARG(FUNC, ARG1)          \
D3_CALLBACK_METHOD_1_ARG_IMPL(CONVERT_FUNCSTRING_TO_FUNCTION_IF_IN_NAMESPACE_OR_WINDOW("d3", "emp"), FUNC, ARG1);

//Wraps CPP_FUN (a C++ function pointer, std::function object, or lambda) with
//JSWrap and passes it to the FUNC method of the current d3 object, along with an argument
#define D3_CALLBACK_METHOD_CPP_FUNCTION_2_ARGS(FUNC, ARG1, CPP_FUN)        \
  uint32_t fun_id = emp::JSWrap(CPP_FUN, "", false);                       \
  EM_ASM_ARGS({                                                            \
    emp.__new_object = js.objects[$0].FUNC(UTF8ToString($1),          \
                                    function(d, i, j) {                    \
                                      return emp.Callback($2, d, i, j);    \
                                    });                                    \
  }, this->id, ARG1, fun_id);                                              \
  emp::JSDelete(fun_id);

  //Wraps CPP_FUN (a C++ function pointer, std::function object, or lambda) with
  //JSWrap and passes it to the FUNC method of the current d3 object, along with an argument
  #define D3_CALLBACK_METHOD_CPP_FUNCTION_1_ARG(FUNC, CPP_FUN)               \
    uint32_t fun_id = emp::JSWrap(CPP_FUN, "", false);                       \
    EM_ASM_ARGS({                                                            \
      emp.__new_object = js.objects[$0].FUNC(function(d, i, j) {             \
                                            return emp.Callback($1, d, i, j);\
                                            });                              \
    }, this->id, fun_id);                                                    \
    emp::JSDelete(fun_id);

//Store return of one of the above functions in js.objects
void StoreNewObject(int id){
    EM_ASM_ARGS({
        js.objects[$0] = emp.__new_object;

    }, id);
}


#endif
