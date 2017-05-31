//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Some example code for using TypeTracker

#include <iostream>
#include <string>

#include "../../base/vector.h"
#include "../../tools/TypeTracker.h"
#include "../../tools/debug.h"

// Build some sample functions that we want called by type.
void fun_int_int(int x, int y) { std::cout << x << "," << y << " : " << x+y << std::endl; }
void fun_int_double(int x, double y) { std::cout << x << "," << y << " : " << y * (double) x << std::endl; }
void fun_string_double(std::string x, int y) { std::cout << x << " -> " << y << std::endl; }

int main()
{
  using tt_t = emp::TypeTracker<int, std::string, double>;
  tt_t tt;     // Build the tracker.
  tt.AddFunction(fun_int_int);                       // Add the functions...
  tt.AddFunction(fun_int_double);
  tt.AddFunction(fun_string_double);

  // Crate a vector of objects of the generic, tracked type that will need to be converted back.
  emp::vector<emp::TrackedType *> objs;
  objs.push_back( tt.New<int>(12) );
  objs.push_back( tt.New<int>(100) );
  objs.push_back( tt.New<double>(1.25) );
  objs.push_back( tt.New<std::string>("Ping!") );

  // Now run the appropriate function for any pair of objects.  Undefined ones should be skipped.
  for (auto x : objs) {
    if (tt.IsType<int>(*x)) std::cout << "INT" << std::endl;
    if (tt.IsType<double>(*x)) std::cout << "DOUBLE" << std::endl;
    if (tt.IsType<std::string>(*x)) std::cout << "STRING" << std::endl;
    for (auto y : objs) {
      tt.RunFunction(x,y);
    }
  }

  // Let's convert one back!
  int x = tt.ToType<int>(*(objs[1]));
  std::cout << "And the second value was " << x << std::endl;

  // Cleanup objects.
  for (auto x : objs) delete x;
  objs.resize(0);


  // Try another tracker with pointers.
  emp::TypeTracker<int*, std::string*, double*> tt2;
  int * int_ptr = new int(12);
  std::string * str_ptr = new std::string("allocated string");
  double * double_ptr = new double(1.25);
  objs.push_back( tt2.New<int*>(int_ptr) );
  objs.push_back( tt2.New<std::string*>(str_ptr) );
  objs.push_back( tt2.New<double*>(double_ptr) );


  // Explore IDs and ComboIDs.
  EMP_DEBUG_PRINT(tt_t::GetID<int>());
  EMP_DEBUG_PRINT(tt_t::GetID<double>());
  EMP_DEBUG_PRINT(tt_t::GetID<int,int>());
  EMP_DEBUG_PRINT(tt_t::GetID<double,int>());
  EMP_DEBUG_PRINT(tt_t::GetID<std::string,std::string>());
  EMP_DEBUG_PRINT(tt_t::GetID<int,double>());
  EMP_DEBUG_PRINT(tt_t::GetID<double,double>());

  std::cout << std::endl;
  EMP_DEBUG_PRINT(tt_t::GetComboID<int>());
  EMP_DEBUG_PRINT(tt_t::GetComboID<double>());
  EMP_DEBUG_PRINT(tt_t::GetComboID<int,int>());
  EMP_DEBUG_PRINT(tt_t::GetComboID<double,int>());
  EMP_DEBUG_PRINT(tt_t::GetComboID<std::string,std::string>());
  EMP_DEBUG_PRINT(tt_t::GetComboID<int,double>());
  EMP_DEBUG_PRINT(tt_t::GetComboID<double,double>());
  EMP_DEBUG_PRINT(tt_t::GetComboID<int,double,int,std::string>());
}
