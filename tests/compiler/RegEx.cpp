/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file RegEx.cpp
 */

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <iostream>
#include <sstream>
#include <string>

#include "emp/compiler/RegEx.hpp"

TEST_CASE("Test RegEx", "[compiler]")
{
  emp::RegEx re("1|2");
  CHECK(re.Test("1") == true);
  CHECK(re.Test("2") == true);
  CHECK(re.Test("12") == false);

  // test operator=
  emp::RegEx re0 = re;
  CHECK(re0.Test("1") == true);
  CHECK(re0.Test("2") == true);
  CHECK(re0.Test("12") == false);
  emp::RegEx re1("3|4");
  re1 = re0;
  CHECK(re1.Test("1") == true);
  CHECK(re1.Test("2") == true);
  CHECK(re1.Test("12") == false);

  // AsString
  std::string reS = re.AsString();
  CHECK(reS == "\"1|2\"");

  // test all the prints

  // Try to hit all the different structs in RegEx.h
  emp::RegEx re2("\"r.*\"");
  //re2.PrintDebug();
  //CHECK(re2.Test("\"rats\""));
}

TEST_CASE("Test regular expressions (RegEx)", "[compiler]")
{
  emp::RegEx re1("a|bcdef");
  CHECK(re1.Test("a") == true);
  CHECK(re1.Test("bc") == false);
  CHECK(re1.Test("bcdef") == true);
  CHECK(re1.Test("bcdefg") == false);

  emp::RegEx re2("#[abcdefghijklm]*abc");
  CHECK(re2.Test("") == false);
  CHECK(re2.Test("#a") == false);
  CHECK(re2.Test("#aaaabc") == true);
  CHECK(re2.Test("#abcabc") == true);
  CHECK(re2.Test("#abcabcd") == false);

  emp::RegEx re3("xx(y|(z*)?)+xx");
  CHECK(re3.Test("xxxx") == true);
  CHECK(re3.Test("xxxxx") == false);
  CHECK(re3.Test("xxyxx") == true);
  CHECK(re3.Test("xxyyxx") == true);
  CHECK(re3.Test("xxzzzxx") == true);

  emp::RegEx re_WHITESPACE("[ \t\r]");
  emp::RegEx re_COMMENT("#.*");
  emp::RegEx re_INT_LIT("[0-9]+");
  emp::RegEx re_FLOAT_LIT("[0-9]+[.][0-9]+");
  emp::RegEx re_CHAR_LIT("'(.|(\\\\[\\\\'nt]))'");
  emp::RegEx re_STRING_LIT("[\"]((\\\\[nt\"\\\\])|[^\"])*\\\"");
  emp::RegEx re_ID("[a-zA-Z0-9_]+");

  CHECK(re_INT_LIT.Test("1234") == true);
  CHECK(re_FLOAT_LIT.Test("1234") == false);
  CHECK(re_ID.Test("1234") == true);
  CHECK(re_INT_LIT.Test("1234.56") == false);
  CHECK(re_FLOAT_LIT.Test("1234.56") == true);
  CHECK(re_ID.Test("1234.56") == false);

  std::string test_str = "\"1234\"";
  CHECK(re_STRING_LIT.Test(test_str) == true);
  CHECK(re_INT_LIT.Test(test_str) == false);

  std::string test_str2 = "\"1234\", \"5678\"";
  CHECK(re_STRING_LIT.Test(test_str2) == false);
}

TEST_CASE("Test more complex regular expressions (RegEx)", "[compiler]")
{
  using namespace std::string_literals;

  const std::string no_parens = "[^()\n\r]*";
  const std::string open = "\"(\"";
  const std::string close = "\")\"";
  const std::string matched_parens = open + no_parens + close;
  const std::string multi_parens = no_parens + "("s + matched_parens + no_parens + ")*"s;
  const std::string nested_parens2 = open + multi_parens + close;
  const std::string multi_nested2 = no_parens + "("s + nested_parens2 + no_parens + ")*"s;
  const std::string nested_parens3 = open + multi_nested2 + close;
  const std::string multi_nested3 = no_parens + "("s + nested_parens3 + no_parens + ")*"s;

  emp::RegEx parens3(multi_nested3);
  CHECK( parens3.Test("abcd") == true );
  CHECK( parens3.Test("a(bcd") == false );                   // dangling "open"
  CHECK( parens3.Test("abc)d") == false );                   // dangling "close"
  CHECK( parens3.Test("a(bc)d") == true );
  CHECK( parens3.Test("a)bc(d") == false );                  // wrong order...
  CHECK( parens3.Test("a(bc)defghijk") == true );
  CHECK( parens3.Test("a(bc)def(ghij)k") == true );          // Multiple parens!
  CHECK( parens3.Test("a(bc)d(e)f(ghij)k") == true );
  CHECK( parens3.Test("a(bc)d(ef(ghij)k") == false );
  CHECK( parens3.Test("a(bc)d(ef(ghij)k)") == true );        // Nested parens!
  CHECK( parens3.Test("a(bc)d(ef(g(hi)j)k)") == true );      // Nested 3-deep parens!
  CHECK( parens3.Test("a(b((c)))d(ef(g(hi)j)k)") == true );  // More nested 3-deep parens
  CHECK( parens3.Test("(((())))") == false );                // Too deep...
  CHECK( parens3.Test("((()))((()))((()))((()))") == true ); // Lots of parens
  CHECK( parens3.Test("a(a(a(a(a)a)a)a)a") == false );       // Too deep... with other chartacters
  CHECK( parens3.Test("((()())(()()))((()())(()()))") == true ); // Multi parens at each level

  emp::NFA nfa = emp::to_NFA(parens3);
  emp::DFA dfa = emp::to_DFA(parens3);

  CHECK( nfa.GetSize() <= 44 );
  CHECK( dfa.GetSize() <= 8 );
}
