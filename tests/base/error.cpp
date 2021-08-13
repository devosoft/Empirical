//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2021.
//  Released under the MIT Software license; see doc/LICENSE

#define CATCH_CONFIG_MAIN
#undef NDEBUG
#define TDEBUG 1

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/base/error.hpp"

#include <sstream>
#include <iostream>

TEST_CASE("Test error", "[base]")
{
  // Error should not be thrown by default
  REQUIRE_FALSE(emp::error_thrown);
  // Throw an error and try to detect it! 
  std::string passed_error_string = "This is an error!";
  // Don't separate the next two lines or else the line number test will fail!
  size_t expected_error_line = __LINE__ + 1;
  emp_error(passed_error_string);
  REQUIRE(emp::error_thrown);

  // Did we capture the correct filename and line number? 
  // Trim the filename in case the test is run from elsewhere
  std::string caught_filename = emp::error_info.filename;
  std::string expected_filename = "error.cpp";
	REQUIRE(caught_filename.substr(caught_filename.size()-expected_filename.size()) == expected_filename);
  REQUIRE(emp::error_info.line_num == expected_error_line);
  
  // Test the output message
  REQUIRE(emp::error_info.output == passed_error_string);

  
  // Can we clear the error?
  REQUIRE(emp::error_thrown);
  emp::error_clear();
  REQUIRE_FALSE(emp::error_thrown);

  
  // 1. Do things get updated (we'll run it all again)
  // 2. Can we pass more than a string to emp_error?
  expected_error_line = __LINE__ + 1;
  emp_error("test", 2, 4.5);
  REQUIRE(emp::error_thrown);
  // Check filename and line number again
  caught_filename = emp::error_info.filename;
	REQUIRE(caught_filename.substr(caught_filename.size()-expected_filename.size()) == expected_filename);
  REQUIRE(emp::error_info.line_num == expected_error_line);
  
  // Test the output message
  REQUIRE(emp::error_info.output == "test24.5");

}
