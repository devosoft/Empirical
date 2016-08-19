//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  TypeSet represents a collection of types for manipulation (typically for metaprogramming)

#ifndef EMP_TYPE_SET_H
#define EMP_TYPE_SET_H

#include "../tools/meta.h"

namespace emp {

  template <typename... Ts>
  struct TypeSet {
    // Type ID's can be retrieved with
    //   GetID<my_type>() to get the ID associated with specific type my_type
    //   GetID(owner) to get the ID associated with the type of 'owner'
    template <typename T> constexpr static int GetID() { return get_type_index<T,Ts...>(); }
    template <typename T> constexpr static int GetID(const T &) { return get_type_index<T,Ts...>(); }

    constexpr static int GetSize() { return sizeof...(Ts); }

    constexpr static bool IsUnique() { return has_unique_types<Ts...>(); }

    template <typename T>
    using AddType = TypeSet<Ts...,T>;
  };

};

#endif
