/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015
 *
 *  @file fixed.hpp
 *  @brief A comprehensive (ideally) fixed-point number representation.
 *
 *  Type-name: emp::fixed
 *
 *  Representation: sign (1 bit), int value (21 bits), frac value (10 bits)
 *  Max value:  2097151.999
 *  Min value: -2097152.999 (@CAO or .000?)
 */

#ifndef EMP_IN_PROGRESS_FIXED_HPP_INCLUDE
#define EMP_IN_PROGRESS_FIXED_HPP_INCLUDE

#include <iostream>

namespace emp {

  template <int FRAC_BITS=10> class fixed {
  private:
    int value;
    static const int int_bits = 31 - FRAC_BITS;

    // Private constructor to allow first direct setting of internal value
    fixed(int in_value, bool) : value(in_value) { ; }
  public:
    fixed() : value(0) { ; }
    fixed(const fixed & in_value) : value (in_value.value) { ; }
    fixed(int in_value) : value(in_value << FRAC_BITS) { ; }
    fixed(double in_value) : value(in_value * (1 << FRAC_BITS)) { ; }
    ~fixed() { ; }

    int bits() const { return value; }
    int AsInt() const { return value >> FRAC_BITS; }
    double AsDouble() const { return ((double) value) / ((double) (1 << FRAC_BITS)); }

    fixed & operator=(const fixed & other) { value = other.value; return *this; }
    fixed & operator=(int other) { value = other << FRAC_BITS; return *this; }
    fixed & operator=(double other) { value = other * (1 << FRAC_BITS); return *this; }

    fixed & operator+=(const fixed & rhs) { value += rhs.value; return *this; }
    fixed & operator-=(const fixed & rhs) { value -= rhs.value; return *this; }
    fixed & operator*=(const fixed & rhs) {
      // Take advantage of (a+b)*(c+d) = ac+ad+bc+bd.  Since bd is too low precision: b*c + a*(c+d)
      const int frac_mask = (1 << FRAC_BITS) - 1;
      value = (value & frac_mask) * (rhs.value >> FRAC_BITS) + (value >> FRAC_BITS) * rhs.value
        + ((value & frac_mask) * (rhs.value & frac_mask) >> FRAC_BITS);
      return *this;
    }
    fixed & operator/=(const fixed & rhs) {
      // @CAO can we take advantage of (a+b)/c = a/c + b/c
      value = (((long) value) << FRAC_BITS) / rhs.value;
      return *this;
    }

    fixed & operator++() { value += (1 << FRAC_BITS); }
    fixed operator++(int) { int old_val = value; operator++(); return fixed(old_val); }
    fixed & operator--() { value -= (1 << FRAC_BITS); }
    fixed operator--(int) { int old_val = value; operator--(); return fixed(old_val); }

    friend fixed operator+(const fixed & lhs, const fixed & rhs) {
      return fixed(lhs.value + rhs.value, true);
    }
    friend fixed operator-(const fixed & lhs, const fixed & rhs) {
      return fixed(lhs.value - rhs.value, true);
    }
    friend fixed operator*(const fixed & lhs, const fixed & rhs) {
      // Take advantage of (a+b)*(c+d) = ac+ad+bc+bd.  Since bd is too low precision: b*c + a*(c+d)
      const int frac_mask = (1 << FRAC_BITS) - 1;
      const int new_value = (lhs.value & frac_mask) * (rhs.value >> FRAC_BITS)
        + (lhs.value >> FRAC_BITS) * rhs.value
        + ((lhs.value & frac_mask) * (rhs.value & frac_mask) >> FRAC_BITS);
      return fixed(new_value, true);
    }
    friend fixed operator/(const fixed & lhs, const fixed & rhs) {
      // @CAO can we take advantage of (a+b)/c = a/c + b/c
      const int new_value = (((long) lhs.value) << FRAC_BITS) / rhs.value;
      return fixed(new_value, true);
    }

    friend bool operator==(const fixed & lhs, const fixed & rhs)
    { return lhs.value == rhs.value; }
    friend bool operator!=(const fixed & lhs, const fixed & rhs)
    { return lhs.value != rhs.value; }
    friend bool operator< (const fixed & lhs, const fixed & rhs)
    { return lhs.value <  rhs.value; }
    friend bool operator<=(const fixed & lhs, const fixed & rhs)
    { return lhs.value <= rhs.value; }
    friend bool operator> (const fixed & lhs, const fixed & rhs)
    { return lhs.value >  rhs.value; }
    friend bool operator>=(const fixed & lhs, const fixed & rhs)
    { return lhs.value >= rhs.value; }

  };


};

// Overload istream and ostream to work with fixed.
template <int FRAC_BITS> std::ostream & operator<<(std::ostream & os,
                                                   const emp::fixed<FRAC_BITS> & input) {
  return os << input.AsDouble();
}

template <int FRAC_BITS> std::istream & operator>>(std::istream & is,
                                                   emp::fixed<FRAC_BITS> & input) {
  double tmp_val;
  is >> tmp_val;
  input = tmp_val;
  return is;
}

#endif // #ifndef EMP_IN_PROGRESS_FIXED_HPP_INCLUDE
