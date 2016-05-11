//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "../../tools/functions.h"

int Function3(int x) { 
  return x + 3;
}

int main()
{
  std::cout << "Ping!" << std::endl;

  auto fun1 = [](int i) { return i + 1; };
  std::function<int(int)> fun2(fun1);

  std::function<int(int)> fun4 = emp::to_function<int(int)>(fun1);
  std::function<int(int)> fun5 = emp::to_function(fun2);
  std::function<int(int)> fun6 = emp::to_function(Function3);

  std::cout << fun4(10) << std::endl;
}
