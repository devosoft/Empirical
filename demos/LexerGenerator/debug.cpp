#include <fstream>
#include <iostream>
#include "lexer-debug.hpp"

int main() {
  std::ifstream file("debug.txt");
  emplex::Lexer lexer;
  auto tokens = lexer.Tokenize(file);
  for (auto token : tokens) {
    std::cout << token.id << " : " << lexer.TokenName(token.id) << " : " << token.lexeme << std::endl;
  }
}

