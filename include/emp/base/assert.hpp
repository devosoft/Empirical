/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2016-2020 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/base/assert.hpp
 * @brief A more dynamic replacement for standard library asserts.
 * Status: RELEASE
 *
 * A replacement for the system-level assert.h, called "emp_assert"
 * Added functionality:
 *  - If compiled with Emscripten, will provide pop-up alerts in a web browser.
 *  - emp_assert can take additional arguments.  If the assert is triggered, those extra
 *    arguments will be evaluated and printed.
 *  - if NDEBUG is defined, the expression in emp_assert() is not evaluated.
 *  - if TDEBUG is defined, emp_assert() goes into test mode and records failures, but
 *    does not abort.  (useful for unit tests of asserts)
 *
 * Example:
 *
 *    int a = 6;
 *    emp_assert(a==5, a);
 *
 * When compiled in debug mode (i.e. without the -DNDEBUG flag), this will
 * trigger an assertion error and print the value of a.
 */

#pragma once

#ifndef INCLUDE_EMP_BASE_ASSERT_HPP_GUARD
#define INCLUDE_EMP_BASE_ASSERT_HPP_GUARD

#include "always_assert.hpp"

#ifdef NDEBUG
/// Ideally, this assert should use the expression (to prevent compiler
/// error), but should not generate any assembly code.
/// For now, just make it blank (other options commented out).
#define emp_assert(...)

namespace emp { static constexpr bool is_debug_mode = false; }

#else  // #ifdef NDEBUG
/// Require a specified condition to be true. If it is false, immediately
/// halt execution. Print also extra information on any variables or
/// expressions provided as variadic args. Note: If NDEBUG is defined,
/// emp_assert() will not do anything. Due to macro parsing limitations, extra
/// information will not be printed when compiling with MSVC.
#define emp_assert(...) emp_always_assert(__VA_ARGS__)

namespace emp { static constexpr bool is_debug_mode = true; }

#endif  // #ifdef NDEBUG : #else


#endif  // #ifndef INCLUDE_EMP_BASE_ASSERT_HPP_GUARD
