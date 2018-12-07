#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include "tools/RegEx.h"

#include <sstream>
#include <iostream>

TEST_CASE("Test RegEx", "[tools]")
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
	
	// AsString
	std::string reS = re.AsString();
	REQUIRE(reS == "\"1|2\"");
	
	// test all the prints
	// what even are all the structs in this file???
}