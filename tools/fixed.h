#ifndef EMP_FIXED_H
#define EMP_FIXED_H

/////////////////////////////////////////////////////////////////////////////////////////////
//
//  A comprehensive (ideally) fixed-point number representation.
//
//  Type-name: emp::fixed
//
//  Representation: sign (1 bit), int value (21 bits), frac value (10 bits)
//  Max value:  2097151.999
//  Min value: -2097152.999 (@CAO or .000?)
//

#include <iostream>

namespace emp {

  template <int FRAC_BITS=10> class fixed {
  private:
    int value;
    static const int int_bits = 31 - FRAC_BITS;
    static const int frac_mask = (1 << FRAC_BITS) - 1;

    // Private constructor to allow first direct setting of internal value
    fixed(int in_value, bool test_bool) : value(in_value) { (void) test_bool; }
  public:
    fixed() : value(0) { ; }
    fixed(const fixed & in_value) : value (in_value.value) { ; }
    fixed(int in_value) : value(in_value << FRAC_BITS) { ; }
    fixed(double in_value) : value(in_value * (1 << FRAC_BITS)) { ; }
    ~fixed() { ; }

    fixed & operator=(const fixed & other) { value = other.value; return *this; }
    fixed & operator=(int other) { value = other << FRAC_BITS; return *this; }
    fixed & operator=(double other) { value = other * (1 << FRAC_BITS); return *this; }

    int AsInt() const { return value >> FRAC_BITS; }
    double AsDouble() const { return ((double) value) / ((double) (1 << FRAC_BITS)); }

    fixed operator+(const fixed & other) const { return fixed(value + other.value, true); }
    fixed operator-(const fixed & other) const { return fixed(value - other.value, true); }
    fixed operator*(const fixed & other) const {
      // Take advantage of (a+b)*(c+d) = ac+ad+bc+bd.  Since bd is too low precision: b*c + a*(c+d)
      const int new_value = (value & frac_mask) * (other.value >> FRAC_BITS)
        + (value >> FRAC_BITS) * other.value
        + ((value & frac_mask) * (other.value & frac_mask) >> FRAC_BITS);
      return fixed(new_value, true);
    }
    fixed operator/(const fixed & other) const {
      // @CAO can we take advantage of (a+b)/c = a/c + b/c
      const int new_value = (((long) value) << FRAC_BITS) / other.value;
      return fixed(new_value, true);
    }
  };
  
  
  // Overload ostream to work with fixed.
  template <int FRAC_BITS> std::ostream & operator<<(std::ostream & os, const fixed<FRAC_BITS> & input) { 
    return os << input.AsDouble();
  }
  
};
  
#endif
