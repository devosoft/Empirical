/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2020 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/base/emscripten_assert.hpp
 * @brief Assert evaluated only in debug mode with Emscripten.
 */

#pragma once

#ifndef INCLUDE_EMP_BASE_EMSCRIPTEN_ASSERT_HPP_GUARD
#define INCLUDE_EMP_BASE_EMSCRIPTEN_ASSERT_HPP_GUARD

#include "assert.hpp"

#ifdef __EMSCRIPTEN__

  /// Require a specified condition to be true if this program was compiled to
  /// Javascript with Emscripten. Note: If NDEBUG is defined,
  /// emp_emscripten_assert() will not do anything.
  #define emp_emscripten_assert(...) emp_assert(__VA_ARGS__)
#else
  /// Require a specified condition to be true if this program was compiled to
  /// Javascript with Emscripten. Note: If NDEBUG is defined,
  /// emp_emscripten_assert() will not do anything.
  #define emp_emscripten_assert(...)
#endif

#endif // #ifndef EMP_BASE_EMSCRIPTEN_ASSERT_HPP_INCLUDE
