/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2021
*/
/**
 *  @file
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

TEST_CASE("Test Geometric Distribution", "[math]")
{
  emp::Random random(1000);

  // Try a set of p's

  std::vector<double> p_tests = { 0.001, 0.002, 0.005, 0.01, 0.02, 0.05, 0.10, 0.2, 0.5, 0.8, 0.9, 0.95, 0.99, 0.999 };

  for (double p : p_tests) {
    emp::GeometricDistribution geo(p);

    const size_t num_tests = 1000000;
    size_t total = 0;
    size_t min = static_cast<size_t>(-1);
    size_t max = 0;

    for (size_t test_id = 0; test_id < num_tests; ++test_id) {
      size_t value = geo.PickRandom(random);
      total += value;
      min = std::min(min, value);
      max = std::max(max, value);
    }

    const double exp = 1.0 / p;                 // Expected value.
    const double ave = total/(double)num_tests; // Actual average.

    CHECK(min > 0); // Minimum should never be zero.
    CHECK(ave > exp * 0.995);
    CHECK(ave < exp * 1.005);

    // std::cout << "\n---------- GEOMETRIC"
    //           << "\np = " << p
    //           << "\nmin = " << min
    //           << "\nave = " << ave
    //           << "\nmax = " << max
    //           << "\nexp = " << exp
    //           << "\narray size = " << geo.GetSize()
    //           << std::endl;
  }

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

TEST_CASE("Test Negative Binomial", "[math]")
{
  emp::Random random(2000);

  // Try a set of p's and N's.

  std::vector<double> success_prob = { 0.005, 0.10, 0.5, 0.9, 0.99, 0.99 };
  std::vector<size_t> target_hits =  { 1,     5,    100, 10,  1,    1000 };

  REQUIRE(success_prob.size() == target_hits.size());

  for (size_t treatment_id = 0; treatment_id < success_prob.size(); ++treatment_id) {
    const double p = success_prob[treatment_id];
    const size_t N = target_hits[treatment_id];
    emp::NegativeBinomial nb(p, N);

    const size_t num_tests = 1000000;
    size_t total = 0;
    size_t min = static_cast<size_t>(-1);
    size_t max = 0;

    for (size_t test_id = 0; test_id < num_tests; ++test_id) {
      size_t value = nb.PickRandom(random);
      total += value;
      min = std::min(min, value);
      max = std::max(max, value);
    }

    const double exp = N / p;                   // Expected value.
    const double ave = total/(double)num_tests; // Actual average.

    CHECK(min >= N); // Minimum can never be less than the number of hits.
    CHECK(ave > exp * 0.995);
    CHECK(ave < exp * 1.005);

    std::cout << "\n---------- NEGATIVE BINOMIAL"
              << "\np = " << p
              << "\nN = " << N
              << "\nmin = " << min
              << "\nave = " << ave
              << "\nmax = " << max
              << "\nexp = " << exp
              << std::endl;
  }

}