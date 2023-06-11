/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016
 *
 *  @file Lexer.cpp
 *  @brief Some examples code for using emp::Lexer
 */

#include <iostream>

#include "emp/compiler/Lexer.hpp"

int main()
{
  emp::Lexer lexer;
  lexer.AddToken("Integer", "[0-9]+");
  lexer.AddToken("Float", "[0-9]*\\.[0-9]+");
  lexer.AddToken("Lower", "[a-z]+");
  lexer.AddToken("Upper", "[A-Z]+");
  lexer.AddToken("Mixed", "[a-zA-Z]+");
  lexer.AddToken("Whitespace", "[ \t\n\r]");
  lexer.AddToken("Other", ".");

  lexer.Print();

  std::stringstream ss;
  ss << "This is a 123 TEST.  It should also have 1. .2 123.456 789 FLOATING point NUMbers!";

  emp::Token token = 1;
  while (token > 0) {
    token = lexer.Process(ss);
    std::cout << lexer.GetTokenName(token) << " : \"" << token.lexeme << "\"" << std::endl;
  }
}
