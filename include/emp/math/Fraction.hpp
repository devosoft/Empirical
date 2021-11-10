/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file Fraction.hpp
 *  @brief Tools to maintain a more exact fraction (rather than lose precision as a double)
 *  @note Status: ALPHA
 */

#ifndef EMP_MATH_FRACTION_HPP_INCLUDE
#define EMP_MATH_FRACTION_HPP_INCLUDE

#include "math.hpp"

namespace emp {

  class Fraction {
  private:
    int64_t num;    // Numerator
    int64_t denom;  // Denominator

    void Reduce() {
      if (denom == 0) return;                 // Undefined value!
      if (num == 0) { denom = 1; return; }    // Zero value!

      // Shuffle all negative values to num.
      if (denom < 0) {
        denom = -denom;
        num = -num;
      }
      const uint64_t gcd = emp::GCD(num, denom);
      num /= gcd;
      denom /= gcd;
    }
  public:
    Fraction(int64_t in_num=0, int64_t in_denom=1) : num=in_num, denom=in_denom { }
    Fraction(const Fraction &) = default;

    int64_t GetNumerator() const { return num; }
    int64_t GetDenomonator() const { return denom; }
  };

#endif // #ifndef EMP_MATH_FRACTION_HPP_INCLUDE
