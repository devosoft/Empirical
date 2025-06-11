/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2021 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/base/_emscripten_error_trigger.hpp
 * @brief Terminating error trigger implementation.
 * @note For internal use.
 * @todo Reflect error message to browser (e.g., as an alert)
 */

#pragma once

#ifndef INCLUDE_EMP_BASE_EMSCRIPTEN_ERROR_TRIGGER_HPP_impl_GUARD
#define INCLUDE_EMP_BASE_EMSCRIPTEN_ERROR_TRIGGER_HPP_impl_GUARD

#include <iostream>
#include <stddef.h>
#include <string>

namespace emp {
  template <typename... Ts>
  void trigger_emp_error(const std::string & filename, const size_t line, Ts &&... args) {
    std::cerr << "Fatal Error (In " << filename << " line " << line << "): ";
    (std::cerr << ... << args);
    std::cerr << "\n";
    abort();
  }

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_BASE_EMSCRIPTEN_ERROR_TRIGGER_HPP_impl_GUARD
