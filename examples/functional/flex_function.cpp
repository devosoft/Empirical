//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::flex_function

#include <iostream>
#include "emp/functional/flex_function.hpp"

int Sum4(int a1, int a2, int a3, int a4) {
  return a1 + a2 + a3 + a4;
}

int main()
{
  std::cout << "Testing.  " << std::endl;

  emp::flex_function<int(int,int,int,int)> ff = Sum4;
  ff.SetDefaults(10, 100,1000,10000);
  std::cout << "ff(1,2,3,4) = " << ff(1,2,3,4) << std::endl;
  std::cout << "ff(1,2,3) = " << ff(1,2,3) << std::endl;
  std::cout << "ff(1,2) = " << ff(1,2) << std::endl;
  std::cout << "ff(1) = " << ff(1) << std::endl;
  std::cout << "ff() = " << ff() << std::endl;
}
