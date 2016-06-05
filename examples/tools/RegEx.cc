//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::RegEx

#include <iostream>

#include "../../tools/RegEx.h"

int main()
{
  std::cout << "Ping!" << std::endl;

  emp::RegEx re1("a|bcdef");
  re1.PrintNotes();
  re1.PrintInternal();

  emp::RegEx re2("#[abcdefghijklm]*abc");
  re2.PrintNotes();
  re2.PrintInternal();

  emp::RegEx re3("xx(y|z*?)+xx");
  re3.PrintNotes();
  re3.PrintInternal();

  std::cout << std::endl;

  emp::RegEx re_WHITESPACE("[ \t\r]");
  std::cout << "re_WHITESPACE" << std::endl;
  re_WHITESPACE.PrintNotes();
  re_WHITESPACE.PrintInternal();
  emp::RegEx re_COMMENT("#.*");
  std::cout << "re_COMMENT" << std::endl;
  re_COMMENT.PrintNotes();
  re_COMMENT.PrintInternal();
  emp::RegEx re_INT_LIT("[0-9]+");
  std::cout << "re_INT_LIT" << std::endl;
  re_INT_LIT.PrintNotes();
  re_INT_LIT.PrintInternal();
  emp::RegEx re_FLOAT_LIT("[0-9]+[.]'[0-9]+");
  std::cout << "re_FLOAT_LIT" << std::endl;
  re_FLOAT_LIT.PrintNotes();
  re_FLOAT_LIT.PrintInternal();
  emp::RegEx re_CHAR_LIT("'(.|(\\\\[\\\\'nt]))'");
  std::cout << "re_CHAR_LIT" << std::endl;
  re_CHAR_LIT.PrintNotes();
  re_CHAR_LIT.PrintInternal();
  emp::RegEx re_STRING_LIT("[\"](\\[nt\"\\]|[^\\\"])*\"");
  std::cout << "re_STRING_LIT" << std::endl;
  re_STRING_LIT.PrintNotes();
  re_STRING_LIT.PrintInternal();
  emp::RegEx re_ID("[a-zA-Z0-9_]+");
  std::cout << "re_ID" << std::endl;
  re_ID.PrintNotes();
  re_ID.PrintInternal();
}
