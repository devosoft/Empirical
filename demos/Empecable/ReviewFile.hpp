/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file
 * @brief Manage a single file being reviewed.
 */

#pragma once

// #include <cstring>
#include <filesystem>
#include <fstream>
// #include <iostream>
// #include <map>
// #include <optional>
// #include <set>
// #include <string>
// #include <unordered_set>

// #include "../../include/emp/base/assert.hpp"
// #include "../../include/emp/base/vector.hpp"
// #include "../../include/emp/config/command_line.hpp"
// #include "../../include/emp/config/FlagManager.hpp"
// #include "../../include/emp/io/File.hpp"
// #include "../../include/emp/io/io_utils.hpp"
#include "../../include/emp/tools/String.hpp"
// #include "../../include/emp/tools/string_utils.hpp"

#include "Lexer.hpp"

namespace fs = std::filesystem;

class ReviewFile {
private:
  emp::String filename;
  fs::path path;
  std::unordered_set<emp::String> words; // Words to allow in this file specifically.

  emp::vector<emplex::Token> tokens;     // Tokens that make up this file.
  bool save_required = false;            // Does this file need to be updated on disk?
  bool valid = false;                    // Is this a valid file to work with?

  emp::vector<emp::String> issues;       // List of issues found with this file.

public:
  ReviewFile(emp::String filename) : filename(filename), path(fs::absolute(filename.str())) {
    if (fs::exists(path) && fs::is_regular_file(path)) {
      path = fs::canonical(filename.str());
      valid = true;
    } else {
      std::cerr << ToBoldRed("Error:") << " file does not exist or is not a regular file: "
                << filename << "\n";
    }
  }

  // ======= Accessors =======

  emp::String GetName() const { return filename; }
  fs::path GetPath() const { return path; }

  bool IsValid() const { return valid; }


  // ======= Dictionary Management =======

  bool HasWord(emp::String word) const { return words.contains(word); }
  void AddWord(emp::String word) {
    emp_assert(!HasWord(word), word); // Words should not be added more than once.
    words.insert(word);
    PrintLn("Added '", word.AsANSICyan(), "' to file dictionary.");
    save_required = true;
  }

  // ======= Token Management =======

  emplex::Token GetToken(size_t pos) const { return tokens[pos]; }
  emp::String GetLexeme(size_t pos) const { return tokens[pos].lexeme; }
  int GetTokenID(size_t pos) const { return tokens[pos].id; }
  size_t NumTokens() const { return tokens.size(); }

  void ClearLexeme(size_t pos) {
    tokens[pos].lexeme.clear();
    save_required = true;
  }

  void SetLexeme(size_t pos, emp::String new_word) {
    tokens[pos].lexeme = new_word;
    save_required = true;
  } 

  // Find the spot after previous newline char or beginning of file.
  size_t FindPos_LineStart(size_t token_pos) const {
    while (token_pos > 0 && tokens[token_pos-1].lexeme != "\n") --token_pos;
    return token_pos;
  }

  // Find the next newline character (or EOF)
  size_t FindPos_LineEnd(size_t token_pos) const {
    while (token_pos < tokens.size() && tokens[token_pos].lexeme != "\n") ++token_pos;
    return token_pos;
  }

  emp::String GetTokenLine(size_t pos) const {
    // Determine the set of tokens to print.
    const size_t start = FindPos_LineStart(pos);
    const size_t end = FindPos_LineEnd(pos);

    // Collect the tokens on this line, highlighting the target.
    emp::String result;
    for (size_t i = start; i < end; ++i) {
      if (i == pos) result += ToBoldRed(tokens[i].lexeme).AsANSIUnderline();
      else result += tokens[i].lexeme;
    }
    return result;
  }

  bool Load(emplex::Lexer & lexer) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      PrintLn(ToBoldRed("ERROR: '", filename, "' failed to open."));
      return false;
    }
    tokens = lexer.Tokenize(file);
    issues.resize(0);    // Reset issues for new file.

    // Scan through file for Empecable instructions.
    for (auto & token : tokens) {
      if (token.id == emplex::Lexer::ID_EMP_META_WORDS) {
        const emp::String line = token.lexeme;
        token.lexeme = ""; // Clear out this lexeme; we will reconstruct it when saving.

        size_t pos = 0;
        if (line.ScanWord(pos) != "//") InternalError("Meta-data mismatch.");
        if (line.ScanWord(pos) != "empecable_words:") InternalError("Meta-data mismatch.");
        while (pos < line.size()) {
          words.insert(line.ScanWord(pos));
        }
      }
    }

    return true;
  }

  void Save() {
    if (!save_required) {
      PrintLn("No changes need to be saved in '", emp::ANSI::MakeGreen(filename), "'.");
      return;
    }
    PrintLn("Saving '", emp::ANSI::MakeGreen(filename),"'.");
    std::ofstream file(filename);
    for (auto & token : tokens) {
      file << token.lexeme;
    }

    // Attach a comment to the end of the file if there are local words to save.
    if (words.size()) {
      // Don't let cruft build up at the end of the file.
      while (tokens.back().id == emplex::Lexer::ID_END_LINE ||
             tokens.back().lexeme.empty()) {
        tokens.pop_back();
      }

      file << "\n\n// Special info below for local control over the Empecable file checker.\n"
           << "// empecable_words:";
      for (emp::String word : words) file << " " << word;
      file << '\n';
    }

    file.close();
  }

  // ========== Issue Tracking ==========

  size_t GetNumIssues() const { return issues.size(); }

  // Print tokens to the screen with a particular token highlighted in read.
  void ReportIssue(emp::String issue, size_t token_pos) {
    issues.push_back(issue);

    // Report the issue.
    PrintLn(emp::ANSI::MakeBold(emp::ANSI::MakeYellow("Found: ")), issue);

    // Print line number and the affected code.
    emp::String line_num = emp::MakeString(GetToken(token_pos).line_id).PadFront(' ', 5)+':';
    emp::String code_line = GetTokenLine(token_pos);
    PrintLn(line_num.AsANSIBrightWhite().AsANSIBold(), code_line);
  }

};