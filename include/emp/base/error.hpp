/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file error.hpp
 *  @brief Nearly-universal error, to use in place of emp_assert(false, ...).
 *  Aborts program in both debug and release mode, but does NOT terminate in TDEBUG for testing.
 */

#ifndef EMP_BASE_ERROR_HPP_INCLUDE
#define EMP_BASE_ERROR_HPP_INCLUDE


#include "_error_trigger.hpp"

#define emp_error(...)                                                         \
  do {                                                                         \
    emp::trigger_emp_error(__FILE__, __LINE__, __VA_ARGS__);                   \
  } while(0)

#endif // #ifndef EMP_BASE_ERROR_HPP_INCLUDE
