//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Some code testing the speed of operations on BitSet and BitVector.

#include <ctime>         // For std::clock
#include <iomanip>       // For std::setw
#include <map>

#include "emp/bits/BitVector2.hpp"
#include "emp/base/array.hpp"
#include "emp/base/vector.hpp"
#include "emp/bits/BitSet.hpp"
#include "emp/math/Random.hpp"

#define TEST_SIZES 1, 8, 31, 32, 50, 63, 64, 100, 1000, 10000, 100000, 1000000

// How many total bits should we work with?  The below represents 80 meg worth per test.
static constexpr size_t TEST_BITS = 5120000;
static constexpr size_t TEST_COUNT = 1000;


// Return the timing of a function in seconds.
template <typename T>
double TimeFunction(T && fun) {
  std::clock_t start_time = std::clock();
  fun();
  std::clock_t total_time = std::clock() - start_time; 
  return total_time / (double) CLOCKS_PER_SEC;
}

// Return the timing of a function in seconds.
template <typename T>
double MultiTimeFunction(T && fun) {
  std::clock_t start_time = std::clock();
  for (size_t i = 0; i < TEST_COUNT; ++i) fun();
  std::clock_t total_time = std::clock() - start_time; 
  return total_time / (double) CLOCKS_PER_SEC;
}


using size_timings_t = std::map<size_t, double>;          // Map bit sizes to the associated time.
using timings_t = std::map<std::string, size_timings_t>;  // Map names to all timings.

template <size_t... SIZES> struct SpeedTester { };

template <size_t SIZE1, size_t... OTHER_SIZES>
struct SpeedTester<SIZE1, OTHER_SIZES...> : public SpeedTester<OTHER_SIZES...>{
  static constexpr size_t CUR_BITS = SIZE1;
  static constexpr size_t OTHER_COUNT = sizeof...(OTHER_SIZES);

  // How many bits should we treat each object as?  Put a floor of 256 bits.
  static constexpr size_t OBJ_BITS = (CUR_BITS > 256) ? CUR_BITS : 256;

  // How many objects should we use?
  static constexpr size_t OBJ_COUNT = TEST_BITS / OBJ_BITS;

  emp::array< emp::BitSet<SIZE1>, OBJ_COUNT > bs_objs;
  emp::array< emp::BitVector, OBJ_COUNT > bv_objs;

  using base_t = SpeedTester<OTHER_SIZES...>;

  template <size_t SIZE_ID> auto GetBitSet(size_t index) {
    if constexpr(SIZE_ID == 0) return bs_objs[index];
    else return base_t::template GetBitSet<SIZE_ID-1>(index);
  }

  template <size_t SIZE_ID> auto GetBitVector(size_t index) {
    if constexpr(SIZE_ID == 0) return bv_objs[index];
    else return base_t::template GetBitVector<SIZE_ID-1>(index);
  }

  void TestClear(size_timings_t & bs_map, size_timings_t & bv_map) {
    std::cout << "Testing 'clear' for size " << SIZE1 << std::endl;
    bs_map[SIZE1] = MultiTimeFunction( [this](){ for (auto & x : bs_objs) x.Clear(); } );
    bv_map[SIZE1] = MultiTimeFunction( [this](){ for (auto & x : bv_objs) x.Clear(); } );
    base_t::TestClear(bs_map, bv_map);
  }

  void TestSetAll(size_timings_t & bs_map, size_timings_t & bv_map) {
    std::cout << "Testing 'set_all' for size " << SIZE1 << std::endl;
    bs_map[SIZE1] = MultiTimeFunction([this](){ for (auto & x : bs_objs) x.SetAll(); });
    bv_map[SIZE1] = MultiTimeFunction([this](){ for (auto & x : bv_objs) x.SetAll(); });
    base_t::TestSetAll(bs_map, bv_map);
  }

  SpeedTester() {
    for (auto & x : bv_objs) x.resize(SIZE1);
  }
};

template <>
struct SpeedTester<> {
  static constexpr size_t CUR_BITS = 0;
  static constexpr size_t OTHER_COUNT = 0;

  auto GetBitSet() { return 0; }
  auto GetBitVector() { return 0; }

  void TestClear(size_timings_t &, size_timings_t &) { }
  void TestSetAll(size_timings_t &, size_timings_t &) { }

};

void PrintResults(timings_t bs_timings, timings_t bv_timings, const std::string & name) {
  emp::vector<size_t> sizes{ TEST_SIZES };

  std::cout << "=== Timings for '" << name << "' ===\n";

  for (size_t size : sizes) {
  size_t obj_bits = (size > 256) ? size : 256;
  size_t obj_count = TEST_BITS / obj_bits;

    std::cout << std::left
              << "  size: " << std::setw(7) << size
              << "  count: " << std::setw(7) << obj_count              
              << "  BitSet: " << std::setw(8) << bs_timings[name][size]
              << "  BitVector: " << std::setw(8) << bv_timings[name][size]
              << "  Ratio: " << std::setw(8) << (bs_timings[name][size] / bv_timings[name][size])
              << std::endl;
  }

}

int main()
{
  const emp::vector<size_t> test_sizes = { TEST_SIZES };

  SpeedTester<TEST_SIZES> speed_tester;

  timings_t bs_timings;
  timings_t bv_timings;

  emp::Random random;

  // Conduct the tests.
  speed_tester.TestClear(bs_timings["clear"], bv_timings["clear"]);
  speed_tester.TestSetAll(bs_timings["set_all"], bv_timings["set_all"]);

  // Print the results.
  PrintResults(bs_timings, bv_timings, "clear");
  PrintResults(bs_timings, bv_timings, "set_all");
  
}
