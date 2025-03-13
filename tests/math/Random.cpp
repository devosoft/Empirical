/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2021
*/
/**
 *  @file
 */

#include <algorithm>
#include <climits>
#include <deque>
#include <fstream>
#include <limits>
#include <numeric>
#include <ratio>
#include <sstream>
#include <string>
#include <unordered_set>

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/math/Random.hpp"
#include "emp/math/random_utils.hpp"

TEST_CASE("Test Random", "[Random]")
{
  // Get Seed
  emp::Random rnd(1);
  REQUIRE(rnd.GetSeed() == 1);
  rnd.ResetSeed(5);
  REQUIRE(rnd.GetSeed() == 5);

  // Grab 100 random values.
  std::array<double, 100> value_series;
  for (double & x : value_series) x = rnd.GetDouble();

  // Reset the seed -- we should now get the same 100 values.
  rnd.ResetSeed(5);
  std::array<double, 100> value_series2;
  for (double & x : value_series2) x = rnd.GetDouble();

  REQUIRE( value_series == value_series2 );

  // If we build a new random number generator with the same seed it should ALSO give the same values.
  emp::Random rnd2(5);
  std::array<double, 100> value_series3;
  for (double & x : value_series3) x = rnd2.GetDouble();

  REQUIRE( value_series == value_series3 );

  // Get Double
  double r_d = rnd.GetDouble(emp::Range<double>(0.0,5.0));
  REQUIRE(r_d >= 0.0);
  REQUIRE(r_d < 5.0);

  // Get UInt
  size_t r_ui = rnd.GetUInt(emp::Range<size_t>(0,5));
  REQUIRE(r_ui < 5);

  // Get Int
  int r_i = rnd.GetInt(emp::Range<int>(-5,5));
  REQUIRE(r_i >= -5);
  REQUIRE(r_i < 5);

  // Values are consistent when random seeded with 5
  double rndNormal = rnd.GetNormal(5.0, 0.1);
  REQUIRE( std::abs(rndNormal - 5.0) < 0.5 );

  REQUIRE(rnd.GetPoisson(1.0, 0.9) == 1.0);

  size_t b1_result = rnd.GetBinomial(3000, 0.1);
  REQUIRE(b1_result > 250);
  REQUIRE(b1_result < 350);

  size_t b2_result = rnd.GetBinomial(100, 0.3);
  REQUIRE(b2_result > 15);
  REQUIRE(b2_result < 50);

  emp::RandomStdAdaptor randomStd(rnd);
  REQUIRE(randomStd(4) < 4);

  REQUIRE(rnd.GetGeometric(1) == 1);
  // REQUIRE(rnd.GetGeometric(0) == std::numeric_limits<uint32_t>::infinity());
}

TEST_CASE("Another Test random", "[Random]")
{

  std::unordered_map<std::string, std::pair<size_t, size_t>> n_fails;

  // test over a consistent set of seeds
  for(int s = 1; s <= 251; s += 25) {

    REQUIRE(s > 0); // tests should be replicable
    emp::Random rng(s);

    // HERE'S THE MATH
    // Var(Unif) = 1/12 (1 - 0)^2 = 1/12
    // Std(Unif) = sqrt(1/12) = 0.28867513459481287
    // by central limit theorem,
    // Std(mean) =  Std(observation) / sqrt(num observs)
    // Std(mean) = 0.28867513459481287 / sqrt(100000) = 0.0009128709291752767
    // 0.0035 / 0.0009128709291752767 = 4 standard deviations
    // from WolframAlpha, 6.334×10^-5 observations outside 5.4 standard deviations
    // with 500 reps fail rate is 1 - (1 - 1E-8) ^ 500 = 5E-6
    const size_t num_tests = 100000;
    const double error_thresh = 0.0035;
    const double min_value = 2.5;
    const double max_value = 8.7;

    double total = 0.0;

    for (size_t i = 0; i < num_tests; i++) {
      const double cur_value = (
        (rng.GetDouble(min_value, max_value) - min_value)
        / (max_value - min_value)
      );
      total += cur_value;
    }

    {
      const double expected_mean = 0.5;
      const double min_threshold = (expected_mean-error_thresh);
      const double max_threshold = (expected_mean+error_thresh);
      double mean_value = total/(double) num_tests;

      REQUIRE(mean_value > min_threshold);
      REQUIRE(mean_value < max_threshold);
    }

    // Test GetInt
    total = 0.0;
    for (size_t i = 0; i < num_tests; i++) {
      const size_t cur_value = rng.GetInt(min_value, max_value);
      total += cur_value;
    }

    {
      const double expected_mean = static_cast<double>(
        static_cast<int>(min_value) + static_cast<int>(max_value) - 1
      ) / 2.0;
      const double min_threshold = (expected_mean*0.995);
      const double max_threshold = (expected_mean*1.005);
      double mean_value = total/(double) num_tests;

      n_fails["GetInt"].first += !(mean_value > min_threshold);
      n_fails["GetInt"].second += !(mean_value < max_threshold);
    }

    // Test GetUInt()
    emp::vector<uint32_t> uint32_draws;
    total = 0.0;
    for (size_t i = 0; i < num_tests; i++) {
      const uint32_t cur_value = rng.GetUInt();
      total += (
        cur_value / static_cast<double>(std::numeric_limits<uint32_t>::max())
      );
      uint32_draws.push_back(cur_value);
    }

    {
      const double expected_mean = 0.5;
      const double min_threshold = expected_mean-error_thresh;
      const double max_threshold = expected_mean+error_thresh;
      const double mean_value = total / static_cast<double>(num_tests);
      // std::cout << mean_value * 1000 << std::endl;

      n_fails["GetUInt"].first += !(mean_value > min_threshold);
      n_fails["GetUInt"].second += !(mean_value < max_threshold);
      // ensure that all bits are set at least once and unset at least once
      REQUIRE(std::numeric_limits<uint32_t>::max() == std::accumulate(
          std::begin(uint32_draws),
          std::end(uint32_draws),
          static_cast<uint32_t>(0),
          [](uint32_t accumulator, uint32_t val){ return accumulator | val; }
        )
      );
      REQUIRE(std::numeric_limits<uint32_t>::max() == std::accumulate(
          std::begin(uint32_draws),
          std::end(uint32_draws),
          static_cast<uint32_t>(0),
          [](uint32_t accumulator, uint32_t val){ return accumulator | (~val); }
        )
      );
    }

    // Test RandFill()
    uint32_t randfill_draws[num_tests];
    rng.RandFill(
      reinterpret_cast<unsigned char*>(randfill_draws),
      sizeof(randfill_draws)
    );

    total = 0.0;
    for (size_t i = 0; i < num_tests; i++) {
      total += (
        randfill_draws[i]
        / static_cast<double>(std::numeric_limits<uint32_t>::max())
      );
    }

    {
      const double expected_mean = 0.5;
      const double min_threshold = expected_mean-error_thresh;
      const double max_threshold = expected_mean+error_thresh;
      double mean_value = total / static_cast<double>(num_tests);

      n_fails["RandFill"].first += !(mean_value > min_threshold);
      n_fails["RandFill"].second += !(mean_value < max_threshold);
      // ensure that all bits are set at least once and unset at least once
      REQUIRE(std::numeric_limits<uint32_t>::max() == std::accumulate(
          std::begin(randfill_draws),
          std::end(randfill_draws),
          (uint32_t)0,
          [](uint32_t accumulator, uint32_t val){ return accumulator | val; }
        )
      );
      REQUIRE(std::numeric_limits<uint32_t>::max() == std::accumulate(
          std::begin(randfill_draws),
          std::end(randfill_draws),
          static_cast<uint32_t>(0),
          [](uint32_t accumulator, uint32_t val){ return accumulator | (~val); }
        )
      );
    }

    // Test GetUInt64
    emp::vector<uint64_t> uint64_draws;
    total = 0.0;
    double total2 = 0.0;
    for (size_t i = 0; i < num_tests; i++) {
      const uint64_t cur_value = rng.GetUInt64();
      uint64_draws.push_back(cur_value);

      uint32_t temp;
      std::memcpy(&temp, &cur_value, sizeof(temp));
      total += temp / static_cast<double>(std::numeric_limits<uint32_t>::max());
      std::memcpy(
        &temp,
        reinterpret_cast<const unsigned char *>(&cur_value) + sizeof(temp),
        sizeof(temp)
      );
      total2 += temp / static_cast<double>(std::numeric_limits<uint32_t>::max());

    }

    {
      const double expected_mean = 0.5;
      const double min_threshold = expected_mean-error_thresh;
      const double max_threshold = expected_mean+error_thresh;

      const double mean_value = total / static_cast<double>(num_tests);
      n_fails["GetUInt64"].first += !(mean_value > min_threshold);
      n_fails["GetUInt64"].second += !(mean_value < max_threshold);

      const double mean_value2 = total2 / static_cast<double>(num_tests);
      n_fails["GetUInt64"].first += !(mean_value2 > min_threshold);
      n_fails["GetUInt64"].second += !(mean_value2 < max_threshold);

      // ensure that all bits are set at least once and unset at least once
      REQUIRE(std::numeric_limits<uint64_t>::max() == std::accumulate(
          std::begin(uint64_draws),
          std::end(uint64_draws),
          static_cast<uint64_t>(0),
          [](uint64_t accumulator, uint64_t val){ return accumulator | val; }
        )
      );
      REQUIRE(std::numeric_limits<uint64_t>::max() == std::accumulate(
          std::begin(uint64_draws),
          std::end(uint64_draws),
          static_cast<uint64_t>(0),
          [](uint64_t accumulator, uint64_t val){ return accumulator | (~val); }
        )
      );

    }

    // Test P
    double flip_prob = 0.56789;
    int hit_count = 0;
    for (size_t i = 0; i < num_tests; i++) {
      if (rng.P(flip_prob)) hit_count++;
    }

    double actual_prob = ((double) hit_count) / (double) num_tests;

    REQUIRE(actual_prob < flip_prob + 0.01);
    REQUIRE(actual_prob > flip_prob - 0.01);


    // Mimimal test of Choose()
    emp::vector<size_t> choices = Choose(rng,100,10);

    REQUIRE(choices.size() == 10);

  }

  for (const auto & [k, v] : n_fails) {
    // std::cout << k << ": " << v.first << ", " << v.second << std::endl;
    REQUIRE(v.first + v.second == 0);
  }
}

TEST_CASE("Calling ResetSeed should reset all generator internal state", "[Random]") {

  SECTION("Test internal 'value'") {
    // Get Seed
    emp::Random rnd(-1); // Initialize without a seed
    rnd.ResetSeed(5);
    REQUIRE(rnd.GetSeed() == 5);

    emp::vector<int> sequence_a;
    for (size_t i = 0; i < 10; ++i) {
      sequence_a.emplace_back(rnd.GetInt(10000));
    }

    rnd.ResetSeed(5);
    emp::vector<int> sequence_b;
    for (size_t i = 0; i < 10; ++i) {
      sequence_b.emplace_back(rnd.GetInt(10000));
    }

    // Tests internal 'value'
    REQUIRE(sequence_a == sequence_b);
  }

  SECTION("Test internal expV") {
    emp::Random rnd(10);
    rnd.GetNormal(); // Adjusts expV with time-based seed generator

    rnd.ResetSeed(4); // Should reset expV
    emp::vector<double> norm_seq_a;
    for (size_t i = 0; i < 1000; ++i) {
      norm_seq_a.emplace_back(rnd.GetNormal());
    }

    rnd.ResetSeed(4);
    emp::vector<double> norm_seq_b;
    for (size_t i = 0; i < 1000; ++i) {
      norm_seq_b.emplace_back(rnd.GetNormal());
    }

    // Tests internal expV
    REQUIRE(norm_seq_a == norm_seq_b);
  }
}

TEST_CASE("Test 64bit outputs", "[Random]") {
  emp::Random rnd(1);

  const uint64_t value1 = rnd.GetUInt64(100);
  REQUIRE(value1 < 100);

  const uint64_t value2 = rnd.GetUInt64(100'000'000'000);
  REQUIRE(value2 < 100'000'000'000);

  const uint64_t value3 = rnd.GetUInt64(100'000'000'000, 200'000'000'000);
  REQUIRE(value3 >= 100'000'000'000);
  REQUIRE(value3 < 200'000'000'000);

  // Make sure we are consistent with values...
  emp::Random rnd2(1);
  REQUIRE(rnd2.GetUInt64(100) == value1);
  REQUIRE(rnd2.GetUInt64(100'000'000'000) == value2);
  REQUIRE(rnd2.GetUInt64(100'000'000'000, 200'000'000'000) == value3);

  rnd.ResetSeed(1);
  REQUIRE(rnd.GetUInt64(100) == value1);
  REQUIRE(rnd.GetUInt64(100'000'000'000) == value2);
  REQUIRE(rnd.GetUInt64(100'000'000'000, 200'000'000'000) == value3);

  // Make sure the average of 1 million values is right around where it should be.
  uint64_t total = 0;
  uint64_t low_tot = 0;
  uint64_t high_tot = 0;
  size_t low_count = 0;
  size_t high_count = 0;
  for (size_t i = 0; i < 1'000'000; ++i) {
    const auto value = rnd.GetUInt64(100'000'000'000);
    total += value;
    if (value < 10'000'000'000) { low_tot += value;  ++low_count; }
    if (value > 90'000'000'000) { high_tot += value; ++high_count; }
  }
  const uint64_t ave = total / 1'000'000;
  const uint64_t low_ave = low_tot / low_count;
  const uint64_t high_ave = high_tot / high_count;

  // Make sure the overall average is around what we expect.
  REQUIRE(ave > 49'900'000'000);
  REQUIRE(ave < 50'100'000'000);

  // Make sure the low and high counts are where we expect and the average is good within each.
  REQUIRE(low_count > 99'000);
  REQUIRE(low_count < 101'000);
  REQUIRE(high_count > 99'000);
  REQUIRE(high_count < 101'000);

  REQUIRE(low_ave > 4'900'000'000);
  REQUIRE(low_ave < 5'100'000'000);
  REQUIRE(high_ave > 94'900'000'000);
  REQUIRE(high_ave < 95'100'000'000);
}
