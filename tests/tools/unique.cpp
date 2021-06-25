#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/tools/unique.hpp"

#include <sstream>
#include <iostream>

TEST_CASE("Test Functions", "[tools]")
{
    REQUIRE(emp::UniqueVal() == 0);
    REQUIRE(emp::UniqueVal() == 1);
	REQUIRE(emp::UniqueName("string") == "string2");
}
