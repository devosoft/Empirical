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

#include <iostream>

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

    template <int V, typename T_IN, typename T_OUT >
    struct ip_scan {
      using in_pop = typename T_IN::pop;
      using out_pbin = typename T_OUT::template push_back_if_not<T_IN::first, V>;

      using uniq = typename ip_scan< T_IN::first, in_pop, out_pbin>::uniq;
    };
    template <int V, typename T_OUT >
    struct ip_scan <V, IntPack<>, T_OUT> {
      using uniq = T_OUT;
    };

    template <typename T1, typename T2> struct ip_concat;
    template <int... T1s, int... T2s>
    struct ip_concat<IntPack<T1s...>, IntPack<T2s...>> {
      using result = IntPack<T1s..., T2s...>;
    };

    template <typename T_IN, typename T_OUT, bool DONE=false>
    struct ip_while {
      using in_pop = typename T_IN::pop;
      template <int V>
      using out_pbin = typename T_OUT::template push_back_if_not<T_IN::first, V>;
      using out_shift = typename T_OUT::template push<T_IN::first>;

      template <int V>
      using pop_val = typename ip_while< in_pop, out_pbin<V>, T_IN::first == V >::template pop_val<V>;
      template <int V>
      using remove = typename ip_while< in_pop, out_pbin<V> >::template remove<V>;
      // @CAO
    };
    template <typename T_IN, typename T_OUT>
    struct ip_while<T_IN, T_OUT, true> {
      template <int V> using pop_val = typename T_OUT::template append<T_IN>; // Pop done!
      // @CAO
    };
    template <typename T_OUT>
    struct ip_while<IntPack<>, T_OUT, false> {
      template <int V> using pop_val = T_OUT;  // Nothing to pop! (error?)
      template <int V> using remove = T_OUT;   // Nothing left to remove!
      // @CAO
    };


    template <int V, int X, typename T>
    struct ip_push_if_not {
      using result = typename T::template push<V>;
      using back = typename T::template push_back<V>;
    };
    template <int V, typename T>
    struct ip_push_if_not<V,V,T> {
      using result = T;
      using back = T;
    };

    // Setup ==reverse== operation.
    template <typename T> struct ip_reverse;
    template <int V1, int... Vs>
    struct ip_reverse<IntPack<V1, Vs...>> {
      using result = typename ip_reverse< IntPack<Vs...> >::result::template push_back<V1>;
    };
    template <>
    struct ip_reverse<IntPack<>> {
      using result = IntPack<>;
    };

    // Setup ==uniq== operation.
    template <typename T> struct ip_uniq;
    template <int V1, int... Vs>
    struct ip_uniq<IntPack<V1, Vs...>> {
      using result = typename ip_scan<V1+1, IntPack<V1, Vs...>, IntPack<>>::uniq;
    };
    template <>
    struct ip_uniq<IntPack<>> {
      using result = IntPack<>;
    };
  }

  template <int START, int END, int STEP=1>
  using IntPackRange = typename ip_range<(START >= END), START, END, STEP>::type;

  // IntPack with at least one value.
  template <int V1, int... Vs>
  struct IntPack<V1,Vs...> {
    static constexpr int first = V1;

    using this_t = IntPack<V1,Vs...>;
    using pop = IntPack<Vs...>;

    template <int V> using push = IntPack<V, V1, Vs...>;
    template <int V> using push_back = IntPack<V1, Vs..., V>;
    template <int V, int X> using push_if_not = typename ip_push_if_not<V,X,this_t>::result;
    template <int V, int X> using push_back_if_not = typename ip_push_if_not<V,X,this_t>::back;
    template <int V> using pop_val = typename ip_while<this_t, IntPack<>>::template pop_val<V>;
    template <int V> using remove = typename ip_while<this_t, IntPack<>>::template remove<V>;
    template <typename T> using append = typename ip_concat<this_t,T>::result;

    constexpr static bool Has(int V) { return (V==V1) | pop::Has(V); }
    constexpr static int Count(int V) { return pop::Count(V) + (V==V1); }
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

    static void PrintInts(std::ostream & os=std::cout) {
      os << V1;
      if (GetSize() > 1) os << ',';
      pop::PrintInts(os);
    }
  };

  // IntPack with no values.
  template <> struct IntPack<> {
    using this_t = IntPack<>;

    // using reverse = IntPack<>;
    // using uniq = IntPack<>;

    template <int V> using push = IntPack<V>;
    template <int V> using push_back = IntPack<V>;
    template <int V, int X> using push_if_not = typename ip_push_if_not<V,X,IntPack<>>::result;
    template <int V, int X> using push_back_if_not = typename ip_push_if_not<V,X,IntPack<>>::back;
    template <int V> using remove = IntPack<>;
    template <typename T> using append = T;

    constexpr static bool Has(int) { return false; }
    constexpr static int Count(int) { return 0; }
    constexpr static int GetID(int V) { return -100000; }

    constexpr static int SIZE = 0;
    constexpr static int GetSize() { return 0; }

    constexpr static bool IsEmpty() { return true; }
    constexpr static bool IsUnique() { return true; }

    constexpr static int Sum() { return 0; }
    constexpr static int Product() { return 1; }
    constexpr static int Min(int cap) { return cap; }
    constexpr static int Max(int floor) { return floor; }

    static void PrintInts(std::ostream & os=std::cout) { ; }
  };

  namespace pack {
    template <typename T> using reverse = typename ip_reverse<T>::result;
    template <typename T> using uniq = typename ip_uniq<T>::result;
  }
}

#endif
