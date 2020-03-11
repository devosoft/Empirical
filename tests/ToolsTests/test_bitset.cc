#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include <sstream>

#include "tools/BitSet.h"
#include "tools/Random.h"


/**
 * Status booleans (Any, All, None)
 * as well as Clear and SetAll
 */
void test_status(){
	emp::BitSet<10> bs10;
	REQUIRE(!bs10.any());
	REQUIRE(bs10.none());
	REQUIRE(!bs10.all());
	bs10.SetAll();
	REQUIRE(bs10.all());
	bs10.Clear();
	REQUIRE(bs10.none());
}

/**
 * GetSize
 */
void test_size(){
	emp::BitSet<42> bs42;
	REQUIRE(bs42.size() == 42);

	emp::BitSet<35> bs35;
	REQUIRE(bs35.GetSize() == 35);

	emp::BitSet<1> bs1;
	REQUIRE(bs1.size() == 1);
}

/**
 * Flip and Toggle
 */
void test_flip(){
	emp::BitSet<2> bs2;	// bs2 = 00
	bs2.flip(0);				// bs2 = 01
	REQUIRE(bs2[0]);

	emp::BitSet<8> bs8;	// bs8 = 00000000
	bs8.flip(0,4);			// bs8 = 00001111
	REQUIRE(bs8[0]);
	REQUIRE(bs8[1]);
	REQUIRE(bs8[2]);
	REQUIRE(bs8[3]);
	REQUIRE(!bs8[4]);

	bs8[0].Toggle();		// bs8 = 00001110
	REQUIRE(!bs8[0]);

	emp::BitSet<4> bs4;	// bs4 = 0000
	bs4.flip();					// bs4 = 1111
	REQUIRE(bs4.all());
}

/**
 * FindBit and PopBit
 */
void test_find(){
	emp::BitSet<10> bs10;	// bs10 = 00 00000000
	bs10.flip(3);					// bs10 = 00 00001000
	REQUIRE(bs10.FindBit() == 3);
	bs10.PopBit();				// bs10 = 00 00000000
	REQUIRE(bs10.PopBit() == -1);
	bs10.flip(3);
	bs10.flip(1);
	REQUIRE(bs10.FindBit(2) == 3);
	REQUIRE(bs10.FindBit(4) == -1);
}

/**
 * GetByte and SetByte
 */
void test_byte(){
	emp::BitSet<10>	bs10;
	bs10.SetByte(0, 10);
	REQUIRE(bs10.GetByte(0) == 10);

	bs10.flip(0,4);
	REQUIRE(bs10.GetByte(0) == 5);
	bs10.SetByte(1, 3);
	REQUIRE(bs10.count() == 4);
}

/**
 * Left and Right shifts
 */
void test_shift(){
	emp::BitSet<40> bs40;
	bs40.SetByte(0, 1);
	bs40 <<= 34;
	REQUIRE(bs40.GetByte(4) == 4);

	emp::BitSet<10> bs10;
	bs10.SetByte(0, 10);
	bs10 >>= 2;
	REQUIRE(bs10.GetByte(0) == 2);
}

/**
 * Count ones
 */
void test_count(){
	emp::BitSet<12> bs12;
	bs12.SetAll();
	REQUIRE(bs12.count() == 12);
	REQUIRE(bs12.CountOnes_Sparse() == 12);
	bs12.flip(0,5);
	REQUIRE(bs12.count() == 7);
}

/**
 * Get ones
 */
void test_get_ones(){
	emp::BitSet<5> bs5;
	bs5.flip(2); // 00100
	emp::vector<size_t> ones = bs5.GetOnes();
	REQUIRE(ones.size() == 1);
	REQUIRE(ones[0] == 2);
}


/**
 * Get and Set bits
 */
void test_bit(){
	emp::BitSet<8> bs8;
	bs8.Set(0, 1); 			// bs8 = 00000001
	REQUIRE(bs8.Get(0));

	bs8.Set(7, 1);			// bs8 = 10000001
	bs8.Set(0, 0);			// bs8 = 10000000
	REQUIRE(!bs8.Get(0));
	REQUIRE(bs8.Get(7));
}

/**
 * Bitwise XOR ^
 */
void test_bitwise_xor(){
	emp::BitSet<4> bs4;
	bs4.Set(0, 1);
	emp::BitSet<4> bs4_1;
	bs4_1.SetByte(0,3);
	bs4 ^= bs4_1; 								// bs4 = 0001 ^ 0011 = 0010
	REQUIRE(bs4.GetByte(0) == 2); 	// 0010 = 2
	bs4_1.PopBit(); 							// bs4_1 = 0010
	bs4 ^= bs4_1; 								// bs4 = 0010 ^ 0010 = 0000
	REQUIRE(bs4.GetByte(0) == 0);	// 0000 = 0
}

/**
 * Bitwise OR |
 */
void test_bitwise_or(){
	emp::BitSet<10> bs10;
	emp::BitSet<10> bs10_1;
	bs10.Set(1,1); 								// bs10 = 00 0000 0010
	bs10_1.Set(3,1);
	bs10_1.SetByte(1,3);
	REQUIRE(bs10_1.count() == 3);	// bs10_1 = 11 00001000
	bs10_1 |= bs10;								// bs10_1 = 11 00001000 | 00 00000010 = 11 00001010
	REQUIRE(bs10_1.GetByte(0) == 10);
	REQUIRE(bs10_1.GetByte(1) == 3);
}

/**
 * Bitwise AND &
 */
void test_bitwise_and(){
	emp::BitSet<8> bs8;
	emp::BitSet<8> bs8_1;
	bs8.SetByte(0,13);		// bs8 = 00001101
	bs8_1.SetByte(0,10);	// bs8_1 = 00001010
	bs8_1 &= bs8;					// bs8_1 = 00001010 & 00001101 = 00001000
	REQUIRE(bs8_1.GetByte(0) == 8);
}

/**
 * NAND, NOR, EQU & SELF
 */
void test_more_comparators(){
	// NAND
	emp::BitSet<8> bs8_1;
	emp::BitSet<8> bs8_2;
	bs8_1.SetAll();
	REQUIRE(bs8_1.NAND(bs8_2).All());
	bs8_2.flip(1);
	bs8_1.NAND_SELF(bs8_2);
	REQUIRE(bs8_1.Any());
	REQUIRE( !(bs8_1.Get(1)) );

	// NOR
	bs8_1.SetAll();
	bs8_2.Clear();
	REQUIRE(bs8_1.NOR(bs8_2).None());
	bs8_1.flip(1);
	bs8_1.NOR_SELF(bs8_2);
	REQUIRE(bs8_1.Get(1));

	// EQU
	bs8_1.Clear();
	bs8_2.SetAll();
	REQUIRE( (bs8_1.EQU(bs8_2).None()) );
	bs8_2.Clear();
	bs8_2.EQU_SELF(bs8_1);
	REQUIRE(bs8_2.All());
}

/**
 * Random bitset
 */
void test_random(){
	emp::Random rndm;
	emp::BitSet<8> bs8(rndm);
	bs8.Randomize(rndm, 1);
	REQUIRE(bs8.all());
	bs8.Randomize(rndm, 0);
	REQUIRE(bs8.none());
}

/**
 * Copy
 */
void test_copy(){
	emp::BitSet<10> bs10;
	bs10.SetAll();
	bs10.flip(0,5);

	emp::BitSet<10> bs10_1;
	bs10_1 = bs10;
	REQUIRE(bs10 == bs10_1);
}

/**
 * Comparators (>=,>,==,!=,<,<=)
 */
void test_comparators(){
	emp::BitSet<10> bs10;
	emp::BitSet<10> bs10_1;
	bs10_1.SetAll();
	REQUIRE(bs10_1 != bs10);
	REQUIRE(bs10_1 > bs10);
	bs10.SetAll();
	REQUIRE(bs10_1 >= bs10);
	REQUIRE(bs10_1 <= bs10);
	REQUIRE(bs10_1 == bs10);
	REQUIRE(!(bs10_1 < bs10));
	bs10.Clear();
	REQUIRE( (bs10 < bs10_1) );
}

/**
 * Export
 */
void test_export(){
	emp::BitSet<8> bs8;
	bs8.SetAll();
	REQUIRE(bs8.count() == 8);
	emp::BitSet<10> bs10 = bs8.Export<10>();
	REQUIRE(bs10.size() == 10);
	REQUIRE(bs10.GetByte(0) == 255);
	REQUIRE(bs10.GetByte(1) == 0);
}

/**
 * Import
 */
void test_import(){
	emp::BitSet<8> bs8;
	emp::BitSet<20> bs20;
	bs20[5] = 1;

	bs8.Import(bs20);
	REQUIRE(bs8[5]);

	emp::BitSet<10> bs10;
	bs10.SetAll();
	bs20.Import(bs10);
	REQUIRE(bs20.count() == 10);
}

/**
 * Print
 */
void test_print(){
	emp::BitSet<8> bs8;
	bs8.SetAll();
	bs8.Set(1, false);

	std::stringstream ss;
	bs8.Print(ss);
	REQUIRE(ss.str() == "11111101");
	ss.str(std::string());

	ss << bs8;
	REQUIRE(ss.str() == "11111101");
	ss.str(std::string());

	bs8.PrintArray(ss);
	REQUIRE(ss.str() == "10111111");
	ss.str(std::string());

	bs8.Clear();
	bs8.Set(1, true);
	bs8.Set(4, true);
	bs8.PrintOneIDs(ss);
	REQUIRE(ss.str() == "1 4 ");
}


TEST_CASE("Test BitSet", "[tools]")
{
	test_status();
	test_size();
	test_flip();
	test_bit();
	test_byte();
	test_find();
	test_count();
	test_get_ones();
	test_copy();
	test_shift();
	test_comparators();
	test_bitwise_or();
	test_bitwise_xor();
	test_bitwise_and();
	test_more_comparators();
	test_random();
	test_export();
	test_import();
	test_print();
}
