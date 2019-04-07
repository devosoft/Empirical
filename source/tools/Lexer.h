/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2019.
 *
 *  @file  Lexer.h
 *  @brief A general-purpose, fast lexer.
 *  @note Status: BETA
 */

#ifndef EMP_LEXER_H
#define EMP_LEXER_H

#include <map>
#include <string>

#include "../base/vector.h"

#include "lexer_utils.h"
#include "RegEx.h"

namespace emp {

  /// Information about an individual token type to be processed within a Lexer.
  struct TokenInfo {
    std::string name;    ///< Name of this token type.
    RegEx regex;         ///< Pattern to describe token type.
    size_t id;           ///< Unique id for token.
    bool save_lexeme;    ///< Should we preserve the lexeme for this token?
    bool discard;        ///< Should we elimininate this token after it is identified.

    TokenInfo(const std::string & _name, const std::string & _regex, size_t _id,
              bool _save=true, bool _discard=false)
      : name(_name), regex(_regex), id(_id), save_lexeme(_save), discard(_discard) { ; }
    TokenInfo(const TokenInfo &) = default;
    TokenInfo & operator=(const TokenInfo &) = default;

    /// Print out the status of this token (for debugging)
    void Print(std::ostream & os=std::cout) const {
      os << "Name:" << name
         << "  RegEx:" << regex.AsString()
         << "  ID:" << id
         << "  save_lexeme:" << save_lexeme
         << "  discard:" << discard
         << std::endl;
    }
  };

  /// Information about a token instance from an input stream.
  struct Token {
    size_t token_id;     ///< Which type of token is this?
    std::string lexeme;  ///< The specific sequence matched by this token.

    Token(size_t id, const std::string & str="") : token_id(id), lexeme(str) { ; }
    Token(const Token &) = default;
    Token & operator=(const Token &) = default;

    /// Token will automatically convert to its ID if used as an unsigned int.
    operator size_t() { return token_id; }

    /// Token will automatically convert to its matched sequence (lexeme) is used as a string.
    operator const std::string &() { return lexeme; }
  };

  /// A lexer with a set of token types (and associated regular expressions)
  class Lexer {
  private:
    emp::vector<TokenInfo> token_set;     ///< List of all active tokens.
    size_t cur_token_id;                  ///< Which ID should the next new token get?
    mutable bool generate_lexer;          ///< Do we need to regenerate the lexer?
    mutable DFA lexer_dfa;                ///< Table driven lexer implementation.
    std::string lexeme;                   ///< Current state of lexeme being generated.

  public:
    static const size_t MAX_TOKEN_ID = 256;      // How many token IDs are possible?
    static const size_t ERROR_ID = MAX_TOKEN_ID; // Code for unknown token ID.
    static inline bool TokenOK(size_t id) { return id < MAX_TOKEN_ID; }

    Lexer()
      : token_set(), cur_token_id(MAX_TOKEN_ID), generate_lexer(false), lexer_dfa(), lexeme() { }
    ~Lexer() { ; }

    /// How many types of tokens can be identified in this Lexer?
    size_t GetNumTokens() const { return token_set.size(); }

    /// Add a new token, specified by a name and the regex used to identify it.
    size_t AddToken(const std::string & in_name, const std::string & in_regex,
                    bool save_lexeme=true, bool discard=false) {
      --cur_token_id;
      generate_lexer = true;
      token_set.emplace_back( in_name, in_regex, cur_token_id, save_lexeme, discard );
      return cur_token_id;
    }

    /// How many total token types are allowed in this lexer?
    constexpr static size_t MaxTokenID() { return MAX_TOKEN_ID; }

    /// Get the ID associated with a token type (you provide the token name)
    size_t GetTokenID(const std::string & name) const {
      for (const auto & t : token_set) {
        if (t.name == name) return t.id;
      }
      return ERROR_ID;
    }

    /// Get the name associated with a token type (you provide the ID)
    std::string GetTokenName(size_t id) const {
      if (id >= MAX_TOKEN_ID) return "Error";
      if (id == 0) return "EOF";
      if (id < 128) return emp::to_escaped_string((char) id);  // Individual characters.
      for (const auto & t : token_set) {
        if (t.id == id) return t.name;
      }
      return "Unknown";
    }

    /// Get the full information about a token (you provide the name)
    TokenInfo GetTokenInfo(const std::string & name) const {
      for (const auto & t : token_set) {
        if (t.name == name) return t;
      }
      return TokenInfo("", "", ERROR_ID);
    }

    /// Create the NFA that will identify the current set of tokens in a sequence.
    void Generate() const {
      NFA lexer_nfa;
      for (const auto & t : token_set) {
        lexer_nfa.Merge( to_NFA(t.regex, t.id) );
      }
      generate_lexer = false; // We just generated it!  Don't again unless another change is made.
      lexer_dfa = to_DFA(lexer_nfa);
    }

    /// Get the next token found in an input stream.
    Token Process(std::istream & is) {
      if (generate_lexer) Generate();
      size_t cur_pos = 0;
      size_t best_pos = 0;
      int cur_state = 0;
      int cur_stop = 0;
      int best_stop = -1;
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

      // If we are at the end of this input stream (with no token to return) give back a 0.
      if (best_stop < 0) {
        if (!is) return { 0, "" };
        return { ERROR_ID, lexeme };
      }

      return { (size_t) best_stop, lexeme };
    }

    /// Shortcut to process a string rather than a stream.
    Token Process(std::string & in_str) {
      std::stringstream ss;
      ss << in_str;
      auto out_val = Process(ss);
      in_str = ss.str();
      return out_val;
    }

    emp::vector<Token> Tokenize(std::istream & is) {
      emp::vector<Token> out_tokens;
      emp::Token token = Process(is);
      while (token > 0) {
        out_tokens.push_back(token);
        token = Process(is);
      }
      return out_tokens;
    }

    emp::vector<Token> Tokenize(std::string & str) {
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
  };


}

#endif
