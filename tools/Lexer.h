//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A general-purpose, fast lexer.

#ifndef EMP_LEXER_H
#define EMP_LEXER_H

#include <map>
#include <string>

#include "lexer_utils.h"
#include "RegEx.h"
#include "vector.h"

namespace emp {

  struct TokenInfo {
    std::string name;    // Name of this token type.
    RegEx regex;         // Pattern to describe token type.
    int id;              // Unique id for token.
    bool save_lexeme;    // Should we preserve the lexeme for this token?

    TokenInfo() : name(""), regex(""), id(-1), save_lexeme(false) { ; }
    TokenInfo(const std::string & n, const std::string & r, int i, bool s=false)
      : name(n), regex(r), id(i), save_lexeme(s) { ; }
    TokenInfo(const TokenInfo &) = default;
    TokenInfo & operator=(const TokenInfo &) = default;

    void Print(std::ostream & os=std::cout) const {
      os << "Name:" << name
         << "  RegEx:" << regex.AsString()
         << "  ID:" << id
         << "  save_lexeme:" << save_lexeme
         << std::endl;
    }
  };

  class Lexer {
  private:
    emp::vector<TokenInfo> token_set;     // List of all active tokens.
    int cur_token_id;                     // Which ID should the next new token get?
    mutable bool generate_lexer;          // Do we need to regenerate the lexer?
    mutable DFA lexer_dfa;                // Table driven lexer implementation.
    std::string lexeme;                   // Current state of lexeme being generated.

    static const int MAX_TOKEN_ID = 256;  // How many token IDs are possible?

  public:
    Lexer() : cur_token_id(MAX_TOKEN_ID), generate_lexer(false) { ; }
    ~Lexer() { ; }

    int AddToken(const std::string & in_name, const std::string & in_regex) {
      --cur_token_id;
      generate_lexer = true;
      token_set.emplace_back( in_name, in_regex, cur_token_id );
      return cur_token_id;
    }

    constexpr static int MaxTokenID() { return MAX_TOKEN_ID; }
    int GetTokenID(const std::string & name) const {
      for (const auto & t : token_set) {
        if (t.name == name) return t.id;
      }
      return -1;
    }
    std::string GetTokenName(int id) const {
      if (id == -1) return "Error";
      if (id == 0) return "EOF";
      if (id < 128) return emp::to_escaped_string((char) id);  // Individual characters.
      for (const auto & t : token_set) {
        if (t.id == id) return t.name;
      }
      return "Unknown";
    }
    TokenInfo GetTokenInfo(const std::string & name) const {
      for (const auto & t : token_set) {
        if (t.name == name) return t;
      }
      return TokenInfo("", "", -1);
    }

    void Generate() const {
      NFA lexer_nfa;
      for (const auto & t : token_set) {
        lexer_nfa.Merge( to_NFA(t.regex, t.id) );
      }
      generate_lexer = false; // We just generated it!  Don't again unless another change is made.
      lexer_dfa = to_DFA(lexer_nfa);
    }

    // Get the next token found in an input stream.
    int Process(std::istream & is) {
      if (generate_lexer) Generate();
      int cur_state = 0;
      int cur_stop = 0;
      int cur_pos = 0;
      int best_pos = 0;
      int best_stop = -1;
      lexeme.resize(0);

      // Keep looking as long as:
      // 1: We may still be able to contine the current lexeme.
      // 2: We have not entered an invalid state.
      // 3: Our input stream has more symbols.
      while (cur_stop >= 0 && cur_state >= 0 && is) {
        const uint8_t next_char = is.get();
        cur_pos++;
        cur_state = lexer_dfa.Next(cur_state, next_char);
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
      if (best_stop == -1 && !is) return 0;

      return best_stop;
    }

    // Get the lexeme associated with the last token.
    const std::string & GetLexeme() { return lexeme; }

    void Print(std::ostream & os=std::cout) const {
      for (const auto & t : token_set) t.Print(os);
      if (generate_lexer) Generate();                   // Do we need to regenerate the lexer?
      lexer_dfa.Print(os);                         // Table driven lexer implementation.
    }
  };


}

#endif
