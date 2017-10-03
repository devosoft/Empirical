//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::ce_string

#include <iostream>

#include "constexpr/ce_array.h"

constexpr int CETest() {
  emp::ce_array<int, 20> test_array(0);

  for (size_t i = 0; i < 20; i++) {
    test_array[i] = (int) (i*i);
  }

  int total = 0;
  emp::ce_array<int, 20> test2(test_array);
  for (size_t i = 0; i < 20; i++) {
    total += test2[i];
  }

  if (test_array == test2) total += 100000000;
  if (test_array.Has(100)) total += 100000000;
  if (test_array.Has(101)) total += 100000000;

  return total;
}

constexpr double CETest2() {
  emp::ce_array<double, 10001> test_array(8.8);
  test_array.fill(4.1);
  double total = 0.0;
  for (size_t i = 0; i < test_array.size(); i++) total += test_array[i];
  return total;
}

int main()
{
  std::cout << "Testing." << std::endl;

  constexpr int result = CETest();
  std::cout << "CETest() = " << result << std::endl;

  constexpr double result2 = CETest2();
  std::cout << "CETest2() = " << result2 << std::endl;

  constexpr emp::ce_array<char, 1> x('x');
}
