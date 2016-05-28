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

  public:
    ConfigLexer(std::istream & in_stream) : is(in_stream) { ; }
    ConfigLexer(ConfigLexer &) = delete;
    ~ConfigLexer() { ; }
  };

}

#endif
