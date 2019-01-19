#define CATCH_CONFIG_MAIN

#include "tools/string_utils.h"
#include "../third-party/Catch/single_include/catch.hpp"

TEST_CASE("Test string_utils", "[tools]")
{
	const std::string empt = emp::empty_string();
	REQUIRE(empt == "");

	REQUIRE('\0' == NULL);
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
	
	// TODO: try this with more arguments
	int one;
	emp::from_string<int>("1", one);
	REQUIRE(one == 1);
}