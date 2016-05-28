//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A simple lexer for the Empirical configuration language.
//
//
//  Development notes: Initially building the lexer to be language specific, but a
//  more general lexer will be a good addition to Empirical.

#ifndef EMP_CONFIG_LEXER_H
#define EMP_CONFIG_LEXER_H

#include <iostream>
#include <string>

#include "Token.h"

namespace emp {

  class ConfigLexer {
  private:
    std::istream & is;
    char next_char;

    std::string cur_lexeme;

  public:
    ConfigLexer(std::istream & in_stream) : is(in_stream) { is.get(next_char); }
    ConfigLexer(ConfigLexer &) = delete;
    ~ConfigLexer() { ; }

    emp::Token GetToken() {
      if (next_char == '\0') return Token(Token::NONE);

      if (is_digit(next_char)) {       // Must be a number
        cur_lexeme.resize(1);
        cur_lexeme[0] = next_char;
        while (is_digit(next_char = is.get())) {
          cur_lexeme.push_back(next_char);
        }
        return Token(Token::INT_LIT, cur_lexeme);
      }
      if (is_idchar(next_char)) {                 // Must be ID or Keyword (number already captured)
        cur_lexeme.resize(1);
        cur_lexeme[0] = next_char;
        while (is_idchar(next_char = is.get())) {
          cur_lexeme.push_back(next_char);
        }
        return Token(Token::INT_LIT, cur_lexeme);
      }

      char unk_char = next_char;
      next_char = is.get()

      return Token(Token::UNKNOWN, std::string(1, unk_char));
    }
  };

}

#endif
