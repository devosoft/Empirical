//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  ce_string defines a limited string object for use within a constexpr class or function.
//
//  Developer note:
//   @CAO: Not ready from prime time-- for some reason I can't get Set to work as constexpr.
//  REQUIRES: C++14

#ifndef EMP_CS_STRING_HH
#define EMP_CS_STRING_HH

#include <type_traits>
#include "functions.h"

namespace emp {

  class ce_string {
  public:
    using size_t = std::size_t;

  private:
    const char * m_str;
    size_t m_size;

  public:
    template<size_t N>
    constexpr ce_string(const char (&in)[N]) : m_str(in), m_size(N-1) { ; }
    constexpr ce_string(const ce_string & in) = default;

    constexpr bool operator==(const ce_string & in) const {
      if (m_size != in.m_size) return false;
      for (auto i = 0; i < m_size; i++) if (m_str[i] != in.m_str[i]) return false;
      return true;
    }
    constexpr bool operator!=(const ce_string & in) const { return !(*this == in); }
    constexpr bool operator<(const ce_string & in) const {
      auto min_size = std::min(m_size, in.m_size);
      for (auto i = 0; i < min_size; i++) {
        if (m_str[i] != in.m_str[i]) return (m_str[i] < in.m_str[i]);
      }
      return m_size < in.m_size;
    }
    constexpr bool operator>(const ce_string & in) const { return (in < *this); }
    constexpr bool operator<=(const ce_string & in) const { return !(in < *this); }
    constexpr bool operator>=(const ce_string & in) const { return !(*this < in); }

    constexpr size_t size() const { return m_size; }

    constexpr char operator[](const size_t pos) const {
      // static_assert(pos < m_size, "ce_array index out of range.");
      return m_str[pos];
    }
  };

}

#endif
