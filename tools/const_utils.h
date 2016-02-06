//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file contains functions that all run at compile time to produce a constant
//  value.


#ifndef EMP_CONST_UTILS_H
#define EMP_CONST_UTILS_H

namespace emp {
namespace constant {

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
