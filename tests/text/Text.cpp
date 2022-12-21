/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022
 *
 *  @file Text.cpp
 */

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/text/Text.hpp"

TEST_CASE("Test Text", "[text]") {
  emp::Text text("Test Text");
  REQUIRE(text.GetSize() == 9);
  REQUIRE(text.GetString() == "Test Text");
  REQUIRE(text.AsHTML() == "Test Text");

  text.Bold(5,9);
  REQUIRE(text.GetSize() == 9);
  REQUIRE(text.GetString() == "Test Text");
  REQUIRE(text.AsHTML() == "Test <b>Text</b>");

}
