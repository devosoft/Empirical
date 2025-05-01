/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2024-2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file
 * @brief Load a series of filenames and clean up each file.
 */

#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <unordered_set>

#include "../../include/emp/base/assert.hpp"
#include "../../include/emp/base/vector.hpp"
#include "../../include/emp/config/command_line.hpp"
#include "../../include/emp/io/File.hpp"
#include "../../include/emp/tools/String.hpp"
#include "../../include/emp/tools/string_utils.hpp"
#include "../../include/emp/config/FlagManager.hpp"

#include "PPLexer.hpp"
#include "WordLexer.hpp"

struct Checks {
  // Important fixes; on by default.
  bool require_copyright = true;      // Check Copyright at the front.
  bool prevent_end_spaces = true;     // Check the no lines end with spaces.
  bool disallow_tabs = true;          // Check that no tabs are in code.
  bool require_include_guards = true; // Check that the file is protected by include guards.
  bool require_pragma_once = true;    // Check that the file has a "#pragma once"
  bool sort_include = true;           // Check that the includes in the file are grouped correctly.

  // Warnings that you can turn on.
  size_t warn_line_width = 0;     // Warn if a line is too long (zero = off)

  void SetAll(bool _in=true) {
    require_copyright = _in;
    prevent_end_spaces = _in;
    disallow_tabs = _in;
    require_include_guards = _in;
    require_pragma_once = _in;
    sort_include = _in;
  }
};

class Empecable {
private:
  // Options used:
  Checks checks;
  bool verbose = false;
  bool interactive = false;

  emp::FlagManager flags;

  emplex::PPLexer pp_lexer;
  emplex::WordLexer word_lexer;

  emp::String word_file = "word_list.txt";
  using word_set_t = std::unordered_set<emp::String>;
  word_set_t words;                // Dictionary of legal words.
  word_set_t new_local_words;      // New words to add to just in this file.
  word_set_t new_global_words;     // New words to add to .Empecable file.
  size_t issue_count = 0;          // Number of issues detected in current file.

  // Collect issues



  // === HELPER FUNCTIONS ===

  template <typename... Ts>
  void PrintBoldRed(Ts &&... args) {
    std::cout << emp::MakeString(args...).AsANSIRed().AsANSIBold();
  }

  void SetupOptionFlags() {
    flags.AddGroup("Basic Operation");
    flags.AddOption('a', "all", [this](){ SetAll(true); },
      "Activate ALL fixes (except those explicitly excluded; see below)");
    flags.AddOption('h', "help", [this](){ PrintHelp(); },
      "Get additional information about options.");
    flags.AddOption('i', "interactive", [this](){ interactive = true; },
      "Interactively fix file problems");
    flags.AddOption('v', "verbose", [this](){ verbose = true; },
      "Provide more detailed output");
    flags.AddOption('w', "word_file", [this](emp::String filename){ word_file = filename; },
      "Specify the word file to use for spell checks.");

    flags.AddGroup("Activating Specific Fixes");
    flags.AddOption("require_copyright", [this](){ checks.require_copyright = true; });
    flags.AddOption("prevent_end_spaces", [this](){ checks.prevent_end_spaces = true; });
    flags.AddOption("disallow_tabs", [this](){ checks.disallow_tabs = true; });
    flags.AddOption("require_include_guards", [this](){ checks.require_include_guards = true; });
    flags.AddOption("require_pragma_once", [this](){ checks.require_pragma_once = true; });
    flags.AddOption("sort_include", [this](){ checks.sort_include = true; });

    flags.AddGroup("Deactivate Specific Fixes");
    flags.AddOption("no_require_copyright", [this](){ checks.require_copyright = false; });
    flags.AddOption("no_prevent_end_spaces", [this](){ checks.prevent_end_spaces = false; });
    flags.AddOption("no_disallow_tabs", [this](){ checks.disallow_tabs = false; });
    flags.AddOption("no_sort_include_guards", [this](){ checks.require_include_guards = false; });
    flags.AddOption("no_require_pragma_once", [this](){ checks.require_pragma_once = false; });
    flags.AddOption("no_sort_include", [this](){ checks.sort_include = false; });

    flags.AddGroup("Extra Warnings to Enable (off by default)");
    flags.AddOption('W', "max_line_width",
      [this](emp::String max_w){ checks.warn_line_width = max_w.AsULL(); } );
  }

  void LoadWords() {
    if (words.size() == 0) {
      std::ifstream file(word_file);
      emp::String word;
      while (std::getline(file, word)) {
        words.insert(word.Filter(emp::IDCharSet()));
      }
    }
  }

  void SaveGlobalWords() {
    if (new_global_words.size() == 0) return;

    // Move the words to a vector and sort them.
    emp::vector<emp::String> out_words;
    out_words.reserve(words.size());
    for (emp::String word : words) out_words.emplace_back(word);
    std::sort(out_words.begin(), out_words.end());

    // Print the new file
    std::ofstream file(word_file);
    for (emp::String word : out_words) file << word << '\n';
  }

  void AddGlobalWord(emp::String word) {
    emp_assert(!emp::Has(new_global_words, word), word);
    new_global_words.insert(word);
  }

  void AddLocalWord(emp::String word) {
    emp_assert(!emp::Has(new_local_words, word), word);
    new_local_words.insert(word);
  }

  // 'd' - Delete (or 'D' to Delete All)
  // 'r' - Replace with (or 'R' to Replace All)


  bool TestWord(emp::String word) const {
    // If a word is short (2 or fewer letters) or is in the dictionary (either directly or or as
    // a lowercase word), mark it as valid.
    if (word.size() <= 2 || words.contains(word) || words.contains(word.AsLower())) return true;

    // Otherwise, we should see if we need to break up the word.  For example, "TestWord" should
    // be allowed because both "Test" and "Word" are allowed.  If it were all lowercase, it should
    // be written as "test_word", which would already have been split on the '_'.
    size_t upper_pos = 0;
    if (word.HasLower() && (upper_pos = word.FindUpper(1)) != emp::String::npos) {
      return TestWord(word.PopFixed(upper_pos)) && TestWord(word);
    }

    return false;
  }

  // Print tokens to the screen with a particular token highlighted in read.
  template <typename STREAM_T>
  void ReportError(emp::String error, const STREAM_T & tokens, size_t token_id) {
    namespace ANSI = emp::ANSI;

    // Report the actual error.
    std::cout << ANSI::MakeBold(ANSI::MakeYellow("Found: ")) << error << '\n';

    // Determine the set of tokens to print.
    size_t start = token_id;
    size_t end = token_id+1;
    // Rewind to beginning of line.
    while (start > 0 && tokens[start-1].lexeme != "\n") --start;
    // Fast forward to end of line.
    while ((end < tokens.size() && tokens[end].lexeme != "\n")) ++end;

    // Format and print the line number.
    emp::String line_num = emp::MakeString(tokens[token_id].line_id).PadFront(' ', 5)+':';
    // std::cout << line_num.AsANSIYellow();
    std::cout << line_num.AsANSIBrightWhite().AsANSIBold();

    // Print the series of tokens on this line, highlighting the problem.
    for (size_t i = start; i < end; ++i) {
      if (i == token_id) PrintBoldRed(tokens[i].lexeme).AsANSIUnderline();
      else std::cout << tokens[i].lexeme;
    }
    std::cout << '\n';
  }

public:
  Empecable(int argc, char * argv[]) : flags(argc, argv) {
    SetupOptionFlags();
    flags.Process();

    LoadWords();

    auto filenames = flags.GetExtras();
    if (filenames.size() == 0) {
      std::cout << "No files listed." << std::endl;
      PrintUsage();
    }
    for (emp::String filename : filenames) {
      ProcessFile(filename);
    }
  }

  ~Empecable() {
    SaveGlobalWords();
  }

  Empecable & SetAll(bool _in=true) { checks.SetAll(_in); return *this; }

  Empecable & SetVerbose(bool _in) { verbose = _in; return *this; }

  void PrintUsage() const {
    std::cout << "Usage: " << flags[0] << " {options ...} files ..." << std::endl;
    std::cout << "Type `" << flags[0] << " -h` for more detailed help." << std::endl;
  }

  void PrintVersion() const {
    std::cout << "File formatter version 0.1." << std::endl;
  }

  void PrintHelp() const {
    PrintVersion();
    std::cout << '\n';
    PrintUsage();
    std::cout << '\n';
    flags.PrintOptions();
    exit(0);
  }

  bool ProcessFile(emp::String filename) {
    // Interface options
    // 'd' - Delete (or 'D' to Delete All)
    // 'r' - Replace with (or 'R' to Replace All)
    // 's' - Skip (or 'S' to Skip All)
    // 'f' - Add word to file dictionary (or 'F' to add all lowercase version of word)
    // 'p' - Add word to project dictionary (or 'P' to add all lowercase version of word)
    // 'q' - Quit
    // 'h' - Help

    std::cout << "=== File: " << filename.AsANSIBrightCyan() << " ===\n";
    issue_count = 0;    // Reset issues for new file.

    std::ifstream file(filename);
    if (!file.is_open()) {
      PrintBoldRed("ERROR: '", filename, "' failed to open.\n");
      return false;
    }

    // First check spelling and illegal character placement.
    auto tokens = word_lexer.Tokenize(file);

    // For each misspelled word, track the token ids associated with it.
    std::map<emp::String, std::vector<size_t>> word_ids;
    emp::String error = "";

    for (size_t token_id = 0; token_id < tokens.size(); ++token_id) {
      const auto & token = tokens[token_id];
      switch (token.id) {
        using namespace emplex;
      case WordLexer::ID_WORD:
        if (!TestWord(token.lexeme)) {
          word_ids[token.lexeme].push_back(token_id);
          ++issue_count;  
        }
        break;
      case WordLexer::ID_ERR_END_LINE_WS:        
        error = emp::MakeString("Extra whitespace at end of line:");
        ReportError(error, tokens, token_id);
        ++issue_count;
        break;
      case WordLexer::ID_ERR_WS:
        error = emp::MakeString("Illegal whitespace:");
        ReportError(error, tokens, token_id);
        ++issue_count;
        break;
      default:
        break;
      }
    }

    for (auto & [word, ids] : word_ids) {
      error = emp::MakeString("Unknown word '", word.AsANSICyan(),
        "' appears ", ids.size(), " times; first occurrence:");
      ReportError(error, tokens, ids[0]);
    }

    std::cout << issue_count << " issues found." << std::endl;

    return issue_count;
  }
};

int main(int argc, char * argv[])
{
  class Empecable formatter(argc, argv);
}
