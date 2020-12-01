//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::Action

#include <iostream>

#include "emp/control/Action.hpp"

int main()
{
  std::function<void(int,int,int&)> sum_fun = [](int x, int y, int& result){ result=x+y; };

  emp::Action<void(int,int,int&)> sum_action(sum_fun, "sum");
  std::cout << "Ping!" << std::endl;
  int result;
  sum_action.Call(5,6,result);
  std::cout << "5+6 = " << result << std::endl;

  // Test actions with return values...
  std::function<int(int,int)> mult_fun = [](int x, int y){ return x*y; };
  emp::Action<int(int,int)> mult_action(mult_fun, "mult");
  std::cout << "5*6 = " << mult_action.Call(5,6) << std::endl;
}
