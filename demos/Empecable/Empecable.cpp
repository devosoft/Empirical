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

  emp::FlagManager flags;

  emplex::PPLexer pp_lexer;
  emplex::WordLexer word_lexer;

  emp::String word_file = "word_list.txt";
  std::unordered_set<emp::String> words; // Dictionary of legal words.
  bool save_words = false;               // If words are added to the dictionary, save at end.

  void SetupOptionFlags() {
    flags.AddGroup("Basic Operation");
    flags.AddOption('a', "all", [this](){ SetAll(true); },
      "Activate ALL fixes (except those explicitly excluded; see below)");
    flags.AddOption('h', "help", [this](){ PrintHelp(); },
      "Get additional information about options.");
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

  void SaveWords() {
    emp::vector<emp::String> out_words;
    out_words.reserve(words.size());
    for (emp::String word : words) out_words.emplace_back(word);
    std::sort(out_words.begin(), out_words.end());
    std::ofstream file(word_file);
    for (emp::String word : out_words) file << word << '\n';
  }

  bool TestWord(emp::String word) const {
    return word.size() <= 2 || words.contains(word) || words.contains(word.AsLower());
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
    if (save_words) SaveWords();
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
    std::cout << "File: " << filename << std::endl;

    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cout << "ERROR: '" << filename << "' failed to open.";
      return false;
    }

    size_t issue_count = 0;

    // First check spelling and illegal character placement.
    auto tokens = word_lexer.Tokenize(file);

    for (const auto & token : tokens) {
      switch (token.id) {
        using namespace emplex;
      case WordLexer::ID_WORD:
        if (!TestWord(token.lexeme)) {
          std::cerr << "LINE " << token.line_id << ": Unknown word '" << token.lexeme << "'.\n";
          ++issue_count;
        }
        break;
      case WordLexer::ID_ERR_END_LINE_WS:
        std::cerr << "LINE " << token.line_id << ": Extra whitespace at end of line.\n";
        ++issue_count;
        break;
      case WordLexer::ID_ERR_WS:
        std::cerr << "LINE " << token.line_id << ": Illegal whitespace.\n";
        ++issue_count;
        break;
      default:
        break;
      }
    }

    std::cout << issue_count << " issues found." << std::endl;

    return issue_count;
  }
};

int main(int argc, char * argv[])
{
  class Empecable formatter(argc, argv);
}
