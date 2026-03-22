/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2021-2025
*/
/**
 *  @file
 */

#include <iostream>
#include <span>
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
  std::string reS = re.ToString();
  CHECK(reS == "1|2");

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
  CHECK( parens3.Test("a(a(a(a(a)a)a)a)a") == false );       // Too deep... with other characters
  CHECK( parens3.Test("((()())(()()))((()())(()()))") == true ); // Multi parens at each level

  emp::NFA nfa = emp::to_NFA(parens3);
  emp::DFA dfa = emp::to_DFA(parens3);

  CHECK( nfa.GetSize() <= 44 );
  CHECK( dfa.GetSize() <= 10 );
}

TEST_CASE("RegEx: quantifiers + ? . {n} {n,} {m,n}", "[compiler]")
{
  // + (one or more)
  emp::RegEx re_plus("a+");
  CHECK(re_plus.Test("a")    == true);
  CHECK(re_plus.Test("aaa")  == true);
  CHECK(re_plus.Test("")     == false);
  CHECK(re_plus.Test("b")    == false);
  CHECK(re_plus.Test("ab")   == false);  // extra char not allowed

  // ? (zero or one)
  emp::RegEx re_opt("ab?c");
  CHECK(re_opt.Test("ac")    == true);   // b omitted
  CHECK(re_opt.Test("abc")   == true);   // b present
  CHECK(re_opt.Test("abbc")  == false);  // two b's
  CHECK(re_opt.Test("bc")    == false);  // missing a

  // . (any char except newline)
  emp::RegEx re_dot("a.b");
  CHECK(re_dot.Test("axb")    == true);
  CHECK(re_dot.Test("a1b")    == true);
  CHECK(re_dot.Test("a b")    == true);
  CHECK(re_dot.Test("a\nb")   == false); // newline excluded
  CHECK(re_dot.Test("ab")     == false); // middle char required
  CHECK(re_dot.Test("axxb")   == false); // only one middle char

  // {n} exact repeat
  emp::RegEx re_exact("a{3}");
  CHECK(re_exact.Test("aaa")   == true);
  CHECK(re_exact.Test("aa")    == false);
  CHECK(re_exact.Test("aaaa")  == false);

  // {0} — zero-times repeat matches only empty string
  emp::RegEx re_zero("a{0}b");
  CHECK(re_zero.Test("b")   == true);
  CHECK(re_zero.Test("ab")  == false);
  CHECK(re_zero.Test("")    == false);  // 'b' still required

  // {n,} at-least-n repeat
  emp::RegEx re_atleast("a{2,}");
  CHECK(re_atleast.Test("a")     == false);
  CHECK(re_atleast.Test("aa")    == true);
  CHECK(re_atleast.Test("aaaa")  == true);

  // {m,n} bounded repeat
  emp::RegEx re_range("a{2,4}");
  CHECK(re_range.Test("a")      == false);
  CHECK(re_range.Test("aa")     == true);
  CHECK(re_range.Test("aaa")    == true);
  CHECK(re_range.Test("aaaa")   == true);
  CHECK(re_range.Test("aaaaa")  == false);

  // {1,1} same as no quantifier
  emp::RegEx re_one("a{1,1}");
  CHECK(re_one.Test("a")   == true);
  CHECK(re_one.Test("aa")  == false);
  CHECK(re_one.Test("")    == false);
}

TEST_CASE("RegEx: character class shorthands \\d \\l \\s \\w", "[compiler]")
{
  // \d  digit,  \D  non-digit
  emp::RegEx re_d(R"(\d+)");
  CHECK(re_d.Test("0")      == true);
  CHECK(re_d.Test("9")      == true);
  CHECK(re_d.Test("12345")  == true);
  CHECK(re_d.Test("a")      == false);
  CHECK(re_d.Test("1a")     == false);

  emp::RegEx re_D(R"(\D+)");
  CHECK(re_D.Test("abc")   == true);
  CHECK(re_D.Test("!")      == true);
  CHECK(re_D.Test("0")      == false);
  CHECK(re_D.Test("a1")     == false);

  // \l  letter,  \L  non-letter
  emp::RegEx re_l(R"(\l+)");
  CHECK(re_l.Test("abc")   == true);
  CHECK(re_l.Test("ABC")   == true);
  CHECK(re_l.Test("1")     == false);
  CHECK(re_l.Test("a1")    == false);

  emp::RegEx re_L(R"(\L)");
  CHECK(re_L.Test("1")     == true);
  CHECK(re_L.Test(" ")     == true);
  CHECK(re_L.Test("a")     == false);

  // \s  whitespace,  \S  non-whitespace
  emp::RegEx re_s(R"(\s+)");
  CHECK(re_s.Test(" ")     == true);
  CHECK(re_s.Test("\t")    == true);
  CHECK(re_s.Test("\n")    == true);
  CHECK(re_s.Test("a")     == false);

  emp::RegEx re_S(R"(\S+)");
  CHECK(re_S.Test("abc")   == true);
  CHECK(re_S.Test("a1_")   == true);
  CHECK(re_S.Test(" ")     == false);
  CHECK(re_S.Test("a b")   == false);

  // \w  word char,  \W  non-word
  emp::RegEx re_w(R"(\w+)");
  CHECK(re_w.Test("abc")     == true);
  CHECK(re_w.Test("abc123_") == true);
  CHECK(re_w.Test(" ")       == false);
  CHECK(re_w.Test(".")       == false);

  emp::RegEx re_W(R"(\W)");
  CHECK(re_W.Test(" ")  == true);
  CHECK(re_W.Test(".")  == true);
  CHECK(re_W.Test("a")  == false);
  CHECK(re_W.Test("_")  == false);
}

TEST_CASE("RegEx: character sets [...] ranges, negation, escapes", "[compiler]")
{
  // Simple set
  emp::RegEx re_set("[abc]");
  CHECK(re_set.Test("a")  == true);
  CHECK(re_set.Test("b")  == true);
  CHECK(re_set.Test("c")  == true);
  CHECK(re_set.Test("d")  == false);
  CHECK(re_set.Test("ab") == false);  // only one char

  // Negated set
  emp::RegEx re_neg("[^abc]");
  CHECK(re_neg.Test("a")  == false);
  CHECK(re_neg.Test("d")  == true);
  CHECK(re_neg.Test("1")  == true);
  CHECK(re_neg.Test("ad") == false);  // still only one char

  // Range [a-z]
  emp::RegEx re_rng("[a-z]");
  CHECK(re_rng.Test("a")  == true);
  CHECK(re_rng.Test("m")  == true);
  CHECK(re_rng.Test("z")  == true);
  CHECK(re_rng.Test("A")  == false);
  CHECK(re_rng.Test("1")  == false);

  // Negated range [^a-z]
  emp::RegEx re_nrng("[^a-z]");
  CHECK(re_nrng.Test("A")  == true);
  CHECK(re_nrng.Test("1")  == true);
  CHECK(re_nrng.Test("a")  == false);
  CHECK(re_nrng.Test("z")  == false);

  // Multiple ranges and chars combined
  emp::RegEx re_multi("[a-zA-Z0-9_]");
  CHECK(re_multi.Test("a")  == true);
  CHECK(re_multi.Test("Z")  == true);
  CHECK(re_multi.Test("5")  == true);
  CHECK(re_multi.Test("_")  == true);
  CHECK(re_multi.Test(" ")  == false);
  CHECK(re_multi.Test(".")  == false);

  // Escape sequences inside charset: \n \t
  emp::RegEx re_esc(R"([\n\t])");
  CHECK(re_esc.Test("\n")  == true);
  CHECK(re_esc.Test("\t")  == true);
  CHECK(re_esc.Test("a")   == false);
  CHECK(re_esc.Test(" ")   == false);

  // Escaped hyphen is literal, not a range: [a\-z] = {a, -, z}
  emp::RegEx re_hyph(R"([a\-z])");
  CHECK(re_hyph.Test("a")  == true);
  CHECK(re_hyph.Test("-")  == true);
  CHECK(re_hyph.Test("z")  == true);
  CHECK(re_hyph.Test("b")  == false);  // b..y not in set

  // Hyphen at start of set is also literal: [-az] = {-, a, z}
  emp::RegEx re_hyph2("[-az]");
  CHECK(re_hyph2.Test("-")  == true);
  CHECK(re_hyph2.Test("a")  == true);
  CHECK(re_hyph2.Test("z")  == true);
  CHECK(re_hyph2.Test("b")  == false);

  // ASCII decimal escape: \065 encodes decimal 65 = 'A'
  emp::RegEx re_ascii(R"([\065])");
  CHECK(re_ascii.GetNotes().empty());
  CHECK(re_ascii.Test("A") == true);
  CHECK(re_ascii.Test("B") == false);
  CHECK(re_ascii.Test("0") == false);

  // \032 = decimal 32 = space
  emp::RegEx re_sp(R"([\032])");
  CHECK(re_sp.GetNotes().empty());
  CHECK(re_sp.Test(" ")  == true);
  CHECK(re_sp.Test("a")  == false);
}

TEST_CASE("RegEx: literal strings \"...\"", "[compiler]")
{
  // Dot inside "..." is literal, not wildcard
  emp::RegEx re_dot(R"("a.b")");
  CHECK(re_dot.Test("a.b")  == true);
  CHECK(re_dot.Test("axb")  == false);  // . is not a wildcard here

  // Spaces and other special chars are literal
  emp::RegEx re_sp(R"("hello world")");
  CHECK(re_sp.Test("hello world")  == true);
  CHECK(re_sp.Test("helloworld")   == false);
  CHECK(re_sp.Test("hello.world")  == false);

  // Escaped quote inside literal string
  emp::RegEx re_q(R"("say \"hi\"")");
  CHECK(re_q.Test("say \"hi\"")  == true);
  CHECK(re_q.Test("say hi")      == false);

  // Literal string combined with a normal regex segment
  emp::RegEx re_comb(R"("int"\s+\w+)");
  CHECK(re_comb.Test("int foo")    == true);
  CHECK(re_comb.Test("int  bar")   == true);
  CHECK(re_comb.Test("intfoo")     == false);  // whitespace required
  CHECK(re_comb.Test("double foo") == false);
}

TEST_CASE("RegEx: escape sequences in main pattern", "[compiler]")
{
  // \n matches newline
  emp::RegEx re_nl(R"(a\nb)");
  CHECK(re_nl.Test("a\nb")  == true);
  CHECK(re_nl.Test("ab")    == false);
  CHECK(re_nl.Test("a b")   == false);

  // \t matches tab
  emp::RegEx re_tab(R"(a\tb)");
  CHECK(re_tab.Test("a\tb")  == true);
  CHECK(re_tab.Test("a b")   == false);

  // Escaped special chars become literals
  emp::RegEx re_edot(R"(\.)");
  CHECK(re_edot.Test(".")  == true);
  CHECK(re_edot.Test("a")  == false);

  emp::RegEx re_estar(R"(\*)");
  CHECK(re_estar.Test("*")  == true);
  CHECK(re_estar.Test("a")  == false);

  emp::RegEx re_eplus(R"(\+)");
  CHECK(re_eplus.Test("+")  == true);
  CHECK(re_eplus.Test("a")  == false);

  emp::RegEx re_epipe(R"(\|)");
  CHECK(re_epipe.Test("|")  == true);
  CHECK(re_epipe.Test("a")  == false);

  emp::RegEx re_epar(R"(\(\))");
  CHECK(re_epar.Test("()")   == true);
  CHECK(re_epar.Test("(a)")  == false);

  // \? is literal question mark
  emp::RegEx re_eq(R"(\?)");
  CHECK(re_eq.Test("?")  == true);
  CHECK(re_eq.Test("a")  == false);
}

TEST_CASE("RegEx: error detection", "[compiler]")
{
  // Unclosed bracket: [abc (missing ']')
  emp::RegEx re_unclosed("[abc");
  CHECK(!re_unclosed.GetNotes().empty());

  // Reversed range [z-a]: z > a is invalid
  emp::RegEx re_rev("[z-a]");
  CHECK(!re_rev.GetNotes().empty());

  // Quantifier at start with nothing to quantify
  emp::RegEx re_star("*abc");
  CHECK(!re_star.GetNotes().empty());

  emp::RegEx re_plus("+abc");
  CHECK(!re_plus.GetNotes().empty());

  // Trailing OR with nothing on right side
  emp::RegEx re_or("abc|");
  CHECK(!re_or.GetNotes().empty());

  // {m,n} with m > n
  emp::RegEx re_rep("a{5,3}");
  CHECK(!re_rep.GetNotes().empty());

  // Missing closing brace in repeat
  emp::RegEx re_nobrace("a{3");
  CHECK(!re_nobrace.GetNotes().empty());

  // Valid regex has empty notes
  emp::RegEx re_ok("[a-z]+");
  CHECK(re_ok.GetNotes().empty());
}

TEST_CASE("RegEx: combinations and structural coverage", "[compiler]")
{
  // Nested quantifiers
  emp::RegEx re_nest("(ab)+c*d?");
  CHECK(re_nest.Test("ab")       == true);  // +, no c, no d
  CHECK(re_nest.Test("abd")      == true);  // +, no c, with d
  CHECK(re_nest.Test("ababccd")  == true);  // ++ cc d
  CHECK(re_nest.Test("b")        == false);

  // Alternation with groups
  emp::RegEx re_or("(cat|dog)s?");
  CHECK(re_or.Test("cat")   == true);
  CHECK(re_or.Test("cats")  == true);
  CHECK(re_or.Test("dog")   == true);
  CHECK(re_or.Test("dogs")  == true);
  CHECK(re_or.Test("fish")  == false);

  // Star on charset
  emp::RegEx re_cstar("[0-9]*");
  CHECK(re_cstar.Test("")     == true);   // zero allowed
  CHECK(re_cstar.Test("123")  == true);
  CHECK(re_cstar.Test("1a2")  == false);

  // Plus on charset
  emp::RegEx re_cplus("[0-9]+");
  CHECK(re_cplus.Test("")     == false);  // at least one required
  CHECK(re_cplus.Test("0")    == true);
  CHECK(re_cplus.Test("999")  == true);

  // OR between charset and literal
  emp::RegEx re_corl("[0-9]+|[a-z]+");
  CHECK(re_corl.Test("123")   == true);
  CHECK(re_corl.Test("abc")   == true);
  CHECK(re_corl.Test("123abc") == false);  // must be all one or the other

  // Copy constructor preserves behavior
  emp::RegEx re_orig("[a-z]+");
  emp::RegEx re_copy = re_orig;
  CHECK(re_copy.Test("hello")  == true);
  CHECK(re_copy.Test("HELLO")  == false);

  // Assignment operator overwrites behavior
  emp::RegEx re_asgn("[0-9]+");
  re_asgn = re_orig;
  CHECK(re_asgn.Test("hello")  == true);
  CHECK(re_asgn.Test("123")    == false);

  // Complex: number or identifier
  emp::RegEx re_complex("[0-9]+|[a-zA-Z_][a-zA-Z0-9_]*");
  CHECK(re_complex.Test("123")     == true);
  CHECK(re_complex.Test("foo")     == true);
  CHECK(re_complex.Test("foo123")  == true);
  CHECK(re_complex.Test("_bar")    == true);
  CHECK(re_complex.Test("123abc")  == false);  // digit-start must be all digits
  CHECK(re_complex.Test("")        == false);

  // Internal representation round-trips via ToString
  emp::RegEx re_ts("ab*c");
  CHECK(re_ts.ToString() == "ab*c");

  emp::RegEx re_ts2("[0-9]+");
  CHECK(re_ts2.ToString() == "[0-9]+");
}

// Local settings for Empecable file checker.
// empecable_words: ghij defghijk xxzzzxx xxxx abcabc xxxxx xxyyxx abcabcd bcd aaaabc xxyxx bcdef abcdefghijklm bcdefg helloworld axxb nrng hyph intfoo estar ababccd cstar corl asgn
