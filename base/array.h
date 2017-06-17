//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This class is a drop-in wrapper for std::array, adding on bounds checking.
//  If EMP_NDEBUG is set then it reverts back to std::array.


#ifndef EMP_ARRAY_H
#define EMP_ARRAY_H

#include <initializer_list>
#include <array>

#include "assert.h"
#include "../meta/TypeID.h"


#ifdef EMP_NDEBUG

namespace emp {
  template <typename T, size_t N> using array = std::array<T,N>;
}


#else // #EMP_NDEBUG *not* set

namespace emp {

  template <typename T, size_t N>
  class array : public std::array<T,N> {
  private:
    using stda_t = std::array<T,N>;

  public:
    using value_type = T;

    array() : a() { ; }
    array(const emp::array<T,N> & _in) : stda_t(_in) { ; };
    array(std::initializer_list<T> in_list) : stda_t(in_list) { ; }
    array(const std::array<T,N> & in) : stda_t(in) { ; }         // Emergency fallback conversion.
    ~array() = default;

    T & operator[](size_t pos) {
      emp_assert(pos < (int) N, pos);
      return stda_t::operator[](pos);
    }

    const T & operator[](size_t pos) const {
      emp_assert(pos < (int) N, pos);
      return stda_t::operator[](pos);
    }

  };

}

#endif

namespace emp{
  // Ensure that emp::array works with TypeID
  template <typename T, size_t N> struct TypeID<emp::array<T,N>> {
    static std::string GetName() {
      std::stringstream ss;
      ss << "emp::array<" << TypeID<T>::GetName() << "," << N << ">";
      return ss.str();
    }
  };
}



#endif
