/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2021 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/base/_tdebug_error_trigger.hpp
 * @brief Non-terminating error trigger implementation for unit testing.
 * @note For internal use.
 */

#pragma once

#ifndef INCLUDE_EMP_BASE_TDEBUG_ERROR_TRIGGER_HPP_impl_GUARD
#define INCLUDE_EMP_BASE_TDEBUG_ERROR_TRIGGER_HPP_impl_GUARD


#include <stddef.h>
#include <string>

namespace emp {

  struct ErrorInfo {
    std::string filename;
    size_t line_num;
    std::string output;
  };

  ErrorInfo error_info;
  bool error_thrown = false;

  void error_clear() { emp::error_thrown = false; }

  template <typename... Ts>
  void trigger_emp_error(const std::string & filename, const size_t line, Ts &&... args) {
    std::cout << "Would-be fatal error (In " << filename << " line " << line << "): ";
    (std::cout << ... << args);
    std::cout << "\n";
    std::stringstream tmp_stream;
    (tmp_stream << ... << args);
    emp::error_info.filename = filename;
    emp::error_info.line_num = line;
    emp::error_info.output   = tmp_stream.str();
    emp::error_thrown        = true;
  }

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_BASE_TDEBUG_ERROR_TRIGGER_HPP_impl_GUARD
