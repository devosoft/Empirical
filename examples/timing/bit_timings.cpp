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

#define TEST_SIZES 1, 8, 31, 32, 50, 63, 64, 100, 1000, 10000, 1000000, 1048576

// Return the timing of a function in seconds.
template <typename T>
double TimeFunction(T && fun) {
  std::clock_t start_time = std::clock();
  fun();
  std::clock_t total_time = std::clock() - start_time; 
  return total_time / (double) CLOCKS_PER_SEC;
}


template <size_t... SIZES> struct SpeedTester { };

template <size_t SIZE1, size_t... OTHER_SIZES>
struct SpeedTester<SIZE1, OTHER_SIZES...> : public SpeedTester<OTHER_SIZES...>{
  static constexpr size_t cur_size = SIZE1;
  static constexpr size_t bs_count = sizeof...(OTHER_SIZES);
  emp::BitSet<SIZE1> bs;
  emp::BitVector bv;

  template <size_t ID> auto GetBitSet() {
    if constexpr(ID == 0) return bs;
    else return SpeedTester<OTHER_SIZES...>::template GetBitSet<ID-1>();
  }

  template <size_t ID> auto GetBitVector() {
    if constexpr(ID == 0) return bs;
    else return SpeedTester<OTHER_SIZES...>::template GetBitVector<ID-1>();
  }

  SpeedTester() : bv(SIZE1) { }
};

template <>
struct SpeedTester<> {
  static constexpr size_t cur_size = 0;
  static constexpr size_t bs_count = 0;

  auto GetBitSet() { return 0; }
  auto GetBitVector() { return 0; }
};

int main()
{
  emp::Random random;

  const emp::vector<size_t> test_sizes = { TEST_SIZES };

  for (size_t num_bits : test_sizes) {
    std::cout << "Testing with " << num_bits << " bits." << std::endl;
  }

}
