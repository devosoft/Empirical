//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::ce_string

#include <iostream>

#include "../../tools/ce_string.h"

int main()
{
  constexpr auto s = emp::const_string("abc");
  
  std::cout << "Test." << std::endl;
}
