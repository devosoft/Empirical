/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file Lexer.cpp
 */

#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/compiler/Lexer.hpp"

TEST_CASE("Test Lexer", "[compiler]")
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

TEST_CASE("Another Test Lexer", "[compiler]")
{
  emp::Lexer lexer;
  lexer.AddToken("Integer", "[0-9]+");
  lexer.AddToken("Float", "[0-9]*\\.[0-9]+");
  lexer.AddToken("Lower", "[a-z]+");
  lexer.AddToken("Upper", "[A-Z]+");
  lexer.AddToken("Mixed", "[a-zA-Z]+");
  lexer.AddToken("Whitespace", "[ \t\n\r]");
  lexer.AddToken("Other", ".");

  std::stringstream ss;
  ss << "This is a 123 TEST.  It should also have 1. .2 123.456 789 FLOATING point NUMbers!";

  REQUIRE(lexer.Process(ss).lexeme == "This");
  REQUIRE(lexer.Process(ss).lexeme == " ");
  REQUIRE(lexer.Process(ss).lexeme == "is");
  REQUIRE(lexer.Process(ss).lexeme == " ");
  REQUIRE(lexer.Process(ss).lexeme == "a");
  REQUIRE(lexer.Process(ss).lexeme == " ");
  REQUIRE(lexer.Process(ss).lexeme == "123");
  REQUIRE(lexer.Process(ss).lexeme == " ");
  REQUIRE(lexer.Process(ss).lexeme == "TEST");
  REQUIRE(lexer.Process(ss).lexeme == ".");
  REQUIRE(lexer.Process(ss).lexeme == " ");
  REQUIRE(lexer.Process(ss).lexeme == " ");

  REQUIRE(lexer.GetTokenName(lexer.Process(ss)) == "Mixed");
  REQUIRE(lexer.GetTokenName(lexer.Process(ss)) == "Whitespace");
  REQUIRE(lexer.GetTokenName(lexer.Process(ss)) == "Lower");
}
