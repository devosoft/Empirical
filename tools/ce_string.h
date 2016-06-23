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
    template <class T>
    constexpr ce_string(T && S) {
      static_assert(SIZE == ::size(S), "ce_string must be initialized with correct size string!");
      for (int i=0; i < SIZE; i++) str[i] = S[i];
    }
    
    constexpr int size() const { return SIZE; }
  };

}

#endif
