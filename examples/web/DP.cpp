//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include <string>

#include "emp/tools/string_utils.hpp"
#include "emp/web/web.hpp"

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
  emp::vector<char> keep_last(num_words);
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
  emp::vector<char> keep(num_words, false);
  for (size_t i = num_words-1; i < num_words; i--) {
    if (keep_last[i]) { keep[i] = true; i--; }
  }

  // And print them on the screen.
  auto result = doc.Div("result");
  result.Clear();
  result << "<p>Words (without punctuation) with the best selection in bold:</p>";
  for (size_t i = 0; i < num_words; i++) {
    if (keep[i]) result << "<big><b>";
    result << words[i];
    if (keep[i]) result << "</b></big>";
    result << " ";
  }
  result << "<br><br>Best Score = " << score[num_words-1] << "<br><br>";

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
  std::string start_text = "Erase this sentence and type another one that you like better.";
  const int width = 600;

  doc << "<h2>Choose Your Words!</h2>";

  auto desc = doc.AddDiv("desc");
  desc.SetWidth(width);
  desc << "<p>Consider the following problem: you have a sentence and want to select words from that sentence.  If you pick a word, you may not pick either of its neighbors.  Which words should you pick to have the maximum total number of letters?</p>"
       << "<p>For example, if you had \"This is a sentence\", you could get 12 points by picking the words 'This' and 'sentence'.  You could not pick both 'This' and 'is' because they are next to each other.</p>"
       << "<p>Try typing a sentence here to see the results:</p>";
  auto ta = doc.AddTextArea(TextCB, "text_area");
  ta.SetSize(width, 80);
  ta.SetText(start_text);

  auto result = doc.AddDiv("result");
  result << "<br><br>";

  auto tdesc = doc.AddDiv("tdesc");
  tdesc.SetWidth(width);
  tdesc << "<p>How did we solve this problem?  Consider the recursive approach.  If we knew the best possible score if we had only the first <i>n</i>-2 words, and we also knew the best possible score for the first <i>n</i>-1 words, do these values help us find the best score for all <i>n</i> words?</p>"
	<< "<p>Yes!</p>"
	<< "<p>For word <i>n</i>, we can either exclude the word from the answer (and just use the anser same score as <i>n</i>-1) -or- we can include it.  If we include it, we must exclude <i>n</i>-1, so we take the length of word <i>n</i> and add it to the best score for the first <i>n</i>-2 words.</p>";
  tdesc << "<p>Here is the table that we use to perform this calculation:</p>";

  auto table = doc.AddTable((size_t)3, (size_t)1, "table");
  table.GetCell(0,0).SetHeader() << "WORD:";
  table.GetCell(1,0).SetHeader() << "LETTERS:";
  table.GetCell(2,0).SetHeader() << "BEST SCORE:";
  table.SetCSS("border-collapse", "collapse");
  table.SetCSS("border", "3px solid black");
  table.CellsCSS("border", "1px solid black");

  TextCB(start_text);
}
