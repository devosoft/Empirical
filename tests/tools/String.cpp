/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2023.
 *
 *  @file String.cpp
 */

#include "../third-party/Catch/single_include/catch2/catch.hpp"

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
  CHECK(up.ToUpper() == "A STRING!");
  CHECK(up.ToLower() == "a string!");

  CHECK(emp::MakeRoman(50) == "L");
  CHECK(emp::MakeRoman(562) == "DLXII");
  CHECK(emp::MakeRoman(2128) == "MMCXXVIII");
  CHECK(emp::MakeRoman(-999) == "-CMXCIX");
  CHECK(emp::MakeRoman(444) == "CDXLIV");
  CHECK(emp::MakeRoman(500000) == "D|");
  CHECK(emp::MakeRoman(500000000) == "D||");
  CHECK(emp::MakeRoman(500500500) == "D|D|D");
}

TEST_CASE("Test String Composition Functions", "[tools]")
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
  CHECK(emp::MakeFromLiteral_Char("'f'") == 'f');
  CHECK(emp::MakeFromLiteral_Char("'\n'") == '\n');
  
  CHECK(emp::String("\"He llo!\"").IsLiteralString());
  CHECK(!emp::String("\"\\\\He\"llo!\"").IsLiteralString());
  CHECK(emp::String("\"Hel\n\t\r\\\'lo!\"").IsLiteralString());
  CHECK(emp::String("\"Hel\n \t \r \'lo!\"").IsLiteralString());
  CHECK(emp::MakeFromLiteral_String("\"Hello!\"") == "Hello!");
  CHECK(emp::MakeFromLiteral_String("\"Hel\n \t \r \'lo!\"") == "Hel\n \t \r \'lo!");

/*
  CHECK(emp::is_valid("aaaaaaaaa", [](char x) { return (x == 'a'); } ));
  CHECK( !(emp::is_valid("aaaabaaaa", [](char x) { return (x == 'a'); })) );
*/
}

TEST_CASE("Test String Pop Functions", "[tools]")
{
  emp::String start = "a string.";
  CHECK(start.PopFixed(9) == "a string.");
  CHECK(start == "");
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


/*
  emp::vector<std::string> numbers;
  numbers.push_back("1");
  numbers.push_back("2");
  numbers.push_back("3");
  emp::vector<int> int_numbers = emp::from_strings<int>(numbers);
  CHECK(int_numbers[0] == 1);
  CHECK(int_numbers[1] == 2);
  CHECK(int_numbers[2] == 3);

}

TEST_CASE("Another Test string_utils", "[tools]")
{

  // TEST1: lets test our conversion to an escaped string.
  const std::string special_string = "This\t5tr1ng\nis\non THREE (3) \"lines\".";
  std::string escaped_string = emp::to_escaped_string(special_string);

  // note: we had to double-escape the test to make sure this worked.
  CHECK(escaped_string == "This\\t5tr1ng\\nis\\non THREE (3) \\\"lines\\\".");

  // TEST2: Test more general conversion to literals.
  CHECK(emp::to_literal(42) == "42");
  CHECK(emp::to_literal('a') == "'a'");
  CHECK(emp::to_literal('\t') == "'\\t'");
  CHECK(emp::to_literal(1.234) == "1.234000");

  // TEST3: Make sure that we can properly identify different types of characters.
  int num_ws = 0;
  int num_cap = 0;
  int num_lower = 0;
  int num_let = 0;
  int num_num = 0;
  int num_alphanum = 0;
  int num_i = 0;
  int num_vowel = 0;
  for (char cur_char : special_string) {
    if (emp::is_whitespace(cur_char)) num_ws++;
    if (emp::is_upper_letter(cur_char)) num_cap++;
    if (emp::is_lower_letter(cur_char)) num_lower++;
    if (emp::is_letter(cur_char)) num_let++;
    if (emp::is_digit(cur_char)) num_num++;
    if (emp::is_alphanumeric(cur_char)) num_alphanum++;
    if (emp::is_valid(cur_char, [](char c){ return c=='i'; })) num_i++;
    if (emp::is_valid(cur_char, [](char c){return c=='a' || c=='A';},
                      [](char c){return c=='e' || c=='E';},
                      [](char c){return c=='i' || c=='I';},
                      [](char c){return c=='o' || c=='O';},
                      [](char c){return c=='u' || c=='U';},
                      [](char c){return c=='y';}
                      )) num_vowel++;
  }
  int num_other = ((int) special_string.size()) - num_alphanum - num_ws;


  CHECK(num_ws == 6);
  CHECK(num_cap == 6);
  CHECK(num_lower == 16);
  CHECK(num_let == 22);
  CHECK(num_num == 3);
  CHECK(num_alphanum == 25);
  CHECK(num_other == 5);
  CHECK(num_i == 3);
  CHECK(num_vowel == 7);

  std::string base_string = "This is an okay string.\n  \tThis\nis   -MY-    very best string!!!!   ";

  CHECK(
    emp::slugify(base_string)
    == "this-is-an-okay-string-this-is-my-very-best-string"
  );

  std::string first_line = emp::string_pop_line(base_string);

  CHECK(first_line == "This is an okay string.");
  CHECK(emp::string_get_word(first_line) == "This");

  emp::string_pop_word(first_line);

  CHECK(first_line == "is an okay string.");

  emp::remove_whitespace(first_line);

  CHECK(first_line == "isanokaystring.");

  std::string popped_str = emp::string_pop(first_line, "ns");

  CHECK(popped_str == "i");
  CHECK(first_line == "anokaystring.");


  popped_str = emp::string_pop(first_line, "ns");


  CHECK(popped_str == "a");
  CHECK(first_line == "okaystring.");


  popped_str = emp::string_pop(first_line, 'y');

  CHECK(popped_str == "oka");
  CHECK(first_line == "string.");

  emp::left_justify(base_string);
  CHECK(base_string == "This\nis   -MY-    very best string!!!!   ");

  emp::right_justify(base_string);
  CHECK(base_string == "This\nis   -MY-    very best string!!!!");

  emp::compress_whitespace(base_string);
  CHECK(base_string == "This is -MY- very best string!!!!");


  std::string view_test = "This is my view test!";
  CHECK( emp::view_string(view_test) == "This is my view test!" );
  CHECK( emp::view_string(view_test, 5) == "is my view test!" );
  CHECK( emp::view_string(view_test, 8, 2) == "my" );
  CHECK( emp::view_string_front(view_test,4) == "This" );
  CHECK( emp::view_string_back(view_test, 5) == "test!" );
  CHECK( emp::view_string_range(view_test, 11, 15) == "view" );
  CHECK( emp::view_string_to(view_test, ' ') == "This" );
  CHECK( emp::view_string_to(view_test, ' ', 5) == "is" );

  // Do some tests on quotes in strings...
  std::string quotes = "\"abc\"\"def\"123 \"\"\"long\\\"er\"";  // "abc""def"123 """long\"er"
  CHECK( emp::find_quote_match(quotes) == 4 );
  CHECK( emp::find_quote_match(quotes, 1) == 1 );
  CHECK( emp::find_quote_match(quotes, 5) == 9 );
  CHECK( emp::find_quote_match(quotes, 10) == 10 );
  CHECK( emp::find_quote_match(quotes, 14) == 15 );
  CHECK( emp::find_quote_match(quotes, 16) == 25 );

  CHECK( emp::string_pop_quote(quotes) == "\"abc\"");
  CHECK( emp::string_pop_quote(quotes) == "\"def\"");
  CHECK( emp::string_pop_quote(quotes) == "");
  CHECK( emp::string_pop_word(quotes) == "123");
  CHECK( emp::string_pop_quote(quotes) == "\"\"");
  CHECK( emp::string_pop_quote(quotes) == "\"long\\\"er\"");
  CHECK( emp::string_pop_quote(quotes) == "");

  // Do some tests on parentheses matching...
  std::string parens = "(()(()()))((())\")))))()\")";
  CHECK( emp::find_paren_match(parens) == 9 );
  CHECK( emp::find_paren_match(parens, 0) == 9 );
  CHECK( emp::find_paren_match(parens, 1) == 2 );
  CHECK( emp::find_paren_match(parens, 2) == 2 );
  CHECK( emp::find_paren_match(parens, 3) == 8 );
  CHECK( emp::find_paren_match(parens, 3) == 8 );
  CHECK( emp::find_paren_match(parens, 10) == 24 );
  CHECK( emp::find_paren_match(parens, 11) == 14 );
  CHECK( emp::find_paren_match(parens, 21) == 22 ); // Works inside a quote if start there.
  CHECK( emp::find_paren_match(parens, 10,'(',')',true) == 24 );  // Specify parens and show works.
  CHECK( emp::find_paren_match(parens, 10,'(',')',false) == 16 ); // Do no ignore quotes.
  CHECK( emp::find_paren_match(parens, 10,'a','b',false) == 10 ); // Using non-parens works.

  // Extra tests with braces and single quotes.
  std::string braces = "{{}{}}{'{}}'}";
  CHECK( emp::find_paren_match(braces, 0) == 0 );
  CHECK( emp::find_paren_match(braces, 0, '{', '}') == 5 );
  CHECK( emp::find_paren_match(braces, 1, '{', '}') == 2 );
  CHECK( emp::find_paren_match(braces, 3, '{', '}') == 4 );
  CHECK( emp::find_paren_match(braces, 6, '{', '}') == 12 );  // Across single quotes
  CHECK( emp::find_paren_match(braces, 6, '{', '}', false) == 10 ); // Don't ignore quotes.

  // Test a multi-find.
  emp::vector<size_t> found = emp::find_all(view_test, ' ');
  CHECK( found == emp::vector<size_t>{4,7,10,15} );
  emp::find_all(view_test, 'i', found);
  CHECK( found == emp::vector<size_t>{2,5,12} );
  emp::find_all(parens, ')', found);
  CHECK( found == emp::vector<size_t>{2,5,7,8,9,13,14,16,17,18,19,20,22,24} );
  emp::find_all(parens, ')', found, true); // Ignore items in quotes.
  CHECK( found == emp::vector<size_t>{2,5,7,8,9,13,14,24} );

  emp::vector<std::string_view> slice_view = emp::view_slices(view_test, ' ');
  CHECK( slice_view.size() == 5 );
  CHECK( slice_view[0] == "This" );
  CHECK( slice_view[1] == "is" );
  CHECK( slice_view[2] == "my" );
  CHECK( slice_view[3] == "view" );
  CHECK( slice_view[4] == "test!" );

  std::string quoted_text = "This is \"text with quotes in it\" that we can \"divide up\" in different ways.";
  slice_view = emp::view_slices(quoted_text, ' ');
  CHECK( slice_view.size() == 15 );
  CHECK( slice_view[0] == "This" );
  CHECK( slice_view[1] == "is" );
  CHECK( slice_view[2] == "\"text" );
  CHECK( slice_view[3] == "with" );
  CHECK( slice_view[4] == "quotes" );

  slice_view = emp::view_slices(quoted_text, ' ', true); // Don't break up quotes!
  CHECK( slice_view.size() == 10 );
  CHECK( slice_view[0] == "This" );
  CHECK( slice_view[1] == "is" );
  CHECK( slice_view[2] == "\"text with quotes in it\"" );
  CHECK( slice_view[3] == "that" );
  CHECK( slice_view[4] == "we" );


  auto slices = emp::slice(
    "This is a test of a different version of slice.",
    ' '
  );
  CHECK(slices.size() == 10);
  CHECK(slices[8] == "of");

  slices = emp::slice(
    "This is a test of a different version of slice.",
    ' ',
    101
  );
  CHECK(slices.size() == 10);
  CHECK(slices[8] == "of");

  slices = emp::slice(
    "This is a test.",
    ' ',
    0
  );
  CHECK(slices.size() == 1);
  CHECK(slices[0] == "This is a test.");

  slices = emp::slice(
    "This is a test.",
    ' ',
    1
  );
  CHECK(slices.size() == 2);
  CHECK(slices[0] == "This");
  CHECK(slices[1] == "is a test.");

  slices = emp::slice(
    "This is a test.",
    ' ',
    2
  );
  CHECK(slices.size() == 3);
  CHECK(slices[0] == "This");
  CHECK(slices[1] == "is");
  CHECK(slices[2] == "a test.");

  // Try other ways of using slice().
  emp::slice(base_string, slices, 's');

  CHECK(slices.size() == 5);
  CHECK(slices[1] == " i");
  CHECK(slices[3] == "t ");


  std::string CSV_line = "123.5,\"Smith,John\",\"123 ABC Street\",single_token,\"a,b,c,d\",DONE.";
  auto CSV_data = emp::ViewCSV(CSV_line);
  CHECK(CSV_data.size() == 6);
  CHECK(CSV_data[0] == "123.5");
  CHECK(CSV_data[1] == "\"Smith,John\"");
  CHECK(CSV_data[2] == "\"123 ABC Street\"");
  CHECK(CSV_data[3] == "single_token");
  CHECK(CSV_data[4] == "\"a,b,c,d\"");
  CHECK(CSV_data[5] == "DONE.");

  // Test ViewNestedBlock
  std::string code = "abc(def(ghi(())j)k(lm(n))o)pq";
  CHECK(emp::ViewNestedBlock(code) == "");
  CHECK(emp::ViewNestedBlock(code, "()", 0) == "");
  CHECK(emp::ViewNestedBlock(code, "()", 1) == "");
  CHECK(emp::ViewNestedBlock(code, "()", 3) == "def(ghi(())j)k(lm(n))o");
  CHECK(emp::ViewNestedBlock(code, "()", 7) == "ghi(())j");
  CHECK(emp::ViewNestedBlock(code, "()", 11) == "()");
  CHECK(emp::ViewNestedBlock(code, "()", 18) == "lm(n)");
  CHECK(emp::ViewNestedBlock(code, "()", 21) == "n");
  CHECK(emp::ViewNestedBlock(code, "()", 22) == "");
  CHECK(emp::ViewNestedBlock(code, "{}", 3) == "");
  CHECK(emp::ViewNestedBlock(code, "aj") == "bc(def(ghi(())");

  // Some tests of to_string() function.
  CHECK(emp::to_string((int) 1) == "1");
  CHECK(emp::to_string("2") == "2");
  CHECK(emp::to_string(std::string("3")) == "3");
  CHECK(emp::to_string('4') == "4");
  CHECK(emp::to_string((int16_t) 5) == "5");
  CHECK(emp::to_string((int32_t) 6) == "6");
  CHECK(emp::to_string((int64_t) 7) == "7");
  CHECK(emp::to_string((uint16_t) 8) == "8");
  CHECK(emp::to_string((uint32_t) 9) == "9");
  CHECK(emp::to_string((uint64_t) 10) == "10");
  CHECK(emp::to_string((size_t) 11) == "11");
  CHECK(emp::to_string((long) 12) == "12");
  CHECK(emp::to_string((unsigned long) 13) == "13");
  CHECK(emp::to_string((float) 14.0) == "14");
  CHECK(emp::to_string((float) 14.1) == "14.1");
  CHECK(emp::to_string((float) 14.1234) == "14.1234");
  CHECK(emp::to_string((double) 15.0) == "15");
  CHECK(emp::to_string(16.0) == "16");
  CHECK(emp::to_string(emp::vector<size_t>({17,18,19})) == "[ 17 18 19 ]");
  CHECK(emp::to_string((char) 32) == " ");
  CHECK(emp::to_string((unsigned char) 33) == "!");

  std::string cat_a = "ABC";
  bool cat_b = true;
  char cat_c = '2';
  int cat_d = 3;

  std::string cat_full = emp::to_string(cat_a, cat_b, cat_c, cat_d);

  CHECK(cat_full == "ABC123");
  emp::array<int, 3> test_arr({{ 4, 2, 5 }});
  CHECK(emp::to_string(test_arr) == "[ 4 2 5 ]");
  CHECK(emp::count(emp::to_string(test_arr), ' ') == 4);
  CHECK(emp::join(emp::vector<size_t>({17,18,19}), ",") == "17,18,19");
  CHECK(emp::join(emp::vector<size_t>({}), ",") == "");
  CHECK(emp::join(emp::vector<size_t>({17}), ",") == "17");

  // tests adapted from https://stackoverflow.com/questions/5288396/c-ostream-out-manipulation/5289170#5289170
  std::string els[] = { "aap", "noot", "mies" };

  using strings = emp::vector<std::string>;

  CHECK( ""  == emp::join(strings(), "") );
  CHECK( "" == emp::join(strings(), "bla") );
  CHECK( "aap" == emp::join(strings(els, els + 1), "") );
  CHECK( "aap" == emp::join(strings(els, els + 1), "#") );
  CHECK( "aap" == emp::join(strings(els, els + 1), "##") );
  CHECK( "aapnoot" == emp::join(strings(els, els + 2), "") );
  CHECK( "aap#noot" == emp::join(strings(els, els + 2), "#") );
  CHECK( "aap##noot" == emp::join(strings(els, els + 2), "##") );
  CHECK( "aapnootmies" == emp::join(strings(els, els + 3), "") );
  CHECK( "aap#noot#mies" == emp::join(strings(els, els + 3), "#") );
  CHECK( "aap##noot##mies" == emp::join(strings(els, els + 3), "##") );
  CHECK( "aap  noot  mies" == emp::join(strings(els, els + 3), "  ") );
  CHECK( "aapnootmies" == emp::join(strings(els, els + 3), "\0"));
  CHECK(
    "aapnootmies"
    ==
    emp::join(strings(els, els + 3), std::string("\0" , 1).c_str())
  );
  CHECK(
    "aapnootmies"
    ==
    emp::join(strings(els, els + 3), std::string("\0+", 2).c_str())
  );
  CHECK(
    "aap+noot+mies"
    ==
    emp::join(strings(els, els + 3), std::string("+\0", 2).c_str())
  );

  emp::string_vec_t string_v;

  CHECK( emp::to_english_list(string_v) == "" );

  string_v.push_back("one");

  CHECK( emp::to_english_list(string_v) == "one" );

  string_v.push_back("two");

  CHECK( emp::to_english_list(string_v) == "one and two" );

  string_v.push_back("three");

  CHECK( emp::to_english_list(string_v) == "one, two, and three" );

  string_v.push_back("four");

  CHECK( emp::to_english_list(string_v) == "one, two, three, and four" );
  CHECK( emp::to_quoted_list(string_v) == "'one', 'two', 'three', and 'four'");

  emp::string_vec_t quoted_strings = emp::quote_strings(string_v);

  CHECK( quoted_strings[0] == "'one'" );
  CHECK( quoted_strings[2] == "'three'" );

  quoted_strings = emp::quote_strings(string_v, "***");

  CHECK( quoted_strings[1] == "***two***" );
  CHECK( quoted_strings[3] == "***four***" );

  quoted_strings = emp::quote_strings(string_v, "([{<", ">}])");

  CHECK( quoted_strings[0] == "([{<one>}])" );
  CHECK( quoted_strings[2] == "([{<three>}])" );

  CHECK( emp::to_titlecase("Harry Potter and the pRisoner of azkaban") == "Harry Potter And The Prisoner Of Azkaban");
}

TEST_CASE("Test to_web_safe_string", "[tools]" ){
  // requires that angle backets are replaced with &lt or &gt
  CHECK( emp::to_web_safe_string("<h1>hi</h1>" ) == "&lth1&gthi&lt/h1&gt");
  // requires that ampersands are replaced with &amp
  CHECK( emp::to_web_safe_string("one & two" ) == "one &amp two");
  // requires that double quotes are replaced with &quot
  CHECK( emp::to_web_safe_string("\"one and two\"" ) == "&quotone and two&quot");
  // requires that single quotes are replaced with &apos
  CHECK( emp::to_web_safe_string("'one and two'" ) == "&aposone and two&apos");
  // requires that strings with multiple reserved characters are replaced as expected
  REQUIRE ( emp::to_web_safe_string("<h1>\"Hello\" & 'bye'</h1>") == "&lth1&gt&quotHello&quot &amp &aposbye&apos&lt/h1&gt" );

}

TEST_CASE("Test format_string", "[tools]") {

  CHECK( emp::format_string("") == "" );
  CHECK( emp::format_string("%s hi", "twee") == "twee hi" );
  CHECK( emp::format_string("a %d b %s", 7, "foo") == "a 7 b foo" );

  const std::string multiline{ R"(
    my code;
    %s
    more code;
  )" };
  const std::string replacement{ "foo code;" };


  CHECK( emp::format_string(multiline, replacement.c_str()) == R"(
    my code;
    foo code;
    more code;
  )" );

}

TEST_CASE("Test repeat", "[tools]") {

  CHECK( emp::repeat("", 0) == "" );
  CHECK( emp::repeat("", 1) == "" );
  CHECK( emp::repeat("", 2) == "" );

  CHECK( emp::repeat("abc", 0) == "" );
  CHECK( emp::repeat("abc", 1) == "abc" );
  CHECK( emp::repeat("abc", 2) == "abcabc" );

}

TEST_CASE("Test url-encode", "[tools]") {
  REQUIRE( emp::url_encode("шеллы") == "%D1%88%D0%B5%D0%BB%D0%BB%D1%8B" );
  REQUIRE( emp::url_decode("%D1%88%D0%B5%D0%BB%D0%BB%D1%8B") == "шеллы" );
  REQUIRE( emp::url_encode(" ") == "%20" );
  REQUIRE( emp::url_encode<true>(" ") == "+" );
  REQUIRE( emp::url_decode("%20+") == " +" );
  REQUIRE( emp::url_decode<true>("%20+") == "  " );
}

*/