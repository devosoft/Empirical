#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/datastructs/tuple_utils.hpp"
#include "emp/base/vector.hpp"

#include <sstream>
#include <iostream>


TEST_CASE("Test tuple_utils", "[datastructs]")
{
	std::tuple<int, int, int> tup(1,2,3);
	REQUIRE(emp::tuple_size<decltype(tup)>() == 3);

	emp::ValPack<2, 1, 0> ints;
	std::tuple<int, int, double> tup1(150, 1, 5.0);
	tup1 = emp::shuffle_tuple(tup1, ints);
	REQUIRE(std::get<0>(tup1) == 5.0);
	REQUIRE(std::get<1>(tup1) == 1);
	REQUIRE(std::get<2>(tup1) == 150);

	emp::vector<double> list;
	auto addToList = [&list](double x) { list.push_back(x); };
	emp::TupleIterate(tup1, addToList);
	REQUIRE(list[0] == 5.0);
	REQUIRE(list[1] == 1.0);
	REQUIRE(list[2] == 150.0);

	std::tuple<double, double, double> tup2(5.0, 25.0, 1.0/6.0);
	auto multiplyThenAddToList = [&list] (double x, double y) { list.push_back(x*y); };
	emp::TupleIterate(tup1, tup2, multiplyThenAddToList);
	REQUIRE(list[3] == 25.0);
	REQUIRE(list[4] == 25.0);
	REQUIRE(list[5] == 25.0);
}
