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

  template <int SIZE>
  class ce_string {
  private:
    char str[SIZE];
  public:
    constexpr ce_string(const ce_string<SIZE> & S) {
      for (int i=0; i < SIZE; i++) str[i] = S[i];
    }

    template <typename T, size_t N>
    constexpr ce_string(T (&S)[N]) {
      static_assert(N == SIZE, "ce_string must be initialized with correct size string!");
      for (int i=0; i < N; i++) str[i] = S[i];
    }

    constexpr ce_string & operator=(const ce_string & S) {
      for (int i=0; i < SIZE; i++) str[i] = S[i];
      return *this;
    }

    constexpr int size() const { return SIZE; }

    constexpr char operator[](int i) const { return str[i]; }
  };

  template <typename T, size_t N>
  constexpr auto const_string(T (&str)[N]) {
    return ce_string<N>(str);
  }

}

#endif
