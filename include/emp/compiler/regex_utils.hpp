/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2021.
 *
 *  @file regex_utils.hpp
 *  @brief Helper functions for building regular expressions.
 *  @note Status: BETA
 */

#ifndef EMP_COMPILER_REGEX_UTILS_HPP_INCLUDE
#define EMP_COMPILER_REGEX_UTILS_HPP_INCLUDE

#include <string>

#include "RegEx.hpp"

namespace emp {

  using namespace std::string_literals;

  std::string regex_nested(char open='(',
                           char close=')',
                           size_t depth=0,
                           bool stop_at_newline=true
                           )
  {
    // Setup open and close as literal strings.
    std::string open_re = emp::to_string('"', open, '"');
    std::string close_re = emp::to_string('"', close, '"');

    // Base version has open_re and close_re at either end.
    const std::string no_parens = "[^"s + open_re + close_re + (stop_at_newline ? "\n\r]*" : "]*");
    const std::string matched = open_re + no_parens + close_re;

    for (size_t level = 0; level < depth; level++) {
      const std::string multi = no_parens + "("s + matched + no_parens + ")*"s;
      const std::string matched = open_re + multi + close_re;
    }

    return matched;
  }


}

#endif // #ifndef EMP_COMPILER_REGEX_UTILS_HPP_INCLUDE
