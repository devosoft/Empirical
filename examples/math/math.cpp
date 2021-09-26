/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2017
 *
 *  @file math.cpp
 *  @brief Some examples code for using math functions.
 */

#include <iostream>

#include "emp/math/math.hpp"

int main()
{
  std::cout.setf( std::ios::fixed, std::ios::floatfield );
  for (double i = 1; i <= 20; i += 1.0) {
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
  std::cout << "emp::Mod(3.0, 2.5) = " << emp::Mod(3.0, 2.5) << " (expected 0.5)" << std::endl;
  std::cout << "emp::Mod(-1.1, 2.5) = " << emp::Mod(-1.1, 2.5) << " (expected 1.4)" << std::endl;
  std::cout << "emp::Mod(12.34, 2.5) = " << emp::Mod(12.34, 2.5) << " (expected 2.34)" << std::endl;
  std::cout << "emp::Mod(-12.34, 2.5) = " << emp::Mod(-12.34, 2.5) << " (expected 0.16)" << std::endl;

  std::cout << "emp::Pow(2,3) = " << emp::Pow(2,3) << " (expected 8)" << std::endl;
  std::cout << "emp::Pow(-2,2) = " << emp::Pow(-2,2) << " (expected 4)" << std::endl;
  std::cout << "emp::Pow(3,4) = " << emp::Pow(3,4) << " (expected 81)" << std::endl;
}
