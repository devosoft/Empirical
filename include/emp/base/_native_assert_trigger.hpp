/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2020 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/base/_native_assert_trigger.hpp
 * @brief Native assert trigger implementation.
 * @note For internal use.
 */

#pragma once

#ifndef INCLUDE_EMP_BASE_NATIVE_ASSERT_TRIGGER_HPP_impl_GUARD
#define INCLUDE_EMP_BASE_NATIVE_ASSERT_TRIGGER_HPP_impl_GUARD

#include <iostream>
#include <stddef.h>
#include <string>

#include "concepts.hpp"

namespace emp {

  constexpr bool assert_on = true;

  /// Base case for assert_to_stream...
  inline void assert_to_stream(std::ostream &) { ; }

  /// Convert information about the next variable to a string and recurse...
  template <typename T, typename... EXTRA>
  void assert_to_stream(std::ostream & ss, std::string name, T && val, EXTRA &&... extra) {
    ss << ((name[0] == '"') ? std::string{"MESSAGE: "} : (name + " = "));
    if constexpr (emp::is_streamable<T>()) {
      ss << val << '\n';
    } else {
      ss << "(non-streamable type)" << '\n';
    }

    assert_to_stream(ss, std::forward<EXTRA>(extra)...);
  }

  template <typename... EXTRA>
  bool assert_trigger(std::string filename, size_t line, std::string expr, EXTRA &&... extra) {
    std::cerr << "Assert Error (In " << filename << ":" << line << "): " << expr << std::endl;
    assert_to_stream(std::cerr, std::forward<EXTRA>(extra)...);

    return true;  // do process subsequent abort
  }

}  // namespace emp


#endif  // #ifndef INCLUDE_EMP_BASE_NATIVE_ASSERT_TRIGGER_HPP_impl_GUARD
