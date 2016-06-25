//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  ce_string defines a limited string object for use within a constexpr class or function.
//
//  REQUIRES: C++14

#ifndef EMP_CS_STRING_HH
#define EMP_CS_STRING_HH

#include <type_traits>
#include "functions.h"

namespace emp {

  class ce_string {
  private:
    const char * m_str;
    int m_size;

  public:
    template<std::size_t N>
    constexpr ce_string(const char (&in)[N]) : m_str(in), m_size(N-1) { ; }
    constexpr ce_string(const ce_string & in) = default;

    // constexpr ce_string & operator=(const ce_string & S) {
    //   for (int i=0; i < SIZE; i++) str[i] = S[i];
    //   return *this;
    // }

    constexpr int size() const { return m_size; }

    template<std::size_t N>
    constexpr bool Set(const char (&in)[N]) { m_str = in; m_size = N; }

    // constexpr void Set(const int pos, const char val) {
    //   // static_assert(pos < m_size, "ce_array index out of range.");
    //   return m_str[pos] = val;
    // }
    constexpr char operator[](const int pos) const {
      // static_assert(pos < m_size, "ce_array index out of range.");
      return m_str[pos];
    }
  };

}

#endif
