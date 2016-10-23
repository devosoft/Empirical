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
    template <typename... Ts> struct ts_merge_impl;

    template <typename... IN, typename... Ts>
    struct ts_merge_impl<TypeSet<IN...>, Ts...> {
      using type = TypeSet<Ts..., IN...>;
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

    using first_t = T1;
    using last_t = last_type<T1,Ts...>;
    using pop_front_t = TypeSet<Ts...>;

    template <typename T> using push_front_t = TypeSet<T,T1,Ts...>;
    template <typename T> using push_back_t = TypeSet<T1,Ts...,T>;
    template <typename T> using add_t = TypeSet<T1,Ts...,T>;           // Same as push_back_t...
    template <typename IN> using merge_t = typename ts_merge_impl<IN, T1, Ts...>::type;
  };

  // Specialized TypeSet with no types.
  template <>
  struct TypeSet<> {
    template <typename T> constexpr static int Has() { return false; }

    // GetID() NOT IMPLEMENTED since no ID's are available.
    constexpr static int GetSize() { return 0; }
    constexpr static bool IsEmpty() { return true; }
    constexpr static bool IsUnique() { return true; }

    // first_t, last_t, and pop_front_t not implemented, since no types are available.
    // (could implement with null_t)

    template <typename T> using push_front_t = TypeSet<T>;
    template <typename T> using push_back_t = TypeSet<T>;
    template <typename T> using add_t = TypeSet<T>;
    template <typename IN> using merge_t = IN;
  };
}

#endif
