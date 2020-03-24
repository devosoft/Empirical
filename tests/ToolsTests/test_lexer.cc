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
  // REQUIRE(lx.MaxTokenID() == 256);
  REQUIRE(!lx.TokenOK(257));
  
  std::string word = "hello";
  lx.Process(word);
  REQUIRE(lx.GetLexeme() == word);
  word = "why";
  emp::Token t = lx.Process(word);
  REQUIRE(t.lexeme != word);
  REQUIRE((std::string)t == "");
  
  std::stringstream ss;
  emp::TokenInfo ti = lx.GetTokenInfo(lx.GetTokenID("vowel"));
  ti.Print(ss);
  REQUIRE(ss.str() == "Name:vowel  RegEx:\"[a-z]*([aeiou])+[a-z]*\"  ID:255  save_lexeme:1  save_token:1\n");
  ss.str(std::string());
  
  emp::Lexer lxr;
  lxr.Print(ss);
  REQUIRE(ss.str() == "Num states = 0\nStop IDs:\n");
}
