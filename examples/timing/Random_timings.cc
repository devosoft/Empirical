//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Some code testing the speed of random operations.

#include <algorithm>     // For std::sort
#include <ctime>         // For std::clock
#include <vector>

#include "tools/Random.h"

constexpr const size_t N = 1000000;

int main()
{
  emp::Random random;

  size_t success_count = 0;
  std::clock_t start_time = std::clock();

  constexpr const size_t N_P = N * 500;
  for (size_t i = 0; i < N_P; i++) {
    if (random.P(0.345)) success_count++;
  }
  
  std::clock_t total_time = std::clock() - start_time;

  std::cout << "num hits = " << success_count
            << ";  time = " << ((double) total_time) / (double) CLOCKS_PER_SEC
            << " seconds." << std::endl;

}
