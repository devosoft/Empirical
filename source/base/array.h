/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2017
 *
 *  @file array.h
 *  @brief A drop-in wrapper for std::array, adding on bounds checking.
 *  @note Status: RELEASE
 *
 *  If EMP_NDEBUG is set, emp::array is just an alias for std::array.
 *  Otherwise, every time an array is accessed, tests are done to make sure that the
 *  access is legal.
 *
 *  @todo Add tests in array::front and array::back to ensure not empty.
 *  @todo Add tests for get.
 */


#ifndef EMP_ARRAY_H
#define EMP_ARRAY_H

#include <initializer_list>
#include <array>

#include "assert.h"
#include "../meta/TypeID.h"


#ifdef EMP_NDEBUG

namespace emp {
  /// We are in release mode, so emp::array is simply an alias for std::array.
  template <typename T, size_t N> using array = std::array<T,N>;
}


#else // #EMP_NDEBUG *not* set

namespace emp {

  /// We are in debug mode, so emp::array has the same interface as std::array, but with extra
  /// bounds checking.
  template <typename T, size_t N>
  class array : public std::array<T,N> {
  private:
    using stda_t = std::array<T,N>;

  public:
    using value_type = T;

    array() : stda_t() { ; }
    array(const emp::array<T,N> & _in) : stda_t(_in) { ; };
    array(std::initializer_list<T> in_list) : stda_t(in_list) { ; }
    array(const std::array<T,N> & in) : stda_t(in) { ; }         // Emergency fallback conversion.
    ~array() = default;

    /// Access array with extra bounds checking.
    T & operator[](size_t pos) {
      emp_assert(pos < (int) N, pos);
      return stda_t::operator[](pos);
    }

    /// Const access of array with extra bounds checking.
    const T & operator[](size_t pos) const {
      emp_assert(pos < (int) N, pos);
      return stda_t::operator[](pos);
    }

  };

}

#endif

namespace emp{
  /// Ensure that emp::array works with TypeID
  template <typename T, size_t N> struct TypeID<emp::array<T,N>> {
    static std::string GetName() {
      std::stringstream ss;
      ss << "emp::array<" << TypeID<T>::GetName() << "," << N << ">";
      return ss.str();
    }
  };
}



#endif
