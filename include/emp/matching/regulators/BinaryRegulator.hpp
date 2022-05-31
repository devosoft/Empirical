/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file BinaryRegulator.hpp
 *  @brief Regulator that modifies match distance through addition
 *  and decays to baseline with a countdown timer.
 *
 */

#ifndef EMP_BINARY_REGULATOR_HPP
#define EMP_BINARY_REGULATOR_HPP

#include <algorithm>
#include <cmath>
#include <ratio>
#include <string>
#include <tuple>
#include <utility>

namespace emp {

template <
  uint32_t CountdownStart=1
>
struct BinaryRegulator {

  using set_t = float;
  using adj_t = float;
  using view_t = float;

  // positive = inactivated
  // zero = activated
  float state{};

  // countdown timer to reseting state
  uint32_t timer{};

  /// Apply regulation to a raw match score.
  /// Returns a value between 0.0f and 1.0f
  __attribute__ ((hot))
  float operator()(const float raw_score) const {
    assert( state >= 0.0f);
    return (state == 0.0f)
      ? raw_score
      : 1.0;
  }

  /// A positive value downregulates the item,
  /// a value of zero is neutral,
  /// and a negative value upregulates the item.
  float Set(const float& set) {
    float set_ = set;
    if ( std::isnan( set ) ) set_ = 0;

    timer = CountdownStart;

    if (set_ <= 0.0f) return 0.0f - std::exchange(state, 0.0f);
    else return 1.0f - std::exchange(state, 1.0f);;
  }

  /// A negative value upregulates the item,
  /// a value of exactly zero is neutral
  /// and a postive value downregulates the item.
  float Adj(const float& amt) {
    return Set(amt);
  }

  /// Timer decay.
  /// Return whether MatchBin should be updated
  float Decay(const int32_t steps) {

    const int64_t res = static_cast<int64_t>(timer) - steps;
    constexpr int64_t floor = std::numeric_limits<decltype(timer)>::lowest();
    constexpr int64_t ceil = std::numeric_limits<decltype(timer)>::max();
    timer = std::clamp( res, floor, ceil );

    if ( timer == 0 ) return 0.0f - std::exchange(state, 0.0f);
    else return false;

  }

  /// Return a float representing the state of the regulator.
  const float & View() const { return state; }

  bool operator==( const BinaryRegulator& other ) const {
    return std::tuple{
      state, timer
    } == std::tuple{
      other.state, other.timer
    };
  }

  bool operator!=( const BinaryRegulator& other ) const {
    return !operator==( other );
  }

  std::string name() const { return "Binary Countdown Regulator"; }

};

} // namespace emp

#endif // #ifndef EMP_BINARY_REGULATOR_HPP
