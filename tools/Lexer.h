//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A general-purpose, fast lexer.

#ifndef EMP_LEXER_H
#define EMP_LEXER_H

#include <string>

#include "lexer_utils.h"
#include "RegEx.h"
#include "vector.h"

namespace emp {

  struct TokenInfo {
    std::string name;
    RegEx regex;
    int id;
  };

  class Lexer {
  private:
    emp::vector<TokenInfo> token_info;       // List of all active tokens.

    static const int MAX_TOKEN_ID = 65536;   // How many token IDs are possible?
    int cur_token_id;                        // Which ID should the next token get?

    bool generate_lexer;                     // Do we need to regenerate the lexer?
  public:
    Lexer() : cur_token_id(MAX_TOKEN_ID), generate_lexer(false) { ; }
    ~Lexer() { ; }

    int AddToken(const std::string & in_name, const std::string & in_regex) {
      --cur_token_id;
      generate_lexer = true;
      token_info.emplace_back(in_name, in_regex, cur_token_id);
      return cur_token_id;
    }
  };

}

#endif
