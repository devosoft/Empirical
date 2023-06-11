/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file PlusCountdownRegulator.hpp
 *  @brief Regulator that modifies match distance through addition
 *  and decays to baseline with a countdown timer.
 *
 */

#ifndef EMP_MATCHING_REGULATORS_PLUSCOUNTDOWNREGULATOR_HPP_INCLUDE
#define EMP_MATCHING_REGULATORS_PLUSCOUNTDOWNREGULATOR_HPP_INCLUDE

#include <algorithm>
#include <cmath>
#include <ratio>
#include <utility>

namespace emp {

template <typename Slope=std::deci>
struct PlusCountdownRegulator {

  using set_t = float;
  using adj_t = float;
  using view_t = float;

  static constexpr float slope = (
    static_cast<float>(Slope::num) / static_cast<float>(Slope::den)
  );

  // positive = downregulated
  // negative = upregulated
  float state{};
  // state | add to match score
  // +inf  | += 1.0f
  // ...   | ...
  // 1.0f   | += ~slope
  // 0.0f   | neutral
  // -1.0f  | -= slope
  // ...   | ...
  // -inf  | -= 1.0f

  // countdown timer to reseting state
  unsigned char timer{};

  /// Apply regulation to a raw match score.
  /// Returns a value between 0.0f and 1.0f
  __attribute__ ((hot))
  float operator()(const float raw_score) const {
    const float res = std::clamp(
      slope * state + raw_score,
      0.0f,
      1.0f
    );
    emp_assert(state <= 0.0f || res >= raw_score || std::isnan(state),
      state, res, raw_score);
    emp_assert(state >= 0.0f || res <= raw_score || std::isnan(state),
      state, res, raw_score);
    emp_assert( (res >= 0.0f && res <= 1.0f) || std::isnan(state),
      res);
    return res;

  }

  /// A positive value downregulates the item,
  /// a value of zero is neutral,
  /// and a negative value upregulates the item.
  bool Set(const float& set) {
    if ( std::isnan( set ) ) return false;

    timer = 1;

    // return whether regulator value changed
    // (i.e., we need to purge the cache)
    return std::exchange(state, set) != set;
  }

  /// A negative value upregulates the item,
  /// a value of exactly zero is neutral
  /// and a postive value downregulates the item.
  bool Adj(const float& amt) {
    if ( std::isnan( amt ) ) return false;

    timer = 1;

    state += amt;

    // return whether regulator value changed
    // (i.e., we need to purge the cache)
    return amt != 0.0f;
  }

  /// Timer decay.
  /// Return whether MatchBin should be updated
  bool Decay(const int steps) {
    if (steps < 0) {
      // if reverse decay is requested
      timer += -steps;
    } else {
      // if forward decay is requested
      timer -= std::min(timer, static_cast<unsigned char>(steps));
    }

    return timer == 0 ? (
      std::exchange(state, 0.0f) != 0.0f
    ) : false;

  }

  /// Return a float representing the state of the regulator.
  const float & View() const { return state; }

};

} // namespace emp

#endif // #ifndef EMP_MATCHING_REGULATORS_PLUSCOUNTDOWNREGULATOR_HPP_INCLUDE
