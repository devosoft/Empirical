/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file TypeTracker.cc
 *  @brief Unit tests for TypeTracker class.
 */

#include "base/vector.h"
#include "tools/TypeTracker.h"

#include "../unit_tests.h"

std::string tt_result;

// Some functions to print a single type and its value
void fun_int(int x) { tt_result = emp::to_string("int:", x); }
void fun_double(double x) { tt_result = emp::to_string("double:", x); }
void fun_string(std::string x) { tt_result = emp::to_string("string:", x); }

// And some silly ways to combine types.
void fun_int_int(int x, int y) { tt_result = emp::to_string(x+y); }
void fun_int_double(int x, double y) { tt_result = emp::to_string(y * (double) x); }
void fun_string_int(std::string x, int y) {
  tt_result = "";
  for (int i=0; i < y; i++) tt_result += x;
}
void fun_5ints(int v, int w, int x, int y, int z) {
  tt_result = emp::to_string(v, '+', w, '+', x, '+', y, '+', z, '=', v+w+x+y+z);
}

void emp_test_main()
{
  using tt_t = emp::TypeTracker<int, std::string, double>;   // Setup the tracker type.
  tt_t tt;                                                   // Build the tracker.

  // Add some functions.
  tt.AddFunction( [](int x){ tt_result = emp::to_string("int:", x); } );
  tt.AddFunction(fun_double);
  tt.AddFunction(fun_string);
  tt.AddFunction(fun_int_int);
  tt.AddFunction(fun_int_double);
  tt.AddFunction(fun_string_int);
  tt.AddFunction(fun_5ints);

  emp::TrackedVar tt_int1 = tt.Convert<int>(1);
  emp::TrackedVar tt_int2 = tt.Convert<int>(2);
  emp::TrackedVar tt_int3 = tt.Convert<int>(3);

  emp::TrackedVar tt_str  = tt.Convert<std::string>("FOUR");
  emp::TrackedVar tt_doub = tt.Convert<double>(5.5);

  tt.RunFunction(tt_int1, tt_int2);  // An int and another int should add.
  EMP_TEST_VALUE( tt_result, "3" );

  tt.RunFunction(tt_int3, tt_doub);  // An int and a double should multiply.
  EMP_TEST_VALUE( tt_result, "16.500000" );

  tt.RunFunction(tt_doub, tt_int2); // A double and an int is unknown; should leave old result.
  EMP_TEST_VALUE( tt_result, "16.500000" );

  tt.RunFunction(tt_str, tt_int3);    // A string an an int should duplicate the string.
  EMP_TEST_VALUE( tt_result, "FOURFOURFOUR" );

  tt.RunFunction(tt_int1, tt_int2, tt_int3, tt_int2, tt_int1);  // Add five ints!
  EMP_TEST_VALUE( tt_result, "1+2+3+2+1=9" );


  // Load all types into a vector and then experiment with them.
  emp::vector<emp::TrackedVar> vars;
  vars.push_back(tt_int1);
  vars.push_back(tt_int2);
  vars.push_back(tt_int3);
  vars.push_back(tt_str);
  vars.push_back(tt_doub);

  emp::vector<std::string> results = { "int:1", "int:2", "int:3", "string:FOUR", "double:5.5" };

  for (size_t i = 0; i < vars.size(); i++) {
    tt(vars[i]);
    EMP_TEST_VALUE(tt_result, results[i]);
  }

  // Make sure TypeTracker can determine consistant IDs.
  EMP_TEST_VALUE( (tt_t::GetID<int,std::string,double>()), (tt_t::GetTrackedID(tt_int1, tt_str, tt_doub)) );
  EMP_TEST_VALUE( (tt_t::GetComboID<int,std::string,double>()), (tt_t::GetTrackedComboID(tt_int1, tt_str, tt_doub)) );



  // Make sure a TypeTracker can work with a single type.
  size_t num_args = 0;
  emp::TypeTracker<int> tt1;
  tt1.AddFunction( [&num_args](int){ num_args=1; } );
  tt1.AddFunction( [&num_args](int,int){ num_args=2; } );
  tt1.AddFunction( [&num_args](int,int,int){ num_args=3; } );

  tt_int1 = tt1.Convert<int>(1);
  tt_int2 = tt1.Convert<int>(2);
  tt_int3 = tt1.Convert<int>(3);

  tt1.RunFunction(tt_int1);
  EMP_TEST_VALUE(num_args, 1);
  tt1(tt_int2, tt_int3);
  EMP_TEST_VALUE(num_args, 2);
  tt1(tt_int1, tt_int2, tt_int3);
  EMP_TEST_VALUE(num_args, 3);
}
