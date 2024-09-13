/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2021-2024
*/
/**
 *  @file
 */

#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/compiler/Lexer.hpp"

TEST_CASE("Test Lexer", "[compiler]")
{
  emp::Lexer lx;
  CHECK(lx.GetNumTokens() == 0);
  lx.AddToken("vowel", "[a-z]*([aeiou])+[a-z]*");
  CHECK(lx.GetNumTokens() == 1);
  CHECK(lx.GetTokenID("vowel") == 255);
  CHECK(lx.GetTokenName(255) == "vowel");
  // CHECK(lx.MaxTokenID() == 256);

  std::string word = "hello";
  lx.ToToken(word);
  CHECK(lx.GetLexeme() == word);
  word = "why";
  emp::Token t = lx.ToToken(word);
  CHECK(t.lexeme != word);
  CHECK((std::string)t == "w");

  std::stringstream ss;
  emp::TokenType ti = lx.GetTokenType(lx.GetTokenID("vowel"));
  ti.Print(ss);
  CHECK(ss.str() == "Name:vowel  RegEx:\"[a-z]*([aeiou])+[a-z]*\"  ID:255  save_lexeme:1  save_token:1\n");
  ss.str(std::string());

  emp::Lexer lxr;
  lxr.Print(ss);
  CHECK(ss.str() == "Num states = 0\nStop IDs:\n");
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

  CHECK(lexer.TokenizeNext(ss).lexeme == "This");
  CHECK(lexer.TokenizeNext(ss).lexeme == " ");
  CHECK(lexer.TokenizeNext(ss).lexeme == "is");
  CHECK(lexer.TokenizeNext(ss).lexeme == " ");
  CHECK(lexer.TokenizeNext(ss).lexeme == "a");
  CHECK(lexer.TokenizeNext(ss).lexeme == " ");
  CHECK(lexer.TokenizeNext(ss).lexeme == "123");
  CHECK(lexer.TokenizeNext(ss).lexeme == " ");
  CHECK(lexer.TokenizeNext(ss).lexeme == "TEST");
  CHECK(lexer.TokenizeNext(ss).lexeme == ".");
  CHECK(lexer.TokenizeNext(ss).lexeme == " ");
  CHECK(lexer.TokenizeNext(ss).lexeme == " ");

  CHECK(lexer.GetTokenName(lexer.TokenizeNext(ss)) == "Mixed");
  CHECK(lexer.GetTokenName(lexer.TokenizeNext(ss)) == "Whitespace");
  CHECK(lexer.GetTokenName(lexer.TokenizeNext(ss)) == "Lower");
}

TEST_CASE("Test Lexer with ignore tokens", "[compiler]")
{
  emp::Lexer lexer;
  lexer.AddToken("Integer", "[0-9]+");
  lexer.AddToken("Float", "[0-9]*\\.[0-9]+");
  lexer.AddToken("Lower", "[a-z]+");
  lexer.AddToken("Upper", "[A-Z]+");
  lexer.AddToken("Mixed", "[a-zA-Z]+");
  lexer.IgnoreToken("Whitespace", "[ \t\n\r]");
  lexer.AddToken("Other", ".");

  std::stringstream ss;
  ss << "This is a 123 TEST.  It should also have 1. .2 123.456 789 FLOATING point NUMbers!";

  size_t line_num;
  CHECK(lexer.TokenizeNext(ss, line_num).lexeme == "This");
  CHECK(lexer.TokenizeNext(ss, line_num).lexeme == "is");
  CHECK(lexer.TokenizeNext(ss, line_num).lexeme == "a");
  CHECK(lexer.TokenizeNext(ss, line_num).lexeme == "123");
  CHECK(lexer.TokenizeNext(ss, line_num).lexeme == "TEST");
  CHECK(lexer.TokenizeNext(ss, line_num).lexeme == ".");

  CHECK(lexer.GetTokenName(lexer.TokenizeNext(ss, line_num)) == "Mixed");    // It
  CHECK(lexer.GetTokenName(lexer.TokenizeNext(ss, line_num)) == "Lower");    // should
  CHECK(lexer.GetTokenName(lexer.TokenizeNext(ss, line_num)) == "Lower");    // also
  CHECK(lexer.GetTokenName(lexer.TokenizeNext(ss, line_num)) == "Lower");    // have
  CHECK(lexer.GetTokenName(lexer.TokenizeNext(ss, line_num)) == "Integer");  // 1
  CHECK(lexer.GetTokenName(lexer.TokenizeNext(ss, line_num)) == "Other");    // .
  CHECK(lexer.GetTokenName(lexer.TokenizeNext(ss, line_num)) == "Float");    // .2
  CHECK(lexer.GetTokenName(lexer.TokenizeNext(ss, line_num)) == "Float");    // 123.456
  CHECK(lexer.GetTokenName(lexer.TokenizeNext(ss, line_num)) == "Integer");  // 789

  CHECK(lexer.GetLexeme() == "789");

  CHECK(lexer.GetTokenName(lexer.TokenizeNext(ss, line_num)) == "Upper");    // FLOATING
  CHECK(lexer.GetTokenName(lexer.TokenizeNext(ss, line_num)) == "Lower");    // point
  CHECK(lexer.GetTokenName(lexer.TokenizeNext(ss, line_num)) == "Mixed");    // NUMbers
  CHECK(lexer.GetTokenName(lexer.TokenizeNext(ss, line_num)) == "Other");    // !
}

TEST_CASE("Full tokinization of an entire stream", "[compiler]")
{
  emp::Lexer lexer;
  lexer.AddToken("Integer", "\\d+");
  lexer.AddToken("ID", "[a-zA-Z_]\\w*");
  lexer.IgnoreToken("Whitespace", "[ \t\n\r]");
  lexer.AddToken("Other", ".");

  std::string input("ABC abc 123 aB3.");

  auto tokens = lexer.Tokenize(input);

  CHECK(tokens.size() == 5);

  std::stringstream ss;
  ss << input;

  tokens = lexer.Tokenize(ss);

  CHECK(tokens.size() == 5);
}
