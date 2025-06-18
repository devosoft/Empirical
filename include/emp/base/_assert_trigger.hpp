/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2020 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/base/_assert_trigger.hpp
 * @brief Assert trigger implementation selector.
 * @note For internal use.
 */

#pragma once

#ifndef INCLUDE_EMP_BASE_ASSERT_TRIGGER_HPP_impl_GUARD
#define INCLUDE_EMP_BASE_ASSERT_TRIGGER_HPP_impl_GUARD

#include <iostream>
#include <sstream>
#include <stddef.h>
#include <string>

#include "concepts.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif  // #ifdef __EMSCRIPTEN__

/// TDEBUG should trigger its EMP equivalent.
#ifdef TDEBUG
#define EMP_TDEBUG 1
#endif  // #ifdef TDEBUG

namespace emp {

  // Helper tools for asserts
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

  // Include the appropriate version of assert.
  // Options are __EMSCRIPTEN__, EMP_TDEBUG, or regular.
#if defined(__EMSCRIPTEN__)

#ifndef DOXYGEN_SHOULD_SKIP_THIS

  static int TripAssert() {
    static int trip_count = 0;
    return ++trip_count;
  }

  template <typename... EXTRA>
  bool assert_trigger(std::string filename, size_t line, std::string expr, EXTRA &&... extra) {
    std::stringstream ss;
    ss << "Assert Error (In " << filename << " line " << line << "): " << expr << '\n';
    assert_to_stream(ss, std::forward<EXTRA>(extra)...);
    if (emp::TripAssert() <= 3) {
      EM_ASM(
        {
          msg = UTF8ToString($0);
          if (typeof alert == "undefined") {
            // node polyfill
            globalThis.alert = console.log;
          }
          alert(msg);
        },
        ss.str().c_str());
    }

    // Print the current state of the stack.
    EM_ASM(console.log('Callstack:\n' + stackTrace()););

    return false;
  }

#endif  // #ifndef DOXYGEN_SHOULD_SKIP_THIS

#elif defined(EMP_TDEBUG)  // #if defined(__EMSCRIPTEN__)

  constexpr bool assert_on = true;

  struct AssertFailInfo {
    std::string filename;
    int line_num;
    std::string error;
  };

  AssertFailInfo assert_fail_info;
  bool assert_last_fail = false;

  template <typename... EXTRA>
  bool assert_trigger(std::string filename, size_t line, std::string expr, EXTRA &&...) {
    emp::assert_fail_info.filename = filename;
    emp::assert_fail_info.line_num = line;
    emp::assert_fail_info.error    = expr;
    emp::assert_last_fail          = true;

    return false;  // do not process subsequent abort
  }

  void assert_clear() { emp::assert_last_fail = false; }

#else  // #if defined(__EMSCRIPTEN__) : #elif defined(EMP_TDEBUG)

  constexpr bool assert_on = true;

  template <typename... EXTRA>
  bool assert_trigger(std::string filename, size_t line, std::string expr, EXTRA &&... extra) {
    std::cerr << "Assert Error (In " << filename << ":" << line << "): " << expr << std::endl;
    assert_to_stream(std::cerr, std::forward<EXTRA>(extra)...);

    return true;  // do process subsequent abort
  }

#endif  // #if defined(__EMSCRIPTEN__) : #elif defined(EMP_TDEBUG) : #else

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_BASE_ASSERT_TRIGGER_HPP_impl_GUARD
