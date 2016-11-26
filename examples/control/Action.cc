//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::Action

#include <iostream>

#include "../../control/Action.h"

int main()
{
  std::function<int(int,int)> sum_fun = [](int x, int y){ return x+y; };

  emp::Action<int,int,int> sum_action(sum_fun, "sum");
  std::cout << "Ping!" << std::endl;
  std::cout << "5+6 = " << sum_action.Call(5,6) << std::endl;
}

