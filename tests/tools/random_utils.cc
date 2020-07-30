#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "tools/random_utils.h"

#include <sstream>
#include <string>

TEST_CASE("Test random_utils", "[tools]")
{
	emp::Random rnd(5);
	size_t SIZE = 10;
	emp::vector<size_t> permutation = emp::GetPermutation(rnd, SIZE);
	REQUIRE(permutation.size() == SIZE);
	// Ensure there is one of each number 0-10 in the permutation
	std::unordered_map<int, int> counts({
										{0,1},{1,1},{2,1},
										{3,1},{4,1},{5,1},
										{6,1},{7,1},{8,1},{9,1}
										});
	for(size_t i=0;i<SIZE;i++){
		counts[permutation[i]] -= 1;
	}
	for(size_t i=0;i<SIZE;i++){
		REQUIRE(counts[i] == 0);
	}
	
	emp::BitVector bv = emp::RandomBitVector(rnd, SIZE);
	REQUIRE(bv.size() == SIZE);
	
	emp::vector<double> doubleVec = emp::RandomDoubleVector(rnd, SIZE, 0, 12.5);
	REQUIRE(doubleVec.size() == SIZE);
	for(size_t i=0;i<SIZE;i++){
		REQUIRE(doubleVec[i] < 12.5);
		REQUIRE(doubleVec[i] >= 0);
	}
	
	emp::vector<int> intVec = emp::RandomVector(rnd, SIZE, -30, -10);
	REQUIRE(intVec.size() == SIZE);
	for(size_t i=0;i<SIZE;i++){
		REQUIRE(intVec[i] <= -10);
		REQUIRE(intVec[i] >= -30);
	}
	
	emp::RandomizeBitVector(bv, rnd, 1.0);
	REQUIRE(bv.count() == SIZE);
	
	emp::RandomizeVector(doubleVec, rnd, -15.0, 15.0);
	REQUIRE(doubleVec.size() == SIZE);
	for(size_t i=0;i<SIZE;i++){
		REQUIRE(doubleVec[i] < 15.0);
		REQUIRE(doubleVec[i] >= -15.0);
	}
	
}