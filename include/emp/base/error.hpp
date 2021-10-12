/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file _tdebug_assert_trigger.hpp
 *  @brief Universal error, to use in place of emp_assert(false, ...).
 *  Evaluated in both debug and release mode.
 */


#ifndef EMP_ERROR_HPP
#define EMP_ERROR_HPP

#include <iostream>
#include <string>

namespace emp {

  template <typename... Ts>
  void trigger_emp_error(std::string filename, size_t line, Ts &&... args) {
    std::cerr << "Fatal Error (In " << filename << " line " << line
              <<  "): ";
    (std::cerr << ... << args);
    std::cerr << std::endl;
    abort();
  }

} // namespace emp

/// Universal error.
/// Use in place of emp_assert(false, ...); no need to debug toggle.
#define emp_error(...)                                                         \
  do {                                                                         \
    emp::trigger_emp_error(__FILE__, __LINE__, __VA_ARGS__);                   \
  } while(0)

#endif // #ifndef EMP_ERROR_HPP
