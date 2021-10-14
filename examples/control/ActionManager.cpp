/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016
 *
 *  @file ActionManager.cpp
 *  @brief Some examples code for using emp::ActionManager
 */

#include <iostream>

#include "emp/control/ActionManager.hpp"

int main()
{
  emp::ActionManager action_m;
  std::function<void(int,int,int&)> sum_fun = [](int x, int y, int& result){ result=x+y; };

  emp::Action<void(int,int,int&)> sum_action(sum_fun, "sum");
  std::cout << "Ping!" << std::endl;
  int result;
  sum_action.Call(5,6,result);
  std::cout << "5+6 = " << result << std::endl;
}
