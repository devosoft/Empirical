//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::RegEx

#include <iostream>

#include "tools/RegEx.h"

int main()
{
  std::cout << "Ping!" << std::endl;

  emp::RegEx reA("a|bcdef");
  reA.PrintDebug();

  emp::RegEx reB("#[abcdefghijklm]*abc");
  reB.PrintDebug();

  emp::RegEx reC("xx(y|z*?)+xx");
  reC.PrintDebug();

  std::cout << std::endl;

  emp::RegEx re_WHITESPACE("[ \t\r]");
  std::cout << "re_WHITESPACE" << std::endl;
  re_WHITESPACE.PrintDebug();
  std::cout << std::endl;

  emp::RegEx re_COMMENT("#.*");
  std::cout << "re_COMMENT" << std::endl;
  re_COMMENT.PrintDebug();
  std::cout << std::endl;

  emp::RegEx re_INT_LIT("[0-9]+");
  std::cout << "re_INT_LIT" << std::endl;
  re_INT_LIT.PrintDebug();
  std::cout << std::endl;

  emp::RegEx re_FLOAT_LIT("[0-9]+[.][0-9]+");
  std::cout << "re_FLOAT_LIT" << std::endl;
  re_FLOAT_LIT.PrintDebug();
  std::cout << std::endl;

  emp::RegEx re_CHAR_LIT("'(.|(\\\\[\\\\'nt]))'");
  std::cout << "re_CHAR_LIT" << std::endl;
  re_CHAR_LIT.PrintDebug();
  std::cout << std::endl;

  //emp::RegEx re_STRING_LIT("[\"](\\[nt\"\\]|[^\\\"])*\"");
  emp::RegEx re_STRING_LIT("[\"]((\\\\[nt\"\\\\])|[^\"])*\\\"");
  std::cout << "re_STRING_LIT" << std::endl;
  re_STRING_LIT.PrintDebug();
  std::cout << std::endl;

  emp::RegEx re_ID("[a-zA-Z0-9_]+");
  std::cout << "re_ID" << std::endl;
  re_ID.PrintDebug();
  std::cout << std::endl;


  emp::RegEx re1("a|bcdef");
  emp_assert(re1.Test("a") == true);
  emp_assert(re1.Test("bc") == false);
  emp_assert(re1.Test("bcdef") == true);
  emp_assert(re1.Test("bcdefg") == false);

  emp::RegEx re2("#[abcdefghijklm]*abc");
  emp_assert(re2.Test("") == false);
  emp_assert(re2.Test("#a") == false);
  emp_assert(re2.Test("#aaaabc") == true);
  emp_assert(re2.Test("#abcabc") == true);
  emp_assert(re2.Test("#abcabcd") == false);

  emp::RegEx re3("xx(y|(z*)?)+xx");
  emp_assert(re3.Test("xxxx") == true);
  emp_assert(re3.Test("xxxxx") == false);
  emp_assert(re3.Test("xxyxx") == true);
  emp_assert(re3.Test("xxyyxx") == true);
  emp_assert(re3.Test("xxzzzxx") == true);

  emp_assert(re_INT_LIT.Test("1234") == true);
  emp_assert(re_FLOAT_LIT.Test("1234") == false);
  emp_assert(re_ID.Test("1234") == true);
  emp_assert(re_INT_LIT.Test("1234.56") == false);
  emp_assert(re_FLOAT_LIT.Test("1234.56") == true);
  emp_assert(re_ID.Test("1234.56") == false);

  std::string test_str = "\"1234\"";
  emp_assert(re_STRING_LIT.Test(test_str) == true);
  emp_assert(re_INT_LIT.Test(test_str) == false);

  std::string test_str2 = "\"1234\", \"5678\"";
  emp_assert(re_STRING_LIT.Test(test_str2) == false);
}
