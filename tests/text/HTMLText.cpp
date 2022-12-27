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

#include "emp/text/HTMLText.hpp"

TEST_CASE("Testing HTMLText", "[text]") {
  emp::HTMLText text("Test Text");
  REQUIRE(text.GetSize() == 9);
  REQUIRE(text.GetText() == "Test Text");
  REQUIRE(text.ToString() == "Test Text");

  // Try adding style.
  text.Bold(5,9);
  REQUIRE(text.GetSize() == 9);
  REQUIRE(text.GetText() == "Test Text");
  REQUIRE(text.ToString() == "Test <b>Text</b>");

  // Try appending.
  text << " and more Text.";
  REQUIRE(text.GetSize() == 24);
  REQUIRE(text.GetText() == "Test Text and more Text.");
  REQUIRE(text.ToString() == "Test <b>Text</b> and more Text.");

  // Try changing letters.
  text[21] = 's';
  text[23] = 's';
  REQUIRE(text.GetSize() == 24);
  REQUIRE(text.GetText() == "Test Text and more Tests");
  REQUIRE(text.ToString() == "Test <b>Text</b> and more Tests");

  // Try making a change that involves style.
  text[19] = text[5];
  REQUIRE(text.GetSize() == 24);
  REQUIRE(text.GetText() == "Test Text and more Tests");
  REQUIRE(text.ToString() == "Test <b>Text</b> and more <b>T</b>ests");

  // Try erasing the text.
  text.Resize(0);
  REQUIRE(text.GetSize() == 0);
  REQUIRE(text.GetText() == "");
  REQUIRE(text.ToString() == "");
  REQUIRE(text.GetStyles().size() == 0);

  text << "This is <i><b>Pre-</i>formatted</b> text.";
  // REQUIRE(text.GetSize() == 27);
  REQUIRE(text.GetText() == "This is Pre-formatted text.");
  REQUIRE(text.ToString() == "This is <i><b>Pre-</i>formatted</b> text.");

  emp::Text plain_text("The <b> and </b> here should not be converted.");
  REQUIRE(plain_text.ToString() == "The <b> and </b> here should not be converted.");

  // Now let's bold the word "not".
  plain_text.Bold(29,32);

  // Should not change the output of ToString(), just internal style.
  REQUIRE(plain_text.ToString() == "The <b> and </b> here should not be converted.");
  REQUIRE(plain_text[28].IsBold() == false);
  REQUIRE(plain_text[29].IsBold() == true);
  REQUIRE(plain_text[31].IsBold() == true);
  REQUIRE(plain_text[32].IsBold() == false);

  // But if we put it into an HTML object, the style should be reflected.
  emp::HTMLText html_text(plain_text);
  REQUIRE(html_text.GetText() == "The <b> and </b> here should not be converted.");
  REQUIRE(html_text.ToString() == "The &lt;b&gt; and &lt;/b&gt; here should <b>not</b> be converted.");
}
