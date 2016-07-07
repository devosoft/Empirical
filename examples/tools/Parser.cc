//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::Parser

#include <iostream>

#include "../../tools/Lexer.h"
#include "../../tools/Parser.h"

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
  parser("program").Rule() // empty is okay.
                   .Rule("program", "statement");
  parser("statement").Rule("expr");
  parser("expr").Rule("Integer")
                .Rule("expr", '+', "expr")
                .Rule("expr", '*', "expr")
                .Rule('(', "expr", ')');


  parser.Print();
}
