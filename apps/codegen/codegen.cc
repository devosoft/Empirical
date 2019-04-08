/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  codegen.cc
 *  @brief A system to generate dynamic concept code for C++17.
 *  @note Status: PLANNING
 */

#include <iostream>

#include "CodeGen.h"

int main(int argc, char *argv[])
{
  if (argc != 2) {
    std::cout << "Format: " << argv[0] << " [input file]" << std::endl;
    exit(0);
  }

  CodeGen codegen(argv[1]);
  codegen.PrintLexerState();
  std::cout << std::endl;
  codegen.PrintTokens();
}
