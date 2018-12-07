#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include "tools/tuple_utils.h"

#include <sstream>
#include <iostream>

TEST_CASE("Test tuple_utils", "[tools]")
{
	std::tuple<int, int, int> tup(1,2,3);
	REQUIRE(emp::tuple_size<decltype(tup)>() == 3);
}