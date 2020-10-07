//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Some code testing the speed of random operations.

#include <algorithm>     // For std::sort
#include <ctime>         // For std::clock
#include <vector>

#include "tools/Random.h"
#include "tools/Random2.h"

constexpr const size_t N = 1000000;

int main()
{
  emp::Random random;
  // emp2::Random random;

  size_t success_count = 0;
  std::clock_t start_time = std::clock();

  constexpr const size_t N_P = N * 500;
  constexpr const double p = 0.345;
  constexpr const size_t expected = N_P * p;

  for (size_t i = 0; i < N_P; i++) {
    if (random.P(p)) success_count++;
  }
  
  std::clock_t total_time = std::clock() - start_time;

  std::cout << "Testing Random::P()."
	    << "  Doing " << N_P << " tests;"
	    << " expecting ~" << expected << " hits.\n"
	    << "  num hits = " << success_count
            << ";  time = " << ((double) total_time) / (double) CLOCKS_PER_SEC
            << " seconds." << std::endl;

}
