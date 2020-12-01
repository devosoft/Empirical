#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/datastructs/UnorderedIndexMap.hpp"

#include <sstream>
#include <iostream>

TEST_CASE("Test UnorderedIndexMap", "[datastructs]")
{
	emp::UnorderedIndexMap uim(10);
	REQUIRE(uim.GetSize() == 10);
	uim.resize(5);
	REQUIRE(uim.size() == 5);

	emp::vector<double> vecd;
	vecd.push_back(5);
	vecd.push_back(4);
	vecd.push_back(3);
	vecd.push_back(2);
	vecd.push_back(1);
	uim.Adjust(vecd);
	REQUIRE(uim.GetSize() == 5);
	REQUIRE(uim.GetWeight() == 15);
	REQUIRE(uim.GetProb(0) ==  (1.0/3.0));
	uim.Clear();
	REQUIRE(uim.GetWeight() == 0);
	uim.AdjustAll(2.0);
	REQUIRE(uim.GetWeight() == 10);
	uim.ResizeClear(10);
	REQUIRE(uim.GetWeight() == 0);
	REQUIRE(uim.GetSize() == 10);
	uim.AdjustAll(2.0);

	// Index
	size_t ind = uim.Index(0);
	REQUIRE(uim[ind] == 2);
	uim.Adjust(ind, 3.0);
	REQUIRE(uim[ind] == 3);
	uim[ind] = 4;
	REQUIRE(uim[ind] == 4);

	// -= +=
	emp::UnorderedIndexMap uim2(10);
	uim2.AdjustAll(2.0);
	uim -= uim2;
	REQUIRE(uim.GetWeight() == 2);
	uim2 += uim;
	REQUIRE(uim2.GetWeight() == 22);
}