//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Some code testing the speed of random operations.

#include <ctime>         // For std::clock
#include <map>

#include "emp/base/vector.hpp"
#include "emp/bits/BitSet.hpp"
#include "emp/bits/BitVector.hpp"
#include "emp/math/Random.hpp"

// Return the timing of a function in seconds.
template <typename T>
double TimeFunction(T && fun) {
  std::clock_t start_time = std::clock();
  fun();
  std::clock_t total_time = std::clock() - start_time; 
  return total_time / (double) CLOCKS_PER_SEC;
}

int main()
{
  emp::Random random;

  const emp::vector<size_t> test_sizes = { 1, 31, 32, 50, 63, 64, 100, 1000, 10000, 1000000 };

  for (size_t num_bits : test_sizes) {
    std::cout << "Testing with " << num_bits << " bits." << std::endl;
  }

}
