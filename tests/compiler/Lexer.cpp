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

  std::string word = "hello";
  CHECK(lx.ToToken(word).lexeme == word);
  word = "why";
  emp::Token t = lx.ToToken(word);
  CHECK(t.lexeme != word);
  CHECK((std::string)t == "w");

  std::stringstream ss;
  emp::TokenType ti = lx.GetTokenType(lx.GetTokenID("vowel"));
  ti.Print(ss);
  CHECK(ss.str() == "Name:vowel  RegEx:[a-z]*([aeiou])+[a-z]*  ID:255  save_lexeme:1  save_token:1\n");
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

  // Capture token to check both type and lexeme.
  auto t_int = lexer.TokenizeNext(ss, line_num);
  CHECK(lexer.GetTokenName(t_int) == "Integer");  // 789
  CHECK(t_int.lexeme == "789");

  CHECK(lexer.GetTokenName(lexer.TokenizeNext(ss, line_num)) == "Upper");    // FLOATING
  CHECK(lexer.GetTokenName(lexer.TokenizeNext(ss, line_num)) == "Lower");    // point
  CHECK(lexer.GetTokenName(lexer.TokenizeNext(ss, line_num)) == "Mixed");    // NUMbers
  CHECK(lexer.GetTokenName(lexer.TokenizeNext(ss, line_num)) == "Other");    // !
}

TEST_CASE("Full tokenization of an entire stream", "[compiler]")
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

TEST_CASE("Lexer Reset", "[compiler]")
{
  emp::Lexer lexer;
  lexer.AddToken("A", "a+");
  lexer.AddToken("B", "b+");
  CHECK(lexer.GetNumTokens() == 2);
  CHECK(lexer.GetTokenID("A") == 255);
  CHECK(lexer.GetTokenID("B") == 254);

  lexer.Reset();
  CHECK(lexer.GetNumTokens() == 0);

  // After reset, IDs restart from MAX_ID.
  lexer.AddToken("X", "x+");
  CHECK(lexer.GetTokenID("X") == 255);
  CHECK(lexer.GetTokenName(255) == "X");
}

TEST_CASE("Lexer GetTokenName edge cases", "[compiler]")
{
  emp::Lexer lexer;
  lexer.AddToken("Alpha", "[a-z]+");

  CHECK(lexer.GetTokenName(-1) == "ERROR");
  CHECK(lexer.GetTokenName(0)  == "EOF");
  CHECK(lexer.GetTokenName('A') == "'A'");
  CHECK(lexer.GetTokenName('!') == "'!'");
  CHECK(lexer.GetTokenName(255) == "Alpha");
}

TEST_CASE("Lexer GetTokenID edge cases", "[compiler]")
{
  emp::Lexer lexer;
  lexer.AddToken("Word", "[a-z]+");

  CHECK(lexer.GetTokenID("Word") == 255);
  // Single-char name not in map → returns ASCII value as default.
  CHECK(lexer.GetTokenID(".") == '.');
  // Multi-char unknown name → ERROR_ID.
  CHECK(lexer.GetTokenID("Unknown") == -1);
}

TEST_CASE("Lexer GetSaveToken", "[compiler]")
{
  emp::Lexer lexer;
  int id_saved   = lexer.AddToken("Saved", "a+");
  int id_ignored = lexer.IgnoreToken("Ignored", "b+");

  CHECK(lexer.GetSaveToken(id_saved)   == true);
  CHECK(lexer.GetSaveToken(id_ignored) == false);

  // Error and EOF tokens are always treated as saved.
  CHECK(lexer.GetSaveToken(-1) == true);
  CHECK(lexer.GetSaveToken(0)  == true);
}

TEST_CASE("Lexer unrecognized character fallback", "[compiler]")
{
  emp::Lexer lexer;
  lexer.AddToken("Word", "[a-z]+");

  // A '!' is not matched by any rule; lexer returns it as a token with id = '!'.
  emp::Token t = lexer.ToToken("!");
  CHECK(t.id == '!');
  CHECK(t.lexeme == "!");
  CHECK(lexer.GetTokenName('!') == "'!'");

  // EOF on empty input.
  CHECK(lexer.ToToken("").id == 0);
}

TEST_CASE("Lexer line tracking with string_view", "[compiler]")
{
  emp::Lexer lexer;
  lexer.AddToken("Word", "[a-z]+");
  lexer.IgnoreToken("WS", "[ \t\n]+");

  std::string input = "one\ntwo\nthree";
  size_t start_pos  = 0;
  size_t cur_line   = 1;

  emp::Token t1 = lexer.TokenizeNext(input, cur_line, start_pos);
  CHECK(t1.lexeme  == "one");
  CHECK(t1.line_id == 1);

  emp::Token t2 = lexer.TokenizeNext(input, cur_line, start_pos);
  CHECK(t2.lexeme  == "two");
  CHECK(t2.line_id == 2);

  emp::Token t3 = lexer.TokenizeNext(input, cur_line, start_pos);
  CHECK(t3.lexeme  == "three");
  CHECK(t3.line_id == 3);
}

TEST_CASE("Lexer keep_all includes ignored tokens", "[compiler]")
{
  emp::Lexer lexer;
  lexer.AddToken("Word", "[a-z]+");
  lexer.IgnoreToken("Space", " +");

  std::string input = "a b c";

  // Default: spaces skipped.
  auto tokens_filtered = lexer.Tokenize(input);
  CHECK(tokens_filtered.size() == 3);

  // keep_all: spaces included.
  auto tokens_all = lexer.Tokenize(input, "test", true);
  CHECK(tokens_all.size() == 5);
  CHECK(tokens_all[0].lexeme == "a");
  CHECK(tokens_all[1].lexeme == " ");
  CHECK(tokens_all[2].lexeme == "b");
}

TEST_CASE("Lexer Tokenize vector of strings", "[compiler]")
{
  emp::Lexer lexer;
  lexer.AddToken("Word", "[a-z]+");
  lexer.IgnoreToken("WS", "[ \t\n]+");

  emp::vector<emp::String> lines = {"hello", "world"};
  auto tokens = lexer.Tokenize(lines);

  CHECK(tokens.size() == 2);
  CHECK(tokens[0].lexeme == "hello");
  CHECK(tokens[1].lexeme == "world");
}

// Local settings for Empecable file checker.
// empecable_words: mbers lxr t_int
