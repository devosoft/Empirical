//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::memo_function

#include <iostream>
#include "../../tools/memo_function.h"

double F(int N) {
  double PI = 3.14159;
  double val = 1.0;
  while (N-- > 0) { val *= PI; if (val > 1000.0) val /= 1000.0; }
  return val;
}

int main()
{
  std::cout << "Testing.  " << std::endl;

  emp::memo_function<double(int)> test_fun(F);

  for (int i = 0; i < 400; i++) {
    std::cout << i%100 << ":" << test_fun(i%100+10000000) << " ";
    if (i%8 == 7) std::cout << std::endl;
  }

}
