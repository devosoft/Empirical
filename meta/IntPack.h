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
  template <int... Ts> struct IntPack;

  // IntPack with at least one value.
  template <int V1, int... Vs>
  struct IntPack<V1,Vs...> {
    template <int V> constexpr static bool Has() { return (V==V1) | IntPack<Vs...>::template Has<V>(); }
    template <int V> constexpr static int Count() { return IntPack<Vs...>::template Count<V>() + (V==V1); }

    // Type ID's can be retrieved with
    //   GetID<my_type>() to get the ID associated with specific type my_type
    //   GetID(owner) to get the ID associated with the type of 'owner'
    template <int V> constexpr static int GetID() { return (V==V1) ? 0 : (1+IntPack<Vs...>::template GetID<V>()); }
    constexpr static int GetID(int V) { return (V==V1) ? 0 : (1+IntPack<Vs...>::template GetID<V>()); }

    constexpr static int SIZE = 1+sizeof...(Vs);
    constexpr static int GetSize() { return SIZE; }

    constexpr static bool IsEmpty() { return false; }

    //constexpr static bool IsUnique() { return has_unique_types<T1,Ts...>(); }
    // Sum()
    // Product()
  };

  // IntPack with no values.
  template <> struct IntPack<> {
    template <int V> constexpr static bool Has() { return false; }
    template <int V> constexpr static int Count() { return 0; }

    // Type ID's can be retrieved with
    //   GetID<my_type>() to get the ID associated with specific type my_type
    //   GetID(owner) to get the ID associated with the type of 'owner'
    template <int V> constexpr static int GetID() { return -100000; }
    constexpr static int GetID(int V) { return -100000; }

    constexpr static int SIZE = 0;
    constexpr static int GetSize() { return 0; }

    constexpr static bool IsEmpty() { return true; }
  };
}

#endif
