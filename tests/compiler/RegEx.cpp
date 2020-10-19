#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/compiler/RegEx.hpp"

#include <sstream>
#include <iostream>

TEST_CASE("Test RegEx", "[compiler]")
{
	emp::RegEx re("1|2");
	REQUIRE(re.Test("1") == true);
	REQUIRE(re.Test("2") == true);
	REQUIRE(re.Test("12") == false);

	// test operator=
	emp::RegEx re0 = re;
	REQUIRE(re0.Test("1") == true);
	REQUIRE(re0.Test("2") == true);
	REQUIRE(re0.Test("12") == false);
	emp::RegEx re1("3|4");
	re1 = re0;
	REQUIRE(re1.Test("1") == true);
	REQUIRE(re1.Test("2") == true);
	REQUIRE(re1.Test("12") == false);

	// AsString
	std::string reS = re.AsString();
	REQUIRE(reS == "\"1|2\"");

	// test all the prints

	// Try to hit all the different structs in RegEx.h
	emp::RegEx re2("\"r.*\"");
	//re2.PrintDebug();
	//REQUIRE(re2.Test("\"rats\""));
}

TEST_CASE("Test regular expressions (RegEx)", "[compiler]")
{
  emp::RegEx re1("a|bcdef");
  REQUIRE(re1.Test("a") == true);
  REQUIRE(re1.Test("bc") == false);
  REQUIRE(re1.Test("bcdef") == true);
  REQUIRE(re1.Test("bcdefg") == false);

  emp::RegEx re2("#[abcdefghijklm]*abc");
  REQUIRE(re2.Test("") == false);
  REQUIRE(re2.Test("#a") == false);
  REQUIRE(re2.Test("#aaaabc") == true);
  REQUIRE(re2.Test("#abcabc") == true);
  REQUIRE(re2.Test("#abcabcd") == false);

  emp::RegEx re3("xx(y|(z*)?)+xx");
  REQUIRE(re3.Test("xxxx") == true);
  REQUIRE(re3.Test("xxxxx") == false);
  REQUIRE(re3.Test("xxyxx") == true);
  REQUIRE(re3.Test("xxyyxx") == true);
  REQUIRE(re3.Test("xxzzzxx") == true);

  emp::RegEx re_WHITESPACE("[ \t\r]");
  emp::RegEx re_COMMENT("#.*");
  emp::RegEx re_INT_LIT("[0-9]+");
  emp::RegEx re_FLOAT_LIT("[0-9]+[.][0-9]+");
  emp::RegEx re_CHAR_LIT("'(.|(\\\\[\\\\'nt]))'");
  emp::RegEx re_STRING_LIT("[\"]((\\\\[nt\"\\\\])|[^\"])*\\\"");
  emp::RegEx re_ID("[a-zA-Z0-9_]+");

  REQUIRE(re_INT_LIT.Test("1234") == true);
  REQUIRE(re_FLOAT_LIT.Test("1234") == false);
  REQUIRE(re_ID.Test("1234") == true);
  REQUIRE(re_INT_LIT.Test("1234.56") == false);
  REQUIRE(re_FLOAT_LIT.Test("1234.56") == true);
  REQUIRE(re_ID.Test("1234.56") == false);

  std::string test_str = "\"1234\"";
  REQUIRE(re_STRING_LIT.Test(test_str) == true);
  REQUIRE(re_INT_LIT.Test(test_str) == false);

  std::string test_str2 = "\"1234\", \"5678\"";
  REQUIRE(re_STRING_LIT.Test(test_str2) == false);
}
