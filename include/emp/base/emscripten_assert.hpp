/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file emscripten_assert.hpp
 *  @brief Assert evaluated only in debug mode with Emscripten.
 */

#ifndef EMP_BASE_EMSCRIPTEN_ASSERT_HPP_INCLUDE
#define EMP_BASE_EMSCRIPTEN_ASSERT_HPP_INCLUDE

#include "assert.hpp"

#ifdef __EMSCRIPTEN__

  /// Require a specified condition to be true if this program was compiled to
  /// Javascript with Emscripten. Note: If NDEBUG is defined,
  /// emp_emscripten_assert() will not do anything.
  #define emp_emscripten_assert(...) emp_assert(__VA_ARGS__)
#else
  #define emp_emscripten_assert(...)
#endif

#endif // #ifndef EMP_BASE_EMSCRIPTEN_ASSERT_HPP_INCLUDE
