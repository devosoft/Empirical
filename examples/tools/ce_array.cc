//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::ce_string

#include <iostream>

#include "../../tools/ce_array.h"

constexpr int CETest() {
  emp::ce_array<int, 20> test_array(0);
  int total = 0;

  for (int i = 0; i < 20; i++) {
    test_array[i] = i*i;
  }
  for (int i = 0; i < 20; i++) {
    total += test_array[i];
  }

  return total;
}

int main()
{
  std::cout << "Testing." << std::endl;

  constexpr int result = CETest();
  std::cout << "CETest() = " << result << std::endl;
}
