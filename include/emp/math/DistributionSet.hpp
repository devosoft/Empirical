/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2022 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/math/DistributionSet.hpp
 * @brief Management of pre-calculated distributions with different input values.
 * @note Status: ALPHA
 *
 * A DistributionSet manages a set of pre-calculated distributions.  When input values are
 * provided, the correct distribution is identified, and the associated value is drawn.
 *
 */

#pragma once

#ifndef INCLUDE_EMP_MATH_DISTRIBUTION_SET_HPP_GUARD
#define INCLUDE_EMP_MATH_DISTRIBUTION_SET_HPP_GUARD

#include <tuple>
#include <unordered_map>

#include "../datastructs/tuple_utils.hpp"

#include "Distribution.hpp"

namespace emp {

  /// @param DIST_T Type of distribution being used.
  /// @param Ts Types of parameters to choose the set based on.
  template <typename DIST_T, typename... Ts>
  class DistributionSet {
  private:
    /// Map parameters to pre-calculated distributions.
    unordered_map<std::tuple<Ts...>, DIST_T, emp::TupleHash<Ts...>> dist_map;

  public:
    size_t PickRandom(Random & random, Ts... args) {
      auto arg_tup       = std::make_tuple(args...);  // Build the tuple to use as a key.
      auto [it, success] = dist_map.emplace(arg_tup, DIST_T(args...));
      return it->second.PickRandom(random);
    }
  };

  using BinomialSet         = emp::DistributionSet<emp::Binomial, double, size_t>;
  using NegativeBinomialSet = emp::DistributionSet<emp::NegativeBinomial, double, size_t>;

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_MATH_DISTRIBUTION_SET_HPP_GUARD

// Local settings for Empecable file checker.
// empecable_words: tup
