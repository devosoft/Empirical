//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  IntPack represents a collection of integers for easy manipulation and use in
//  template specification (typically for metaprogramming)

#ifndef EMP_INT_PACK_H
#define EMP_INT_PACK_H

#include "meta.h"

namespace emp {

  // Pre-declaration of IntPack
  template <typename... Ts> struct IntPack;

  // IntPack with at least one value.
  template <int V, int... Vs>
  struct IntPack {
  };

  // IntPack with no values.
  template <> struct IntPack<> {
  };
}
