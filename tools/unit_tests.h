// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is 
// Copyright (C) Michigan State University, 2015. It is licensed 
// under the MIT Software license; see doc/LICENSE

#ifndef EMP_UNIT_TESTS_H
#define EMP_UNIT_TESTS_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Macros to facilitate unit testing.
//
//  EMP_TEST_MACRO(MACRO, EXP_RESULT)
//    Input:  A macro call and an expected result.
//    Output: Code to test the macro and optionally print it (if either in verbose mode
//            or the macro does not produce the expected result.)
//
//  EMP_TEST_VALUE(VALUE, EXP_RESULT) make sure function prints expect result; print if verbose.
//    Input:  An expression and an expected evaluation.
//    Output: Code to evaluate the expression and optionally print it (if either in verbose mode
//            or the macro does not produce the expected result).
//

// Take in a macro call and a string representing the expected result; verify that the
// stringifyied macro result is the expected string AND print this info if -v flag is set. 
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

// Take in an expression and a string representing the expected result; verify that the
// stringifyied result is the expected string AND print this info if -v flag is set. 
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
