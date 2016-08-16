//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using const_utils functions.

#include <iostream>

#include "../../tools/const.h"
#include "../../tools/const_utils.h"

int main()
{
  std::cout.setf( std::ios::fixed, std::ios::floatfield );
  for (int i = 1; i <= 20; i++) {
    std::cout << "Log2(" << i << ") = " << emp::constant::Log2(i)
              << "   Log(10, " << i << ") = " << emp::constant::Log(10, i)
              << "   Pow(" << i << ", 3.0) = " << emp::constant::Pow(i, 3.0)
              << std::endl;
  }

  constexpr double x = emp::constant::Log(10, emp::E);
  std::cout << "x = " << x << std::endl;
}
