/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2023-2024 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/base/compile_consts.hpp
 * @brief Constants associated with compile options to simplify other implementations.
 *
 */

#pragma once

#ifndef INCLUDE_EMP_BASE_COMPILE_CONSTS_HPP_GUARD
#define INCLUDE_EMP_BASE_COMPILE_CONSTS_HPP_GUARD

#include <cstdint>  // uint8_t, uint16_t, etc.

namespace emp::compile {

  #ifdef NDEBUG
    static constexpr bool DEBUG = false;
  #else
    static constexpr bool DEBUG = true;
  #endif

  #ifdef TDEBUG
    static constexpr bool TEST = true;
  #else
    static constexpr bool TEST = false;
  #endif

  #ifdef __EMSCRIPTEN__
    static constexpr bool NATIVE_TARGET = false;
    static constexpr bool WEB_TARGET = true;
  #else
    static constexpr bool NATIVE_TARGET = true;
    static constexpr bool WEB_TARGET = false;
  #endif

  #ifdef __EMSCRIPTEN_PTHREADS__
    static constexpr bool EMSCRIPTEN_PTHREADS = true;
  #else
    static constexpr bool EMSCRIPTEN_PTHREADS = false;
  #endif

}  // namespace emp::compile

#endif  // #ifndef INCLUDE_EMP_BASE_COMPILE_CONSTS_HPP_GUARD
