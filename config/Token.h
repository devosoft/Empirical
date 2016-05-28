//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A simple lexer for the Empirical configuration language.
//
//
//  Development notes: Initially building the lexer to be language specific, but a
//  more general lexer will be a good addition to Empirical.

#ifndef EMP_TOKEN_H
#define EMP_TOKEN_H

#include <iostream>
#include <string>

namespace emp {

  struct Token {
    std::string lexeme;
    enum TolenTypes { NONE=0, UNKNOWN=256,
      INT_LIT, FLOAT_LIT, CHAR_LIT, STRING_LIT, ID,

      CASSIGN_ADD, CASSIGN_SUB, CASSIGN_MULT, CASSIGN_DIV, CASSIGN_MOD,
      COMP_EQU, COMP_NEQU, COMP_LESS, COMP_LTE, COMP_GTR, COMP_GTE,
      BOOL_AND, BOOL_OR,
      COMMAND_PRINT, COMMAND_RANDOM,
      COMMAND_IF, COMMAND_ELSE, COMMAND_WHILE, COMMAND_FOREACH, COMMAND_BREAK,
      COMMAND_FUNCTION, COMMAND_RETURN };

    Token(const std::string & in_lexeme) : lexeme(in_lexeme) { ; }
    Token(const Token &) = default;
    ~Token() { ; }
  };

}

#endif
