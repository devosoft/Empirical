/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file DynamicString.cpp
 */

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/datastructs/DynamicString.hpp"

TEST_CASE("Test DynamicString", "[datastructs]")
{
  emp::DynamicString test_set;

  test_set.Append("Line Zero");  // Test regular append
  test_set << "Line One";        // Test stream append

  // Test append-to-stream
  std::stringstream ss;
  ss << test_set;
  REQUIRE(ss.str() == "Line ZeroLine One");

  // Test direct conversion to string.
  REQUIRE(test_set.str() == "Line ZeroLine One");

  // Test appending functions.
  test_set.Append( [](){ return std::string("Line Two"); } );
  test_set.Append( [](){ return "Line Three"; } );

  // Test appending functions with variable output
  int line_no = 20;
  test_set.Append( [&line_no](){ return std::string("Line ") + std::to_string(line_no); } );

  REQUIRE(test_set[4] == "Line 20");
  line_no = 4;
  REQUIRE(test_set[4] == "Line 4");

  // Make sure we can change an existing line.
  test_set.Set(0, "Line 0");
  REQUIRE(test_set[0] == "Line 0");

  // Make sure all lines are what we expect.
  REQUIRE(test_set[0] == "Line 0");
  REQUIRE(test_set[1] == "Line One");
  REQUIRE(test_set[2] == "Line Two");
  REQUIRE(test_set[3] == "Line Three");
  REQUIRE(test_set[4] == "Line 4");
}
