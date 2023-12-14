/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2022
*/
/**
 *  @file
 */

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/text/Text.hpp"

TEST_CASE("Test Text", "[text]") {
  emp::Text text("Test Text");
  REQUIRE(text.GetSize() == 9);
  REQUIRE(text.GetText() == "Test Text");
  REQUIRE(text.HasBold(0) == false);
  REQUIRE(text.HasBold(5) == false);

  // Try adding style.
  text.Bold(5,9);
  REQUIRE(text.GetSize() == 9);
  REQUIRE(text.GetText() == "Test Text");
  REQUIRE(text.HasBold(0) == false);
  REQUIRE(text.HasBold(5) == true);

  // Try appending.
  text << " and more Text.";
  REQUIRE(text.GetSize() == 24);
  REQUIRE(text.GetText() == "Test Text and more Text.");
  REQUIRE(text.HasBold(0) == false);
  REQUIRE(text.HasBold(5) == true);
  REQUIRE(text.HasBold(18) == false);
  REQUIRE(text.HasBold(19) == false);
  REQUIRE(text.HasBold(20) == false);

  // Try changing letters.
  text[21] = 's';
  text[23] = 's';
  REQUIRE(text.GetSize() == 24);
  REQUIRE(text.GetText() == "Test Text and more Tests");
  REQUIRE(text.HasBold(0) == false);
  REQUIRE(text.HasBold(5) == true);
  REQUIRE(text.HasBold(18) == false);
  REQUIRE(text.HasBold(19) == false);
  REQUIRE(text.HasBold(20) == false);

  // Try making a change that involves style.
  text[19] = text[5];
  REQUIRE(text.GetSize() == 24);
  REQUIRE(text.GetText() == "Test Text and more Tests");
  REQUIRE(text.HasBold(0) == false);
  REQUIRE(text.HasBold(5) == true);
  REQUIRE(text.HasBold(18) == false);
  REQUIRE(text.HasBold(19) == true);
  REQUIRE(text.HasBold(20) == false);
}
