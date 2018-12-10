#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include "tools/BitVector.h"
#include "base/vector.h"

#include <sstream>

TEST_CASE("Test BitVector", "[tools]")
{
	// Constructor
	emp::BitVector bv(10);
	
	// Get Size
	REQUIRE( (bv.GetSize() == 10) );
	REQUIRE( (bv.size() == 10) );
	
	// Set & Get
	bv.Set(0);
	REQUIRE(bv.Get(0));
	bv.Set(1, false);
	REQUIRE(!bv.Get(1));
	
	// Assignment operator
	emp::BitVector bv1(10);
	bv1 = bv;
	REQUIRE(bv1.Get(0));
	
	// Resize
	bv1.Set(9);
	bv1.resize(8);
	REQUIRE( (bv1.GetSize() == 8) );
	REQUIRE( (bv1.GetByte(0) == 1) );
	bv1.resize(128);	
	REQUIRE( (bv1.GetSize() == 128) );
	REQUIRE( (bv1.GetByte(1) == 0) );
	
	// Comparison operators
	REQUIRE((bv1 != bv));
	bv1.Resize(10);
	REQUIRE((bv1 == bv));
	REQUIRE((bv1 >= bv));
	bv.Set(1);
	REQUIRE((bv > bv1));
	REQUIRE((bv >= bv1));
	
	// Set & Get Byte
	emp::BitVector bv2(32);
	bv2.SetByte(0, 128);
	bv2.SetByte(1, 255);
	REQUIRE((bv2.GetByte(0) == 128));
	REQUIRE((bv2.GetByte(1) == 255));
	
	// Count Ones
	REQUIRE((bv2.CountOnes() == 9));
	REQUIRE((bv2.CountOnes_Mixed() == 9));
	REQUIRE((bv2.CountOnes_Sparse() == 9));
	REQUIRE((bv2.count() == 9));
	
	// Any All None SetAll Clear
	REQUIRE(bool(bv2)); // operator bool()
	REQUIRE(bool(bv2[7])); // bool operator[]
	REQUIRE(bv2.any());
	REQUIRE(!bv2.all());
	REQUIRE(!bv2.none());
	bv2.SetAll();
	REQUIRE(!bv2.none());
	REQUIRE(bv2.all());
	bv2.Clear();
	REQUIRE(bv2.none());
	REQUIRE(!bv2.all());
	
	// Prints
	std::stringstream ss;
	emp::BitVector bv3(8);
	bv3.SetByte(0,255);
	bv3.Print(ss);
	REQUIRE((ss.str() == "11111111"));
	ss.str(std::string()); // clear ss
	
	ss << bv3;
	REQUIRE((ss.str() == "11111111"));
	ss.str(std::string()); // clear ss
	
	bv3.SetByte(0,130);
	bv3.PrintOneIDs(ss);
	REQUIRE((ss.str() == "1 7 "));
	ss.str(std::string()); // clear ss
	
	bv3.PrintArray(ss);
	REQUIRE((ss.str() == "01000001"));
	ss.str(std::string()); // clear ss
	
	emp::BitVector bv4(96);
	bv4.SetByte(1,1);
	bv4.PrintFields(ss);
	REQUIRE((ss.str() == "000000000000000000000000000000000 000000000000000000000000000000000000000000000000000000100000000"));
	ss.str(std::string()); // clear ss
	
	// Find & Pop Bit
	bv3.SetByte(0,74);
	REQUIRE((bv3.PopBit() == 1));
	REQUIRE((bv3.CountOnes() == 2));
	REQUIRE((bv3.GetByte(0) == 72));
	REQUIRE((bv3.FindBit() == 3));
	REQUIRE((bv3.FindBit(4) == 6));
	bv3.PopBit();
	bv3.PopBit();
	REQUIRE((bv3.FindBit() == -1));
	REQUIRE((bv3.FindBit(2) == -1));
	REQUIRE((bv3.PopBit() == -1));
	
	// Get Ones
	emp::vector<size_t> ones = bv3.GetOnes();
	REQUIRE((ones.size() == 0));
	bv3.SetByte(0,10);
	ones = bv3.GetOnes();
	REQUIRE((ones[0] == 1));
	REQUIRE((ones[1] == 3));
	
	// Logic operators
	emp::BitVector bv5(8);
	bv5.SetByte(0,28);
	REQUIRE((bv3.CountOnes() == 8-((~bv3).CountOnes())));
	REQUIRE(((bv3 & bv5).GetByte(0) == 8));
	REQUIRE(((bv3 | bv5).GetByte(0) == 30));
	REQUIRE(((bv3 ^ bv5).GetByte(0) == 22));
	REQUIRE(((bv3 << 2).GetByte(0) == 40));
	REQUIRE(((bv5 >> 2).GetByte(0) == 7));
	
	// Compound operators
	bv5 &= bv3;
	REQUIRE((bv5.GetByte(0) == 8));
	bv5 |= bv3;
	REQUIRE((bv5.GetByte(0) == 10));
	bv5 ^= bv3;
	REQUIRE((bv5.GetByte(0) == 0));
	bv3 >>= 2;
	REQUIRE((bv3.GetByte(0) == 2));
	bv3 <<= 4;
	REQUIRE((bv3.GetByte(0) == 32));
}