/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2021 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/base/_error_trigger.hpp
 * @brief Error trigger implementation selector
 * @note For internal use.
 */

#pragma once

#ifndef INCLUDE_EMP_BASE_ERROR_TRIGGER_HPP_impl_GUARD
#define INCLUDE_EMP_BASE_ERROR_TRIGGER_HPP_impl_GUARD

#include <iostream>
#include <stddef.h>
#include <string>

/// TDEBUG should trigger its EMP equivalent.
#ifdef TDEBUG
#define EMP_TDEBUG 1
#endif  // #ifdef TDEBUG

namespace emp {

  // Choose the correct version of trigger_emp_error in the pre-processor.  Options are:
  // - _EMSCRIPTEN_ if compiling for the web
  // - TDEBUG - if in test mode
  // - native - otherwise

#if defined(__EMSCRIPTEN__)

  template <typename... Ts>
  void trigger_emp_error(const std::string & filename, const size_t line, Ts &&... args) {
    std::cerr << "Fatal Error (In " << filename << " line " << line << "): ";
    (std::cerr << ... << args);
    std::cerr << "\n";
    abort();
  }

#elif defined(EMP_TDEBUG)  // #if defined(__EMSCRIPTEN__)

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
    std::cout << "TDEBUG: Would-be fatal error (In " << filename << " line " << line << "): ";
    (std::cout << ... << args);
    std::cout << "\n";
    std::stringstream tmp_stream;
    (tmp_stream << ... << args);
    emp::error_info.filename = filename;
    emp::error_info.line_num = line;
    emp::error_info.output   = tmp_stream.str();
    emp::error_thrown        = true;
  }


#else  // #if defined(__EMSCRIPTEN__) : #elif defined(EMP_TDEBUG)

  template <typename... Ts>
  void trigger_emp_error(const std::string & filename, const size_t line, Ts &&... args) {
    std::cerr << "Fatal Error (In " << filename << " line " << line << "): ";
    (std::cerr << ... << args);
    std::cerr << "\n";
    abort();
  }

#endif  // #if defined(__EMSCRIPTEN__) : #elif defined(EMP_TDEBUG) : #else

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_BASE_ERROR_TRIGGER_HPP_impl_GUARD
