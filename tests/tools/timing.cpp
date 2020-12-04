#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/tools/timing.hpp"

TEST_CASE("Test Functions", "[tools]")
{
	REQUIRE(emp::TimeFun([] () { return 2*2; }) < 2.0);

}
