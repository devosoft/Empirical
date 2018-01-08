/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2017
 *
 *  @file unit_tests.h
 *  @brief Macros to facilitate unit testing.
 *  @note Status: technically DEPRECATED (now using Catch, but may revert back)
 */


#ifndef EMP_UNIT_TESTS_H
#define EMP_UNIT_TESTS_H

///  Input:  A macro call and a strings indicating the expected result.
///  Output: Code that tests of the macro result matches the expected results, and optionally
///          print it (if in verbose mode or if the macro fails to produce the expected result.)

#define EMP_TEST_MACRO( MACRO, EXP_RESULT )                             \
  do {                                                                  \
    std::string result = std::string(EMP_STRINGIFY( MACRO ));           \
    bool match = (result == EXP_RESULT);                                \
    if (verbose || !match) {                                            \
      std::cout << #MACRO << " == " << result << std::endl;             \
    }                                                                   \
    if (!match) {                                                       \
      std::cout << "MATCH FAILED!  Expected: "                          \
                << EXP_RESULT << std::endl;                             \
      abort();                                                          \
    }                                                                   \
  } while (false)


///  Take in an expression and a string representing the expected result; verify that the
///  stringifyied result is the expected string AND print this info if -v flag is set.
///     Input:  An expression and an expected evaluation.
///     Output: Code to evaluate the expression and optionally print it (if either in verbose mode
///             or the macro does not produce the expected result).

#define EMP_TEST_VALUE( VALUE, EXP_RESULT )                             \
  do {                                                                  \
    std::stringstream ss;                                               \
    auto result = VALUE;                                                \
    ss << result;                                                       \
    bool match = (ss.str() == EXP_RESULT);                              \
    if (verbose || !match) {                                            \
      std::cout << #VALUE << " == " << result << std::endl;             \
    }                                                                   \
    if (!match) {                                                       \
      std::cout << "MATCH FAILED!  Expected: "                          \
                << EXP_RESULT << std::endl;                             \
      abort();                                                          \
    }                                                                   \
  } while (false)


#endif
