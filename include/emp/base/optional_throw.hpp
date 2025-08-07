/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2024 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/base/optional_throw.hpp
 * @brief Like emp_assert, but throws a runtime error if compiled with -DEMP_OPTIONAL_THROW_ON.
 *
 * Useful if you want the option to throw a runtime error outside of debug mode. A common use
 * case is wrapping C++ code in Python, since segfaults kill the entire Python interpreter.
 */

#pragma once

#ifndef INCLUDE_EMP_BASE_OPTIONAL_THROW_HPP_GUARD
#define INCLUDE_EMP_BASE_OPTIONAL_THROW_HPP_GUARD

#include "_optional_throw.hpp"
#include "assert.hpp"

/// NDEBUG should trigger its EMP equivalent.
#ifdef NDEBUG
#define EMP_NDEBUG
#endif  // #ifdef NDEBUG


#if defined(EMP_OPTIONAL_THROW_ON)

#define emp_optional_throw(TEST, MESSAGE)                                 \
    do {                                                                  \
      if (!(TEST)) {                                                      \
        emp::assert_throw_opt(__FILE__, __LINE__, #TEST, MESSAGE, 0);     \
      }                                                                   \
    } while(0)

#elif defined(EMP_NDEBUG)  // #if defined(EMP_OPTIONAL_THROW_ON)

#define emp_optional_throw(...)

#else  // #if defined(EMP_OPTIONAL_THROW_ON) : #elif defined(EMP_NDEBUG)
/// Require a specified condition to be true. If it is false, immediately
/// halt execution. Print also extra information on any variables or
/// expressions provided as variadic args. Note: If NDEBUG is defined,
/// emp_assert() will not do anything. Due to macro parsing limitations, extra
/// information will not be printed when compiling with MSVC.
#define emp_optional_throw(...) emp_assert(__VA_ARGS__)

#endif  // #if defined(EMP_OPTIONAL_THROW_ON) : #elif defined(EMP_NDEBUG) : #else


#endif  // #ifndef INCLUDE_EMP_BASE_OPTIONAL_THROW_HPP_GUARD

// Local settings for Empecable file checker.
// empecable_words: mscv
