/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2017
 *
 *  @file assert.h
 *  @brief A more dynamic replacement for standard library asserts.
 *  @note Status: RELEASE
 *
 *  A replacement for the system-level assert.h, called "emp_assert"
 *  Added functionality:
 *   - If compiled with Emscripten, will provide pop-up alerts in a web browser.
 *   - emp_assert can take additional arguments.  If the assert is triggered, those extra
 *     arguments will be evaluated and printed.
 *   - if NDEBUG -or- EMP_NDEBUG is defined, the expression in emp_assert() is not evaluated.
 *   - if EMP_TDEBUG is defined, emp_assert() goes into test mode and records failures, but
 *     does not abort.  (useful for unit tests of asserts)
 *
 *  Example:
 *
 *     int a = 6;
 *     emp_assert(a==5, a);
 *
 *  When compiled in debug mode (i.e. without the -DNDEBUG flag), this will trigger an assertion
 *  error and print the value of a.
 *
 *
 *  @todo: Add emp_assert_warning() for non-terminating assert.  Should be able to disable with
 *         a command-line option (-DEMP_NO_WARNINGS)
 */

#ifndef EMP_ASSERT_H
#define EMP_ASSERT_H

#include <iostream>
#include <string>
#include <sstream>

#include "macros.h"

/// @cond DEFINES

/// If we are in emscripten, make sure to include the header.
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

/// NDEBUG and TDEBUG should trigger their EMP equivilents.
#ifdef NDEBUG
#define EMP_NDEBUG
#endif

#ifdef TDEBUG
#define EMP_TDEBUG
#endif


/// Helper macros used throughout...
#define emp_assert_TO_PAIR(X) EMP_STRINGIFY(X) , X

/// Turn off all asserts in EMP_NDEBUG
#ifdef EMP_NDEBUG
namespace emp {
  constexpr bool assert_on = false;
}

// GROUP 1:   --- Debug OFF ---

/// Ideally, this assert should use the expression (to prevent compiler error), but should not
/// generate any assembly code.  For now, just make it blank (other options commented out)
#define emp_assert(...)
// #define emp_assert(EXPR) ((void) sizeof(EXPR) )
// #define emp_assert(EXPR, ...) { constexpr bool __emp_assert_tmp = false && (EXPR); (void) __emp_assert_tmp; }

// Asserts to check only when in Emscripten should also be disabled.
#define emp_emscripten_assert(...)


// GROUP 2:   --- Unit Testing ON ---
#elif defined(EMP_TDEBUG)           // EMP_NDEBUG not set, but EMP_TDEBUG is!

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
  bool assert_trigger(std::string filename, size_t line, std::string expr) {
    emp::assert_fail_info.filename = __FILE__;
    emp::assert_fail_info.line_num = __LINE__;
    emp::assert_fail_info.error = expr;
    emp::assert_last_fail = true;

    return true;
  }

  void assert_clear() { emp::assert_last_fail = false; }
}

// Unit Testing ON

#define emp_assert(...)                                                                       \
  do {                                                                                        \
    !(EMP_GET_ARG_1(__VA_ARGS__, ~)) &&                                                       \
    emp::assert_trigger(__FILE__, __LINE__, EMP_STRINGIFY( EMP_GET_ARG_1(__VA_ARGS__, ~) ));  \
  } while(0)

// Unit-testing asserts to check only when in Emscripten should depend on if we are in Emscripten
#ifdef EMSCRIPTEN
#define emp_emscripten_assert(...) emp_assert(__VA_ARGS__)
#else
#define emp_emscripten_assert(...)
#endif


// GROUP 3:   --- Emscripten debug ON ---
#elif EMSCRIPTEN  // Neither EMP_NDEBUG nor EMP_TDEBUG set, but compiling with Emscripten

namespace emp {
  constexpr bool assert_on = true;
  static int TripAssert() {
    static int trip_count = 0;
    return ++trip_count;
  }

  /// Base case for assert_print...
  void assert_print(std::stringstream &) { ; }

  /// Print out information about the next variable and recurse...
  template <typename T, typename... EXTRA>
  void assert_print(std::stringstream & ss, std::string name, T && val, EXTRA &&... extra) {
    ss << name << ": [" << val << "]" << std::endl;
    assert_print(ss, std::forward<EXTRA>(extra)...);
  }

  template <typename IGNORE, typename... EXTRA>
  bool assert_trigger(std::string filename, size_t line, std::string expr, IGNORE, EXTRA &&... extra) {
    std::stringstream ss;
    ss << "Assert Error (In " << filename << " line " << line << "): " << expr << '\n';
    assert_print(ss, std::forward<EXTRA>(extra)...);
    if (emp::TripAssert() <= 3) {
      EM_ASM_ARGS({ msg = UTF8ToString($0); alert(msg); }, ss.str().c_str());
    }

    // Print the current state of the stack.
    EM_ASM( console.log('Callstack:\n' + stackTrace()); );
    return true;
  }
}

// Debug; Emscripten ON

#define emp_assert(...)                                                                       \
  do {                                                                                        \
    !(EMP_GET_ARG_1(__VA_ARGS__, ~)) &&                                                       \
    emp::assert_trigger(__FILE__, __LINE__, EMP_WRAP_ARGS(emp_assert_TO_PAIR, __VA_ARGS__) ); \
  } while(0)

// Emscripten asserts should be on since we are in Emscripten
#define emp_emscripten_assert(...) emp_assert(__VA_ARGS__)


// GROUP 3:   --- Debug ON, but Emscripten OFF ---
#else

namespace emp {
  constexpr bool assert_on = true;

  /// Base case for assert_print...
  void assert_print() { ; }

  /// Print out information about the next variable and recurse...
  template <typename T, typename... EXTRA>
  void assert_print(std::string name, T && val, EXTRA &&... extra) {
    std::cerr << name << ": [" << val << "]" << std::endl;
    assert_print(std::forward<EXTRA>(extra)...);
  }

  template <typename IGNORE, typename... EXTRA>
  bool assert_trigger(std::string filename, size_t line, std::string expr, IGNORE, EXTRA &&... extra) {
    std::cerr << "Assert Error (In " << filename << " line " << line
              <<  "): " << expr << std::endl;
    assert_print(std::forward<EXTRA>(extra)...);
    return true;
  }
}

/// @endcond

// Debug; Not Emscripten

/// Require a specified condition to be true.  If it is false, immediately halt execution.
/// Note: If NDEBUG is defined, emp_assert() will not do anything.
#define emp_assert(...)                                                                          \
  do {                                                                                           \
    !(EMP_GET_ARG_1(__VA_ARGS__, ~)) &&                                                          \
    emp::assert_trigger(__FILE__, __LINE__, EMP_WRAP_ARGS(emp_assert_TO_PAIR, __VA_ARGS__) ) &&  \
    (abort(), false);                                                                            \
  } while(0)

// Emscripten-only asserts should be disabled since we are not in Emscripten
/// Require a specified condition to be true if this program was compiled to Javascript with Emscripten.
/// Note: If NDEBUG is defined, emp_emscripten_assert() will not do anything.
#define emp_emscripten_assert(...) emp_assert(__VA_ARGS__)

/// @cond DEFINES

#endif // NDEBUG


#endif // Include guard

/// @endcond
