//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::Random

#include <iostream>
#include <vector>
#include "emp/math/Random14.h"

constexpr double TestConstRandom(int seed) {
  emp14::Random random(seed);
  return random.GetDouble(100.0,200.0);
}

constexpr double TestConstRandom(emp14::Random random) {
  return random.GetDouble(100.0,200.0);
}

int main()
{
  constexpr double x = TestConstRandom(1);
  std::cout << "x=" << x << '\n';
  constexpr double y = TestConstRandom(2);
  std::cout << "y=" << y << '\n';

  constexpr emp14::Random random(3);
  constexpr double z = TestConstRandom(random);
  std::cout << "z=" << z << '\n';
}
