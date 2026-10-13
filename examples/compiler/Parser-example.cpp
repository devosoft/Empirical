/**
 * Example usage of emp::Parser: a small calculator, both parsed dynamically and
 * output as standalone C++ code.
 *
 * Compile:  g++ -std=c++23 -I include -I third-party/cereal/include Parser-example.cpp
 * Then compile the generated file's driver (see generated_calc_driver.cpp):
 *           g++ -std=c++23 generated_calc_driver.cpp
 */

#include <cmath>
#include <fstream>
#include <iostream>

#include "emp/compiler/Lexer.hpp"
#include "emp/compiler/Parser.hpp"
#include "emp/io/CPPFile.hpp"

int main() {
  // === Build a calculator lexer ===
  emp::Lexer lexer;
  lexer.AddToken("NUMBER", "[0-9]+(\\.[0-9]+)?");
  lexer.AddToken("ID", "[a-zA-Z_][a-zA-Z0-9_]*");
  lexer.IgnoreToken("WS", "[ \t\n]+");

  // === Build a calculator grammar with precedence ===
  //   expr   ::= term (('+' | '-') term)*
  //   term   ::= factor (('*' | '/') factor)*
  //   factor ::= NUMBER | '(' expr ')' | '-' factor
  //
  // Each AddRule() call provides BOTH a run-time action (a lambda called when the AST
  // node is Run) and a function name (called by the generated C++ code's Run).
  using node_t = emp::Parser::node_t;   // emp::Parser = Parser_Base<double>
  emp::Parser parser(lexer);

  parser.AddRule("expr", "term (('+' | '-') term)*",
    [](node_t & node) {
      double out = node.RunChild(0);
      for (size_t i = 1; i < node.NumChildren(); i += 2) {
        if (node.GetChild(i).GetToken().id == '+') out += node.RunChild(i+1);
        else out -= node.RunChild(i+1);
      }
      return out;
    }, "DoExpr");

  parser.AddRule("term", "factor (('*' | '/') factor)*",
    [](node_t & node) {
      double out = node.RunChild(0);
      for (size_t i = 1; i < node.NumChildren(); i += 2) {
        if (node.GetChild(i).GetToken().id == '*') out *= node.RunChild(i+1);
        else out /= node.RunChild(i+1);
      }
      return out;
    }, "DoTerm");

  parser.AddRule("factor", "NUMBER",
    [](node_t & node) { return node.GetChildLexeme(0).AsDouble(); }, "DoNumber");
  parser.AddRule("factor", "'(' expr ')'",
    [](node_t & node) { return node.RunChild(1); }, "DoParen");
  parser.AddRule("factor", "'-' factor",
    [](node_t & node) { return -node.RunChild(1); }, "DoNegate");

  parser.Print();  // Show the grammar.
  std::cout << std::endl;

  // === Dynamic parsing ===
  for (emp::String input : { "1+2*3", "(1+2)*3", "-(3+4)*-2", "2*(3+(4-1)*5)/9" }) {
    emp::AST<double> ast = parser.Parse(input);
    if (ast) { std::cout << input << " = " << ast.Run() << "\n"; }
  }

  // Inspect an AST directly.
  std::cout << "\nAST for \"1+2*3\":\n";
  auto ast = parser.Parse("1+2*3");
  parser.PrintAST(ast);

  // === Code generation ===
  // Write the lexer and parser into the same CPPFile so token IDs line up.
  emp::CPPFile file("generated_calc.hpp");
  file.SetGuards("GENERATED_CALC_HPP");
  lexer.WriteCPP(file);
  parser.WriteCPP(file);
  std::ofstream out_file("generated_calc.hpp");
  file.Write(out_file);
  std::cout << "\nStandalone lexer+parser written to generated_calc.hpp\n";
}
