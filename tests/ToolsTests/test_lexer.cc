#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include "tools/Lexer.h"

#include <sstream>

TEST_CASE("Test Lexer", "[tools]")
{
  emp::Lexer lx;
  REQUIRE(lx.GetNumTokens() == 0);
  lx.AddToken("vowel", "[a-z]*([aeiou])+[a-z]*");
  REQUIRE(lx.GetNumTokens() == 1);
  REQUIRE(lx.GetTokenID("vowel") == 255);
  REQUIRE(lx.GetTokenName(255) == "vowel");
  std::string word = "hello";
  lx.Process(word);
  REQUIRE(lx.GetLexeme() == word);
  word = "why";
  emp::Token t = lx.Process(word);
  REQUIRE(t.lexeme != word);
  REQUIRE(t.lexeme == "");
}