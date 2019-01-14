#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include "tools/Random.h"

#include <iostream>

TEST_CASE("Test Random", "[tools]")
{
	// Get Seed
	emp::Random rnd(1);
	REQUIRE(rnd.GetSeed() == 1);
	REQUIRE(rnd.GetOriginalSeed() == 1);
	rnd.ResetSeed(5);
	REQUIRE(rnd.GetOriginalSeed() == 5);
	
	// Get Double
	double r_d = rnd.GetDouble(emp::Range<double>(0.0,5.0));
	REQUIRE(r_d >= 0.0);
	REQUIRE(r_d < 5.0);
	
	// Get UInt
	size_t r_ui = rnd.GetUInt(emp::Range<size_t>(0,5));
	REQUIRE(r_ui >= 0);
	REQUIRE(r_ui < 5);
	
	// Get Int
	int r_i = rnd.GetInt(emp::Range<int>(-5,5));
	REQUIRE(r_i >= -5);
	REQUIRE(r_i < 5);
}