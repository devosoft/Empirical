/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2018
 *
 *  @file  js_utils.h
 *  @brief Tools for passing data between C++ and Javascript.
 */


#ifndef EMP_JS_UTILS_H
#define EMP_JS_UTILS_H

#include <map>
#include <string>
#include <typeinfo>

#include "../base/assert.h"
#include "../base/vector.h"
#include "../base/array.h"

#include "init.h"


namespace emp {

  /// This function returns a std::map mapping typeid names to the appropriate
  /// strings to describe those types in Javscript. This is useful when using
  /// getValue() from within EM_ASM macros.
  ///
  ///  For example, say we have a templated function that takes a pointer to type
  /// T. We find out the appropriate string for type T:
  /// std::map<const char*, std::string> type_map = GetTypeToStringMap();
  /// std::string type_string = type_map[typeid(T).name()];
  ///
  /// Now we can pass type_string.c_str() into EM_ASM_ARGS:
  /// `EM_ASM_ARGS({
  ///    var value = getValue($0, $1);
  /// }, pointer, type_string.c_str();`

  std::map<std::string, std::string> get_type_to_string_map() {
    // Using typeid().name() could create problems because it varies by
    // implementation. All that matters is consistency, but obscure could
    // technically be given the same name. So far it seems to not be an issue
    // with Emscripten, which is most critical for this code.
    std::map<std::string, std::string> map_type_names;
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

  /// This function can be called to pass an array, vector, or other container with contiguously
  /// stored data into Javascript.
  /// The array will be stored in emp.__incoming_array. Currently supports
  /// arrays containing all of the types defined in get_type_to_string_map, which
  /// are also all of the types that emscripten supports getting via pointer.
  /// This function also supports nested arrays, and arrays of objects created with
  /// introspective tuple structs.

  /// @cond TEMPLATEs

  // This code now works for all containers, as long as they store data contiguously

  template<typename C, class = typename C::value_type>
  typename std::enable_if<std::is_pod<typename C::value_type>::value, void>::type
  pass_array_to_javascript(C values, emp::vector<int> recursive_el)
  {
    using T = typename C::value_type;
    //Figure out what string to use for the type we've been given
    std::map<std::string, std::string> map_type_names = get_type_to_string_map();
    emp_assert((map_type_names.find(typeid(T).name()) != map_type_names.end()));
    int type_size = sizeof(T);
    (void) type_size;
    std::string type_string = map_type_names[typeid(T).name()];

    // Clear array, if this isn't a recursive call
    if (recursive_el.size() == 0) {
      EM_ASM({emp_i.__incoming_array = [];});
    }

    EM_ASM_ARGS({
    	var curr_array = emp_i.__incoming_array;
    	var depth = 0;

    	// Make sure that we're at the right depth, in case of recursive call.
    	while (curr_array.length > 0) {
    	  var next_index = getValue($4+(depth*4), "i32");
    	  depth += 1;
    	  curr_array = curr_array[next_index];
    	}

    	// Iterate over array, get values, and add them to incoming array.
    	for (i=0; i<$1; i++) {
    	  curr_array.push(getValue($0+(i*$2), Pointer_stringify($3)));
    	}
    }, &values[0], values.size(), type_size, type_string.c_str(), recursive_el.data());
  }

  // Specialization for strings
  template<typename C, class = typename C::value_type>
  typename std::enable_if<std::is_same<typename C::value_type, std::string>::value, void>::type
  pass_array_to_javascript(C values, emp::vector<int> recursive_el)
  {
    // Clear array, if this isn't a recursive call
    if (recursive_el.size() == 0) {
      EM_ASM({emp_i.__incoming_array = [];});
    };

    EM_ASM_ARGS({
      emp_i.__curr_array = emp_i.__incoming_array;
      var depth = 0;

      // Make sure that we are at the right depth, in case of recursive call.
      while (emp_i.__curr_array.length > 0) {
        var next_index = getValue($0+(depth*4), "i32");
        depth += 1;
        emp_i.__curr_array = emp_i.__curr_array[next_index];
      };
    }, recursive_el.data());

    // Iterate over array, get values, and add them to incoming array.
    for (auto val : values) {
      (void) val;
      EM_ASM_ARGS({
        emp_i.__curr_array.push(Pointer_stringify($0));
      }, val.c_str());
    };

    EM_ASM({delete emp_i.__curr_array;});
  }

  // Handle user-defined JSON_TYPE
  template<typename C, class = typename C::value_type>
  typename std::enable_if<C::value_type::n_fields != -1, void>::type
  pass_array_to_javascript(C values, emp::vector<int> recursive_el) {

    std::map<std::string, std::string> map_type_names = get_type_to_string_map();
    // Initialize array in Javascript
    if (recursive_el.size() == 0) {
      EM_ASM({emp_i.__incoming_array = [];});
    }

    // Initialize objects in Javascript
    EM_ASM_ARGS({
    	var curr_array = emp_i.__incoming_array;
    	var depth = 0;

    	// Make sure that we're at the right depth, in case of recursive call.
    	while (curr_array.length > 0) {
    	  var next_index = getValue($1+(depth*4), "i32");
    	  depth += 1;
    	  curr_array = curr_array[next_index];
    	}

    	// Append empty objects
    	for (i=0; i<$0; i++) {
    	  var new_obj = {};
    	  curr_array.push(new_obj);
    	}
    }, values.size(), recursive_el.data());

    for (std::size_t j = 0; j<values.size(); j++) { // Iterate over array
      for (std::size_t i = 0; i<values[j].var_names.size(); i++) { // Iterate over object members

      	// Get variable name and type for this member variable
      	std::string var_name = values[j].var_names[i];
      	std::string type_string = map_type_names[values[j].var_types[i].name()];
      	// Make sure member variable is an allowed type
      	emp_assert((map_type_names.find(values[j].var_types[i].name())
      		    != map_type_names.end()), values[j].var_types[i].name());

      	// Load data into array of objects
      	EM_ASM_ARGS({
    	    var curr_array = emp_i.__incoming_array;
    	    var depth = 0;

    	    // Make sure we are at the right depth, in case of recursive call.
    	    while (curr_array[0].length > 0) {
    	      var next_index = getValue($4+(depth*4), "i32");
    	      depth += 1;
    	      curr_array = curr_array[next_index];
    	    }

    	    if (Pointer_stringify($1) == "string") {
    	      curr_array[$3][Pointer_stringify($2)] = Pointer_stringify($0);
    	    } else {
    	      curr_array[$3][Pointer_stringify($2)] = getValue($0, Pointer_stringify($1));
    	    }
    	  }, values[j].pointers[i], type_string.c_str(), var_name.c_str(),
    	  j, recursive_el.data());
      }
    }
  }

  /// @endcond

  // This version of the function handles non-nested containers
  template<typename C, class = typename C::value_type>
  void pass_array_to_javascript(C values) {
    pass_array_to_javascript(values, emp::vector<int>(0));
  }

  /// @cond TEMPLATES

  // This version of the function handles nested arrays with recursive calls
  // until a non-array type is found.
  template<std::size_t SIZE1, std::size_t SIZE2, typename T>
  void pass_array_to_javascript(emp::array<emp::array<T, SIZE1>, SIZE2> values,
				emp::vector<int> recursive_el = emp::vector<int>()) {

    // Initialize if this is the first call to this function
    if (recursive_el.size() == 0) {
      EM_ASM({emp_i.__incoming_array = [];});
    }

    // Append empty arrays to array that we are currently handling in recursion
    EM_ASM_ARGS({
    	var curr_array = emp_i.__incoming_array;
    	var depth = 0;
    	while (curr_array.length > 0) {
    	  var next_index = getValue($0+(depth*4), "i32");
    	  depth += 1;
    	  curr_array = curr_array[next_index];
    	}
    	for (i=0; i<$1; i++) {
    	  curr_array.push([]);
    	}
    }, recursive_el.data(), values.size());

    // Make recursive calls - recursive_els specifies coordinates of array we're
    // currently operating on
    for (std::size_t i = 0; i<values.size(); i++) {
      emp::vector<int> new_recursive_el (recursive_el);
      new_recursive_el.push_back((int) i);
      pass_array_to_javascript(values[i], new_recursive_el);
    }
  }

  // This version of the function handles nested vectors with recursive calls
  // until a non-array type is found.
  template<typename T>
  void pass_array_to_javascript(emp::vector<emp::vector<T> > values,
				emp::vector<int> recursive_el = emp::vector<int>()) {

    // Initialize if this is the first call to this function
    if (recursive_el.size() == 0) {
      EM_ASM({emp_i.__incoming_array = [];});
    }

    // Append empty arrays to array that we are currently handling in recursion
    EM_ASM_ARGS({
    	var curr_array = emp_i.__incoming_array;
    	var depth = 0;
    	while (curr_array.length > 0) {
    	  var next_index = getValue($0+(depth*4), "i32");
    	  depth += 1;
    	  curr_array = curr_array[next_index];
    	}
    	for (i=0; i<$1; i++) {
    	  curr_array.push([]);
    	}
    }, recursive_el.data(), values.size());

    // Make recursive calls - recursive_els specifies coordinates of array we are
    // currently operating on
    for (std::size_t i = 0; i<values.size(); i++) {
      emp::vector<int> new_recursive_el (recursive_el);
      new_recursive_el.push_back((int) i);
      pass_array_to_javascript(values[i], new_recursive_el);
    }
  }

  /// @endcond

  /// This function lets you pass an array from javascript to C++!
  /// It takes a reference to the array as an argument and populates it
  /// with the contents of emp.__outgoing_array.
  ///
  /// Currently accepts arrays of ints, floats, doubles, chars, and std::strings
  /// The size of the passed array must be equal to the size of the array stored
  /// in emp.__outgoing_array
  //
  // Don't worry about the recurse argument - it's for handling nested arrays
  // internally
  template <std::size_t SIZE, typename T>
  void pass_array_to_cpp(emp::array<T, SIZE> & arr, bool recurse = false) {

    //Figure out type stuff
    std::map<std::string, std::string> map_type_names = get_type_to_string_map();
    emp_assert((map_type_names.find(typeid(T).name()) != map_type_names.end()), typeid(T).name());
    int type_size = sizeof(T);
    (void) type_size;
    std::string type_string = map_type_names[typeid(T).name()];

    //Make sure arrays have the same length
    emp_assert(arr.size() == EM_ASM_INT_V({return emp_i.__outgoing_array.length}),
               arr.size(), EM_ASM_INT_V({return emp_i.__outgoing_array.length}));

    //Write emp.__outgoing_array contents to a buffer
    T * buffer = (T*) EM_ASM_INT({
    	var buffer = Module._malloc(emp_i.__outgoing_array.length*$0);

    	for (i=0; i<emp_i.__outgoing_array.length; i++) {
    	  setValue(buffer+(i*$0), emp_i.__outgoing_array[i], Pointer_stringify($1));
    	}

      return buffer;
    }, type_size, type_string.c_str());

    // Populate array from buffer
    for (std::size_t i=0; i<arr.size(); i++) {
      arr[i] = *(buffer + i);
    }

    // Free the memory we allocated in Javascript
    free(buffer);
  }

  /// Same as pass_array_to_cpp, but lets you store values in a vector instead
  template <typename T>
  void pass_vector_to_cpp(emp::vector<T> & arr, bool recurse = false) {

    // Figure out type stuff
    std::map<std::string, std::string> map_type_names = get_type_to_string_map();
    emp_assert((map_type_names.find(typeid(T).name()) != map_type_names.end()), typeid(T).name());
    int type_size = sizeof(T);
    (void) type_size;
    std::string type_string = map_type_names[typeid(T).name()];

    // Write emp.__outgoing_array contents to a buffer
    T * buffer = (T*) EM_ASM_INT({
    	var buffer = Module._malloc(emp_i.__outgoing_array.length*$0);

    	for (i=0; i<emp_i.__outgoing_array.length; i++) {
    	  setValue(buffer+(i*$0), emp_i.__outgoing_array[i], Pointer_stringify($1));
    	}

    	return buffer;
    }, type_size, type_string.c_str());

    // Populate array from buffer
    for (int i=0; i < EM_ASM_INT_V({return emp_i.__outgoing_array.length}); i++) {
      arr.push_back(*(buffer + i));
    }

    //Free the memory we allocated in Javascript
    free(buffer);
  }

  /// @cond TEMPLATES


  // template <typename T>
  // typename std::enable_if<C::value_type::n_fields != -1, void>::type
  // pass_vector_to_cpp(emp::vector<T> & arr, bool recurse = false) {
  //
  //   // Figure out type stuff
  //   std::map<std::string, std::string> map_type_names = get_type_to_string_map();
  //   emp_assert((map_type_names.find(typeid(T).name()) != map_type_names.end()), typeid(T).name());
  //   int type_size = sizeof(T);
  //   (void) type_size;
  //   std::string type_string = map_type_names[typeid(T).name()];
  //
  //   // Write emp.__outgoing_array contents to a buffer
  //   T * buffer = (T*) EM_ASM_INT({
  //       var buffer = Module._malloc(emp_i.__outgoing_array.length*$0);
  //
  //       for (i=0; i<emp_i.__outgoing_array.length; i++) {
  //         setValue(buffer+(i*$0), emp_i.__outgoing_array[i], Pointer_stringify($1));
  //       }
  //
  //       return buffer;
  //   }, type_size, type_string.c_str());
  //
  //   // Populate array from buffer
  //   for (int i=0; i < EM_ASM_INT_V({return emp_i.__outgoing_array.length}); i++) {
  //     arr.push_back(*(buffer + i));
  //   }
  //
  //   //Free the memory we allocated in Javascript
  //   free(buffer);
  // }

  // Chars aren't one of the types supported by setValue, but by treating them
  // as strings in Javascript we can pass them out to a C++ array
  template <std::size_t SIZE>
  void pass_array_to_cpp(emp::array<char, SIZE> & arr, bool recurse = false) {

    emp_assert(arr.size() == EM_ASM_INT_V({return emp_i.__outgoing_array.length}));

    char * buffer = (char *) EM_ASM_INT_V({
    	// Since we're treating each char as it's own string, each one
    	// will be null-termianted. So we malloc length*2 addresses.
    	var buffer = Module._malloc(emp_i.__outgoing_array.length*2);

    	for (i=0; i<emp_i.__outgoing_array.length; i++) {
    	  writeStringToMemory(emp_i.__outgoing_array[i], buffer+(i*2));
    	}

    	return buffer;
    });

    for (size_t i=0; i<arr.size(); i++) {
      arr[i] = *(buffer + i*2);
    }

    free(buffer);
  }


  // Chars aren't one of the types supported by setValue, but by treating them
  // as strings in Javascript we can pass them out to a C++ array
  void pass_vector_to_cpp(emp::vector<char> & arr, bool recurse = false) {

    char * buffer = (char *) EM_ASM_INT_V({
    	// Since we're treating each char as it's own string, each one
    	// will be null-termianted. So we malloc length*2 addresses.
    	var buffer = Module._malloc(emp_i.__outgoing_array.length*2);

    	for (i=0; i<emp_i.__outgoing_array.length; i++) {
    	  writeStringToMemory(emp_i.__outgoing_array[i], buffer+(i*2));
    	}

    	return buffer;
    });

    for (int i=0; i<EM_ASM_INT_V({return emp_i.__outgoing_array.length}); i++) {
      arr.push_back(*(buffer + i*2));
    }

    free(buffer);
  }

  // We can handle strings in a similar way
  template <std::size_t SIZE>
  void pass_array_to_cpp(emp::array<std::string, SIZE> & arr, bool recurse = false) {

    emp_assert(arr.size() == EM_ASM_INT_V({return emp_i.__outgoing_array.length}));

    char * buffer = (char *) EM_ASM_INT_V({
    	// Figure how much memory to allocate
    	var arr_size = 0;
    	for (i=0; i<emp_i.__outgoing_array.length; i++) {
    	  arr_size += emp_i.__outgoing_array[i].length + 1;
    	}

    	var buffer = Module._malloc(arr_size);

    	// Track place in memory to write too
    	var cumulative_size = 0;
    	for (i=0; i<emp_i.__outgoing_array.length; i++) {
    	  writeStringToMemory(emp_i.__outgoing_array[i], buffer + (cumulative_size));
    	    cumulative_size += emp_i.__outgoing_array[i].length + 1;
    	}

    	return buffer;
    });

    // Track place in memory to read from
    int cumulative_size = 0;
    for (size_t i=0; i<arr.size(); i++) {
      // Since std::string constructor reads to null terminator, this just works.
      arr[i] = std::string(buffer + cumulative_size);
      cumulative_size += arr[i].size() + 1;
    }

    free(buffer);
  }

  // We can handle strings in a similar way
  void pass_vector_to_cpp(emp::vector<std::string> & arr, bool recurse = false) {

    char * buffer = (char *) EM_ASM_INT_V({
    	// Figure how much memory to allocate
    	var arr_size = 0;
    	for (i=0; i<emp_i.__outgoing_array.length; i++) {
    	  arr_size += emp_i.__outgoing_array[i].length + 1;
    	}

    	var buffer = Module._malloc(arr_size);

    	// Track place in memory to write too
    	var cumulative_size = 0;
    	for (i=0; i<emp_i.__outgoing_array.length; i++) {
    	  writeStringToMemory(emp_i.__outgoing_array[i], buffer + (cumulative_size));
    	  cumulative_size += emp_i.__outgoing_array[i].length + 1;
    	}

    	return buffer;
    });

    // Track place in memory to read from
    int cumulative_size = 0;
    for (int i=0; i<EM_ASM_INT_V({return emp_i.__outgoing_array.length}); i++) {
      // Since std::string constructor reads to null terminator, this just works.
      arr.push_back(std::string(buffer + cumulative_size));
      cumulative_size += arr[(size_t)i].size() + 1;
    }

    free(buffer);
  }

  // We can handle nested arrays through recursive calls on chunks of them
  template <std::size_t SIZE, std::size_t SIZE2, typename T>
  void pass_array_to_cpp(emp::array<emp::array<T, SIZE2>, SIZE> & arr, bool recurse = false) {

    emp_assert(arr.size() == EM_ASM_INT_V({return emp_i.__outgoing_array.length}));

    // Create temp array to hold whole array while pieces are passed in
    if (recurse == 0) {
      EM_ASM({emp_i.__temp_array = [emp_i.__outgoing_array];});
    } else {
      // This is a little wasteful of space, but the alternatives are
      // surprisingly ugly
      EM_ASM({emp_i.__temp_array.push(emp_i.__outgoing_array);});
    }

    for (size_t i = 0; i < arr.size(); i++) {
      EM_ASM_ARGS({
	      emp_i.__outgoing_array = emp_i.__temp_array[emp_i.__temp_array.length - 1][$0];
	    }, i);
      pass_array_to_cpp(arr[i], true);
    }

    // Clear temp array
    if (recurse == 0) { EM_ASM({emp_i.__temp_array = [];}); }
    else { EM_ASM({emp_i.__temp_array.pop();}); }
  }

  /// We can handle nested arrays through recursive calls on chunks of them
  template <typename T>
  void pass_vector_to_cpp(emp::vector<emp::vector<T> > & arr, bool recurse = false) {

    // Create temp array to hold whole array while pieces are passed in
    int size = EM_ASM_INT_V({return emp_i.__outgoing_array.length});

    if (recurse == 0) {
      EM_ASM({
        emp_i.__temp_array = [emp_i.__outgoing_array];
        console.log(emp_i.__outgoing_array);
      });
    } else {
      // This is a little wasteful of space, but the alternatives are
      // surprisingly ugly
      EM_ASM({emp_i.__temp_array.push(emp_i.__outgoing_array);});
    }

    for (int i = 0; i < size; i++) {
      EM_ASM_ARGS({
	      emp_i.__outgoing_array = emp_i.__temp_array[emp_i.__temp_array.length - 1][$0];
      }, i);
      while ((int)arr.size() <= i) {
        arr.push_back(emp::vector<T>());
      }
      pass_vector_to_cpp(arr[i], true);
    }

    // Clear temp array
    if (recurse == 0) {
      EM_ASM({emp_i.__temp_array = [];});
    } else {
      EM_ASM({emp_i.__temp_array.pop();});
    }
  }

/// @endcond

}


#endif
