/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file  Bool.hpp
 *  @brief A bool representation that doesn't trip up std::vector
 *  @note Status: ALPHA
 *
 */


#ifndef EMP_BOOL_H
#define EMP_BOOL_H

#include <iostream>

#include "../base/assert.hpp"

namespace emp {

  /// @brief A simple replacement for bool type that doesn't trip up std::vector

  class Bool {
  private:
    bool value;

  public:
      // Bool(const Bool & b) : value(b.value) { }
      Bool(bool b=false) : value(b) { }
      Bool & operator=(bool b) { value = b; return *this; }

      /// Conversion of this proxy to Boolean (as an rvalue)
      operator bool() const { return value; }

      // Conversion to a bool reference.
      bool & Value() { return value; }
      bool Value() const { return value; }

      /// Compound assignement operators using Bool as lvalue.
      Bool & operator &=(bool b) { value &= b; return *this; }
      Bool & operator |=(bool b) { value |= b; return *this; }
      Bool & operator ^=(bool b) { value ^= b; return *this; }
      Bool & operator +=(bool b) { value += b; return *this; }
      Bool & operator -=(bool b) { value -= b; return *this; }
      Bool & operator *=(bool b) { value = value && b; return *this; }
      Bool & operator /=(bool b) { emp_assert(b == true); return *this; }
    };

}

namespace std {

  /// Setup operator<< to work with ostream (must be in std to work)
  inline std::istream & operator>>(std::istream & in, emp::Bool & b) {
    in >> b.Value();
    return in;
  }

  /// Setup operator<< to work with ostream (must be in std to work)
  inline std::ostream & operator<<(std::ostream & out, const emp::Bool & b) {
    out << (bool) b;
    return out;
  }

}

#endif
