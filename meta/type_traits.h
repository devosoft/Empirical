//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Extensions on the standard library type traits to handle Empirical classes (such as Ptr).

#ifndef EMP_TYPE_TRAITS_H
#define EMP_TYPE_TRAITS_H

#include <functional>
#include <tuple>
#include <utility>

#include "../base/Ptr.h"

namespace emp {

  // Customized type traits
  template <typename T> struct is_ptr_type           { enum { value = false }; };
  template <typename T> struct is_ptr_type<T*>       { enum { value = true }; };
  template <typename T> struct is_ptr_type<T* const> { enum { value = true }; };
  template <typename T> struct is_ptr_type<Ptr<T>>   { enum { value = true }; };
  template <typename T>
  constexpr bool is_ptr_type_v(const T&) { return is_ptr_type<T>::value; }

  template <typename T> struct remove_ptr_type { };    // Not ponter; should break!
  template <typename T> struct remove_ptr_type<T*>     { using type = T; };
  template <typename T> struct remove_ptr_type<Ptr<T>> { using type = T; };
  template <typename T>
  using remove_ptr_type_t = typename remove_ptr_type<T>::type;
  // @CAO: Make sure we are dealing with const and volitile pointers correctly.

  // Can we convert the first pointer into the second?
  template <typename T1, typename T2> struct ptr_pair {
    static constexpr bool Same() { return false; }
    static constexpr bool SameBase() { return false; }
    static bool ConvertOK(T1 * ptr) { return dynamic_cast<T2*>(ptr); }
  };
  template <typename T> struct ptr_pair<T,T> {
    static constexpr bool Same() { return true; }
    static constexpr bool SameBase() { return true; }
    static constexpr bool ConvertOK(T *) { return true; }
  };
  template <typename T> struct ptr_pair<T, const T> {
    static constexpr bool Same() { return false; }
    static constexpr bool SameBase() { return true; }
    static constexpr bool ConvertOK(T *) { return true; }
  };
  template <typename T> struct ptr_pair<const T, T> {
    static constexpr bool Same() { return false; }
    static constexpr bool SameBase() { return true; }
    static constexpr bool ConvertOK(T *) { return false; }
  };
}


#endif
