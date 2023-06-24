/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2021.
*/
/**
 *  @file
 *  @brief Terminating error trigger implementation.
 *  @note For internal use.
 */

#ifndef EMP_BASE__NATIVE_ERROR_TRIGGER_HPP_INCLUDE
#define EMP_BASE__NATIVE_ERROR_TRIGGER_HPP_INCLUDE


#include <string>

namespace emp {

  template <typename... Ts>
  void trigger_emp_error(const std::string& filename, const size_t line, Ts &&... args) {
    std::cerr << "Fatal Error (In " << filename << " line " << line
              <<  "): ";
    (std::cerr << ... << args);
    std::cerr << "\n";
    abort();
  }


} // namespace emp

#endif // #ifndef EMP_BASE__NATIVE_ERROR_TRIGGER_HPP_INCLUDE
