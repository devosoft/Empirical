//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some example code for using TypeTracker

#include <iostream>
#include <string>

#include "../../tools/TypeTracker.h"
#include "../../tools/vector.h"

void fun_int_int(int x, int y) { std::cout << x+y << std::endl; }
void fun_int_double(int x, double y) { std::cout << y * (double) x << std::endl; }
void fun_string_double(std::string x, int y) { std::cout << x << " : " << y << std::endl; }

int main()
{
  std::cout << "Testing!" << std::endl;

  using tracker_t = emp::TypeTracker<int, std::string, double>;
  tracker_t tt;
  tt.AddFunction<int,int>(fun_int_int);
  tt.AddFunction<int,double>(fun_int_double);
  tt.AddFunction<std::string,int>(fun_string_double);

  emp::vector< tracker_t::base_t *> objs;
  objs.push_back( tt.New<int>(12) );
  objs.push_back( tt.New<int>(100) );
  objs.push_back( tt.New<double>(1.25) );
  objs.push_back( tt.New<std::string>("Ping!") );

  for (auto x : objs) {
    for (auto y : objs) {
      tt.RunFunction(x,y);
    }
  }
}
