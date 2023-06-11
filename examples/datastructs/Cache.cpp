/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016
 *
 *  @file Cache.cpp
 *  @brief Some examples code for using emp::Cache
 */

#include <iostream>

#include "emp/datastructs/Cache.hpp"

double F(int N) {
  double PI = 3.14159;
  double val = 1.0;
  while (N-- > 0) { val *= PI; if (val > 1000.0) val /= 1000.0; }
  return val;
}

int main()
{
  std::cout << "Testing.  "
            << std::endl;

  emp::Cache<int, double> test_cache;

  for (int i = 0; i < 400; i++) {
    // std::cout << i%100 << ":" << F(i%100+10000000) << " ";
    std::cout << i%100 << ":" << test_cache.Get(i%100+10000000, F) << " ";
    if (i%8 == 7) std::cout << std::endl;
  }

}
