//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some example code for using emp::valsort_map

#include <set>

#include "tools/valsort_map.h"
#include "tools/Random.h"

int main()
{
  constexpr size_t num_vals = 20;
  emp::valsort_map<size_t, double> test_map;
  emp::Random random;

  for (size_t i = 0; i < num_vals; i++) {
    test_map.Set(i, random.GetDouble(-100.0, 100.0));
  }

  std::cout << "INSERT ORDER:" << std::endl;
  for (auto it = test_map.cbegin(); it != test_map.cend(); it++) {
    std::cout << it->first << " : " << it->second << std::endl;
  }

  std::cout << "\nSORTED ORDER:" << std::endl;
  for (auto it = test_map.cvbegin(); it != test_map.cvend(); it++) {
    std::cout << it->first << " : " << it->second << std::endl;
  }
}
