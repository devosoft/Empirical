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

  auto var = empower.NewVar<int>("test", 5);
  int x = var.Restore<int>();
  std::cout << "x = " << x << std::endl;
}

