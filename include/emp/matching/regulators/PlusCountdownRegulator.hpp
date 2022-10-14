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

#ifndef EMP_PLUS_COUNTDOWN_REGULATOR_HPP
#define EMP_PLUS_COUNTDOWN_REGULATOR_HPP

#include <algorithm>
#include <cmath>
#include <ratio>
#include <string>
#include <tuple>
#include <utility>

#include "../../base/assert.hpp"

namespace emp {

template <
  typename Slope=std::deci,
  typename MaxUpreg=std::ratio<1>,
  typename ClampLeeway=std::ratio<0>,
  uint32_t CountdownStart=1
>
struct PlusCountdownRegulator {

  using set_t = float;
  using adj_t = float;
  using view_t = float;

  static constexpr float slope = (
    static_cast<float>(Slope::num) / static_cast<float>(Slope::den)
  );
  static constexpr float max_up = (
    -static_cast<float>(MaxUpreg::num) / static_cast<float>(MaxUpreg::den)
  );
  static constexpr float clamp_leeway = (
    static_cast<float>(ClampLeeway::num) / static_cast<float>(ClampLeeway::den)
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
  // -inf  | -= MaxUpreg

  // countdown timer to reseting state
  uint32_t timer{};

  /// Apply regulation to a raw match score.
  /// Returns a value between 0.0f and 1.0f
  __attribute__ ((hot))
  float operator()(const float raw_score) const {
    const float res = std::clamp(
      std::max(slope * state, max_up) + raw_score,
      -clamp_leeway,
      1.0f + clamp_leeway
    );
    emp_assert(state <= 0.0f || res >= raw_score
      || raw_score > 1.0f || std::isnan(state),
      state, res, raw_score);
    emp_assert(
      state >= 0.0f || res <= raw_score
      || raw_score < 0.0f || std::isnan(state),
      state, res, raw_score);
    emp_assert(
      (res >= -clamp_leeway && res <= 1.0f + clamp_leeway) || std::isnan(state),
      res
    );
    return res;

  }

  /// A positive value downregulates the item,
  /// a value of zero is neutral,
  /// and a negative value upregulates the item.
  bool Set(const float& set) {
    if ( std::isnan( set ) ) return false;

    timer = CountdownStart;

    // return whether regulator value changed
    // (i.e., we need to purge the cache)
    return std::exchange(state, set) != set;
  }

  /// A negative value upregulates the item,
  /// a value of exactly zero is neutral
  /// and a postive value downregulates the item.
  bool Adj(const float& amt) {
    if ( std::isnan( amt ) ) return false;

    timer = CountdownStart;

    state += amt;

    // return whether regulator value changed
    // (i.e., we need to purge the cache)
    return amt != 0.0f;
  }

  /// Timer decay.
  /// Return whether MatchBin should be updated
  bool Decay(const int32_t steps) {

    const int64_t res = static_cast<int64_t>(timer) - steps;
    constexpr int64_t floor = std::numeric_limits<decltype(timer)>::lowest();
    constexpr int64_t ceil = std::numeric_limits<decltype(timer)>::max();
    timer = std::clamp( res, floor, ceil );

    if ( timer == 0 ) return std::exchange(state, 0.0f) != 0.0f;
    else return false;

  }

  /// Return a float representing the state of the regulator.
  const float & View() const { return state; }

  bool operator==( const PlusCountdownRegulator& other ) const {
    return std::tuple{
      state, timer
    } == std::tuple{
      other.state, other.timer
    };
  }

  bool operator!=( const PlusCountdownRegulator& other ) const {
    return !operator==( other );
  }

  std::string name() const { return "Plus Countdown Regulator"; }

};

} // namespace emp

#endif // #ifndef EMP_PLUS_COUNTDOWN_REGULATOR_HPP
