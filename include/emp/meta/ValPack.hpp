/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2021.
 *
 *  @file ValPack.hpp
 *  @brief A set of values that can be manipulated at compile time (good for metaprogramming)
 *
 *  Any built-in type can be added to ValPack to be manipulated at compile time.
 */

#ifndef EMP_META_VALPACK_HPP_INCLUDE
#define EMP_META_VALPACK_HPP_INCLUDE


#include "meta.hpp"

#include <iostream>
#include <string>

namespace emp {

  // Pre-declaration of ValPack
  template <auto... Ts> struct ValPack;

  // Anonymous implementations of ValPack interface.
  #ifndef DOXYGEN_SHOULD_SKIP_THIS
  namespace internal {
    template <bool DONE, auto START, auto END, auto STEP, auto... VALS>
    struct vp_range {
      static constexpr auto NEXT = START + STEP;
      using type = typename vp_range<(NEXT >= END), NEXT, END, STEP, VALS..., START>::type;
    };
    template <auto START, auto END, auto STEP, auto... VALS>
    struct vp_range <true, START, END, STEP, VALS...> {
      using type = ValPack<VALS...>;
    };

    template <typename T1, typename T2> struct vp_concat;
    template <auto... T1s, auto... T2s>
    struct vp_concat<ValPack<T1s...>, ValPack<T2s...>> {
      using result = ValPack<T1s..., T2s...>;
    };

    template <typename T_IN, typename T_OUT=ValPack<>, bool DONE=false, auto VERSION=0>
    struct vp_loop {
      // Helpers...
      using in_pop = typename T_IN::pop;
      template <auto V> using out_pbin = typename T_OUT::template push_back_if_not<T_IN::first, V>;
      template <auto V, bool D=false> using pnext = vp_loop< in_pop, out_pbin<V>, D, VERSION >;  // Prune

      // Main operations...
      template <auto V> using pop_val = typename pnext<V, T_IN::first==V>::template pop_val<V>;
      template <auto V> using remove = typename pnext<V>::template remove<V>;
      template <auto V> using uniq = typename pnext<V>::template uniq<T_IN::first>;
    };
    template <typename T_IN, typename T_OUT, auto VERSION>
    struct vp_loop<T_IN, T_OUT, true, VERSION> {
      template <auto V> using pop_val = typename T_OUT::template append<T_IN>; // Pop done!
    };
    template <typename T_OUT, auto VERSION>
    struct vp_loop<ValPack<>, T_OUT, false, VERSION> {
      template <auto V> using pop_val = T_OUT;  // Nothing to pop! (error?)
      template <auto V> using remove = T_OUT;   // Nothing left to remove!
      template <auto V> using uniq = T_OUT;     // Nothing left to check!
    };

    // Implement == ip_push_if_not ==
    template <auto V, auto X, typename T>
    struct vp_push_if_not {
      using result = typename T::template push<V>;
      using back = typename T::template push_back<V>;
    };
    template <auto V, typename T>
    struct vp_push_if_not<V,V,T> {
      using result = T;
      using back = T;
    };

    // Setup ==reverse== operation.
    template <typename T> struct vp_reverse;
    template <auto V1, auto... Vs> struct vp_reverse<ValPack<V1, Vs...>> {
      using result = typename vp_reverse< ValPack<Vs...> >::result::template push_back<V1>;
    };
    template <> struct vp_reverse<ValPack<>> { using result = ValPack<>; };

    // Setup ==uniq== operation.
    template <typename T> struct vp_uniq;
    template <auto V1, auto... Vs> struct vp_uniq<ValPack<V1, Vs...>> {
      using result = typename vp_loop<ValPack<V1, Vs...>, ValPack<>, false, 1>::template uniq<V1+1>;
    };
    template <> struct vp_uniq<ValPack<>> { using result = ValPack<>; };

    // Setup ==sort== operation.
    template <typename T_IN, typename T_OUT>
    struct vp_sort_impl {
      template <auto V> using spop = typename T_IN::template pop_val<V>;
      template <auto V> using snext = vp_sort_impl< spop<V>, typename T_OUT::template push_back<V> >;
      template <auto V> using sort = typename snext<V>::template sort< spop<V>::Min(T_IN::first) >;
    };
    template <typename T_OUT>
    struct vp_sort_impl<ValPack<>, T_OUT> {
      template <auto V> using sort = T_OUT;     // Nothing left to sort!
    };

    template <typename T> struct vp_sort;
    template <auto V1, auto... Vs> struct vp_sort<ValPack<V1, Vs...>> {
      using vp = ValPack<V1, Vs...>;
      using result = typename vp_sort_impl<vp, ValPack<>>::template sort<vp::Min()>;
    };
    template <> struct vp_sort<ValPack<>> { using result = ValPack<>; };
  } // End internal namespace
  #endif // DOXYGEN_SHOULD_SKIP_THIS

  /// Generate a ValPack with a specified range of values.
  template <auto START, auto END, auto STEP=1>
  using ValPackRange = typename internal::vp_range<(START >= END), START, END, STEP>::type;

  /// Generate a count from 0 to COUNT-1 (so a total of COUNT values)
  template <auto COUNT>
  using ValPackCount = ValPackRange<0, COUNT, 1>;

  /// ValPack with at least one value.
  template <auto V1, auto... Vs>
  struct ValPack<V1,Vs...> {
    /// First value in ValPack
    static constexpr auto first = V1;

    /// Number of values in ValPack
    constexpr static size_t SIZE = 1+sizeof...(Vs);

    using this_t = ValPack<V1,Vs...>;
    using pop = ValPack<Vs...>;

    /// Add a value to the front of an ValPack
    template <auto V> using push = ValPack<V, V1, Vs...>;

    /// Add a value to the back of an ValPack
    template <auto V> using push_back = ValPack<V1, Vs..., V>;

    /// Push V onto front of an ValPack if it does not equal X
    template <auto V, auto X> using push_if_not = typename internal::vp_push_if_not<V,X,this_t>::result;

    /// Push V onto back of an ValPack if it does not equal X
    template <auto V, auto X> using push_back_if_not = typename internal::vp_push_if_not<V,X,this_t>::back;

    /// Remove the first time value V appears from an ValPack
    template <auto V> using pop_val = typename internal::vp_loop<this_t, ValPack<>, false, 2>::template pop_val<V>;

    /// Remove the all appearances of value V from an ValPack
    template <auto V> using remove = typename internal::vp_loop<this_t, ValPack<>, false, 3>::template remove<V>;

    /// Append one whole ValPack to the end of another.
    template <typename T> using append = typename internal::vp_concat<this_t,T>::result;

    /// Apply to a specified template with ValPack as template arguments.
    template <template <int...> class TEMPLATE> using apply = TEMPLATE<V1, Vs...>;


    /// ---=== Member Functions ===---

    /// Return wheter an ValPack contains the value V.
    template <typename T>
    constexpr static bool Has(T val) { return (val==V1) | pop::Has(val); }

    /// Count the number of occurances of value V in ValPack.
    template <typename T>
    constexpr static size_t Count(T val) { return pop::Count(val) + (val==V1); }

    /// Determine the position at which V appears in ValPack.
    template <typename T>
    constexpr static int GetID(T val) { return (val==V1) ? 0 : (1+pop::GetID(val)); }

    /// Function to retrieve number of elements in ValPack
    constexpr static size_t GetSize() { return SIZE; }

    /// Determine if there are NO value in an ValPack
    constexpr static bool IsEmpty() { return false; }

    /// Determine if all values in ValPack are different from each other.
    constexpr static bool IsUnique() { return pop::IsUnique() && !pop::Has(V1); }

    /// Add up all values in an ValPack
    constexpr static auto Sum() { return V1 + pop::Sum(); }

    /// Multiply together all value in an ValPack
    constexpr static auto Product() { return V1 * pop::Product(); }

    /// Find the smallest value in an ValPack, to a maximum of cap.
    template <typename T>
    constexpr static auto Min(T cap) { return cap < pop::Min(V1) ? cap : pop::Min(V1); }

    /// Find the overall smallest value in an ValPack
    constexpr static auto Min() { return pop::Min(V1); }

    /// Find the maximum value in an ValPack, to a minimum of floor.
    template <typename T>
    constexpr static auto Max(T floor) { return floor > pop::Max(V1) ? floor : pop::Max(V1); }

    /// Find the overall maximum value in an ValPack.
    constexpr static auto Max() { return pop::Max(V1); }

    /// Use each value in an ValPack as an index and return results as a tuple.
    template <typename T>
    constexpr static auto ApplyIndex(T && container) {
      return std::make_tuple(container[V1], container[Vs]...);
    }

    /// Convert all values from an ValPack into a string, treating each as a char.
    static std::string ToString() {
      return std::string(1, (char) V1) + pop::ToString();
    }

    /// Print all values in an ValPack into a stream.
    static void PrintVals(std::ostream & os=std::cout) {
      os << V1;
      if (GetSize() > 1) os << ',';
      pop::PrintVals(os);
    }
  };

  // ValPack with no values.
  template <> struct ValPack<> {
    using this_t = ValPack<>;

    template <auto V> using push = ValPack<V>;
    template <auto V> using push_back = ValPack<V>;
    template <auto V, auto X> using push_if_not = typename internal::vp_push_if_not<V,X,ValPack<>>::result;
    template <auto V, auto X> using push_back_if_not = typename internal::vp_push_if_not<V,X,ValPack<>>::back;
    template <auto V> using pop_val = ValPack<>;  // No value to pop!  Faulure?
    template <auto V> using remove = ValPack<>;
    template <typename T> using append = T;

    template <typename T>
    constexpr static bool Has(T) { return false; }
    template <typename T>
    constexpr static size_t Count(T) { return 0; }
    template <typename T>
    constexpr static int GetID(T) { return -100000; }

    constexpr static size_t SIZE = 0;
    constexpr static size_t GetSize() { return 0; }

    constexpr static bool IsEmpty() { return true; }
    constexpr static bool IsUnique() { return true; }

    constexpr static double Sum() { return 0.0; }
    constexpr static double Product() { return 1.0; }
    template <typename T>
    constexpr static auto Min(T cap) { return cap; }
    template <typename T>
    constexpr static auto Max(T floor) { return floor; }

    static std::string ToString() { return ""; }

    static void PrintVals(std::ostream & /* os */=std::cout) { ; }
  };

  namespace pack {
    template <typename T> using reverse = typename internal::vp_reverse<T>::result;
    template <typename T> using uniq = typename internal::vp_uniq<T>::result;

    template <typename T> using sort = typename internal::vp_sort<T>::result;
    template <typename T> using Rsort = reverse< sort<T> >;
    template <typename T> using Usort = uniq< sort<T> >;
    template <typename T> using RUsort = reverse< Usort<T> >;
  }
}

#endif // #ifndef EMP_META_VALPACK_HPP_INCLUDE
