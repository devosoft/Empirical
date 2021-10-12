/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2019
 *
 *  @file unit_tests.hpp
 *  @brief Macros to facilitate unit testing.
 *  @note Status: RESURGENT (Was depricated; now back to ALPHA)
 *
 *  @todo Setup proper command line color handline (probably in its own file.)
 *  @todo Add EMP_TEST_BELOW, EMP_TEST_ABOVE, and EMP_TEST_RANGE to ensurge that a
 *        value is in a certain range.
 *  @todo Consider adding EMP_TEST_ASSERT to ensure that the contained value is true.
 *  @todo Consider adding EMP_TEST_APPROX to compare within a tollerance level.
 */

#ifndef EMP_UNIT_TESTS_H
#define EMP_UNIT_TESTS_H

#include <iostream>
#include <sstream>

#include "../config/command_line.hpp"
#include "../tools/string_utils.hpp"

namespace emp {

  // Unit tests verbosity levels:
  struct UnitTestOutput {
    enum Mode {
      SILENT = 0,    // Just return error code.
      NORMAL = 1,    // Print errors and summary.
      VERBOSE = 2    // Print results for each test performed.
    };

    UnitTestOutput::Mode verbose = UnitTestOutput::Mode::NORMAL;
    size_t num_tests = 0;
    size_t errors = 0;
    bool abort = false;
  };

  static UnitTestOutput & GetUnitTestOutput() {
    static UnitTestOutput output;
    return output;
  }


  void ResolveUnitTest(bool pass, const std::string & test_input,
                   const std::string & result, const std::string & exp_result,
                   const std::string & filename, size_t line_num, bool is_require=false) {
    const UnitTestOutput::Mode verbose = GetUnitTestOutput().verbose;
    GetUnitTestOutput().num_tests++;

    // If we are verbose OR the test failed, print information about it.
    if (verbose == UnitTestOutput::Mode::VERBOSE || !pass) {
      std::cout << filename << ", line " << line_num << ": "
                << test_input << " == " << result << std::endl;
    }
    if (!pass) {
      if (is_require) {
        std::cout << "-> \033[1;31mREQUIREMENT FAILED!\033[0m" << std::endl;
      } else {
        std::cout << "-> \033[1;31mMATCH FAILED!  Expected: ["
                  << exp_result << "]\033[0m" << std::endl;
        std::cout << "                    Output: ["
                  << result << "]" << std::endl;
      }
      GetUnitTestOutput().errors++;
      if (GetUnitTestOutput().abort) {
        std::cout << "Aborting!\n";
        abort();
      }
    } else if (verbose == UnitTestOutput::Mode::VERBOSE) {
      std::cout << "-> \033[1;32mPASSED!" << "\033[0m" << std::endl;
    }
  }
}

///  Input:  A macro call and a strings indicating the expected result.
///  Output: Code that tests of the macro result matches the expected results, and optionally
///          print it (if in verbose mode or if the macro fails to produce the expected result.)

#define EMP_TEST_MACRO( MACRO, EXP_RESULT )                                       \
  do {                                                                            \
    std::string result = std::string(EMP_STRINGIFY( MACRO ));                     \
    bool match = (result == EXP_RESULT);                                          \
    emp::ResolveUnitTest(match, #MACRO, result, #EXP_RESULT, __FILE__, __LINE__); \
  } while (false)


///  Take in an expression and a string representing the expected result; verify that the
///  stringifyied result is the expected string AND print this info if -v flag is set.
///     Input:  An expression and an expected evaluation.
///     Output: Code to evaluate the expression and optionally print it (if either in verbose mode
///             or the macro does not produce the expected result).

#define EMP_TEST_VALUE( VALUE, EXP_RESULT )                                              \
  do {                                                                                   \
    auto result = VALUE;                                                                 \
    auto exp_result = EXP_RESULT;                                                        \
    bool match = (result == (EXP_RESULT));                                               \
    std::string result_str = emp::to_literal(result);                                    \
    std::string exp_result_str = emp::to_literal(exp_result);                            \
    emp::ResolveUnitTest(match, #VALUE, result_str, exp_result_str, __FILE__, __LINE__); \
  } while (false)

#define EMP_REQUIRE( VALUE )                                                             \
  do {                                                                                   \
    auto result = VALUE;                                                                 \
    std::string result_str = emp::to_string(result);                                     \
    emp::ResolveUnitTest(result, #VALUE, result_str, "true", __FILE__, __LINE__, true);  \
  } while (false)

// Setup a default approx range.
#define EMP_TEST_APPROX(...) EMP_TEST_APPROX_impl(__VA_ARGS__, 1.0, ~)

#define EMP_TEST_APPROX_impl( VALUE, EXP_RESULT, THRESHOLD, ...)                          \
  do {                                                                                    \
    auto result = VALUE;                                                                  \
    auto exp_result = EXP_RESULT;                                                         \
    auto upper_bound = exp_result + THRESHOLD;                                            \
    auto lower_bound = exp_result - THRESHOLD;                                            \
    bool match = (result <= upper_bound && result >= lower_bound);                        \
    std::string result_str = emp::to_string(result);                                      \
    std::string exp_result_str = emp::to_string('[', lower_bound, ',', upper_bound, ']'); \
    emp::ResolveUnitTest(match, #VALUE, result_str, exp_result_str, __FILE__, __LINE__);  \
  } while (false)


  void SetupUnitTestArgs(emp::vector<std::string> args) {
        emp::UnitTestOutput::Mode & verbose = emp::GetUnitTestOutput().verbose;
    if (emp::cl::use_arg(args, "--help")) {
      std::cout << "Usage: \033[1;36m" << args[0] << " [args]\033[0m\n"
        << "  \033[1m--abort\033[0m   : Stop execution immediately if a test fails.\n"
        << "  \033[1m--help\033[0m    : This message.\n"
        << "  \033[1m--silent\033[0m  : Produce no output except result code.\n"
        << "  \033[1m--verbose\033[0m : Produce detailed output for each test.\n";
      exit(0);
    }
    if (emp::cl::use_arg(args, "--abort")) { emp::GetUnitTestOutput().abort = true; }
    if (emp::cl::use_arg(args, "--verbose")) verbose = emp::UnitTestOutput::Mode::VERBOSE;
    if (emp::cl::use_arg(args, "--silent")) {
      std::cout.setstate(std::ios_base::failbit); // Disable cout
      verbose = emp::UnitTestOutput::SILENT;
    }
  }

  int ProcessUnitTestResults() {
    int num_errors = (int) emp::GetUnitTestOutput().errors;
    int num_tests = (int) emp::GetUnitTestOutput().num_tests;
    if (num_errors) {
      std::cout << "\033[1;31mRESULT: " << num_errors << "/" << num_tests
                << " tests failed!"
                << "\033[0m" << std::endl;
    } else {
      std::cout << "\033[1;32mRESULT: " << num_tests << "/" << num_tests << " tests PASSED!"
                << "\033[0m" << std::endl;
    }
    return num_errors;
  }

#define emp_test_main() emp_test_main_impl();                    \
  int main(int argc, char * argv[]) {                            \
    SetupUnitTestArgs( emp::cl::args_to_strings(argc, argv) );   \
    emp_test_main_impl();                                        \
    return ProcessUnitTestResults();                             \
  }                                                              \
  void emp_test_main_impl()

#endif
