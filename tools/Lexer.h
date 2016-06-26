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
    std::string name;
    RegEx regex;
    int id;

    TokenInfo() : name(""), regex(""), id(-1) { ; }
    TokenInfo(const std::string & in_name, const std::string & in_regex, int in_id)
      : name(in_name), regex(in_regex), id(in_id) { ; }
    TokenInfo(const TokenInfo &) = default;
    TokenInfo & operator=(const TokenInfo &) = default;
  };

  class Lexer {
  private:
    std::map<std::string, TokenInfo> token_map;         // List of all active tokens.
    int cur_token_id;                                   // Which ID should the next token get?
    bool generate_lexer;                                // Do we need to regenerate the lexer?
    DFA lexer_dfa;                                      // Table driven lexer implementation.

    static const int MAX_TOKEN_ID = 65536;              // How many token IDs are possible?

    void Generate() {
      NFA lexer_nfa;
      for (auto t : token_map) {
        lexer_nfa.Merge( to_NFA(t.second.regex, t.second.id) );
      }
      generate_lexer = false; // We just generated it!  Don't again unless another change is made.
      lexer_dfa = to_DFA(lexer_nfa);
    }
  public:
    Lexer() : cur_token_id(MAX_TOKEN_ID), generate_lexer(false) { ; }
    ~Lexer() { ; }

    int AddToken(const std::string & in_name, const std::string & in_regex) {
      --cur_token_id;
      generate_lexer = true;
      token_map[in_name] = TokenInfo(in_name, in_regex, cur_token_id);
      return cur_token_id;
    }

    int GetTokenID(const std::string & name) const {
      auto ti = token_map.find(name);
      if (ti == token_map.end()) return -1;
      return ti->second.id;
    }
    TokenInfo GetTokenInfo(const std::string & name) const {
      auto ti = token_map.find(name);
      if (ti == token_map.end()) return TokenInfo("", "", -1);
      return ti->second;
    }


  };

}

#endif
