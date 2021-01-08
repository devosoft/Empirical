/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file assert_warning.hpp
 *  @brief A non-terminating replacement for standard library asserts.
 *
 *  A supplement for the system-level assert.h, called "emp_assert_warning"
 *  Added functionality:
 *   - If compiled with Emscripten, will provide pop-up alerts in a web browser.
 *   - emp_assert_warning can take additional arguments.  If the assert is
 *     triggered, those extra arguments will be evaluated and printed.
 *   - if NDEBUG -or- EMP_NDEBUG is defined, the expression in
 *     emp_assert_warning() is not evaluated.
 *   - emp_assert_warning() records failures, but does not abort.
 *
 *  Example:
 *
 *     int a = 6;
 *     emp_assert_warning(a==5, a);
 *
 *  When compiled in debug mode (i.e., without the -DNDEBUG flag), this will
 *  pring an assertion error and print the value of a.
 */

#ifndef EMP_ASSERT_WARNING_HPP
#define EMP_ASSERT_WARNING_HPP

#include "always_assert.hpp"

/// NDEBUG hould trigger its EMP equivalent.
#ifdef NDEBUG
#define EMP_NDEBUG
#endif

#if defined( EMP_NDEBUG )
  /// Ideally, this assert should use the expression (to prevent compiler
  /// error), but should not generate any assembly code.
  /// For now, just make it blank (other options commented out).
  #define emp_assert_warning(...)
  // #define emp_assert_warning(EXPR) ((void) sizeof(EXPR) )
  // #define emp_assert_warning(EXPR, ...) { constexpr bool __emp_assert_warning_tmp = false && (EXPR); (void) __emp_assert_warning_tmp; }

#else
  /// Require a specified condition to be true. If it is false, print extra
  /// information on any variables or expressions provided as variadic args.
  /// Note: If NDEBUG is defined, emp_assert_warning() will not do anything.
  /// Due to macro parsing limitations, extra information will not be printed when compiling with MSVC.
  #define emp_assert_warning(...) emp_always_assert(__VA_ARGS__)

#endif


#endif // #ifndef EMP_ASSERT_WARNING_HPP
