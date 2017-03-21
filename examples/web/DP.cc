//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include <string>

#include "tools/string_utils.h"
#include "web/web.h"

namespace UI = emp::web;

UI::Document doc("emp_base");

void TextCB(const std::string & in_str) {
  // Process the incoming string.
  std::string str(in_str);
  emp::remove_punctuation(str);
  emp::compress_whitespace(str);
  emp::vector<std::string> words = emp::slice(str, ' ');

  // Convert words to sizes.
  const size_t num_words = words.size();
  emp::vector<size_t> word_sizes(num_words);
  for (size_t i = 0; i < num_words; i++) {
    word_sizes[i] = words[i].size();
  }

  // Determine the score contribution for each word.
  emp::vector<size_t> score(num_words);
  emp::vector<bool> keep_last(num_words);
  if (num_words > 0) { score[0] = word_sizes[0]; keep_last[0] = true; }
  if (num_words > 1) {
    if (word_sizes[1] > word_sizes[0]) { score[1] = word_sizes[1]; keep_last[1] = true; }
    else { score[1] = word_sizes[0]; keep_last[1] = false; }
  }
  for (size_t w = 2; w < num_words; w++) {
    size_t score_no = score[w-1];                   // Don't keep last word.
    size_t score_yes = score[w-2] + word_sizes[w];  // DO keep last word.
    if (score_no >= score_yes) {
      score[w] = score_no;
      keep_last[w] = false;
    }
    else {
      score[w] = score_yes;
      keep_last[w] = true;
    }
  }

  // Map which words we keep.
  emp::vector<bool> keep(num_words, false);
  for (size_t i = num_words-1; i < num_words; i--) {
    if (keep_last[i]) { keep[i] = true; i--; }
  }

  // And print them on the screen.
  auto result = doc.Slate("result");
  result.Clear();
  result << "<br>";
  for (size_t i = 0; i < num_words; i++) {
    if (keep[i]) result << "<big><b>";
    result << words[i];
    if (keep[i]) result << "</b></big>";
    result << " ";
  }
  result << "<br><br><br>";

  // Print the results in a table.
  auto table = doc.Table("table");
  table.Resize(3, num_words+1);
  for (size_t i = 0; i < num_words; i++) {
    table.GetCell(0, i+1).ClearChildren() << words[i];
    table.GetCell(1, i+1).ClearChildren() << word_sizes[i];
    table.GetCell(2, i+1).ClearChildren() << score[i];
  }
  table.CellsCSS("border", "1px solid black");
  table.CellsCSS("padding", "5px");
  table.Redraw();
}

int main()
{
  doc << "<h2>Choose Your Words!</h2>";

  auto desc = doc.AddSlate("desc");
  desc.SetWidth(400);
  desc << "<p>Consider the following problem: you have a sentence and may pick any number of words, as long as no two words are consecutive.  Which words should you pick to have the maximum total number of letters?</p>";
  desc << "<p>Try typing a sentence here to see the results:</p>";
  auto ta = doc.AddTextArea(TextCB, "text_area");
  ta.SetSize(400, 80);

  auto result = doc.AddSlate("result");
  result << "<br><br>";

  auto tdesc = doc.AddSlate("tdesc");
  tdesc.SetWidth(400);
  tdesc << "<p>Here is the table that we use to perform this calculation:</p>";

  auto table = doc.AddTable(3, 1, "table");
  table.GetCell(0,0).SetHeader() << "WORD:";
  table.GetCell(1,0).SetHeader() << "LETTERS:";
  table.GetCell(2,0).SetHeader() << "BEST SCORE:";
  table.SetCSS("border-collapse", "collapse");  
  table.SetCSS("border", "3px solid black");
  table.CellsCSS("border", "1px solid black");
}
