//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using const_utils functions.

#include <iostream>

#include "../../tools/const_utils.h"

int main()
{
  for (int i = 1; i <= 20; i++) {
    std::cout << "Log2(" << i << ") = " << emp::constant::Log2(i)
      //              << "   Log2(" << i << ".0) = " << emp::constant::Log2((double) i)
              << std::endl;
  }
}
