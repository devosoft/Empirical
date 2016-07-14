#ifndef __UTILS_H__
#define __UTILS_H__

#include <map>
#include <string>

#include "../../emtools/init.h"
#include "../../tools/macros.h"
#include "../../emtools/js_utils.h"

//Let's try some awful macro stuff

//This macro finds a function specified by ARG2 in either the d3, emp, or
//window namespace and feeds it to FUNC, which is called on the current
//selection. ARG1 is the first argument to FUNC. If ARG2 is not found to
//be a function, it will be passed to FUNC as a string.

#define IS_JS_FUNCTION(FN) (typeof FN === "function")

#define CALL_JS_FUNC_1(FUNC, A1) FUNC(A1)
#define CALL_JS_FUNC_2(FUNC, A1, A2) FUNC(A1, A2)
#define CALL_JS_FUNC_3(FUNC, A1, A2, A3) FUNC(A1, A2, A3)
#define CALL_JS_FUNC_4(FUNC, A1, A2, A3, A4) FUNC(A1, A2, A3, A4)
#define CALL_JS_FUNC_5(FUNC, A1, A2, A3, A4, A5) FUNC(A1, A2, A3, A4, A5)
#define CALL_JS_FUNC_6(FUNC, A1, A2, A3, A4, A5, A6)	\
  FUNC(A1, A2, A3, A4, A5, A6)

#define CALL_JS_FUNC_(...) EMP_ASSEMBLE_MACRO(CALL_JS_FUNC_, __VA_ARGS__)

#define DO_IF_FUNCTION_IN_WINDOW(STRING, BODY)				\
  if IS_JS_FUNCTION(window[STRING]){					\
    func_string = window[STRING];					\
    BODY								\
    }

#define DO_IF_FUNCTION_IN_NAMESPACE_1(STRING, BODY, A1)			\
  if IS_JS_FUNCTION(window[A1][STRING]){				\
    func_string = window[A1][STRING];					\
    BODY								\
    }

#define DO_IF_FUNCTION_IN_NAMESPACE_2(STRING, BODY, A1, A2)	        \
  DO_IF_FUNCTION_IN_NAMESPACE_1(STRING,BODY,A1)				\
  else DO_IF_FUNCTION_NAMESPACE_1(STRING,BODY,A2)

#define DO_IF_FUNCTION_IN_NAMESPACE_3(STRING, BODY, A1, A2, A3)         \
  DO_IF_FUNCTION_IN_NAMESPACE_2(STRING, BODY, A1, A2)			\
  else DO_IF_FUNCTION_IN_NAMESPACE_1(STRING, BODY, A3)

#define DO_IF_FUNCTION_IN_NAMESPACE(...) EMP_ASSEMBLE_MACRO_2ARGS(DO_IF_FUNCTION_IN_NAMESPACE_, STRING, BODY, __VA_ARGS__)

#define DO_IF_FUNCTION_IN_NAMESPACE_OR_WINDOW(STRING, BODY, ...)	\
  DO_IF_FUNCTION_IN_NAMESPACE(STRING, BODY, __VA_ARGS__)		\
  DO_IF_FUNCTION_IN_WINDOW(STRING, BODY)


#define D3_CALLBACK_FUNCTION_2_ARGS(FUNC, CALLBACK, ARG1)		\
  EM_ASM_ARGS({								\
      var arg1 = Pointer_stringify($0);					\
      var func_string = Pointer_stringify($1);				\
      DO_IF_FUNCTION_IN_NAMESPACE(func_string,				\
				  CALL_JS_FUNC(FUNC, arg1, func_string), \
				  "d3", "emp")				\
      else {FUNC(arg1, func_string);}					\
    }, ARG1, CALLBACK);
/*
#define D3_CALLBACK_FUNCTION_2_ARGS(FUNC, ARG1, ARG2)			\
  EM_ASM_ARGS({								\
      var arg1 = Pointer_stringify($1);					\
      var in_string = Pointer_stringify($2);				\
      var fn = window["d3"][in_string];					\
      if (typeof fn === "function"){					\
	FUNC(arg1, fn);							\
      } else {								\
	var fn = window["emp"][in_string];				\
	if (typeof fn === "function"){					\
	  FUNC(arg1, fn);						\
	} else {							\
	  var fn = window[in_string];					\
	  if (typeof fn === "function"){				\
	    FUNC(arg1, fn);						\
	  } else {							\
	    FUNC(arg1, in_string);					\
	  }								\
	}								\
      }									\
      }, this->id, ARG1, ARG2);
	    */


#define CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_2_ARGS(FUNC, ARG1, ARG2)	\
  EM_ASM_ARGS({								\
      var arg1 = Pointer_stringify($1);					\
      var in_string = Pointer_stringify($2);				\
      var fn = window["d3"][in_string];					\
      if (typeof fn === "function"){					\
	var new_sel = js.objects[$0].FUNC(arg1, fn);			\
      } else {								\
	var fn = window["emp"][in_string];				\
	if (typeof fn === "function"){					\
	  var new_sel = js.objects[$0].FUNC(arg1, fn);			\
	} else {							\
	  var fn = window[in_string];					\
	  if (typeof fn === "function"){				\
	    var new_sel = js.objects[$0].FUNC(arg1, fn);		\
	  } else {							\
	    var new_sel = js.objects[$0].FUNC(arg1, in_string);		\
	  }								\
	}								\
      }									\
      emp.__new_object = new_sel;					\
      }, this->id, ARG1, ARG2);

//The same as CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_2_ARGS except for FUNCs
//that only accept a single argument (the string that might or might not
//be a function. This can probably be streamlined with the 2_ARGs version
#define CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(FUNC, ARG1)		\
  EM_ASM_ARGS({								\
      var in_string = Pointer_stringify($1);				\
      var fn = window["d3"][in_string];					\
      if (typeof fn === "function"){					\
	var new_sel = js.objects[$0].FUNC(fn);				\
      } else {								\
	var fn = window["emp"][in_string];				\
	if (typeof fn === "function"){					\
	  var new_sel = js.objects[$0].FUNC(fn);			\
	} else {							\
	  var fn = window[in_string];					\
	  if (typeof fn === "function"){				\
	    var new_sel = js.objects[$0].FUNC(fn);			\
	  } else {							\
	    var new_sel = js.objects[$0].FUNC(in_string);		\
	  }								\
	}								\
      }									\
      emp.__new_object = new_sel;					\
    }, this->id, ARG1);


void StoreNewObject(){EM_ASM({js.objects.push(emp.__new_object);});}

/*
template <std::size_t SIZE>
void pass_array_to_cpp(std::array<int32_t, SIZE> & arr) {

  emp_assert(SIZE == EM_ASM_INT_V({return emp.__outgoing_array.length}));

  int buffer = EM_ASM_INT_V({
      var buffer = Module._malloc(emp.__outgoing_array.length*4);

      for (i=0; i<emp.__outgoing_array.length; i++){
	setValue(buffer+(i*4), emp.__outgoing_array[i], "i32");
      }

      return buffer;
    });

  for (int i=0; i<SIZE; i++){
    arr[i] = *(int*) (buffer + i*4);
  }

  free((void*)buffer);
}

template <std::size_t SIZE>
void pass_array_to_cpp(std::array<double, SIZE> & arr) {

  emp_assert(SIZE == EM_ASM_INT_V({return emp.__outgoing_array.length}));

  int buffer = EM_ASM_INT_V({
      var buffer = Module._malloc(emp.__outgoing_array.length*8);

      for (i=0; i<emp.__outgoing_array.length; i++){
	setValue(buffer+(i*8), emp.__outgoing_array[i], "double");
      }

      return buffer;
    });

  for (int i=0; i<SIZE; i++){
    arr[i] = *(double*) (buffer + i*sizeof(double));
  }

  free((void*)buffer);
}
*/





#endif
