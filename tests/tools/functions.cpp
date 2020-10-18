#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/tools/functions.h"

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

  REQUIRE(emp::ToRange(-10000, 10, 20) == 10);
  REQUIRE(emp::ToRange(9, 10, 20) == 10);
  REQUIRE(emp::ToRange(10, 10, 20) == 10);
  REQUIRE(emp::ToRange(11, 10, 20) == 11);
  REQUIRE(emp::ToRange(17, 10, 20) == 17);
  REQUIRE(emp::ToRange(20, 10, 20) == 20);
  REQUIRE(emp::ToRange(21, 10, 20) == 20);
  REQUIRE(emp::ToRange(12345678, 10, 20) == 20);
  REQUIRE(emp::ToRange<double>(12345678, 10, 20.1) == 20.1);
  REQUIRE(emp::ToRange(12345678.0, 10.7, 20.1) == 20.1);
}
