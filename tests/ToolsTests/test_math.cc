#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include "tools/math.h"

#include <sstream>
#include <iostream>

TEST_CASE("Test Math", "[tools]")
{
	REQUIRE(emp::Mod(5.5, 3.3) == 2.2);
	REQUIRE(emp::MinRef(0,4,-1,6,52) == -1);
	REQUIRE(emp::MaxRef(0,4,-1,6,52) == 52);
	
	REQUIRE(emp::Log10(100.0) == 2);
	REQUIRE(emp::Ln(emp::E) == 1);
	REQUIRE( emp::Abs(emp::Ln(emp::Exp(5)) - 5) < 0.01);
	
	REQUIRE(emp::IntLog2(10) == 3);
	REQUIRE(emp::CountOnes(15) == 4);
	REQUIRE(emp::CountOnes(255) == 8);
	
	unsigned long long large = 0x8000000000000000;
	REQUIRE(emp::MaskHigh<unsigned long long>(1) == large);
	
	REQUIRE(emp::Min(7,3,100,-50,62) == -50);
	REQUIRE(emp::Max(7,3,100,-50,62) == 100);
}