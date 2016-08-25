//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Some example code for using TypeTracker

#include <iostream>
#include <string>

#include "../../tools/TypeTracker.h"
#include "../../tools/vector.h"

// Build some sample functions that we want called by type.
void fun_int_int(int x, int y) { std::cout << x << "," << y << " : " << x+y << std::endl; }
void fun_int_double(int x, double y) { std::cout << x << "," << y << " : " << y * (double) x << std::endl; }
void fun_string_double(std::string x, int y) { std::cout << x << " -> " << y << std::endl; }

int main()
{
  using tracker_t = emp::TypeTracker<int, std::string, double>;  // Tracker must know all types.
  tracker_t tt;                                                  // Build the tracker.
  tt.AddFunction(fun_int_int);                                   // Add the functions...
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
    for (auto y : objs) {
      tt.RunFunction(x,y);
    }
  }
}
