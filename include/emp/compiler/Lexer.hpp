/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2016-2024.
*/
/**
 *  @file
 *  @brief A general-purpose, fast lexer.
 *  @note Status: BETA
 *
 *  Build a lexer that can convert input strings or streams into a series of provided tokens.
 *
 *  Use AddToken(name, regex) to list out the relevant tokens.
 *   'name' is the unique name for this token.
 *   'regex' is the regular expression that describes this token.
 *  It will return a unique ID associated with this token that will be used in output Token
 *  objects later.
 *
 *  IgnoreToken(name, regex) uses the same arguments, but is used for tokens that
 *  should be skipped over during lexical analysis
 *
 *  Names and IDs can be recovered later using GetTokenID(name) and GetTokenName(id).
 *
 *  Either strings or streams can be converted into their associated Tokens.
 *    Tokenize(in) returns the entire series of tokens as a TokenStream.
 *    TokenizeNext(in, &line) return the next token only, and updates the line number
 *    Process(in) returns the next token (even if it's normally ignored); does not track line
 *
 *  Finally, GetLexeme() returns the lexeme from the most recent token found.
 */

#ifndef EMP_COMPILER_LEXER_HPP_INCLUDE
#define EMP_COMPILER_LEXER_HPP_INCLUDE

#include <cstdint>
#include <iostream>
#include <map>
#include <stddef.h>
#include <string>

#include "../base/map.hpp"
#include "../base/vector.hpp"
#include "../datastructs/map_utils.hpp"
#include "../tools/String.hpp"

#include "_Token.hpp"
#include "_TokenStream.hpp"
#include "_TokenType.hpp"
#include "lexer_utils.hpp"
#include "RegEx.hpp"

namespace emp {

  /// A lexer with a set of token types (and associated regular expressions)
  /// MAX_ID determines the maximum number of allowed lexical rules.
  template <int MAX_ID>
  class Lexer_Base {
  private:
    static constexpr int ERROR_ID = -1;     ///< Code for unknown token ID.

    emp::vector<TokenType> token_set;       ///< List of all active tokens types.
    emp::map<String, int> token_map;        ///< Map of token names to id.
    int cur_token_id = MAX_ID;              ///< ID for next new token (higher has priority)
    mutable bool generate_lexer = false;    ///< Do we need to regenerate the lexer?
    mutable DFA lexer_dfa;                  ///< Table driven lexer implementation.
    mutable String lexeme;                  ///< Current state of lexeme being generated.

    static const TokenType & ERROR_TOKEN() {
      static const TokenType token;
      return token;
    }

  public:
    // All constructors and assignment operators are default.

    /// How many types of tokens can be identified in this Lexer?
    size_t GetNumTokens() const { return token_set.size(); }

    /// Reset the entire lexer to its start state.
    void Reset();

    /// Add a new token, specified by a name and the regex used to identify it.
    /// Note that token ids count down with highest IDs having priority.
    int AddToken(
      String name,
      String regex,
      bool save_lexeme = true,
      bool save_token = true,
      String desc = "");

    /// Add a token to ignore, specified by a name and the regex used to identify it.
    int IgnoreToken(
      String name,
      String regex,
      String desc="");

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

    /// Get the next token found in an input stream.  Do so by examining one character at a time.
    /// Keep going as long as there is a chance of a valid lexeme (since we want to choose the
    /// longest one we can find.)  Every time we do hit a valid lexeme, store it as the current
    /// "best" and keep going.  Once we hit a point where no other valid lexemes are possible,
    /// stop and return the best we've found so far.
    Token Process(std::istream & is) const;

    /// Shortcut to process a string rather than a stream, chopping off one token each time.
    Token Process(std::string & in_str) const;

    /// Shortcut to just get a single token.
    Token ToToken(std::string_view in_str) const;

    /// Perform a single step in the Tokenize process, tracking line number as we go.
    emp::Token TokenizeNext(std::istream & is, size_t & cur_line) const;

    /// Turn an input stream of text into a vector of tokens.
    TokenStream Tokenize(std::istream & is, String name="in_stream") const;

    /// Turn an input string into a vector of tokens.
    TokenStream Tokenize(std::string_view str, String name="in_view") const;

    /// Turn a vector of strings into a vector of tokens.
    TokenStream Tokenize(const emp::vector<String> & str_v,
                         String name="in_string vector") const;

    /// Get the lexeme associated with the last token identified.
    const String & GetLexeme() const { return lexeme; }

    /// Print the full information about this lexer (for debugging)
    void Print(std::ostream & os=std::cout) const;

    /// Try out the lexer on a string and demonstrate how it's tokenized.
    void DebugString(String test_string) const;

    void DebugToken(int token_id) const;

    void WriteCPP(emp::CPPFile & file, emp::String object_name="AutoLexer") const;
  };

  using Lexer = Lexer_Base<255>;

  /// --------------------
  /// FUNCTION DEFINITIONS
  /// --------------------

  template <int MAX_ID>
  void Lexer_Base<MAX_ID>::Reset() {
    token_set.resize(0);
    token_map.clear();
    cur_token_id = MAX_ID;
    generate_lexer = false;
  }

  template <int MAX_ID>
  int Lexer_Base<MAX_ID>::AddToken(String name,
                String regex,
                bool save_lexeme,
                bool save_token,
                String desc)
  {
    int id = cur_token_id--;                // Grab the next available token id.
    emp_assert(id > 0, "Too many lexer rules added! Increase MAX_ID", name, regex, MAX_ID);
    generate_lexer = true;                  // Indicate the the lexer DFA needs to be rebuilt.
    token_set.emplace_back( name, regex, id, save_lexeme, save_token, desc );
    token_map[name] = id;
    return id;
  }

  template <int MAX_ID>
  int Lexer_Base<MAX_ID>::IgnoreToken(String name, String regex, String desc) {
    int id = cur_token_id--;                // Grab the next available token id.
    generate_lexer = true;                  // Indicate the the lexer DFA needs to be rebuilt.
    token_set.emplace_back( name, regex, id, false, false, desc );
    token_map[name] = id;
    return id;
  }

  template <int MAX_ID>
  int Lexer_Base<MAX_ID>::GetTokenID(String name) const {
    int default_id = ERROR_ID;
    if (name.size() == 1) default_id = (int) name[0];
    return emp::Find(token_map, name, default_id);
  }

  template <int MAX_ID>
  const TokenType & Lexer_Base<MAX_ID>::GetTokenType(int id) const {
    if (id > MAX_ID || id <= cur_token_id) return ERROR_TOKEN();
    return token_set[(size_t)(MAX_ID - id)];
  }

  template <int MAX_ID>
  String Lexer_Base<MAX_ID>::GetTokenName(int id) const {
    if (id < 0) return emp::to_string("Error (", id, ")");   // Negative tokens specify errors.
    if (id == 0) return "EOF";                               // Token zero is end-of-file.
    if (id < 128) return emp::to_escaped_string((char) id);  // Individual characters.
    if (id <= cur_token_id) return emp::to_string("Error (", id, ")"); // Invalid token range!
    return GetTokenType(id).name;                            // User-defined tokens.
  }

  template <int MAX_ID>
  bool Lexer_Base<MAX_ID>::GetSaveToken(int id) const {
    if (id > MAX_ID || id <= cur_token_id) return true;
    return GetTokenType(id).save_token;
  }

  template <int MAX_ID>
  void Lexer_Base<MAX_ID>::Generate() const {
    NFA lexer_nfa;
    for (const auto & t : token_set) {
      lexer_nfa.Merge( to_NFA(t.regex, (uint8_t) t.id) );
    }
    generate_lexer = false; // We just generated it!  Don't again until another change is made.
    lexer_dfa = to_DFA(lexer_nfa);
  }

  template <int MAX_ID>
  Token Lexer_Base<MAX_ID>::Process(std::istream & is) const {
    // If we still need to generate the DFA for the lexer, do so.
    if (generate_lexer) Generate();

    size_t cur_pos = 0;   // Position in the input that we are actively analyzing
    size_t best_pos = 0;  // Best look-ahead we've found so far
    int cur_state = 0;    // Next state for the DFA analysis
    int cur_stop = 0;     // Current "stop" state (if we can stop here)
    int best_stop = -1;   // Best stop state found so far?

    lexeme.resize(0);     // Prep the lexeme for the next token.

    // Keep looking as long as:
    // 1: We may be able to continue the current lexeme, and
    // 2: We have not entered an invalid state, and
    // 3: Our input stream has more symbols.
    while (cur_stop >= 0 && cur_state >= 0 && is) {
      const char next_char = (char) is.get();
      cur_pos++;
      cur_state = lexer_dfa.Next(cur_state, (size_t) next_char);
      cur_stop = lexer_dfa.GetStop(cur_state);
      if (cur_stop > 0) { best_pos = cur_pos; best_stop = cur_stop; }
      lexeme.push_back( next_char );
    }

    // If best_pos < cur_pos, rewind the input stream and adjust the lexeme.
    if (best_pos > 0 && best_pos < cur_pos) {
      lexeme.resize(best_pos);
      while (best_pos < cur_pos) { is.unget(); cur_pos--; }
    }

    // If we can't find a token, return 0 (for end of stream) or error (for no valid options)
    if (best_stop < 0) {
      if (!is) return { 0, "" };   // 0 = end of stream.
      return { ERROR_ID, lexeme }; // ERROR_ID = no valid tokens available!
    }

    // Otherwise return the best token we've found so far.
    return { best_stop, lexeme };
  }

  template <int MAX_ID>
  Token Lexer_Base<MAX_ID>::Process(std::string & in_str) const {
    std::stringstream ss;
    ss << in_str;
    auto out_val = Process(ss);
    in_str = ss.str();
    return out_val;
  }

  template <int MAX_ID>
  Token Lexer_Base<MAX_ID>::ToToken(std::string_view in_str) const {
    std::stringstream ss;
    ss << in_str;
    return Process(ss);
  }

  template <int MAX_ID>
  emp::Token Lexer_Base<MAX_ID>::TokenizeNext(std::istream & is, size_t & cur_line) const {
    // Loop until we get a token to return or hit the end of the stream.
    while (true) {
      emp::Token token = Process(is);
      token.line_id = cur_line;
      cur_line += emp::count(token.lexeme, '\n');
      if (GetSaveToken(token)) return token;      // Skip ignored tokens and search for another.
    }
  }

  template <int MAX_ID>
  TokenStream Lexer_Base<MAX_ID>::Tokenize(std::istream & is, String name) const {
    emp::vector<Token> out_tokens;
    size_t cur_line = 1;
    while (emp::Token token = TokenizeNext(is, cur_line)) {
      out_tokens.push_back(token);
    }
    return TokenStream{out_tokens, name};
  }

  template <int MAX_ID>
  TokenStream Lexer_Base<MAX_ID>::Tokenize(std::string_view str, String name) const {
    std::stringstream ss;
    ss << str;
    return Tokenize(ss, name);
  }

  template <int MAX_ID>
  TokenStream Lexer_Base<MAX_ID>::Tokenize(const emp::vector<String> & sv, String name) const
  {
    std::stringstream ss;
    for (auto & str : sv) {
      ss << str << '\n';
    }

    return Tokenize(ss, name);
  }

  template <int MAX_ID>
  void Lexer_Base<MAX_ID>::Print(std::ostream & os) const {
    for (const auto & t : token_set) t.Print(os);
    if (generate_lexer) Generate();               // Do we need to regenerate the lexer?
    lexer_dfa.Print(os);                          // Table driven lexer implementation.
  }

  template <int MAX_ID>
  void Lexer_Base<MAX_ID>::DebugString(String test_string) const {
    std::stringstream ss;
    ss << test_string;

    emp::Token token = 1;
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
  void Lexer_Base<MAX_ID>::WriteCPP(emp::CPPFile & file, emp::String object_name) const
  {
    // If we still need to generate the DFA for the lexer, do so.
    if (generate_lexer) Generate();

    file.Include("<algorithm>")
        .Include("<array>")
        .Include("<iostream>")
        .Include("<string>")
        .Include("<unordered_map>")
        .Include("<vector>");

    file.AddCode("// Struct to store information about a found Token")
        .AddCode("struct Token {")
        .AddCode("  int id;                             // Type ID for token")
        .AddCode("  std::string lexeme;                 // Sequence matched by token")
        .AddCode("  size_t line_id;                     // Line token started on")
        .AddCode("  operator int() const { return id; } // Auto-convert tokens to IDs")
        .AddCode("};")
        .AddCode("");

    file.AddCode("// Deterministic Finite Automaton (DFA) for token recognition.");
    lexer_dfa.WriteCPP(file, "DFA");
    file.AddCode("");

    file.AddCode("class ", object_name, " {")
        .AddCode("private:")
        .AddCode("  static constexpr size_t NUM_TOKENS=", token_set.size(), ";")
        .AddCode("  static constexpr int ERROR_ID = -1;     ///< Code for unknown token ID.")
        .AddCode("")
        .AddCode("  // -- Current State --")
        .AddCode("  size_t cur_line = 1; // Lexeme found for the current token")
        .AddCode("  std::string lexeme;  // Lexeme found for the current token")
        .AddCode("  std::string errors;  // Description of any errors encountered")
        .AddCode("")
        .AddCode("public:")
        .AddCode("  static constexpr size_t ID__EOF_ = 0;");
    for (size_t i = token_set.size(); i > 0; --i) {
      const auto & token = token_set[i-1];
      file.AddCode("  static constexpr size_t ID_", token.name, " = ", token.id, ";");
    }
    file.AddCode("")
        .AddCode("  // Return the name of a token given its ID.")
        .AddCode("  static constexpr const char * TokenName(size_t id) {")
        .AddCode("    switch (id) {")
        .AddCode("    case 0: return \"_EOF_\";");
    for (size_t i = token_set.size(); i > 0; --i) {
      const auto & token = token_set[i-1];
      file.AddCode("    case ", token.id, ": return ", token.name.AsLiteral(), ";");
    }
    file.AddCode("    default: return \"_UNKNOWN_\";" )
        .AddCode("    };")
        .AddCode("  }")
        .AddCode("")
        .AddCode("  // Identify if a token (by ID) should be skipped during tokenizing.")
        .AddCode("  static constexpr bool IgnoreToken(size_t id) {")
        .AddCode("    switch (id) {")
        .AddCode("    case 0:");
    for (size_t i = token_set.size(); i > 0; --i) {
      const auto & token = token_set[i-1];
      if (token.save_lexeme == false)
      file.AddCode("    case ", token.id, ":");
    }
    file.AddCode("      return true;")
        .AddCode("    default: return false;")
        .AddCode("    };")
        .AddCode("  }")
        .AddCode("")
        .AddCode("  // Return the number of token types the lexer recognizes.")
        .AddCode("  static constexpr size_t GetNumTokens() { return NUM_TOKENS; }")
        .AddCode("")
        .AddCode("  // Generate and return the next token from the input stream.")
        .AddCode("  Token NextToken(std::istream & is) {")
        .AddCode("    size_t cur_pos = 0;   // Position in the input that we are actively analyzing")
        .AddCode("    size_t best_pos = 0;  // Best look-ahead we've found so far")
        .AddCode("    int cur_state = 0;    // Next state for the DFA analysis")
        .AddCode("    int cur_stop = 0;     // Current \"stop\" state (or 0 if we can't stop here)")
        .AddCode("    int best_stop = -1;   // Best stop state found so far?")
        .AddCode("    lexeme.resize(0);     // Reset lexeme for next token")
        .AddCode("")
        .AddCode("    // Keep looking as long as:")
        .AddCode("    // 1: We may be able to continue the current lexeme, and")
        .AddCode("    // 2: We have not entered an invalid state, and")
        .AddCode("    // 3: Our input stream has more symbols to provide")
        .AddCode("    while (cur_stop >= 0 && cur_state >= 0 && is) {")
        .AddCode("      const char next_char = static_cast<char>(is.get());")
        .AddCode("      if (next_char < 0) break;")      
        .AddCode("      cur_pos++;")
        .AddCode("      cur_state = DFA::GetNext(cur_state, static_cast<size_t>(next_char));")
        .AddCode("      cur_stop = DFA::GetStop(cur_state);")
        .AddCode("      if (cur_stop > 0) { best_pos = cur_pos; best_stop = cur_stop; }")
        .AddCode("      lexeme.push_back( next_char );")
        .AddCode("    }")
        .AddCode("")
        .AddCode("    // If best_pos < cur_pos, rewind the input stream and adjust the lexeme.")
        .AddCode("    if (best_pos > 0 && best_pos < cur_pos) {")
        .AddCode("      lexeme.resize(best_pos);")
        .AddCode("      while (best_pos < cur_pos) { is.unget(); cur_pos--; }")
        .AddCode("    }")
        .AddCode("")
        .AddCode("    // Update the line number we are on.")
        .AddCode("    const size_t out_line = cur_line;")
        .AddCode("    cur_line+=std::count(lexeme.begin(),lexeme.end(),'\\n');")
        .AddCode("")
        .AddCode("    // If we can't find a token, return 0 (for end of stream) or error (for no valid options)")
        .AddCode("    if (best_stop < 0) {")
        .AddCode("      if (!is) return { 0, \"\", out_line };  // 0 = end of stream.")
        .AddCode("      return { ERROR_ID, lexeme, out_line };  // ERROR_ID = no valid tokens available!")
        .AddCode("    }")
        .AddCode("")
        .AddCode("    // Otherwise return the best token we've found so far.")
        .AddCode("    return { best_stop, lexeme, out_line };")
        .AddCode("  }")
        .AddCode("")
        .AddCode("  // Analyze an input stream and return a vector of all tokens.")
        .AddCode("  std::vector<Token> Tokenize(std::istream & is) {")
        .AddCode("    cur_line = 1;  // Assume we are starting at the first line of the input.")
        .AddCode("    std::vector<Token> out_tokens;")
        .AddCode("    while (Token token = NextToken(is)) {")
        .AddCode("      if (!IgnoreToken(token.id)) out_tokens.push_back(token);")
        .AddCode("    }")
        .AddCode("    return out_tokens;")
        .AddCode("  }")
        .AddCode("};");

  }

  
}

#endif // #ifndef EMP_COMPILER_LEXER_HPP_INCLUDE
