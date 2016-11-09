//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  TypePack represents a collection of types for manipulation (typically for metaprogramming)
//
//  TypePacks are static structues that provide a large set of mechanisms to access and adjust
//  the included types.
//
//
//  Developer notes:
//  * If IntPack is at a lower level than TypePack, we'll be able to have slightly more complex
//    functions below.  For example, returning sets of positions OR providing sets of positions
//    for the purposes of reorganizing types.
//    - GetIDPack could return ALL ID's for a type that appears more than once.

#ifndef EMP_TYPE_PACK_H
#define EMP_TYPE_PACK_H

#include "meta.h"

namespace emp {

  // Pre-declaration of TypePack
  template <typename... Ts> struct TypePack;

  // Create a null type for padding.
  struct null_t {};

  // Anonymous helpers for TypePack
  namespace {
    // Create add N copies of the same type to the end of a TypePack.
    template <typename START, typename T, int N>
    struct tp_pad { using type = typename tp_pad<START,T,N-1>::type::template add<T>; };
    template <typename START, typename T>
    struct tp_pad<START, T,0> { using type = START; };

    // Helper for shifting a specified number of types to TypePack T1 from TypePack T2.
    // Example: to shrink, move the specified number of types to empty TypePack and return it.
    //          to merge, move all of one TypePack over to the other and return it.
    template <int S, typename T1, typename T2>
    struct tp_shift {
      using move_t = typename T2::first_t;
      using inc_t = typename T1::template add<move_t>;
      using dec_t = typename T2::pop;
      using type1 = typename tp_shift<S-1, inc_t, dec_t>::type1;
      using type2 = typename tp_shift<S-1, inc_t, dec_t>::type2;
    };
    template <typename T1, typename T2>
    struct tp_shift<0, T1, T2> {
      using type1 = T1;
      using type2 = T2;
    };

    // Filters create a TypePack with the element if the filter is true, empty if false.
    template <typename T, bool> struct tp_filter1 { using type=TypePack<T>; };
    template <typename T> struct tp_filter1<T,false> { using type=TypePack<>; };

    template <typename T, template <typename...> class FILTER, int N>
    struct tp_filter {
      using cur_t = typename T::first_t;                                 // Isolate the first type
      using other_tp = typename T::pop;                                  // Isolate remaining types
      constexpr static bool cur_result = test_type<FILTER,cur_t>();      // Run filter of cur type
      using cur_ftp = typename tp_filter1<cur_t, cur_result>::type;      // Use cur type if true
      using other_ftp = typename tp_filter<other_tp, FILTER, N-1>::type; // Recurse
      using type = typename cur_ftp::template merge< other_ftp >;        // Merge
    };
    template <typename T, template <typename...> class FILTER>
      struct tp_filter<T,FILTER,0> { using type = TypePack<>; };

    template <typename T, template <typename...> class FILTER>
      using tp_filter_t = typename tp_filter<T,FILTER,T::SIZE>::type;

    // Wrappers create a TypePack with the wrapped element if the filter is true, empty if false.
    template <typename T, template <typename...> class W, bool> struct tp_wrap1
      { using type=TypePack<W<T>>; };
    template <typename T, template <typename...> class W> struct tp_wrap1<T,W,false>
      { using type=TypePack<>; };

    template <typename T, template <typename...> class W, int N> struct tp_wrap {
      using cur_t = typename T::first_t;                               // Isolate the first type
      using other_tp = typename T::pop;                                // Isolate remaining types
      constexpr static bool cur_result = test_type<W,cur_t>();         // Use wrap to filter cur type
      using cur_ftp = typename tp_wrap1<cur_t, W, cur_result>::type;   // Use cur type if true
      using other_ftp = typename tp_wrap<other_tp, W, N-1>::type;      // Recurse
      using type = typename cur_ftp::template merge< other_ftp >;      // Merge
    };
    template <typename T, template <typename...> class W>
      struct tp_wrap<T,W,0> { using type = TypePack<>; };

    template <typename T, template <typename...> class W>
      using tp_wrap_t = typename tp_wrap<T,W,T::SIZE>::type;
  }

  template <typename T, int N> using TypePackFill = typename tp_pad<TypePack<>,T,N>::type;

  // Specialized TypePack with at least one type.
  template <typename T1, typename... Ts>
  struct TypePack<T1, Ts...> {
    template <typename T> constexpr static bool Has() { return has_type<T,T1,Ts...>(); }
    template <typename T> constexpr static int Count() { return count_type<T,T1,Ts...>(); }

    // Type ID's can be retrieved with
    //   GetID<my_type>() to get the ID associated with specific type my_type
    //   GetID(owner) to get the ID associated with the type of 'owner'
    template <typename T> constexpr static int GetID() { return get_type_index<T,T1,Ts...>(); }
    template <typename T> constexpr static int GetID(const T &) { return get_type_index<T,T1,Ts...>(); }

    constexpr static int SIZE = 1+sizeof...(Ts);
    constexpr static int GetSize() { return SIZE; }

    constexpr static bool IsEmpty() { return false; }
    constexpr static bool IsUnique() { return has_unique_types<T1,Ts...>(); }

    // Get the type associated with a specified position in the pack.
    template <int POS> using get = pack_id<POS, T1, Ts...>;

    // Other type lookups
    using this_t = TypePack<T1, Ts...>;
    using first_t = T1;
    using last_t = last_type<T1,Ts...>;

    // Modifications
    template <typename... T> using push_front = TypePack<T...,T1,Ts...>;
    template <typename... T> using push_back = TypePack<T1,Ts...,T...>;
    template <typename... T> using add = TypePack<T1,Ts...,T...>;    // Same as push_back_t...

    using pop = TypePack<Ts...>;
    template <int N> using popN = typename tp_shift<N, TypePack<>, this_t>::type2;
    template <int N> using shrink = typename tp_shift<N, TypePack<>, this_t>::type1;

    template <typename T, int N=1> using pad = typename tp_pad<this_t,T,N>::type;
    template <int N, typename DEFAULT=null_t>
      using resize = typename pad<DEFAULT,(N>SIZE)?(N-SIZE):0>::template shrink<N>;
    template <typename IN> using merge = typename tp_shift<IN::SIZE, this_t, IN>::type1;
    using reverse = typename pop::reverse::template push_back<T1>;
    using rotate = typename pop::template push_back<T1>;
    template <int ID, typename T>
      using set = typename shrink<ID>::template push_back<T>::template merge<popN<ID+1>>;

    // Choose a set of specific positions.
    template <int... Ps> using select = TypePack< get<Ps>... >;

    // Conversions
    template <typename RETURN_T> using to_function_t = RETURN_T(T1,Ts...);
    template <template <typename...> class TEMPLATE> using apply = TEMPLATE<T1, Ts...>;

    // Filters
    template <template <typename...> class FILTER> using filter = tp_filter_t<this_t, FILTER>;
    template <template <typename...> class FILTER>
      using find_t = typename tp_filter_t<this_t, FILTER>::first_t;
    template <template <typename...> class WRAPPER> using wrap = tp_wrap_t<this_t, WRAPPER>;
  };

  // Specialized TypePack with no types.
  template <>
  struct TypePack<> {
    template <typename T> constexpr static bool Has() { return false; }
    template <typename T> constexpr static int Count() { return 0; }

    // GetID() NOT IMPLEMENTED since no ID's are available.
    constexpr static int SIZE = 0;
    constexpr static int GetSize() { return 0; }
    constexpr static bool IsEmpty() { return true; }
    constexpr static bool IsUnique() { return true; }

    // pop_t not implemented, since no types are available.

    using this_t = TypePack<>;
    using first_t = null_t;
    using last_t = null_t;

    template <typename... T> using push_front = TypePack<T...>;
    template <typename... T> using push_back = TypePack<T...>;
    template <typename... T> using add = TypePack<T...>;

    template <typename T, int N=1> using pad = typename tp_pad<this_t,T,N>::type;
    template <int N, typename DEFAULT=null_t> using resize = pad<DEFAULT,N>;

    template <typename IN> using merge = IN;
    using reverse = this_t;
    using rotate = this_t;

    template <typename RETURN_T> using to_function_t = RETURN_T();

    template <template <typename...> class TEMPLATE> using apply = TEMPLATE<>;

    // There's nothing to filter, so return this_t (TypePack<>) or null_t
    template <template <typename...> class FILTER> using filter = this_t;
    template <template <typename...> class FILTER> using find_t = null_t;
    template <template <typename...> class WRAPPER> using wrap = this_t;
  };
}

#endif
