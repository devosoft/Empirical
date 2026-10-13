/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University; MIT license; see doc/LICENSE.md
 * @brief Optional token defaults in runtime parsers.
 */
#include <sstream>
#include <stdexcept>

#include "third-party/Catch/single_include/catch2/catch.hpp"
#include "emp/compiler/Parser.hpp"

namespace {
// notify::Error terminates even if a handler returns true. Throw so invalid
// grammars can be checked, then restore the original handlers after each test.
struct ParserErrorScope {
  emp::notify::HandlerSet saved = emp::notify::ErrorHandlers();
  ParserErrorScope() {
    emp::notify::AddHandler("ERROR", [](emp::notify::id_arg_t, emp::notify::message_arg_t msg) -> bool {
      throw std::runtime_error(msg);
    });
  }
  ~ParserErrorScope() { emp::notify::ErrorHandlers() = saved; }
};
}

TEST_CASE("Parser inserts default tokens without consuming input", "[compiler][parser]") {
  ParserErrorScope errors;
  emp::Lexer lexer;
  lexer.IgnoreToken("SPACE", "[ \\t\\n]+");
  const int integer = lexer.AddToken("INTEGER", "[0-9]+");
  emp::Parser parser(lexer);
  parser.AddRule("term", "INTEGER?:1 'd' INTEGER", [](auto & node) {
    return node.GetChildLexeme(0).AsDouble() * node.GetChildLexeme(2).AsDouble();
  });
  parser.AddRule("term", "INTEGER");
  REQUIRE(parser.Validate());

  auto tokens = lexer.Tokenize("\nd6");
  auto ast = parser.Parse(tokens);
  REQUIRE(ast);
  REQUIRE(ast.GetRoot().NumChildren() == 3);
  const auto & count = ast.GetRoot().GetChild(0).GetToken();
  CHECK(count.id == integer);
  CHECK(count.lexeme == "1");
  CHECK(count.line_id == 0);
  CHECK(ast.GetRoot().GetChild(1).GetToken().id == 'd');
  CHECK(ast.GetRoot().GetChild(2).GetLexeme() == "6");
  CHECK(ast.Run() == 6.0);
  CHECK(tokens.size() == 2);  // Insertion changes only the AST.

  auto explicit_count = parser.Parse("3d6");
  REQUIRE(explicit_count);
  CHECK(explicit_count.GetRoot().GetChild(0).GetLexeme() == "3");
  CHECK(explicit_count.GetRoot().GetChild(0).GetToken().line_id == 1);
  CHECK(explicit_count.Run() == 18.0);

  auto number = parser.Parse("42");  // First alternative fails after consuming 42.
  REQUIRE(number);
  CHECK(number.GetRoot().NumChildren() == 1);
  CHECK(number.GetRoot().GetChild(0).GetLexeme() == "42");
  CHECK_THROWS(parser.Parse("d"));
  CHECK_THROWS(parser.Parse("d6!"));

  std::ostringstream printed;
  parser.Print(printed);
  CHECK(printed.str().find("INTEGER?:\"1\"") != std::string::npos);
}

TEST_CASE("Defaults at EOF, inside groups, and on failed alternatives", "[compiler][parser]") {
  ParserErrorScope errors;
  emp::Lexer lexer;
  lexer.AddToken("INTEGER", "[0-9]+");
  emp::Parser parser(lexer);
  parser.AddRule("root", "(INTEGER?:1 'x') | INTEGER?:2");
  auto empty = parser.Parse("");  // Discard the synthetic 1 when 'x' fails.
  REQUIRE(empty);
  REQUIRE(empty.GetRoot().NumChildren() == 1);
  CHECK(empty.GetRoot().GetChild(0).GetLexeme() == "2");
  auto grouped = parser.Parse("x");
  REQUIRE(grouped);
  CHECK(grouped.GetRoot().NumChildren() == 2);
  CHECK(grouped.GetRoot().GetChild(0).GetLexeme() == "1");

  parser.Reset();
  parser.AddRule("root", "INTEGER?");
  auto optional = parser.Parse("");
  REQUIRE(optional);
  CHECK(optional.GetRoot().NumChildren() == 0);  // Existing ? semantics are unchanged.
}

TEST_CASE("Quoted default lexemes decode escapes and print safely", "[compiler][parser]") {
  ParserErrorScope errors;
  emp::Lexer lexer;
  lexer.AddToken("TEXT", "(.|\\n|\\r)+");
  emp::Parser parser(lexer);
  parser.AddRule("root", R"(TEXT?:"a\"b\\c\t\n\r |()?:+")");
  auto ast = parser.Parse("");
  REQUIRE(ast);
  CHECK(ast.GetRoot().GetChild(0).GetLexeme() == "a\"b\\c\t\n\r |()?:+");
  std::ostringstream printed;
  parser.Print(printed);
  CHECK(printed.str().find(R"(\t\n\r)") != std::string::npos);
}

TEST_CASE("Parser rejects malformed or unsuitable token defaults", "[compiler][parser]") {
  ParserErrorScope errors;
  emp::Lexer lexer;
  lexer.IgnoreToken("SPACE", "[ ]+");
  lexer.AddToken("INTEGER", "[0-9]+");
  lexer.AddToken("KEYWORD", "if");
  lexer.AddToken("WORD", "[a-z]+");
  for (const auto & pattern : {
    "INTEGER?:", "INTEGER?: 1", "INTEGER?:\"\"", "INTEGER?:\"1", "INTEGER?:\"\\q\"",
    "INTEGER:1", "INTEGER*:1", "INTEGER+:1", "'d'?:d", "(INTEGER)?:1", "child?:1",
    "INTEGER?:abc", "INTEGER?:1x", "INTEGER?:\"1 2\"", "WORD?:if", "SPACE?:\" \"",
    "(INTEGER?:1)*", "(INTEGER?:1)+"
  }) {
    INFO(pattern);
    emp::Parser parser(lexer);
    CHECK_THROWS([&]() { parser.AddRule("root", pattern); parser.Validate(); }());
  }
}

TEST_CASE("AST suppression consumes input and discards complete subtrees", "[compiler][parser]") {
  ParserErrorScope errors;
  emp::Lexer lexer;
  lexer.IgnoreToken("SPACE", "[ ]+");
  lexer.AddToken("INTEGER", "[0-9]+");
  emp::Parser parser(lexer);
  parser.AddRule("root", "-'{' -discarded+ -('x' 'y') -INTEGER?:1 kept -'}'", nullptr, "", false, "block");
  parser.AddRule("discarded", "INTEGER");
  parser.AddRule("kept", "'z'");
  auto ast = parser.Parse("{1 2 xyz}");
  REQUIRE(ast.HasRoot());
  CHECK(ast.GetRoot().note == "block");
  REQUIRE(ast.GetRoot().NumChildren() == 1);
  CHECK(parser.GetRuleName(ast.GetRoot().GetChild(0).GetRuleID()) == "kept");
  CHECK(ast.GetRoot().GetChild(0).GetChildLexeme(0) == "z");
  CHECK_THROWS(parser.Parse("{1 2 xz}"));
  CHECK_FALSE(parser.LastParseSucceeded());
}

TEST_CASE("No AST promotes children and permits successful empty trees", "[compiler][parser]") {
  ParserErrorScope errors;
  emp::Lexer lexer;
  lexer.AddToken("INTEGER", "[0-9]+");
  emp::Parser parser(lexer);
  parser.AddRule("start", "wrapped | empty", nullptr, "", true);
  parser.AddRule("wrapped", "-'(' inner -')'", nullptr, "", true);
  parser.AddRule("inner", "INTEGER", nullptr, "", false, "number");
  parser.AddRule("empty", "-'!'", nullptr, "", true);
  auto ast = parser.Parse("(123)");
  REQUIRE(ast.HasRoot());
  CHECK(parser.LastParseSucceeded());
  CHECK(parser.GetRuleName(ast.GetRoot().GetRuleID()) == "inner");
  CHECK(ast.GetRoot().note == "number");
  CHECK(ast.GetRoot().GetChildLexeme(0) == "123");
  auto empty = parser.Parse("!");
  CHECK(empty);
  CHECK_FALSE(empty.HasRoot());
  CHECK(parser.LastParseSucceeded());
  std::ostringstream printed;
  parser.PrintAST(empty, printed);
  CHECK(printed.str().find("successful parse; no AST") != std::string::npos);
  CHECK_THROWS(parser.Parse("!1"));
  CHECK_FALSE(parser.LastParseSucceeded());

  parser.Reset();
  parser.AddRule("start", "INTEGER?", nullptr, "", true);
  auto leaf = parser.Parse("7");
  REQUIRE(leaf.HasRoot());
  CHECK(leaf.GetRoot().IsLeaf());
  CHECK(leaf.GetRoot().GetLexeme() == "7");
  CHECK_FALSE(parser.Parse("").HasRoot());
}

TEST_CASE("No AST validates maximum retained children through groups and rules", "[compiler][parser]") {
  ParserErrorScope errors;
  emp::Lexer lexer;
  lexer.IgnoreToken("SPACE", "[ ]+");
  lexer.AddToken("INTEGER", "[0-9]+");
  for (const auto & pattern : {"INTEGER INTEGER?", "INTEGER+", "(INTEGER 'x')?", "child*", "child INTEGER", "recursive"}) {
    INFO(pattern);
    emp::Parser parser(lexer);
    parser.AddRule("start", pattern, nullptr, "", true);
    parser.AddRule("child", "INTEGER", nullptr, "", true);
    if (std::string(pattern) == "recursive") {
      parser.AddRule("recursive", "-'(' recursive -')' INTEGER? | INTEGER", nullptr, "", true);
    }
    CHECK_THROWS(parser.Validate());
  }
  for (bool with_note : {false, true}) {
    emp::Parser parser(lexer);
    parser.AddRule("start", "INTEGER", nullptr, with_note ? "" : "Number", true, with_note ? "note" : "");
    CHECK_THROWS(parser.Validate());
  }
  emp::Parser parser(lexer);
  parser.AddRule("start", "empty* (INTEGER | leaf)?", nullptr, "", true);
  parser.AddRule("empty", "-'!'", nullptr, "", true);
  parser.AddRule("leaf", "-'(' start -')'", nullptr, "", true);
  REQUIRE(parser.Validate());
  CHECK_FALSE(parser.Parse("!!!").HasRoot());
  CHECK(parser.Parse("!!!(1)").GetRoot().GetLexeme() == "1");
}

TEST_CASE("Pruning preserves ordered alternatives, notes, and literal minus", "[compiler][parser]") {
  ParserErrorScope errors;
  emp::Lexer lexer;
  lexer.AddToken("INTEGER", "[0-9]+");
  emp::Parser parser(lexer);
  parser.AddRule("start", "INTEGER '-' INTEGER", nullptr, "", false, "subtract");
  parser.AddRule("start", "INTEGER", nullptr, "", true);
  auto single = parser.Parse("1");
  REQUIRE(single.HasRoot());
  CHECK(single.GetRoot().IsLeaf());
  auto sum = parser.Parse("1-2");
  REQUIRE(sum.HasRoot());
  CHECK(sum.GetRoot().note == "subtract");
  CHECK(sum.GetRoot().NumChildren() == 3);
  CHECK(sum.GetRoot().GetChildLexeme(1) == "-");
}
