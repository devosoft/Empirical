#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "tools/functions.h"

#include <sstream>
#include <iostream>

TEST_CASE("Test Functions", "[tools]")
{ 
	REQUIRE(emp::TimeFun([] () { return 2*2; }) < 2.0);
	
	REQUIRE(emp::AnyTrue(true, false, false, false, true, false) == true);
	
	int some_ints[] = {1, 2, 4, 8};
	REQUIRE(emp::GetSize(some_ints) == 4);
	
	REQUIRE(emp::UniqueName("string") == "string0");
}

TEST_CASE("Another Test functions", "[tools]")
{

  bool test_bool = true;
  emp::Toggle(test_bool);
  REQUIRE(test_bool == false);
}
