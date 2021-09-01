//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#define CATCH_CONFIG_MAIN

#include "emp/tools/string_utils.hpp"
#include "../third-party/Catch/single_include/catch2/catch.hpp"

TEST_CASE("Test string_utils", "[tools]")
{
	const std::string empt = emp::empty_string();
	CHECK(empt == "");

	CHECK('\0' == (char) NULL);
	std::string zero = emp::to_escaped_string('\0');
	CHECK(zero != "\0");
	CHECK(zero == "\\0");

	CHECK(emp::to_escaped_string((char)1) == "\\001");
	CHECK(emp::to_escaped_string((char)2) == "\\002");
	CHECK(emp::to_escaped_string((char)3) == "\\003");
	CHECK(emp::to_escaped_string((char)4) == "\\004");
	CHECK(emp::to_escaped_string((char)5) == "\\005");
	CHECK(emp::to_escaped_string((char)6) == "\\006");

	std::string to_escp = "\b";
	std::string escaped = emp::to_escaped_string(to_escp);
	CHECK(escaped[0] == '\\');
	CHECK(escaped[1] == 'b');

	std::string string1 = "\a\v\f\r";
	std::string string2 = emp::to_escaped_string(string1);
	CHECK(string2[1] == 'a');
	CHECK(string2[3] == 'v');
	CHECK(string2[5] == 'f');
	CHECK(string2[7] == 'r');
	CHECK(string2[0] == '\\');

	CHECK(emp::to_escaped_string((char)14) == "\\016");
	CHECK(emp::to_escaped_string((char)15) == "\\017");
	CHECK(emp::to_escaped_string((char)16) == "\\020");
	CHECK(emp::to_escaped_string((char)17) == "\\021");
	CHECK(emp::to_escaped_string((char)18) == "\\022");
	CHECK(emp::to_escaped_string((char)19) == "\\023");
	CHECK(emp::to_escaped_string((char)20) == "\\024");
	CHECK(emp::to_escaped_string((char)21) == "\\025");
	CHECK(emp::to_escaped_string((char)22) == "\\026");
	CHECK(emp::to_escaped_string((char)23) == "\\027");
	CHECK(emp::to_escaped_string((char)24) == "\\030");
	CHECK(emp::to_escaped_string((char)25) == "\\031");
	CHECK(emp::to_escaped_string((char)26) == "\\032");
	CHECK(emp::to_escaped_string((char)27) == "\\033");
	CHECK(emp::to_escaped_string((char)28) == "\\034");
	CHECK(emp::to_escaped_string((char)29) == "\\035");
	CHECK(emp::to_escaped_string((char)30) == "\\036");
	CHECK(emp::to_escaped_string((char)31) == "\\037");

	CHECK(emp::to_escaped_string('\'') == "\\\'");
	CHECK(emp::to_escaped_string('\\') == "\\\\");

	std::string up = "A String!";
	CHECK(emp::to_upper(up) == "A STRING!");
	CHECK(emp::to_lower(up) == "a string!");

	CHECK(emp::to_roman_numeral(500000) == "");
	CHECK(emp::to_roman_numeral(50) == "L");
	CHECK(emp::to_roman_numeral(562) == "DLXII");
	CHECK(emp::to_roman_numeral(2128) == "MMCXXVIII");
	CHECK(emp::to_roman_numeral(-999) == "-CMXCIX");
	CHECK(emp::to_roman_numeral(444) == "CDXLIV");

	CHECK(emp::is_idchar('a'));
	CHECK(emp::is_idchar('_'));
	CHECK(emp::is_idchar('5'));
	CHECK(!emp::is_idchar('?'));

	CHECK(emp::is_one_of('v',"uvwxyz"));
	CHECK(emp::is_one_of(';',"!?.,;:'()"));
	CHECK(!emp::is_one_of('a',"!?.,;:'()"));

	CHECK(emp::is_composed_of("aabcccabbcccabcbca","abc"));
	CHECK(!emp::is_composed_of("aabcccabbcccxxbcbca","abc"));

	CHECK(emp::has_whitespace("This string has lots of space!\n"));
	CHECK(emp::has_whitespace("abcdefghijklmnop\nqrstuvwxyz"));
	CHECK(!emp::has_whitespace("none"));

	CHECK(emp::has_upper_letter("--Uppercase Letters--"));
	CHECK(!emp::has_upper_letter("lowercase..."));
	CHECK(emp::has_lower_letter("~Lots of Lowercase~"));
	CHECK(!emp::has_lower_letter("ALL UPPERCASE!!"));

	CHECK(emp::has_letter(",./';'[]-6q82348962"));
	CHECK(!emp::has_letter("1-2-3-4-5"));
	CHECK(emp::has_digit("!@#$%^&*()0987654321"));
	CHECK(!emp::has_digit("!@#$%^&*()abcdefg"));
	CHECK(emp::has_alphanumeric("all letters"));
	CHECK(emp::has_alphanumeric("12345"));
	CHECK(emp::has_alphanumeric("s0m3 l3tt3r5 @nd num83r5"));
	CHECK(!emp::has_alphanumeric(")(!*#@&#^%&!"));

	CHECK(emp::has_idchar("!@#$%^&*()_"));
	CHECK(!emp::has_idchar("!@#$%^&*()"));
	CHECK(emp::has_one_of("abcdefghijklmnopqrstuvwxyz","aeiou"));
	CHECK(emp::has_one_of("abcdefghijklmnopqrstuvwxyz","abc123"));
	CHECK(!emp::has_one_of("abcdefghijklmnopqrstuvwxyz","12345"));

	CHECK(emp::is_valid("aaaaaaaaa", [](char x) { return (x == 'a'); } ));
	CHECK( !(emp::is_valid("aaaabaaaa", [](char x) { return (x == 'a'); })) );

	std::string start = "a string.";
	CHECK(emp::string_pop_fixed(start, 9) == "a string.");
	CHECK(start == "");

	CHECK(emp::string_get("John Doe"," ") == "John");
	CHECK(emp::string_get_line("Line1\nLine2\nLine3") == "Line1");

	std::string hello = "!!h&&e#l!!&l###o&!!";
	emp::remove_chars(hello, "!&#");
	CHECK(hello == "hello");

	std::string email = "you@example.com";
	emp::remove_punctuation(email);
	CHECK(email == "youexamplecom");

	emp::vector<std::string> numbers;
	numbers.push_back("1");
	numbers.push_back("2");
	numbers.push_back("3");
	emp::vector<int> int_numbers = emp::from_strings<int>(numbers);
	CHECK(int_numbers[0] == 1);
	CHECK(int_numbers[1] == 2);
	CHECK(int_numbers[2] == 3);

  CHECK(emp::is_digits("391830581734"));
  CHECK(!emp::is_digits("3h91830581734"));
  CHECK(emp::is_alphanumeric("39adg18af3tj05ykty81734"));
  CHECK(!emp::is_alphanumeric("39adg18af?3tj05ykty81734"));
  CHECK(emp::is_literal_char("'f'"));
  CHECK(emp::is_literal_char("' '"));
  CHECK(!emp::is_literal_char("f"));
  CHECK(emp::is_literal_char("'\n'"));
  CHECK(!emp::is_literal_char("'\\'"));
  CHECK(emp::from_literal_char("'f'") == 'f');
  CHECK(emp::from_literal_char("'\n'") == '\n');
  CHECK(emp::is_literal_string("\"He llo!\""));
  CHECK(!emp::is_literal_string("\"\\\\He\"llo!\""));
  CHECK(emp::is_literal_string("\"Hel\n\t\r\\\'lo!\""));
  CHECK(emp::is_literal_string("\"Hel\n \t \r \'lo!\""));
  CHECK(emp::from_literal_string("\"Hello!\"") == "Hello!");
  CHECK(emp::from_literal_string("\"Hel\n \t \r \'lo!\"") == "Hel\n \t \r \'lo!");

  // TODO: try this with more arguments
	int one;
	emp::from_string<int>("1", one);
	CHECK(one == 1);
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
  CHECK( emp::find_quote_end(quotes) == 5 );
  CHECK( emp::find_quote_end(quotes, 1) == 1 );
  CHECK( emp::find_quote_end(quotes, 5) == 10 );
  CHECK( emp::find_quote_end(quotes, 10) == 10 );
  CHECK( emp::find_quote_end(quotes, 14) == 16 );
  CHECK( emp::find_quote_end(quotes, 16) == 26 );

  CHECK( emp::string_pop_quote(quotes) == "\"abc\"");
  CHECK( emp::string_pop_quote(quotes) == "\"def\"");
  CHECK( emp::string_pop_quote(quotes) == "");
  CHECK( emp::string_pop_word(quotes) == "123");
  CHECK( emp::string_pop_quote(quotes) == "\"\"");
  CHECK( emp::string_pop_quote(quotes) == "\"long\\\"er\"");
  CHECK( emp::string_pop_quote(quotes) == "");

  emp::vector<std::string_view> slice_view = emp::view_slices(view_test, ' ');
  CHECK( slice_view.size() == 5 );
  CHECK( slice_view[0] == "This" );
  CHECK( slice_view[1] == "is" );
  CHECK( slice_view[2] == "my" );
  CHECK( slice_view[3] == "view" );
  CHECK( slice_view[4] == "test!" );


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

  typedef emp::vector<std::string> strings;

  CHECK( ""  == emp::join_on(strings(), "") );
  CHECK( "" == emp::join_on(strings(), "bla") );
  CHECK( "aap" == emp::join_on(strings(els, els + 1), "") );
  CHECK( "aap" == emp::join_on(strings(els, els + 1), "#") );
  CHECK( "aap" == emp::join_on(strings(els, els + 1), "##") );
  CHECK( "aapnoot" == emp::join_on(strings(els, els + 2), "") );
  CHECK( "aap#noot" == emp::join_on(strings(els, els + 2), "#") );
  CHECK( "aap##noot" == emp::join_on(strings(els, els + 2), "##") );
  CHECK( "aapnootmies" == emp::join_on(strings(els, els + 3), "") );
  CHECK( "aap#noot#mies" == emp::join_on(strings(els, els + 3), "#") );
  CHECK( "aap##noot##mies" == emp::join_on(strings(els, els + 3), "##") );
  CHECK( "aap  noot  mies" == emp::join_on(strings(els, els + 3), "  ") );
  CHECK( "aapnootmies" == emp::join_on(strings(els, els + 3), "\0"));
  CHECK(
    "aapnootmies"
    ==
    emp::join_on(strings(els, els + 3), std::string("\0" , 1).c_str())
  );
  CHECK(
    "aapnootmies"
    ==
    emp::join_on(strings(els, els + 3), std::string("\0+", 2).c_str())
  );
  CHECK(
    "aap+noot+mies"
    ==
    emp::join_on(strings(els, els + 3), std::string("+\0", 2).c_str())
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
