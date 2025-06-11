/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2020 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/base/_tdebug_assert_trigger.hpp
 * @brief Non-terminating assert trigger implementation for unit testing.
 * @note For internal use.
 */

#pragma once

#ifndef INCLUDE_EMP_BASE_TDEBUG_ASSERT_TRIGGER_HPP_GUARD
#define INCLUDE_EMP_BASE_TDEBUG_ASSERT_TRIGGER_HPP_GUARD

#include <stddef.h>
#include <string>

namespace emp {

  constexpr bool assert_on = true;
  struct AssertFailInfo {
    std::string filename;
    int line_num;
    std::string error;
  };
  AssertFailInfo assert_fail_info;
  bool assert_last_fail = false;

  template <typename... EXTRA>
  bool assert_trigger(
    std::string filename, size_t line, std::string expr, EXTRA &&...
  ) {
    emp::assert_fail_info.filename = filename;
    emp::assert_fail_info.line_num = line;
    emp::assert_fail_info.error = expr;
    emp::assert_last_fail = true;

    return false; // do not process subsequent abort
  }

  void assert_clear() { emp::assert_last_fail = false; }

} // namespace emp

#endif // #ifndef EMP_BASE_TDEBUG_ASSERT_TRIGGER_HPP_INCLUDE
