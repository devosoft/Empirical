/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020-2021
 *
 *  @file bit_timings.cpp
 *  @brief Some code testing the speed of operations on BitSet and BitVector.
 */

#include <algorithm>     // For std::max
#include <ctime>         // For std::clock
#include <iomanip>       // For std::setw
#include <map>

#include "emp/base/array.hpp"
#include "emp/base/vector.hpp"
#include "emp/bits/BitSet.hpp"
#include "emp/bits/BitVector.hpp"
#include "emp/math/Random.hpp"

#define TEST_SIZES 1, 8, 31, 32, 50, 63, 64, 100, 1000, 10000, 100000, 1000000

// How many total bits should we work with?  The below represents 80 meg worth per test.
static constexpr size_t TEST_BITS = 5120000;
//static constexpr size_t TEST_BITS = 1000000;
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
  double total_time = (double) (std::clock() - start_time);
  return total_time / (double) CLOCKS_PER_SEC;
}


using size_timings_t = std::map<size_t, double>;          // Map bit sizes to the associated time.
using timings_t = std::map<std::string, size_timings_t>;  // Map names to all timings.

template <size_t... SIZES> struct SpeedTester_impl { };

template <size_t SIZE1, size_t... OTHER_SIZES>
struct SpeedTester_impl<SIZE1, OTHER_SIZES...> : public SpeedTester_impl<OTHER_SIZES...> {
  static constexpr size_t CUR_BITS = SIZE1;
  static constexpr bool HAS_OTHERS = sizeof...(OTHER_SIZES) > 0;

  // How many bits should we treat each object as?  Put a floor of 256 bits.
  static constexpr size_t OBJ_BITS = (CUR_BITS > 256) ? CUR_BITS : 256;

  // How many objects should we use?
  static constexpr size_t OBJ_COUNT = TEST_BITS / OBJ_BITS;

  emp::array< emp::BitSet<SIZE1>, OBJ_COUNT > bs_objs;
  emp::array< emp::BitVector, OBJ_COUNT > bv_objs;

  using base_t = SpeedTester_impl<OTHER_SIZES...>;

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
    if constexpr (HAS_OTHERS) base_t::TestClear(bs_map, bv_map);
  }

  void TestSetAll(size_timings_t & bs_map, size_timings_t & bv_map) {
    std::cout << "Testing 'set_all' for size " << SIZE1 << std::endl;
    bs_map[SIZE1] = MultiTimeFunction([this](){ for (auto & x : bs_objs) x.SetAll(); });
    bv_map[SIZE1] = MultiTimeFunction([this](){ for (auto & x : bv_objs) x.SetAll(); });
    if constexpr (HAS_OTHERS) base_t::TestSetAll(bs_map, bv_map);
  }

  void TestToggleRange(size_timings_t & bs_map, size_timings_t & bv_map) {
    std::cout << "Testing 'toggle_range' for size " << SIZE1 << std::endl;
    const size_t end_pos = std::max<size_t>(1, SIZE1 - 1);
    bs_map[SIZE1] = MultiTimeFunction([this](){ for (auto & x : bs_objs) x.Toggle(1, end_pos); });
    bv_map[SIZE1] = MultiTimeFunction([this](){ for (auto & x : bv_objs) x.Toggle(1, end_pos); });
    if constexpr (HAS_OTHERS) base_t::TestToggleRange(bs_map, bv_map);
  }

  void TestRandomize(size_timings_t & bs_map, size_timings_t & bv_map, emp::Random & random) {
    std::cout << "Testing 'randomize' for size " << SIZE1 << std::endl;
    bs_map[SIZE1] = MultiTimeFunction([this, &random](){ for (auto & x : bs_objs) x.Randomize(random); });
    bv_map[SIZE1] = MultiTimeFunction([this, &random](){ for (auto & x : bv_objs) x.Randomize(random); });
    if constexpr (HAS_OTHERS) base_t::TestRandomize(bs_map, bv_map, random);
  }

  void TestRandomize75(size_timings_t & bs_map, size_timings_t & bv_map, emp::Random & random) {
    std::cout << "Testing 'randomize75' for size " << SIZE1 << std::endl;
    bs_map[SIZE1] = MultiTimeFunction([this, &random](){ for (auto & x : bs_objs) x.Randomize(random, 0.75); });
    bv_map[SIZE1] = MultiTimeFunction([this, &random](){ for (auto & x : bv_objs) x.Randomize(random, 0.75); });
    if constexpr (HAS_OTHERS) base_t::TestRandomize75(bs_map, bv_map, random);
  }

  void TestRandomize82(size_timings_t & bs_map, size_timings_t & bv_map, emp::Random & random) {
    std::cout << "Testing 'randomize82' for size " << SIZE1 << std::endl;
    bs_map[SIZE1] = MultiTimeFunction([this, &random](){ for (auto & x : bs_objs) x.Randomize(random, 0.82); });
    bv_map[SIZE1] = MultiTimeFunction([this, &random](){ for (auto & x : bv_objs) x.Randomize(random, 0.82); });
    if constexpr (HAS_OTHERS) base_t::TestRandomize82(bs_map, bv_map, random);
  }

  SpeedTester_impl() {
    for (auto & x : bv_objs) x.resize(SIZE1);
  }
};

struct SpeedTester {
  SpeedTester_impl<TEST_SIZES> impl;

  timings_t bs_timings;
  timings_t bv_timings;
  emp::Random random;

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

  void RunTests() {
    // Conduct the tests.
    impl.TestClear(bs_timings["clear"], bv_timings["clear"]);
    impl.TestSetAll(bs_timings["set_all"], bv_timings["set_all"]);
    impl.TestToggleRange(bs_timings["toggle_range"], bv_timings["toggle_range"]);
    impl.TestRandomize(bs_timings["randomize"], bv_timings["randomize"], random);
    impl.TestRandomize75(bs_timings["randomize75"], bv_timings["randomize75"], random);
    impl.TestRandomize82(bs_timings["randomize82"], bv_timings["randomize82"], random);
  }

  void PrintResults() {
    // Print the results.
    PrintResults(bs_timings, bv_timings, "clear");
    PrintResults(bs_timings, bv_timings, "set_all");
    PrintResults(bs_timings, bv_timings, "toggle_range");
    PrintResults(bs_timings, bv_timings, "randomize");
    PrintResults(bs_timings, bv_timings, "randomize75");
    PrintResults(bs_timings, bv_timings, "randomize82");
  }
};


int main()
{
  const emp::vector<size_t> test_sizes = { TEST_SIZES };

  SpeedTester speed_tester;

  speed_tester.RunTests();
  speed_tester.PrintResults();
}
