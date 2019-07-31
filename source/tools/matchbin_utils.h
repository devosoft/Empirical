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
#include <string>
#include <tuple>
#include <array>

#include "../base/assert.h"
#include "../base/array.h"
#include "../base/vector.h"
#include "../tools/math.h"
#include "../tools/IndexMap.h"
#include "../tools/BitSet.h"
#include "../tools/string_utils.h"
#include "../tools/hash_utils.h"

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
      std::hash<query_t> qhasher;
      std::hash<tag_t> thasher;

      return static_cast<double>(emp::hash_combine(
        qhasher(a),
        thasher(b)
      )) / std::numeric_limits<size_t>::max();
    }

  };

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
      const size_t difference = ((Max + 1) + b - a) % (Max + 1);
      return (double)(difference % (Max + 1)) / Max;
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
      return (
        a > b ? a - b : b - a
      ).GetDouble() /  emp::BitSet<Width>::MaxDouble();
    }

  };

  /// Matches based on the longest segment of equal and uneqal bits in two bitsets
  /// Adapted from Downing, Keith L. Intelligence emerging: adaptivity and search in evolving neural systems. MIT Press, 2015.
  template<size_t Width>
  struct StreakMetric : public BaseMetric<emp::BitSet<Width>, emp::BitSet<Width>> {

    using query_t = emp::BitSet<Width>;
    using tag_t = emp::BitSet<Width>;

    size_t dim() const override { return 1; }

    size_t width() const override { return Width; }

    std::string name() const override {
      return emp::to_string(Width) + "-bit " + base();
    }

    std::string base() const override { return "Streak Metric"; }

    double operator()(const query_t& a, const tag_t& b) const override {
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
  struct SlideMod : public Metric {

    using query_t = typename Metric::query_t;
    using tag_t = typename Metric::tag_t;

    Metric metric;

    std::string name() const override { return "Sliding " + metric.name(); }

    double operator()(const query_t& a, const tag_t& b) const override {

      query_t dup(a);

      double best = 1.0;

      for(size_t i = 0; i < metric.width(); ++ i) {
        best = std::min(metric(dup, b), best);
        dup.template ROTL_SELF<1>();
      }

      return best;
    }
  };

  template<typename Metric>
  struct AntiMod : public Metric {

    using query_t = typename Metric::query_t;
    using tag_t = typename Metric::tag_t;

    Metric metric;

    std::string name() const override { return "Inverse " + metric.name(); }

    double operator()(const query_t& a, const tag_t& b) const override {
      return 1.0 - metric(a,b);
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

    double operator()(const query_t& a, const tag_t& b) const override {
      constexpr double exp = static_cast<double>(Root::num) / Root::den;
      emp_assert(exp > 0);
      if constexpr (exp == 1.0) return metric(a,b);

      const double base = -1.0 + 2.0 * metric(a,b);
      return 0.5 * (1.0 + std::copysign(std::pow(std::abs(base), exp), base));
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

    double operator()(const query_t& a, const tag_t& b) const override {
      constexpr double base = static_cast<double>(Base::num) / Base::den;
      emp_assert(base > 0);
      if constexpr (base == 1.0) return metric(a,b);


      const double raw = metric(a,b) - 0.5;
      const double antilog = (
        (2 - base)
        + 2 * (base - 1) * (std::abs(raw) + 0.5)
      );
      return 0.5 * (
        1.0 + std::copysign(emp::Log(antilog, base), raw)
      );
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

      double res = 0.0;
      for (size_t d = 0; d < Dim; ++d) res += metric(a[d], b[d]);

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

      double res = 0.0;
      for (size_t d = 0; d < Dim; ++d) {
        const double amt = metric(a[d], b[d]);
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

      double res = 1.0;
      for (size_t d = 0; d < Dim; ++d) res = std::min(res, metric(a[d], b[d]));

      return res;
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

    DimMetric metric;

    size_t width() const override { return metric.width(); }

    size_t dim() const override { return metric.dim(); }

    std::string name() const override { return metric.name(); }

    std::string base() const override { return metric.base(); }

    double operator()(const query_t& a, const tag_t& b) const override {

      typename DimMetric::query_t arr_a;
      typename DimMetric::tag_t arr_b;

      for (size_t d = 0; d < dim(); ++d) {
        arr_a[d].Import(a, d * DimMetric::query_t::value_type::GetSize());
        arr_b[d].Import(b, d * DimMetric::tag_t::value_type::GetSize());
      }

      return metric(arr_a, arr_b);

    }

  };


  struct CacheStateBase{
    CacheStateBase() = default;
    virtual std::optional<emp::vector<size_t>> operator()(size_t n) = 0;
  };

  struct RouletteCacheState : public CacheStateBase{
    RouletteCacheState() = default;
    RouletteCacheState(emp::IndexMap& im, emp::vector<size_t>& ids, emp::Random& r)
      : indexMap(im)
      , uids(ids)
      , rand(r){}
    std::optional<emp::vector<size_t>> operator()(size_t n){
      emp::vector<size_t> res;
      res.reserve(n);

      for (size_t j = 0; j < n; ++j) {
        const double match_pos = rand.GetDouble(indexMap.GetWeight());
        const size_t idx = indexMap.Index(match_pos);
        res.push_back(uids[idx]);
      }
      return res;
    }

    emp::IndexMap indexMap;
    emp::vector<size_t> uids;
    emp::Random rand;
  };

  struct RankedCacheState: public CacheStateBase{

    RankedCacheState() = default;
    RankedCacheState(emp::vector<size_t>::iterator begin, size_t back, size_t n):
        uids(emp::vector<size_t>(begin, begin + back))
      , requestSize(n){}

    std::optional<emp::vector<size_t>> operator()(size_t n){
      if (n > requestSize){ return std::nullopt; }
      if (n >= uids.size()){ return uids; }
      return emp::vector<size_t>(uids.begin(), uids.begin()+n);
    }

    emp::vector<size_t> uids;
    size_t requestSize;
  };

  /// Abstract base class for selectors
  template<typename CacheType>
  struct SelectorBase{
    virtual ~SelectorBase() {};
    virtual CacheType operator()(
        emp::vector<size_t>& uids,
        std::unordered_map<size_t, double>& scores,
        size_t n
        ) = 0;
  };

  /// Returns matches within the threshold ThreshRatio sorted by match quality.
  template<typename ThreshRatio = std::ratio<-1,1>> // neg numerator means +infy
  struct RankedSelector : public SelectorBase<RankedCacheState> {

    RankedSelector(emp::Random&){ ; }

    using cache_state_type_t = RankedCacheState;

    RankedCacheState operator()(
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
          std::swap(uids.at(back), uids.at(minIndex));
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

      return RankedCacheState(uids.begin(), back, n);
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
  struct RouletteSelector : public SelectorBase<RouletteCacheState> {

    using cache_state_type_t = RouletteCacheState;

    emp::Random & rand;

    RouletteSelector(emp::Random & rand_)
    : rand(rand_)
    { ; }

    RouletteCacheState operator()(
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

      return RouletteCacheState(match_index, uids, rand);

      /*
      emp::vector<size_t> res;
      res.reserve(n);

      for (size_t j = 0; j < n; ++j) {
        const double match_pos = rand.GetDouble(match_index.GetWeight());
        const size_t idx = match_index.Index(match_pos);
        res.push_back(uids[idx]);
      }

      return res;*/
    }

};
/*
struct DynamicSelector : public SelectorBase<emp::vector<size_t>>{

  using cache_state_type_t = emp::vector<size_t>;

  emp::vector<emp::Ptr<SelectorBase>> selectors;
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
*/

}

#endif
