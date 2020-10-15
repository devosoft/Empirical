/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2019.
 *
 *  @file  Lexer.h
 *  @brief A general-purpose, fast lexer.
 *  @note Status: ALPHA
 */

#ifndef EMP_LEXER_H
#define EMP_LEXER_H

#include <map>
#include <string>

#include "../base/map.h"
#include "../base/vector.h"

#include "lexer_utils.h"
#include "map_utils.h"
#include "RegEx.h"

namespace emp {

  /// Information about an individual TYPE of token to be processed within a Lexer.
  struct TokenInfo {
    std::string name;    ///< Name of this token type.
    std::string desc;    ///< More detailed description of this token type.
    RegEx regex;         ///< Pattern to describe token type.
    int id;              ///< Unique id for token.
    bool save_lexeme;    ///< Preserve the lexeme for this token?
    bool save_token;     ///< Keep token at all? (Whitespace and comments are often discarded).

    TokenInfo(const std::string & _name, const std::string & _regex, int _id,
              bool _save_l=true, bool _save_t=true, const std::string & _desc="")
      : name(_name), desc(_desc), regex(_regex), id(_id), save_lexeme(_save_l), save_token(_save_t) { ; }
    TokenInfo(const TokenInfo &) = default;
    TokenInfo(TokenInfo &&) = default;
    TokenInfo & operator=(const TokenInfo &) = default;
    TokenInfo & operator=(TokenInfo &&) = default;

    /// Print out the status of this token (for debugging)
    void Print(std::ostream & os=std::cout) const {
      os << "Name:" << name
         << "  RegEx:" << regex.AsString()
         << "  ID:" << id
         << "  save_lexeme:" << save_lexeme
         << "  save_token:" << save_token
         << std::endl;
    }
  };

  /// Information about a token instance from an input stream.
  struct Token {
    int token_id;        ///< Which type of token is this?
    std::string lexeme;  ///< Sequence matched by this token (or empty if not saved)
    size_t line_id;      ///< Which line did this token start on?

    Token(int id, const std::string & str="", size_t _line=0)
      : token_id(id), lexeme(str), line_id(_line) { ; }
    Token(const Token &) = default;
    Token(Token &&) = default;
    Token & operator=(const Token &) = default;
    Token & operator=(Token &&) = default;

    /// Token will automatically convert to its ID if used as an int.
    operator int() const { return token_id; }

    /// Token will automatically convert to its matched sequence (lexeme) is used as a string.
    operator const std::string &() const { return lexeme; }
  };

  /// A lexer with a set of token types (and associated regular expressions)
  class Lexer {
  private:
    static constexpr int MAX_ID = 255;      ///< IDs count down so that first ones have priority.
    static constexpr int ERROR_ID = -1;     ///< Code for unknown token ID.

    emp::vector<TokenInfo> token_set;       ///< List of all active tokens.
    emp::map<std::string, int> token_map;   ///< Map of token names to id.
    int cur_token_id = MAX_ID;              ///< Which ID should the next new token get?
    mutable bool generate_lexer = false;    ///< Do we need to regenerate the lexer?
    mutable DFA lexer_dfa;                  ///< Table driven lexer implementation.
    std::string lexeme;                     ///< Current state of lexeme being generated.

    const TokenInfo ERROR_TOKEN{"", "", ERROR_ID, true, true, "Unable to parse input!"};

  public:
    Lexer() { ; }
    ~Lexer() { ; }

    /// How many types of tokens can be identified in this Lexer?
    size_t GetNumTokens() const { return token_set.size(); }

    bool TokenOK(int id) const { return id >= 0 && id < cur_token_id; }

    /// Add a new token, specified by a name and the regex used to identify it.
    /// Note that token ids count down with highest IDs having priority.
    int AddToken(const std::string & name, const std::string & regex,
                    bool save_lexeme=true, bool save_token=true, const std::string & desc="") {
      int id = cur_token_id--;                // Grab the next available token id.
      generate_lexer = true;                  // Indicate the the lexer DFA needs to be rebuilt.
      token_set.emplace_back( name, regex, id, save_lexeme, save_token, desc );
      token_map[name] = id;
      return id;
    }

    /// Add a token to ignore, specified by a name and the regex used to identify it.
    int IgnoreToken(const std::string & name, const std::string & regex, const std::string & desc="") {
      int id = cur_token_id--;                // Grab the next available token id.
      generate_lexer = true;                  // Indicate the the lexer DFA needs to be rebuilt.
      token_set.emplace_back( name, regex, id, false, false, desc );
      token_map[name] = id;
      return id;
    }

    /// Get the ID associated with a token type (you provide the token name)
    int GetTokenID(const std::string & name) const {
      int default_id = ERROR_ID;
      if (name.size() == 1) default_id = (int) name[0];
      return emp::Find(token_map, name, default_id);
    }

    /// Get the full information about a token (you provide the id)
    const TokenInfo & GetTokenInfo(int id) const {
      if (id > MAX_ID || id <= cur_token_id) return ERROR_TOKEN;
      return token_set[(size_t)(MAX_ID - id)];
    }

    /// Get the name associated with a token type (you provide the ID)
    std::string GetTokenName(int id) const {      
      if (id < 0) return emp::to_string("Error (", id, ")");   // Negative tokens specify errors.
      if (id == 0) return "EOF";                               // Token zero is end-of-file.
      if (id < 128) return emp::to_escaped_string((char) id);  // Individual characters.
      if (id <= cur_token_id) return emp::to_string("Error (", id, ")"); // Invalid token range!
      return GetTokenInfo(id).name;                            // User-defined tokens.
    }

    /// Identify if a token should be saved.
    bool GetSaveToken(int id) const {
      if (id > MAX_ID || id <= cur_token_id) return true;
      return GetTokenInfo(id).save_token;
    }

    /// Create the NFA that will identify the current set of tokens in a sequence.
    void Generate() const {
      NFA lexer_nfa;
      for (const auto & t : token_set) {
        lexer_nfa.Merge( to_NFA(t.regex, (uint8_t) t.id) );
      }
      generate_lexer = false; // We just generated it!  Don't again until another change is made.
      lexer_dfa = to_DFA(lexer_nfa);
    }

    /// Get the next token found in an input stream.  Do so by examining one character at a time.
    /// Keep going as long as there is a chance of a valid lexeme (since we want to choose the
    /// longest one we can find.)  Every time we do hit a valid lexeme, store it as the current
    /// "best" and keep going.  Once we hit a point where no other valid lexemes are possible,
    /// stop and return the best we've found so far.
    Token Process(std::istream & is) {
      // If we still need to generate the DFA for the lexer, do so.
      if (generate_lexer) Generate();

      size_t cur_pos = 0;   // What position in the file are we actively analyzing?
      size_t best_pos = 0;  // What is the best look-ahead we've found so far?
      int cur_state = 0;    // What is the next state for the DFA analysis?
      int cur_stop = 0;     // What is the current "stop" state (if we can stop here)
      int best_stop = -1;   // What is the best stop state found so far?

      lexeme.resize(0);

      // Keep looking as long as:
      // 1: We may still be able to contine the current lexeme.
      // 2: We have not entered an invalid state.
      // 3: Our input stream has more symbols.
      while (cur_stop >= 0 && cur_state >= 0 && is) {
        const char next_char = (char) is.get();
        cur_pos++;
        cur_state = lexer_dfa.Next(cur_state, (size_t) next_char);
        cur_stop = lexer_dfa.GetStop(cur_state);
        if (cur_stop > 0) { best_pos = cur_pos; best_stop = cur_stop; }
        lexeme.push_back( next_char );
      }

      // If best_pos < cur_pos, we need to rewind the input stream and adjust the lexeme.
      if (best_pos > 0 && best_pos < cur_pos) {
        lexeme.resize(best_pos);
        while (best_pos < cur_pos) { is.unget(); cur_pos--; }
      }

      // If we are at the end of this input stream and still haven't found a token, return 0.
      if (best_stop < 0) {
        if (!is) return { 0, "" };
        return { ERROR_ID, lexeme };
      }

      // Otherwise return the best token we've found so far.
      return { best_stop, lexeme };
    }

    /// Shortcut to process a string rather than a stream.
    Token Process(std::string & in_str) {
      std::stringstream ss;
      ss << in_str;
      auto out_val = Process(ss);
      in_str = ss.str();
      return out_val;
    }

    /// Turn an input stream of text into a vector of tokens.
    emp::vector<Token> Tokenize(std::istream & is) {
      emp::vector<Token> out_tokens;
      size_t cur_line = 1;
      emp::Token token = Process(is);
      while (token > 0) {
        token.line_id = cur_line;
        cur_line += emp::count(token.lexeme, '\n');
        if (GetSaveToken(token)) out_tokens.push_back(token);
        token = Process(is);
      }
      return out_tokens;
    }

    /// Turn an input string into a vector of tokens.
    emp::vector<Token> Tokenize(const std::string & str) {
      std::stringstream ss;
      ss << str;
      return Tokenize(ss);
    }

    /// Get the lexeme associated with the last token identified.
    const std::string & GetLexeme() { return lexeme; }

    /// Print the full information about this lexer (for debugging)
    void Print(std::ostream & os=std::cout) const {
      for (const auto & t : token_set) t.Print(os);
      if (generate_lexer) Generate();               // Do we need to regenerate the lexer?
      lexer_dfa.Print(os);                          // Table driven lexer implementation.
    }

    /// Try out the lexer on a string and demonstrate how it's tokenized.
    void DebugString(std::string test_string) {
      std::stringstream ss;
      ss << test_string;

      emp::Token token = 1;
      while (token > 0) {
        token = Process(ss);
        std::cout << GetTokenName(token) << " : \"" << token.lexeme << "\"" << std::endl;
      }
    }

    void DebugToken(int token_id) const {
      const auto & token = GetTokenInfo(token_id);
      std::cout << "Debugging token " << token.id << ": '" << token.name << "'.\n"
                << "  Desc: " << token.desc << "\n";
      token.regex.PrintDebug();
    }
  };


}

#endif
