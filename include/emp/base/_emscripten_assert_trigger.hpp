/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file _assert_trigger.hpp
 *  @brief Assert trigger implementation.
 *  @note For internal use.
 */

#ifndef EMP_EMSCRIPTEN_ASSERT_TRIGGER_HPP
#define EMP_EMSCRIPTEN_ASSERT_TRIGGER_HPP

#include <iostream>
#include <string>
#include <sstream>

#include <emscripten.h>

#include "_is_streamable.hpp"

namespace emp {

  static int TripAssert() {
    static int trip_count = 0;
    return ++trip_count;
  }

  /// Base case for assert_print...
  inline void assert_print(std::stringstream &) { ; }

  /// Print out information about the next variable and recurse...
  template <typename T, typename... EXTRA>
  void assert_print(std::stringstream & ss, std::string name, T && val, EXTRA &&... extra) {
    if constexpr ( emp::is_streamable<std::stringstream, T>::value ) {
      ss << name << ": [" << val << "]" << std::endl;
    } else ss << name << ": (non-streamable type)" << std::endl;
    assert_print(ss, std::forward<EXTRA>(extra)...);
  }

  template <typename... EXTRA>
  bool assert_trigger(std::string filename, size_t line, std::string expr, EXTRA &&... extra) {
    std::stringstream ss;
    ss << "Assert Error (In " << filename << " line " << line << "): " << expr << '\n';
    assert_print(ss, std::forward<EXTRA>(extra)...);
    if (emp::TripAssert() <= 3) {
      EM_ASM({
        msg = UTF8ToString($0);
        if (typeof alert == "undefined") {
          // node polyfill
          globalThis.alert = console.log;
        }
        alert(msg);
      }, ss.str().c_str());
    }

    // Print the current state of the stack.
    EM_ASM( console.log('Callstack:\n' + stackTrace()); );

    return false;

  }

} // namespace emp

#endif // #ifndef EMP_EMSCRIPTEN_ASSERT_TRIGGER_HPP
