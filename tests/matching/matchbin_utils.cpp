//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>

#include "emp/data/DataNode.hpp"
#include "emp/matching/matchbin_utils.hpp"
#include "emp/matching/MatchBin.hpp"
#include "emp/math/Random.hpp"

#include <sstream>
#include <string>

// TODO break these tests up into multiple files

TEST_CASE("Test matchbin_utils", "[matchbin]")
{

  // test ExactStreakDistribution
  {
    emp::ExactStreakDistribution<4> dist;

    REQUIRE( dist.GetStreakProbability(2,2) == 0.25 );
    REQUIRE( dist.GetStreakProbability(2,3) == 0.375 );
    REQUIRE( dist.GetStreakProbability(2,4) == 8.0/16.0 );

    REQUIRE( dist.GetStreakProbability(0) == 16.0/16.0 );
    REQUIRE( dist.GetStreakProbability(1) == 15.0/16.0 );
    REQUIRE( dist.GetStreakProbability(2) == 8.0/16.0 );
    REQUIRE( dist.GetStreakProbability(3) == 3.0/16.0 );
    REQUIRE( dist.GetStreakProbability(4) == 1.0/16.0 );

  }

  // test ApproxSingleStreakMetric
  {

    emp::ApproxSingleStreakMetric<4> metric;

    REQUIRE( metric({0,0,0,0},{0,0,0,0}) < metric({0,0,0,0},{1,0,0,0}) );
    REQUIRE( metric({0,0,0,0},{0,0,0,1}) < metric({0,0,0,0},{0,1,0,0}) );
    // REQUIRE( metric({0,0,0,0},{1,1,0,1}) < metric({0,0,0,0},{1,1,1,1}) );
    REQUIRE( metric({0,0,0,0},{1,1,0,1}) == 1.0 ); // in lieu
    REQUIRE( metric({0,0,0,0},{1,1,1,1}) == 1.0 ); // in lieu
    REQUIRE( metric({0,0,0,0},{1,1,0,0}) < metric({0,0,0,0},{1,1,0,1}) );
    // REQUIRE( metric({0,0,0,0},{1,1,0,1}) < metric({0,0,0,0},{1,1,1,0}) );
    REQUIRE( metric({0,0,0,0},{1,1,0,1}) == 1.0 ); // in lieu
    REQUIRE( metric({0,0,0,0},{1,1,1,0}) == 1.0 ); // in lieu
    REQUIRE( metric({0,0,0,0},{0,0,1,1}) == metric({0,0,0,0},{0,0,1,0}) );

    emp::Random rand(1);
    for (size_t i = 0; i < 1000; ++i) {
      emp::BitSet<4> a(rand);
      emp::BitSet<4> b(rand);
      REQUIRE(metric(a,b) <= 1.0);
      REQUIRE(metric(a,b) >= 0.0);
    }

  }

  // test ApproxDualStreakMetric
  {
  emp::ApproxDualStreakMetric<4> metric;

  REQUIRE( metric({0,0,0,0},{0,0,0,0}) < metric({0,0,0,0},{1,0,0,0}) );
  REQUIRE( metric({0,0,0,0},{0,0,0,1}) < metric({0,0,0,0},{0,1,0,0}) );
  REQUIRE( metric({0,0,0,0},{1,1,0,1}) < metric({0,0,0,0},{1,1,1,1}) );
  REQUIRE( metric({0,0,0,0},{1,1,0,0}) < metric({0,0,0,0},{1,1,0,1}) );
  REQUIRE( metric({0,0,0,0},{1,1,0,1}) < metric({0,0,0,0},{1,1,1,0}) );
  REQUIRE( metric({0,0,0,0},{0,0,1,1}) > metric({0,0,0,0},{0,0,1,0}) );

  emp::Random rand(1);
  for (size_t i = 0; i < 1000; ++i) {
    emp::BitSet<4> a(rand);
    emp::BitSet<4> b(rand);
    REQUIRE(metric(a,b) <= 1.0);
    REQUIRE(metric(a,b) >= 0.0);
  }

  }

  // test OptimizedApproxDualStreakMetric
  {
  emp::OptimizedApproxDualStreakMetric<4> metric;

  REQUIRE( metric({0,0,0,0},{0,0,0,0}) < metric({0,0,0,0},{1,0,0,0}) );
  REQUIRE( metric({0,0,0,0},{0,0,0,1}) < metric({0,0,0,0},{0,1,0,0}) );
  REQUIRE( metric({0,0,0,0},{1,1,0,1}) < metric({0,0,0,0},{1,1,1,1}) );
  REQUIRE( metric({0,0,0,0},{1,1,0,0}) < metric({0,0,0,0},{1,1,0,1}) );
  REQUIRE( metric({0,0,0,0},{1,1,0,1}) < metric({0,0,0,0},{1,1,1,0}) );
  REQUIRE( metric({0,0,0,0},{0,0,1,1}) > metric({0,0,0,0},{0,0,1,0}) );

  emp::Random rand(1);
  for (size_t i = 0; i < 1000; ++i) {
    emp::BitSet<4> a(rand);
    emp::BitSet<4> b(rand);
    // optimizations aren't free ¯\_(ツ)_/¯, approximation okay
    REQUIRE(metric(a,b) <= 1.05);
    REQUIRE(metric(a,b) >= -0.05);
  }

  }

  // test ExactSingleStreakMetric
  {

    emp::ExactSingleStreakMetric<4> metric;

    REQUIRE( metric({0,0,0,0},{0,0,0,0}) < metric({0,0,0,0},{1,0,0,0}) );
    REQUIRE( metric({0,0,0,0},{0,0,0,1}) < metric({0,0,0,0},{0,1,0,0}) );
    REQUIRE( metric({0,0,0,0},{1,1,0,1}) < metric({0,0,0,0},{1,1,1,1}) );
    REQUIRE( metric({0,0,0,0},{1,1,0,0}) < metric({0,0,0,0},{1,1,0,1}) );
    REQUIRE( metric({0,0,0,0},{1,1,0,1}) == metric({0,0,0,0},{1,1,1,0}) );
    REQUIRE( metric({0,0,0,0},{0,0,1,1}) == metric({0,0,0,0},{0,0,1,0}) );

    emp::Random rand(1);
    for (size_t i = 0; i < 1000; ++i) {
      emp::BitSet<4> a(rand);
      emp::BitSet<4> b(rand);
      REQUIRE(metric(a,b) <= 1.0);
      REQUIRE(metric(a,b) >= 0.0);
    }

  }

  // test ExactDualStreakMetric
  {

    emp::ExactDualStreakMetric<4> metric;

    REQUIRE( metric({0,0,0,0},{0,0,0,0}) < metric({0,0,0,0},{1,0,0,0}) );
    REQUIRE( metric({0,0,0,0},{0,0,0,1}) < metric({0,0,0,0},{0,1,0,0}) );
    REQUIRE( metric({0,0,0,0},{1,1,0,1}) < metric({0,0,0,0},{1,1,1,1}) );
    REQUIRE( metric({0,0,0,0},{1,1,0,0}) < metric({0,0,0,0},{1,1,0,1}) );
    REQUIRE( metric({0,0,0,0},{1,1,0,1}) < metric({0,0,0,0},{1,1,1,0}) );
    REQUIRE( metric({0,0,0,0},{0,0,1,1}) > metric({0,0,0,0},{0,0,1,0}) );

    emp::Random rand(1);
    for (size_t i = 0; i < 1000; ++i) {
      emp::BitSet<4> a(rand);
      emp::BitSet<4> b(rand);
      REQUIRE(metric(a,b) <= 1.0);
      REQUIRE(metric(a,b) >= 0.0);
    }

  }

  // test SieveSelector with auto adjust
  {
  emp::Random rand(1);
  emp::MatchBin<
    std::string,
    emp::NextUpMetric<>,
    emp::SieveSelector<>,
    emp::AdditiveCountdownRegulator<>
  > bin(rand);

  bin.Put("one", 1);

  bin.Put("two-two-seven", 227);

  bin.Put("nine-two-eight", 928);

  bin.Put("fifteen", 15);

  bin.Put("one-fifteen", 115);

  const size_t nrep = 1000;

  std::unordered_map<std::string, size_t> res;
  for (size_t rep = 0; rep < nrep; ++rep) {

    const auto matches = bin.GetVals(bin.Match(2));
    REQUIRE(matches.size() >= 2);

    std::unordered_set<std::string> uniques;

    for (const auto & val : matches) {
      ++res[val];
      uniques.insert(val);
    }

    REQUIRE(uniques.size() == matches.size());

  }

  REQUIRE(res["one"] == 0);
  REQUIRE(res["two-two-seven"] > 0);
  REQUIRE(res["two-two-seven"] < nrep);
  REQUIRE(res["nine-two-eight"] == 0);
  REQUIRE(res["one-fifteen"] == nrep);
  REQUIRE(res["fifteen"] == nrep);

  bin.Put(emp::to_string(0), 0);
  for (size_t i = 0; i < 45; ++i) {
    bin.Put(emp::to_string(i*10), i*10);
  }

  res.clear();

  for (size_t rep = 0; rep < nrep; ++rep) {
    for (const auto & val : bin.GetVals(bin.Match(2))) {
      ++res[val];
    }
  }

  REQUIRE(res["one"] == 0);
  REQUIRE(res["two-two-seven"] == 0);
  REQUIRE(res["nine-two-eight"] == 0);
  REQUIRE(res["one-fifteen"] > 0);
  REQUIRE(res["one-fifteen"] < nrep);
  REQUIRE(res["fifteen"] == nrep);
  }

  // test SieveSelector with no stochastic
  {
  emp::Random rand(1);
  emp::MatchBin<
    std::string,
    emp::NextUpMetric<>,
    emp::SieveSelector<std::ratio<0,1>>,
    emp::AdditiveCountdownRegulator<>
  > bin(rand);

  bin.Put("one", 1);

  bin.Put("two-two-seven", 227);

  bin.Put("nine-two-eight", 928);

  bin.Put("fifteen", 15);

  bin.Put("one-fifteen", 115);

  const size_t nrep = 1000;

  std::unordered_map<std::string, size_t> res;
  for (size_t rep = 0; rep < nrep; ++rep) {

    const auto matches = bin.GetVals(bin.Match(2));
    REQUIRE(matches.size() >= 2);

    std::unordered_set<std::string> uniques;

    for (const auto & val : matches) {
      ++res[val];
      uniques.insert(val);
    }

    REQUIRE(uniques.size() == matches.size());

  }

  REQUIRE(res["one"] == 0);
  REQUIRE(res["two-two-seven"] == 0);
  REQUIRE(res["nine-two-eight"] == 0);
  REQUIRE(res["one-fifteen"] == nrep);
  REQUIRE(res["fifteen"] == nrep);

  bin.Put(emp::to_string(0), 0);
  for (size_t i = 0; i < 45; ++i) {
    bin.Put(emp::to_string(i*10), i*10);
  }

  res.clear();

  for (size_t rep = 0; rep < nrep; ++rep) {
    for (const auto & val : bin.GetVals(bin.Match(2))) {
      ++res[val];
    }
  }

  REQUIRE(res["one"] == 0);
  REQUIRE(res["two-two-seven"] == 0);
  REQUIRE(res["nine-two-eight"] == 0);
  REQUIRE(res["one-fifteen"] == 0);
  REQUIRE(res["fifteen"] == nrep);
  }

  // test SieveSelector with no auto adjust
  {
  emp::Random rand(1);
  emp::MatchBin<
    std::string,
    emp::NextUpMetric<>,
    emp::SieveSelector<
      std::ratio<1, 10>,
      std::ratio<1, 5>
    >,
    emp::AdditiveCountdownRegulator<>
  > bin(rand);

  bin.Put("one", 1);

  bin.Put("two-two-seven", 227);

  bin.Put("nine-two-eight", 928);

  bin.Put("fifteen", 15);

  bin.Put("one-fifteen", 115);

  const size_t nrep = 1000;

  std::unordered_map<std::string, size_t> res;
  for (size_t rep = 0; rep < nrep; ++rep) {

    const auto matches = bin.GetVals(bin.Match(2));
    REQUIRE(matches.size() >= 2);

    std::unordered_set<std::string> uniques;

    for (const auto & val : matches) {
      ++res[val];
      uniques.insert(val);
    }

    REQUIRE(uniques.size() == matches.size());

  }

  REQUIRE(res["one"] == 0);
  REQUIRE(res["two-two-seven"] > 0);
  REQUIRE(res["two-two-seven"] < nrep);
  REQUIRE(res["nine-two-eight"] == 0);
  REQUIRE(res["one-fifteen"] == nrep);
  REQUIRE(res["fifteen"] == nrep);

  bin.Put(emp::to_string(0), 0);
  for (size_t i = 0; i < 45; ++i) {
    bin.Put(emp::to_string(i*10), i*10);
  }

  res.clear();

  for (size_t rep = 0; rep < nrep; ++rep) {
    for (const auto & val : bin.GetVals(bin.Match(2))) {
      ++res[val];
    }
  }

  REQUIRE(res["one"] == 0);
  REQUIRE(res["two-two-seven"] > 0);
  REQUIRE(res["two-two-seven"] < nrep);
  REQUIRE(res["nine-two-eight"] == 0);
  REQUIRE(res["one-fifteen"] == nrep);
  REQUIRE(res["fifteen"] == nrep);
  }

  // test PowMod, LogMod
  {
  emp::HammingMetric<4> baseline;

  emp::PowMod<emp::HammingMetric<4>, std::ratio<3>> squish_pow;
  emp::PowMod<emp::HammingMetric<4>, std::ratio<1>> same_pow;
  emp::PowMod<emp::HammingMetric<4>, std::ratio<1,3>> stretch_pow;

  emp::LogMod<emp::HammingMetric<4>, std::ratio<1,3>> squish_log;
  emp::LogMod<emp::HammingMetric<4>, std::ratio<1>> same_log;
  emp::LogMod<emp::HammingMetric<4>, std::ratio<3>> stretch_log;

  REQUIRE( squish_pow({0,0,0,0},{0,0,0,0}) == baseline({0,0,0,0},{0,0,0,0}) );
  REQUIRE( squish_pow({0,0,0,0},{0,0,0,1}) > baseline({0,0,0,0},{0,0,0,1}) );
  REQUIRE( squish_pow({0,0,0,0},{0,0,1,1}) == baseline({0,0,0,0},{0,0,1,1}) );
  REQUIRE( squish_pow({0,0,0,0},{0,1,1,1}) < baseline({0,0,0,0},{0,1,1,1}) );
  REQUIRE( squish_pow({0,0,0,0},{1,1,1,1}) == baseline({0,0,0,0},{1,1,1,1}) );

  REQUIRE( same_pow({0,0,0,0},{0,0,0,0}) == baseline({0,0,0,0},{0,0,0,0}) );
  REQUIRE( same_pow({0,0,0,0},{0,0,0,1}) == baseline({0,0,0,0},{0,0,0,1}) );
  REQUIRE( same_pow({0,0,0,0},{0,0,1,1}) == baseline({0,0,0,0},{0,0,1,1}) );
  REQUIRE( same_pow({0,0,0,0},{0,1,1,1}) == baseline({0,0,0,0},{0,1,1,1}) );
  REQUIRE( same_pow({0,0,0,0},{1,1,1,1}) == baseline({0,0,0,0},{1,1,1,1}) );

  REQUIRE( stretch_pow({0,0,0,0},{0,0,0,0}) == baseline({0,0,0,0},{0,0,0,0}) );
  REQUIRE( stretch_pow({0,0,0,0},{0,0,0,1}) < baseline({0,0,0,0},{0,0,0,1}) );
  REQUIRE( stretch_pow({0,0,0,0},{0,0,1,1}) == baseline({0,0,0,0},{0,0,1,1}) );
  REQUIRE( stretch_pow({0,0,0,0},{0,1,1,1}) > baseline({0,0,0,0},{0,1,1,1}) );
  REQUIRE( stretch_pow({0,0,0,0},{1,1,1,1}) == baseline({0,0,0,0},{1,1,1,1}) );

  REQUIRE( squish_log({0,0,0,0},{0,0,0,0}) == baseline({0,0,0,0},{0,0,0,0}) );
  REQUIRE( squish_log({0,0,0,0},{0,0,0,1}) > baseline({0,0,0,0},{0,0,0,1}) );
  REQUIRE( squish_log({0,0,0,0},{0,0,1,1}) == baseline({0,0,0,0},{0,0,1,1}) );
  REQUIRE( squish_log({0,0,0,0},{0,1,1,1}) < baseline({0,0,0,0},{0,1,1,1}) );
  REQUIRE( squish_log({0,0,0,0},{1,1,1,1}) == baseline({0,0,0,0},{1,1,1,1}) );

  REQUIRE( same_log({0,0,0,0},{0,0,0,0}) == baseline({0,0,0,0},{0,0,0,0}) );
  REQUIRE( same_log({0,0,0,0},{0,0,0,1}) == baseline({0,0,0,0},{0,0,0,1}) );
  REQUIRE( same_log({0,0,0,0},{0,0,1,1}) == baseline({0,0,0,0},{0,0,1,1}) );
  REQUIRE( same_log({0,0,0,0},{0,1,1,1}) == baseline({0,0,0,0},{0,1,1,1}) );
  REQUIRE( same_log({0,0,0,0},{1,1,1,1}) == baseline({0,0,0,0},{1,1,1,1}) );

  REQUIRE( stretch_log({0,0,0,0},{0,0,0,0}) == baseline({0,0,0,0},{0,0,0,0}) );
  REQUIRE( stretch_log({0,0,0,0},{0,0,0,1}) < baseline({0,0,0,0},{0,0,0,1}) );
  REQUIRE( stretch_log({0,0,0,0},{0,0,1,1}) == baseline({0,0,0,0},{0,0,1,1}) );
  REQUIRE( stretch_log({0,0,0,0},{0,1,1,1}) > baseline({0,0,0,0},{0,1,1,1}) );
  REQUIRE( stretch_log({0,0,0,0},{1,1,1,1}) == baseline({0,0,0,0},{1,1,1,1}) );
  }

  // more tests for PowMod, LogMod
  {

  emp::PowMod<emp::HashMetric<32>, std::ratio<5>> squish_pow;
  emp::PowMod<emp::HashMetric<32>, std::ratio<1>> same_pow;
  emp::PowMod<emp::HashMetric<32>, std::ratio<1,5>> stretch_pow;

  emp::LogMod<emp::HashMetric<32>, std::ratio<1,5>> squish_log;
  emp::LogMod<emp::HashMetric<32>, std::ratio<1>> same_log;
  emp::LogMod<emp::HashMetric<32>, std::ratio<5>> stretch_log;

  emp::Random rand(1);
  for (size_t rep = 0; rep < 1000; ++rep) {
    emp::BitSet<32> a(rand);
    emp::BitSet<32> b(rand);
    REQUIRE(squish_pow(a,b) >= 0.0);
    REQUIRE(squish_pow(a,b) <= 1.0);

    REQUIRE(same_pow(a,b) >= 0.0);
    REQUIRE(same_pow(a,b) <= 1.0);

    REQUIRE(stretch_pow(a,b) >= 0.0);
    REQUIRE(stretch_pow(a,b) <= 1.0);

    REQUIRE(squish_log(a,b) >= 0.0);
    REQUIRE(squish_log(a,b) <= 1.0);

    REQUIRE(same_log(a,b) >= 0.0);
    REQUIRE(same_log(a,b) <= 1.0);

    REQUIRE(stretch_log(a,b) >= 0.0);
    REQUIRE(stretch_log(a,b) <= 1.0);
  }

  }

  // test CacheMod
  // test PowMod, LogMod
  {
  emp::HammingMetric<4> baseline;

  emp::PowMod<emp::HammingMetric<4>, std::ratio<3>> squish;

  emp::CacheMod<emp::PowMod<
    emp::HammingMetric<4>,
    std::ratio<3>
  >> cache_squish;

  emp::CacheMod<emp::PowMod<
    emp::HammingMetric<4>,
    std::ratio<3>
  >, 2> small_cache_squish;

  // put in cache
  REQUIRE( squish({0,0,0,0},{0,0,0,0}) == cache_squish({0,0,0,0},{0,0,0,0}) );
  REQUIRE( squish({0,0,0,0},{0,0,0,1}) == cache_squish({0,0,0,0},{0,0,0,1}) );
  REQUIRE( squish({0,0,0,0},{0,0,1,1}) == cache_squish({0,0,0,0},{0,0,1,1}) );
  REQUIRE( squish({0,0,0,0},{0,1,1,1}) == cache_squish({0,0,0,0},{0,1,1,1}) );
  REQUIRE( squish({0,0,0,0},{1,1,1,1}) == cache_squish({0,0,0,0},{1,1,1,1}) );

  // hit cache
  REQUIRE( squish({0,0,0,0},{0,0,0,0}) == cache_squish({0,0,0,0},{0,0,0,0}) );
  REQUIRE( squish({0,0,0,0},{0,0,0,1}) == cache_squish({0,0,0,0},{0,0,0,1}) );
  REQUIRE( squish({0,0,0,0},{0,0,1,1}) == cache_squish({0,0,0,0},{0,0,1,1}) );
  REQUIRE( squish({0,0,0,0},{0,1,1,1}) == cache_squish({0,0,0,0},{0,1,1,1}) );
  REQUIRE( squish({0,0,0,0},{1,1,1,1}) == cache_squish({0,0,0,0},{1,1,1,1}) );

  // put in cache
  REQUIRE(
    squish({0,0,0,0},{0,0,0,0}) == small_cache_squish({0,0,0,0},{0,0,0,0})
  );
  REQUIRE(
    squish({0,0,0,0},{0,0,0,1}) == small_cache_squish({0,0,0,0},{0,0,0,1})
  );
  REQUIRE(
    squish({0,0,0,0},{0,0,1,1}) == small_cache_squish({0,0,0,0},{0,0,1,1})
  );
  REQUIRE(
    squish({0,0,0,0},{0,1,1,1}) == small_cache_squish({0,0,0,0},{0,1,1,1})
  );
  REQUIRE(
    squish({0,0,0,0},{1,1,1,1}) == small_cache_squish({0,0,0,0},{1,1,1,1})
  );

  // hit cache
  REQUIRE(
    squish({0,0,0,0},{0,0,0,0}) == small_cache_squish({0,0,0,0},{0,0,0,0})
  );
  REQUIRE(
    squish({0,0,0,0},{0,0,0,1}) == small_cache_squish({0,0,0,0},{0,0,0,1})
  );
  REQUIRE(
    squish({0,0,0,0},{0,0,1,1}) == small_cache_squish({0,0,0,0},{0,0,1,1})
  );
  REQUIRE(
    squish({0,0,0,0},{0,1,1,1}) == small_cache_squish({0,0,0,0},{0,1,1,1})
  );
  REQUIRE(
    squish({0,0,0,0},{1,1,1,1}) == small_cache_squish({0,0,0,0},{1,1,1,1})
  );

  }


  // test UnifMod
  {

  emp::HashMetric<32> hash;
  emp::UnifMod<emp::HashMetric<32>> unif_hash;
  emp::UnifMod<emp::HashMetric<32>, 1> unif_hash_small;

  emp::HammingMetric<32> hamming;
  emp::UnifMod<emp::HammingMetric<32>> unif_hamming;
  emp::UnifMod<emp::HammingMetric<32>, 1> unif_hamming_small;

  emp::Random rand(1);

  for (size_t rep = 0; rep < 5000; ++rep) {

    emp::BitSet<32> a(rand);
    emp::BitSet<32> b(rand);

    emp::BitSet<32> c(rand);
    emp::BitSet<32> d(rand);

    REQUIRE(unif_hash(a,b) >= 0.0);
    REQUIRE(unif_hash(a,b) <= 1.0);
    if (unif_hash(a,b) > unif_hash(c,d)) {
      REQUIRE(hash(a,b) > hash(c,d));
    } else if (unif_hash(a,b) < unif_hash(c,d)) {
      REQUIRE(hash(a,b) < hash(c,d));
    } else {
      // unif_hash(a,b) == unif_hash(c,d)
      REQUIRE(hash(a,b) == hash(c,d));
    }

    REQUIRE(unif_hash_small(a,b) >= 0.0);
    REQUIRE(unif_hash_small(a,b) <= 1.0);
    if (unif_hash_small(a,b) > unif_hash_small(c,d)) {
      REQUIRE(hash(a,b) > hash(c,d));
    } else if (unif_hash_small(a,b) < unif_hash_small(c,d)) {
      REQUIRE(hash(a,b) < hash(c,d));
    } else {
      // unif_hash_small(a,b) == unif_hash_small(c,d)
      REQUIRE(hash(a,b) == hash(c,d));
    }

    REQUIRE(unif_hamming(a,b) >= 0.0);
    REQUIRE(unif_hamming(a,b) <= 1.0);
    if (unif_hamming(a,b) > unif_hamming(c,d)) {
      REQUIRE(hamming(a,b) > hamming(c,d));
    } else if (unif_hamming(a,b) < unif_hamming(c,d)) {
      REQUIRE(hamming(a,b) < hamming(c,d));
    } else {
      // unif_hamming(a,b) == unif_hamming(c,d)
      REQUIRE(hamming(a,b) == hamming(c,d));
    }

    REQUIRE(unif_hamming_small(a,b) >= 0.0);
    REQUIRE(unif_hamming_small(a,b) <= 1.0);
    if (unif_hamming_small(a,b) > unif_hamming_small(c,d)) {
      REQUIRE(hamming(a,b) > hamming(c,d));
    } else if (unif_hamming_small(a,b) < unif_hamming_small(c,d)) {
      REQUIRE(hamming(a,b) < hamming(c,d));
    } else {
      // unif_hamming_small(a,b) == unif_hamming_small(c,d)
      REQUIRE(hamming(a,b) == hamming(c,d));
    }

  }

  }

  // test EuclideanDimMod
  {
  emp::Random rand(1);

  emp::BitSet<32> a1(rand);
  emp::BitSet<32> b1(rand);

  emp::HammingMetric<32> hamming;

  emp::FlatMod<
    emp::MeanDimMod<
      typename emp::HammingMetric<32>,
      1
    >
  > d_hamming1;
  REQUIRE(d_hamming1.width() == hamming.width());

  REQUIRE(hamming(a1, b1) == d_hamming1(a1, b1));
  }

  // test EuclideanDimMod
  {
  emp::Random rand(1);

  emp::BitSet<32> a1(rand);
  emp::BitSet<32> b1(rand);

  emp::HammingMetric<32> hamming;

  emp::FlatMod<
    emp::MeanDimMod<
      typename emp::HammingMetric<32>,
      1
    >
  > d_hamming1;
  REQUIRE(d_hamming1.width() == hamming.width());

  REQUIRE(hamming(a1, b1) == d_hamming1(a1, b1));
  }

  // more tests for EuclideanDimMod
  {
    emp::HammingMetric<4> hamming;

    emp::FlatMod<
      emp::EuclideanDimMod<
        typename emp::HammingMetric<2>,
        2
      >
    > d_hamming2;
    REQUIRE(d_hamming2.width() == hamming.width());

    REQUIRE(d_hamming2({0,0,0,0}, {0,0,0,0}) == 0.0);

    REQUIRE(d_hamming2({0,0,1,1}, {0,0,0,0}) == std::sqrt(0.5));
    REQUIRE(d_hamming2({0,0,0,0}, {1,1,0,0}) == std::sqrt(0.5));
    REQUIRE(d_hamming2({0,0,1,1}, {1,1,1,1}) == std::sqrt(0.5));
    REQUIRE(d_hamming2({1,1,1,1}, {0,0,1,1}) == std::sqrt(0.5));

    REQUIRE(d_hamming2({0,0,1,1}, {0,1,1,0}) == 0.5);
    REQUIRE(d_hamming2({0,0,1,1}, {0,1,1,0}) == 0.5);
    REQUIRE(d_hamming2({0,0,0,0}, {0,1,1,0}) == 0.5);
    REQUIRE(d_hamming2({0,1,1,1}, {1,1,1,0}) == 0.5);

    REQUIRE(d_hamming2({0,0,0,0}, {1,1,1,1}) == 1.0);
    REQUIRE(d_hamming2({1,1,1,1}, {0,0,0,0}) == 1.0);
  }

  // more tests for EuclideanDimMod
  {

    emp::FlatMod<
      emp::MeanDimMod<
        typename emp::HammingMetric<8>,
        4
      >
    > metric;

  emp::Random rand(1);
  for (size_t rep = 0; rep < 1000; ++rep) {
    emp::BitSet<32> a(rand);
    emp::BitSet<32> b(rand);
    REQUIRE(metric(a,b) >= 0.0);
    REQUIRE(metric(a,b) <= 1.0);
  }

  }

  // tests for AdditiveCountdownRegulator
  {

    // TODO:
    // Fails with random seed 1, passes with other random seeds (2 & 3)
    // Failure on seed 1 appears stochastic, but we should investigate further and
    // clean up this test.
    emp::Random rand(1);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RouletteSelector<>,
    emp::AdditiveCountdownRegulator<>
  >bin(rand);

  const size_t ndraws = 100000;

  const size_t hi = bin.Put("hi", std::numeric_limits<int>::max()/5);
  REQUIRE( bin.GetVal(hi) == "hi" );
  const size_t salut = bin.Put("salut", std::numeric_limits<int>::max()/100);
  REQUIRE( bin.GetVal(salut) == "salut" );

  REQUIRE( bin.Size() == 2 );
  REQUIRE( bin.ViewRegulator(hi) == 0.0 );
  REQUIRE( bin.ViewRegulator(salut) == 0.0 );

  // baseline, "salut" should match much better
  auto res = bin.GetVals(bin.Match(0, ndraws));
  const size_t count = std::count(std::begin(res), std::end(res), "salut");
  REQUIRE( count > ndraws/2 );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  // downregulate "salut," now "hi" should match better
  bin.AdjRegulator(salut, 20.0); // downregulate
  REQUIRE( bin.ViewRegulator(salut) == 20.0 );
  REQUIRE( bin.ViewRegulator(hi) == 0.0 );
  res = bin.GetVals(bin.Match(0, ndraws));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > 0 );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > ndraws/2 );

  // upregulate both, "hi" should still match better
  bin.AdjRegulator(hi, -20.0); // upregulate
  bin.AdjRegulator(salut, -20.0); // upregulate
  REQUIRE( bin.ViewRegulator(salut) == 0.0 );
  REQUIRE( bin.ViewRegulator(hi) == -20.0 );
  res = bin.GetVals(bin.Match(0, ndraws));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > 0 );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > ndraws/2 );

  // set salut and hi regulators, salut hi should still match better
  bin.SetRegulator(salut, 2.0); // downregulate
  bin.SetRegulator(hi, -2.0); // upregulate
  REQUIRE( bin.ViewRegulator(salut) == 2.0 );
  REQUIRE( bin.ViewRegulator(hi) == -2.0 );

  res = bin.GetVals(bin.Match(0, ndraws));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > 0 );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > ndraws/2 );

  // set salut and hi regulators, now salut should match better
  // and should match even better than it at the top
  bin.SetRegulator(salut, -1.0); // upregulate
  bin.SetRegulator(hi, 1.0); // downregulate
  REQUIRE( bin.ViewRegulator(salut) == -1.0 );
  REQUIRE( bin.ViewRegulator(hi) == 1.0 );
  res = bin.GetVals(bin.Match(0, ndraws));
  const size_t hi_count = std::count(std::begin(res), std::end(res), "salut");
  REQUIRE( hi_count > count );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  // reverse-decay regulator, regulator values should be unaffected
  bin.DecayRegulator(salut, -2);
  REQUIRE( bin.ViewRegulator(salut) == -1.0 );
  REQUIRE( bin.ViewRegulator(hi) == 1.0 );

  // salut should still match even better than it at the top
  res = bin.GetVals(bin.Match(0, ndraws));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > count );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  // decay the salut regulator but not the hi regulator
  bin.DecayRegulator(salut, 1);
  bin.DecayRegulator(hi, 0);
  REQUIRE( bin.ViewRegulator(salut) == -1.0 );
  REQUIRE( bin.ViewRegulator(hi) == 1.0 );

  // salut should still match even better than it did at the top
  res = bin.GetVals(bin.Match(0, ndraws));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > count );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  // decay the regulators down to baseline
  bin.DecayRegulator(salut, 500);
  bin.DecayRegulators();
  REQUIRE( bin.ViewRegulator(salut) == 0.0 );
  REQUIRE( bin.ViewRegulator(hi) == 0.0 );
  // salut should match better than hi, but not as well as it did when it was
  // upregulated
  res = bin.GetVals(bin.Match(0, ndraws));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > ndraws/2 );
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") < hi_count );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  }

  // tests for MultiplicativeCountdownRegulator
  {

  emp::Random rand(1);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RouletteSelector<>,
    emp::MultiplicativeCountdownRegulator<>
  >bin(rand);

  const size_t ndraws = 1000000;

  const size_t hi = bin.Put("hi", std::numeric_limits<int>::max()/2);
  REQUIRE( bin.GetVal(hi) == "hi" );
  const size_t salut = bin.Put("salut", std::numeric_limits<int>::max()/10);
  REQUIRE( bin.GetVal(salut) == "salut" );

  REQUIRE( bin.Size() == 2 );
  REQUIRE( bin.ViewRegulator(hi) == 0.0 );
  REQUIRE( bin.ViewRegulator(salut) == 0.0 );

  auto res = bin.GetVals(bin.Match(0, ndraws));
  const size_t count = std::count(std::begin(res), std::end(res), "salut");
  REQUIRE( count > ndraws/2);
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  bin.AdjRegulator(salut, 20.0); // downregulate
  REQUIRE( bin.ViewRegulator(salut) == 20.0 );
  REQUIRE( bin.ViewRegulator(hi) == 0.0 );
  res = bin.GetVals(bin.Match(0, ndraws));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > 0 );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > ndraws/2 );

  bin.AdjRegulator(hi, -20.0); // upregulate
  bin.AdjRegulator(salut, -20.0); // restore
  REQUIRE( bin.ViewRegulator(salut) == 0.0 );
  REQUIRE( bin.ViewRegulator(hi) == -20.0 );
  res = bin.GetVals(bin.Match(0, ndraws));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > 0 );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > ndraws/2 );

  bin.SetRegulator(salut, 5.0); // downregulate
  bin.SetRegulator(hi, -5.0); // upregulate
  REQUIRE( bin.ViewRegulator(salut) == 5.0 );
  REQUIRE( bin.ViewRegulator(hi) == -5.0 );

  bin.SetRegulator(salut, -1.0); // upregulate
  bin.SetRegulator(hi, 1.0); // downregulate
  REQUIRE( bin.ViewRegulator(salut) == -1.0 );
  REQUIRE( bin.ViewRegulator(hi) == 1.0 );
  res = bin.GetVals(bin.Match(0, ndraws));
  const size_t hi_count = std::count(std::begin(res), std::end(res), "salut");
  REQUIRE( hi_count > count );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  bin.DecayRegulator(salut, -2);
  REQUIRE( bin.ViewRegulator(salut) == -1.0 );
  REQUIRE( bin.ViewRegulator(hi) == 1.0 );

  res = bin.GetVals(bin.Match(0, ndraws));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > count );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  bin.DecayRegulator(salut, 1);
  bin.DecayRegulator(hi, 0);
  REQUIRE( bin.ViewRegulator(salut) == -1.0 );
  REQUIRE( bin.ViewRegulator(hi) == 1.0 );

  res = bin.GetVals(bin.Match(0, ndraws));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > count );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  bin.DecayRegulator(salut, 500);
  bin.DecayRegulator(hi, 1);
  REQUIRE( bin.ViewRegulator(salut) == 0.0 );
  REQUIRE( bin.ViewRegulator(hi) == 0.0 );
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > ndraws/2 );
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") < hi_count );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  }

  // tests for NopRegulator
  {

  emp::Random rand(1);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RouletteSelector<>,
    emp::NopRegulator
  >bin(rand);

  const size_t ndraws = 1000000;
  const size_t error = 5000;

  const size_t hi = bin.Put("hi", std::numeric_limits<int>::max()/2);
  REQUIRE( bin.GetVal(hi) == "hi" );
  const size_t salut = bin.Put("salut", std::numeric_limits<int>::max()/10);
  REQUIRE( bin.GetVal(salut) == "salut" );

  REQUIRE( bin.Size() == 2 );
  REQUIRE( bin.ViewRegulator(hi) == 0.0 );
  REQUIRE( bin.ViewRegulator(salut) == 0.0 );

  auto res = bin.GetVals(bin.Match(0, ndraws));
  const size_t count = std::count(std::begin(res), std::end(res), "salut");
  REQUIRE( count > ndraws/2);
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  bin.AdjRegulator(salut, 20.0); // downregulate
  REQUIRE( bin.ViewRegulator(salut) == 0.0 );
  REQUIRE( bin.ViewRegulator(hi) == 0.0 );
  res = bin.GetVals(bin.Match(0, ndraws));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > ndraws/2 );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  bin.AdjRegulator(hi, -20.0); // upregulate
  bin.AdjRegulator(salut, -20.0); // restore
  REQUIRE( bin.ViewRegulator(salut) == 0.0 );
  REQUIRE( bin.ViewRegulator(hi) == 0.0 );
  res = bin.GetVals(bin.Match(0, ndraws));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > ndraws/2 );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  bin.SetRegulator(salut, 5.0); // downregulate
  bin.SetRegulator(hi, -5.0); // upregulate
  REQUIRE( bin.ViewRegulator(salut) == 0.0 );
  REQUIRE( bin.ViewRegulator(hi) == 0.0 );

  bin.SetRegulator(salut, -1.0); // upregulate
  bin.SetRegulator(hi, 1.0); // downregulate
  REQUIRE( bin.ViewRegulator(salut) == 0.0 );
  REQUIRE( bin.ViewRegulator(hi) == 0.0 );
  res = bin.GetVals(bin.Match(0, ndraws));
  const size_t hi_count = std::count(std::begin(res), std::end(res), "salut");
  REQUIRE( std::max(hi_count, count) - std::min(hi_count, count) < error );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  bin.DecayRegulator(salut, -2);
  REQUIRE( bin.ViewRegulator(salut) == 0.0 );
  REQUIRE( bin.ViewRegulator(hi) == 0.0 );

  {
  res = bin.GetVals(bin.Match(0, ndraws));
  const size_t s_count = std::count(std::begin(res), std::end(res), "salut");
  REQUIRE( std::max(s_count, count) - std::min(s_count, count) < error );
  const size_t h_count = std::count(std::begin(res), std::end(res), "hi");
  REQUIRE((
    std::max(h_count, ndraws - count)
    - std::min(h_count, ndraws - count)
    < error
  ));
  }

  bin.DecayRegulator(salut, 1);
  bin.DecayRegulator(hi, 0);
  REQUIRE( bin.ViewRegulator(salut) == 0.0 );
  REQUIRE( bin.ViewRegulator(hi) == 0.0 );

  {
  res = bin.GetVals(bin.Match(0, ndraws));
  const size_t s_count = std::count(std::begin(res), std::end(res), "salut");
  REQUIRE( std::max(s_count, count) - std::min(s_count, count) < error );
  REQUIRE( std::max(s_count, hi_count) - std::min(s_count, hi_count) < error );
  const size_t h_count = std::count(std::begin(res), std::end(res), "hi");
  REQUIRE((
    std::max(h_count, ndraws - count)
    - std::min(h_count, ndraws - count)
    < error
  ));
  }

  bin.DecayRegulator(salut, 500);
  bin.DecayRegulator(hi, 1);
  REQUIRE( bin.ViewRegulator(salut) == 0.0 );
  REQUIRE( bin.ViewRegulator(hi) == 0.0 );
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > ndraws/2 );
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") < hi_count );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  {
  res = bin.GetVals(bin.Match(0, ndraws));
  const size_t s_count = std::count(std::begin(res), std::end(res), "salut");
  REQUIRE( std::max(s_count, count) - std::min(s_count, count) < error );
  REQUIRE( std::max(s_count, hi_count) - std::min(s_count, hi_count) < error );
  const size_t h_count = std::count(std::begin(res), std::end(res), "hi");
  REQUIRE((
    std::max(h_count, ndraws - count)
    - std::min(h_count, ndraws - count)
    < error
  ));
  }

  }


}

TEST_CASE("Test codon metric", "[matchbin]")
{

    emp::Random rand(1);

    emp::MatchBin<
      size_t,
      emp::CodonMetric<64>,
      emp::RankedSelector<>,
      emp::NopRegulator
    >bin(rand);

    for (size_t i{}; i < 100; ++i) {
      bin.Put(i, emp::BitSet<64>(rand));
    }

    REQUIRE( bin.Size() == 100 );

    for (size_t rep{}; rep < 100; ++rep) {
      const auto res = bin.Match(emp::BitSet<64>(rand));
      REQUIRE(res.size() == 1);
    }

}
