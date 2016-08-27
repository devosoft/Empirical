//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  TypeSet represents a collection of types for manipulation (typically for metaprogramming)

#ifndef EMP_TYPE_SET_H
#define EMP_TYPE_SET_H

#include "meta.h"

namespace emp {

  template <typename T1, typename... Ts>
  struct TypeSet {
    template <typename T> constexpr static int Has() { return has_type<T,T1,Ts>(); }

    // Type ID's can be retrieved with
    //   GetID<my_type>() to get the ID associated with specific type my_type
    //   GetID(owner) to get the ID associated with the type of 'owner'
    template <typename T> constexpr static int GetID() { return get_type_index<T,T1,Ts...>(); }
    template <typename T> constexpr static int GetID(const T &) { return get_type_index<T,T1,Ts...>(); }

    constexpr static int GetSize() { return 1+sizeof...(Ts); }

    constexpr static bool IsEmpty() { return false; }
    constexpr static bool IsUnique() { return has_unique_types<T1,Ts...>(); }

    template <typename T> using add_t = TypeSet<T1,Ts...,T>;
    using first_t = T1;
    using last_t = last_type<T1,Ts...>;
    using pop_front_t = TypeSet<Ts...>;
  };

}

#endif
