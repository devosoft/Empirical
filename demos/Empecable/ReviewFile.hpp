/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file
 * @brief Manage a single file being reviewed.
 */

#pragma once

#include <filesystem>
#include <fstream>

#include "../../include/emp/io/io_utils.hpp"
#include "../../include/emp/tools/String.hpp"

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

  size_t token_pos = 0;                  // What is the next token to use?
  emp::vector<emp::String> issues;       // List of issues found with this file.
  size_t pragma_once_line = 0;           // Have we found a "#pragma once" in this file?

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

  [[nodiscard]] emp::String GetName() const { return filename; }
  [[nodiscard]] fs::path GetPath() const { return path; }

  [[nodiscard]] bool IsValid() const { return valid; }
  [[nodiscard]] size_t MaxLineID() const { return tokens.size() ? tokens.back().line_id : 0; }

  [[nodiscard]] size_t GetPragmaOnce() const { return pragma_once_line; }
  void SetPragmaOnce() { pragma_once_line = GetLineID(token_pos); }

  // ======= Dictionary Management =======

  bool HasWord(emp::String word) const { return words.contains(word); }
  void AddWord(emp::String word) {
    emp_assert(!HasWord(word), word); // Words should not be added more than once.
    words.insert(word);
    PrintLn("Added '", word.AsANSICyan(), "' to file dictionary.");
    save_required = true;
  }

  // ======= Token Management =======

  emplex::Token NextToken() {
    if (token_pos >= tokens.size()) return emplex::Token{0,"",0}; // No tokens left.
    return tokens[token_pos++];
  }

  emplex::Token GetToken() const { return tokens[token_pos]; }
  emp::String GetLexeme() const { return tokens[token_pos].lexeme; }
  size_t GetLineID() const { return tokens[token_pos].line_id; }
  int GetTokenID() const { return tokens[token_pos].id; }
  emplex::Token GetToken(size_t pos) const { return tokens[pos]; }
  emp::String GetLexeme(size_t pos) const { return tokens[pos].lexeme; }
  size_t GetLineID(size_t pos) const { return tokens[pos].line_id; }
  int GetTokenID(size_t pos) const { return tokens[pos].id; }
  size_t NumTokens() const { return tokens.size(); }

  void ClearLexeme(size_t pos) {
    tokens[pos].lexeme.clear();
    save_required = true;
  }
  void ClearLexeme() { ClearLexeme(token_pos); }

  void SetLexeme(size_t pos, emp::String new_word) {
    tokens[pos].lexeme = new_word;
    save_required = true;
  }
  void SetLexeme(emp::String new_word) { SetLexeme(token_pos, new_word); }

  // Find a token anywhere on the target line.
  size_t FindPos_Line(size_t target_line) const {
    emp_assert(target_line >= tokens[0].line_id && target_line <= tokens.back().line_id);
    size_t min=0, max=tokens.size();
    while (tokens[min].line_id != target_line) {
      size_t mid = (min+max)/2;
      if (tokens[mid].line_id > target_line) max = mid;
      else min = mid;
    }
    return min;
  }

  // Find the first token of the target line.
  size_t FindPos_LineStart(size_t target_line) const {
    size_t pos = FindPos_Line(target_line);
    while (pos > 0 && tokens[pos-1].line_id == target_line) --pos;
    return pos;
  }

  // Find the first token of the target line.
  size_t FindPos_LineEnd(size_t target_line) const {
    size_t pos = FindPos_Line(target_line);
    while (pos+1 < tokens.size() && tokens[pos+1].line_id == target_line) ++pos;
    return pos;
  }


  emp::String GetTokenLine(size_t line_id) const {
    // Determine the set of tokens to print.
    const size_t start = FindPos_LineStart(line_id);
    const size_t end = FindPos_LineEnd(line_id);

    // Collect the tokens on this line, highlighting the current token.
    emp::String result;
    for (size_t i = start; i < end; ++i) {
      if (i == token_pos) result += ToBoldRed(tokens[i].lexeme).AsANSIUnderline();
      else result += tokens[i].lexeme;
    }
    return result;
  }

  // Print a line near a given token, highlighting that token.
  void PrintLine(size_t line_id) const {
    emp::String line_num_str = emp::MakeString(line_id).PadFront(' ', 5)+':';
    emp::String code_line = GetTokenLine(line_id);
    PrintLn(line_num_str.AsANSIBrightWhite().AsANSIBold(), code_line);
  }

  // Print a range of lines the current token.
  void PrintTokenRange(size_t range=1) {
    size_t line_id = tokens[token_pos].line_id;
    size_t min_line = (line_id > range) ? (line_id - range) : 0;
    size_t max_line = (line_id+range <= MaxLineID()) ? line_id+range : MaxLineID();
    for (size_t i = min_line; i <= max_line; ++i) {
      PrintLine(i);
    }
  }

  bool Load(emplex::Lexer & lexer, const std::unordered_set<emp::String> & project_words) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      PrintLn(ToBoldRed("ERROR: '", filename, "' failed to open."));
      return false;
    }
    tokens = lexer.Tokenize(file);
    token_pos = 0;       // Reset the next token to be analyzed.
    issues.resize(0);    // Reset issues for new file.

    // Scan through file for Empecable instructions.
    for (auto & token : tokens) {
      switch (token) {
      case emplex::Lexer::ID_EMP_META_START:
        token.lexeme = ""; // Clear out this lexeme; we will reconstruct it when saving.        
        break;
      case emplex::Lexer::ID_EMP_META_START_OLD:
        SetLexeme(""); // Clear out this lexeme; record change to ensure file save later.        
        break;
      case emplex::Lexer::ID_EMP_META_WORDS: {
        const emp::String line = token.lexeme;
        token.lexeme = ""; // Clear out this lexeme; we will reconstruct it if we save.

        size_t pos = 0;
        if (line.ScanWord(pos) != "//" ||
            line.ScanWord(pos) != "empecable_words:") {
          InternalError(token, "Meta-data mismatch; expected: \"// empecable_words:\"\n");
        }
        while (pos < line.size()) {
          const emp::String word = line.ScanWord(pos);
          if (project_words.contains(word)) {
            save_required = true;  // Word is now in project; don't keep as part of this file.
          } else {
            words.insert(word);
          }
        }
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
      file << "\n\n// Local settings for Empecable file checker.\n"
           << "// empecable_words:";
      for (emp::String word : words) file << " " << word;
    }

    file << '\n'; // End the file in a single newline.

    file.close();
  }

  // ========== Issue Tracking ==========

  size_t GetNumIssues() const { return issues.size(); }

  // Print tokens to the screen with a particular token highlighted in read.
  template <typename... ARG_Ts>
  void ReportIssue(ARG_Ts &&... args) {
    emp::String issue = emp::MakeString(std::forward<ARG_Ts>(args)...);
    issues.push_back(issue);

    // Report the issue.
    if (mode >= Mode::Verbose) {
      size_t line_num = GetLineID(token_pos);
      PrintLn(emp::MakeString("Found in ", filename, ":", line_num, ": ").AsANSIYellow().AsANSIBold(), issue);

      // Print the affected code.
      PrintTokenRange(1);
    }
  }

};