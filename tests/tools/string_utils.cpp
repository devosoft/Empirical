//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#define CATCH_CONFIG_MAIN

#include "emp/base/optional.hpp"
#include "emp/tools/string_utils.hpp"
#include "../third-party/Catch/single_include/catch2/catch.hpp"

TEST_CASE("Test string_utils", "[tools]")
{
	const std::string empt = emp::empty_string();
	REQUIRE(empt == "");

	REQUIRE('\0' == (char) NULL);
	std::string zero = emp::to_escaped_string('\0');
	REQUIRE(zero != "\0");
	REQUIRE(zero == "\\0");

	REQUIRE(emp::to_escaped_string((char)1) == "\\001");
	REQUIRE(emp::to_escaped_string((char)2) == "\\002");
	REQUIRE(emp::to_escaped_string((char)3) == "\\003");
	REQUIRE(emp::to_escaped_string((char)4) == "\\004");
	REQUIRE(emp::to_escaped_string((char)5) == "\\005");
	REQUIRE(emp::to_escaped_string((char)6) == "\\006");

	std::string to_escp = "\b";
	std::string escaped = emp::to_escaped_string(to_escp);
	REQUIRE(escaped[0] == '\\');
	REQUIRE(escaped[1] == 'b');

	std::string string1 = "\a\v\f\r";
	std::string string2 = emp::to_escaped_string(string1);
	REQUIRE(string2[1] == 'a');
	REQUIRE(string2[3] == 'v');
	REQUIRE(string2[5] == 'f');
	REQUIRE(string2[7] == 'r');
	REQUIRE(string2[0] == '\\');

	REQUIRE(emp::to_escaped_string((char)14) == "\\016");
	REQUIRE(emp::to_escaped_string((char)15) == "\\017");
	REQUIRE(emp::to_escaped_string((char)16) == "\\020");
	REQUIRE(emp::to_escaped_string((char)17) == "\\021");
	REQUIRE(emp::to_escaped_string((char)18) == "\\022");
	REQUIRE(emp::to_escaped_string((char)19) == "\\023");
	REQUIRE(emp::to_escaped_string((char)20) == "\\024");
	REQUIRE(emp::to_escaped_string((char)21) == "\\025");
	REQUIRE(emp::to_escaped_string((char)22) == "\\026");
	REQUIRE(emp::to_escaped_string((char)23) == "\\027");
	REQUIRE(emp::to_escaped_string((char)24) == "\\030");
	REQUIRE(emp::to_escaped_string((char)25) == "\\031");
	REQUIRE(emp::to_escaped_string((char)26) == "\\032");
	REQUIRE(emp::to_escaped_string((char)27) == "\\033");
	REQUIRE(emp::to_escaped_string((char)28) == "\\034");
	REQUIRE(emp::to_escaped_string((char)29) == "\\035");
	REQUIRE(emp::to_escaped_string((char)30) == "\\036");
	REQUIRE(emp::to_escaped_string((char)31) == "\\037");

	REQUIRE(emp::to_escaped_string('\'') == "\\\'");
	REQUIRE(emp::to_escaped_string('\\') == "\\\\");

	std::string up = "A String!";
	REQUIRE(emp::to_upper(up) == "A STRING!");
	REQUIRE(emp::to_lower(up) == "a string!");

	REQUIRE(emp::to_roman_numeral(500000) == "");
	REQUIRE(emp::to_roman_numeral(50) == "L");
	REQUIRE(emp::to_roman_numeral(562) == "DLXII");
	REQUIRE(emp::to_roman_numeral(2128) == "MMCXXVIII");
	REQUIRE(emp::to_roman_numeral(-999) == "-CMXCIX");
	REQUIRE(emp::to_roman_numeral(444) == "CDXLIV");

	REQUIRE(emp::is_idchar('a'));
	REQUIRE(emp::is_idchar('_'));
	REQUIRE(emp::is_idchar('5'));
	REQUIRE(!emp::is_idchar('?'));

	REQUIRE(emp::is_one_of('v',"uvwxyz"));
	REQUIRE(emp::is_one_of(';',"!?.,;:'()"));
	REQUIRE(!emp::is_one_of('a',"!?.,;:'()"));

	REQUIRE(emp::is_composed_of("aabcccabbcccabcbca","abc"));
	REQUIRE(!emp::is_composed_of("aabcccabbcccxxbcbca","abc"));

	REQUIRE(emp::has_whitespace("This string has lots of space!\n"));
	REQUIRE(emp::has_whitespace("abcdefghijklmnop\nqrstuvwxyz"));
	REQUIRE(!emp::has_whitespace("none"));

	REQUIRE(emp::has_upper_letter("--Uppercase Letters--"));
	REQUIRE(!emp::has_upper_letter("lowercase..."));
	REQUIRE(emp::has_lower_letter("~Lots of Lowercase~"));
	REQUIRE(!emp::has_lower_letter("ALL UPPERCASE!!"));

	REQUIRE(emp::has_letter(",./';'[]-6q82348962"));
	REQUIRE(!emp::has_letter("1-2-3-4-5"));
	REQUIRE(emp::has_digit("!@#$%^&*()0987654321"));
	REQUIRE(!emp::has_digit("!@#$%^&*()abcdefg"));
	REQUIRE(emp::has_alphanumeric("all letters"));
	REQUIRE(emp::has_alphanumeric("12345"));
	REQUIRE(emp::has_alphanumeric("s0m3 l3tt3r5 @nd num83r5"));
	REQUIRE(!emp::has_alphanumeric(")(!*#@&#^%&!"));

	REQUIRE(emp::has_idchar("!@#$%^&*()_"));
	REQUIRE(!emp::has_idchar("!@#$%^&*()"));
	REQUIRE(emp::has_one_of("abcdefghijklmnopqrstuvwxyz","aeiou"));
	REQUIRE(emp::has_one_of("abcdefghijklmnopqrstuvwxyz","abc123"));
	REQUIRE(!emp::has_one_of("abcdefghijklmnopqrstuvwxyz","12345"));

	REQUIRE(emp::is_valid("aaaaaaaaa", [](char x) { return (x == 'a'); } ));
	REQUIRE( !(emp::is_valid("aaaabaaaa", [](char x) { return (x == 'a'); })) );

	std::string start = "a string.";
	REQUIRE(emp::string_pop_fixed(start, 9) == "a string.");
	REQUIRE(start == "");

	REQUIRE(emp::string_get("John Doe"," ") == "John");
	REQUIRE(emp::string_get_line("Line1\nLine2\nLine3") == "Line1");

	std::string hello = "!!h&&e#l!!&l###o&!!";
	emp::remove_chars(hello, "!&#");
	REQUIRE(hello == "hello");

	std::string email = "you@example.com";
	emp::remove_punctuation(email);
	REQUIRE(email == "youexamplecom");

	emp::vector<std::string> numbers;
	numbers.push_back("1");
	numbers.push_back("2");
	numbers.push_back("3");
	emp::vector<int> int_numbers = emp::from_strings<int>(numbers);
	REQUIRE(int_numbers[0] == 1);
	REQUIRE(int_numbers[1] == 2);
	REQUIRE(int_numbers[2] == 3);

  REQUIRE(emp::is_digits("391830581734"));
  REQUIRE(!emp::is_digits("3h91830581734"));
  REQUIRE(emp::is_alphanumeric("39adg18af3tj05ykty81734"));
  REQUIRE(!emp::is_alphanumeric("39adg18af?3tj05ykty81734"));
  REQUIRE(emp::is_literal_char("'f'"));
  REQUIRE(emp::is_literal_char("' '"));
  REQUIRE(!emp::is_literal_char("f"));
  REQUIRE(emp::is_literal_char("'\n'"));
  REQUIRE(!emp::is_literal_char("'\\'"));
  REQUIRE(emp::from_literal_char("'f'") == 'f');
  REQUIRE(emp::from_literal_char("'\n'") == '\n');
  REQUIRE(emp::is_literal_string("\"He llo!\""));
  REQUIRE(!emp::is_literal_string("\"\\\\He\"llo!\""));
  REQUIRE(emp::is_literal_string("\"Hel\n\t\r\\\'lo!\""));
  REQUIRE(emp::is_literal_string("\"Hel\n \t \r \'lo!\""));
  REQUIRE(emp::from_literal_string("\"Hello!\"") == "Hello!");
  REQUIRE(emp::from_literal_string("\"Hel\n \t \r \'lo!\"") == "Hel\n \t \r \'lo!");

  // TODO: try this with more arguments
	int one;
	emp::from_string<int>("1", one);
	REQUIRE(one == 1);
}


TEST_CASE("Another Test string_utils", "[tools]")
{

  // TEST1: lets test our conversion to an escaped string.
  const std::string special_string = "This\t5tr1ng\nis\non THREE (3) \"lines\".";
  std::string escaped_string = emp::to_escaped_string(special_string);

  // note: we had to double-escape the test to make sure this worked.
  REQUIRE(escaped_string == "This\\t5tr1ng\\nis\\non THREE (3) \\\"lines\\\".");

  // TEST2: Test more general conversion to literals.
  REQUIRE(emp::to_literal(42) == "42");
  REQUIRE(emp::to_literal('a') == "'a'");
  REQUIRE(emp::to_literal('\t') == "'\\t'");
  REQUIRE(emp::to_literal(1.234) == "1.234000");

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


  REQUIRE(num_ws == 6);
  REQUIRE(num_cap == 6);
  REQUIRE(num_lower == 16);
  REQUIRE(num_let == 22);
  REQUIRE(num_num == 3);
  REQUIRE(num_alphanum == 25);
  REQUIRE(num_other == 5);
  REQUIRE(num_i == 3);
  REQUIRE(num_vowel == 7);

  std::string base_string = "This is an okay string.\n  \tThis\nis   -MY-    very best string!!!!   ";

  REQUIRE(
    emp::slugify(base_string)
    == "this-is-an-okay-string-this-is-my-very-best-string"
  );

  std::string first_line = emp::string_pop_line(base_string);

  REQUIRE(first_line == "This is an okay string.");
  REQUIRE(emp::string_get_word(first_line) == "This");

  emp::string_pop_word(first_line);

  REQUIRE(first_line == "is an okay string.");

  emp::remove_whitespace(first_line);

  REQUIRE(first_line == "isanokaystring.");

  std::string popped_str = emp::string_pop(first_line, "ns");

  REQUIRE(popped_str == "i");
  REQUIRE(first_line == "anokaystring.");


  popped_str = emp::string_pop(first_line, "ns");


  REQUIRE(popped_str == "a");
  REQUIRE(first_line == "okaystring.");


  popped_str = emp::string_pop(first_line, 'y');

  REQUIRE(popped_str == "oka");
  REQUIRE(first_line == "string.");

  emp::left_justify(base_string);
  REQUIRE(base_string == "This\nis   -MY-    very best string!!!!   ");

  emp::right_justify(base_string);
  REQUIRE(base_string == "This\nis   -MY-    very best string!!!!");

  emp::compress_whitespace(base_string);
  REQUIRE(base_string == "This is -MY- very best string!!!!");


  std::string view_test = "This is my view test!";
  REQUIRE( emp::view_string(view_test) == "This is my view test!" );
  REQUIRE( emp::view_string(view_test, 5) == "is my view test!" );
  REQUIRE( emp::view_string(view_test, 8, 2) == "my" );
  REQUIRE( emp::view_string_front(view_test,4) == "This" );
  REQUIRE( emp::view_string_back(view_test, 5) == "test!" );
  REQUIRE( emp::view_string_range(view_test, 11, 15) == "view" );
  REQUIRE( emp::view_string_to(view_test, ' ') == "This" );
  REQUIRE( emp::view_string_to(view_test, ' ', 5) == "is" );

  emp::vector<std::string_view> slice_view = emp::view_slices(view_test, ' ');
  REQUIRE( slice_view.size() == 5 );
  REQUIRE( slice_view[0] == "This" );
  REQUIRE( slice_view[1] == "is" );
  REQUIRE( slice_view[2] == "my" );
  REQUIRE( slice_view[3] == "view" );
  REQUIRE( slice_view[4] == "test!" );


  auto slices = emp::slice(
    "This is a test of a different version of slice.",
    ' '
  );
  REQUIRE(slices.size() == 10);
  REQUIRE(slices[8] == "of");

  slices = emp::slice(
    "This is a test of a different version of slice.",
    ' ',
    101
  );
  REQUIRE(slices.size() == 10);
  REQUIRE(slices[8] == "of");

  slices = emp::slice(
    "This is a test.",
    ' ',
    0
  );
  REQUIRE(slices.size() == 1);
  REQUIRE(slices[0] == "This is a test.");

  slices = emp::slice(
    "This is a test.",
    ' ',
    1
  );
  REQUIRE(slices.size() == 2);
  REQUIRE(slices[0] == "This");
  REQUIRE(slices[1] == "is a test.");

  slices = emp::slice(
    "This is a test.",
    ' ',
    2
  );
  REQUIRE(slices.size() == 3);
  REQUIRE(slices[0] == "This");
  REQUIRE(slices[1] == "is");
  REQUIRE(slices[2] == "a test.");

  // Try other ways of using slice().
  emp::slice(base_string, slices, 's');

  REQUIRE(slices.size() == 5);
  REQUIRE(slices[1] == " i");
  REQUIRE(slices[3] == "t ");


  // Some tests of to_string() function.
  REQUIRE(emp::to_string((int) 1) == "1");
  REQUIRE(emp::to_string("2") == "2");
  REQUIRE(emp::to_string(std::string("3")) == "3");
  REQUIRE(emp::to_string('4') == "4");
  REQUIRE(emp::to_string((int16_t) 5) == "5");
  REQUIRE(emp::to_string((int32_t) 6) == "6");
  REQUIRE(emp::to_string((int64_t) 7) == "7");
  REQUIRE(emp::to_string((uint16_t) 8) == "8");
  REQUIRE(emp::to_string((uint32_t) 9) == "9");
  REQUIRE(emp::to_string((uint64_t) 10) == "10");
  REQUIRE(emp::to_string((size_t) 11) == "11");
  REQUIRE(emp::to_string((long) 12) == "12");
  REQUIRE(emp::to_string((unsigned long) 13) == "13");
  REQUIRE(emp::to_string((float) 14.0) == "14");
  REQUIRE(emp::to_string((float) 14.1) == "14.1");
  REQUIRE(emp::to_string((float) 14.1234) == "14.1234");
  REQUIRE(emp::to_string((double) 15.0) == "15");
  REQUIRE(emp::to_string(16.0) == "16");
  REQUIRE(emp::to_string(emp::vector<size_t>({17,18,19})) == "[ 17 18 19 ]");
  REQUIRE(emp::to_string((char) 32) == " ");
  REQUIRE(emp::to_string((unsigned char) 33) == "!");

  std::string cat_a = "ABC";
  bool cat_b = true;
  char cat_c = '2';
  int cat_d = 3;

  std::string cat_full = emp::to_string(cat_a, cat_b, cat_c, cat_d);

  REQUIRE(cat_full == "ABC123");
  emp::array<int, 3> test_arr({{ 4, 2, 5 }});
  REQUIRE(emp::to_string(test_arr) == "[ 4 2 5 ]");
  REQUIRE(emp::count(emp::to_string(test_arr), ' ') == 4);
  REQUIRE(emp::join(emp::vector<size_t>({17,18,19}), ",") == "17,18,19");
  REQUIRE(emp::join(emp::vector<size_t>({}), ",") == "");
  REQUIRE(emp::join(emp::vector<size_t>({17}), ",") == "17");

  // tests adapted from https://stackoverflow.com/questions/5288396/c-ostream-out-manipulation/5289170#5289170
  std::string els[] = { "aap", "noot", "mies" };

  typedef emp::vector<std::string> strings;

  REQUIRE( ""  == emp::join_on(strings(), "") );
  REQUIRE( "" == emp::join_on(strings(), "bla") );
  REQUIRE( "aap" == emp::join_on(strings(els, els + 1), "") );
  REQUIRE( "aap" == emp::join_on(strings(els, els + 1), "#") );
  REQUIRE( "aap" == emp::join_on(strings(els, els + 1), "##") );
  REQUIRE( "aapnoot" == emp::join_on(strings(els, els + 2), "") );
  REQUIRE( "aap#noot" == emp::join_on(strings(els, els + 2), "#") );
  REQUIRE( "aap##noot" == emp::join_on(strings(els, els + 2), "##") );
  REQUIRE( "aapnootmies" == emp::join_on(strings(els, els + 3), "") );
  REQUIRE( "aap#noot#mies" == emp::join_on(strings(els, els + 3), "#") );
  REQUIRE( "aap##noot##mies" == emp::join_on(strings(els, els + 3), "##") );
  REQUIRE( "aap  noot  mies" == emp::join_on(strings(els, els + 3), "  ") );
  REQUIRE( "aapnootmies" == emp::join_on(strings(els, els + 3), "\0"));
  REQUIRE(
    "aapnootmies"
    ==
    emp::join_on(strings(els, els + 3), std::string("\0" , 1).c_str())
  );
  REQUIRE(
    "aapnootmies"
    ==
    emp::join_on(strings(els, els + 3), std::string("\0+", 2).c_str())
  );
  REQUIRE(
    "aap+noot+mies"
    ==
    emp::join_on(strings(els, els + 3), std::string("+\0", 2).c_str())
  );

  emp::string_vec_t string_v;

  REQUIRE( emp::to_english_list(string_v) == "" );

  string_v.push_back("one");

  REQUIRE( emp::to_english_list(string_v) == "one" );

  string_v.push_back("two");

  REQUIRE( emp::to_english_list(string_v) == "one and two" );

  string_v.push_back("three");

  REQUIRE( emp::to_english_list(string_v) == "one, two, and three" );

  string_v.push_back("four");

  REQUIRE( emp::to_english_list(string_v) == "one, two, three, and four" );
  REQUIRE( emp::to_quoted_list(string_v) == "'one', 'two', 'three', and 'four'");

  emp::string_vec_t quoted_strings = emp::quote_strings(string_v);

  REQUIRE( quoted_strings[0] == "'one'" );
  REQUIRE( quoted_strings[2] == "'three'" );

  quoted_strings = emp::quote_strings(string_v, "***");

  REQUIRE( quoted_strings[1] == "***two***" );
  REQUIRE( quoted_strings[3] == "***four***" );

  quoted_strings = emp::quote_strings(string_v, "([{<", ">}])");

  REQUIRE( quoted_strings[0] == "([{<one>}])" );
  REQUIRE( quoted_strings[2] == "([{<three>}])" );

  REQUIRE( emp::to_titlecase("Harry Potter and the pRisoner of azkaban") == "Harry Potter And The Prisoner Of Azkaban");
}

TEST_CASE("Test to_web_safe_string", "[tools]" ){
  // requires that angle backets are replaced with &lt or &gt
  REQUIRE( emp::to_web_safe_string("<h1>hi</h1>" ) == "&lth1&gthi&lt/h1&gt");
  // requires that ampersands are replaced with &amp
  REQUIRE( emp::to_web_safe_string("one & two" ) == "one &amp two");
  // requires that double quotes are replaced with &quot
  REQUIRE( emp::to_web_safe_string("\"one and two\"" ) == "&quotone and two&quot");
  // requires that single quotes are replaced with &apos
  REQUIRE( emp::to_web_safe_string("'one and two'" ) == "&aposone and two&apos");
  // requires that strings with multiple reserved characters are replaced as expected
  REQUIRE ( emp::to_web_safe_string("<h1>\"Hello\" & 'bye'</h1>") == "&lth1&gt&quotHello&quot &amp &aposbye&apos&lt/h1&gt" );

}

TEST_CASE("Test format_string", "[tools]") {

	REQUIRE( emp::format_string("") == "" );
	REQUIRE( emp::format_string("%s hi", "twee") == "twee hi" );
	REQUIRE( emp::format_string("a %d b %s", 7, "foo") == "a 7 b foo" );

	const std::string multiline{ R"(
		my code;
		%s
		more code;
	)" };
	const std::string replacement{ "foo code;" };


	REQUIRE( emp::format_string(multiline, replacement.c_str()) == R"(
		my code;
		foo code;
		more code;
	)" );

}

TEST_CASE("Test repeat", "[tools]") {

	REQUIRE( emp::repeat("", 0) == "" );
	REQUIRE( emp::repeat("", 1) == "" );
	REQUIRE( emp::repeat("", 2) == "" );

	REQUIRE( emp::repeat("abc", 0) == "" );
	REQUIRE( emp::repeat("abc", 1) == "abc" );
	REQUIRE( emp::repeat("abc", 2) == "abcabc" );

}

TEST_CASE("Test to_string with emp::optional", "[tools]") {

	emp::optional<size_t> foo;
	REQUIRE( emp::to_string( foo ) == "" );

	foo = 1;
	REQUIRE( emp::to_string( foo ) == "1" );

}
