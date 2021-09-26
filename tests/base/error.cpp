/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2021
 *
 *  @file error.cpp
 */

#undef NDEBUG

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#define TDEBUG 1
#include "emp/base/error.hpp"

TEST_CASE("Test error", "[base]")
{
  // Error should not be thrown by default
  REQUIRE_FALSE(emp::error_thrown);
  // Throw an error and try to detect it!
  const std::string passed_error_string = "This is an error!";
  // Don't separate the next two lines or else the line number test will fail!
  const size_t expected_error_line = __LINE__ + 1;
  emp_error(passed_error_string);
  REQUIRE(emp::error_thrown);

  // Did we capture the correct filename and line number?
  const std::string caught_filename = emp::error_info.filename;
  REQUIRE_THAT(caught_filename, Catch::Matchers::EndsWith("error.cpp"));
  REQUIRE(emp::error_info.line_num == expected_error_line);

  // Test the output message
  REQUIRE(emp::error_info.output == passed_error_string);


  // Can we clear the error?
  REQUIRE(emp::error_thrown);
  emp::error_clear();
  REQUIRE_FALSE(emp::error_thrown);


  // 1. Do things get updated (we'll run it all again)
  // 2. Can we pass more than a string to emp_error?
  const int expected_error_line_2 = __LINE__ + 1;
  emp_error("test", 2, 4.5);
  REQUIRE(emp::error_thrown);
  // Check filename and line number again
  const std::string caught_filename_2 = emp::error_info.filename;
  REQUIRE_THAT(caught_filename_2, Catch::Matchers::EndsWith("error.cpp"));
  REQUIRE(emp::error_info.line_num == expected_error_line_2);

  // Test the output message
  REQUIRE(emp::error_info.output == "test24.5");
}
