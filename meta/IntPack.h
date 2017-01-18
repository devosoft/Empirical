//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  IntPack represents a collection of integers for easy manipulation and use in
//  template specification (typically for metaprogramming)
//
//
//  Developer Notes:
//  * Can we shift this to a ValPack where the type is specified and then the values?  then
//    we would be able to create an IntPack as a specialty on ValPack.

#ifndef EMP_INT_PACK_H
#define EMP_INT_PACK_H

#include "meta.h"

namespace emp {

  // Pre-declaration of IntPack
  template <int... Ts> struct IntPack;

  namespace {
    template <bool DONE, int START, int END, int STEP, int... VALS>
    struct ip_range {
      static constexpr int NEXT = START + STEP;
      using type = typename ip_range<(NEXT >= END), NEXT, END, STEP, VALS..., START>::type;
    };
    template <int START, int END, int STEP, int... VALS>
    struct ip_range <true, START, END, STEP, VALS...> {
      using type = IntPack<VALS...>;
    };
  }

  template <int START, int END, int STEP=1>
  using IntPackRange = typename ip_range<(START >= END), START, END, STEP>::type;

  // IntPack with at least one value.
  template <int V1, int... Vs>
  struct IntPack<V1,Vs...> {
    static constexpr int first = V1;

    using pop = IntPack<Vs...>;

    template <int V> using push = IntPack<V, V1, Vs...>;
    template <int V> using push_back = IntPack<V1, Vs..., V>;

    template <int V> constexpr static bool Has() { return (V==V1) | pop::template Has<V>(); }
    constexpr static bool Has(int V) { return (V==V1) | pop::Has(V); }
    template <int V> constexpr static int Count() { return pop::template Count<V>() + (V==V1); }
    constexpr static int Count(int V) { return pop::Count(V) + (V==V1); }

    // Type ID's can be retrieved with
    //   GetID<my_type>() to get the ID associated with specific type my_type
    //   GetID(owner) to get the ID associated with the type of 'owner'
    template <int V> constexpr static int GetID() { return (V==V1) ? 0 : (1+pop::template GetID<V>()); }
    constexpr static int GetID(int V) { return (V==V1) ? 0 : (1+pop::GetID(V)); }

    constexpr static int SIZE = 1+sizeof...(Vs);
    constexpr static int GetSize() { return SIZE; }

    constexpr static bool IsEmpty() { return false; }
    constexpr static bool IsUnique() { return pop::IsUnique() && !pop::Has(V1); }

    constexpr static int Sum() { return V1 + pop::Sum(); }
    constexpr static int Product() { return V1 * pop::Product(); }
    constexpr static int Min(int cap) { return cap < pop::Min(V1) ? cap : pop::Min(V1); }
    constexpr static int Min() { return pop::Min(V1); }
    constexpr static int Max(int floor) { return floor > pop::Max(V1) ? floor : pop::Max(V1); }
    constexpr static int Max() { return pop::Max(V1); }
  };

  // IntPack with no values.
  template <> struct IntPack<> {
    template <int V> using push = IntPack<V>;
    template <int V> using push_back = IntPack<V>;

    template <int V> constexpr static bool Has() { return false; }
    constexpr static bool Has(int) { return false; }
    template <int V> constexpr static int Count() { return 0; }
    constexpr static int Count(int) { return 0; }

    // Type ID's can be retrieved with
    //   GetID<my_type>() to get the ID associated with specific type my_type
    //   GetID(owner) to get the ID associated with the type of 'owner'
    template <int V> constexpr static int GetID() { return -100000; }
    constexpr static int GetID(int V) { return -100000; }

    constexpr static int SIZE = 0;
    constexpr static int GetSize() { return 0; }

    constexpr static bool IsEmpty() { return true; }
    constexpr static bool IsUnique() { return true; }

    constexpr static int Sum() { return 0; }
    constexpr static int Product() { return 1; }
    constexpr static int Min(int cap) { return cap; }
    constexpr static int Max(int floor) { return floor; }
  };
}

#endif
