/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2018
 *
 *  @file  math.h
 *  @brief Useful mathematical functions (that are constexpr when possible.)
 *  @note Status: BETA (though new functions are added frequently)
 */


#ifndef EMP_MATH_H
#define EMP_MATH_H

#include <initializer_list>
#include <algorithm>
#include <cmath>

#include "../base/assert.h"
#include "../meta/reflection.h"
#include "Random.h"
#include "const.h"

namespace emp {

  /// % is actually remainder; Mod is a proper modulus command that handles negative #'s correctly
  inline constexpr int Mod(int in_val, int mod_val) {
    emp_assert(mod_val != 0);
    in_val %= mod_val;
    return (in_val < 0) ? (in_val + mod_val) : in_val;
  }

  /// Regular Mod doesn't work on doubles.  Build one that does!
  // @CAO Make constexpr?
  inline double Mod(double in_val, double mod_val) {
    emp_assert(mod_val != 0);
    const double remain = std::remainder(in_val, mod_val);
    return (remain < 0.0) ? (remain + mod_val) : remain;
  }

  /// Calculate the sign (i.e., +1, -1, or 0) of a value.
  template <typename T> inline int Sgn(T val) {
    return (T(0) < val) - (val < T(0));
  }

  /// Find the absolute value for any variable.
  template <typename T> constexpr T Abs(T in) { return (in > 0) ? in : (-in); }

  /// Divide one integer by another, rounding towards minus infinity.
  int inline FloorDivide(int dividend, int divisor) {
    int q = dividend/divisor;
    int r = dividend%divisor;
    if ((r!=0) && ((r<0) != (divisor<0))) --q;
    return q;
  }

  /// Default integer division is truncated, not rounded.
  /// Round the division result instead of truncating it.
  /// Rounding ties (i.e., result % divisor == 0.5) are rounded up.
  int inline RoundedDivide(int dividend, int divisor) {
    //TODO add emp_assert to check for overflow
    emp_assert(divisor != 0);
    // adding divisor/2 to dividend
    // is equivalent to adding 1/2 to the result
    return FloorDivide(dividend + divisor / 2, divisor);
  }

  /// Default integer division is truncated, not rounded.
  /// Round the division result instead of truncating it.
  /// Rounding ties (i.e., result % divisor == 0.5)
  /// will be rounded up.
  size_t inline RoundedDivide(size_t dividend, size_t divisor) {
    //TODO add emp_assert to check for overflow
    emp_assert(divisor != 0);
    // adding divisor/2 to dividend
    // is equivalent to adding 1/2 to the result
    return (dividend + divisor / 2) / divisor;
  }

  /// Regular integer division is truncated, not rounded.
  /// Round the division result instead of truncating it.
  /// Rounding ties (i.e., result % divisor == 0.5) are broken
  /// by coin toss.
  int inline UnbiasedDivide(int dividend, int divisor, emp::Random& r) {
    //TODO add emp_assert to check for overflow
    int res = RoundedDivide(dividend, divisor);
    // if dividend/divisor % 1 == 0.5...
    if (Abs(dividend % divisor) * 2 == Abs(divisor)) {
      // ... by default, the result is rounded up;
      // with 1/2 probability round down instead
      res -= r.GetInt(2);
    }
    return res;
  }

  /// Regular integer division is truncated, not rounded.
  /// Round the division result instead of truncating it.
  /// Rounding ties (i.e., result % divisor == 0.5) are broken
  /// by coin toss.
  inline size_t UnbiasedDivide(size_t dividend, size_t divisor, emp::Random& r) {
    //TODO add emp_assert to check for overflow
    size_t res = RoundedDivide(dividend, divisor);
    // if dividend/divisor % 1 == 0.5...
    if ((dividend % divisor) * 2 == divisor) {
      // ... by default, the result is rounded up;
      // with 1/2 probability round down instead
      res -= r.GetInt(2);
    }
    return res;
  }

  /// Run both min and max on a value to put it into a desired range.
  template <typename TYPE> constexpr TYPE ToRange(const TYPE & value, const TYPE & in_min, const TYPE & in_max) {
    emp_assert(in_min <= in_max);
    return (value < in_min) ? in_min : ((value > in_max) ? in_max : value);
  }

  /// Min of only one element is that element itself!
  template <typename T> constexpr T Min(T in1) { return in1; }

  /// Min of multiple elements is solved recursively.
  template <typename T, typename... Ts>
  constexpr T Min(T in1, T in2, Ts... extras) {
    T cur_result = Min(in2, extras...);
    return (in1 < cur_result) ? in1 : cur_result;
  }


  /// Max of only one element is that element itself!
  template <typename T> constexpr T Max(T in1) { return in1; }

  /// Max of multiple elements is solved recursively.
  template <typename T, typename... Ts>
  constexpr T Max(T in1, T in2, Ts... extras) {
    T cur_result = Max(in2, extras...);
    return (in1 < cur_result) ? cur_result : in1;
  }

  /// MinRef works like Min, but never copies any inputs; always treats as references.
  /// MinRef of only one element returns reference to that element itself!
  template <typename T> constexpr const T & MinRef(const T& in1) { return in1; }

  /// MinRef of multiple elements returns reference to minimum value.
  template <typename T, typename... Ts>
  constexpr const T & MinRef(const T& in1, const T& in2, const Ts&... extras) {
    const T & cur_result = MinRef(in2, extras...);
    return (in1 < cur_result) ? in1 : cur_result;
  }


  /// MaxRef works like Max, but never copies any inputs; always treats as references.
  /// MaxRef of only one element returns reference to that element itself!
  template <typename T> constexpr const T & MaxRef(const T& in1) { return in1; }

  /// MaxRef of multiple elements returns reference to maximum value.
  template <typename T, typename... Ts>
  constexpr const T & MaxRef(const T& in1, const T& in2, const Ts&... extras) {
    const T & cur_result = MaxRef(in2, extras...);
    return (in1 < cur_result) ? cur_result : in1;
  }


  namespace internal {
    // A compile-time log calculator for values [1,2)
    static constexpr double Log2_base(double x) {
      emp_assert(x > 0);
      return log2_chart_1_2[(int)((x-1.0)*1024)];
      // return InterpolateTable(log2_chart_1_2, x-1.0, 1024);
    }

    // A compile-time log calculator for values < 1
    static constexpr double Log2_frac(double x) {
      emp_assert(x > 0);
      return (x >= 1.0) ? Log2_base(x) : (Log2_frac(x*2.0) - 1.0);
    }

    // A compile-time log calculator for values >= 2
    static constexpr double Log2_pos(double x) {
      emp_assert(x > 0);
      emp_assert(x != INFINITY);
      return (x < 2.0) ? Log2_base(x) : (Log2_pos(x/2.0) + 1.0);
    }

  }

  /// @endcond

  /// Compile-time log base 2 calculator.
  static constexpr double Log2(double x) {
    emp_assert(x > 0);
    return (x < 1.0) ? internal::Log2_frac(x) : internal::Log2_pos(x);
  }

  /// Compile-time log calculator
  static constexpr double Log(double x, double base=10.0) { return Log2(x) / Log2(base); }
  /// Compile-time natural log calculator
  static constexpr double Ln(double x) { return Log(x, emp::E); }   // Natural Log...
  /// Compile-time log base 10 calculator.
  static constexpr double Log10(double x) { return Log(x, 10.0); }

  /// A simple function to square a value.
  template <typename T>
  static constexpr T Square(T val) { return val * val; }

  // Pow helper functions.
  namespace internal {
    static constexpr double Pow2_lt1(double exp, int id=0) {
      return (id==32) ? 1.0 :
        ( (exp > 0.5) ? (pow2_chart_bits[id]*Pow2_lt1(exp*2.0-1.0,id+1)) : Pow2_lt1(exp*2.0,id+1) );
    }

    static constexpr double Pow2_impl(double exp) {
      return (exp >= 1.0) ? (2.0*Pow2_impl(exp-1.0)) : Pow2_lt1(exp);
    }
  }

  /// A fast (O(log p)) integral-power command.
  template <typename T>
  static constexpr type_if<T, std::is_integral> Pow(T base, T p) {
    if (p <= 0) return 1;
    if (p & 1) return base * Pow(base, p-1); // Odd exponent: strip one mulitple off and recurse.
    return Square( Pow(base,p/2) );          // Even exponent: calc for half and square result.
  }

  /// A fast 2^x command.
  static constexpr double Pow2(double exp) {
    return (exp < 0.0) ? (1.0/internal::Pow2_impl(-exp)) : internal::Pow2_impl(exp);
  }

  /// A fast method for calculating exponents for int types.
  template <typename TYPE>
  static constexpr TYPE IntPow(TYPE base, TYPE exp) {
    emp_assert(exp < 65536); // Set a crazy-high limit for exp to identify problems...
    return exp < 1 ? 1 : (base * IntPow(base, exp-1));
  }

  /// A fast method for calculating exponents on doubles.
  static constexpr double Pow(double base, double exp) {
    // Normally, convert to a base of 2 and then use Pow2.
    // If base is negative, we don't want to deal with imaginary numbers, so use IntPow.
    return (base > 0) ? Pow2(Log2(base) * exp) : IntPow(base,exp);
  }

  // A fast (O(log p)) integer-power command.
  // static constexpr int Pow(int base, int p) {
  //   return (p <= 0) ? 1 : (base * Pow(base, p-1));
  // }

  /// A fast method of calculating e^x
  static constexpr double Exp(double exp) {
    return Pow2(Log2(emp::E) * exp);  // convert to a base of e.
  }


  /// A compile-time int-log calculator (aka, significant bits)
  template <typename TYPE>
  static constexpr int IntLog2(TYPE x) { return x <= 1 ? 0 : (IntLog2(x/2) + 1); }

  /// A compile-time bit counter.
  template <typename TYPE>
  static constexpr int CountOnes(TYPE x) { return x == 0 ? 0 : (CountOnes(x/2) + (x&1)); }

  /// Quick bit-mask generator for low bits.
  template <typename TYPE>
  static constexpr TYPE MaskLow(std::size_t num_bits) {
    return (num_bits == 8*sizeof(TYPE)) ? ((TYPE)-1) : ((((TYPE)1) << num_bits) - 1);
  }

  /// Quick bit-mask generator for high bits.
  template <typename TYPE>
  static constexpr TYPE MaskHigh(std::size_t num_bits) {
    return MaskLow<TYPE>(num_bits) << (8*sizeof(TYPE)-num_bits);
  }

  /// Return the minimum of three values.
  template <typename T> constexpr const T & Min(const T& in1, const T& in2, const T& in3) {
    return std::min(std::min(in1,in2), in3);
  }

  /// A version of Min that allows a variable number of inputs to be compared.
  template <typename T> const T & Min(std::initializer_list<const T&> lst) {
    emp_assert(lst.size > 0); // Nothing to return if nothing in the list!
    auto min_found = lst.begin();
    for (auto it = lst.begin()+1; it < lst.end(); it++) {
      if (*it < *min_found) min_found = it;
    }
    return *min_found;
  }

  /// A version of Max that allows a variable number of inputs to be compared.
  template <typename T> const T & Max(std::initializer_list<const T&> lst) {
    emp_assert(lst.size > 0); // Nothing to return if nothing in the list!
    auto max_found = lst.begin();
    for (auto it = lst.begin()+1; it < lst.end(); it++) {
      if (*it > *max_found) max_found = it;
    }
    return *max_found;
  }

}

#endif
