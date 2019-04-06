/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  concept_gen.cc
 *  @brief A system to generate dynamic concept code for C++17.
 *  @note Status: PLANNING
 */

#include <iostream>

#include "../source/tools/File.h"
#include "../source/tools/Lexer.h"

int main(int argc, char *argv[])
{
  if (argc != 2) {
    std::cout << "Format: " << argv[0] << " [input file]" << std::endl;
    exit(0);
  }

  emp::Lexer lexer;
  lexer.AddToken("ID", "[a-zA-Z0-9.]+");         // Identifiers, tokens, and numbers (including dots)
  lexer.AddToken("Whitespace", "[ \t\n\r]+");    // Any form of whitespace.
  lexer.AddToken("String", "\\\"[^\"]*\\\"");    // Literal strings.
  lexer.AddToken("Other", ".");                  // Symbols

  lexer.Print();
  std::cout << std::endl;

  lexer.DebugString("This is    a \"test\".");
  // emp::File file(argv[1]);
}
