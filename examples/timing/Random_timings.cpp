/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2020
*/
/**
 *  @file
 *  @brief Some code testing the speed of random operations.
 */

#include <algorithm>     // For std::sort
#include <chrono>

#include "emp/base/vector.hpp"
#include "emp/math/Random.hpp"
#include "emp/tools/String.hpp"

struct TimingData {
  struct Entry {
    emp::String rng_name;
    double time;
    double result;
  };

  using treatment_info_t = emp::vector<Entry>;
  std::map<emp::String, treatment_info_t> result_map;

  void Add(emp::String treatment, emp::String in_rng, double time, double result) {
    result_map[treatment].push_back( Entry{in_rng, time / 1'000'000.0, result} );
  }

  void Print(std::ostream & os=std::cout) {
    for (auto [treatment, info] : result_map) {
      os << "TREATMENT: " << treatment << std::endl;
      for (Entry x : info) {
        os << "  TIME: " << x.time
           << "  RESULT: " << x.result
           << "  RNG:" << x.rng_name
           << std::endl;
      }
    }
  }
};

TimingData data;

constexpr const size_t N = 100'000'000;

template <typename RNG_T, typename... ARG_Ts>
double TestGetUInt64(ARG_Ts &&... args) {
  RNG_T random;
  double total = 0.0;
  auto start_time = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < N; ++i) {
    total += random.GetUInt64(args...);
  }
  auto end_time = std::chrono::high_resolution_clock::now();
  auto total_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

  emp::String args_str = emp::MakeArgList(args...);
  emp::String treatment = emp::MakeString("GetUInt64(", args_str, ")");
  data.Add(treatment, random.GetEngineType(), total_time, total);

  return total;
}

template <typename RNG_T, typename... ARG_Ts>
double TestGetDouble(ARG_Ts &&... args) {
  RNG_T random;
  double total = 0.0;
  auto start_time = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < N; ++i) {
    total += random.GetDouble(args...);
  }
  auto end_time = std::chrono::high_resolution_clock::now();
  auto total_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

  emp::String args_str = emp::MakeArgList(args...);
  emp::String treatment = emp::MakeString("GetDouble(", args_str, ")");
  data.Add(treatment, random.GetEngineType(), total_time, total);

  return total;
}

int main()
{
  TestGetUInt64<emp::RandomBest>();
  TestGetUInt64<emp::Random32>();
  TestGetUInt64<emp::RandomFast>();
  
  TestGetDouble<emp::RandomBest>();
  TestGetDouble<emp::Random32>();
  TestGetDouble<emp::RandomFast>();

  TestGetDouble<emp::RandomBest>(100.0);
  TestGetDouble<emp::Random32>(100.0);
  TestGetDouble<emp::RandomFast>(100.0);

  TestGetDouble<emp::RandomBest>(1000000.0, 2000000.5);
  TestGetDouble<emp::Random32>(1000000.0, 2000000.5);
  TestGetDouble<emp::RandomFast>(1000000.0, 2000000.5);

  data.Print();
/*
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
*/

}
