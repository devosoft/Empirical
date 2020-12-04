/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file  AlignedCharArrayUnion.hpp
 *  @brief A suitably aligned and sized character array member which can hold
 *  elements of any type. These types may be arrays, structs, or any other
 *  types. This exposes a buffer member which can be used as suitable storage
 *  for a placement new of any of these types.
 *
 *  @note Adapted from the LLVM Project, under the Apache License v2.0 with
 *  LLVM Exceptions. See https://llvm.org/LICENSE.txt for license information.
 *  SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#ifndef EMP_ALIGNED_CHAR_ARRAY_UNION_HPP
#define EMP_ALIGNED_CHAR_ARRAY_UNION_HPP

#include <cstddef>

namespace emp {

namespace detail {

template <typename T, typename... Ts> class AlignerImpl {
  T t;
  AlignerImpl<Ts...> rest;
  AlignerImpl() = delete;
};

template <typename T> class AlignerImpl<T> {
  T t;
  AlignerImpl() = delete;
};

template <typename T, typename... Ts> union SizerImpl {
  char arr[sizeof(T)];
  SizerImpl<Ts...> rest;
};

template <typename T> union SizerImpl<T> { char arr[sizeof(T)]; };
} // end namespace detail

/// A suitably aligned and sized character array member which can hold elements
/// of any type.
///
/// These types may be arrays, structs, or any other types. This exposes a
/// `buffer` member which can be used as suitable storage for a placement new of
/// any of these types.
template <typename T, typename... Ts> struct AlignedCharArrayUnion {
  alignas(::emp::detail::AlignerImpl<T, Ts...>) char buffer[sizeof(
      emp::detail::SizerImpl<T, Ts...>)];
};

} // end namespace emp

#endif // #ifndef EMP_ALIGNED_CHAR_ARRAY_UNION_HPP
