/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file always_assert_warning.hpp
 *  @brief A more dynamic replacement for standard library asserts.
 *  @note Status: RELEASE
 *
 *  A replacement for the system-level assert.h, called "emp_always_assert"
 *  Added functionality:
 *   - If compiled with Emscripten, will provide pop-up alerts in a web browser.
 *   - emp_assert can take additional arguments.  If the assert is triggered,
 *     those extra arguments will be evaluated and printed.
 *   - if EMP_TDEBUG is defined, emp_assert() goes into test mode and records
 *     failures, but does not abort.  (useful for unit tests of asserts)
 *
 *  Example:
 *
 *     int a = 6;
 *     emp_always_assert(a==5, a);
 *
 *  Unlinke "emp_assert_warning", "emp_always_assert_warning" will trigger an
 *  assertion error whether compiled in debug mode or not.
 */

#ifndef EMP_BASE_ALWAYS_ASSERT_WARNING_HPP_INCLUDE
#define EMP_BASE_ALWAYS_ASSERT_WARNING_HPP_INCLUDE

#include <cstdlib>

#include "_assert_macros.hpp"
#include "_assert_trigger.hpp"

#if defined( _MSC_VER )

  #define emp_always_assert_warning_msvc_impl(TEST)                            \
    do {                                                                       \
      !(TEST)                                                                  \
      && emp::assert_trigger(__FILE__, __LINE__, #TEST, 0);                    \
    } while(0)

  #define emp_always_assert_warning_impl(TEST) \
    emp_always_assert_warning_msvc_impl(TEST)

#else

  #define emp_always_assert_warning_impl(...)                                  \
    do {                                                                       \
      !(emp_assert_GET_ARG_1(__VA_ARGS__, ~))                                  \
      && emp::assert_trigger(                                                  \
        __FILE__, __LINE__,                                                    \
        emp_assert_STRINGIFY( emp_assert_GET_ARG_1(__VA_ARGS__, ~) ),          \
        emp_assert_TO_PAIRS(__VA_ARGS__)                                       \
      );                                                                       \
    } while(0)

#endif

/// Require a specified condition to be true. If it is false, immediately
/// halt execution. Print also extra information on any variables or
/// expressions provided as variadic args. Will be evaluated when compiled in
/// both debug and release mode. Can be disabled with -DEMP_NO_WARNINGS.
#ifndef EMP_NO_WARNINGS
  #define emp_always_assert_warning(...) \
    emp_always_assert_warning_impl(__VA_ARGS__)
#else
  #define emp_always_assert_warning(...)
#endif

#endif // #ifndef EMP_BASE_ALWAYS_ASSERT_WARNING_HPP_INCLUDE
