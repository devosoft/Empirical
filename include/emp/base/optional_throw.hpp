/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2023.
 *
 *  @file python_throw.hpp
 *  @brief Like emp_assert, but throws an exception if in Python
 *  @note Status: RELEASE
 *
 */

#ifndef EMP_BASE_PYTHON_THROW_HPP_INCLUDE
#define EMP_BASE_PYTHON_THROW_HPP_INCLUDE

#include "assert.hpp"

/// NDEBUG should trigger its EMP equivalent.
#ifdef NDEBUG
#define EMP_NDEBUG
#endif

#if defined( EMP_NDEBUG )

  #define emp_optional_throw(...)

  namespace emp {
    static constexpr bool is_debug_mode = false;
  }

#elif defined(IN_PYTHON)

  #define emp_optional_throw(...)                                     \
    do {                                                                  \
      if (!(emp_assert_GET_ARG_1(__VA_ARGS__, ~))) {                      \
        emp::assert_throw(                                             \
        __FILE__, __LINE__,                                               \
        emp_assert_STRINGIFY( emp_assert_GET_ARG_1(__VA_ARGS__, ~),  ),   \
        emp_assert_TO_PAIRS(__VA_ARGS__));                                       \
      }                                                                   \
    } while(0)

#else
  /// Require a specified condition to be true. If it is false, immediately
  /// halt execution. Print also extra information on any variables or
  /// expressions provided as variadic args. Note: If NDEBUG is defined,
  /// emp_assert() will not do anything. Due to macro parsing limitations, extra
  /// information will not be printed when compiling with MSVC.
  #define emp_optional_throw(...) emp_assert(__VA_ARGS__)

  namespace emp {
    static constexpr bool is_debug_mode = true;
  }

#endif


#endif // #ifndef EMP_BASE_ASSERT_HPP_INCLUDE
