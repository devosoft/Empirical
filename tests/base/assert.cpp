/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2017
 *
 *  @file assert.cpp
 */

#undef NDEBUG

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#define TDEBUG 1
#include "emp/base/assert.hpp"

TEST_CASE("Test assert", "[base]")
{

  // Asserts are tricky to test.  Here are a bunch that should PASS.
  emp_assert(true);
  REQUIRE(emp::assert_last_fail == 0);

  emp_assert(100);
  REQUIRE(emp::assert_last_fail == 0);

  emp_assert(23 < 24);
  REQUIRE(emp::assert_last_fail == 0);

  emp_assert((14 < 13)?0:1);
  REQUIRE(emp::assert_last_fail == 0);

  // Best way to test different behavior of emp_assert based on compiler flags?

  const int expected_line_num = __LINE__ + 1; // Line must precede the assert for line num to pass
  emp_assert(false);
  REQUIRE(emp::assert_last_fail);

  // Test filename and line number
  // Trim the filename in case it runs from another directory
  const std::string filename = emp::assert_fail_info.filename;
  REQUIRE_THAT(filename, Catch::Matchers::EndsWith("assert.cpp"));
  REQUIRE(emp::assert_fail_info.line_num == expected_line_num);
  // Error message
  REQUIRE(emp::assert_fail_info.error == "false");

  // Can we clear the assert?
  REQUIRE(emp::assert_last_fail);
  emp::assert_clear();
  REQUIRE_FALSE(emp::assert_last_fail);

  // Run through similar tests to ensure no holdovers from cleared assert.
  emp_assert(true);
  REQUIRE(emp::assert_last_fail == 0);

  emp_assert((2 + 4) / 2);
  REQUIRE(emp::assert_last_fail == 0);


  const int expected_line_num_2 = __LINE__ + 1; // Line must precede the assert for line num to pass
  emp_assert(2 > 3); // False! Trigger the assert
  REQUIRE(emp::assert_last_fail);


  const std::string filename_2 = emp::assert_fail_info.filename;
  REQUIRE_THAT(filename_2, Catch::Matchers::EndsWith("assert.cpp"));
  REQUIRE(emp::assert_fail_info.line_num == expected_line_num_2);
  // Error message
  REQUIRE(emp::assert_fail_info.error == "2 > 3");
}
