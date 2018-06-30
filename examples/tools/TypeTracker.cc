//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Some example code for using TypeTracker

#include <iostream>
#include <string>

#include "base/vector.h"
#include "tools/TypeTracker.h"
#include "tools/debug.h"

// Build some sample functions that we want called by type.
void fun_int_int(int x, int y) { std::cout << x << "," << y << " : " << x+y << std::endl; }
void fun_int_double(int x, double y) { std::cout << x << "," << y << " : " << y * (double) x << std::endl; }
void fun_string_double(std::string x, int y) { std::cout << x << " -> " << y << std::endl; }
void fun_string(std::string x) { std::cout << "The lonely string is '" << x << "'" << std::endl; }
void fun_int(int x) { std::cout << "The lonely int is '" << x << "'" << std::endl; }
void fun_double(double x) { std::cout << "The lonely double is '" << x << "'" << std::endl; }
void fun_5ints(int v, int w, int x, int y, int z) {
  std::cout << "Eval " << v << "+" << w << "+" << x << "+" << y << "+" << z
            << " = " << (v+w+x+y+z) << std::endl;
}

int main()
{
  using tt_t = emp::TypeTracker<int, std::string, double>;
  tt_t tt;     // Build the tracker.
  tt.AddFunction(fun_int_int);                       // Add the functions...
  tt.AddFunction(fun_int_double);
  tt.AddFunction(fun_string_double);
  tt.AddFunction(fun_string);
  tt.AddFunction(fun_double);
  tt.AddFunction(fun_int);
  tt.AddFunction(fun_5ints);

  // Crate a vector of objects of the generic, tracked type that will need to be converted back.
  emp::vector< emp::TrackedVar > objs;
  objs.push_back( tt.Convert<int>(12) );
  objs.push_back( tt.Convert<int>(100) );
  objs.push_back( tt.Convert<double>(1.25) );
  objs.push_back( tt.Convert<std::string>("Ping!") );

  // Now run the appropriate function for any pair of objects.  Undefined ones should be skipped.
  for (auto & x : objs) {
    if (tt.IsType<int>(x)) std::cout << "INT" << std::endl;
    if (tt.IsType<double>(x)) std::cout << "DOUBLE" << std::endl;
    if (tt.IsType<std::string>(x)) std::cout << "STRING" << std::endl;
    tt.RunFunction(x);
    for (auto & y : objs) {
      tt.RunFunction(x,y);
    }
  }

  // Let's convert one back!
  int x = tt.ToType<int>(objs[1]);
  std::cout << "And the second value was " << x << std::endl;

  // Cleanup objects.
  objs.clear();


  // Try another tracker with pointers.
  emp::TypeTracker<emp::Ptr<int>, emp::Ptr<std::string>, emp::Ptr<double>> tt2;
  emp::Ptr<int> int_ptr = emp::NewPtr<int>(12);
  emp::Ptr<std::string> str_ptr = emp::NewPtr<std::string>("allocated string");
  emp::Ptr<double> double_ptr = emp::NewPtr<double>(1.25);

  //objs.push_back( tt2.Convert<emp::Ptr<int>>(int_ptr) );
  objs.push_back( tt2.Convert(int_ptr) );

  objs.push_back( tt2.Convert<emp::Ptr<std::string>>(str_ptr) );
  objs.push_back( tt2.Convert<emp::Ptr<double>>(double_ptr) );

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

  emp::TrackedVar tval1( tt.Convert<int>(3) );
  emp::TrackedVar tval2( tt.Convert<std::string>("FOUR") );
  emp::TrackedVar tval3( tt.Convert<double>(5.5) );
  emp::TrackedVar tval4( tt.Convert<int>(6) );
  emp::TrackedVar tval5( tt.Convert<int>(7) );
  emp::TrackedVar tval6( tt.Convert<int>(8) );
  emp::TrackedVar tval7( tt.Convert<int>(9) );

  std::cout << std::endl;
  EMP_DEBUG_PRINT(tt_t::GetID<int,std::string,double>());
  EMP_DEBUG_PRINT(tt_t::GetTrackedID(tval1, tval2, tval3));
  EMP_DEBUG_PRINT(tt_t::GetComboID<int,std::string,double>());
  EMP_DEBUG_PRINT(tt_t::GetTrackedComboID(tval1, tval2, tval3));

  tt(tval1, tval3);

  tt.RunFunction(tval1, tval4, tval5, tval6, tval7);
  tt(tval1, tval4, tval5, tval6, tval7);

  int_ptr.Delete();
  str_ptr.Delete();
  double_ptr.Delete();
}
