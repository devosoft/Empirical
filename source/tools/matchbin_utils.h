/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file matchbin_utils.h
 *  @brief Metric and Selector structs that can be plugged into MatchBin.
 *
 */


#ifndef EMP_MATCH_BIN_UTILS_H
#define EMP_MATCH_BIN_UTILS_H

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <ratio>
#include <math.h>

#include "../base/assert.h"
#include "../base/array.h"
#include "../base/vector.h"
#include "../tools/IndexMap.h"
#include "../tools/BitSet.h"

namespace emp {

  /// Metric for MatchBin stored in the struct so we can template on it
  /// Returns the number of bits not in common between two BitSets
  template<size_t Width>
  struct HammingMetric {

    using tag_t = emp::BitSet<Width>;
    using query_t = emp::BitSet<Width>;

    static constexpr size_t width = Width;

    double operator()(const query_t& a, const tag_t& b) const{
      return (double)(a^b).CountOnes() / Width;
    }
  };

  /// Metric gives the absolute difference between two integers
  struct AbsDiffMetric {

    using tag_t = int;
    using query_t = int;

    double operator()(const query_t& a, const tag_t& b) const {

      return (double)std::abs(a-b) / std::numeric_limits<int>::max();
    }
  };

  /// Metric gives the matchings by the closest tag on or above itself.
  /// Wraps on Max.
  /// Adapted from Spector, Lee, et al. "Tag-based modules in genetic programming." Proceedings of the 13th annual conference on Genetic and evolutionary computation. ACM, 2011.
  template<size_t Max=1000>
  struct NextUpMetric {

    using tag_t = size_t;
    using query_t = size_t;

    static constexpr size_t width = sizeof(size_t) * 8;

    double operator()(const query_t& a, const tag_t& b) const {
      const size_t difference = ((Max + 1) + b - a) % (Max + 1);
      return (double)(difference % (Max + 1)) / Max;
    }
  };

  /// BitSet-based implementation of NextUpMetric.
  /// Adapted from Spector, Lee, et al. "Tag-based modules in genetic programming." Proceedings of the 13th annual conference on Genetic and evolutionary computation. ACM, 2011.
  template<size_t Width>
  struct AsymmetricWrapMetric {

    using tag_t = emp::BitSet<Width>;
    using query_t = emp::BitSet<Width>;

    static constexpr size_t width = Width;

    double operator()(const query_t& a, const tag_t& b) const {
      return (b - a).GetDouble() / emp::BitSet<Width>::MaxDouble();
    }

  };

  /// BitSet-based implementation of NextUpMetric without wrapping.
  template<size_t Width>
  struct AsymmetricNoWrapMetric {

    using tag_t = emp::BitSet<Width>;
    using query_t = emp::BitSet<Width>;

    static constexpr size_t width = Width;

    double operator()(const query_t& a, const tag_t& b) const {
      constexpr double max_dist = emp::BitSet<Width>::MaxDouble() + 1.0;
      return (b >= a ? (b - a).GetDouble() : max_dist) / max_dist;
    }

  };

  /// Metric gives the absolute value of the difference between the integer
  /// representations of the BitSets with wrap from zero to the maximum value
  /// the BitSet can represent.
  /// Adapted from Downing, Keith L. Intelligence emerging: adaptivity and search in evolving neural systems. MIT Press, 2015.
  template<size_t Width>
  struct SymmetricWrapMetric {

    using tag_t = emp::BitSet<Width>;
    using query_t = emp::BitSet<Width>;

    static constexpr size_t width = Width;

    double operator()(const query_t& a, const tag_t& b) {
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
  struct SymmetricNoWrapMetric {

    using tag_t = emp::BitSet<Width>;
    using query_t = emp::BitSet<Width>;

    static constexpr size_t width = Width;

    double operator()(const query_t& a, const tag_t& b) {
      return (
        a > b ? a - b : b - a
      ).GetDouble() /  emp::BitSet<Width>::MaxDouble();
    }

  };

  /// Matches based on the longest segment of equal and uneqal bits in two bitsets
  /// Adapted from Downing, Keith L. Intelligence emerging: adaptivity and search in evolving neural systems. MIT Press, 2015.
  template<size_t Width>
  struct StreakMetric {

    using tag_t = emp::BitSet<Width>;
    using query_t = emp::BitSet<Width>;

    static constexpr size_t width = Width;

    double operator()(const query_t& a, const tag_t& b) const {
      const emp::BitSet<Width> bs = a^b;
      const size_t same = (~bs).LongestSegmentOnes();
      const size_t different = bs.LongestSegmentOnes();
      const double ps = ProbabilityKBitSequence(same);
      const double pd = ProbabilityKBitSequence(different);

      const double match = (pd / (ps + pd));
      // Note: here, close match score > poor match score
      // However, we're computing distance where smaller means closer match.
      // Note also: 0.0 < match < 1.0
      // So, we subtract match score from 1.0 to get a distance.
      return 1.0 - match;
    }

    inline double ProbabilityKBitSequence(size_t k) const {
      return (Width - k + 1) / std::pow(2, k);
    }
  };

  template<typename Metric>
  struct SlideMod {

    using tag_t = typename Metric::tag_t;
    using query_t = typename Metric::query_t;

    static constexpr size_t width = Metric::width;

    Metric metric;

    double operator()(query_t& a, const tag_t& b) {

      emp_assert(&a != &b);

      double best = 1.0;

      for(size_t i = 0; i < Metric::width; ++ i) {
        best = std::min(metric(a, b), best);
        a.template ROTL_SELF<1>();
      }

      return best;
    }
  };

  template<typename Metric>
  struct AntiMod {

    using tag_t = typename Metric::tag_t;
    using query_t = typename Metric::query_t;

    static constexpr size_t width = Metric::width;

    Metric metric;

    double operator()(const query_t& a, const tag_t& b) { return 1.0 - metric(a,b); }

  };

  template<typename Metric, size_t Dim>
  struct DimMod {

    using tag_t = emp::array<typename Metric::tag_t, Dim>;
    using query_t = emp::array<typename Metric::query_t, Dim>;

    static constexpr size_t width = Dim * Metric::width;

    Metric metric;

    double operator()(query_t& a, const tag_t& b) {

      double res = 0.0;
      for (size_t d = 0; d < Dim; ++d) res += metric(a[d], b[d]);

      return res / (double)Dim;
    }

  };

  /// Abstract base class for selectors
  struct Selector {

    virtual ~Selector() {};
    virtual emp::vector<size_t> operator()(
      emp::vector<size_t>& uids,
      std::unordered_map<size_t, double>& scores,
      size_t n
    ) = 0;
  };

  /// Returns matches within the threshold ThreshRatio sorted by match quality.
  template<typename ThreshRatio = std::ratio<-1,1>> // neg numerator means +infy
  struct RankedSelector : public Selector {
    emp::vector<size_t> operator()(
      emp::vector<size_t>& uids,
      std::unordered_map<size_t, double>& scores,
      size_t n
    ){

      size_t back = 0;

      // treat any negative numerator as positive infinity
      const double thresh = (
        ThreshRatio::num < 0
        ? std::numeric_limits<double>::infinity()
        : ((double) ThreshRatio::num) / ((double)ThreshRatio::den)
      );


      if (n < std::log2(uids.size())) {
        // Perform a bounded selection sort to find the first n results
        for (; back < n; ++back) {
          int minIndex = -1;
          for (size_t j = back; j < uids.size(); ++j) {
            if (
              (minIndex == -1 || scores.at(uids[j]) < scores.at(uids[minIndex]))
              && (scores.at(uids[j]) <= thresh)
            ) {
              minIndex = j;
            }
          }
          if (minIndex == -1) break;
          std::swap(uids.at(back),uids.at(minIndex));
        }

      } else {

        std::sort(
          uids.begin(),
          uids.end(),
          [&scores](const size_t &a, const size_t &b) {
            return scores.at(a) < scores.at(b);
          }
        );

        while (
          back < uids.size()
          && back < n
          && scores.at(uids[back]) <= thresh
        ) ++back;

      }

      return emp::vector<size_t>(uids.begin(), uids.begin() + back);
    }
  };

  /// Selector chooses probabilistically based on match quality with replacement.
  /// ThreshRatio: what is the raw maximum score to even be considered to match
  /// SkewRatio: how much more heavily should the best matches be weighted
  /// in terms of match probability; must be greater than 0
  /// (close to zero: very heavily, large: mostly even weighting)
  /// MaxBaselineRatio: maximum score that all scores will be normalized to
  /// baseline = min(min_score, MaxBaselineRatio)
  /// normalized_score = score - baseline
  /// ...
  ///  overall, p_match ~ 1 / (skew + score - baseline)
  template<
    typename ThreshRatio = std::ratio<-1, 1>,// we treat neg numerator as +infty
    typename SkewRatio = std::ratio<1, 10>,
    typename MaxBaselineRatio = std::ratio<1, 1>// treat neg numerator as +infty
  >
  struct RouletteSelector : public Selector {

    emp::Random & rand;

    RouletteSelector(emp::Random & rand_)
    : rand(rand_)
    { ; }

    emp::vector<size_t> operator()(
      emp::vector<size_t>& uids,
      std::unordered_map<size_t, double>& scores,
      size_t n
    ) {

      const double skew = ((double) SkewRatio::num / SkewRatio::den);
      emp_assert(skew > 0);

      // treat any negative numerator as positive infinity
      const double thresh = (
        ThreshRatio::num < 0
        ? std::numeric_limits<double>::infinity()
        : ((double) ThreshRatio::num) / ThreshRatio::den
      );

      // treat any negative numerator as positive infinity
      const double max_baseline = (
        MaxBaselineRatio::num < 0
        ? std::numeric_limits<double>::infinity()
        : ((double) MaxBaselineRatio::num) / MaxBaselineRatio::den
      );

      // partition by thresh
      size_t partition = 0;
      double min_score = std::numeric_limits<double>::infinity();
      for (size_t i = 0; i < uids.size(); ++i) {
        emp_assert(scores[uids[i]] >= 0);
        min_score = std::min(min_score, scores[uids[i]]);
        if (scores[uids[i]] <= thresh) {
          std::swap(uids[i], uids[partition++]);
        }
      }

      // skew relative to strongest match less than or equal to max_baseline
      // to take into account regulation...
      // (the default value of max_baseline is 1.0 because without
      // upregulation, the best possible match score is 1.0)
      const double baseline = std::min(min_score, max_baseline);
      emp_assert(baseline >= 0);
      emp_assert(baseline <= max_baseline);

      IndexMap match_index(partition);

      for (size_t p = 0; p < partition; ++p) {
        emp_assert(scores[uids[p]] - baseline >= 0);
        match_index.Adjust(p, 1.0 / ( skew + scores[uids[p]] - baseline ));
      }

      emp::vector<size_t> res;
      res.reserve(n);

      for (size_t j = 0; j < n; ++j) {
        const double match_pos = rand.GetDouble(match_index.GetWeight());
        const size_t idx = match_index.Index(match_pos);
        res.push_back(uids[idx]);
      }

      return res;
    }

};

struct DynamicSelector : public Selector {

  emp::vector<emp::Ptr<Selector>> selectors;

  size_t mode{0};

  emp::vector<size_t> operator()(
    emp::vector<size_t>& uids,
    std::unordered_map<size_t, double>& scores,
    size_t n
  ) {
    emp_assert(mode < selectors.size());
    return (*selectors[mode])(uids, scores, n);
  }

  ~DynamicSelector() {
    for (auto &ptr : selectors) ptr.Delete();
  }

};

}

#endif
