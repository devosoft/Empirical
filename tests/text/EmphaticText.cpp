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

#include "emp/text/EmphaticText.hpp"

TEST_CASE("Testing EmphaticText", "[text]") {
  emp::EmphaticText text("Test Text");
  REQUIRE(text.GetSize() == 9);
  REQUIRE(text.GetText() == "Test Text");
  REQUIRE(text.Encode() == "Test Text");

  // Try adding style.
  text.Bold(5,9);
  REQUIRE(text.GetSize() == 9);
  REQUIRE(text.GetText() == "Test Text");
  REQUIRE(text.Encode() == "Test `*Text`*");

  // Try appending.
  text << " and more Text.";
  REQUIRE(text.GetSize() == 24);
  REQUIRE(text.GetText() == "Test Text and more Text.");
  REQUIRE(text.Encode() == "Test `*Text`* and more Text.");

  // Try changing letters.
  text[21] = 's';
  text[23] = 's';
  REQUIRE(text.GetSize() == 24);
  REQUIRE(text.GetText() == "Test Text and more Tests");
  REQUIRE(text.Encode() == "Test `*Text`* and more Tests");

  // Try making a change that involves style.
  text[19] = text[5];
  REQUIRE(text.GetSize() == 24);
  REQUIRE(text.GetText() == "Test Text and more Tests");
  REQUIRE(text.Encode() == "Test `*Text`* and more `*T`*ests");

  // Try erasing the text.
  text.Resize(0);
  REQUIRE(text.GetSize() == 0);
  REQUIRE(text.GetText() == "");
  REQUIRE(text.Encode() == "");
  REQUIRE(text.GetStyles().size() == 0);

  text << "This is `/`*Pre-`/formatted`* text.";
  // REQUIRE(text.GetSize() == 27);
  REQUIRE(text.GetText() == "This is Pre-formatted text.");
  REQUIRE(text.Encode() == "This is `*`/Pre-`/formatted`* text.");

  emp::Text plain_text("The `* and `* here should not be converted.");
  REQUIRE(plain_text.Encode() == "The `* and `* here should not be converted.");

  // Now let's bold the word "not".
  plain_text.Bold(26,29);

  // Should not change the output of Encode(), just internal style.
  REQUIRE(plain_text.Encode() == "The `* and `* here should not be converted.");
  REQUIRE(plain_text[25].IsBold() == false);
  REQUIRE(plain_text[26].IsBold() == true);
  REQUIRE(plain_text[28].IsBold() == true);
  REQUIRE(plain_text[29].IsBold() == false);

  // But if we put it into an HTML object, the style should be reflected.
  emp::EmphaticText emphatic_text(plain_text);
  REQUIRE(emphatic_text.GetText() == "The `* and `* here should not be converted.");
  REQUIRE(emphatic_text.Encode() == "The \\`* and \\`* here should `*not`* be converted.");
}
