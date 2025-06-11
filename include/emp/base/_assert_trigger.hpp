/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2020 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/base/_assert_trigger.hpp
 * @brief Assert trigger implementation selector.
 * @note For internal use.
 */

#pragma once

#ifndef INCLUDE_EMP_BASE_ASSERT_TRIGGER_HPP_impl_GUARD
#define INCLUDE_EMP_BASE_ASSERT_TRIGGER_HPP_impl_GUARD

/// TDEBUG should trigger its EMP equivalent.
#ifdef TDEBUG
#define EMP_TDEBUG 1
#endif  // #ifdef TDEBUG

#if defined(__EMSCRIPTEN__)
#include "_emscripten_assert_trigger.hpp"
#elif defined(EMP_TDEBUG)  // #if defined(__EMSCRIPTEN__)
#include "_tdebug_assert_trigger.hpp"
#else  // #if defined(__EMSCRIPTEN__) : #elif defined(EMP_TDEBUG)
#include "_native_assert_trigger.hpp"
#endif  // #if defined(__EMSCRIPTEN__) : #elif defined(EMP_TDEBUG) : #else

#endif  // #ifndef INCLUDE_EMP_BASE_ASSERT_TRIGGER_HPP_impl_GUARD
