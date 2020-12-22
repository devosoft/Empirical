//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Some code testing the speed of random operations.

#include <ctime>         // For std::clock
#include <iomanip>       // For std::setw
#include <map>

#include "emp/bits/BitVector2.hpp"
#include "emp/base/vector.hpp"
#include "emp/bits/BitSet.hpp"
#include "emp/math/Random.hpp"

#define TEST_SIZES 1, 8, 31, 32, 50, 63, 64, 100, 1000, 10000, 100000, 1000000
#define TEST_COUNT 500000


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
  static constexpr size_t cur_size = SIZE1;
  static constexpr size_t bs_count = sizeof...(OTHER_SIZES);
  emp::BitSet<SIZE1> bs;
  emp::BitVector bv;

  using base_t = SpeedTester<OTHER_SIZES...>;

  template <size_t ID> auto GetBitSet() {
    if constexpr(ID == 0) return bs;
    else return base_t::template GetBitSet<ID-1>();
  }

  template <size_t ID> auto GetBitVector() {
    if constexpr(ID == 0) return bs;
    else return base_t::template GetBitVector<ID-1>();
  }

  void TestClear(size_timings_t & bs_map, size_timings_t & bv_map) {
    std::cout << "Testing clear for size " << SIZE1 << std::endl;
    bs_map[SIZE1] = MultiTimeFunction([this](){ bs.Clear(); });
    bv_map[SIZE1] = MultiTimeFunction([this](){ bv.Clear(); });
    base_t::TestClear(bs_map, bv_map);
  }

  void TestSetAll(size_timings_t & bs_map, size_timings_t & bv_map) {
    std::cout << "Testing set_all for size " << SIZE1 << std::endl;
    bs_map[SIZE1] = MultiTimeFunction([this](){ bs.SetAll(); });
    bv_map[SIZE1] = MultiTimeFunction([this](){ bv.SetAll(); });
    base_t::TestSetAll(bs_map, bv_map);
  }

  SpeedTester() : bv(SIZE1) { }
};

template <>
struct SpeedTester<> {
  static constexpr size_t cur_size = 0;
  static constexpr size_t bs_count = 0;

  auto GetBitSet() { return 0; }
  auto GetBitVector() { return 0; }

  void TestClear(size_timings_t &, size_timings_t &) { }
  void TestSetAll(size_timings_t &, size_timings_t &) { }

};

void PrintResults(timings_t bs_timings, timings_t bv_timings, const std::string & name) {
  emp::vector<size_t> sizes{ TEST_SIZES };

  std::cout << "=== Timings for '" << name << "' ===\n";

  for (size_t size : sizes) {
    std::cout << std::left
              << "  size: " << std::setw(7) << size
              << "  BitSet: " << std::setw(8) << bs_timings[name][size]
              << "  BitVector: " << std::setw(8) << bv_timings[name][size]
              << std::endl;
  }

}

int main()
{
  emp::Random random;

  const emp::vector<size_t> test_sizes = { TEST_SIZES };

  SpeedTester<TEST_SIZES> speed_tester;

  timings_t bs_timings;
  timings_t bv_timings;

  // Conduct the tests.
  speed_tester.TestClear(bs_timings["clear"], bv_timings["clear"]);
  speed_tester.TestSetAll(bs_timings["set_all"], bv_timings["set_all"]);

  // Print the results.
  PrintResults(bs_timings, bv_timings, "clear");
  PrintResults(bs_timings, bv_timings, "set_all");
}
