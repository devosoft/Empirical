//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some example code for using IntPack

#include <iostream>
#include <string>
#include <vector>

#include "../../meta/IntPack.h"

#define EMP_PRINT_VAL(X) std::cout << #X << " = " << X << std::endl;

int main()
{
  using rise_t = emp::IntPack<0,1,2,3,4,5,6,7,8,9,10>;
  using fib_t = emp::IntPack<0,1,1,2,3,5,8,13,21,34>;
  std::cout << "using rise_t = emp::IntPack<1,2,3,4,5,6,7,8,9,10>" << std::endl;
  std::cout << "using fib_t = emp::IntPack<0,1,1,2,3,5,8,13,21,34>" << std::endl;

  EMP_PRINT_VAL(rise_t::GetSize());
  EMP_PRINT_VAL(fib_t::GetSize());
}
