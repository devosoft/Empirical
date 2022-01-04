/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2020.
 *
 *  @file assert.hpp
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
 *  When compiled in debug mode (i.e. without the -DNDEBUG flag), this will
 *  trigger an assertion error and print the value of a.
 */

#ifndef EMP_BASE_ASSERT_HPP_INCLUDE
#define EMP_BASE_ASSERT_HPP_INCLUDE

#include "always_assert.hpp"

/// NDEBUG hould trigger its EMP equivalent.
#ifdef NDEBUG
#define EMP_NDEBUG
#endif

#if defined( EMP_NDEBUG )
  /// Ideally, this assert should use the expression (to prevent compiler
  /// error), but should not generate any assembly code.
  /// For now, just make it blank (other options commented out).
  #define emp_assert(...)
  // #define emp_assert(EXPR) ((void) sizeof(EXPR) )
  // #define emp_assert(EXPR, ...) { constexpr bool __emp_assert_tmp = false && (EXPR); (void) __emp_assert_tmp; }

#else
  /// Require a specified condition to be true. If it is false, immediately
  /// halt execution. Print also extra information on any variables or
  /// expressions provided as variadic args. Note: If NDEBUG is defined,
  /// emp_assert() will not do anything. Due to macro parsing limitations, extra
  /// information will not be printed when compiling with MSVC.
  #define emp_assert(...) emp_always_assert(__VA_ARGS__)

#endif


#endif // #ifndef EMP_BASE_ASSERT_HPP_INCLUDE
