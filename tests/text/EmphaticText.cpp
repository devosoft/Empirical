/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2022-2024
*/
/**
 *  @file
 */

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/text/EmphaticEncoding.hpp"

TEST_CASE("Testing EmphaticText", "[text]") {
  emp::EmphaticText text("Test Text");
  text.PrintDebug();
  CHECK(text.GetSize() == 9);
  CHECK(text.AsString() == "Test Text");
  CHECK(text.Encode() == "Test Text");

  // Try adding style.
  text.Bold(5,9);
  CHECK(text.GetSize() == 9);
  CHECK(text.AsString() == "Test Text");
  CHECK(text.Encode() == "Test `*Text`*");

  // Try appending.
  text << " and more Text.";
  CHECK(text.GetSize() == 24);
  CHECK(text.AsString() == "Test Text and more Text.");
  CHECK(text.Encode() == "Test `*Text`* and more Text.");

  // Try changing letters.
  text[21] = 's';
  text[23] = 's';
  CHECK(text.GetSize() == 24);
  CHECK(text.AsString() == "Test Text and more Tests");
  CHECK(text.Encode() == "Test `*Text`* and more Tests");

  // Try making a change that involves style.
  text[19] = text[5];
  CHECK(text.GetSize() == 24);
  CHECK(text.AsString() == "Test Text and more Tests");
  CHECK(text.Encode() == "Test `*Text`* and more `*T`*ests");

  // Try erasing the text.
  text.Resize(0);
  CHECK(text.GetSize() == 0);
  CHECK(text.AsString() == "");
  CHECK(text.Encode() == "");
  CHECK(text.GetStyles().size() == 0);

  text << "This is `/`*Pre-`/formatted`* text.";
  // CHECK(text.GetSize() == 27);
  CHECK(text.AsString() == "This is Pre-formatted text.");
  CHECK(text.Encode() == "This is `*`/Pre-`/formatted`* text.");

  emp::Text plain_text("The `* and `* here should not be converted.");
  CHECK(plain_text.Encode() == "The `* and `* here should not be converted.");

  // Now let's bold the word "not".
  plain_text.Bold(26,29);

  // Should not change the output of Encode(), just internal style.
  CHECK(plain_text.Encode() == "The `* and `* here should not be converted.");
  CHECK(plain_text[25].IsBold() == false);
  CHECK(plain_text[26].IsBold() == true);
  CHECK(plain_text[28].IsBold() == true);
  CHECK(plain_text[29].IsBold() == false);

  // But if we put it into an HTML object, the style should be reflected.
  emp::EmphaticText emphatic_text(plain_text);
  CHECK(emphatic_text.AsString() == "The `* and `* here should not be converted.");
  CHECK(emphatic_text.Encode() == "The \\`* and \\`* here should `*not`* be converted.");
}
