/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2018
 *
 *  @file unit_tests.h
 *  @brief Macros to facilitate unit testing.
 *  @note Status: technically DEPRECATED (now using Catch, but may revert back)
 */


#ifndef EMP_UNIT_TESTS_H
#define EMP_UNIT_TESTS_H

#include <iostream>
#include <sstream>

namespace emp {
  static bool & UnitTestVerbose() {
    static bool verbose = false;
    return verbose;
  }

  static bool & UnitTestVerbose(bool in_verbose) {
    return (UnitTestVerbose() = in_verbose);
  }

  static size_t & UnitTestErrors() {
    static size_t errors = 0;
    return errors;
  }
}

///  Input:  A macro call and a strings indicating the expected result.
///  Output: Code that tests of the macro result matches the expected results, and optionally
///          print it (if in verbose mode or if the macro fails to produce the expected result.)

#define EMP_TEST_MACRO( MACRO, EXP_RESULT )                             \
  do {                                                                  \
    std::string result = std::string(EMP_STRINGIFY( MACRO ));           \
    bool match = (result == EXP_RESULT);                                \
    if (emp::UnitTestVerbose() || !match) {                             \
      std::cout << __FILE__ << ", line " << __LINE__ << ": "            \
                << #MACRO << " == " << result << std::endl;             \
    }                                                                   \
    if (!match) {                                                       \
      std::cout << "\033[1;31mMATCH FAILED!  Expected: "                \
                << EXP_RESULT << "\033[0m" << std::endl;               \
      emp::UnitTestErrors()++;                                          \
    } else if (emp::UnitTestVerbose()) {                                \
      std::cout << "\033[1;32mPASSED!"                                  \
                << "\033[0m" << std::endl;                              \
    }                                                                   \
  } while (false)


///  Take in an expression and a string representing the expected result; verify that the
///  stringifyied result is the expected string AND print this info if -v flag is set.
///     Input:  An expression and an expected evaluation.
///     Output: Code to evaluate the expression and optionally print it (if either in verbose mode
///             or the macro does not produce the expected result).

#define EMP_TEST_VALUE( VALUE, EXP_RESULT )                             \
  do {                                                                  \
    auto result = VALUE;                                                \
    bool match = (result == (EXP_RESULT));                              \
    if (emp::UnitTestVerbose() || !match) {                             \
      std::cout << __FILE__ << ", line " << __LINE__ << ": "            \
                << #VALUE << " == " << result << std::endl;             \
    }                                                                   \
    if (!match) {                                                       \
      std::cout << "\033[1;31mMATCH FAILED!  Expected: "                \
                << #EXP_RESULT << "\033[0m" << std::endl;               \
      emp::UnitTestErrors()++;                                          \
    } else if (emp::UnitTestVerbose()) {                                \
      std::cout << "\033[1;32mPASSED!"                                  \
                << "\033[0m" << std::endl;                              \
    }                                                                   \
  } while (false)


#define emp_test_main                                                     \
  emp_main_function();                                                    \
  int main() {                                                            \
    emp_main_function();                                                  \
    int num_errors = emp::UnitTestErrors();                               \
    if (emp::UnitTestErrors()) {                                          \
      std::cout << "\033[1;31mRESULT: " << num_errors << " tests failed!" \
                << "\033[0m" << std::endl;                                \
    } else {                                                              \
      std::cout << "\033[1;32mRESULT: all tests PASSED!"                  \
                << "\033[0m" << std::endl;                                \
    }                                                                     \
    return num_errors;                                                    \
  }                                                                       \
  int emp_main_function

#endif
