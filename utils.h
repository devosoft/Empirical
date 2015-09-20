#ifndef __UTILS_H__
#define __UTILS_H__

#include <map>
#include <emscripten.h>
#include <string>

//Let's try some awful macro stuff

//This macro finds a function specified by ARG2 in either the d3, emp, or 
//window namespace and feeds it to FUNC, which is called on the current
//selection. ARG1 is the first argument to FUNC. If ARG2 is not found to
//be a function, it will be passed to FUNC as a string.
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

/*template <typename T, std::size_t SIZE>
std::array<T, SIZE> pass_array_to_cpp() {
  

}*/

#endif
