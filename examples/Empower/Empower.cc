//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::Random

#include <iostream>

#include "../../source/Empower/Empower.h"

int main()
{
  emp::Empower empower;

  auto var = empower.NewVar<int>("test", 258);
  auto var2 = empower.NewVar<int>("test2", 1001);
  auto var3 = empower.NewVar<std::string>("test3", "Hello Var!");
  auto var4 = empower.NewVar<int>("test4", 4);

  int & x = var.Restore<int>();
  std::cout << "x = " << x << std::endl;
  x = 12345;
  std::cout << "var  = " << var.Restore<int>() << std::endl;
  std::cout << "var2 = " << var2.Restore<int>() << std::endl;
  std::cout << "var3 = " << var3.Restore<std::string>() << std::endl;
  std::cout << "var4 = " << var4.Restore<int>() << std::endl;
}

