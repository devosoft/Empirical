#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include "tools/UnorderedIndexMap.h"

#include <sstream>
#include <iostream>

TEST_CASE("Test UnorderedIndexMap", "[tools]")
{
	emp::UnorderedIndexMap uim(5);
	REQUIRE(uim.GetSize() == 5);
	uim.Resize(10);
	REQUIRE(uim.GetSize() == 10);
	
	//uim.RawAdjust(0, 5);
}