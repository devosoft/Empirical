//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  TypeSet represents a collection of types for manipulation (typically for metaprogramming)

#ifndef EMP_TYPE_SET_H
#define EMP_TYPE_SET_H

#include "meta.h"

namespace emp {

  // Pre-declaration of TypeSet
  template <typename... Ts> struct TypeSet;

  // Anonymous helpers for TypeSet
  namespace {
    // Helper for shifting a specified number of types to TypeSet T1 from TypeSet T2.
    // Example: to crop, move a specified number of types to empty TypeSet and return it.
    //          to merge, move all of one typeset over to the other and return it.
    template <int S, typename T1, typename T2>
    struct ts_shift {
      using move_t = typename T2::first_t;
      using inc_t = typename T1::template add_t<move_t>;
      using dec_t = typename T2::pop_t;
      using type1 = typename ts_shift<S-1, inc_t, dec_t>::type1;
      using type2 = typename ts_shift<S-1, inc_t, dec_t>::type2;
    };
    template <typename T1, typename T2>
    struct ts_shift<0, T1, T2> {
      using type1 = T1;
      using type2 = T2;
    };
  }

  // Generic TypeSet (not implemented since specializations cover all cases)
  template <typename... Ts> struct TypeSet;

  // Specialized TypeSet with at least one type.
  template <typename T1, typename... Ts>
  struct TypeSet<T1, Ts...> {
    template <typename T> constexpr static int Has() { return has_type<T,T1,Ts...>(); }

    // Type ID's can be retrieved with
    //   GetID<my_type>() to get the ID associated with specific type my_type
    //   GetID(owner) to get the ID associated with the type of 'owner'
    template <typename T> constexpr static int GetID() { return get_type_index<T,T1,Ts...>(); }
    template <typename T> constexpr static int GetID(const T &) { return get_type_index<T,T1,Ts...>(); }

    constexpr static int GetSize() { return 1+sizeof...(Ts); }

    constexpr static bool IsEmpty() { return false; }
    constexpr static bool IsUnique() { return has_unique_types<T1,Ts...>(); }

    // Get the type associated with a specified position in the pack.
    template <int POS> using type = pack_id<POS, T1, Ts...>;

    using this_t = TypeSet<T1, Ts...>;
    using first_t = T1;
    using last_t = last_type<T1,Ts...>;
    using pop_t = TypeSet<Ts...>;

    template <int S> using crop_t = typename ts_shift<S, TypeSet<>, this_t>::type1;

    template <typename T> using push_front_t = TypeSet<T,T1,Ts...>;
    template <typename T> using push_back_t = TypeSet<T1,Ts...,T>;
    template <typename T> using add_t = TypeSet<T1,Ts...,T>;           // Same as push_back_t...
    template <typename IN> using merge_t = typename ts_shift<IN::GetSize(), this_t, IN>::type1;

    template <typename RETURN_T> using to_function_t = RETURN_T(T1,Ts...);
    template <template <typename...> class TEMPLATE>
    using apply_t = TEMPLATE<T1, Ts...>;
  };

  // Specialized TypeSet with no types.
  template <>
  struct TypeSet<> {
    template <typename T> constexpr static int Has() { return false; }

    // GetID() NOT IMPLEMENTED since no ID's are available.
    constexpr static int GetSize() { return 0; }
    constexpr static bool IsEmpty() { return true; }
    constexpr static bool IsUnique() { return true; }

    // first_t, last_t, and pop_t not implemented, since no types are available.
    // (could implement with null_t)

    template <typename T> using push_front_t = TypeSet<T>;
    template <typename T> using push_back_t = TypeSet<T>;
    template <typename T> using add_t = TypeSet<T>;
    template <typename IN> using merge_t = IN;

    template <typename RETURN_T> using to_function_t = RETURN_T();
  };
}

#endif
