/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2021 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/base/_error_trigger.hpp
 * @brief Error trigger implementation selector
 * @note For internal use.
 */

#pragma once

#ifndef INCLUDE_EMP_BASE_ERROR_TRIGGER_HPP_GUARD
#define INCLUDE_EMP_BASE_ERROR_TRIGGER_HPP_GUARD

/// TDEBUG should trigger its EMP equivalent.
#ifdef TDEBUG
#define EMP_TDEBUG 1
#endif

#if defined(__EMSCRIPTEN__)
#include "_emscripten_error_trigger.hpp"
#elif defined(EMP_TDEBUG)
#include "_tdebug_error_trigger.hpp"
#else
#include "_native_error_trigger.hpp"
#endif

#endif  // #ifndef EMP_BASE_ERROR_TRIGGER_HPP_INCLUDE
