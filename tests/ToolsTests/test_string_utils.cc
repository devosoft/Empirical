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
	
}