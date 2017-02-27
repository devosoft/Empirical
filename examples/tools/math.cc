//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using math functions.

#include <iostream>

#include "../../tools/math.h"

int main()
{
  std::cout.setf( std::ios::fixed, std::ios::floatfield );
  for (int i = 1; i <= 20; i++) {
    std::cout << "Log2(" << i << ") = " << emp::Log2(i)
              << "   Log(10, " << i << ") = " << emp::Log(10, i)
              << "   Pow(" << i << ", 3.0) = " << emp::Pow(i, 3.0)
              << std::endl;
  }

  constexpr double x = emp::Log(10, emp::E);
  std::cout << "x = " << x << std::endl;

  std::cout << "emp::Mod(10, 7) = " << emp::Mod(10, 7) << " (expected 3)" << std::endl;
  std::cout << "emp::Mod(3, 7) = " << emp::Mod(3, 7) << " (expected 3)" << std::endl;
  std::cout << "emp::Mod(-4, 7) = " << emp::Mod(-4, 7) << " (expected 3)" << std::endl;
  std::cout << "emp::Mod(-11, 7) = " << emp::Mod(-11, 7) << " (expected 3)" << std::endl;

  std::cout << "emp::Pow(2,3) = " << emp::Pow(2,3) << " (expected 8)" << std::endl;
  std::cout << "emp::Pow(-2,2) = " << emp::Pow(-2,2) << " (expected 4)" << std::endl;
  std::cout << "emp::Pow(3,4) = " << emp::Pow(3,4) << " (expected 81)" << std::endl;

}
