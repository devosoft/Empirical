/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file ExampleFail2.cpp
 *  @brief An second example file to show what a single FAILING unit test looks like.
 */

// The only required include for unit tests is the one below.
// It includes everything else it needs.

#include "emp/testing/unit_tests.hpp"

// You may create an extra functions or macros that you may need.

bool IsPrime(int val) {
  if (val < 2) return false;
  for (int i = 2; i*i <= val; i++) {
    if ((val/i)*i == val) return false;
  }
  return true;
}

bool IsOdd(int val) { return val % 2; }

// Define the main function like the one below.  You do NOT create main(); that will be done
// for you and cammand-line arguments will be handled automatically.

void emp_test_main()
{
  // A simple example in a loop.
  for (int x = 2; x < 15; x++) {
    // Assert that all odd numbers are prime!
    EMP_TEST_VALUE(IsPrime(x), IsOdd(x));
  }
}
