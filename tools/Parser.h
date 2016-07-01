//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A general-purpose, fast parser.

#ifndef EMP_PARSER_H
#define EMP_PARSER_H

#include "Lexer.h"

namespace emp {

  struct ParseRule {
  };

  class Parser {
  private:
    Lexer & lexer;

  public:
    Parser(Lexer & in_lexer) : lexer(in_lexer) { ; }
    ~Parser() { ; }

    Lexer & GetLexer() { return lexer; }
  };

}

#endif
