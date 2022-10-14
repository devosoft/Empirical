/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2020.
 *
 *  @file  math.hpp
 *  @brief Useful mathematical functions (that are constexpr when possible.)
 *  @note Status: BETA (though new functions are added frequently)
 */


#ifndef EMP_MATH_H
#define EMP_MATH_H

#include <initializer_list>
#include <algorithm>
#include <cmath>

#include "../base/assert.hpp"
#include "../meta/reflection.hpp"
#include "Random.hpp"
#include "constants.hpp"

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
      res -= r.GetUInt(2);
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

  #ifndef DOXYGEN_SHOULD_SKIP_THIS
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
  #endif // DOXYGEN_SHOULD_SKIP_THIS

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
  #ifndef DOXYGEN_SHOULD_SKIP_THIS
  namespace internal {
    static constexpr double Pow2_lt1(double exp, int id=0) {
      return (id==32) ? 1.0 :
        ( (exp > 0.5) ? (pow2_chart_bits[id]*Pow2_lt1(exp*2.0-1.0,id+1)) : Pow2_lt1(exp*2.0,id+1) );
    }

    static constexpr double Pow2_impl(double exp) {
      return (exp >= 1.0) ? (2.0*Pow2_impl(exp-1.0)) : Pow2_lt1(exp);
    }
  }

  namespace internal {
  /// A fast (O(log p)) integral-power command.
  template <typename T>
  static constexpr T PowIntImpl(T base, T p) {
    if (p <= 0) return 1;
    if (p & 1) return base * PowIntImpl(base, p-1); // Odd exponent: strip one mulitple off and recurse.
    return emp::Square( PowIntImpl(base,p/2) );          // Even exponent: calc for half and square result.
  }
  } // namespace internal
  #endif // DOXYGEN_SHOULD_SKIP_THIS

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

  #ifndef DOXYGEN_SHOULD_SKIP_THIS
  namespace internal {
  /// A fast method for calculating exponents on doubles.
  static constexpr double PowDoubleImpl(double base, double exp) {
    // Normally, convert to a base of 2 and then use Pow2.
    // If base is negative, we don't want to deal with imaginary numbers, so use IntPow.
    return (base > 0)
      ? emp::Pow2(emp::Log2(base) * exp)
      : emp::IntPow(base,exp);
  }

  // adapted from https://stackoverflow.com/a/30836042
  // prevents argument from being used for type deduction
  template <typename T> struct identity { typedef T type; };

  } // namespace internal
  #endif // DOXYGEN_SHOULD_SKIP_THIS

  /// A fast method for calculating exponents on doubles or integral types.
  /// Uses if constexpr to work around compiler bug in Emscripten (issue #296).
  template<typename T>
  static constexpr T Pow(
    T base, typename internal::identity<T>::type exp
  ) {
    // TODO cpp20, C++20 replace with std::is_constant_evaluated
    // adapted from https://stackoverflow.com/a/62610143
    // exclude clang versions with compiler bug https://reviews.llvm.org/D35190
    #if defined(__clang__) && __clang_major__>=9 || defined(__GNUC__) && !defined(__clang__)
    // if base is not known at compile time, use std::pow which is faster
    if ( !__builtin_constant_p( base ) ) return std::pow(base, exp);
    // otherwise, use constexpr-friendly implementations
    else
    #endif
    if constexpr( std::is_integral<T>::value ){
      return internal::PowIntImpl(base, exp);
    } else return internal::PowDoubleImpl(base, exp);
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

  /// Returns the next power of two (in 64-bits) that is strictly greater than A.
  /// Returns zero on overflow.
  inline uint64_t NextPowerOf2(uint64_t A) {
    A |= (A >> 1);
    A |= (A >> 2);
    A |= (A >> 4);
    A |= (A >> 8);
    A |= (A >> 16);
    A |= (A >> 32);
    return A + 1;
  }

  /// Tests if a number is a power of two.
  inline constexpr bool IsPowerOf2(const size_t x) {
    return x > 0 && !(x & (x - 1));
  }

  inline constexpr int Factorial(int i) {
    int result = 1;
    while (i > 0) {
      result *= i;
      i--;
    }
    return result;
  }

  /// Toggle an input bool.
  inline bool Toggle(bool & in_bool) { return (in_bool = !in_bool); }

  /// Combine bools to AND them all together.
  inline constexpr bool AllTrue() { return true; }
  template <typename... Ts>
  inline bool AllTrue(bool result, Ts... OTHER) {
    return result && AllTrue(OTHER...);
  }

  /// Combine bools to OR them all together.
  inline constexpr bool AnyTrue() { return false; }
  template <typename... Ts>
  inline bool AnyTrue(bool result, Ts... OTHER) {
    return result || AnyTrue(OTHER...);
  }

  /// Calculate 2**exp fast by manually setting double exponent field.
  /// Gives incorrect answer for exp outside of representable range of double
  /// exponent field.
  inline constexpr double exp2_overflow_unsafe(const int64_t exp) {
    // benchmarking result https://quick-bench.com/q/8Q7UMP35zo3t2C70HvEz4AzyWVE
    // ieee double has 11-bit exponent field
    emp_assert(exp > -1023, exp);
    emp_assert(exp <= 1024, exp);
    static_assert(sizeof(double) == sizeof(uint64_t));
    static_assert(std::numeric_limits<double>::is_iec559);

    union {
      uint64_t i;
      double d;
    } result{};
    result.i = (exp + 1023) << 52;
    emp_assert(result.d > 0.0, exp);
    return result.d;
  }

  /// Calculate 2**exp fast by manually setting float exponent field.
  /// Gives incorrect answer for exp outside of representable range of float
  /// exponent field.
  inline constexpr float exp2f_overflow_unsafe(const int32_t exp) {
    // ieee float has 8-bit exponent field
    emp_assert(exp > -127);
    emp_assert(exp <= 128);
    static_assert(sizeof(float) == sizeof(uint32_t));
    static_assert(std::numeric_limits<float>::is_iec559);

    union {
      uint32_t i;
      float f;
    } result{};
    result.i = (exp + 127) << 23;
    emp_assert(result.f > 0.0f);
    return result.f;
  }

  /// Calculate 2**exp fast for small exp, but handle large exp correctly.
  inline constexpr double exp2(const int64_t exp) {
    // benchmarking result https://quick-bench.com/q/8Q7UMP35zo3t2C70HvEz4AzyWVE
    if (exp <= -1023l || exp > 1024l) return std::exp2(exp);
    else return exp2_overflow_unsafe(exp);
  }

  /// Calculate 2**exp fast for small exp, but handle large exp correctly.
  inline constexpr float exp2f(const int32_t exp) {
    if (exp <= -127 || exp > 128) return std::exp2f(exp);
    else return exp2f_overflow_unsafe(exp);
  }

}

#endif
