/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file Distribution.cpp
 */

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/math/Distribution.hpp"
#include "emp/math/Random.hpp"

/// Ensures that
/// 1) A == B
/// 2) A and B can be constexprs or non-contexprs.
/// 3) A and B have the same values regardless of constexpr-ness.
#define CONSTEXPR_REQUIRE_EQ(A, B)       \
  {                                      \
    static_assert(A == B, #A " == " #B); \
    REQUIRE(A == B);                     \
  }


TEST_CASE("Test Binomial", "[math]")
{
  // test over a consistent set of seeds
  for (int s = 1; s <= 200; ++s) {

  REQUIRE(s > 0);
  emp::Random random(s);

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
    // std::cout << "bi100[" << i << "] = " << bi100[i]
    //           << "  test_count = " << counts[i]
    //           << "\n";
    REQUIRE(bi100[i] < ((double) counts[i]) / (double) num_tests + 0.02);
    REQUIRE(bi100[i] > ((double) counts[i]) / (double) num_tests - 0.02);
  }
  // std::cout << "Total = " << bi100.GetTotalProb() << std::endl;

  // // Pick some random values...
  // std::cout << "\nSome random values:";
  // for (size_t i = 0; i < 100; i++) {
  //   std::cout << " " << bi100.PickRandom(random);
  // }
  // std::cout << std::endl;
  }
}
