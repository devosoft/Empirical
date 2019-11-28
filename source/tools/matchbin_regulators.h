/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file matchbin_regulators.h
 *  @brief Regulator structs that can be plugged into MatchBin.
 *
 */


#ifndef EMP_MATCH_BIN_REGULATORS_H
#define EMP_MATCH_BIN_REGULATORS_H

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

#include "tools/Binomial.h"

#include "../base/assert.h"
#include "../base/array.h"
#include "../base/vector.h"
#include "../tools/math.h"
#include "../tools/IndexMap.h"
#include "../tools/BitSet.h"
#include "../tools/string_utils.h"
#include "../tools/hash_utils.h"

namespace emp {

  /// Abstract base class for regulators
  template<typename set_t_, typename adj_t_, typename view_t_>
  struct RegulatorBase {

    using set_t = set_t_;
    using adj_t = adj_t_;
    using view_t = view_t_;

    virtual ~RegulatorBase() {};
    virtual bool Set(const set_t & set) = 0;
    virtual bool Adj(const adj_t & set) = 0;
    virtual view_t View() const = 0;
    virtual double operator()(double raw_score) const = 0;
    virtual std::string name() const = 0;

  };


  struct LinearRegulator : RegulatorBase<double, double, double> {

    // >1.0 downregulated
    // 1.0 neutral
    // <1.0 upgregulated
    // must be >=0.0
    double state;

    LinearRegulator() : state(1.0) {}

    /// Apply regulation to a raw match score.
    double operator()(const double raw_score) const override {
      return state * raw_score + state;
    }

    //TODO should we adopt a convention that's consistent across regulators?

    /// A value between zero and one upregulates the item,
    /// a value of exactly one is neutral,
    /// and a value greater than one downregulates the item.
    bool Set(const double & set) override {
      // regulator value must be positive
      // return whether regulator value changed
      // (i.e., we need to purge the cache)
      return std::exchange(state, std::abs(set)) != std::abs(set);
    }

    /// A negative value upregulates the item,
    /// a value of exactly zero is neutral
    /// and a postive value the item.
    bool Adj(const double & amt) override {
      // regulator value must be positive
      state = std::max(0.0, state + amt);
      // return whether regulator value changed
      // (i.e., we need to purge the cache)
      return amt != 0.0;
    }

    /// Return a double representing the state of the regulator.
    double View() const override {
      return state;
    }

    std::string name() const override {
      return "Linear Regulator";
    }

    bool operator!=(const LinearRegulator & other) const {
      return state != other.state;
    }

    template <class Archive>
    void serialize( Archive & ar )
    {
      ar(
        CEREAL_NVP(state)
      );
    }

  };

}

#endif
