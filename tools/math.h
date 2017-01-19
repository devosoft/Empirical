//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file contains useful mathematical functions (that are constexpr when possible.)


#ifndef EMP_MATH_H
#define EMP_MATH_H

#include <initializer_list>
#include <algorithm>
#include "const.h"

namespace emp {

  namespace {
    // A compile-time log calculator for values [1,2)
    static constexpr double Log2_base(double x) {
      return log2_chart_1_2[(int)((x-1.0)*1024)];
      // return InterpolateTable(log2_chart_1_2, x-1.0, 1024);
    }

    // A compile-time log calculator for values < 1
    static constexpr double Log2_frac(double x) {
      return (x >= 1.0) ? Log2_base(x) : (Log2_frac(x*2.0) - 1.0);
    }

    // A compile-time log calculator for values >= 2
    static constexpr double Log2_pos(double x) {
      return (x < 2.0) ? Log2_base(x) : (Log2_pos(x/2.0) + 1.0);
    }

  }

  /// @endcond

  /// Compile-time log base 2 calculator.
  static constexpr double Log2(double x) {
    return (x < 1.0) ? Log2_frac(x) : Log2_pos(x);
  }

  /// Compile-time log calculator
  static constexpr double Log(double x, double base=10.0) { return Log2(x) / Log2(base); }
  /// Compile-time natural log calculator
  static constexpr double Ln(double x) { return Log(x, emp::E); }   // Natural Log...
  /// Compile-time log base 10 calculator.
  static constexpr double Log10(double x) { return Log(x, 10.0); }

  namespace {
    static constexpr double Pow2_lt1(double exp, int id=0) {
      return (id==32) ? 1.0 :
        ( (exp > 0.5) ? (pow2_chart_bits[id]*Pow2_lt1(exp*2.0-1.0,id+1)) : Pow2_lt1(exp*2.0,id+1) );
    }

    static constexpr double Pow2_impl(double exp) {
      return (exp >= 1.0) ? (2.0*Pow2_impl(exp-1.0)) : Pow2_lt1(exp);
    }
  }

  // A fast (O(log p)) integer-power command.
  static constexpr int Pow(int base, int p) {
    return (p <= 0) ? 1 : base * Pow(base, p-1);
  }

  static constexpr double Pow2(double exp) {
    return (exp < 0.0) ? (1.0/Pow2_impl(-exp)) : Pow2_impl(exp);
  }

  static constexpr double Pow(double base, double exp) {
    return Pow2(Log2(base) * exp);  // convert to a base of 2.
  }

  static constexpr double Exp(double exp) {
    return Pow2(Log2(emp::E) * exp);  // convert to a base of e.
  }

  /// A compile-time exponentiation calculator.
  template <typename TYPE>
  static constexpr TYPE IntPow(TYPE base, TYPE exp) {
    return exp < 1 ? 1 : (base * IntPow(base, exp-1));
  }

  /// A compile-time int-log calculator (aka, significant bits)
  template <typename TYPE>
  static constexpr int IntLog2(TYPE x) { return x <= 1 ? 0 : (IntLog2(x/2) + 1); }

  /// A compile-time bit counter.
  template <typename TYPE>
  static constexpr int CountOnes(TYPE x) { return x == 0 ? 0 : (CountOnes(x/2) + (x&1)); }

  /// Quick bit-mask generators...
  template <typename TYPE>
  static constexpr TYPE MaskLow(std::size_t num_bits) {
    return (num_bits == 8*sizeof(TYPE)) ? ((TYPE)-1) : ((((TYPE)1) << num_bits) - 1);
  }

  template <typename TYPE>
  static constexpr TYPE MaskHigh(std::size_t num_bits) {
    return MaskLow<TYPE>(num_bits) << (8*sizeof(TYPE)-num_bits);
  }

  /// % is actually remainder; this is a proper modulus command that handles negative #'s correctly
  inline constexpr int Mod(int in_val, int mod_val) {
    return (in_val < 0) ? (in_val % mod_val + mod_val) : (in_val % mod_val);
  }

  template <typename T> constexpr const T & Min(const T& in1, const T& in2, const T& in3) {
    return std::min(std::min(in1,in2), in3);
  }

  // Build a min and max that allows a variable number of inputs to be compared.
  template <typename T> const T & Min(std::initializer_list<const T&> lst) {
    emp_assert(lst.size > 0); // Nothing to return if nothing in the list!
    auto min_found = lst.begin();
    for (auto it = lst.begin()+1; it < lst.end(); it++) {
      if (*it < *min_found) min_found = it;
    }
    return *min_found;
  }

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
