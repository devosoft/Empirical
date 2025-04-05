/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2016-2024.
*/
/**
 *  @file
 *  @brief Some examples code for using emp::Lexer
 */

#include <fstream>
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

  emp::CPPFile file("auto-lexer.cpp");
  file.Include("<sstream>");
  // file.SetGuards("EXAMPLE_GENERATED_LEXER__HPP");
  lexer.WriteCPP(file, "Lexer");
  file.AddCode("")
      .AddCode("int main() {")
      .AddCode("  Lexer lexer;")
      .AddCode("  std::stringstream ss;")
      .AddCode("  ss << \"This is a 123 TEST.  It should also have 1. .2 123.456 789 FLOATING point NUMbers!\";")
      .AddCode("  auto tokens = lexer.Tokenize(ss);")
      .AddCode("  for (auto token : tokens) {")
      .AddCode("    std::cout << lexer.GetTokenName(token.id) << \": '\" << token.lexeme << \"'\" << std::endl;")
      .AddCode("  }")
      .AddCode("}")
      ;
  file.Write();
}
