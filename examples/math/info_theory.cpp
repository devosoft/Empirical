//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using info theory tools.

#include <iostream>

#include "emp/math/info_theory.hpp"

int main()
{
  std::cout << "Testing!" << std::endl;

  emp::vector<double> vals = {0.25, 0.25, 0.5};
  std::cout << emp::Entropy(vals) << std::endl;

  emp::vector<int> vals2 = {20, 40, 20};
  std::cout << emp::Entropy(vals2) << std::endl;
}

