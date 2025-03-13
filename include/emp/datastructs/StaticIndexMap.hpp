/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2025.
*/
/**
 *  @file
 *  @brief A fast version of IndexMap where weights cannot individually change.
 *  @note Status: BETA
 */

#ifndef EMP_DATASTRUCTS_STATICINDEXMAP_HPP_INCLUDE
#define EMP_DATASTRUCTS_STATICINDEXMAP_HPP_INCLUDE

#include "emp/base/array.hpp"

#include "UnorderedIndexMap.hpp"

namespace emp {

  /// A map of indices with fixed weights. If a random index is selected, the probability of an index being
  /// returned is directly proportional to its weight.

  template <size_t MAX_BINS=1000>
  class StaticIndexMap {
  private:
    double bin_weight = 0.0;        // Weight of each bin.
    double total_bin_weight = 0.0;  // bin_weight * num bins in use.
    emp::array<size_t, MAX_BINS> bins{};
    UnorderedIndexMap index_map;    // Remaining weight, not in bins.
    emp::vector<double> weights;    // List of all original weights.

  public:
    /// Default constructor; set up with no weights.
    StaticIndexMap() { }

    /// Construct an StaticIndexMap where num_items is the maximum number of items that
    /// can be placed into the data structure.  All item weights default to zero.
    StaticIndexMap(const UnorderedIndexMap & in_map) : index_map(in_map) {
      weights.reserve(in_map.GetSize());
      for (size_t i=0; i < in_map.GetSize(); ++i) weights.push_back(in_map.GetWeight(i));
      Optimize();
    }
    /// Construct an StaticIndexMap with a specified initial set of weights.
    StaticIndexMap(const emp::vector<double> & in_weights)
      : index_map(in_weights), weights(in_weights) { Optimize(); }

    StaticIndexMap(const StaticIndexMap &) = default;
    StaticIndexMap(StaticIndexMap &&) = default;
    ~StaticIndexMap() = default;
    StaticIndexMap & operator=(const StaticIndexMap &) = default;
    StaticIndexMap & operator=(StaticIndexMap &&) = default;

    void Reset() {
      bin_weight = 0.0;
      total_bin_weight = 0.0;
    }

    /// Set the whole distribution.
    void Set(const UnorderedIndexMap & in_map) {
      Reset();
      index_map = in_map;
      weights.reserve(in_map.GetSize());
      for (size_t i=0; i < in_map.GetSize(); ++i) weights.push_back(in_map.GetWeight(i));
      Optimize();
    }

    void Set(const emp::vector<double> & in_weights) {
      Reset();
      index_map = in_weights;
      weights = in_weights;
      Optimize();
    }

    /// How many indices are in this map?
    size_t GetSize() const { return index_map.GetSize(); }

    /// Get the total weight of all indices in this map.
    double GetWeight() const { return index_map.GetWeight() + total_bin_weight; }

    /// Get the weight of a specified index in this map.
    double GetWeight(size_t id) const { return weights[id]; }

    /// Determine the ID at the specified index position.
    size_t Index(double index) const {
      emp_assert(index >= 0.0 && index <= GetWeight(), index, GetWeight());

      // Check if we can do a fast lookup of the index in a bin.
      if (index < total_bin_weight) {
        size_t bin_id = static_cast<size_t>(index / bin_weight);
        return bins[bin_id];
      }

      // Not in a bin, look at the remnants in the index map.
      return index_map.Index(index - total_bin_weight);
    }

    /// Calculate how many bins would be used for a given bin weight.
    size_t CalcBinCount(double bin_weight) const {
      size_t bins_found = 0;
      for (double weight : weights) {
        bins_found += static_cast<size_t>(weight/bin_weight);
      }
      return bins_found;
    }

    /// Determine how big bins should be for optimal coverage.
    double OptimizeBinWeight() const {
      double best_coverage = 0.0;
      double best_bin_weight = 0.0;      
      for (size_t div = MAX_BINS; ; ++div) {
        const double bin_weight = GetWeight() / div;                
        const size_t cur_bins = CalcBinCount(bin_weight);
        // std::cout << "div=" << div
        //           << " bin_weight=" << bin_weight
        //           << " cur_bins=" << cur_bins
        //           << " best_cov=" << (best_coverage / GetWeight())
        //           << " best_bin_w=" << best_bin_weight
        //           << std::endl;
        if (cur_bins > MAX_BINS) break;
        const double coverage = cur_bins * bin_weight;
        if (coverage > best_coverage) {
          best_coverage = coverage;
          best_bin_weight = bin_weight;
        }
      }
      // std::cout << "Bin count = " << CalcBinCount(best_bin_weight)
      //           << "; coverage = " << best_coverage
      //           << "; bin_weight = " << best_bin_weight
      //           << std::endl;

      return best_bin_weight;
    }

    /// Scan through the probabilities and bin them as much as possible.
    void Optimize() {
      bin_weight = OptimizeBinWeight();

      size_t num_bins = 0;
      for (size_t id = 0; id < index_map.GetSize(); ++id) {
        const double id_weight = index_map.GetWeight(id);
        size_t id_bins = static_cast<size_t>(id_weight / bin_weight);
        size_t excess_weight = id_weight - id_bins * bin_weight;
        // std::cout << "id " << id << ":"
        //           << " weight=" << id_weight
        //           << " bins=" << id_bins
        //           << std::endl;
        while (id_bins--) bins[num_bins++] = id;
        index_map[id] = excess_weight;
      }
      total_bin_weight = num_bins * bin_weight;
      // std::cout << "Optimized!"
      //           << " num_bins=" << num_bins
      //           << " bin_weight=" << bin_weight
      //           << " total_bin_weight=" << total_bin_weight
      //           << " remaining weight=" << index_map.GetWeight()
      //           << std::endl;
    }

  };
}

#endif // #ifndef EMP_DATASTRUCTS_STATICINDEXMAP_HPP_INCLUDE
