/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  concept_gen.cc
 *  @brief A system to generate dynamic concept code for C++17.
 *  @note Status: PLANNING
 */

#include <fstream>
#include <iostream>
#include <string>

#include "../../source/tools/Lexer.h"

class CodeGen {
private:
  std::string filename;
  emp::Lexer lexer;
  emp::vector<emp::Token> tokens;

  int token_id = -1;
  int token_number = -1;
  int token_string = -1;
  int token_other = -1;

public:
  CodeGen(std::string in_filename) : filename(in_filename) {
    lexer.AddToken("Whitespace", "[ \t\n\r]+", false, false);   // Any form of whitespace.
    lexer.AddToken("Comment", "//.*", true, false);             // Any '//'-style comment.
    token_id = lexer.AddToken("ID", "[a-zA-Z_][a-zA-Z0-9_]+", true, true);     // Identifiers
    token_number = lexer.AddToken("Number", "[0-9]+(.[0-9]+)?", true, true);   // Literal numbers.
    token_string = lexer.AddToken("String", "\\\"[^\"]*\\\"", true, true);     // Literal strings.
    token_other = lexer.AddToken("Other", ".", true, true);                    // Symbols

    std::ifstream file(filename);
    tokens = lexer.Tokenize(file);
    file.close();
  }

  // Process the tokens starting from the outer-most scope.
  size_t ProcessTop(size_t pos=0) {
    if (tokens[pos] != token_id) {
      std::cerr << "Statements in outer scope must being with an identifier or keyword.  Aborting."
                << std::endl;
      exit(1);
    }

    if (tokens[pos].lexeme == "concept") {
      return ProcessConcept(pos + 1);
    }
    else {
      std::cerr << "Unknown keyword '" << tokens[pos].lexeme << "'.  Aborting." << std::endl;
      exit(1);
    }
  }

  // We know we are in a concept definition.  Collect appropriate information.
  size_t ProcessConcept(size_t pos) {
    return pos;
  }
  
  void PrintLexerState() { lexer.Print(); }

  void PrintTokens() {
    for (auto token : tokens) {
      std::cout << lexer.GetTokenName(token) << " : \"" << token.lexeme << "\"" << std::endl;
    }
  }
};

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
