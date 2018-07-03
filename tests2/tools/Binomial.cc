/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file Binomial.cc
 *  @brief Unit tests for Binomial class.
 */

#include <iostream>

#include "base/vector.h"
#include "tools/Binomial.h"
#include "tools/Random.h"

#include "../unit_tests.h"

void emp_test_main()
{
  emp::Random random;

  const double flip_prob = 0.03;
  const size_t num_flips = 100;

  const size_t num_tests = 100000;
  const size_t view_count = 10;

  emp::Binomial bi100(flip_prob, num_flips);

  emp::vector<size_t> counts(num_flips+1, 0);
  
  for (size_t test_id = 0; test_id < num_tests; test_id++) {
    size_t win_count = 0;
    for (size_t i = 0; i < num_flips; i++) {
      if (random.P(0.03)) win_count++;
    }
    counts[win_count]++;
  }

  // Print out the first values in the distribution.
  for (size_t i = 0; i < view_count; i++) {
    std::cout << "bi100[" << i << "] = " << bi100[i]
	      << "  test_count = " << counts[i]
	      << "\n";
    EMP_TEST_APPROX(bi100[i], ((double) counts[i]) / (double) num_tests, 0.02);
  }
  std::cout << "Total = " << bi100.GetTotalProb() << std::endl;

  // Pick some random values...
  std::cout << "\nSome random values:";
  for (size_t i = 0; i < 100; i++) {
    std::cout << " " << bi100.PickRandom(random);
  }
  std::cout << std::endl;
}
