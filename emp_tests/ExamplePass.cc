/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file ExamplePass.cc
 *  @brief An example file to show what PASSING unit tests look like, with commentary.
 */


// The only required include for unit tests is the one below.
// It includes everything else it needs.

#include "../source/tools/unit_tests.h"

// You may also include any other files that you need for your specific tests.

#include <string>
#include "../source/tools/string_utils.h"

// Define the main function like the one below.  You do NOT create main(); that will be done
// for you and cammand-line arguments will be handled automatically.

int emp_test_main()
{
  int x = 5;

  emp::UnitTestVerbose(true);
  EMP_TEST_VALUE(x, "5");

  x *= 5;

  EMP_TEST_VALUE(x, "25");

  // Tests can be run with expressions.
  EMP_TEST_VALUE(4*x, "100");

  
}
