/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2021 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/base/error.hpp
 * @brief Nearly-universal error, to use in place of emp_assert(false, ...).
 * Aborts program in both debug and release mode, but does NOT terminate in TDEBUG for testing.
 */

#pragma once

#ifndef INCLUDE_EMP_BASE_ERROR_HPP_GUARD
#define INCLUDE_EMP_BASE_ERROR_HPP_GUARD


#include "_error_trigger.hpp"

#define emp_error(...)                                                         \
  do {                                                                         \
    emp::trigger_emp_error(__FILE__, __LINE__, __VA_ARGS__);                   \
  } while(0)

#endif  // #ifndef INCLUDE_EMP_BASE_ERROR_HPP_GUARD
