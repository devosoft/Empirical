/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file ExamplePass.cc
 *  @brief An example file to show what PASSING unit tests look like, with commentary.
 *
 *  Unit tests can be built easily, with a good bit of flexibility, as described below.
 *
 *  All unit tests can take command line arguments:
 *
 *    --help
 *    --silent
 *    --verbose
 *
 *  Try running the executable with --verbose to see the full information provided.
 */


// The only required include for unit tests is the one below.
// It includes everything else it needs.

#include "emp/testing/unit_tests.hpp"

// You may also include any other files that you need for your specific tests.

#include <string>
#include "emp/tools/string_utils.hpp"

// You may create an extra functins or macros that you may need.

#define TRIPLE_INPUT(VAL) VAL+VAL+VAL

// Define the main function like the one below.  You do NOT create main(); that will be done
// for you and cammand-line arguments will be handled automatically.

void emp_test_main()
{
  std::cout << "Run with --verbose flag in order to get more detailed output." << std::endl;

  int x = 5;

  // Simple tests might examine the value of a variable.
  EMP_TEST_VALUE(x, 5);

  x *= 5;

  // Variables always use their most up-to-date values, and can be test again.
  EMP_TEST_VALUE(x, 25);

  // Tests can be run with expressions.
  EMP_TEST_VALUE(4*x, 100);

  // You can also test macros.  Provide the expected macro translation as a string.
  EMP_TEST_MACRO(TRIPLE_INPUT(7), "7+7+7");
}
