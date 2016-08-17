//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file contains functions that all run at compile time to produce a constant value.


#ifndef EMP_CONST_UTILS_H
#define EMP_CONST_UTILS_H

#include "const.h"
#include "const_tables.h"

namespace emp {
namespace constant {

  namespace internal {
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

  // And a more generic compile-time log calculator.
  static constexpr double Log2(double x) {
    return (x < 1.0) ? internal::Log2_frac(x) : internal::Log2_pos(x);
  }

  // Log varients
  static constexpr double Log(double x, double base) { return Log2(x) / Log2(base); }
  static constexpr double Ln(double x) { return Log(x, emp::E); }   // Natural Log...
  static constexpr double Log10(double x) { return Log(x, 10.0); }

  static constexpr double Pow2(double exp) {
    //return (exp >= 1.0) ? (2.0*Pow2(exp-1.0)) : pow2_chart_0_1[(int)(exp*1024.0)];
    return (exp >= 1.0) ? (2.0*Pow2(exp-1.0)) : internal::InterpolateTable(pow2_chart_0_1, exp, 1024);
  }

  static constexpr double Pow(double base, double exp) {
    return Pow2(Log2(base) * exp);  // convert to a base of 2.
  }

  // A compile-time exponentiation calculator.
  template <typename TYPE>
  static constexpr TYPE IntPow(TYPE base, TYPE exp) {
    return exp < 1 ? 1 : (base * IntPow(base, exp-1));
  }

  // A compile-time int-log calculator (aka, significant bits)
  template <typename TYPE>
  static constexpr int IntLog2(TYPE x) {
    return x <= 1 ? 0 : (IntLog2(x/2) + 1);
  }

  // A compile-time bit counter.
  template <typename TYPE>
  static constexpr int CountOnes(TYPE x) {
    return x == 0 ? 0 : (CountOnes(x/2) + (x&1));
  }

  // Quick bit-mask generators...
  template <typename TYPE>
  static constexpr TYPE MaskLow(int num_bits) {
    return (num_bits == 8*sizeof(TYPE)) ? -1 : ((((TYPE)1) << num_bits) - 1);
  }

  template <typename TYPE>
  static constexpr TYPE MaskHigh(int num_bits) {
    return MaskLow<TYPE>(num_bits) << (8*sizeof(TYPE)-num_bits);
  }


}
}

#endif
