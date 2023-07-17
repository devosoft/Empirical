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

#include "../base/assert.hpp"
#include "../base/vector.hpp"

namespace emp {

  /// A range of values from a lower limit to and upper limit, of any provided type.
  template <typename T, bool INCLUDE_UPPER=true>
  class Range {
  private:
    T lower = std::numeric_limits<T>::lowest();  ///< Beginning of range, inclusive.
    T upper = std::numeric_limits<T>::max();     ///< End of range, (included if INCLUDE_UPPER)

  public:
    Range() = default;
    Range(T _l, T _u) : lower(_l), upper(_u) { emp_assert(_l <= _u, _l, _u); }
    Range(const Range &) = default;

    T GetLower() const { return lower; }
    T GetUpper() const { return upper; }
    T GetSize() const { return upper - lower + INCLUDE_UPPER; }

    Range & operator=(const Range&) = default;
    bool operator==(const Range& _in) const = default;
    bool operator!=(const Range& _in) const = default;

    void SetLower(T l) { lower = l; }
    void SetUpper(T u) { upper = u; }
    void Set(T _l, T _u) { emp_assert(_l < _u); lower = _l; upper = _u; }

    void SetMaxLower() { lower = std::numeric_limits<T>::min(); }
    void SetMaxUpper() { upper = std::numeric_limits<T>::max(); }

    // Flexible lower/upper accessor that can get and set.
    T & Lower() { return lower; }
    T & Upper() { return upper; }

    const T & Lower() const noexcept { return lower; }
    const T & Upper() const noexcept { return upper; }

    /// Determine if a provided value is in the range INCLUSIVE of the endpoints.
    bool Valid(T value) const {
      return (value >= lower && value < upper) || (INCLUDE_UPPER && value == upper);
    }

    /// Force a value into range
    T LimitValue(T _in) const {
      if constexpr (INCLUDE_UPPER) {
        return (_in < lower) ? lower : ((_in > upper) ? upper : _in);
      } else {
        return (_in < lower) ? lower : ((_in > upper) ? (upper) : _in);
      }
    }
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
