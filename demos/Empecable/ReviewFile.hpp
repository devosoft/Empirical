/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file demos/Empecable/ReviewFile.hpp
 * @brief Manage a single file being reviewed.
 */

#pragma once

#ifndef DEMOS_EMPECABLE_REVIEW_FILE_HPP_GUARD
#define DEMOS_EMPECABLE_REVIEW_FILE_HPP_GUARD

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

  static constexpr emplex::Token empty_token{0,"",0};  // Token for out-of-range positions.

public:
  static constexpr size_t npos = static_cast<size_t>(-1); // ID for "beyond the end of the file"

  ReviewFile(emp::String filename, Mode mode)
    : filename(filename), path(fs::absolute(filename.str())), mode(mode)
  {
    if (fs::exists(path) && fs::is_regular_file(path)) {
      path = fs::canonical(filename.str());
      valid = true;
    } else {
      PrintError("file does not exist or is not a regular file: ", filename);
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
    emp::PrintLn("Added '", word.AsANSICyan(), "' to file dictionary.");
    save_required = true;
  }

  // ======= Token Management =======

  [[nodiscard]] size_t GetTokenPos() const { return token_pos; }
  [[nodiscard]] bool AtFront() const { return token_pos == 0; }

  [[nodiscard]] const emplex::Token & GetToken(size_t pos) const {
    if (token_pos >= tokens.size()) return empty_token; // Out of range.
    return tokens[pos];
  }
  [[nodiscard]] emp::String GetLexeme(size_t pos) const { return GetToken(pos).lexeme; }
  [[nodiscard]] size_t GetLineID(size_t pos) const { return GetToken(pos).line_id; }
  [[nodiscard]] int GetTokenID(size_t pos) const { return GetToken(pos).id; }

  [[nodiscard]] const emplex::Token & GetToken() const { return GetToken(token_pos); }
  [[nodiscard]] emp::String GetLexeme() const { return GetLexeme(token_pos); }
  [[nodiscard]] size_t GetLineID() const { return GetLineID(token_pos); }
  [[nodiscard]] int GetTokenID() const { return GetTokenID(token_pos); }

  [[nodiscard]] size_t NumTokens() const { return tokens.size(); }

  void SetTokenPos(size_t pos) {
    emp_assert(pos <= tokens.size(), pos, tokens.size());
    token_pos = pos;
  }

  /// Find the position of the next token of the specified type (or npos if not found.)
  [[nodiscard]] size_t FindTokenPos(int type_id, size_t start_pos=0) const {
    for (size_t pos = start_pos; pos < tokens.size(); ++pos) {
      if (tokens[pos] == type_id) return pos;
    }
    return npos;
  }

  const emplex::Token & ResetTokens() { token_pos = 0; return GetToken(); }
  const emplex::Token & NextToken() { return GetToken(++token_pos); }
  const emplex::Token & PrevToken() { return GetToken(--token_pos); }

  void SkipNewLines() {
    while (token_pos < tokens.size() && GetToken() == emplex::Lexer::ID_END_LINE) ++token_pos;
  }

  void InsertToken(size_t pos, emplex::Token token) {
    emp_assert(pos <= tokens.size());
    tokens.insert(tokens.begin()+pos, token);
    if (token_pos >= pos) ++token_pos;  // Make sure active token keeps pointing to same place.
    save_required = true;
  }

  void InsertLexeme(size_t pos, const emp::String & lexeme) {
    emp_assert(pos <= tokens.size());

    // Determine line number for inserted token.
    size_t line_id;
    if (pos == 0) line_id = 0;
    else if (pos < tokens.size()) line_id = tokens[pos].line_id;
    else line_id = tokens[pos-1].line_id+1;

    InsertToken(pos, emplex::Token{emplex::Lexer::ID_OTHER, lexeme, line_id});
  }

  void InsertLexeme(const emp::String & lexeme) {
    InsertLexeme(token_pos, lexeme);
  }

  void InsertBack(const emp::String & lexeme) {
    InsertLexeme(tokens.size(), lexeme);
  }

  void RemoveToken(size_t pos, size_t count=1) {
    emp_assert(pos+count <= tokens.size());
    tokens.erase(tokens.begin()+pos, tokens.begin()+pos+count);
    if (token_pos >= pos+count) token_pos -= count;  // Shift back by deleted count.
    else if (token_pos >= pos) token_pos = pos;      // Move to beginning of deleted portion.
    save_required = true;
  }

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
  [[nodiscard]] size_t FindPos_Line(size_t target_line) const {
    emp_assert(target_line >= tokens[0].line_id &&
               target_line <= tokens.back().line_id,
              target_line, tokens[0].line_id, tokens.back().line_id);
    size_t min=0, max=tokens.size();
    while (tokens[min].line_id != target_line) {
      size_t mid = (min+max)/2;
      if (tokens[mid].line_id > target_line) max = mid;
      else min = mid;
    }
    return min;
  }

  // Find the first token of the target line.
  [[nodiscard]] size_t FindPos_LineStart(size_t target_line) const {
    size_t pos = FindPos_Line(target_line);
    while (pos > 0 && tokens[pos-1].line_id == target_line) --pos;
    return pos;
  }

  // Find the first token of the target line.
  [[nodiscard]] size_t FindPos_LineEnd(size_t target_line) const {
    size_t pos = FindPos_Line(target_line);
    while (pos+1 < tokens.size() && tokens[pos+1].line_id == target_line) ++pos;
    return pos;
  }

  // Get the entire line that a particular token is on, highlighting that token.
  [[nodiscard]] emp::String GetTokenLine(size_t line_id) const {
    // Determine the set of tokens to print.
    const size_t start = FindPos_LineStart(line_id);
    const size_t end = FindPos_LineEnd(line_id);

    // Collect the tokens on this line, highlighting the current token.
    emp::String result;
    emp::String target_word = GetToken().lexeme;
    for (size_t i = start; i < end; ++i) {
      if (i == token_pos) {
        result += ToBoldRed(tokens[i].lexeme).AsANSIUnderline();
      }
      else if (tokens[i].lexeme == target_word) {
        result += target_word.AsANSIBlue().AsANSIUnderline();
      }
      else { result += tokens[i].lexeme; }
    }
    return result;
  }

  // Return a string representing the combined lexemes for the position provided.
  // (exclusive of end_pos)
  [[nodiscard]] emp::String GetPosRange(size_t start_pos, size_t end_pos) const {
    // Collect the tokens on this line, highlighting the current token.
    emp::String result;
    for (size_t i = start_pos; i < end_pos; ++i) {
      result += tokens[i].lexeme;
    }
    return result;
  }

  [[nodiscard]] emp::String GetRangeToPos(size_t end_pos) const {
    return GetPosRange(token_pos, end_pos);
  }

  // Return a string representing the specific line numbers provided, inclusive.
  [[nodiscard]] emp::String GetLines(size_t start_line, size_t end_line) const {
    return GetPosRange(FindPos_LineStart(start_line), FindPos_LineEnd(end_line));
  }

  [[nodiscard]] emp::String GetLine(size_t start_line) const {
    return GetLines(start_line, start_line);
  }

  // Get the remainder of the line from a particular token.
  [[nodiscard]] emp::String GetLineFrom(size_t pos) const {
    emp::String out;
    while (pos < tokens.size() && tokens[pos] != emplex::Lexer::ID_END_LINE) {
      out += tokens[pos].lexeme;
      ++pos;
    }
    return out;
  }

  // Print a line near a given token, highlighting that token.
  void PrintLine(size_t line_id) const {
    emp::String line_num_str = emp::MakeString(line_id).PadFront(' ', 5)+':';
    emp::String code_line = GetTokenLine(line_id);
    emp::PrintLn(line_num_str.AsANSIBrightWhite().AsANSIBold(), code_line);
  }

  // Print a range of lines the current token.
  void PrintTokenRange(size_t range=1) const {
    const size_t line_id = GetLineID();
    size_t min_line = (line_id > range) ? (line_id - range) : 0;
    min_line = std::max(min_line, tokens[0].line_id); // Cannot start before beginning of file.
    size_t max_line = (line_id+range <= MaxLineID()) ? line_id+range : MaxLineID();
    if (mode == Mode::DEBUG) {
      emp::PrintLn("Printing ", range, "lines around ", line_id,
                   "(from ", min_line, " to ", max_line, ").");
    }
    for (size_t i = min_line; i <= max_line; ++i) {
      PrintLine(i);
    }
  }

  // Print a range of lines the current token.
  void PrintFront(size_t lines=1) const {
    for (size_t i = 1; i <= lines; ++i) {
      PrintLine(i);
    }
  }

  // Try to open this file, read tokens, and load settings.
  // Return whether additional analysis should continue.
  bool Load(emplex::Lexer & lexer, const std::unordered_set<emp::String> & project_words) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      PrintError("'", filename, "' failed to open.");
      valid = false;
      return false;
    }
    tokens = lexer.Tokenize(file);
    token_pos = 0;       // Reset the next token to be analyzed.
    issues.resize(0);    // Reset issues for new file.

    if (NumTokens() == 0) {
      PrintWarning("'", filename, "' is empty.");
      valid = true;
      return false;
    }

    // Scan through file for Empecable instructions.
    for (auto & token : tokens) {
      switch (token) {
      case emplex::Lexer::ID_EMP_META_START:
        token.lexeme = ""; // Clear out this lexeme; we will reconstruct it when saving.
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
      if (mode >= Mode::Interactive) {
        emp::PrintLn("No changes need to be saved in '", emp::ANSI::MakeGreen(filename), "'.");
      }
      return;
    }
    if (!valid) {
      PrintError("Trying to save an invalid file: ", filename);
      exit(1);
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

  // Clean up space for current file.
  void Close() {
    tokens.resize(0);
    valid = false;
  }

  // ========== Issue Tracking ==========

  [[nodiscard]] size_t GetNumIssues() const { return issues.size(); }

  // Print tokens to the screen with a particular token highlighted in read.
  template <typename... ARG_Ts>
  void ReportIssue(ARG_Ts &&... args) {
    emp::String issue = emp::MakeString(std::forward<ARG_Ts>(args)...);
    issues.push_back(issue);

    // Report the issue.
    if (mode >= Mode::Verbose) {
      size_t line_num = GetLineID(token_pos);
      emp::PrintLn(ToBoldYellow("Found in ", filename, ":", line_num, ": "), issue);

      // Print the affected code.
      PrintTokenRange(1);
    }
  }

};

#endif // #ifndefDEMOS_EMPECABLE_REVIEW_FILE_HPP_GUARD

