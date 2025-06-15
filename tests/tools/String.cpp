/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2023
*/
/**
 *  @file
 */

#include <iostream>
#include <string>
#include <string_view>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/tools/String.hpp"


TEST_CASE("Test String Constructors", "[tools]")
{
  const emp::String empty_str;
  CHECK(empty_str == "");

  emp::String zero = emp::MakeLiteral('\0');
  CHECK(zero != "\0");
  CHECK(zero != "\\0");
  CHECK(zero == "'\\0'");

  CHECK(emp::MakeLiteral((char)1) == "'\\001'");
  CHECK(emp::MakeLiteral((char)2) == "'\\002'");
  CHECK(emp::MakeLiteral((char)3) == "'\\003'");
  CHECK(emp::MakeLiteral((char)4) == "'\\004'");
  CHECK(emp::MakeLiteral((char)5) == "'\\005'");
  CHECK(emp::MakeLiteral((char)6) == "'\\006'");

  CHECK(emp::MakeLiteral('\'') == "'\\\''");
  CHECK(emp::MakeLiteral('\\') == "'\\\\'");

  emp::String up = "A String!";
  CHECK(up.AsUpper() == "A STRING!");
  CHECK(up.AsLower() == "a string!");

  CHECK(emp::MakeRoman(50) == "L");
  CHECK(emp::MakeRoman(562) == "DLXII");
  CHECK(emp::MakeRoman(2128) == "MMCXXVIII");
  CHECK(emp::MakeRoman(-999) == "-CMXCIX");
  CHECK(emp::MakeRoman(444) == "CDXLIV");
  CHECK(emp::MakeRoman(500000) == "D|");
  CHECK(emp::MakeRoman(500000000) == "D||");
  CHECK(emp::MakeRoman(500500500) == "D|D|D");

  emp::String input = "abc";
  emp::String transformed(input, [](char c){ return emp::String(1, std::toupper(c)); });
  CHECK(transformed == "ABC");

  std::string_view sv = "string_view test";
  emp::String from_sv(sv);
  CHECK(from_sv == "string_view test");
}

TEST_CASE("Assignment operators", "[tools]")
{
  emp::String a = "Hello";
  emp::String b;
  b = a;
  CHECK(b == "Hello");

  b = std::string("World");
  CHECK(b == "World");

  b = "C-style";
  CHECK(b == "C-style");

  b = 'X';
  CHECK(b == "X");

  b = std::string_view("from_view");
  CHECK(b == "from_view");

  b = {'a', 'b', 'c'};
  CHECK(b == "abc");
  CHECK(b != "abcd");
}

TEST_CASE("Test Accessors", "[tools]")
{
  emp::String str = "abc";
  const emp::String const_str = "ABC";

  // Check types, including with str() call
  static_assert(std::same_as<decltype(str), emp::String>);
  static_assert(std::same_as<decltype(const_str), const emp::String>);
  static_assert(std::same_as<decltype(str.str()), std::string&>);
  static_assert(std::same_as<decltype(const_str.str()), const std::string&>);

  // Check basic indexing
  CHECK(str[0] == 'a');
  CHECK(str[1] == 'b');
  CHECK(str[2] == 'c');
  CHECK(const_str[0] == 'A');
  CHECK(const_str[1] == 'B');
  CHECK(const_str[2] == 'C');

  // Check indexing to change values.
  str[1] = 'd';
  CHECK(str[0] == 'a');
  CHECK(str[1] == 'd');
  CHECK(str[2] == 'c');

  // Check front / back
  CHECK(str.front() == 'a');
  CHECK(str.back() == 'c');
  CHECK(const_str.front() == 'A');
  CHECK(const_str.back() == 'C');

  // Make sure front and back are modifiable.
  str.front() = 'e';
  str.back() = 'f';
  CHECK(str[0] == 'e');
  CHECK(str[1] == 'd');
  CHECK(str[2] == 'f');

  // Make sure still works with a new and longer string.
  str = "ghijkl";
  CHECK(str.front() == 'g');
  CHECK(str.back() == 'l');

  // Get should work like indexing, but allow out-of-range access (returning '\0')
  CHECK(str.Get(0) == 'g');
  CHECK(str.Get(2) == 'i');
  CHECK(str.Get(5) == 'l');
  CHECK(str.Get(6) == '\0');
  CHECK(str.Get(10) == '\0');
  CHECK(str.Get(emp::String::npos) == '\0');

  // Get should allow setting.
  str.Get(2) = 'm';
  CHECK(str == "ghmjkl");

  // Setting non-existant character with Get() should have no effect.
  str.Get(6) = 'n';
  CHECK(str == "ghmjkl");
  CHECK(str.Get(6) == '\0');


}

TEST_CASE("Test String Composition-ID Functions", "[tools]")
{
  emp::String abc = "aabcccabbcccabcbca";
  CHECK(abc.IsComposedOf("abc"));
  CHECK(!abc.IsComposedOf("abx"));

  CHECK(emp::String("This string has lots of space!\n").HasWhitespace());
  CHECK(emp::String("abcdefghijklm\nopqrstuvwxyz").HasWhitespace());
  CHECK(!emp::String("none").HasWhitespace());

  CHECK(emp::String("--Uppercase Letters--").HasUpper());
  CHECK(!emp::String("lowercase...").HasUpper());
  CHECK(emp::String("~Lots of Lowercase~").HasLower());
  CHECK(!emp::String("ALL UPPERCASE!!").HasLower());

  CHECK(emp::String(",./';'[]-6q82348962").HasLetter());
  CHECK(!emp::String("1-2-3-4-5").HasLetter());
  CHECK(emp::String("!@#$%^&*()0987654321").HasDigit());
  CHECK(!emp::String("!@#$%^&*()abcdefg").HasDigit());

  CHECK(emp::String("all letters").HasAlphanumeric());
  CHECK(emp::String("12345").HasAlphanumeric());
  CHECK(emp::String("s0m3 l3tt3r5 @nd num83r5").HasAlphanumeric());
  CHECK(!emp::String(")(!*#@&#^%&!").HasAlphanumeric());

  CHECK(emp::String("abcdefghijklmnopqrstuvwxyz").HasOneOf("aeiou"));
  CHECK(emp::String("abcdefghijklmnopqrstuvwxyz").HasOneOf("abc123"));
  CHECK(!emp::String("abcdefghijklmnopqrstuvwxyz").HasOneOf("12345"));

  CHECK(emp::String("391830581734").OnlyDigits());
  CHECK(!emp::String("3h91830581734").OnlyDigits());
  CHECK(!emp::String("3.14").OnlyDigits());
  CHECK(!emp::String("8.5e-6").OnlyDigits());
  CHECK(!emp::String("9e27").OnlyDigits());

  CHECK(emp::String("391830581734").IsNumber());
  CHECK(!emp::String("3h91830581734").IsNumber());
  CHECK(emp::String("3.14").IsNumber());
  CHECK(emp::String("8.5e-6").IsNumber());
  CHECK(emp::String("9e27").IsNumber());
  CHECK(!emp::String("e").IsNumber());
  CHECK(!emp::String("-.e").IsNumber());
  CHECK(!emp::String("-4.5e").IsNumber());
  CHECK(emp::String("-4.5e+4").IsNumber());
  CHECK(!emp::String(".").IsNumber());
  CHECK(emp::String(".1").IsNumber());
  CHECK(!emp::String("1.").IsNumber());

  CHECK(emp::String("39adg18af3tj05ykty81734").OnlyAlphanumeric());
  CHECK(!emp::String("39adg18af?3tj05ykty81734").OnlyAlphanumeric());

  CHECK(emp::String("'f'").IsLiteralChar());
  CHECK(emp::String("' '").IsLiteralChar());
  CHECK(!emp::String("f").IsLiteralChar());
  CHECK(emp::String("'\n'").IsLiteralChar());
  CHECK(!emp::String("'\\'").IsLiteralChar());
  CHECK(emp::MakeCharFromLiteral("'f'") == 'f');
  CHECK(emp::MakeCharFromLiteral("'\n'") == '\n');

  CHECK(emp::String("\"He llo!\"").IsLiteralString());
  CHECK(!emp::String("\"\\\\He\"llo!\"").IsLiteralString());
  CHECK(emp::String("\"Hel\n\t\r\\\'lo!\"").IsLiteralString());
  CHECK(emp::String("\"Hel\n \t \r \'lo!\"").IsLiteralString());
  CHECK(emp::MakeStringFromLiteral("\"Hello!\"") == "Hello!");
  CHECK(emp::MakeStringFromLiteral("\"Hel\n \t \r \'lo!\"") == "Hel\n \t \r \'lo!");

  // Make sure that we can properly identify different types of characters.
  const emp::String special_string = "This\t5tr1ng\nis\non THREE (3) \"lines\".";

  CHECK(special_string.CountWhitespace() == 6);
  CHECK(special_string.CountUpper() == 6);
  CHECK(special_string.CountLower() == 16);
  CHECK(special_string.CountLetters() == 22);
  CHECK(special_string.CountDigits() == 3);
  CHECK(special_string.CountAlphanumeric() == 25);
  CHECK(special_string.CountNonwhitespace() == 30);

  /*
  CHECK(emp::is_valid("aaaaaaaaa", [](char x) { return (x == 'a'); } ));
  CHECK( !(emp::is_valid("aaaabaaaa", [](char x) { return (x == 'a'); })) );
*/
}

TEST_CASE("Test String Find Functions", "[tools]")
{
  emp::String pal = "able was I ere I saw Elba";
  // Search for single characters.
  CHECK(pal.Find('a') == 0);
  CHECK(pal.Find('b') == 1);
  CHECK(pal.Find('c') == std::string::npos);
  CHECK(pal.Find('e') == 3);
  CHECK(pal.Find('I') == 9);
  CHECK(pal.Find('E') == 21);

  // Try with offset
  CHECK(pal.Find('a', 10) == 18);
  CHECK(pal.Find('b', 10) == 23);
  CHECK(pal.Find('c', 10) == std::string::npos);
  CHECK(pal.Find('e', 10) == 11);
  CHECK(pal.Find('I', 10) == 15);
  CHECK(pal.Find('E', 10) == 21);

  // Try reversed.
  CHECK(pal.RFind('a') == 24);
  CHECK(pal.RFind('b') == 23);
  CHECK(pal.RFind('c') == std::string::npos);
  CHECK(pal.RFind('e') == 13);
  CHECK(pal.RFind('I') == 15);
  CHECK(pal.RFind('E') == 21);

  // Try reversed with offset.
  CHECK(pal.RFind('a', 20) == 18);
  CHECK(pal.RFind('b', 20) == 1);
  CHECK(pal.RFind('c', 20) == std::string::npos);
  CHECK(pal.RFind('e', 20) == 13);
  CHECK(pal.RFind('I', 20) == 15);
  CHECK(pal.RFind('E', 20) == std::string::npos);

  // Try string Find.
  CHECK(pal.Find("able") == 0);
  CHECK(pal.Find("was") == 5);
  CHECK(pal.Find("I") == 9);
  CHECK(pal.Find("able", 5) == std::string::npos);
  CHECK(pal.Find("was", 5) == 5);
  CHECK(pal.Find("I", 10) == 15);
  CHECK(pal.RFind("able") == 0);
  CHECK(pal.RFind("was") == 5);
  CHECK(pal.RFind("I") == 15);

  // Try CharSet Find.
  CHECK(pal.Find(emp::LowerCharSet()) == 0);
  CHECK(pal.Find(emp::WhitespaceCharSet()) == 4);
  CHECK(pal.Find(emp::UpperCharSet()) == 9);
  CHECK(pal.Find(emp::UpperCharSet(), 10) == 15);
  CHECK(pal.Find(emp::UpperCharSet(), 16) == 21);
  CHECK(pal.RFind(emp::UpperCharSet()) == 21);
  CHECK(pal.RFind(emp::UpperCharSet(), 20) == 15);
  CHECK(pal.RFind(emp::UpperCharSet(), 10) == 9);
  CHECK(pal.RFind(emp::UpperCharSet(), 5) == std::string::npos);

  // Do some tests on quotes in strings...
  emp::String quotes = "\"abc\"\"def\"123 \"\"\"long\\\"er\"";  // "abc""def"123 """long\"er"
  CHECK( quotes.FindQuoteMatch() == 4 );
  CHECK( quotes.FindQuoteMatch(1) == std::string::npos );
  CHECK( quotes.FindQuoteMatch(5) == 9 );
  CHECK( quotes.FindQuoteMatch(10) == std::string::npos );
  CHECK( quotes.FindQuoteMatch(14) == 15 );
  CHECK( quotes.FindQuoteMatch(16) == 25 );

  CHECK( quotes.RFindQuoteMatch(4) == 0 );
  CHECK( quotes.RFindQuoteMatch(9) == 5 );
  CHECK( quotes.RFindQuoteMatch(15) == 14 );
  CHECK( quotes.RFindQuoteMatch(25) == 16 );

  // Do some tests on parentheses matching...
  emp::String parens = "(()(()()))((())\")))))()\")";
  CHECK( parens.FindParenMatch() == 9 );
  CHECK( parens.FindParenMatch(0) == 9 );
  CHECK( parens.FindParenMatch(1) == 2 );
  CHECK( parens.FindParenMatch(2) == std::string::npos );
  CHECK( parens.FindParenMatch(3) == 8 );
  CHECK( parens.FindParenMatch(10) == 16 );
  CHECK( parens.FindParenMatch(11) == 14 );
  CHECK( parens.FindParenMatch(21) == 22 ); // Works inside a quote if start there.
  CHECK( parens.FindParenMatch(10,{"","()"}) == 16 );   // Specify parens and show works.
  CHECK( parens.FindParenMatch(10,{"\"","()"}) == 24 ); // Do not ignore quotes.
  CHECK( parens.FindParenMatch(10,{"","ab"}) == std::string::npos );   // Using non-parens works.
  CHECK( quotes.FindParenMatch(1,{"","ab"}) == 2 );   // Using non-parens works.

  CHECK( parens.RFindParenMatch(9) == 0 );
  CHECK( parens.RFindParenMatch(2) == 1 );
  CHECK( parens.RFindParenMatch(8) == 3 );
  CHECK( parens.RFindParenMatch(16) == 10 );
  CHECK( parens.RFindParenMatch(14) == 11 );
  CHECK( parens.RFindParenMatch(22) == 21 ); // Works inside a quote if start there.

  // Extra tests with braces and single quotes.
  emp::String braces = "{{}{}}{'{}}'}";
  CHECK( braces.FindParenMatch(0) == 5 );
  CHECK( braces.FindParenMatch(0,{"","{}"}) == 5 );
  CHECK( braces.FindParenMatch(1,{"","{}"}) == 2 );
  CHECK( braces.FindParenMatch(2,{"","{}"}) == std::string::npos );
  CHECK( braces.FindParenMatch(3,{"","{}"}) == 4 );
  CHECK( braces.FindParenMatch(6,{"","{}"}) == 10 );  // Across single quotes
  CHECK( braces.FindParenMatch(6,{"'","{}"}) == 12 ); // Don't ignore quotes.

  // Test a multi-find.
  emp::String test_str = "This is my best test!";
  emp::vector<size_t> found = test_str.FindAll(' ');
  CHECK( found == emp::vector<size_t>{4,7,10,15} );
  test_str.FindAll('i', found);
  CHECK( found == emp::vector<size_t>{2,5} );
  parens.FindAll(')', found);
  CHECK( found == emp::vector<size_t>{2,5,7,8,9,13,14,16,17,18,19,20,22,24} );
  parens.FindAll(')', found, "\""); // Ignore items in quotes.
  CHECK( found == emp::vector<size_t>{2,5,7,8,9,13,14,24} );
}

TEST_CASE("Test String Pop and Slice Functions", "[tools]")
{
  emp::String start = "a string.";
  CHECK(start.PopFixed(9) == "a string.");
  CHECK(start == "");

  start = "This is a slightly longer string";
  auto split = start.Slice(" ");
  CHECK(split.size() == 6);
  CHECK(split[0] == "This");
  CHECK(split[5] == "string");

  start = "This string has \"internal quotes\" that shouldn't be split.";
  split = start.Slice(" ", {"\""}); // Slice, but keep quotes as one unit.
  for (auto x : split) { std::cout << ":" << x << ": "; } std::cout << std::endl;
  CHECK(split.size() == 8);
  CHECK(split[0] == "This");
  CHECK(split[3] == "\"internal quotes\"");
  CHECK(split[5] == "shouldn't");
}

TEST_CASE("Test String Removal Functions", "[tools]")
{

  emp::String hello = "!!h&&e#l!!&l###o&!!";
  hello.RemoveChars("!&#");
  CHECK(hello == "hello");

  emp::String email = "you@example.com";
  email.RemovePunctuation();
  CHECK(email == "youexamplecom");
}

TEST_CASE("Test String Conversion Functions", "[tools]")
{
  // Test conversion to an escaped string.
  const emp::String special_string = "This\t5tr1ng\nis\non THREE (3) \"lines\".";
  emp::String escaped_string = emp::MakeEscaped(special_string);

  // note: we had to double-escape the test to make sure this worked.
  CHECK(escaped_string == "This\\t5tr1ng\\nis\\non THREE (3) \\\"lines\\\".");

  // Test more general conversion to literals.
  CHECK(emp::MakeLiteral(42) == "42");
  CHECK(emp::MakeLiteral('a') == "'a'");
  CHECK(emp::MakeLiteral('\t') == "'\\t'");
  CHECK(emp::MakeLiteral(1.234) == "1.234");

  CHECK(special_string.AsEscaped() == escaped_string);
  CHECK(special_string.AsEscaped() == escaped_string);

  emp::String base_string = "This is an okay string.\n  \tThis\nis   -MY-    very best str1ng!!!!   ";
  CHECK(emp::MakeSlugify(base_string) == "this-is-an-okay-string-this-is-my-very-best-str1ng");

  emp::String first_line = base_string.PopLine();

  CHECK(first_line == "This is an okay string.");
  CHECK(first_line.ViewWord() == "This");

  CHECK(first_line.PopWord() == "This");
  CHECK(first_line == "is an okay string.");

  CHECK(first_line.RemoveWhitespace() == "isanokaystring.");

  CHECK(first_line.Pop("ns") == "i");
  CHECK(first_line == "anokaystring.");

  emp::String popped_str = first_line.Pop("ns");
  CHECK(popped_str == "a");
  CHECK(first_line == "okaystring.");

  popped_str = first_line.Pop('y');
  CHECK(popped_str == "oka");
  CHECK(first_line == "string.");

  CHECK(base_string.TrimFront() == "This\nis   -MY-    very best str1ng!!!!   ");
  CHECK(base_string.TrimBack() == "This\nis   -MY-    very best str1ng!!!!");
  CHECK(base_string.Compress() == "This is -MY- very best str1ng!!!!");
}


TEST_CASE("Test String assign and Macro functions", "[tools]")
{
  emp::String test = "TIMES(abc,3) + TIMES(def,2) + TIMES(g, 8)";
  test.ReplaceMacro("TIMES(", ")",
    [](emp::String check_body, size_t, size_t) {
      emp::String pattern = check_body.Pop(',');
      size_t count = check_body.As<size_t>();
      return pattern * count;
    });

  CHECK(test == "abcabcabc + defdef + gggggggg");

  test = "MACRO(ABC);\n  MACRO(\"DEF\");\n MACRO( \"([{\");\n  and normal;";
  test.ReplaceMacro("MACRO(", ")",
    [](emp::String check_body, size_t, size_t) { return emp::MakeString("[[[", check_body, "]]]"); }
  );
  CHECK(test == "[[[ABC]]];\n  [[[\"DEF\"]]];\n [[[ \"([{\"]]];\n  and normal;");
}
