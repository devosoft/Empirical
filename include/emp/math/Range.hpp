/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2016-2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/math/Range.hpp
 * @brief A simple way to track value ranges
 * @note Status: BETA
 */

#pragma once

#ifndef INCLUDE_EMP_MATH_RANGE_HPP_GUARD
#define INCLUDE_EMP_MATH_RANGE_HPP_GUARD

#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>

#include "../base/assert.hpp"
#include "../base/vector.hpp"
#include "../tools/String.hpp"

namespace emp {

  /// A range of values from a lower limit to an upper limit, of any provided type.
  /// When INCLUDE_UPPER=true  (default), the range is [lower, upper] (both inclusive).
  /// When INCLUDE_UPPER=false, the range is [lower, upper) (upper is exclusive).
  /// Internally, `upper` always stores the exclusive boundary; GetMaxValue() returns
  /// the largest *included* value.
  template <typename T, bool INCLUDE_UPPER = true>
  class Range {
  private:
    T lower = MinLimit();  ///< Beginning of range, inclusive.
    T upper = MaxLimit();  ///< End of range (exclusive if !INCLUDE_UPPER).

    using this_t = Range<T, INCLUDE_UPPER>;

  public:
    static constexpr bool is_integral = std::is_integral_v<T>;

    Range() = default;

    /// Construct a single-value range containing exactly `val`.
    Range(T val) : lower(val), upper(val) {
      if constexpr (!INCLUDE_UPPER) { upper = NextAbove(val); }
    }

    Range(T _l, T _u) : lower(_l), upper(_u) { emp_assert(_l <= _u, _l, _u); }

    Range(const Range &) = default;

    Range & operator=(const Range &)         = default;
    bool operator==(const Range & _in) const = default;
    bool operator!=(const Range & _in) const = default;

    T GetLower() const { return lower; }

    T GetUpper() const { return upper; }

    /// One step above val in type T (for advancing past the included maximum).
    [[nodiscard]] static T NextAbove(T val) {
      if constexpr (is_integral) return val + T{1};
      else return std::nextafter(val, std::numeric_limits<T>::max());
    }

    /// One step below val in type T (for retreating from the exclusive upper).
    [[nodiscard]] static T NextBelow(T val) {
      if constexpr (is_integral) return val - T{1};
      else return std::nextafter(val, std::numeric_limits<T>::lowest());
    }

    /// Smallest representable delta (useful for integral arithmetic; for floats
    /// prefer NextAbove/NextBelow which are value-scaled).
    [[nodiscard]] static constexpr T GetEpsilon() {
      if constexpr (is_integral) return T{1};
      else return std::numeric_limits<T>::epsilon();
    }

    /// The largest value that is considered *inside* the range.
    [[nodiscard]] T GetMaxValue() const {
      if constexpr (INCLUDE_UPPER) return upper;
      else                         return NextBelow(upper);
    }

    /// Number of values in the range (count for integrals, length for floats).
    [[nodiscard]] T GetSize() const {
      if constexpr (INCLUDE_UPPER && is_integral) return upper - lower + T{1};
      else                                        return upper - lower;
    }

    [[nodiscard]] static constexpr T MinLimit() { return std::numeric_limits<T>::lowest(); }

    [[nodiscard]] static constexpr T MaxLimit() { return std::numeric_limits<T>::max(); }

    [[nodiscard]] emp::String ToString() const {
      if constexpr (INCLUDE_UPPER) return emp::MakeString('[', lower, ',', upper, ']');
      else                         return emp::MakeString('[', lower, ',', upper, ')');
    }

    void SetLower(T l) { lower = l; }

    void SetUpper(T u) { upper = u; }

    void Set(T _l, T _u) {
      emp_assert(_l <= _u, _l, _u);
      lower = _l;
      upper = _u;
    }

    void ShiftDown(T shift) {
      emp_assert(shift > T{}, shift);
      emp_assert(lower <= upper, lower, upper);
      upper = (MinLimit() + shift < upper) ? (upper - shift) : MinLimit();
      lower = (MinLimit() + shift < lower) ? (lower - shift) : MinLimit();
    }

    void ShiftUp(T shift) {
      emp_assert(shift > T{}, shift);
      emp_assert(lower <= upper, lower, upper);
      upper = (MaxLimit() - shift > upper) ? (upper + shift) : MaxLimit();
      lower = (MaxLimit() - shift > lower) ? (lower + shift) : MaxLimit();
    }

    void Shift(T shift) {
      if      (shift > T{}) ShiftUp(shift);
      else if (shift < T{}) ShiftDown(-shift);
      // shift == T{}: no-op
    }

    void SetMinLower() { lower = std::numeric_limits<T>::lowest(); }

    void SetMaxUpper() { upper = std::numeric_limits<T>::max(); }

    void Grow(T amount = 1) {
      if (amount > T{}) upper += amount;
      else              lower += amount;
    }

    // Flexible lower/upper accessors that can get and set.
    T &       Lower()       { return lower; }
    T &       Upper()       { return upper; }
    const T & Lower() const noexcept { return lower; }
    const T & Upper() const noexcept { return upper; }

    /// Returns true if `value` is within the range.
    [[nodiscard]] bool Has(T value) const {
      return (value >= lower && value < upper) || (INCLUDE_UPPER && value == upper);
    }

    [[deprecated("Renamed to Has()")]] bool Valid(T value) const { return Has(value); }

    /// Returns true if every value in `in_range` is also in this range.
    [[nodiscard]] bool HasRange(const this_t & in_range) const {
      return Has(in_range.lower) && Has(in_range.GetMaxValue());
    }

    /// Will identify if two ranges are next to each other or overlapping.
    [[nodiscard]] bool IsConnected(this_t in) const {
      return (in.lower >= lower && in.lower <= upper) || (lower >= in.lower && lower <= in.upper);
    }

    /// Determine if there is overlap between two ranges (not merely adjacent).
    [[nodiscard]] bool HasOverlap(this_t in) const {
      return (in.lower >= lower && in.lower < upper) || (lower >= in.lower && lower < in.upper);
    }

    /// Determine the amount of overlap between two ranges.
    [[nodiscard]] T CalcOverlap(this_t in) const {
      const T combo_upper = std::min(upper, in.upper);
      const T combo_lower = std::max(lower, in.lower);
      return (combo_upper > combo_lower) ? (combo_upper - combo_lower) : T{};
    }

    /// @brief Expand this range to encompass a provided value.
    /// @param val Value to include in the range.
    /// @return Whether the range changed.
    bool Expand(T val) {
      if (val < lower) {
        lower = val;
        return true;
      }
      if (!Has(val)) {
        // val is at or beyond the upper bound; extend to include it.
        upper = INCLUDE_UPPER ? val : NextAbove(val);
        return true;
      }
      return false;
    }

    /// @brief Expand this range to encompass all provided values.
    /// @return Whether the range changed at all.
    template <typename... Ts>
    bool Expand(T val1, T val2, Ts... args) {
      return Expand(val1) + Expand(val2, args...);  // Use + to avoid short-circuiting.
    }

    /// Merge this range with another.  Must be adjacent or overlap (returns false if not).
    bool Merge(this_t in) {
      if (!IsConnected(in)) { return false; }
      Expand(in.lower, in.upper);
      return true;
    }

    /// Add the next consecutive value to the end of this range (integral ranges only).
    /// Returns false if `val` is not the immediate successor of the current maximum.
    bool Append(T val) {
      static_assert(is_integral, "Only integral ranges can call Append() with a single value.");
      if (val != upper + INCLUDE_UPPER) { return false; }
      upper++;
      return true;
    }

    /// Clamp `_in` to the nearest included value in this range.
    [[nodiscard]] T Clamp(T _in) const {
      return (_in < lower) ? lower : ((_in >= upper) ? GetMaxValue() : _in);
    }

    [[deprecated("Renamed to Clamp()")]] T LimitValue(T _in) const { return Clamp(_in); }

    [[nodiscard]] double ToFraction(T _in) const {
      emp_assert(GetSize() != T{});
      return static_cast<double>(_in - lower) / static_cast<double>(GetSize());
    }

    [[nodiscard]] T FromFraction(double frac) const { return static_cast<T>(frac * GetSize()) + lower; }

    // Adjust the upper or lower if provided value is more limiting.
    void LimitLower(T in) { if (in > lower) { lower = in; } }

    void LimitUpper(T in) { if (in < upper) { upper = in; } }

    [[nodiscard]] size_t CalcBin(T value, size_t num_bins) const {
      if (upper == lower) { return 0; }
      return static_cast<size_t>(
        static_cast<double>(value - lower) / static_cast<double>(upper - lower) * num_bins
      );
    }

    /// Produce a vector that spreads values evenly across the range.
    [[nodiscard]] emp::vector<T> Spread(const size_t s) const {
      emp_assert(s >= 1);
      if (s == 1) {
        return emp::vector<T>(1, lower / 2 + upper / 2);  // midpoint
      }
      emp::vector<T> out(s);
      out[0] = lower;
      const T range = upper - lower;
      for (size_t i = 1; i < s; i++) {
        const double frac = static_cast<double>(i) / static_cast<double>(s - 1);
        out[i] = lower + static_cast<T>(frac * range);
      }
      return out;
    }
  };

  /// Build a new range with auto-detected type.
  template <typename T, bool INCLUDE_UPPER = true>
  Range<T, INCLUDE_UPPER> MakeRange(T _l, T _u) {
    return Range<T, INCLUDE_UPPER>(_l, _u);
  }

  /// Build a new range from a string.
  template <typename T, bool INCLUDE_UPPER = true>
  Range<T, INCLUDE_UPPER> MakeRange(emp::String in_str) {
    T val1 = in_str.Pop("-").As<T>();  // Respect a dash if there is one.
    T val2 = in_str.size() ? in_str.As<T>() : val1;
    return emp::MakeRange(val1, val2);
  }

  /// Build a new range of type int.
  template <bool INCLUDE_UPPER = true>
  inline Range<int, INCLUDE_UPPER> IntRange(int _l, int _u) {
    return Range<int, INCLUDE_UPPER>(_l, _u);
  }

  /// Build a new range of type double.
  template <bool INCLUDE_UPPER = true>
  inline Range<double, INCLUDE_UPPER> DRange(double _l, double _u) {
    return Range<double, INCLUDE_UPPER>(_l, _u);
  }

  // Allow a Range to be printed.
  template <typename T, bool INCLUDE_UPPER = true>
  std::ostream & operator<<(std::ostream & out, const emp::Range<T, INCLUDE_UPPER> & range) {
    out << '[' << range.GetLower() << ',' << range.GetUpper() << (INCLUDE_UPPER ? ']' : ')');
    return out;
  }

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_MATH_RANGE_HPP_GUARD
