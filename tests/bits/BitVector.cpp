#define CATCH_CONFIG_MAIN
#ifndef NDEBUG
	#undef NDEBUG
	#define TDEBUG 1
#endif

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/bits/BitVector.hpp"
#include "emp/base/vector.hpp"

#include <sstream>
#include <map>
#include <limits>

TEST_CASE("Test BitVector", "[bits]")
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
	emp::BitVector bv20(20);
	emp::BitVector bv30(30);
	bv20.Set(1);
	REQUIRE(bv20.Get(1));
	bv20 = bv;
	REQUIRE(!bv20.Get(1));
	bv20 = bv30;
	REQUIRE(!bv20.Get(1));

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
	REQUIRE((bv1 <= bv));
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

	// Hash
	emp::BitVector bv_a(2);
	bv_a.Set(0);
	emp::BitVector bv_b(2);
	bv_b.Set(0);
	REQUIRE(bv_a.Hash() == bv_b.Hash());
	bv_b.Set(0, false);
	REQUIRE(bv_a.Hash() != bv_b.Hash());
	bv_b.Set(0, true);

	// EQU_SELF
	REQUIRE(bv_a.EQU_SELF(bv_b).all());
	// bv_a = 01, bv_b = 01, ~(01 ^ 01) = 11
	REQUIRE(bv_a.GetByte(0) == 3);
	REQUIRE(bv_b.GetByte(0) == 1);
	REQUIRE(!(bv_a.EQU_SELF(bv_b).all()));
	// bv_a = 11, bv_b = 01, ~(11 ^ 01) = 01
	REQUIRE(bv_a.GetByte(0) == 1);
	REQUIRE(bv_b.GetByte(0) == 1);

	// NAND SELF
	// bv_a = 01, bv_b = 01, ~(01 & 01) = 10
	REQUIRE(bv_a.NAND_SELF(bv_b) == ~bv_b);
	REQUIRE(bv_a.GetByte(0) == 2);

	// NOR SELF
	// bv_a = 10, bv_b = 01, ~(10 | 01) = 00
	REQUIRE(bv_a.NOR_SELF(bv_b).none());
	REQUIRE(bv_a.GetByte(0) == 0);

	// NOT SELF
	REQUIRE(bv_a.NOT_SELF().all());

	// EQU
	emp::BitVector bv_c(3);
	bv_c.SetByte(0,2);
	emp::BitVector bv_d(3);
	bv_d.SetByte(0,2);
	REQUIRE(bv_c.EQU(bv_d).all());
	REQUIRE(bv_c.GetByte(0) == 2);

	// NAND
	REQUIRE(bv_c.NAND(bv_d) == ~bv_c);
	REQUIRE(bv_c.GetByte(0) == 2);

	// NOR
	REQUIRE(bv_c.NOR(bv_d) == ~bv_c);
	REQUIRE(bv_c.GetByte(0) == 2);

	// Bit proxy compound assignment operators
	// AND
	// bv_c = 010
	bv_c[0] &= 1;
	REQUIRE(bv_c[0] == 0);
	REQUIRE(bv_c[1] == 1);
	bv_c[1] &= 0;
	REQUIRE(bv_c[1] == 0);
	// OR
	// bv_d = 010
	bv_d[1] |= 0;
	REQUIRE(bv_d[1] == 1);
	bv_d[0] |= 1;
	REQUIRE(bv_d[0] == 1);
	bv_d[2] |= 0;
	REQUIRE(bv_d[2] == 0);
	// XOR
	// bv_c = 000
	bv_c[0] ^= 1;
	REQUIRE(bv_c[0] == 1);
	bv_c[0] ^= 1;
	REQUIRE(bv_c[0] == 0);
	//PLUS
	// bv_d = 011
	bv_d[2] += 1;
	REQUIRE(bv_d[2] == 1);
	// MINUS
	// bv_d = 111
	bv_d[1] -= 1;
	REQUIRE(bv_d[1] == 0);
	// TIMES
	//bv_d = 101
	bv_d[2] *= 1;
	REQUIRE(bv_d[2] == 1);
	bv_d[0] *= 0;
	REQUIRE(bv_d[0] == 0);
	// DIV
	// bv_c = 000
	bv_c[0] /= 1;
	REQUIRE(bv_c[0] == 0);

	// GetUInt SetUInt
	emp::BitVector bv_e(5);
	bv_e.SetUInt(0, 16);
	REQUIRE(bv_e.GetUInt(0) == 16);

	// Shift Left
	emp::BitVector bv_f(128);
	bv_f.SetAll();
	REQUIRE(bv_f.all());
	bv_f <<= 127;
	REQUIRE(bv_f.count() == 1);
	bv_f <<= 1;
	REQUIRE(bv_f.none());
}

TEST_CASE("Another Test BitVector", "[bits]")
{
  emp::BitVector bv10(10);
  emp::BitVector bv32(32);
  emp::BitVector bv50(50);
  emp::BitVector bv64(64);
  emp::BitVector bv80(80);

  bv80[70] = 1;
  emp::BitVector bv80c(bv80);

  bv80 <<= 1;

  for (size_t i = 0; i < 75; i += 2) {
    emp::BitVector shift_vector = bv80 >> i;
    REQUIRE((shift_vector.CountOnes() == 1) == (i <= 71));
  }

  bv10 = (bv80 >> 70);

  // Test arbitrary bit retrieval of UInts
  bv80[65] = 1;
  REQUIRE(bv80.GetUIntAtBit(64) == 130);
  REQUIRE(bv80.GetValueAtBit<5>(64) == 2);

}

TEST_CASE("BitVector padding bits protected", "[bits]") {
#ifdef TDEBUG

  for (size_t i = 1; i < 32; ++i) {

    emp::BitVector vec(i);
    REQUIRE(emp::assert_last_fail == 0);
    vec.SetUInt(0, std::numeric_limits<uint32_t>::max());
    REQUIRE(emp::assert_last_fail);
    emp::assert_clear();

  }

  REQUIRE(emp::assert_last_fail == 0);

  emp::BitVector vec(32);
  vec.SetUInt(0, std::numeric_limits<uint32_t>::max());

  REQUIRE(emp::assert_last_fail == 0);

#endif
}

TEST_CASE("BitVector regression test for #277", "[bits]") {
  emp::BitVector vec1(4);
  emp::BitVector vec2(4);

  for (size_t i = 0; i < 4; ++i) REQUIRE(!vec1[i]);
  for (size_t i = 0; i < 4; ++i) REQUIRE(!vec2[i]);
  vec1.SetUInt(0, 15);
  vec2.SetUIntAtBit(0, 15);
  for (size_t i = 0; i < 4; ++i) REQUIRE(vec1[i]);
  for (size_t i = 0; i < 4; ++i) REQUIRE(vec2[i]);
}
