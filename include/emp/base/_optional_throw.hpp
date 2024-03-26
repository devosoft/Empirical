/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2024
*/
/**
 *  @file
 *  @brief Implementation of emp_optional_throw.
 */

#ifndef EMP_BASE__OPTIONAL_THROW_HPP_INCLUDE
#define EMP_BASE__OPTIONAL_THROW_HPP_INCLUDE

#include <sstream>
#include <string>

#include "_is_streamable.hpp"

namespace emp {

  /// Base case for assert_print...
  inline void assert_print_opt(std::stringstream &) { ; }

  /// Print out information about the next variable and recurse...
  template <typename T, typename... EXTRA>
  inline void assert_print_opt(std::stringstream & ss, std::string name, T && val, EXTRA &&... extra) {
    if constexpr ( emp::is_streamable<std::stringstream, T>::value ) {
      ss << name << ": [" << val << "]" << std::endl;
    } else ss << name << ": (non-streamable type)" << std::endl;
    assert_print_opt(ss, std::forward<EXTRA>(extra)...);
  }

  template <typename T, typename... EXTRA>
  inline void assert_print_second_opt(std::stringstream & ss, std::string name, T && val, EXTRA &&... extra) {
    assert_print_opt(ss, std::forward<EXTRA>(extra)...);
  }

  template <typename T>
  inline void assert_print_second_opt(std::stringstream & ss, std::string name, T && val) {;}

  template <typename T, typename... EXTRA>
  inline void assert_print_first_opt(std::stringstream & ss, std::string name, T && val, EXTRA &&... extra) {
    if constexpr ( emp::is_streamable<std::stringstream, T>::value ) {
      ss << name << ": [" << val << "]" << std::endl;
    } else ss << name << ": (non-streamable type)" << std::endl;
    assert_print_second_opt(ss, std::forward<EXTRA>(extra)...);
  }

  inline void assert_print_first_opt(std::stringstream & ss, int placeholder) {;}

  template <typename... EXTRA>
  inline void assert_throw_opt(std::string filename, size_t line, std::string expr, std::string message, EXTRA &&... extra) {
    std::stringstream ss;
    ss << "Internal Error (in " << filename << " line " << line << "): " << expr << ".\n\n Message: " << message << "\n\n";
    assert_print_first_opt(ss, std::forward<EXTRA>(extra)...);
    throw(std::runtime_error(ss.str()));
  }
}

#endif // #ifndef EMP_BASE__OPTIONAL_THROW_HPP_INCLUDE
