//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::Empower

#include <iostream>

#include "../../include/emp/in_progress/Empower/Empower.h"

int main()
{
  emp::Empower empower;

  auto var = empower.NewVar<int>("test", 258);
  auto var2 = empower.NewVar<int>("test2", 1001);
  auto var3 = empower.NewVar<std::string>("test3", "Hello Var!");
  auto var4 = empower.NewVar<int>("test4", 4);

  int & x = var.Restore<int>();
  std::cout << "x = " << x << '\n';
  x = 12345;
  std::cout << "var  = " << var.Restore<int>() << '\n';
  std::cout << "var2 = " << var2.Restore<int>() << '\n';
  std::cout << "var3 = " << var3.Restore<std::string>() << '\n';
  std::cout << "var4 = " << var4.Restore<int>() << '\n';
}

