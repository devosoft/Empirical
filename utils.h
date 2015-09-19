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


std::map<const char *, std::string> GetTypeToStringMap(){
      //Using typeid().name() could potentially create problems
      //because it varies by implementation.
      //So far it seems to work with emscripten though...
      //And that's really the only compiler anyone would be using here...
      std::map<const char *, std::string> map_type_names;
      map_type_names[typeid(int8_t).name()] = "i8";
      map_type_names[typeid(int16_t).name()] = "i16";
      map_type_names[typeid(int32_t).name()] = "i32";
      map_type_names[typeid(int64_t).name()] = "i64";
      map_type_names[typeid(float).name()] = "float";
      map_type_names[typeid(double).name()] = "double";
      map_type_names[typeid(int8_t*).name()] = "i8*";
      map_type_names[typeid(int16_t*).name()] = "i16*";
      map_type_names[typeid(int32_t*).name()] = "i32*";
      map_type_names[typeid(int64_t*).name()] = "i64*";
      map_type_names[typeid(float*).name()] = "float*";
      map_type_names[typeid(double*).name()] = "double*";
      map_type_names[typeid(void*).name()] = "*";
      map_type_names[typeid(std::string).name()] = "string";

      return map_type_names;
}

template<std::size_t SIZE, typename T>
  void PassArrayToJavascript(std::array<T,SIZE> values, std::vector<int> recursive_el = {}){

  std::map<const char *, std::string> map_type_names = GetTypeToStringMap();
  emp_assert(map_type_names.find(typeid(T).name()) != map_type_names.end());
  
  int type_size = sizeof(T);
  std::string type_string = map_type_names[typeid(T).name()];

  static_assert(!std::is_array<T>::value, "No nested arrays allowed.");

  //clear array
  if (recursive_el.size() == 0){
    EM_ASM({emp.__incoming_array = [];});
  }

  EM_ASM_ARGS({
      var curr_array = emp.__incoming_array;
      var depth = 0;
      while (curr_array.length > 0){
	var next_index = getValue($4+(depth*4), "i32");
	console.log(next_index);
	depth += 1;
	curr_array = curr_array[next_index];
      }
      for (i=0; i<$1; i++){
	curr_array.push(getValue($0+(i*$2), Pointer_stringify($3)));
      }
    }, values, values.size(), type_size, type_string.c_str(), recursive_el.data());
}

template<std::size_t SIZE>
void PassArrayToJavascript(std::array<JSDataObject,SIZE> values){
  std::map<const char *, std::string> map_type_names = GetTypeToStringMap();
  
  //Initialize array objects in javascript
  EM_ASM_ARGS({	    
      emp.__incoming_array = [];
      for (i=0; i<$0; i++){
	var new_obj = {};
	emp.__incoming_array.push(new_obj);
      }
    }, values.size());
  
  for (int j = 0; j<SIZE; j++){ //iterate over array
    for (int i = 0; i<DATA_OBJECT_SIZE; i++){ //iterate over object members
      
      //Get variable name and type for this member variable 
      std::string var_name = values[j].var_names[i];
      std::string type_string =	map_type_names[values[j].var_types[i].name()];
      
      //Make sure member variable is an allowed type
      emp_assert(map_type_names.find(values[j].var_types[i].name())	\
		 != map_type_names.end());
      
      
      //Load data into array of objects
      EM_ASM_ARGS({
	  if (Pointer_stringify($1) == "string"){
	    emp.__incoming_array[$3][Pointer_stringify($2)] = \
	      Pointer_stringify($0);  
	  } else{
	    emp.__incoming_array[$3][Pointer_stringify($2)] =	\
	      getValue($0, Pointer_stringify($1));  
	  }
	}, values[j].pointers[i], type_string.c_str(), var_name.c_str(), j);
    }
  }
}

template<std::size_t SIZE1, std::size_t SIZE2, typename T>
void PassArrayToJavascript(std::array<std::array<T, SIZE1>, SIZE2> values, std::vector<int> recursive_el = std::vector<int>()){

  if (recursive_el.size() == 0){
    std::cout << "initializing array" << std::endl;
    EM_ASM({emp.__incoming_array = [];});
  }

  std::cout << "recursive els: " << recursive_el[0] << " " << recursive_el[1] << std::endl;

  EM_ASM_ARGS({
      var curr_array = emp.__incoming_array;
      var depth = 0;
      while (curr_array.length > 0){
	var next_index = getValue($0+(depth*4), "i32");
	console.log(curr_array, next_index);
	depth += 1;
	curr_array = curr_array[next_index];
      }
      for (i=0; i<$1; i++){
	curr_array.push([]);
      }
    }, recursive_el.data(), SIZE2);

  for (int i = 0; i<SIZE2; i++){
    std::vector<int> new_recursive_el (recursive_el);
    new_recursive_el.push_back(i);
    PassArrayToJavascript(values[i], new_recursive_el);
  }
}

void StoreNewObject(){EM_ASM({js.objects.push(emp.__new_object);});}

#endif
