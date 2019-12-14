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
    virtual bool Adj(const adj_t & adj) = 0;
    virtual bool Decay(const int steps) = 0;
    virtual const view_t & View() const = 0;
    virtual double operator()(double raw_score) const = 0;
    virtual std::string name() const = 0;

  };

  /// This regulator does nothing!
  /// Useful for control experiments.
  struct NoopRegulator : RegulatorBase<double, double, double> {

    constexpr static double state = 0.0;

    /// Apply regulation to a raw match score.
    double operator()(const double raw_score) const override {
      return raw_score;
    }

    /// No-op set.
    /// Return whether MatchBin should be updated (never).
    bool Set(const double & set) override {
      std::ignore = set;
      return false;
    }

    /// No-op adjustment.
    /// Return whether MatchBin should be updated (never).
    bool Adj(const double & amt) override {
      std::ignore = amt;
      return false;
    }

    /// No-op decay.
    /// Return whether MatchBin should be updated (never).
    bool Decay(const int steps) override {
      std::ignore = steps;
      return false;
    }

    /// Return a default value.
    const double & View() const override { return state; }

    std::string name() const override { return "No-op Regulator"; }

    bool operator!=(const NoopRegulator & other) const { return false; }

    #ifdef CEREAL_NVP
    template <class Archive>
    void serialize( Archive & ar ){ std::ignore = ar; }
    #endif

  };

  /// Don't use this regulator.
  /// It's here so tests don't break.
  struct LegacyRegulator : RegulatorBase<double, double, double> {

    // >1.0 downregulated
    // 1.0 neutral
    // <1.0 upgregulated
    // must be >=0.0
    double state;

    LegacyRegulator() : state(1.0) {}

    /// Apply regulation to a raw match score.
    double operator()(const double raw_score) const override {
      return state * raw_score + state;
    }

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

    /// No-op decay.
    /// Return whether MatchBin should be updated (never).
    bool Decay(const int steps) override {
      std::ignore = steps;
      return false;
    }

    /// Return a double representing the state of the regulator.
    const double & View() const override {
      return state;
    }

    std::string name() const override {
      return "Legacy Regulator";
    }

    bool operator!=(const LegacyRegulator & other) const {
      return state != other.state;
    }

    #ifdef CEREAL_NVP
    template <class Archive>
    void serialize( Archive & ar )
    {
      ar(
        CEREAL_NVP(state)
      );
    }
    #endif

  };

  template <typename Slope=std::ratio<1,10>>
  struct AdditiveCountdownRegulator : RegulatorBase<double, double, double> {

    static constexpr double slope = (
      static_cast<double>(Slope::num) / static_cast<double>(Slope::den)
    );

    // positive = downregulated
    // negative = upregulated
    double state;
    // state | add to match score
    // +inf  | += 1.0
    // ...   | ...
    // 1.0   | += ~slope
    // 0.0   | neutral
    // -1.0  | -= slope
    // ...   | ...
    // -inf  | -= 1.0

    // countdown timer to reseting state
    size_t timer;

    AdditiveCountdownRegulator() : state(0.0), timer(0) {}

    /// Apply regulation to a raw match score.
    /// Returns a value between 0.0 and 1.0
    double operator()(const double raw_score) const override {
      const double res = std::clamp(
        std::tanh(slope * state) + raw_score,
        0.0,
        1.0
      );
      emp_assert(state <= 0 || res > raw_score);
      emp_assert(state >= 0 || res < raw_score);
      emp_assert(res >= 0.0 && res <= 1.0);
      return res;

    }

    /// A positive value downregulates the item,
    /// a value of zero is neutral,
    /// and a negative value upregulates the item.
    bool Set(const double & set) override {
      timer = 1;

      // return whether regulator value changed
      // (i.e., we need to purge the cache)
      return std::exchange(state, set) != set;
    }

    /// A negative value upregulates the item,
    /// a value of exactly zero is neutral
    /// and a postive value downregulates the item.
    bool Adj(const double & amt) override {
      timer = 1;

      state += amt;

      // return whether regulator value changed
      // (i.e., we need to purge the cache)
      return amt != 0.0;
    }

    /// Timer decay.
    /// Return whether MatchBin should be updated
    bool Decay(const int steps) override {
      if (steps < 0) {
        // if reverse decay is requested
        timer += -steps;
      } else {
        // if forward decay is requested
        timer -= std::min(timer, static_cast<size_t>(steps));
      }

      return timer == 0 ? (
        std::exchange(state, 0.0) != 0.0
      ) : false;
    }

    /// Return a double representing the state of the regulator.
    const double & View() const override { return state; }

    std::string name() const override {
      return "Additive Countdown Regulator";
    }

    bool operator!=(const AdditiveCountdownRegulator & other) const {
      return state != other.state || timer != other.timer;
    }

    #ifdef CEREAL_NVP
    template <class Archive>
    void serialize( Archive & ar )
    {
      ar(
        CEREAL_NVP(state),
        CEREAL_NVP(timer)
      );
    }
    #endif

  };

  template <typename Slope=std::ratio<1,10>>
  struct MultiplicativeCountdownRegulator : RegulatorBase<double, double, double> {

    static constexpr double slope = (
      static_cast<double>(Slope::num) / static_cast<double>(Slope::den)
    );

    // positive = downregulated
    // negative = upregulated
    double state;
    // state | add to match score
    // +inf  | += 1.0 * (1.0 - score)
    // ...   | ...
    // 1.0   | += ~slope * (1.0 - score)
    // 0.0   | neutral
    // -1.0  | -= ~slope * score
    // ...   | ...
    // -inf  | -= 1.0 * score

    // which is equivalent to
    // state | add to match score
    // +inf  | += 1.0 * (1.0 - score)
    // ...   | ...
    // 1.0   | += ~slope * (1.0 - score)
    // 0.0   | neutral
    // -1.0  | += ~slope * -score
    // ...   | ...
    // -inf  | += 1.0 * -score

    // countdown timer to reseting state
    size_t timer;

    MultiplicativeCountdownRegulator() : state(0.0), timer(0) {}

    /// Apply regulation to a raw match score.
    /// Returns a value between 0 and 1.
    double operator()(const double raw_score) const override {
      const double res = (
        raw_score
        + std::tanh(slope * state) * (
          state < 0
          ? raw_score
          : 1.0 - raw_score
        )
      );
      emp_assert(state <= 0 || res >= raw_score);
      emp_assert(state >= 0 || res <= raw_score);
      emp_assert(res >= 0.0 && res <= 1.0);
      return res;
    }

    /// A positive value downregulates the item,
    /// a value of zero is neutral,
    /// and a negative value upregulates the item.
    bool Set(const double & set) override {
      timer = 1;

      // return whether regulator value changed
      // (i.e., we need to purge the cache)
      return std::exchange(state, set) != set;
    }

    /// A negative value upregulates the item,
    /// a value of exactly zero is neutral
    /// and a postive value downregulates the item.
    bool Adj(const double & amt) override {
      timer = 1;

      state += amt;

      // return whether regulator value changed
      // (i.e., we need to purge the cache)
      return amt != 0.0;
    }

    /// Timer decay.
    /// Return whether MatchBin should be updated
    bool Decay(const int steps) override {
      if (steps < 0) {
        // if reverse decay is requested
        timer += -steps;
      } else {
        // if forward decay is requested
        timer -= std::min(timer, static_cast<size_t>(steps));
      }

      return timer == 0 ? (
        std::exchange(state, 0.0) != 0.0
      ) : false;
    }

    /// Return a double representing the state of the regulator.
    const double & View() const override { return state; }

    std::string name() const override {
      return "Multiplicative Countdown Regulator";
    }

    bool operator!=(const MultiplicativeCountdownRegulator & other) const {
      return state != other.state || timer != other.timer;
    }

    #ifdef CEREAL_NVP
    template <class Archive>
    void serialize( Archive & ar )
    {
      ar(
        CEREAL_NVP(state),
        CEREAL_NVP(timer)
      );
    }
    #endif

  };

}

#endif
