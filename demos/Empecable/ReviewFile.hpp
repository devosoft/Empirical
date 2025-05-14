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
#include "../../include/emp/io/io_utils.hpp"
#include "../../include/emp/tools/String.hpp"
// #include "../../include/emp/tools/string_utils.hpp"

#include "helpers.hpp"
#include "Lexer.hpp"

namespace fs = std::filesystem;

class ReviewFile {
private:
  emp::String filename;
  fs::path path;
  std::unordered_set<emp::String> words; // Words to allow in this file specifically.
  Mode mode = Mode::Normal;              // How much I/O should we do?

  emp::vector<emplex::Token> tokens;     // Tokens that make up this file.
  bool save_required = false;            // Does this file need to be updated on disk?
  bool valid = false;                    // Is this a valid file to work with?

  emp::vector<emp::String> issues;       // List of issues found with this file.

public:
  ReviewFile(emp::String filename, Mode mode)
    : filename(filename), path(fs::absolute(filename.str())), mode(mode)
  {
    if (fs::exists(path) && fs::is_regular_file(path)) {
      path = fs::canonical(filename.str());
      valid = true;
    } else {
      std::cerr << ToBoldRed("Error:") << " file does not exist or is not a regular file: "
                << filename << "\n";
      valid = false;
    }
  }

  // ======= Accessors =======

  emp::String GetName() const { return filename; }
  fs::path GetPath() const { return path; }

  bool IsValid() const { return valid; }
  size_t MaxLineID() const { return tokens.size() ? tokens.back().line_id : 0; }


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
  size_t GetLineID(size_t pos) const { return tokens[pos].line_id; }
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

  // Find the first token of the target line (starting from line_id).
  size_t FindPos_LineStart(size_t token_pos, size_t line_id) const {
    // Scan forward if needed.
    while (tokens[token_pos].line_id < line_id) ++token_pos;

    // Scan backward, if needed.
    while (token_pos > 0 && tokens[token_pos-1].line_id >= line_id) --token_pos;

    return token_pos;
  }

  // Newline at the end of the target line (or EOF)
  size_t FindPos_LineEnd(size_t token_pos, size_t line_id) const {
    // Scan forward, if needed.
    while (token_pos < tokens.size() && tokens[token_pos].line_id <= line_id) ++token_pos;

    // scan backward, if needed.
    while (token_pos > 0 && tokens[token_pos-1].line_id > line_id) --token_pos;

    // Move on to the \n at the end of the line.
    if (token_pos && tokens[token_pos-1].lexeme == "\n") --token_pos;

    return token_pos;
  }

  emp::String GetTokenLine(size_t pos, size_t line_id) const {
    // Determine the set of tokens to print.
    const size_t start = FindPos_LineStart(pos, line_id);
    const size_t end = FindPos_LineEnd(pos, line_id);

    // Collect the tokens on this line, highlighting the target.
    emp::String result;
    for (size_t i = start; i < end; ++i) {
      if (i == pos) result += ToBoldRed(tokens[i].lexeme).AsANSIUnderline();
      else result += tokens[i].lexeme;
    }
    return result;
  }

  // Print a line near a given token, highlighting that token.
  void PrintTokenLine(size_t token_pos, size_t line_id) const {
    emp::String line_str = emp::MakeString(line_id).PadFront(' ', 5)+':';
    emp::String code_line = GetTokenLine(token_pos, line_id);
    PrintLn(line_str.AsANSIBrightWhite().AsANSIBold(), code_line);
  }

  // Print a range of lines around a given token.
  void PrintTokenRange(size_t token_pos, size_t range=1) {
    size_t line_id = tokens[token_pos].line_id;
    size_t min_line = (line_id > range) ? (line_id - range) : 0;
    size_t max_line = (line_id+range <= MaxLineID()) ? line_id+range : MaxLineID();
    for (size_t i = min_line; i <= max_line; ++i) {
      PrintTokenLine(token_pos, i);
    }
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
        if (line.ScanWord(pos) != "//") {
          InternalError(token, "Meta-data mismatch; expected: \"//\"");
        }
        if (line.ScanWord(pos) != "empecable_words:") {
          pos = 0; line.ScanWord(pos);
          InternalError(token, "Meta-data mismatch; expected: \"empecable_words:\"\n");
        }
        while (pos < line.size()) {
          words.insert(line.ScanWord(pos));
        }
      }
    }

    return true;
  }

  void Save() {
    if (!save_required) {
      if (mode == Mode::Interactive) {
        emp::PrintLn("No changes need to be saved in '", emp::ANSI::MakeGreen(filename), "'.");
      }
      return;
    }
    emp::PrintLn("Saving '", emp::ANSI::MakeGreen(filename),"'.");

    // Don't let cruft build up at the end of the file.
    while (tokens.back().id == emplex::Lexer::ID_END_LINE ||
           tokens.back().lexeme.empty()) {
      tokens.pop_back();
    }

    // Output the main body of the file.
    std::ofstream file(filename);
    for (auto & token : tokens) {
      file << token.lexeme;
    }

    // Attach a comment to the end of the file if there are local words to save.
    if (words.size()) {
      file << "\n\n// Special info below for local control over the Empecable file checker.\n"
           << "// empecable_words:";
      for (emp::String word : words) file << " " << word;
    }

    file << '\n'; // End the file in a single newline.

    file.close();
  }

  // ========== Issue Tracking ==========

  size_t GetNumIssues() const { return issues.size(); }

  // Print tokens to the screen with a particular token highlighted in read.
  void ReportIssue(emp::String issue, size_t token_pos) {
    issues.push_back(issue);

    // Report the issue.
    if (mode >= Mode::Verbose) {
      size_t line_num = GetLineID(token_pos);
      PrintLn(emp::MakeString("Found in ", filename, ":", line_num, ": ").AsANSIYellow().AsANSIBold(), issue);

      // Print the affected code.
      PrintTokenRange(token_pos, 1);
    }
  }

};