//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::Parser

#include <iostream>

#include "tools/Lexer.h"
#include "tools/Parser.h"

int main()
{
  emp::Lexer lexer;
  lexer.AddToken("Integer", "[0-9]+");
  lexer.AddToken("Float", "[0-9]*\\.[0-9]+");
  lexer.AddToken("Lower", "[a-z]+");
  lexer.AddToken("Upper", "[A-Z]+");
  lexer.AddToken("Mixed", "[a-zA-Z]+");
  lexer.AddToken("Whitespace", "[ \t\n\r]");
  lexer.AddToken("Other", ".");

  emp::Parser parser(lexer);
  parser("program").Rule("statement_list");
  parser("statement_list").Rule() // empty is okay.
                         .Rule("statement_list", "statement");
  parser("statement").Rule("expr", ';');
  parser("expr").Rule("Integer")
                .Rule("expr", '+', "expr")
                .Rule("expr", '*', "expr")
                .Rule('(', "expr", ')');

  std::stringstream ss;
  ss << "This is a 123 TEST.  It should also have 1. .2 123.456 789 FLOATING point NUMbers!";
  parser.Process(ss);

  parser.Print();
}
