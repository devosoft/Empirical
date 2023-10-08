/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2023.
 *
 *  @file Range.hpp
 *  @brief A simple way to track value ranges
 *  @note Status: BETA
 */

#ifndef EMP_MATH_RANGE_HPP_INCLUDE
#define EMP_MATH_RANGE_HPP_INCLUDE

#include <limits>
#include <type_traits>

#include "../base/assert.hpp"
#include "../base/vector.hpp"
#include "../tools/String.hpp"

namespace emp {

  /// A range of values from a lower limit to and upper limit, of any provided type.
  template <typename T, bool INCLUDE_UPPER=true>
  class Range {
  private:
    T lower = std::numeric_limits<T>::lowest();  ///< Beginning of range, inclusive.
    T upper = std::numeric_limits<T>::max();     ///< End of range, (included if INCLUDE_UPPER)

    using this_t = Range<T, INCLUDE_UPPER>;
  public:
    static constexpr bool is_integral = std::is_integral<T>();

    Range() = default;
    Range(T val) : lower(val), upper(val) {
      if constexpr (!INCLUDE_UPPER) upper += GetEpsilon();
    }
    Range(T _l, T _u) : lower(_l), upper(_u) { emp_assert(_l <= _u, _l, _u); }
    Range(const Range &) = default;

    Range & operator=(const Range&) = default;
    bool operator==(const Range& _in) const = default;
    bool operator!=(const Range& _in) const = default;

    T GetLower() const { return lower; }
    T GetUpper() const { return upper; }
    T GetEpsilon() const {
      if constexpr (is_integral) return 1;
      else return upper * std::numeric_limits<T>::epsilon();
    }
    T GetMaxValue() const { // What is the maximum included value?
      if constexpr (INCLUDE_UPPER) return upper;
      else return upper - GetEpsilon();
    }
    T GetSize() const { return upper - lower + (INCLUDE_UPPER && is_integral); }
    [[nodiscard]] static constexpr T MaxLimit() { return std::numeric_limits<T>::max(); }
    [[nodiscard]] static constexpr T MinLimit() { return std::numeric_limits<T>::lowest(); }

    emp::String ToString() const {
      if constexpr (INCLUDE_UPPER) {
        return emp::MakeString('[', lower, ',', upper, ']');
      } else {
        return emp::MakeString('[', lower, ',', upper, ')');
      }
    }

    void SetLower(T l) { lower = l; }
    void SetUpper(T u) { upper = u; }
    void Set(T _l, T _u) { emp_assert(_l < _u); lower = _l; upper = _u; }
    void ShiftDown(T shift) {
      emp_assert(shift > 0);
      emp_assert(lower <= upper, lower, upper);
      // Guard against underflow
      upper = (MinLimit() + shift < upper) ? (upper - shift) : MinLimit();
      lower = (MinLimit() + shift < lower) ? (lower - shift) : MinLimit();
    }
    void ShiftUp(T shift) {
      emp_assert(shift > 0);
      emp_assert(lower <= upper, lower, upper);
      // Guard against overflow
      upper = (MaxLimit() - shift > upper) ? (upper + shift) : MaxLimit();
      lower = (MaxLimit() - shift > lower) ? (lower + shift) : MaxLimit();
    }
    void Shift(T shift) {
      if (shift > 0) ShiftUp(shift); 
      else ShiftDown(-shift);
    }

    void SetMinLower() { lower = std::numeric_limits<T>::min(); }
    void SetMaxUpper() { upper = std::numeric_limits<T>::max(); }

    void Grow(T amount=1) {
      if (amount > 0) upper += amount;
      else lower += amount;
    }

    // Flexible lower/upper accessor that can get and set.
    T & Lower() { return lower; }
    T & Upper() { return upper; }

    const T & Lower() const noexcept { return lower; }
    const T & Upper() const noexcept { return upper; }

    /// Determine if a provided value is in the range INCLUSIVE of the endpoints.
    bool Has(T value) const {
      return (value >= lower && value < upper) || (INCLUDE_UPPER && value == upper);
    }
    [[deprecated("Renamed to Has()")]]
    bool Valid(T value) const { return Has(value); }

    bool HasRange(this_t in_range) {
      return Has(in_range.lower) && Has(in_range.upper);
    }

    /// Will identify if two ranges are next to each other or overlapping.
    bool IsConnected(this_t in) const {
      return (in.lower >= lower && in.lower <= upper) ||
             (lower >= in.lower && lower <= in.upper);
    }

    /// Determine if there is overlap between two range.
    /// Similar to IsConnected, but cannot be merely adjacent.
    bool HasOverlap(this_t in) const {
      return (in.lower >= lower && in.lower < upper) ||
             (lower >= in.lower && lower < in.upper);
    }

    /// Determine the amount of overlap between two range.
    T CalcOverlap(this_t in) const {
      const T combo_upper = std::min(upper, in.upper);
      const T combo_lower = std::max(lower, in.lower);
      return (combo_upper > combo_lower) ? (combo_upper - combo_lower) : T{};
    }

    /// @brief  Expand this range to encompass a provided value.
    /// @param val Value to expand through.
    /// @return Whether the range has changed due to this expansion.
    bool Expand(T val) {
      if (val < lower) lower = val;
      else if (val > upper) {
        upper = val;
        if constexpr (INCLUDE_UPPER) upper += GetEpsilon();
      } else return false;
      return true;
    }

    /// @brief Expand this range to encompass all provided values.
    /// @return Whether the range has changed due to this expansion.
    template <typename... Ts>
    bool Expand(T val1, T val2, Ts... args) {
      return Expand(val1) + Expand(val2, args...); // Use + to avoid short-circuiting.
    }

    /// Merge this range with another.  Must be adjacent or overlap (return false if not!)
    bool Merge(this_t in) {
      if (!IsConnected(in)) return false;
      Expand(in.lower, in.upper);
      return true;
    }

    /// Add a specified value to the end of a range (or return false if failed).
    bool Append(T val) {
      emp_assert(is_integral, "Only integral ranges can call Append() with a single value.");
      if (val != upper + INCLUDE_UPPER) return false;
      upper++;
      return true;
    }

    /// Force a value into range
    T Clamp(T _in) const {
      return (_in < lower) ? lower : ((_in >= upper) ? GetMaxValue() : _in);
    }
    [[deprecated("Renamed to Clamp()")]]
    T LimitValue(T _in) const { return Clamp(_in); }

    double ToFraction(T _in) const {
      emp_assert(GetSize() != 0);
      return static_cast<double>(_in - lower) / static_cast<double>(GetSize());
    }
    T FromFraction(double frac) const { return frac * GetSize() + lower; }

    // Adjust the upper or lower if provided value is more limiting.
    void LimitLower(T in) { if (in > lower) lower = in; }
    void LimitUpper(T in) { if (in < upper) upper = in; }

    size_t CalcBin(T value, size_t num_bins) const {
      if (upper == lower) return 0;
      return (size_t) (((double) (value - lower)) / ((double) (upper - lower)) * (double) num_bins);
    }

    /// Produce a vector that spreads values evenly across the range.
    emp::vector<T> Spread(const size_t s) const {
      emp_assert(s >= 1);
      if (s == 1) return emp::vector<T>(1,lower/2+upper/2); // On point is in the middle of the range.
      emp::vector<T> out(s);
      out[0] = lower;
      T range = upper - lower;
      for (size_t i = 1; i < s; i++) {
        double frac = static_cast<double>(i)/static_cast<double>(s-1);
        out[i] = lower + static_cast<T>(frac * range);
      }
      return out;
    }
  };

  /// Build a new range with auto-detected type.
  template <typename T, bool INCLUDE_UPPER=true>
  Range<T, INCLUDE_UPPER> MakeRange(T _l, T _u) {
    return Range<T, INCLUDE_UPPER>(_l,_u);
  }

  /// Build a new range of type int.
  template <bool INCLUDE_UPPER=true>
  inline Range<int, INCLUDE_UPPER> IntRange(int _l, int _u) {
    return Range<int, INCLUDE_UPPER>(_l,_u);
  }

  /// Build a new range of type double.
  template <bool INCLUDE_UPPER=true>
  inline Range<double, INCLUDE_UPPER> DRange(double _l, double _u) {
    return Range<double, INCLUDE_UPPER>(_l,_u);
  }
}

#endif // #ifndef EMP_MATH_RANGE_HPP_INCLUDE
