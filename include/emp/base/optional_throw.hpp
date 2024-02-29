/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2024
*/
/**
 *  @file
 *  @brief TODO.
 *
 */

#ifndef EMP_BASE_OPTIONAL_THROW_HPP_INCLUDE
#define EMP_BASE_OPTIONAL_THROW_HPP_INCLUDE

#include "assert.hpp"

/// NDEBUG should trigger its EMP equivalent.
#ifdef NDEBUG
#define EMP_NDEBUG
#endif


#if defined( EMP_OPTIONAL_THROW_ON )

  // #if defined (_MSC_VER )

  #define emp_optional_throw(TEST, MESSAGE)                                     \
    do {                                                                  \
      if (!(TEST)) {                                                      \
        emp::assert_throw_opt(__FILE__, __LINE__, #TEST, MESSAGE, 0);                \
      }                                                                   \
    } while(0)

  // #define emp_optional_throw_impl(TEST, MESSAGE) emp_optional_throw_mscv_impl(TEST, MESSAGE)

  // #else

  // #define emp_optional_throw_impl(...)                                     \
  //   do {                                                                  \
  //     if (!(emp_assert_GET_ARG_1(__VA_ARGS__, ~))) {                      \
  //       emp::assert_throw(                                             \
  //       __FILE__, __LINE__,                                               \
  //       emp_assert_STRINGIFY( emp_assert_GET_ARG_1(__VA_ARGS__, ~),  ),   \
  //       emp_assert_STRINGIFY( emp_assert_GET_ARG_2(__VA_ARGS__, ~),  ),   \
  //       emp_assert_TO_PAIRS(__VA_ARGS__));                                       \
  //     }                                                                   \
  //   } while(0)

  //   #endif

    // #define emp_optional_throw(...) emp_optional_throw_impl(__VA_ARGS__)

#elif defined( EMP_NDEBUG )

  #define emp_optional_throw(...)

#else
  /// Require a specified condition to be true. If it is false, immediately
  /// halt execution. Print also extra information on any variables or
  /// expressions provided as variadic args. Note: If NDEBUG is defined,
  /// emp_assert() will not do anything. Due to macro parsing limitations, extra
  /// information will not be printed when compiling with MSVC.
  #define emp_optional_throw(...) emp_assert(__VA_ARGS__)

#endif


#endif // #ifndef EMP_BASE_OPTIONAL_THROW_HPP_INCLUDE
