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

#ifndef INCLUDE_EMP_BASE_NATIVE_ASSERT_TRIGGER_HPP_GUARD
#define INCLUDE_EMP_BASE_NATIVE_ASSERT_TRIGGER_HPP_GUARD

#include <iostream>
#include <sstream>
#include <stddef.h>
#include <string>

#include "concepts.hpp"

namespace emp {

  constexpr bool assert_on = true;

  /// Base case for assert_print...
  inline void assert_print() { ; }

  /// Print out information about the next variable and recurse...
  template <typename T, typename... EXTRA>
  void assert_print(std::string name, T && val, EXTRA &&... extra) {
    if constexpr (emp::canStreamTo<decltype(std::cerr), T>) {
      // If we had a literal string fed in, print it as a message.
      if (name[0] == '"') {
        std::cerr << "MESSAGE: " << val << std::endl;
      }
      // Otherwise assume that we have a variable and print that.
      else {
        std::cerr << name << ": [" << val << "]" << std::endl;
      }
    } else {
      std::cerr << name << ": (non-streamable type)" << std::endl;
    }

    assert_print(std::forward<EXTRA>(extra)...);
  }

  template <typename... EXTRA>
  bool assert_trigger(std::string filename, size_t line, std::string expr, EXTRA &&... extra) {
    std::cerr << "Assert Error (In " << filename << " line " << line << "): " << expr << std::endl;
    assert_print(std::forward<EXTRA>(extra)...);

    return true;  // do process subsequent abort
  }

}  // namespace emp


#endif  // #ifndef EMP_BASE_NATIVE_ASSERT_TRIGGER_HPP_INCLUDE
