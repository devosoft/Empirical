#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"


#include "emp/base/vector.hpp"
#include "emp/data/DataNode.hpp"
#include "emp/math/Random.hpp"
#include "emp/math/random_utils.hpp"

#include <cmath>
#include <sstream>
#include <fstream>
#include <string>
#include <deque>
#include <algorithm>
#include <limits>
#include <numeric>
#include <climits>
#include <unordered_set>
#include <ratio>
#include <tuple>

#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>

TEST_CASE("Test Random", "[math]")
{
	// Get Seed
	emp::Random rnd(1);
	REQUIRE(rnd.GetSeed() == 1);
	rnd.ResetSeed(5);
	REQUIRE(rnd.GetSeed() == 5);

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

	// Get UInt64
	uint64_t ui64 = rnd.GetUInt64(100);
	REQUIRE(ui64 < 100);

	ui64 = rnd.GetUInt64(100000000000);
	REQUIRE(ui64 < 100000000000);

	// Values are consistent when random seeded with 5
	double rndNormal = rnd.GetRandNormal(5.0, 0.1);
	REQUIRE( abs(rndNormal - 5.0) < 0.5 );

	REQUIRE(rnd.GetRandPoisson(1.0, 0.9) == 1.0);

  size_t b1_result = rnd.GetRandBinomial(3000, 0.1);
	REQUIRE(b1_result > 250);
	REQUIRE(b1_result < 350);

  size_t b2_result = rnd.GetRandBinomial(100, 0.3);
	REQUIRE(b2_result > 15);
	REQUIRE(b2_result < 50);

	emp::RandomStdAdaptor randomStd(rnd);
	REQUIRE(randomStd(4) == 3);

  REQUIRE(rnd.GetRandGeometric(1) == 1);
  REQUIRE(rnd.GetRandGeometric(0) == std::numeric_limits<uint32_t>::infinity());
  // REQUIRE(rnd.GetRandGeometric(.25) == 8);
}

TEST_CASE("Another Test random", "[math]")
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

TEST_CASE("GetRandPareto", "[math]") {

  emp::Random rand(1);

  // check all sampled values are within distribution support
  for (size_t i{1}; i < std::kilo::num; ++i) {
    REQUIRE( rand.GetRandPareto(i) > 0 );
    REQUIRE( rand.GetRandPareto(1.0, i) >= i );
    REQUIRE( rand.GetRandPareto(i+0.5, i) >= i );
    REQUIRE( rand.GetRandPareto(1.0, 0.1, i) <= i );
    REQUIRE( rand.GetRandPareto(i+1.0, 0.1, i) <= i );
  }

  for (double alpha : emp::vector<double>{0.5, 1.0, 1.5, 5.0}) {
    for (auto [lbound, ubound] : emp::vector<std::tuple<double, double>>{
      {0.1, std::numeric_limits<double>::infinity()},
      {0.1, 10.0},
      {1.0, std::numeric_limits<double>::infinity()},
      {1.0, 10.0},
      {4.0, 20.0}
    }) {
      emp::DataNode<double, emp::data::Stats, emp::data::Log> samples;
      for (size_t i{}; i < 10 * std::kilo::num; ++i) samples.Add(
        rand.GetRandPareto(alpha, lbound, ubound)
      );

      // https://en.wikipedia.org/wiki/Pareto_distribution#:~:text=Bounded%20Pareto%20distribution%5Bedit%5D
      const double expected_mean = (alpha == 1.0)
      ? (
        ubound * lbound
        / (ubound - lbound)
      ) * std::log(
        ubound / lbound
      )
      : (
        std::pow(lbound, alpha)
        / (1.0 - std::pow(lbound/ubound, alpha))
      ) * (
        alpha / (alpha - 1.0)
      ) * (
        1.0 / std::pow(lbound, alpha - 1.0)
        - 1.0 / std::pow(ubound, alpha - 1.0)
      );

      const double actual_mean = samples.GetMean();

      // expected value is unbounded for alpha < 1 without upper bound
      if (alpha > 1.0 || std::isfinite(ubound)) REQUIRE( actual_mean == Approx(expected_mean).epsilon(0.10) );

      const double expected_median = lbound * std::pow(
        1.0 - 0.5 * (
          1.0 - std::pow(lbound / ubound, alpha)
        ),
        -1.0/alpha
      );
      const double actual_median = samples.GetMedian();

       REQUIRE( actual_median == Approx(expected_median).epsilon(0.10) );

    }
  }

}

TEST_CASE("GetRandLomax", "[math]") {

  emp::Random rand(1);

  // check all sampled values are within distribution support
  for (size_t i{1}; i < std::kilo::num; ++i) {
    REQUIRE( rand.GetRandLomax(i) >= 0.0 );
    REQUIRE( rand.GetRandLomax(1.0, i) >= 0.0 );
    REQUIRE( rand.GetRandLomax(i+0.5, i) >= .0 );
    REQUIRE( rand.GetRandLomax(1.0, 0.1, i) <= i );
    REQUIRE( rand.GetRandLomax(i+1.0, 0.1, i) <= i );
  }

  for (double alpha : emp::vector<double>{0.5, 1.0, 1.5, 5.0}) {
    for (auto [lambda, ubound] : emp::vector<std::tuple<double, double>>{
      {0.1, std::numeric_limits<double>::infinity()},
      {0.1, 10.0},
      {1.0, std::numeric_limits<double>::infinity()},
      {1.0, 10.0},
      {4.0, 20.0}
    }) {
      emp::DataNode<double, emp::data::Stats, emp::data::Log> samples;
      for (size_t i{}; i < 10 * std::kilo::num; ++i) samples.Add(
        rand.GetRandLomax(alpha, lambda, ubound)
      );

      const double expected_mean = (alpha == 1.0)
      ? (
        (ubound+lambda) * lambda
        / ubound
      ) * std::log(
        (ubound+lambda) / lambda
      ) - lambda
      : (
        std::pow(lambda, alpha)
        / (1.0 - std::pow(lambda/(ubound+lambda), alpha))
      ) * (
        alpha / (alpha - 1.0)
      ) * (
        1.0 / std::pow(lambda, alpha - 1.0)
        - 1.0 / std::pow(ubound+lambda, alpha - 1.0)
      ) - lambda;

      const double actual_mean = samples.GetMean();

      // expected value is unbounded for alpha < 1 without upper bound
      if (alpha > 1.0 || std::isfinite(ubound)) {
        REQUIRE( actual_mean == Approx(expected_mean).epsilon(0.10) );
      }

      const double expected_median = lambda * std::pow(
        1.0 - 0.5 * (
          1.0 - std::pow(lambda / (ubound+lambda), alpha)
        ),
        -1.0/alpha
      ) - lambda;
      const double actual_median = samples.GetMedian();

      REQUIRE( actual_median == Approx(expected_median).epsilon(0.10) );

    }
  }

}

TEST_CASE("GetRandZeroSymmetricPareto output range", "[math]") {

  emp::Random rand(1);

  // check all sampled values are within distribution support
  for (int i{1}; i < std::kilo::num; ++i) {
    REQUIRE( !std::isnan(rand.GetRandZeroSymmetricPareto(i)) );
    REQUIRE( !std::isnan(rand.GetRandZeroSymmetricPareto(i, i)) );
    REQUIRE( !std::isnan(rand.GetRandZeroSymmetricPareto(i+0.5, i-0.5)) );

    REQUIRE( rand.GetRandZeroSymmetricPareto(1.0, 0.1, 0.0, i) <= i );
    REQUIRE( rand.GetRandZeroSymmetricPareto(1.0, 0.1, 0.0, i) >= 0.0 );
    REQUIRE( rand.GetRandZeroSymmetricPareto(1.0, 0.1, -i, 0.0) <= 0.0 );
    REQUIRE( rand.GetRandZeroSymmetricPareto(1.0, 0.1, -i, 0.0) >= -i );

    REQUIRE( rand.GetRandZeroSymmetricPareto(1.0, 0.1, -i, i) <= i );
    REQUIRE( rand.GetRandZeroSymmetricPareto(1.0, 0.1, -i, i) >= -i );

    REQUIRE( rand.GetRandZeroSymmetricPareto(i+1.0, i+0.1, 0.0, i) <= i );
    REQUIRE( rand.GetRandZeroSymmetricPareto(i+1.0, i+0.1, 0.0, i) >= 0.0 );
    REQUIRE( rand.GetRandZeroSymmetricPareto(i+1.0, i+0.1, -i, 0.0) <= 0.0 );
    REQUIRE( rand.GetRandZeroSymmetricPareto(i+1.0, i+0.1, -i, 0.0) >= -i );

    REQUIRE( rand.GetRandZeroSymmetricPareto(i+1.0, i+0.1, -i, i) <= i );
    REQUIRE( rand.GetRandZeroSymmetricPareto(i+1.0, i+0.1, -i, i) >= -i );

  }

}

TEST_CASE("GetRandZeroSymmetricPareto fat/skinny tails", "[math]") {

  emp::Random rand(1);

  for (double alpha : emp::vector<double>{0.5, 1.0, 1.5, 2.0}) {
  for (double lambda : emp::vector<double>{0.5, 1.0, 1.5, 2.0}) {
    for (auto [innerb, outerb] : emp::vector<std::tuple<double, double>>{
      {0.1, std::numeric_limits<double>::infinity()},
      {0.1, 10.0},
      {1.0, std::numeric_limits<double>::infinity()},
      {1.0, 10.0},
      {4.0, 20.0}
    }) {
      emp::DataNode<double, emp::data::Stats, emp::data::Log> fwd_samples;
      emp::DataNode<double, emp::data::Stats, emp::data::Log> bwd_samples;
      for (size_t i{}; i < 10*std::kilo::num; ++i) {
        fwd_samples.Add(
          rand.GetRandZeroSymmetricPareto(alpha, lambda, -innerb, outerb)
        );
        bwd_samples.Add(
          rand.GetRandZeroSymmetricPareto(alpha, lambda, -outerb, innerb)
        );
      }

      REQUIRE(fwd_samples.GetMean() > 0.0);
      // for extreme parameterizations, medians might reasonably
      // be close to zero but of opposite expected sign
      REQUIRE(fwd_samples.GetMedian() > -0.01);
      REQUIRE(fwd_samples.GetMedian() < fwd_samples.GetMean());

      REQUIRE(bwd_samples.GetMean() < 0.0);
      REQUIRE(bwd_samples.GetMedian() < 0.01);
      REQUIRE(bwd_samples.GetMedian() > bwd_samples.GetMean());

      // can't do this test with means because of extreme variance
      // due to extreme effect outliers
      REQUIRE(
        fwd_samples.GetMedian()
        == Approx(-bwd_samples.GetMedian()).epsilon(0.1).margin(0.1)
      );
      REQUIRE(
        fwd_samples.GetPercentile(20)
        == Approx(-bwd_samples.GetPercentile(80)).epsilon(0.2).margin(0.1)
      );
      REQUIRE(
        fwd_samples.GetPercentile(80)
        == Approx(-bwd_samples.GetPercentile(20)).epsilon(0.2).margin(0.1)
      );

    }

  }
  }

}

TEST_CASE("GetRandZeroSymmetricPareto even tails", "[math]") {

  emp::Random rand(1);

  for (double alpha : emp::vector<double>{0.5, 1.0, 1.5, 2.0}) {
  for (double lambda : emp::vector<double>{0.5, 1.0, 1.5, 2.0}) {
    for (auto bound : emp::vector<double>{
      0.1, 1.0, 4.0, 10.0, 20.0, std::numeric_limits<double>::infinity()
    }) {
      emp::DataNode<double, emp::data::Stats, emp::data::Log> raw_samples;
      emp::DataNode<double, emp::data::Stats, emp::data::Log> abs_samples;
      emp::DataNode<double, emp::data::Stats, emp::data::Log> control_samples;
      for (size_t i{}; i < 10*std::kilo::num; ++i) {
        raw_samples.Add(
          rand.GetRandZeroSymmetricPareto(alpha, lambda, -bound, bound)
        );
        abs_samples.Add( std::abs(
          rand.GetRandZeroSymmetricPareto(alpha, lambda, -bound, bound)
        ));
        control_samples.Add( rand.GetRandLomax(alpha, lambda, bound) );
      }

      REQUIRE(abs_samples.GetMean() > abs_samples.GetMedian());
      // can't do this test with means because of extreme variance
      // due to extreme effect outliers
      REQUIRE( raw_samples.GetMedian() == Approx(0).epsilon(0.1).margin(0.1) );
      REQUIRE(
        abs_samples.GetMedian()
        == Approx(control_samples.GetMedian()).epsilon(0.1).margin(0.1)
      );
      REQUIRE(
        abs_samples.GetPercentile(20)
        == Approx(control_samples.GetPercentile(20)).epsilon(0.2).margin(0.1)
      );
      REQUIRE(
        abs_samples.GetPercentile(80)
        == Approx(control_samples.GetPercentile(80)).epsilon(0.2).margin(0.1)
      );

    }

  }
  }

}
