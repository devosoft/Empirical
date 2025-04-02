/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2025
*/
/**
 *  @file
 *  @brief Some examples code for using emp::Random
 */

#include <ctime>         // For std::clock
#include <iostream>

#include "emp/base/vector.hpp"
#include "emp/math/Random.hpp"
#include "emp/math/Random_MSWS.hpp"
#include "emp/math/random_utils.hpp"

constexpr size_t TEST_COUNT = 1'000'000'000;

template <typename PROB_T, typename RAND_T>
void Test(emp::String name, RAND_T & random, double p) {
  size_t hit = 0;
  std::clock_t start_time = std::clock();
  PROB_T prob(p);
  for (size_t i = 0; i < TEST_COUNT; ++i) {
    if (prob.Test(random)) ++hit;
  }
  std::clock_t prob_time = std::clock() - start_time;
  std::cout << "  " << name << ":  "
            << "Hits = " << hit
            << "; Frac = " << hit / (double) TEST_COUNT
            << "; Time = " << 1000.0 * prob_time / CLOCKS_PER_SEC << " msec"
            << std::endl;
}

int main()
{
  //emp::vector<double> pv{0.00005, 0.0005, 0.005, 0.01, 0.025, 0.05, 0.1, 0.25, 0.5, 0.75, 0.9, 0.95, 0.99 };
  emp::vector<double> pv{0.001, 0.005, 0.0075, 0.01, 0.02 }; // Commonly used values for mutation tests.
  // emp::vector<double> pv{0.1, 0.11, 0.12, 0.13, 0.14, 0.15, 0.2 };

  emp::RandomFast random_fast;
  emp::RandomBest random_best;
  emp::Random32 random_32;

  for (double p : pv) {
    std::cout << ">>>> p=" << p << ":\n";

    Test<emp::RandProbability> ("FAST rand ", random_fast, p);
    Test<emp::RandProbability> ("BEST rand ", random_best, p);
    Test<emp::RandProbability> ("R32  rand ", random_32, p);
    Test<emp::LowProbability>  ("FAST lowP ", random_fast, p);
    Test<emp::LowProbability>  ("BEST lowP ", random_best, p);
    Test<emp::LowProbability>  ("R32  lowP ", random_32, p);

    // Test<emp::RandProbability>        ("RandProb2 ", random2, p);
    // Test<emp::BufferedLowProbability> ("BLowProb  ", random, p);
    // Test<emp::BufferedLowProbability> ("BLowProb2 ", random2, p);
    // Test<emp::BufferedProbability>    ("BufProb   ", random, p);
    // Test<emp::BufferedProbability>    ("BufProb2  ", random2, p);
    // Test<emp::BufferedHighProbability>("BHighProb ", random, p);
    // Test<emp::BufferedHighProbability>("BHighProb2", random2, p);
    // Test<emp::Probability>            ("UniProb   ", random, p);
    // Test<emp::Probability>            ("UniProb2  ", random2, p);

    // Test<emp::RandProbability>       ("RandProb  ", random, p);
    // std::cout << std::endl;
    // Test<emp::DistProbability>       ("DistProb  ", random, p);
    // std::cout << std::endl;
    // Test<emp::HighProbability>       ("HighProb  ", random, p);
    std::cout << std::endl;
  }

}
