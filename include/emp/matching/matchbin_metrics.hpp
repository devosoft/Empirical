/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file matchbin_metrics.hpp
 *  @brief Metric structs that can be plugged into MatchBin.
 *
 */


#ifndef EMP_MATCH_BIN_METRICS_H
#define EMP_MATCH_BIN_METRICS_H

#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <ratio>
#include <string>
#include <tuple>
#include <array>
#include <utility>
#include <queue>

#include "../../../third-party/robin-hood-hashing/src/include/robin_hood.h"

#include "../base/assert.hpp"
#include "../base/array.hpp"
#include "../base/vector.hpp"
#include "../bits/BitSet.hpp"
#include "../datastructs/IndexMap.hpp"
#include "../datastructs/hash_utils.hpp"
#include "../datastructs/tuple_utils.hpp"
#include "../math/math.hpp"
#include "../math/Distribution.hpp"
#include "../tools/string_utils.hpp"
#include "../polyfill/span.hpp"

namespace emp {

  /// Abstract base class for metrics
  template<typename Query, typename Tag>
  struct BaseMetric {

    virtual ~BaseMetric() {};
    virtual double operator()(const Query& a, const Tag& b) const = 0;
    virtual size_t dim() const = 0;
    virtual size_t width() const = 0;
    virtual std::string name() const = 0;
    virtual std::string base() const = 0;

  };

  /// Metric for MatchBin stored in the struct so we can template on it
  /// Returns the number of bits not in common between two BitSets
  template<size_t Width>
  struct HammingMetric: public BaseMetric<emp::BitSet<Width>, emp::BitSet<Width>> {

    using query_t = emp::BitSet<Width>;
    using tag_t = emp::BitSet<Width>;

    size_t dim() const override { return 1; }

    size_t width() const override { return Width; }

    std::string name() const override {
      return emp::to_string(Width) + "-bit " + base();
    }

    std::string base() const override { return "Hamming Metric"; }

    double operator()(const query_t& a, const tag_t& b) const override {
      return calculate(a, b);
    }

    inline static double calculate(const query_t& a, const tag_t& b) {
      return (double)(a^b).CountOnes() / Width;
    }

  };

  /// Generate an arbitrary, but consistent, match score between 0 and 1
  template<size_t Width>
  struct HashMetric: public BaseMetric<emp::BitSet<Width>, emp::BitSet<Width>> {

    using query_t = emp::BitSet<Width>;
    using tag_t = emp::BitSet<Width>;

    size_t dim() const override { return 1; }

    size_t width() const override { return Width; }

    std::string name() const override {
      return emp::to_string(Width) + "-bit " + base();
    }

    std::string base() const override { return "Hash Metric"; }

    double operator()(const query_t& a, const tag_t& b) const override {
      return calculate(a, b);
    }

    inline static double calculate(const query_t& a, const tag_t& b) {
      std::hash<query_t> qhasher;
      std::hash<tag_t> thasher;

      size_t input[2]{qhasher(a), thasher(b)};
      const std::span<const std::byte> input_as_bytes(
          reinterpret_cast<std::byte*>(input),
          sizeof(input)
      );
      const size_t hash = emp::murmur_hash(input_as_bytes);
      return static_cast<double>(hash) / static_cast<double>(std::numeric_limits<size_t>::max());
    }

  };

  #ifdef EMP_HAS_CRYPTO
  #include <openssl/sha.h>
  /// Generate an arbitrary, but consistent, match score between 0 and 1
  /// Be sure to link against -lcrypto, -lssl, and provide -DEMP_HAS_CRYPTO
  template<size_t Width>
  struct CryptoHashMetric: public BaseMetric<emp::BitSet<Width>, emp::BitSet<Width>> {

    using query_t = emp::BitSet<Width>;
    using tag_t = emp::BitSet<Width>;

    size_t dim() const override { return 1; }

    size_t width() const override { return Width; }

    std::string name() const override {
      return emp::to_string(Width) + "-bit " + base();
    }

    std::string base() const override { return "Hash Metric"; }

    double operator()(const query_t& a, const tag_t& b) const override {
      return calculate(a, b);
    }

    // adapted from https://www.uncg.edu/cmp/faculty/srtate/580.f11/sha1examples.php
    inline static double calculate(const query_t& a, const tag_t& b) {

    std::array<unsigned char, a.GetNumBytes() + b.GetNumBytes()> data;

    size_t i = 0;
    for (size_t j = 0; j < a.GetNumBytes(); ++j) data[i++] = a.GetByte(j);
    for (size_t j = 0; j < b.GetNumBytes(); ++j) data[i++] = b.GetByte(j);

    SHA_CTX shactx;
    std::array<unsigned char, SHA_DIGEST_LENGTH> digest;

    SHA1_Init(&shactx);
    SHA1_Update(&shactx, data.data(), data.size());
    SHA1_Final(digest.data(), &shactx);

    const std::string hashme{
      reinterpret_cast<const char *>(digest.data()),
      digest.size()
    };

    return (
      static_cast<double>(std::hash<std::string>{}(hashme))
      / std::numeric_limits<size_t>::max()
    );

  }

  };
  #endif

  /// Metric gives the absolute difference between two integers
  struct AbsDiffMetric : public BaseMetric<int, int> {

    using query_t = int;
    using tag_t = int;

    size_t dim() const override { return 1; }

    size_t width() const override { return sizeof(int) * 8; }

    std::string name() const override {
      return base();
    }

    std::string base() const override {
      return "Absolute Integer Difference Metric";
    }

    double operator()(const query_t& a, const tag_t& b) const override {
      return calculate(a, b);
    }

    inline static double calculate(const query_t& a, const tag_t& b) {

      return (double)std::abs(a-b) / std::numeric_limits<int>::max();
    }

  };

  /// Metric gives the matchings by the closest tag on or above itself.
  /// Wraps on Max.
  /// Adapted from Spector, Lee, et al. "Tag-based modules in genetic programming." Proceedings of the 13th annual conference on Genetic and evolutionary computation. ACM, 2011.
  template<size_t Max=1000>
  struct NextUpMetric : public BaseMetric<size_t, size_t> {

    using query_t = size_t;
    using tag_t = size_t;

    size_t dim() const override { return 1; }

    size_t width() const override { return sizeof(size_t) * 8; }

    std::string name() const override { return base(); }

    std::string base() const override { return "Next Up Metric"; }

    double operator()(const query_t& a, const tag_t& b) const override {
      return calculate(a, b);
    }

    inline static double calculate(const query_t& a, const tag_t& b) {
      const size_t difference = ((Max + 1) + b - a) % (Max + 1);
      return static_cast<double>(difference % (Max + 1)) / Max;
    }

  };

  /// BitSet-based implementation of NextUpMetric.
  /// Adapted from Spector, Lee, et al. "Tag-based modules in genetic programming." Proceedings of the 13th annual conference on Genetic and evolutionary computation. ACM, 2011.
  template<size_t Width>
  struct AsymmetricWrapMetric : public BaseMetric<emp::BitSet<Width>, emp::BitSet<Width>> {

    using query_t = emp::BitSet<Width>;
    using tag_t = emp::BitSet<Width>;

    size_t dim() const override { return 1; }

    size_t width() const override { return Width; }

    std::string name() const override {
      return emp::to_string(Width) + "-bit " + base();
    }

    std::string base() const override { return "Asymmetric Wrap Metric"; }

    double operator()(const query_t& a, const tag_t& b) const override {
      return calculate(a, b);
    }

    inline static double calculate(const query_t& a, const tag_t& b) {
      return (b - a).GetDouble() / emp::BitSet<Width>::MaxDouble();
    }

  };

  /// BitSet-based implementation of NextUpMetric without wrapping.
  template<size_t Width>
  struct AsymmetricNoWrapMetric : public BaseMetric<emp::BitSet<Width>, emp::BitSet<Width>> {

    using query_t = emp::BitSet<Width>;
    using tag_t = emp::BitSet<Width>;

    size_t dim() const override { return 1; }

    size_t width() const override { return Width; }

    std::string name() const override {
      return emp::to_string(Width) + "-bit " + base();
    }

    std::string base() const override { return "Asymmetric No-Wrap Metric"; }

    double operator()(const query_t& a, const tag_t& b) const override {
      return calculate(a, b);
    }

    inline static double calculate(const query_t& a, const tag_t& b) {
      constexpr double max_dist = emp::BitSet<Width>::MaxDouble() + 1.0;
      return (b >= a ? (b - a).GetDouble() : max_dist) / max_dist;
    }

  };

  /// Metric gives the absolute value of the difference between the integer
  /// representations of the BitSets with wrap from zero to the maximum value
  /// the BitSet can represent.
  /// Adapted from Downing, Keith L. Intelligence emerging: adaptivity and search in evolving neural systems. MIT Press, 2015.
  template<size_t Width>
  struct SymmetricWrapMetric : public BaseMetric<emp::BitSet<Width>, emp::BitSet<Width>> {

    using query_t = emp::BitSet<Width>;
    using tag_t = emp::BitSet<Width>;

    size_t dim() const override { return 1; }

    size_t width() const override { return Width; }

    std::string name() const override {
      return emp::to_string(Width) + "-bit " + base();
    }

    std::string base() const override { return "Symmetric Wrap Metric"; }

    double operator()(const query_t& a, const tag_t& b) const override {
      return calculate(a, b);
    }

    inline static double calculate(const query_t& a, const tag_t& b) {
      constexpr double max_dist = (
        (emp::BitSet<Width>::MaxDouble() + 1.0) / 2.0
      );
      return std::min(a - b, b - a).GetDouble() / max_dist;
    }

  };

  /// Metric gives the absolute value of the difference between the integer
  /// representations of the BitSets.
  /// Adapted from Downing, Keith L. Intelligence emerging: adaptivity and search in evolving neural systems. MIT Press, 2015.
  template<size_t Width>
  struct SymmetricNoWrapMetric : public BaseMetric<emp::BitSet<Width>, emp::BitSet<Width>> {

    using query_t = emp::BitSet<Width>;
    using tag_t = emp::BitSet<Width>;

    size_t dim() const override { return 1; }

    size_t width() const override { return Width; }

    std::string name() const override {
      return emp::to_string(Width) + "-bit " + base();
    }

    std::string base() const override { return "Symmetric No-Wrap Metric"; }

    double operator()(const query_t& a, const tag_t& b) const override {
      return calculate(a, b);
    }

    inline static double calculate(const query_t& a, const tag_t& b) {
      return (
        a > b ? a - b : b - a
      ).GetDouble() /  emp::BitSet<Width>::MaxDouble();
    }

  };

  /// Matches based on the number of bits in common between the two bitsets.
  /// Normalized so that each match score represents the cumulative probability
  /// of an as-good or better match.
  template<size_t Width>
  struct HammingCumuMetric : public BaseMetric<emp::BitSet<Width>, emp::BitSet<Width>> {

    struct static_constructed : public emp::vector<double> {

      static_constructed() {
        double cumsum = 0.0;
        emp::Binomial bino(0.5, Width);
        while (this->size() <= Width) {
          cumsum += bino[this->size()];
          this->push_back(cumsum);
        }
      }

    };

    inline static static_constructed cumulative{};

    using query_t = emp::BitSet<Width>;
    using tag_t = emp::BitSet<Width>;

    size_t dim() const override { return 1; }

    size_t width() const override { return Width; }

    std::string name() const override {
      return emp::to_string(Width) + "-bit " + base();
    }

    std::string base() const override { return "Cumulative Hamming Metric"; }

    double operator()(const query_t& a, const tag_t& b) const override {
      return calculate(a, b);
    }

    inline static double calculate(const query_t& a, const tag_t& b) {

      return cumulative[(a^b).CountOnes()];

    }

  };

  /// Matches based on longest streaks of equal bits in two bitsets.
  /// This implementation uses Incorrect Math from Downing's Intelligence
  /// Emerging.
  /// Adapted from Downing, Keith L. Intelligence emerging: adaptivity and search in evolving neural systems. MIT Press, 2015.
  template<size_t Width>
  struct ApproxSingleStreakMetric : public BaseMetric<
    emp::BitSet<Width>,
    emp::BitSet<Width>
  > {

    using query_t = emp::BitSet<Width>;
    using tag_t = emp::BitSet<Width>;

    size_t dim() const override { return 1; }

    size_t width() const override { return Width; }

    std::string name() const override {
      return emp::to_string(Width) + "-bit " + base();
    }

    std::string base() const override { return "Approx Single Streak Metric"; }

    double operator()(const query_t& a, const tag_t& b) const override {
      return calculate(a, b);
    }

    inline static double calculate(const query_t& a, const tag_t& b) {
      const emp::BitSet<Width> bs = a^b;
      const size_t same = (~bs).LongestSegmentOnes();
      const double ps = ProbabilityKBitSequence(same);

      return ps;
    }

    static inline double ProbabilityKBitSequence(const size_t k) {
      const double numerator = static_cast<double>(Width - k + 1);

      double denominator;
      if constexpr (Width < 511) {
        emp_assert(k < 511);
        denominator = emp::exp2_overflow_unsafe(-static_cast<int>(k));
      } else {
        denominator = emp::exp2(-static_cast<int>(k));
      }

      // Bad Math
      // ... at least clamp it
      return std::clamp(numerator * denominator, 0.0, 1.0);

    }

  };

  /// Matches based on longest streaks of equal and unequal bits in two bitsets.
  /// This implementation uses Incorrect Math from Downing's Intelligence
  /// Emerging.
  /// Adapted from Downing, Keith L. Intelligence emerging: adaptivity and search in evolving neural systems. MIT Press, 2015.
  template<size_t Width>
  struct ApproxDualStreakMetric : public BaseMetric<
    emp::BitSet<Width>,
    emp::BitSet<Width>
  > {

    using query_t = emp::BitSet<Width>;
    using tag_t = emp::BitSet<Width>;

    size_t dim() const override { return 1; }

    size_t width() const override { return Width; }

    std::string name() const override {
      return emp::to_string(Width) + "-bit " + base();
    }

    std::string base() const override { return "Approx Dual Streak Metric"; }

    double operator()(const query_t& a, const tag_t& b) const override {
      return calculate(a, b);
    }

    inline static double calculate(const query_t& a, const tag_t& b) {
      const emp::BitSet<Width> bs = a^b;
      const size_t same = (~bs).LongestSegmentOnes();
      const size_t different = bs.LongestSegmentOnes();
      const double ps = ProbabilityKBitSequence(same);
      const double pd = ProbabilityKBitSequence(different);

      const double match = pd / (ps + pd);
      // Note: here, close match score > poor match score
      // However, we're computing distance where smaller means closer match.
      // Note also: 0.0 < match < 1.0
      // So, we subtract match score from 1.0 to get a distance.
      return 1.0 - match;
    }

    // Approximate Math
    inline static double ProbabilityKBitSequence(const size_t k) {

      const double numerator = static_cast<double>(Width - k + 1);

      double denominator;
      if constexpr (Width < 511) {
        emp_assert(k < 511);
        denominator = emp::exp2_overflow_unsafe(-static_cast<int>(k));
      } else {
        denominator = emp::exp2(-static_cast<int>(k));
      }

      // Bad Math
      // but no need to clamp
      return numerator * denominator;

    }

  };

  /// Matches based on longest streaks of equal and unequal bits in two bitsets.
  /// This implementation uses Incorrect Math from Downing's Intelligence
  /// Emerging.
  /// Adapted from Downing, Keith L. Intelligence emerging: adaptivity and search in evolving neural systems. MIT Press, 2015.
  /// ApproxDualStreakMetric has been optimized directly, so this metric is
  /// redundant and should not be used.
  template<size_t Width>
  using OptimizedApproxDualStreakMetric [[deprecated]] = ApproxDualStreakMetric<Width>;

  /// Compute the probability of K or more heads in a row out of N flips.
  /// Adapted from https://www.askamathematician.com/2010/07/q-whats-the-chance-of-getting-a-run-of-k-successes-in-n-bernoulli-trials-why-use-approximations-when-the-exact-answer-is-known/
  /// Helper class for exact streak metrics
  template<size_t N>
  struct ExactStreakDistribution {

    robin_hood::unordered_map<
      std::tuple<size_t, size_t>, /* (min_heads, num_coins) */
      double,
      emp::TupleHash<size_t, size_t>
    > computed{};

    ExactStreakDistribution() {
      // prep the cache
      for (size_t min_heads = 0; min_heads <= N; ++min_heads) {
        CalcStreakProbability(min_heads);
      }
    }

    double GetStreakProbability(
      const size_t min_heads,
      const size_t num_coins=N
    ) const { return computed.at({min_heads, num_coins}); }

    double CalcStreakProbability(
      const size_t min_heads,
      const size_t num_coins=N
    ) {

      // edge cases for recursion
      if (min_heads > num_coins || num_coins <= 0) return 0.0;

      // check the cache
      if (computed.find({min_heads, num_coins}) != std::end(computed)) {
        return computed.at({min_heads, num_coins});
      }

      constexpr double head_prob = 0.5;

      double res = emp::Pow(head_prob, static_cast<double>(min_heads));
      for (size_t first_tail = 0; first_tail < min_heads; ++first_tail) {
        res += (
          emp::Pow(head_prob, static_cast<double>(first_tail))
          * (1.0 - head_prob)
          * CalcStreakProbability(
            min_heads,
            num_coins - first_tail - 1
          )
        );
      }

      // cache result
      computed[{min_heads, num_coins}] = res;

      emp_assert(res >= 0.0 && res <= 1.0);

      return res;

    }

  };

  // RE: the “static initialization order ‘fiasco’ (problem)”
  // https://isocpp.org/wiki/faq/ctors#static-init-order-on-first-use
  template<size_t N>
  const ExactStreakDistribution<N>&
  ExactStreakDistribution_ConstructOnFirstUse() {
    static ExactStreakDistribution<N>* res = new ExactStreakDistribution<N>{};
    return *res;
  }

  /// Matches based on longest streaks of equal and unequal bits in two bitsets.
  /// This implementation uses Corect Math adapted from
  /// https://www.askamathematician.com/2010/07/q-whats-the-chance-of-getting-a-run-of-k-successes-in-n-bernoulli-trials-why-use-approximations-when-the-exact-answer-is-known/
  /// This metric is NOT uniformly distributed.
  /// Adapted from Downing, Keith L. Intelligence emerging: adaptivity and search in evolving neural systems. MIT Press, 2015.
  template<size_t Width>
  struct ExactDualStreakMetric : public BaseMetric<
    emp::BitSet<Width>,
    emp::BitSet<Width>
  > {

    using query_t = emp::BitSet<Width>;
    using tag_t = emp::BitSet<Width>;

    static const ExactStreakDistribution<Width>& GetDistn() {
      return ExactStreakDistribution_ConstructOnFirstUse<Width>();
    };

    size_t dim() const override { return 1; }

    size_t width() const override { return Width; }

    std::string name() const override {
      return emp::to_string(Width) + "-bit " + base();
    }

    std::string base() const override { return "Exact Dual Streak Metric"; }

    double operator()(const query_t& a, const tag_t& b) const override {
      return calculate(a, b);
    }

    inline static double calculate(const query_t& a, const tag_t& b) {
      const emp::BitSet<Width> bs = a^b;
      const size_t same = (~bs).LongestSegmentOnes();
      const size_t different = bs.LongestSegmentOnes();
      const double ps = GetDistn().GetStreakProbability(same);
      const double pd = GetDistn().GetStreakProbability(different);

      const double match = pd / (ps + pd);
      // Note: here, close match score > poor match score
      // However, we're computing distance where smaller means closer match.
      // Note also: 0.0 < match < 1.0
      // So, we subtract match score from 1.0 to get a distance.
      return 1.0 - match;
    }

  };

  /// Matches based on longest streak of equal bits in two bitsets.
  /// This implementation uses Corect Math adapted from
  /// https://www.askamathematician.com/2010/07/q-whats-the-chance-of-getting-a-run-of-k-successes-in-n-bernoulli-trials-why-use-approximations-when-the-exact-answer-is-known/
  /// This metric is uniformly distributed.
  /// Adapted from Downing, Keith L. Intelligence emerging: adaptivity and search in evolving neural systems. MIT Press, 2015.
  template<size_t Width>
  struct ExactSingleStreakMetric : public BaseMetric<
    emp::BitSet<Width>,
    emp::BitSet<Width>
  > {

    using query_t = emp::BitSet<Width>;
    using tag_t = emp::BitSet<Width>;

    static const ExactStreakDistribution<Width>& GetDistn() {
      return ExactStreakDistribution_ConstructOnFirstUse<Width>();
    };

    size_t dim() const override { return 1; }

    size_t width() const override { return Width; }

    std::string name() const override {
      return emp::to_string(Width) + "-bit " + base();
    }

    std::string base() const override { return "Streak Metric"; }

    double operator()(const query_t& a, const tag_t& b) const override {
      return calculate(a, b);
    }

    inline static double calculate(const query_t& a, const tag_t& b) {
      const size_t same = (a.EQU(b)).LongestSegmentOnes();
      // sampling from probabilty distribution
      // and then viewing location in cumulative probability distribution
      // gives us a uniform result
      const double p_same = GetDistn().GetStreakProbability(same);

      return p_same;
    }

  };

  template<size_t Width>
  struct CodonMetric : public BaseMetric<
    emp::BitSet<Width>,
    emp::BitSet<Width>
  > {

    using query_t = emp::BitSet<Width>;
    using tag_t = emp::BitSet<Width>;

    size_t dim() const override { return 1; }

    size_t width() const override { return Width; }

    std::string name() const override {
      return emp::to_string(Width) + "-bit " + base();
    }

    std::string base() const override { return "Codon Metric"; }

    inline static double calculate(const query_t& a, const tag_t& b) {

      // codon code
      emp::Random rand_(1);
      const emp::BitSet<Width> codon_code(rand_);

      emp::vector<size_t> query_codon_idxs;
      emp::vector<size_t> tag_codon_idxs;
      emp::BitSet<Width> mask;
      mask.SetUInt(0, 15);
      for (size_t i{}; i < Width; i += 4) {
        if ((mask & a) == (mask & codon_code)) query_codon_idxs.push_back(i);
        if ((mask & b) == (mask & codon_code)) tag_codon_idxs.push_back(i);
        const auto temp = mask;
        for (size_t j{}; j < 15; ++j) {
          mask += temp;
        }
      }

      emp::vector<emp::BitSet<Width>> query_genes;
      query_genes.push_back(a);
      for (const size_t i : query_codon_idxs) {
        query_genes.push_back(a.ROTATE(i + 4));
      }

      emp::vector<emp::BitSet<Width>> tag_genes;
      tag_genes.push_back(b);
      for (const size_t i : tag_codon_idxs) {
        tag_genes.push_back(b.ROTATE(i + 4));
      }

      emp::vector<size_t> overlaps;
      for (const auto& query_gene : query_genes) {
        for (const auto& tag_gene : tag_genes) {
          overlaps.push_back(
            (query_gene ^ tag_gene).FindBit()
          );
        }
      }

      if (overlaps.size()) return 1.0 / static_cast<double>(
        *std::max_element(std::begin(overlaps), std::end(overlaps)) + 2
      );
      else return 1.0;
    }

    double operator()(const query_t& a, const tag_t& b) const override {
      return calculate(a, b);
    }

  };

  /// Matches based on longest streaks of equal and unequal bits in two bitsets.
  /// Adapted from Downing, Keith L. Intelligence emerging: adaptivity and search in evolving neural systems. MIT Press, 2015.
  template<size_t Width>
  using StreakMetric = ApproxDualStreakMetric<Width>;

  template<typename Metric, size_t MaxCapacity=100000>
  struct CacheMod : public Metric {

    using query_t = typename Metric::query_t;
    using tag_t = typename Metric::tag_t;

    Metric metric;

    static inline robin_hood::unordered_map<
      std::tuple<query_t, tag_t>,
      double,
      emp::TupleHash<query_t, tag_t>
    > cache;
    static inline std::queue<std::tuple<query_t, tag_t>> purge_queue;

    std::string name() const override { return metric.name(); }

    inline static double calculate(const query_t& a, const tag_t& b) {

      if (cache.find({a, b}) == std::end(cache)) {
        // make space if needed
        if (cache.size() >= MaxCapacity) {
          cache.erase(purge_queue.front());
          purge_queue.pop();
        }

        cache[{a, b}] = Metric::calculate(a, b);
        purge_queue.push({a, b});
      }

      return cache.at({a, b});

    }

    double operator()(const query_t& a, const tag_t& b) const override {
      return calculate(a, b);
    }

  };

  template<typename Metric>
  struct SlideMod : public Metric {

    using query_t = typename Metric::query_t;
    using tag_t = typename Metric::tag_t;

    inline static size_t metric_width{ Metric{}.width() };

    inline static double calculate(const query_t& a, const tag_t& b) {

      query_t dup(a);

      double best = 1.0;

      for (size_t i = 0; i < metric_width; ++ i) {
        best = std::min(Metric::calculate(dup, b), best);
        dup.template ROTL_SELF<1>();
      }

      return best;
    }

    double operator()(const query_t& a, const tag_t& b) const override {
      return calculate(a, b);
    }

  };

  template<typename Metric>
  struct HardStartMod : public Metric {

    using query_t = typename Metric::query_t;
    using tag_t = typename Metric::tag_t;

    Metric metric;

    std::string name() const override { return "Hard Started " + metric.name(); }

    inline static double calculate(const query_t& a, const tag_t& b) {

      const double res = Metric::calculate(a, b);

      if (a[0] == b[0]) {
        return res;
      } else {
        return 1.0;
      }

    }

    double operator()(const query_t& a, const tag_t& b) const override {
      return calculate(a, b);
    }

  };

  template<typename Metric>
  struct AntiMod : public Metric {

    using query_t = typename Metric::query_t;
    using tag_t = typename Metric::tag_t;

    Metric metric;

    inline static double calculate(const query_t& a, const tag_t& b) {
      return 1.0 - Metric::calculate(a,b);
    }

    double operator()(const query_t& a, const tag_t& b) const override {
      return calculate(a, b);
    }

  };

  // Root greater than one squishes probability distribution to the center
  // Root less than one flattens out probability distribution
  // Root must be positive
  template <typename Metric, typename Root>
  struct PowMod : public Metric {

    using query_t = typename Metric::query_t;
    using tag_t = typename Metric::query_t;

    Metric metric;

    std::string name() const override {
      return (
        emp::to_string(Root::num) + "/" + emp::to_string(Root::den)
        + " Power " + metric.name()
      );
    }

    inline static double calculate(const query_t& a, const tag_t& b) {
      constexpr double exp = static_cast<double>(Root::num) / Root::den;
      emp_assert(exp > 0);
      if constexpr (exp == 1.0) return Metric::calculate(a,b);

      const double base = -1.0 + 2.0 * Metric::calculate(a,b);
      return 0.5 * (1.0 + std::copysign(std::pow(std::abs(base), exp), base));
    }

    double operator()(const query_t& a, const tag_t& b) const override {
      return calculate(a, b);
    }

  };

  // Base less than one squishes probability distribution to the center
  // Base greater than one flattens out probability distribution
  // Base must be positive
  template <typename Metric, typename Base>
  struct LogMod : public Metric {

    using query_t = typename Metric::query_t;
    using tag_t = typename Metric::query_t;

    Metric metric;

    std::string name() const override {
      return (
        emp::to_string(Base::num) + "/" + emp::to_string(Base::den)
        + " Logarithm " + metric.name()
      );
    }

    inline static double calculate(const query_t& a, const tag_t& b) {
      constexpr double base = static_cast<double>(Base::num) / Base::den;
      emp_assert(base > 0);
      if constexpr (base == 1.0) return Metric::calculate(a,b);


      const double raw = Metric::calculate(a,b) - 0.5;
      const double antilog = (
        (2 - base)
        + 2 * (base - 1) * (std::abs(raw) + 0.5)
      );
      return 0.5 * (
        1.0 + std::copysign(emp::Log(antilog, base), raw)
      );
    }

    double operator()(const query_t& a, const tag_t& b) const override {
      return calculate(a, b);
    }

  };

  #ifndef DOXYGEN_SHOULD_SKIP_THIS
  namespace internal {

    // accessory to UnifMod
    template <typename Metric, size_t Samples>
    class EstimatedLookupTable {

      using query_t = typename Metric::query_t;
      using tag_t = typename Metric::query_t;

      // pairs of (raw, uniformified)
      emp::vector<std::pair<double, double>> table;

    public:
      EstimatedLookupTable(const Metric & metric) {
        emp::Random rand(1);

        emp::vector<double> raw;
        emp::vector<double> uniformified;

        // true for all metrics
        raw.push_back(0.0);
        uniformified.push_back(0.0);

        // add data
        for (size_t i = 0; i < Samples; ++i) {
          raw.push_back(metric(query_t(rand), tag_t(rand)));
          uniformified.push_back(
            // {a} -> {0.5}
            // {a,b} -> {0.333, 0.667}
            // {a,b,c} -> {0.25, 0.5, 0.75}
            // etc.
            static_cast<double>(i+1)/static_cast<double>(Samples+1)
          );
        }

        // true for all metrics
        raw.push_back(1.0);
        uniformified.push_back(1.0);

        std::sort(std::begin(raw), std::end(raw));

        // zip together
        emp::vector<std::pair<double, double>> observations;
        std::transform(
          std::begin(raw), std::end(raw),
          std::begin(uniformified),
          std::back_inserter(observations),
          [](double r, double u){ return std::pair{r, u}; }
        );

        // keep only most extreme duplicate observations
        // e.g., if three or more keep bookend two
        for (
          auto front = std::begin(observations);
          front != std::end(observations);
          front = std::upper_bound(
            front,
            std::end(observations),
            *front,
            [](const auto &a, const auto &b){ return a.first < b.first; }
          )
        ) {

          const auto back = std::upper_bound(
            front,
            std::end(observations),
            *front,
            [](const auto &a, const auto &b){ return a.first < b.first; }
          );
          table.push_back(*front);
          if (
            front != std::prev(back)
          ) table.push_back(*std::prev(back));

        }

      }

      // lookup a raw score to get a uniformified
      double operator()(const double raw) const {

        emp_assert(raw >= 0.0 && raw <= 1.0);

        // handle edge cases
        if (raw == 0.0 || raw == 1.0) return raw;

        // goal: get two exact bookends if we have EXACTLY the raw value
        // stored in the table
        // otherwise, get the less than and greater than value

        // Returns an iterator pointing to the first element in the range
        // [first, last) that is not less than (i.e. greater or equal to)
        // value, or last if no such element is found.
        auto tail = std::lower_bound(
          std::begin(table),
          std::end(table),
          // lexographic comparison with operator<
          // find the first element that is greater or equal to
          std::pair{raw, std::nan("")},
          [](const auto &a, const auto &b){ return a.first < b.first; }
        );

        // if we have a good bookend situation
        if (
          tail->first == raw
          && std::next(tail) != std::end(table)
          && tail->first == std::next(tail)->first
        ) ++tail;

        // we handled edge cases above and added (0.0, 0.0)
        // so tail won't ever be std::begin(table)
        emp_assert(std::begin(table) != tail);
        const auto head = std::prev(tail);

        // linear interpolation time
        const double raw_frac = (tail->first == head->first) ? 0.5 : (
          (raw - head->first)
          / (tail->first - head->first)
        );
        return raw_frac * (tail->second - head->second) + head->second;

      }

    };

  }

  namespace internal {
    template <typename Metric, size_t Samples>
    struct lookup_holder {
      const Metric metric;
      const internal::EstimatedLookupTable<
        Metric,
        Samples
      > lookup;
      lookup_holder() : metric(), lookup(metric) { ; }
    };
  }
  #endif // DOXYGEN_SHOULD_SKIP_THIS

  /// Reshape metric's probability distribution to be approximately uniform.
  /// Sample from the original distribution to create a percentile map,
  /// and then, at runtime, put raw matches through the percentile map to
  /// approximate a uniform distribution.
  template <typename Metric, size_t Samples=10000>
  struct UnifMod : public Metric {

    using query_t = typename Metric::query_t;
    using tag_t = typename Metric::query_t;

    inline const static internal::lookup_holder<Metric, Samples> held{};

    std::string name() const override {
      return emp::to_string("Uniformified ", held.metric.name());
    }

    inline static double calculate(const query_t& a, const tag_t& b) {
      return held.lookup(held.metric(a, b));
    }

    double operator()(const query_t& a, const tag_t& b) const override {
      return calculate(a, b);
    }

  };

  template<typename Metric, size_t Dim>
  struct MeanDimMod
    : public BaseMetric<
      std::array<typename Metric::query_t, Dim>,
      std::array<typename Metric::tag_t, Dim>
    >
  {

    using query_t = std::array<typename Metric::query_t, Dim>;
    using tag_t = std::array<typename Metric::tag_t, Dim>;

    Metric metric;

    size_t width() const override { return Dim * metric.width(); }

    size_t dim() const override { return Dim; }

    std::string name() const override {
      return emp::to_string(Dim) + "-Dimensional Mean " + metric.name();
    }

    std::string base() const override { return metric.base(); }

    double operator()(const query_t& a, const tag_t& b) const override {
      return calculate(a, b);
    }

    inline static double calculate(const query_t& a, const tag_t& b) {

      double res = 0.0;
      for (size_t d = 0; d < Dim; ++d) res += Metric::calculate(a[d], b[d]);

      return res / (double)Dim;
    }

  };

  template<typename Metric, size_t Dim>
  struct EuclideanDimMod
    : public BaseMetric<
      std::array<typename Metric::query_t, Dim>,
      std::array<typename Metric::tag_t, Dim>
    >
  {

    using query_t = std::array<typename Metric::query_t, Dim>;
    using tag_t = std::array<typename Metric::tag_t, Dim>;

    Metric metric;

    size_t width() const override { return Dim * metric.width(); }

    size_t dim() const override { return Dim; }

    std::string name() const override {
      return emp::to_string(Dim) + "-Dimensional Euclidean " + metric.name();
    }

    std::string base() const override { return metric.base(); }

    double operator()(const query_t& a, const tag_t& b) const override {
      return calculate(a, b);
    }

    inline static double calculate(const query_t& a, const tag_t& b) {

      double res = 0.0;
      for (size_t d = 0; d < Dim; ++d) {
        const double amt = Metric::calculate(a[d], b[d]);
        res += amt * amt;
      }

      return std::sqrt(res / Dim);
    }

  };

  template<typename Metric, size_t Dim>
  struct MinDimMod
    : public BaseMetric<
      std::array<typename Metric::query_t, Dim>,
      std::array<typename Metric::tag_t, Dim>
    >
  {

    using query_t = std::array<typename Metric::query_t, Dim>;
    using tag_t = std::array<typename Metric::tag_t, Dim>;

    Metric metric;

    size_t width() const override { return Dim * metric.width(); }

    size_t dim() const override { return Dim; }

    std::string name() const override {
      return emp::to_string(Dim) + "-Dimensional Minimum " + metric.name();
    }

    std::string base() const override { return metric.base(); }

    double operator()(const query_t& a, const tag_t& b) const override {
      return calculate(a, b);
    }

    inline static double calculate(const query_t& a, const tag_t& b) {

      double res = 1.0;
      for (size_t d = 0; d < Dim; ++d) res = std::min(res, Metric::calculate(a[d], b[d]));

      return res;
    }

  };

  template<typename Metric, size_t Dim>
  struct HarmonicDimMod
    : public BaseMetric<
      std::array<typename Metric::query_t, Dim>,
      std::array<typename Metric::tag_t, Dim>
    >
  {

    using query_t = std::array<typename Metric::query_t, Dim>;
    using tag_t = std::array<typename Metric::tag_t, Dim>;

    Metric metric;

    size_t width() const override { return Dim * metric.width(); }

    size_t dim() const override { return Dim; }

    std::string name() const override {
      return emp::to_string(Dim) + "-Dimensional Harmonic " + metric.name();
    }

    std::string base() const override { return metric.base(); }

    double operator()(const query_t& a, const tag_t& b) const override {
      return calculate(a, b);
    }

    inline static double calculate(const query_t& a, const tag_t& b) {

      double res = 0.0;

      for (size_t d = 0; d < Dim; ++d) {
        res += 1 / Metric::metric(a[d], b[d]);
      }

      return static_cast<double>(Dim) / res;
    }

  };

  template<typename DimMetric>
  struct FlatMod
    : public BaseMetric<
      emp::BitSet<
        std::tuple_size<typename DimMetric::query_t>::value
        * DimMetric::query_t::value_type::GetSize()
      >,
      emp::BitSet<
        std::tuple_size<typename DimMetric::tag_t>::value
        * DimMetric::tag_t::value_type::GetSize()
      >
    >
  {

    using query_t = emp::BitSet<
      std::tuple_size<typename DimMetric::query_t>::value
      * DimMetric::query_t::value_type::GetSize()
    >;
    using tag_t = emp::BitSet<
      std::tuple_size<typename DimMetric::query_t>::value
      * DimMetric::tag_t::value_type::GetSize()
    >;

    inline static DimMetric metric{};

    size_t width() const override { return metric.width(); }

    size_t dim() const override { return metric.dim(); }

    std::string name() const override { return metric.name(); }

    std::string base() const override { return metric.base(); }

    double operator()(const query_t& a, const tag_t& b) const override {
      return calculate(a, b);
    }

    inline static double calculate(const query_t& a, const tag_t& b) {

      typename DimMetric::query_t arr_a;
      typename DimMetric::tag_t arr_b;

      for (size_t d = 0; d < metric.dim(); ++d) {
        arr_a[d].Import(a, d * DimMetric::query_t::value_type::GetSize());
        arr_b[d].Import(b, d * DimMetric::tag_t::value_type::GetSize());
      }

      return DimMetric::calculate(arr_a, arr_b);

    }

  };

}

#endif
