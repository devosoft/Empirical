/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022-2023.
 *
 *  @file HTMLText.cpp
 */

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/text/HTMLEncoding.hpp"

TEST_CASE("Testing HTMLText", "[text]") {
  emp::HTMLText text("Test Text");
  CHECK(text.GetSize() == 9);
  CHECK(text.AsString() == "Test Text");
  CHECK(text.Encode() == "Test Text");

  // Try adding style.
  text.Bold(5,9);
  CHECK(text.GetSize() == 9);
  CHECK(text.AsString() == "Test Text");
  CHECK(text.Encode() == "Test <b>Text</b>");

  // Try appending.
  text << " and more Text.";
  CHECK(text.GetSize() == 24);
  CHECK(text.AsString() == "Test Text and more Text.");
  CHECK(text.Encode() == "Test <b>Text</b> and more Text.");

  // Try changing letters.
  text[21] = 's';
  text[23] = 's';
  CHECK(text.GetSize() == 24);
  CHECK(text.AsString() == "Test Text and more Tests");
  CHECK(text.Encode() == "Test <b>Text</b> and more Tests");

  // Try making a change that involves style.
  text[19] = text[5];
  CHECK(text.GetSize() == 24);
  CHECK(text.AsString() == "Test Text and more Tests");
  CHECK(text.Encode() == "Test <b>Text</b> and more <b>T</b>ests");

  text.Italic(14,18);
  CHECK(text.GetSize() == 24);
  CHECK(text.AsString() == "Test Text and more Tests");
  CHECK(text.Encode() == "Test <b>Text</b> and <i>more</i> <b>T</b>ests");

  // Try erasing the text.
  text.Resize(0);
  CHECK(text.GetSize() == 0);
  CHECK(text.AsString() == "");
  CHECK(text.Encode() == "");
  CHECK(text.GetStyles().size() == 0);

  text << "This is <b><i>Pre-</i>formatted</b> text.";
  // text.PrintDebug();
  // CHECK(text.GetSize() == 27);
  CHECK(text.AsString() == "This is Pre-formatted text.");
  CHECK(text.Encode() == "This is <i><b>Pre-</i>formatted</b> text.");

  emp::Text plain_text("The <b> and </b> here should not be converted.");
  CHECK(plain_text.Encode() == "The <b> and </b> here should not be converted.");

  // Now let's bold the word "not".
  plain_text.Bold(29,32);

  // Should not change the output of Encode(), just internal style.
  CHECK(plain_text.Encode() == "The <b> and </b> here should not be converted.");
  CHECK(plain_text[28].IsBold() == false);
  CHECK(plain_text[29].IsBold() == true);
  CHECK(plain_text[31].IsBold() == true);
  CHECK(plain_text[32].IsBold() == false);

  // But if we put it into an HTML object, the style should be reflected.
  emp::HTMLText html_text(plain_text);
  CHECK(html_text.AsString() == "The <b> and </b> here should not be converted.");
  CHECK(html_text.Encode() == "The &lt;b&gt; and &lt;/b&gt; here should <b>not</b> be converted.");

  emp::HTMLText text2 = "Now let's try <b>something</b> with a non-breaking&nbsp;space.";
  text2.PrintDebug();
  CHECK(text2.AsString() == "Now let's try something with a non-breaking space.");
  CHECK(text2.Encode() == "Now let's try <b>something</b> with a non-breaking&nbsp;space.");

  emp::HTMLText text3 = "Three non-breaking spaces in a row&nbsp;&nbsp;&nbsp;!";
  CHECK(text3.AsString() == "Three non-breaking spaces in a row   !");
  CHECK(text3.Encode() == "Three non-breaking spaces in a row&nbsp;&nbsp;&nbsp;!");


  // Play with merging and converting.
  emp::HTMLText part1 = "Part 1 of text;";
  emp::HTMLText part2 = "And now part 2 of text.";
  part1.Bold(5,6);
  part2.Bold(13,14);
  part2.Italic(18,22);

  emp::Text merged = emp::MakeHTMLText(part1, " ", part2);
  CHECK(merged.AsString() == "Part 1 of text; And now part 2 of text.");
  CHECK(merged.Encode() == "Part <b>1</b> of text; And now part <b>2</b> of <i>text</i>.");
}
