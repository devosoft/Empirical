//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Some code testing the speed of random operations.

#include <algorithm>     // For std::sort
#include <ctime>         // For std::clock
#include <vector>

#include "emp/math/Random.hpp"

constexpr const size_t N = 1048576; // 10^20

int main()
{
  emp::Random random;


  //
  // Test GetUInt()
  //

  std::clock_t start_time = std::clock();

  constexpr const size_t buckets = 1024;
  constexpr const size_t N_UInt = N * 512;
  constexpr const size_t expected_per_bucket = N_UInt / buckets;

  emp::vector<size_t> hit_counts(buckets+1, 0);

  for (size_t i = 0; i < N_UInt; i++) {
    const size_t id = random.GetUInt(buckets);
    hit_counts[id]++;
  }

  std::clock_t total_time = std::clock() - start_time;

  std::cout << "Testing Random::GetUInt()."
	    << "  Doing " << N_UInt << " tests "
	    << " with " << buckets << " buckets;"
	    << " expecting ~" << expected_per_bucket << " per_bucket.\n";

  for (size_t i = 0; i < 10; i++) {
    std::cout << "hits[" << i << "] = " << hit_counts[i]
	      << "     hits[" << buckets-i << "] = " << hit_counts[buckets-i]
	      << "\n";
  }

  std::cout << " time = " << ((double) total_time) / (double) CLOCKS_PER_SEC
            << " seconds." << std::endl;


  //
  // Test P()
  //

  size_t success_count = 0;
  start_time = std::clock();

  constexpr const size_t N_P = N * 512;
  constexpr const double p = 0.345;
  constexpr const size_t expected = (size_t) (N_P * p);

  for (size_t i = 0; i < N_P; i++) {
    if (random.P(p)) success_count++;
  }

  total_time = std::clock() - start_time;

  std::cout << "\nTesting Random::P()."
	    << "  Doing " << N_P << " tests;"
	    << " expecting ~" << expected << " hits.\n"
	    << "  num hits = " << success_count
            << ";  time = " << ((double) total_time) / (double) CLOCKS_PER_SEC
            << " seconds." << std::endl;


  //
  // Test RandFill()
  //

  constexpr const size_t layers = 2048;
  unsigned char memory[N];

  start_time = std::clock();

  for (size_t i = 0; i < layers; i++) {
    random.RandFill(memory, N);
  }

  total_time = std::clock() - start_time;

  std::cout << "\nTesting Random::RandFill()."
	    << "  Doing " << N_P << " tests;"
	    << " expecting ~" << expected << " hits.\n"
	    << "  num hits = " << success_count
            << ";  time = " << ((double) total_time) / (double) CLOCKS_PER_SEC
            << " seconds." << std::endl;


}
