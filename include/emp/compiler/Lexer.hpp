/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2016-2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/compiler/Lexer.hpp
 * @brief A general-purpose, fast lexer.
 * @note Status: BETA
 *
 * Build a lexer that can convert input strings or streams into a series of provided tokens.
 *
 * Use AddToken(name, regex) to provide the relevant tokens.
 *  'name' is the unique name for this token.
 *  'regex' is the regular expression that describes this token.
 * It will return a unique ID associated with this token that will be used in output Token
 * objects later.  You may provide additional arguments to indicate of the lexeme should be
 * saved in a token, whether the token should be saved at all (unsaved token types are
 * simply left out of the output), and a more detailed description.
 *
 * IgnoreToken(name, regex) uses the same arguments, but is used for tokens that
 * should be skipped over during lexical analysis
 *
 * Names and IDs can be recovered later using GetTokenID(name) and GetTokenName(id).
 *
 * Either strings or streams can be converted into their associated Tokens.
 *   Tokenize(in) returns the entire series of tokens as a TokenStream.
 *   TokenizeNext(in, &line) return the next token only, and updates the line number
 *   Process(in) returns the next token (even if it's normally ignored); does not track line
 *
 * Finally, GetLexeme() returns the lexeme from the most recent token found.
 */

#pragma once

#ifndef INCLUDE_EMP_COMPILER_LEXER_HPP_GUARD
#define INCLUDE_EMP_COMPILER_LEXER_HPP_GUARD

#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <stddef.h>

#include "../base/vector.hpp"
#include "../datastructs/map_utils.hpp"
#include "../tools/String.hpp"

#include "_Token.hpp"
#include "_TokenStream.hpp"
#include "_TokenType.hpp"
#include "lexer_utils.hpp"

namespace emp {

  /// A lexer with a set of token types (and associated regular expressions)
  /// MAX_ID determines the maximum number of allowed lexical rules.
  template <int MAX_ID>
  class Lexer_Base {
  private:
    emp::vector<TokenType> token_set{};   ///< List of all active tokens types.
    std::map<String, int> token_map{};    ///< Map of token names to id.
    int cur_token_id = MAX_ID;            ///< ID for next new token (higher has priority)
    mutable bool generate_lexer = false;  ///< Do we need to regenerate the lexer?
    mutable DFA lexer_dfa{};              ///< Table driven lexer implementation.
    mutable String lexeme{};              ///< Current state of lexeme being generated.

    static constexpr int ERROR_ID = -1;  ///< Code for unknown token ID.

    static const TokenType & ERROR_TOKEN() {
      static const TokenType token{"ERROR", "", ERROR_ID};
      return token;
    }

    /// Get the next token found in an input string.  Do so by examining one character at a time.
    /// Keep going as long as there is a chance of a valid lexeme (since we need to choose the
    /// longest one available), saving best option so far.  Once no more other valid lexemes are
    /// possible, stop and return the best we've found so far.
    Token Process(std::string_view in_str, size_t & start_pos) const;

    /// Shortcut to process a stream rather than a string.
    Token Process(std::istream & is) const;

  public:
    // All constructors and assignment operators are default.

    /// How many types of tokens can be identified in this Lexer?
    size_t GetNumTokens() const { return token_set.size(); }

    /// Reset the entire lexer to its start state.
    void Reset();

    /// Add a new token, specified by a name and the regex used to identify it.
    /// Note that token ids count down with highest IDs having priority.
    int AddToken(String name,
                 String regex,
                 bool save_lexeme = true,
                 bool save_token  = true,
                 String desc      = "");

    /// Add a token to ignore, specified by a name and the regex used to identify it.
    int IgnoreToken(String name, String regex, String desc = "");

    /// Get the ID associated with a token type (you provide the token name)
    int GetTokenID(String name) const;

    /// Get the full information about a token (you provide the id)
    const TokenType & GetTokenType(int id) const;

    /// Get the name associated with a token type (you provide the ID)
    String GetTokenName(int id) const;

    /// Identify if a token should be saved.
    bool GetSaveToken(int id) const;

    /// Create the NFA that will identify the current set of tokens in a sequence.
    void Generate() const;

    /// Shortcut to just get a single token.
    Token ToToken(std::string_view in_str) const;

    /// Perform a single step in the Tokenize process, updating line number and start position as we go.
    Token TokenizeNext(std::string_view in_str,
                       size_t & cur_line,
                       size_t & start_pos,
                       bool keep_all = false) const;

    /// Perform a single step in the Tokenize process, updating line number as we go.
    Token TokenizeNext(std::istream & in_stream, size_t & cur_line, bool keep_all = false) const;

    /// Perform a single step in the Tokenize process.
    Token TokenizeNext(std::istream & in_stream) const {
      size_t cur_line = 0;
      return TokenizeNext(in_stream, cur_line, false);
    }

    /// Turn an input string into a vector of tokens.
    TokenStream Tokenize(std::string_view str,
                         String name   = "in_view",
                         bool keep_all = false) const;

    /// Turn an input stream of text into a vector of tokens.
    TokenStream Tokenize(std::istream & is, String name = "in_stream", bool keep_all = false) const;

    /// Turn a vector of strings into a vector of tokens.
    TokenStream Tokenize(const emp::vector<String> & str_v,
                         String name   = "in_string vector",
                         bool keep_all = false) const;

    /// Load from a file and tokenize it.
    TokenStream TokenizeFile(String filename) const {
      std::ifstream file(filename);
      return Tokenize(file, filename);
    }

    /// Get the lexeme associated with the last token identified.
    const String & GetLexeme() const { return lexeme; }

    /// Print the full information about this lexer (for debugging)
    void Print(std::ostream & os = std::cout) const;

    /// Try out the lexer on a string and demonstrate how it's tokenized.
    void DebugString(String test_string) const;

    void DebugToken(int token_id) const;

    /// Generate a C++ lexer file.
    /// @param file The C++ file to write to.
    /// @param object_name Class name to use for the lexer object itself. (Default: "Lexer")
    /// @param DFA_name Class name to use for the DFA powering the Lexer. (Default: "DFA")
    /// @param token_name Class name to use for the individual tokens. (Default: "Token")
    /// @param save_lexeme Should lexemes be saved in tokens by default? (Default: true)
    /// @param save_line_id Should we save the line number for each token? (Default: true)
    /// @param save_col_id Should we save the column the token was found on? (Default: true)
    /// @param use_emp Should we use Empirical classes? (Default: false) (Default: true)
    void WriteCPP(emp::CPPFile & file,
                  emp::String object_name = "Lexer",
                  emp::String DFA_name    = "DFA",
                  emp::String token_name  = "Token",
                  bool save_lexeme        = true,
                  bool save_line_id       = true,
                  bool save_col_id        = true,
                  bool use_emp            = false) const;
  };

  using Lexer = Lexer_Base<255>;

  /// --------------------
  /// FUNCTION DEFINITIONS
  /// --------------------

  template <int MAX_ID>
  void Lexer_Base<MAX_ID>::Reset() {
    token_set.resize(0);
    token_map.clear();
    cur_token_id   = MAX_ID;
    generate_lexer = false;
  }

  template <int MAX_ID>
  int Lexer_Base<MAX_ID>::AddToken(String name,
                                   String regex,
                                   bool save_lexeme,
                                   bool save_token,
                                   String desc) {
    int id = cur_token_id--;  // Grab the next available token id.
    emp_assert(id > 0, "Too many lexer rules added! Increase MAX_ID", name, regex, MAX_ID);
    generate_lexer = true;  // Indicate the the lexer DFA needs to be rebuilt.
    token_set.emplace_back(name, regex, id, save_lexeme, save_token, desc);
    token_map[name] = id;
    return id;
  }

  template <int MAX_ID>
  int Lexer_Base<MAX_ID>::IgnoreToken(String name, String regex, String desc) {
    int id         = cur_token_id--;  // Grab the next available token id.
    generate_lexer = true;            // Indicate the the lexer DFA needs to be rebuilt.
    token_set.emplace_back(name, regex, id, false, false, desc);
    token_map[name] = id;
    return id;
  }

  template <int MAX_ID>
  int Lexer_Base<MAX_ID>::GetTokenID(String name) const {
    int default_id = ERROR_ID;
    if (name.size() == 1) { default_id = (int) name[0]; }
    return emp::Find(token_map, name, default_id);
  }

  template <int MAX_ID>
  const TokenType & Lexer_Base<MAX_ID>::GetTokenType(int id) const {
    // emp::notify::Message("id=", id, "; MAX_ID=", MAX_ID, "; cur_token_id=", cur_token_id);
    if (id > MAX_ID || id <= cur_token_id) { return ERROR_TOKEN(); }
    const size_t token_pos = static_cast<size_t>(MAX_ID - id);
    // emp::notify::Message("token_pos=", token_pos);
    return token_set[token_pos];
  }

  template <int MAX_ID>
  String Lexer_Base<MAX_ID>::GetTokenName(int id) const {
    if (id < 0) {
      return MakeString("ERROR");  // Negative tokens is error.
    }
    if (id == 0) {
      return "EOF";  // Token 0 is end-of-file.
    }
    if (id < 128) {
      return MakeString("'", MakeEscaped((char) id), "'");  // Individual character.
    }
    if (id <= cur_token_id) {
      return MakeString("Error (", id, ")");  // Invalid token range!
    }
    return GetTokenType(id).name;  // User-defined tokens.
  }

  template <int MAX_ID>
  bool Lexer_Base<MAX_ID>::GetSaveToken(int id) const {
    if (id > MAX_ID || id <= cur_token_id) { return true; }
    return GetTokenType(id).save_token;
  }

  template <int MAX_ID>
  void Lexer_Base<MAX_ID>::Generate() const {
    NFA lexer_nfa;
    for (const auto & t : token_set) { lexer_nfa.Merge(to_NFA(t.regex, (uint8_t) t.id)); }
    const size_t start_id = lexer_nfa.GetStartID();
    lexer_nfa.AddTransitionSymbol(start_id, start_id, DFA::SYMBOL_START);
    generate_lexer = false;  // We just generated it!  Don't again until another change is made.
    lexer_dfa      = to_DFA(lexer_nfa);
  }

  template <int MAX_ID>
  Token Lexer_Base<MAX_ID>::Process(std::string_view in, size_t & start_pos) const {
    emp_assert(start_pos <= in.size());

    // If we do not have any tokens, just give an error.
    if (token_set.size() == 0) { return {-1, "No Token Types available."}; }

    // If we cannot read in, return an "EOF" token.
    if (start_pos >= in.size()) { return {0, ""}; }

    // If we still need to generate the DFA for the lexer, do so.
    if (generate_lexer) { Generate(); }

    size_t cur_pos  = start_pos;  // Position in the input that we are actively analyzing
    size_t best_pos = start_pos;  // Best look-ahead we've found so far
    int cur_state   = 0;          // Next state for the DFA analysis
    int cur_stop    = 0;          // Current "stop" state (if we can stop here)
    int best_stop   = -1;         // Best stop state found so far?

    // If we are at the START OF A LINE, send a DFA::SYMBOL_START
    if (start_pos == 0 || in[start_pos - 1] == '\n') {
      cur_state = lexer_dfa.Next(0, DFA::SYMBOL_START);
    }

    // Keep looking as long as:
    // 1: We may be able to continue the current lexeme, and
    // 2: We have not entered an invalid state, and
    // 3: Our input stream has more symbols.
    while (cur_stop >= 0 && cur_state >= 0 && cur_pos < in.size()) {
      const char next_char = in[cur_pos++];
      cur_state            = lexer_dfa.Next(cur_state, static_cast<size_t>(next_char));
      cur_stop             = lexer_dfa.GetStop(cur_state);
      if (cur_stop > 0) {
        best_pos  = cur_pos;
        best_stop = cur_stop;
      }
      // Look ahead to see if we are at the END OF A LINE that can finish a token.
      if (cur_pos == in.size() || in[cur_pos] == '\n') {
        int eol_state = lexer_dfa.Next(cur_state, static_cast<size_t>(DFA::SYMBOL_STOP));
        int eol_stop  = lexer_dfa.GetStop(eol_state);
        if (eol_stop > 0) {
          best_pos  = cur_pos;
          best_stop = eol_stop;
        }
      }
    }

    // If we did not find any options, advance best_pos to return a single error char.
    if (best_pos == start_pos) {
      best_stop = in[start_pos];
      ++best_pos;
    }

    // If best_pos < cur_pos, rewind the input stream.
    if (best_pos < cur_pos) { cur_pos = best_pos; }

    lexeme = in.substr(start_pos, best_pos - start_pos);
    start_pos += lexeme.size();

    // If we can't find a token, return error (for no valid options)
    if (best_stop < 0) {
      return {ERROR_ID, lexeme};  // ERROR_ID = no valid tokens available!
    }

    // Otherwise return the best token we've found so far.
    return {best_stop, lexeme};
  }

  static bool AtEOF(std::istream & is) { return is.peek() == std::istream::traits_type::eof(); }

  static bool AtLineStart(std::istream & is) {
    if (!is.good()) {
      return false;  // If the stream is bad, not at start of line
    }
    if (is.tellg() <= 0) {
      return true;  // At beginning of stream must be line start
    }
    is.seekg(-1, std::ios::cur);                   // Move back one position
    return (static_cast<char>(is.get()) == '\n');  // Return whether prev char was a newline.
  }

  template <int MAX_ID>
  Token Lexer_Base<MAX_ID>::Process(std::istream & is) const {
    // If we do not have any tokens, just give an error.
    if (token_set.size() == 0) { return {-1, "No Token Types available."}; }

    // If we cannot read in, return an "EOF" token.
    if (AtEOF(is)) { return {0, ""}; }

    // If we still need to generate the DFA for the lexer, do so.
    if (generate_lexer) { Generate(); }

    size_t cur_pos  = 0;   // Position in the input that we are actively analyzing
    size_t best_pos = 0;   // Best look-ahead we've found so far
    int cur_state   = 0;   // Next state for the DFA analysis
    int cur_stop    = 0;   // Current "stop" state (if we can stop here)
    int best_stop   = -1;  // Best stop state found so far?

    // If we are at the START OF A LINE, send a DFA::SYMBOL_START
    if (AtLineStart(is)) { cur_state = lexer_dfa.Next(0, DFA::SYMBOL_START); }

    // Keep looking as long as:
    // 1: We may be able to continue the current lexeme, and
    // 2: We have not entered an invalid state, and
    // 3: Our input stream has more symbols.
    lexeme.resize(0);
    while (cur_stop >= 0 && cur_state >= 0 && !AtEOF(is)) {
      const char next_char = static_cast<char>(is.get());
      lexeme.push_back(next_char);
      ++cur_pos;
      cur_state = lexer_dfa.Next(cur_state, static_cast<size_t>(next_char));
      cur_stop  = lexer_dfa.GetStop(cur_state);
      if (cur_stop > 0) {
        best_pos  = cur_pos;
        best_stop = cur_stop;
      }
      // Look ahead to see if we are at the END OF A LINE that can finish a token.
      if (AtEOF(is) || is.peek() == '\n') {
        int eol_state = lexer_dfa.Next(cur_state, static_cast<size_t>(DFA::SYMBOL_STOP));
        int eol_stop  = lexer_dfa.GetStop(eol_state);
        if (eol_stop > 0) {
          best_pos  = cur_pos;
          best_stop = eol_stop;
        }
      }
    }

    // If we did not find any options, advance best_pos to return a single char.
    if (best_pos == 0) {
      is.seekg(-static_cast<int>(cur_pos), std::ios::cur);
      best_stop = is.get();
      best_pos  = 1;
    }

    // Otherwise, if cur_pos went past best_pos, rewind the input stream.
    else if (cur_pos > best_pos) {
      is.seekg(best_pos - cur_pos, std::ios::cur);  // Note: negative; Seeking backwards
    }
    lexeme.Resize(best_pos);

    // If we can't find a token, return error (for no valid options)
    if (best_stop < 0) {
      return {ERROR_ID, lexeme};  // ERROR_ID = no valid tokens available!
    }

    // Otherwise return the best token we've found so far.
    return {best_stop, lexeme};
  }

  template <int MAX_ID>
  Token Lexer_Base<MAX_ID>::ToToken(std::string_view in_str) const {
    size_t start_pos = 0;
    return Process(in_str, start_pos);
  }

  template <int MAX_ID>
  Token Lexer_Base<MAX_ID>::TokenizeNext(std::string_view str,
                                         size_t & cur_line,
                                         size_t & start_pos,
                                         bool keep_all) const {
    // Loop until we get a token to return or hit the end of the stream.
    while (true) {
      Token token   = Process(str, start_pos);
      token.line_id = cur_line;
      cur_line += token.lexeme.Count('\n');
      if (keep_all || GetSaveToken(token)) {
        return token;  // Skip ignored tokens and try again.
      }
    }
  }

  template <int MAX_ID>
  Token Lexer_Base<MAX_ID>::TokenizeNext(std::istream & is,
                                         size_t & cur_line,
                                         bool keep_all) const {
    // Loop until we get a token to return or hit the end of the stream.
    while (true) {
      Token token   = Process(is);
      token.line_id = cur_line;
      cur_line += token.lexeme.Count('\n');
      if (keep_all || GetSaveToken(token)) {
        return token;  // Skip ignored tokens and try again.
      }
    }
  }

  template <int MAX_ID>
  TokenStream Lexer_Base<MAX_ID>::Tokenize(std::string_view str, String name, bool keep_all) const {
    emp::vector<Token> out_tokens;
    size_t start_pos = 0;
    size_t cur_line  = 1;
    while (Token token = TokenizeNext(str, cur_line, start_pos, keep_all)) {
      out_tokens.push_back(token);
    }
    return TokenStream{out_tokens, name};
  }

  template <int MAX_ID>
  TokenStream Lexer_Base<MAX_ID>::Tokenize(std::istream & is, String name, bool keep_all) const {
    emp::vector<Token> out_tokens;
    size_t cur_line = 1;
    while (Token token = TokenizeNext(is, cur_line, keep_all)) { out_tokens.push_back(token); }
    return TokenStream{out_tokens, name};
  }

  template <int MAX_ID>
  TokenStream Lexer_Base<MAX_ID>::Tokenize(const emp::vector<String> & sv,
                                           String name,
                                           bool keep_all) const {
    return Tokenize(emp::Join(sv, "\n"), name, keep_all);
  }

  template <int MAX_ID>
  void Lexer_Base<MAX_ID>::Print(std::ostream & os) const {
    for (const auto & t : token_set) { t.Print(os); }
    if (generate_lexer) {
      Generate();  // Do we need to regenerate the lexer?
    }
    lexer_dfa.Print(os);  // Table driven lexer implementation.
  }

  template <int MAX_ID>
  void Lexer_Base<MAX_ID>::DebugString(String test_string) const {
    std::stringstream ss;
    ss << test_string;

    Token token = 1;
    while (token > 0) {
      token = Process(ss);
      std::cout << GetTokenName(token) << " : \"" << token.lexeme << "\"" << std::endl;
    }
  }

  template <int MAX_ID>
  void Lexer_Base<MAX_ID>::DebugToken(int token_id) const {
    const auto & token = GetTokenType(token_id);
    std::cout << "Debugging token " << token.id << ": '" << token.name << "'.\n"
              << "  Desc: " << token.desc << "\n";
    token.regex.PrintDebug();
  }

  template <int MAX_ID>
  void Lexer_Base<MAX_ID>::WriteCPP(
    emp::CPPFile & file,
    emp::String object_name,
    emp::String DFA_name,
    emp::String token_name,
    bool save_lexeme,
    bool save_line_id,
    bool save_col_id,
    bool use_emp
  ) const {
    // If we still need to generate the DFA for the lexer, do so.
    if (generate_lexer) { Generate(); }

    file.AddVarSetting("object_name", object_name);
    file.AddVarSetting("token_set_size", token_set.size());
    file.AddVarSetting("token_name", token_name);
    file.AddVarSetting("DFA_name", DFA_name);
    if (use_emp) {
      file.AddVarSetting("str_type", "emp::String");
      file.AddVarSetting("vec_type", "emp::vector");
    } else {
      file.AddVarSetting("str_type", "std::string");
      file.AddVarSetting("vec_type", "std::vector");
    }

    file.Include("<algorithm>");
    file.Include("<array>");
    file.Include("<cctype>");
    file.Include("<iostream>");
    if (!use_emp) file.Include("<string>");
    file.Include("<unordered_map>");
    if (!use_emp) file.Include("<vector>");

    if (use_emp) {
      file.Include("emp/base/vector.hpp");
      file.Include("emp/tools/String.hpp");
    }

    file.AddCode("// Struct to store information about a found Token")
      .AddCode("struct ", token_name, " {")
      .AddCode("  int id;                             // Type ID for token");
    if (save_lexeme) {
      file.AddCode("  ${str_type} lexeme;                 // Sequence matched by token");
    }
    if (save_line_id) {
      file.AddCode("  size_t line_id;                     // Line token started on");
    }
    if (save_col_id) {
      file.AddCode("  size_t col_id;                      // Column token started on");
    }
    file.AddCode("  operator int() const { return id; } // Auto-convert tokens to IDs")
      .AddCode("};")
      .AddCode("");

    file.AddCode("// Deterministic Finite Automaton (DFA) for token recognition.");
    lexer_dfa.WriteCPP(file, DFA_name);
    file.AddCode("");

    file.AddCodeBlock(
      "class ${object_name} {",
      "private:",
      "  static constexpr int NUM_TOKENS=${token_set_size};",
      "",
      "  // -- Load State --",
      "  size_t cur_line = 1;   // Track LINE we are reading in the input.",
      "  size_t cur_col = 0;    // Track COLUMN we are reading in the input.",
      "  int start_pos = 0;     // Track INDEX for the start of current lexeme.",
      "  ${str_type} lexeme{};  // Lexeme found for the current token",
      "",
      "  // -- Process State --",
      "  ${vec_type}<${token_name}> tokens{}; // Set of tokens loaded so far.",
      "  size_t token_id = 0;                 // Next token to process.",
      "  ${token_name} eof_token{0");
    if (save_lexeme) { file.AppendCode(", \"_EOF_\""); }
    if (save_line_id) { file.AppendCode(", 0"); }
    if (save_col_id) { file.AppendCode(", 0"); }
    file.AppendCode("};").AddCode("").AddCode("public:").AddCode(
      "  static constexpr int ID__EOF_ = 0;");
    for (size_t i = token_set.size(); i > 0; --i) {
      const auto & token = token_set[i - 1];
      file.AddCode("  static constexpr int ID_", token.name, " = ", token.id, "; ")
        .AppendPadding(50)
        .AppendCode("// Regex: ", token.regex.ToString());
    }
    file.AddCodeBlock("",
                      "  // Return the name of a token given its ID.",
                      "  static ${str_type} TokenName(int id) {",
                      "    switch (id) {",
                      "    case -1: return \"_ERROR_\";",
                      "    case 0: return \"_EOF_\";");
    for (size_t i = token_set.size(); i > 0; --i) {
      const auto & token = token_set[i - 1];
      file.AddCode("    case ID_", token.name, ": return ", token.name.AsLiteral(), ";");
    }
    file.AddCodeBlock("    default:",
                      "      // If ID is a visible character print it, otherwise provide ID.",
                      "      if (id > 0 && id < 128 && std::isprint(id)) {",
                      "        return ${str_type}(\"'\") + static_cast<char>(id) + \"'\";",
                      "      }",
                      "      if (id == '\\n') return \"'\\\\n'\";",
                      "      if (id == '\\r') return \"'\\\\r'\";",
                      "      if (id == '\\t') return \"'\\\\t'\";",
                      "      if (id == '\\\\') return \"'\\\\n'\";",
                      "      return ${str_type}(\"Token ID: \") + std::to_string(id);",
                      "    }; // End switch.",
                      "  }",
                      "",
                      "  // Identify if a token should be skipped during tokenizing.",
                      "  static constexpr bool IgnoreToken(int id) {",
                      "    switch (id) {",
                      "    case 0:");
    for (size_t i = token_set.size(); i > 0; --i) {
      const auto & token = token_set[i - 1];
      if (token.save_lexeme == false) { file.AddCode("    case ", token.id, ":"); }
    }
    file.AddCodeBlock("      return true;",
                      "    default: return false;",
                      "    };",
                      "  }",
                      "",
                      "  // Return the number of token types the lexer recognizes.",
                      "  static constexpr int GetNumTokens() { return NUM_TOKENS; }",
                      "",
                      "  // Generate and return the next token from the input stream.",
                      "  ${token_name} NextToken(std::string_view in) {",
                      "    // If we cannot read in, return an \"EOF\" token.",
                      "    if (start_pos >= std::ssize(in)) return { 0");
    if (save_lexeme) { file.AppendCode(", \"\""); }
    if (save_line_id) { file.AppendCode(", cur_line"); }
    if (save_col_id) { file.AppendCode(", cur_col"); }
    file.AppendCode(" };");
    file.AddCodeBlock(
      "",
      "    int cur_pos = start_pos;  // Position in the input that we are actively analyzing",
      "    int best_pos = start_pos; // Best look-ahead we've found so far",
      "    int cur_state = 0;        // Next state for the DFA analysis",
      "    int cur_stop = 0;         // Current \"stop\" state (or 0 if we can't stop here)",
      "    int best_stop = -1;       // Best stop state found so far (longest match w/ max stop value)");
    file.AddCodeBlock(
      "",
      "    // If we are at the START OF A LINE, send a ${DFA_name}::SYMBOL_START",
      "    if (start_pos == 0 || in[start_pos-1] == '\\n') {",
      "      cur_state = ${DFA_name}::GetNext(0, ${DFA_name}::SYMBOL_START);",
      "    }",
      "    // Keep looking as long as:",
      "    // 1: We may be able to continue the current lexeme, and",
      "    // 2: We have not entered an invalid state, and",
      "    // 3: Our input string has more symbols to provide",
      "    while (cur_stop >= 0 && cur_state >= 0 && cur_pos < std::ssize(in)) {",
      "      const char next_char = in[cur_pos++];",
      "      if (next_char < 0) break; // Ignore invalid chars.",
      "      cur_state = ${DFA_name}::GetNext(cur_state, next_char);",
      "      cur_stop = ${DFA_name}::GetStop(cur_state);",
      "      // If we found a valid stopping point, save it as a new best.",
      "      if (cur_stop > 0) { best_pos = cur_pos; best_stop = cur_stop; }",
      "      // Look ahead to see if we are at the END OF A LINE that can finish a token.",
      "      if (cur_pos == std::ssize(in) || in[cur_pos] == '\\n') {",
      "        int eol_state = ${DFA_name}::GetNext(cur_state, ${DFA_name}::SYMBOL_STOP);",
      "        int eol_stop = ${DFA_name}::GetStop(eol_state);",
      "        if (eol_stop > 0) { best_pos = cur_pos; best_stop = eol_stop; }",
      "      }",
      "    }",
      "",
      "    // If we did not find any options, peel off just one character and use it as id.",
      "    if (best_pos == start_pos) { best_stop=in[start_pos]; ++best_pos;}",
      "",
      "    lexeme = in.substr(start_pos, best_pos-start_pos);",
      "    start_pos += std::ssize(lexeme);",
      "",
      "    // Update the line number we are on.",
      "    const size_t out_line = cur_line;",
      "    const size_t out_col = cur_col;",
      "    if ((cur_col = lexeme.rfind('\\n')) == ${str_type}::npos) { // No newlines",
      "      cur_col = out_col + lexeme.size();",
      "    } else {",
      "      cur_col = lexeme.size() - cur_col - 1;",
      "      cur_line += static_cast<size_t>(std::count(lexeme.begin(),lexeme.end(),'\\n'));",
      "    }",
      "",
      "    // Return the token we found.",
      "    return { best_stop");
    if (save_lexeme) { file.AppendCode(", lexeme"); }
    if (save_line_id) { file.AppendCode(", out_line"); }
    if (save_col_id) { file.AppendCode(", out_col"); }
    file.AppendCode(" };");
    file.AddCodeBlock(
      "  }",
      "",
      "  // Convert an input string into a vector of tokens.",
      "  const ${vec_type}<${token_name}> & Tokenize(std::string_view in) {",
      "    start_pos = 0; // Start processing at beginning of string.",
      "    cur_line = 1;  // Start processing at the first line of the input.",
      "    cur_col = 0;   // Start processing at the first position of the input.",
      "    tokens.resize(0);",
      "    token_id = 0;",
      "    while (${token_name} token = NextToken(in)) {",
      "      if (!IgnoreToken(token.id)) tokens.push_back(token);",
      "    }",
      "    eof_token.line_id = cur_line;",
      "    return tokens;",
      "  }",
      "",
      "  // Convert an input stream to a string, then tokenize.",
      "  const ${vec_type}<${token_name}> & Tokenize(std::istream & is) {",
      "    return Tokenize(",
      "      ${str_type}(std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>())",
      "    );",
      "  }",
      "",
      "  // === Functions for Using Tokens ===",
      "",
      "  // Write an error message for invalid tokens and terminate the program.",
      "  template <typename... Ts>",
      "  void Error(Ts... message) {",
      "    std::cerr << \"ERROR");
    if (save_line_id) {
      file.AppendCode(" (line \" << Peek().line_id ");
      if (save_col_id) { file.AppendCode("<< \":\" << Peek().col_id "); }
      file.AppendCode("<<  \")");
    }
    file.AppendCode(": \";");

    file.AddCodeBlock(
      "    (std::cerr << ... << std::forward<Ts>(message)) << std::endl;",
      "    exit(1);",
      "  }",
      "",
      "  // Test if there are ANY tokens remaining.",
      "  bool Any() const { return token_id < tokens.size(); }",
      "",
      "  // Test if there are NO tokens remaining.",
      "  bool None() const { return token_id >= tokens.size(); }",
      "",
      "  // Test if the current token is a specific type.",
      "  bool Is(int id) const { return Any() && Peek() == id; }",
      "",
      "  // Get the current (or upcoming) token, but don't remove it from the queue.",
      "  const ${token_name} & Peek(size_t skip_count=0) const {",
      "    if (token_id + skip_count >= tokens.size()) return eof_token;",
      "    return tokens[token_id + skip_count];",
      "  }",
      "",
      "  // Get the current token, removing it from the queue.",
      "  const ${token_name} & Use() {",
      "    if (None()) return eof_token;",
      "    return tokens[token_id++];",
      "  }",
      "",
      "  // Use the current token if it is the expected type; otherwise error.",
      "  // (Use provided error if available, otherwise use default error)",
      "  template <typename... Ts>",
      "  const ${token_name} & Use(int id, Ts &&... message) {",
      "    if (!Is(id)) {",
      "      if constexpr (sizeof...(Ts) == 0) {",
      "        Error( \"Expected token of type \", TokenName(id),",
      "               \", but found type \", TokenName(Peek()));",
      "      }",
      "      else Error( std::forward<Ts>(message)...);",
      "    }",
      "    return Use();",
      "  }",
      "",
      "  // If the current token is one of the provided ids, use it an return the ID used.",
      "  // Otherwise, don't use it and return 0.",
      "  template <typename... Ts>",
      "  int UseIf(int id, Ts... args) {",
      "    if (Is(id)) { return Use(); }",
      "    return UseIf(args...);",
      "  }",
      "",
      "  // Base case for UseIf",
      "  int UseIf() { return 0; }",
      "",
      "  // Rewind one or more tokens.",
      "  void Rewind(size_t steps=1) {",
      "    if (token_id >= steps) token_id -= steps;",
      "    else token_id = 0;",
      "  }",
      "};");
  }


}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_COMPILER_LEXER_HPP_GUARD

// Local settings for Empecable file checker.
// empecable_words: tellg seekg
