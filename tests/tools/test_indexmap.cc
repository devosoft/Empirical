#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include "tools/IndexMap.h"

#include <sstream>
#include <iostream>

TEST_CASE("Test IndexMap", "[tools]")
{
	emp::IndexMap im(4, 2.5);
	REQUIRE(im.GetSize() == 4);
	
	REQUIRE(im.GetWeight(0) == 2.5);
	im.Adjust(0, 3.0);
	REQUIRE(im.GetWeight(0) == 3.0);
	
	im.ResizeClear(3);
	REQUIRE(im.size() == 3);
	REQUIRE(im.GetWeight(0) == 0);
	im[0] = 1;
	im[1] = 2;
	REQUIRE(im.GetWeight(0) == 1);
	REQUIRE(im.GetWeight(1) == 2);
	REQUIRE(im.RawWeight(2) == 1);
	REQUIRE(im.RawWeight(3) == 2);
	
	im.resize(5);
	REQUIRE(im.size() == 5);
	REQUIRE(im.GetWeight(0) == 1);
	REQUIRE(im.RawWeight(7) == 1);
	
	im[2] = 3;
	im[3] = 4;
	im[4] = 5;
	
	REQUIRE(im.RawProb(7) == (1.0/15.0));
	REQUIRE(im.GetProb(4) == (1.0/3.0));
	
	emp::IndexMap im2(5);
	emp::vector<double> new_weights;
	new_weights.push_back(0);
	new_weights.push_back(5);
	new_weights.push_back(10);
	new_weights.push_back(15);
	new_weights.push_back(20);
	
	im2.Adjust(new_weights);
	REQUIRE(im2[0] == 0);
	REQUIRE(im2[1] == 5);
	REQUIRE(im2[2] == 10);
	REQUIRE(im2[3] == 15);
	REQUIRE(im2[4] == 20);
	
	im2 += im;
	REQUIRE(im2[0] == 1);
	REQUIRE(im2[1] == 7);
	REQUIRE(im2[2] == 13);
	REQUIRE(im2[3] == 19);
	REQUIRE(im2[4] == 25);
	
	im2 -= im;
	
	REQUIRE(im2[0] == 0);
	REQUIRE(im2[1] == 5);
	REQUIRE(im2[2] == 10);
	REQUIRE(im2[3] == 15);
	REQUIRE(im2[4] == 20);
}