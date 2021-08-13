/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file error.hpp
 *  @brief Nearly-universal error, to use in place of emp_assert(false, ...).
 *  Aborts program in both debug and release mode, but does NOT terminate in TDEBUG for testing.
 */


#ifndef EMP_ERROR_HPP
#define EMP_ERROR_HPP

#include <iostream>
#include <string>
#include <sstream>

namespace emp {

/// TDEBUG should trigger its EMP equivalent.
#ifdef TDEBUG
#define EMP_TDEBUG 1
#endif

// If we're testing, do NOT abort, and instead save all data so it can be tested
#ifdef EMP_TDEBUG
  struct ErrorInfo {
    std::string filename;
    size_t line_num;
    std::string output;
  };

  ErrorInfo error_info;
  bool error_thrown = false;

  void error_clear() { emp::error_thrown = false; }

  template <typename... Ts>
  void trigger_emp_error(std::string filename, size_t line, Ts &&... args) {
    std::cout << "Would-be fatal error (In " << filename << " line " << line
              <<  "): ";
    (std::cout << ... << args);
    std::cout << std::endl;
    std::stringstream tmp_stream;
    (tmp_stream << ... << args);
    emp::error_info.filename = filename;
    emp::error_info.line_num = line;
    emp::error_info.output = tmp_stream.str();
    emp::error_thrown = true;
  }

// If we're NOT testing, just print the error and abort
#else 
  template <typename... Ts>
  void trigger_emp_error(std::string filename, size_t line, Ts &&... args) {
    std::cerr << "Fatal Error (In " << filename << " line " << line
              <<  "): ";
    (std::cerr << ... << args);
    std::cerr << std::endl;
    abort();
  }
#endif

} // namespace emp

/// Near-universal error, that aborts in release and debug, but not tbdebug
/// Use in place of emp_assert(false, ...); no need to debug toggle.
#define emp_error(...)                                                         \
  do {                                                                         \
    emp::trigger_emp_error(__FILE__, __LINE__, __VA_ARGS__);                   \
  } while(0)

#endif // #ifndef EMP_ERROR_HPP
