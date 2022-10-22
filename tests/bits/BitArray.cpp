/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file BitArray.cpp
 */

#define EMP_DECORATE(X) [X]
#define EMP_DECORATE_PAIR(X,Y) [X-Y]

#include <algorithm>
#include <climits>
#include <deque>
#include <fstream>
#include <limits>
#include <numeric>
#include <ratio>
#include <sstream>
#include <string>
#include <unordered_set>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>

#include "emp/base/map.hpp"
#include "emp/bits/BitArray.hpp"
#include "emp/compiler/RegEx.hpp"
#include "emp/data/DataNode.hpp"
#include "emp/functional/FunctionSet.hpp"
#include "emp/math/Random.hpp"
#include "emp/tools/attrs.hpp"
#include "emp/tools/TypeTracker.hpp"


template <size_t... VALS> struct TestBitArrayConstruct;

template <size_t VAL1, size_t... VALS>
struct TestBitArrayConstruct<VAL1, VALS...> {
  static void Run() {
    emp::BitArray<VAL1> bit_array;
    REQUIRE( bit_array.GetSize() == VAL1 );
    REQUIRE( bit_array.CountOnes() == 0 );
    for (size_t i = 0; i < VAL1; i++) bit_array[i] = true;
    REQUIRE( bit_array.CountOnes() == VAL1 );

    TestBitArrayConstruct<VALS...>::Run();
  }
};

// Base case for constructors...
template <>
struct TestBitArrayConstruct<> {
  static void Run(){}
};

TEST_CASE("1: Test BitArray Constructors", "[bits]"){
  // Create a size 50 bit vector, default to all zeros.
  emp::BitArray<50> ba1;
  REQUIRE( ba1.GetSize() == 50 );
  REQUIRE( ba1.CountOnes() == 0 );
  REQUIRE( (~ba1).CountOnes() == 50 );

  // Create a size 1000 BitArray, default to all ones.
  emp::BitArray<1000> ba2(true);
  REQUIRE( ba2.GetSize() == 1000 );
  REQUIRE( ba2.CountOnes() == 1000 );

  // Try a range of BitArray sizes, from 1 to 200.
  TestBitArrayConstruct<1,2,7,8,9,15,16,17,31,32,33,63,64,65,127,128,129,191,192,193,200>::Run();

  // Build a relatively large BitArray.
  emp::BitArray<1000000> ba4;
  for (size_t i = 0; i < ba4.GetSize(); i += 100) ba4[i].Toggle();
  REQUIRE( ba4.CountOnes() == 10000 );

  // Try out the copy constructor.
  emp::BitArray<1000000> ba5(ba4);
  REQUIRE( ba5.GetSize() == 1000000 );
  REQUIRE( ba5.CountOnes() == 10000 );

  // Construct from std::bitset.
  std::bitset<6> bit_set;
  bit_set[1] = 1;   bit_set[2] = 1;   bit_set[4] = 1;
  emp::BitArray<6> ba7(bit_set);
  REQUIRE( ba7.GetSize() == 6 );
  REQUIRE( ba7.CountOnes() == 3 );

  // Construct from string.
  std::string bit_string = "10011001010000011101";
  emp::BitArray<20> ba8(bit_string);
  REQUIRE( ba8.GetSize() == 20 );
  REQUIRE( ba8.CountOnes() == 9 );

  // Some random BitArrays
  emp::Random random;
  emp::BitArray<1000> ba9(random);            // 50/50 chance for each bit.
  const size_t ba9_ones = ba9.CountOnes();
  REQUIRE( ba9_ones >= 400 );
  REQUIRE( ba9_ones <= 600 );

  emp::BitArray<1000> ba10(random, 0.8);      // 80% chance of ones.
  const size_t ba10_ones = ba10.CountOnes();
  REQUIRE( ba10_ones >= 750 );
  REQUIRE( ba10_ones <= 850 );

  emp::BitArray<1000> ba11(random, 117);      // Exactly 117 ones, randomly placed.
  const size_t ba11_ones = ba11.CountOnes();
  REQUIRE( ba11_ones == 117 );

  emp::BitArray<13> ba12({1,0,0,0,1,1,1,0,0,0,1,1,1}); // Construct with initializer list.
  REQUIRE( ba12.GetSize() == 13 );
  REQUIRE( ba12.CountOnes() == 7 );
}


template <size_t... VALS> struct TestBVAssign;

template <size_t VAL1, size_t... VALS>
struct TestBVAssign<VAL1, VALS...> {
  static void Run() {
    emp::BitArray<VAL1> ba;

    // Copy to a second ba, make changes, then copy back.
    emp::BitArray<VAL1> ba2;

    for (size_t i = 1; i < ba2.GetSize(); i += 2) {
      ba2[i] = 1;
    }

    ba = ba2;

    REQUIRE( ba.CountOnes() == ba.GetSize()/2 );

    // Try copying in from an std::bitset.
    std::bitset<VAL1> bit_set;
    size_t num_ones = 0;
    if constexpr (VAL1 > 1)   { bit_set[1] = 1; num_ones++; }
    if constexpr (VAL1 > 22)  { bit_set[22] = 1; num_ones++; }
    if constexpr (VAL1 > 444) { bit_set[444] = 1; num_ones++; }

    ba2 = bit_set;  // Copy in an std::bitset.

    REQUIRE( ba2.GetSize() == VAL1 );
    REQUIRE( ba2.CountOnes() == num_ones );

    // Try copying from an std::string
    std::string bit_string = "100110010100000111011001100101000001110110011001010000011101";
    while (bit_string.size() < VAL1) bit_string += bit_string;
    bit_string.resize(VAL1);

    num_ones = 0;
    for (char x : bit_string) if (x == '1') num_ones++;

    ba2 = bit_string;

    REQUIRE( ba2.GetSize() == VAL1 );
    REQUIRE( ba2.CountOnes() == num_ones );

    TestBVAssign<VALS...>::Run();
  }
};

// Base case for constructors...
template<> struct TestBVAssign<> { static void Run(){} };

TEST_CASE("2: Test BitArray Assignemnts", "[bits]"){
  // Try a range of BitArray sizes, from 1 to 200.
  TestBVAssign<1,2,7,8,9,15,16,17,31,32,33,63,64,65,127,128,129,191,192,193,200,1023,1024,1025,1000000>::Run();
}


TEST_CASE("3: Test Simple BitArray Accessors", "[bits]"){
  emp::BitArray<1>  ba1(true);
  emp::BitArray<8>  ba8( "10001101" );
  emp::BitArray<32> ba32( "10001101100011011000110110001101" );
  emp::BitArray<64> ba64( "1000110110001101100000011000110000001101100000000000110110001101" );
  emp::BitArray<75> ba75( "010001011100010111110000011110100011111000001110100000111110010011111000011" );

  emp::Random random;
  emp::BitArray<1000> ba1k(random, 0.75);

  // Make sure all sizes are correct.
  REQUIRE( ba1.GetSize() == 1 );
  REQUIRE( ba8.GetSize() == 8 );
  REQUIRE( ba32.GetSize() == 32 );
  REQUIRE( ba64.GetSize() == 64 );
  REQUIRE( ba75.GetSize() == 75 );
  REQUIRE( ba1k.GetSize() == 1000 );

  // Check byte counts (should always round up!)
  REQUIRE( ba1.GetNumBytes() == 1 );     // round up!
  REQUIRE( ba8.GetNumBytes() == 1 );
  REQUIRE( ba32.GetNumBytes() == 4 );
  REQUIRE( ba64.GetNumBytes() == 8 );
  REQUIRE( ba75.GetNumBytes() == 10 );   // round up!
  REQUIRE( ba1k.GetNumBytes() == 125 );

  // How many states can be represented in each size of BitArray?
  REQUIRE( ba1.GetNumStates() == 2.0 );
  REQUIRE( ba8.GetNumStates() == 256.0 );
  REQUIRE( ba32.GetNumStates() == 4294967296.0 );
  REQUIRE( ba64.GetNumStates() >= 18446744073709551610.0 );
  REQUIRE( ba64.GetNumStates() <= 18446744073709551720.0 );
  REQUIRE( ba75.GetNumStates() >= 37778931862957161709560.0 );
  REQUIRE( ba75.GetNumStates() <= 37778931862957161709570.0 );
  REQUIRE( ba1k.GetNumStates() == emp::Pow2(1000) );

  // Test Get()
  REQUIRE( ba1.Get(0) == 1 );
  REQUIRE( ba8.Get(0) == 1 );
  REQUIRE( ba8.Get(4) == 1 );
  REQUIRE( ba8.Get(6) == 0 );
  REQUIRE( ba8.Get(7) == 1 );
  REQUIRE( ba75.Get(0) == 0 );
  REQUIRE( ba75.Get(1) == 1 );
  REQUIRE( ba75.Get(72) == 0 );
  REQUIRE( ba75.Get(73) == 1 );
  REQUIRE( ba75.Get(74) == 1 );

  // Test Has() (including out of range)
  REQUIRE( ba1.Has(0) == true );
  REQUIRE( ba1.Has(1) == false );
  REQUIRE( ba1.Has(1000000) == false );

  REQUIRE( ba8.Has(0) == true );
  REQUIRE( ba8.Has(4) == true );
  REQUIRE( ba8.Has(6) == false );
  REQUIRE( ba8.Has(7) == true );
  REQUIRE( ba8.Has(8) == false );

  REQUIRE( ba75.Has(0) == false );
  REQUIRE( ba75.Has(1) == true );
  REQUIRE( ba75.Has(72) == false );
  REQUIRE( ba75.Has(73) == true );
  REQUIRE( ba75.Has(74) == true );
  REQUIRE( ba75.Has(75) == false );
  REQUIRE( ba75.Has(79) == false );
  REQUIRE( ba75.Has(1000000) == false );

  // Test Set(), changing in most (but not all) cases.
  ba1.Set(0, 0);
  REQUIRE( ba1.Get(0) == 0 );
  ba8.Set(0, 1);                // Already a 1!
  REQUIRE( ba8.Get(0) == 1 );
  ba8.Set(4, 0);
  REQUIRE( ba8.Get(4) == 0 );
  ba8.Set(6, 1);
  REQUIRE( ba8.Get(6) == 1 );
  ba8.Set(7, 0);
  REQUIRE( ba8.Get(7) == 0 );
  ba75.Set(0, 0);               // Already a 0!
  REQUIRE( ba75.Get(0) == 0 );
  ba75.Set(1, 0);
  REQUIRE( ba75.Get(1) == 0 );
  ba75.Set(72);                 // No second arg!
  REQUIRE( ba75.Get(72) == 1 );
  ba75.Set(73);                 // No second arg AND already a 1!
  REQUIRE( ba75.Get(73) == 1 );
  ba75.Set(74, 0);
  REQUIRE( ba75.Get(74) == 0 );
}

TEST_CASE("4: Test BitArray Set*, Clear* and Toggle* Accessors", "[bits]") {
  // Now try range-based accessors on a single bit.
  emp::BitArray<1> ba1(false);  REQUIRE( ba1[0] == false );   REQUIRE( ba1.CountOnes() == 0 );
  ba1.Set(0);                 REQUIRE( ba1[0] == true );    REQUIRE( ba1.CountOnes() == 1 );
  ba1.Clear(0);               REQUIRE( ba1[0] == false );   REQUIRE( ba1.CountOnes() == 0 );
  ba1.Toggle(0);              REQUIRE( ba1[0] == true );    REQUIRE( ba1.CountOnes() == 1 );
  ba1.Clear();                REQUIRE( ba1[0] == false );   REQUIRE( ba1.CountOnes() == 0 );
  ba1.SetAll();               REQUIRE( ba1[0] == true );    REQUIRE( ba1.CountOnes() == 1 );
  ba1.Toggle();               REQUIRE( ba1[0] == false );   REQUIRE( ba1.CountOnes() == 0 );
  ba1.SetRange(0,1);          REQUIRE( ba1[0] == true );    REQUIRE( ba1.CountOnes() == 1 );
  ba1.Clear(0,1);             REQUIRE( ba1[0] == false );   REQUIRE( ba1.CountOnes() == 0 );
  ba1.Toggle(0,1);            REQUIRE( ba1[0] == true );    REQUIRE( ba1.CountOnes() == 1 );
  ba1.Set(0, false);          REQUIRE( ba1[0] == false );   REQUIRE( ba1.CountOnes() == 0 );
  ba1.SetRange(0,0);          REQUIRE( ba1[0] == false );   REQUIRE( ba1.CountOnes() == 0 );
  ba1.SetRange(1,1);          REQUIRE( ba1[0] == false );   REQUIRE( ba1.CountOnes() == 0 );

  // Test when a full byte is used.
  emp::BitArray<8> ba8( "10001101" );   REQUIRE(ba8.GetValue() == 177.0);  // 10110001
  ba8.Set(2);                         REQUIRE(ba8.GetValue() == 181.0);  // 10110101
  ba8.Set(0, 0);                      REQUIRE(ba8.GetValue() == 180.0);  // 10110100
  ba8.SetRange(1, 4);                 REQUIRE(ba8.GetValue() == 190.0);  // 10111110
  ba8.SetAll();                       REQUIRE(ba8.GetValue() == 255.0);  // 11111111
  ba8.Clear(3);                       REQUIRE(ba8.GetValue() == 247.0);  // 11110111
  ba8.Clear(5,5);                     REQUIRE(ba8.GetValue() == 247.0);  // 11110111
  ba8.Clear(5,7);                     REQUIRE(ba8.GetValue() == 151.0);  // 10010111
  ba8.Clear();                        REQUIRE(ba8.GetValue() ==   0.0);  // 00000000
  ba8.Toggle(4);                      REQUIRE(ba8.GetValue() ==  16.0);  // 00010000
  ba8.Toggle(4,6);                    REQUIRE(ba8.GetValue() ==  32.0);  // 00100000
  ba8.Toggle(0,3);                    REQUIRE(ba8.GetValue() ==  39.0);  // 00100111
  ba8.Toggle(7,8);                    REQUIRE(ba8.GetValue() == 167.0);  // 10100111
  ba8.Toggle();                       REQUIRE(ba8.GetValue() ==  88.0);  // 01011000

  // Test a full field.
  constexpr double ALL_64 = (double) ((uint64_t) -1);
  emp::BitArray<64> ba64( "11011000110110001101" );
  REQUIRE(ba64.GetValue() == 727835.0);
  ba64.Set(6);          REQUIRE(ba64.GetValue() == 727899.0);        // ...0 010110001101101011011
  ba64.Set(0, 0);       REQUIRE(ba64.GetValue() == 727898.0);        // ...0 010110001101101011010
  ba64.SetRange(4, 9);  REQUIRE(ba64.GetValue() == 728058.0);        // ...0 010110001101111111010
  ba64.SetAll();        REQUIRE(ba64.GetValue() == ALL_64);          // ...1 111111111111111111111
  ba64.Clear(2);        REQUIRE(ba64.GetValue() == ALL_64 - 4);      // ...1 111111111111111111011
  ba64.Clear(5,5);      REQUIRE(ba64.GetValue() == ALL_64 - 4);      // ...1 111111111111111111011
  ba64.Clear(5,7);      REQUIRE(ba64.GetValue() == ALL_64 - 100);    // ...1 111111111111110011011
  ba64.Clear();         REQUIRE(ba64.GetValue() == 0.0);             // ...0 000000000000000000000
  ba64.Toggle(19);      REQUIRE(ba64.GetValue() == emp::Pow2(19));   // ...0 010000000000000000000
  ba64.Toggle(15,20);   REQUIRE(ba64.GetValue() == 491520.0);        // ...0 001111000000000000000
  ba64.Toggle();        REQUIRE(ba64.GetValue() == ALL_64-491520.0); // ...1 110000111111111111111
  ba64.Toggle(0,64);    REQUIRE(ba64.GetValue() == 491520.0);        // ...0 001111000000000000000


  emp::BitArray<75> ba75( "010001011100010111110000011110100011111000001110100000111110010011111000011" );

  // Test a full + partial field.
  constexpr double ALL_88 = ((double) ((uint64_t) -1)) * emp::Pow2(24);
  emp::BitArray<88> ba88( "11011000110110001101" ); REQUIRE(ba88.GetValue() == 727835.0);
  REQUIRE(ba88.GetValue() == 727835.0);                              // ...0 010110001101100011011

  // Start with same tests as last time...
  ba88.Set(6);          REQUIRE(ba88.GetValue() == 727899.0);        // ...0 010110001101101011011
  ba88.Set(0, 0);       REQUIRE(ba88.GetValue() == 727898.0);        // ...0 010110001101101011010
  ba88.SetRange(4, 9);  REQUIRE(ba88.GetValue() == 728058.0);        // ...0 010110001101111111010
  ba88.SetAll();        REQUIRE(ba88.GetValue() == ALL_88);          // ...1 111111111111111111111
  ba88.Clear(2);        REQUIRE(ba88.GetValue() == ALL_88 - 4);      // ...1 111111111111111111011
  ba88.Clear(5,5);      REQUIRE(ba88.GetValue() == ALL_88 - 4);      // ...1 111111111111111111011
  ba88.Clear(5,7);      REQUIRE(ba88.GetValue() == ALL_88 - 100);    // ...1 111111111111110011011
  ba88.Clear();         REQUIRE(ba88.GetValue() == 0.0);             // ...0 000000000000000000000
  ba88.Toggle(19);      REQUIRE(ba88.GetValue() == emp::Pow2(19));   // ...0 010000000000000000000
  ba88.Toggle(15,20);   REQUIRE(ba88.GetValue() == 491520.0);        // ...0 001111000000000000000
  ba88.Toggle();        REQUIRE(ba88.GetValue() == ALL_88-491520.0); // ...1 110000111111111111111
  ba88.Toggle(0,88);    REQUIRE(ba88.GetValue() == 491520.0);        // ...0 001111000000000000000

  ba88 <<= 20;          REQUIRE(ba88.CountOnes() == 4);   // four ones, moved to bits 35-39
  ba88 <<= 27;          REQUIRE(ba88.CountOnes() == 4);   // four ones, moved to bits 62-65
  ba88 <<= 22;          REQUIRE(ba88.CountOnes() == 4);   // four ones, moved to bits 84-87
  ba88 <<= 1;           REQUIRE(ba88.CountOnes() == 3);   // three ones left, moved to bits 85-87
  ba88 <<= 2;           REQUIRE(ba88.CountOnes() == 1);   // one one left, at bit 87
  ba88 >>= 30;          REQUIRE(ba88.CountOnes() == 1);   // one one left, now at bit 57
  ba88.Toggle(50,80);   REQUIRE(ba88.CountOnes() == 29);  // Toggling 30 bits, only one was on.
  ba88.Clear(52,78);    REQUIRE(ba88.CountOnes() == 4);   // Leave two 1s on each side of range
  ba88.SetRange(64,66); REQUIRE(ba88.CountOnes() == 6);   // Set two more 1s, just into 2nd field.

  // A larger BitArray with lots of random tests.
  emp::Random random;
  emp::BitArray<1000> ba1k(random, 0.65);
  size_t num_ones = ba1k.CountOnes();  REQUIRE(num_ones > 550);
  ba1k.Toggle();                       REQUIRE(ba1k.CountOnes() == 1000 - num_ones);

  for (size_t test_id = 0; test_id < 10000; ++test_id) {
    size_t val1 = random.GetUInt(1000);
    size_t val2 = random.GetUInt(1001);
    if (val1 > val2) std::swap(val1, val2);
    ba1k.Toggle(val1, val2);

    val1 = random.GetUInt(1000);
    val2 = random.GetUInt(1001);
    if (val1 > val2) std::swap(val1, val2);
    ba1k.Clear(val1, val2);

    val1 = random.GetUInt(1000);
    val2 = random.GetUInt(1001);
    if (val1 > val2) std::swap(val1, val2);
    ba1k.SetRange(val1, val2);
  }

  // Test Any(), All() and None()
  emp::BitArray<6> ba_empty = "000000";
  emp::BitArray<6> ba_mixed = "010101";
  emp::BitArray<6> ba_full  = "111111";

  REQUIRE(ba_empty.Any() == false);
  REQUIRE(ba_mixed.Any() == true);
  REQUIRE(ba_full.Any() == true);

  REQUIRE(ba_empty.All() == false);
  REQUIRE(ba_mixed.All() == false);
  REQUIRE(ba_full.All() == true);

  REQUIRE(ba_empty.None() == true);
  REQUIRE(ba_mixed.None() == false);
  REQUIRE(ba_full.None() == false);
}


TEST_CASE("5: Test Randomize() and variants", "[bits]") {
  emp::Random random;
  emp::BitArray<1000> ba;

  REQUIRE(ba.None() == true);

  // Do all of the random tests 10 times.
  for (size_t test_num = 0; test_num < 10; test_num++) {
    ba.Randomize(random);
    size_t num_ones = ba.CountOnes();
    REQUIRE(num_ones > 300);
    REQUIRE(num_ones < 700);

    // 85% Chance of 1
    ba.Randomize(random, 0.85);
    num_ones = ba.CountOnes();
    REQUIRE(num_ones > 700);
    REQUIRE(num_ones < 950);

    // 15% Chance of 1
    ba.Randomize(random, 0.15);
    num_ones = ba.CountOnes();
    REQUIRE(num_ones > 50);
    REQUIRE(num_ones < 300);

    // Try randomizing only a portion of the genome.
    uint64_t first_bits = ba.GetUInt64(0);
    ba.Randomize(random, 0.7, 64, 1000);

    REQUIRE(ba.GetUInt64(0) == first_bits);  // Make sure first bits haven't changed

    num_ones = ba.CountOnes();
    REQUIRE(num_ones > 500);                 // Expected with new randomization is ~665 ones.
    REQUIRE(num_ones < 850);

    // Try randomizing using specific numbers of ones.
    ba.ChooseRandom(random, 1);       REQUIRE(ba.CountOnes() == 1);
    ba.ChooseRandom(random, 12);      REQUIRE(ba.CountOnes() == 12);
    ba.ChooseRandom(random, 128);     REQUIRE(ba.CountOnes() == 128);
    ba.ChooseRandom(random, 507);     REQUIRE(ba.CountOnes() == 507);
    ba.ChooseRandom(random, 999);     REQUIRE(ba.CountOnes() == 999);

    // Test the probabilistic CHANGE functions.
    ba.Clear();                     REQUIRE(ba.CountOnes() == 0);   // Set all bits to 0.

    ba.FlipRandom(random, 0.3);     // Exprected: 300 ones (from flipping zeros)
    num_ones = ba.CountOnes();      REQUIRE(num_ones > 230);  REQUIRE(num_ones < 375);

    ba.FlipRandom(random, 0.3);     // Exprected: 420 ones (hit by ONE but not both flips)
    num_ones = ba.CountOnes();      REQUIRE(num_ones > 345);  REQUIRE(num_ones < 495);

    ba.SetRandom(random, 0.5);      // Expected: 710 (already on OR newly turned on)
    num_ones = ba.CountOnes();      REQUIRE(num_ones > 625);  REQUIRE(num_ones < 775);

    ba.SetRandom(random, 0.8);      // Expected: 942 (already on OR newly turned on)
    num_ones = ba.CountOnes();      REQUIRE(num_ones > 900);  REQUIRE(num_ones < 980);

    ba.ClearRandom(random, 0.2);    // Expected 753.6 (20% of those on now off)
    num_ones = ba.CountOnes();      REQUIRE(num_ones > 675);  REQUIRE(num_ones < 825);

    ba.FlipRandom(random, 0.5);     // Exprected: 500 ones (each bit has a 50% chance of flipping)
    num_ones = ba.CountOnes();      REQUIRE(num_ones > 425);  REQUIRE(num_ones < 575);


    // Repeat with fixed-sized changes.
    ba.Clear();                        REQUIRE(ba.CountOnes() == 0);     // Set all bits to 0.

    ba.FlipRandomCount(random, 123);   // Flip exactly 123 bits to 1.
    num_ones = ba.CountOnes();         REQUIRE(num_ones == 123);

    ba.FlipRandomCount(random, 877);   // Flip exactly 877 bits; Expected 784.258 ones
    num_ones = ba.CountOnes();         REQUIRE(num_ones > 700);  REQUIRE(num_ones < 850);


    ba.SetAll();                       REQUIRE(ba.CountOnes() == 1000);  // Set all bits to 1.

    ba.ClearRandomCount(random, 123);
    num_ones = ba.CountOnes();         REQUIRE(num_ones == 877);

    ba.ClearRandomCount(random, 877);  // Clear exactly 877 bits; Expected 107.871 ones
    num_ones = ba.CountOnes();         REQUIRE(num_ones > 60);  REQUIRE(num_ones < 175);

    ba.SetRandomCount(random, 500);    // Half of the remaining ones should be set; 553.9355 expected.
    num_ones = ba.CountOnes();         REQUIRE(num_ones > 485);  REQUIRE(num_ones < 630);


    ba.Clear();                        REQUIRE(ba.CountOnes() == 0);     // Set all bits to 0.
    ba.SetRandomCount(random, 567);    // Half of the remaining ones should be set; 607.871 expected.
    num_ones = ba.CountOnes();         REQUIRE(num_ones == 567);
  }


  // During randomization, make sure each bit position is set appropriately.
  std::vector<size_t> one_counts(1000, 0);

  for (size_t test_num = 0; test_num < 1000; ++test_num) {
    // Set bits with different probabilities in different ranges.
    ba.Clear();
    ba.Randomize(random, 0.5,  100, 250);
    ba.Randomize(random, 0.25, 250, 400);
    ba.Randomize(random, 0.75, 400, 550);
    ba.Randomize(random, 0.10, 550, 700);
    ba.Randomize(random, 0.98, 700, 850);

    // Keep count of how many times each position was a one.
    for (size_t i = 0; i < ba.GetSize(); ++i) {
      if (ba.Get(i)) one_counts[i]++;
    }
  }

  // Check if the counts are reasonable.
  for (size_t i = 0;   i < 100; i++)  { REQUIRE(one_counts[i] == 0); }
  for (size_t i = 100; i < 250; i++)  { REQUIRE(one_counts[i] > 420);  REQUIRE(one_counts[i] < 580); }
  for (size_t i = 250; i < 400; i++)  { REQUIRE(one_counts[i] > 190);  REQUIRE(one_counts[i] < 320); }
  for (size_t i = 400; i < 550; i++)  { REQUIRE(one_counts[i] > 680);  REQUIRE(one_counts[i] < 810); }
  for (size_t i = 550; i < 700; i++)  { REQUIRE(one_counts[i] >  60);  REQUIRE(one_counts[i] < 150); }
  for (size_t i = 700; i < 850; i++)  { REQUIRE(one_counts[i] > 950);  REQUIRE(one_counts[i] < 999); }
  for (size_t i = 850; i < 1000; i++) { REQUIRE(one_counts[i] == 0); }
}

TEST_CASE("6: Test getting and setting whole chunks of bits", "[bits]") {
  constexpr size_t num_bits = 145;
  constexpr size_t num_bytes = 19;

  emp::BitArray<num_bits> ba;
  REQUIRE(ba.GetSize() == num_bits);
  REQUIRE(ba.GetNumBytes() == num_bytes);

  // All bytes should start out empty.
  for (size_t i = 0; i < num_bytes; i++) REQUIRE(ba.GetByte(i) == 0);

  ba.SetByte(2, 11);
  REQUIRE(ba.GetByte(2) == 11);

  REQUIRE(ba.GetValue() == 720896.0);

  ba.SetByte(5, 7);
  REQUIRE(ba.GetByte(0) == 0);
  REQUIRE(ba.GetByte(1) == 0);
  REQUIRE(ba.GetByte(2) == 11);
  REQUIRE(ba.GetByte(3) == 0);
  REQUIRE(ba.GetByte(4) == 0);
  REQUIRE(ba.GetByte(5) == 7);
  REQUIRE(ba.GetByte(6) == 0);
  REQUIRE(ba.CountOnes() == 6);

  for (size_t i = 0; i < num_bytes; i++) REQUIRE(ba.GetByte(i) == ba.GetUInt8(i));

  REQUIRE(ba.GetUInt16(0) == 0);
  REQUIRE(ba.GetUInt16(1) == 11);
  REQUIRE(ba.GetUInt16(2) == 1792);
  REQUIRE(ba.GetUInt16(3) == 0);

  REQUIRE(ba.GetUInt32(0) == 720896);
  REQUIRE(ba.GetUInt32(1) == 1792);
  REQUIRE(ba.GetUInt32(2) == 0);

  REQUIRE(ba.GetUInt64(0) == 7696582115328);
  REQUIRE(ba.GetUInt64(1) == 0);

  ba.SetUInt64(0, 12345678901234);
  ba.SetUInt32(2, 2000000);
  ba.SetUInt16(7, 7777);
  ba.SetUInt8(17, 17);

  REQUIRE(ba.GetUInt64(0) == 12345678901234);
  REQUIRE(ba.GetUInt32(2) == 2000000);
  REQUIRE(ba.GetUInt16(7) == 7777);
  REQUIRE(ba.GetUInt8(17) == 17);

  ba.Clear();
  ba.SetUInt16AtBit(40, 40);

  REQUIRE(ba.GetUInt16AtBit(40) == 40);

  REQUIRE(ba.GetUInt8(5) == 40);
  REQUIRE(ba.GetUInt8AtBit(40) == 40);
  REQUIRE(ba.GetUInt32AtBit(40) == 40);
  REQUIRE(ba.GetUInt64AtBit(40) == 40);

  REQUIRE(ba.GetUInt16AtBit(38) == 160);
  REQUIRE(ba.GetUInt16AtBit(39) == 80);
  REQUIRE(ba.GetUInt16AtBit(41) == 20);
  REQUIRE(ba.GetUInt16AtBit(42) == 10);

  REQUIRE(ba.GetUInt8AtBit(38) == 160);
  REQUIRE(ba.GetUInt8AtBit(37) == 64);
  REQUIRE(ba.GetUInt8AtBit(36) == 128);
  REQUIRE(ba.GetUInt8AtBit(35) == 0);
}

TEST_CASE("7: Test functions that analyze and manipulate ones", "[bits]") {

  emp::BitArray<16> ba = "0001000100001110";

  REQUIRE(ba.GetSize() == 16);
  REQUIRE(ba.CountOnes() == 5);

  // Make sure we can find all of the ones.
  REQUIRE(ba.FindOne() == 3);
  REQUIRE(ba.FindOne(4) == 7);
  REQUIRE(ba.FindOne(5) == 7);
  REQUIRE(ba.FindOne(6) == 7);
  REQUIRE(ba.FindOne(7) == 7);
  REQUIRE(ba.FindOne(8) == 12);
  REQUIRE(ba.FindOne(13) == 13);
  REQUIRE(ba.FindOne(14) == 14);
  REQUIRE(ba.FindOne(15) == -1);

  // Get all of the ones at once and make sure they're there.
  emp::vector<size_t> ones = ba.GetOnes();
  REQUIRE(ones.size() == 5);
  REQUIRE(ones[0] == 3);
  REQUIRE(ones[1] == 7);
  REQUIRE(ones[2] == 12);
  REQUIRE(ones[3] == 13);
  REQUIRE(ones[4] == 14);

  // Try finding the length of the longest segment of ones.
  REQUIRE(ba.LongestSegmentOnes() == 3);

  // Identify the final one.
  REQUIRE(ba.FindMaxOne() == 14);

  // Pop all ones, one at a time.
  REQUIRE(ba.PopOne() == 3);
  REQUIRE(ba.PopOne() == 7);
  REQUIRE(ba.PopOne() == 12);
  REQUIRE(ba.PopOne() == 13);
  REQUIRE(ba.PopOne() == 14);
  REQUIRE(ba.PopOne() == -1);

  REQUIRE(ba.CountOnes() == 0);
  REQUIRE(ba.LongestSegmentOnes() == 0);
  REQUIRE(ba.FindMaxOne() == -1);


  ba.SetAll();                             // 1111111111111111
  REQUIRE(ba.LongestSegmentOnes() == 16);
  ba[8] = 0;                               // 1111111101111111
  REQUIRE(ba.LongestSegmentOnes() == 8);
  ba[4] = 0;                               // 1111011101111111
  REQUIRE(ba.LongestSegmentOnes() == 7);

  // Try again with Find, this time with a random sequence of ones.
  emp::Random random;
  ba.Randomize(random);
  size_t count = 0;
  for (int i = ba.FindOne(); i != -1; i = ba.FindOne(i+1)) count++;
  REQUIRE(count == ba.CountOnes());

}

TEST_CASE("8: Test printing and string functions.", "[bits]") {
  emp::BitArray<6> ba6("000111");

  REQUIRE(ba6.ToString() == "000111");
  REQUIRE(ba6.ToBinaryString() == "111000");
  REQUIRE(ba6.ToIDString() == "3 4 5");
  REQUIRE(ba6.ToIDString() == "3 4 5");
  REQUIRE(ba6.ToRangeString() == "3-5");

  emp::BitArray<64> ba64("0001110000000000000100000000000001000110000001000001000100000001");

  REQUIRE(ba64.ToString()       == "0001110000000000000100000000000001000110000001000001000100000001");
  REQUIRE(ba64.ToBinaryString() == "1000000010001000001000000110001000000000000010000000000000111000");
  REQUIRE(ba64.ToIDString() == "3 4 5 19 33 37 38 45 51 55 63");
  REQUIRE(ba64.ToIDString(",") == "3,4,5,19,33,37,38,45,51,55,63");
  REQUIRE(ba64.ToRangeString() == "3-5,19,33,37-38,45,51,55,63");

  emp::BitArray<65> ba65("00011110000000000001000000000000010001100000010000010001000000111");

  REQUIRE(ba65.ToString()       == "00011110000000000001000000000000010001100000010000010001000000111");
  REQUIRE(ba65.ToBinaryString() == "11100000010001000001000000110001000000000000010000000000001111000");
  REQUIRE(ba65.ToIDString()     == "3 4 5 6 19 33 37 38 45 51 55 62 63 64");
  REQUIRE(ba65.ToIDString(",")  == "3,4,5,6,19,33,37,38,45,51,55,62,63,64");
  REQUIRE(ba65.ToRangeString()  == "3-6,19,33,37-38,45,51,55,62-64");
}

TEST_CASE("9: Test Boolean logic and shifting functions.", "[bits]") {
  const emp::BitArray<8> input1 = "00001111";
  const emp::BitArray<8> input2 = "00110011";
  const emp::BitArray<8> input3 = "01010101";

  // Test *_SELF() Boolean Logic functions.
  emp::BitArray<8> ba;       REQUIRE(ba == emp::BitArray<8>("00000000"));
  ba.NOT_SELF();           REQUIRE(ba == emp::BitArray<8>("11111111"));
  ba.AND_SELF(input1);     REQUIRE(ba == emp::BitArray<8>("00001111"));
  ba.AND_SELF(input1);     REQUIRE(ba == emp::BitArray<8>("00001111"));
  ba.AND_SELF(input2);     REQUIRE(ba == emp::BitArray<8>("00000011"));
  ba.AND_SELF(input3);     REQUIRE(ba == emp::BitArray<8>("00000001"));

  ba.OR_SELF(input1);      REQUIRE(ba == emp::BitArray<8>("00001111"));
  ba.OR_SELF(input1);      REQUIRE(ba == emp::BitArray<8>("00001111"));
  ba.OR_SELF(input3);      REQUIRE(ba == emp::BitArray<8>("01011111"));
  ba.OR_SELF(input2);      REQUIRE(ba == emp::BitArray<8>("01111111"));

  ba.NAND_SELF(input1);    REQUIRE(ba == emp::BitArray<8>("11110000"));
  ba.NAND_SELF(input1);    REQUIRE(ba == emp::BitArray<8>("11111111"));
  ba.NAND_SELF(input2);    REQUIRE(ba == emp::BitArray<8>("11001100"));
  ba.NAND_SELF(input3);    REQUIRE(ba == emp::BitArray<8>("10111011"));

  ba.NOR_SELF(input1);     REQUIRE(ba == emp::BitArray<8>("01000000"));
  ba.NOR_SELF(input1);     REQUIRE(ba == emp::BitArray<8>("10110000"));
  ba.NOR_SELF(input2);     REQUIRE(ba == emp::BitArray<8>("01001100"));
  ba.NOR_SELF(input3);     REQUIRE(ba == emp::BitArray<8>("10100010"));

  ba.XOR_SELF(input1);     REQUIRE(ba == emp::BitArray<8>("10101101"));
  ba.XOR_SELF(input1);     REQUIRE(ba == emp::BitArray<8>("10100010"));
  ba.XOR_SELF(input2);     REQUIRE(ba == emp::BitArray<8>("10010001"));
  ba.XOR_SELF(input3);     REQUIRE(ba == emp::BitArray<8>("11000100"));

  ba.EQU_SELF(input1);     REQUIRE(ba == emp::BitArray<8>("00110100"));
  ba.EQU_SELF(input1);     REQUIRE(ba == emp::BitArray<8>("11000100"));
  ba.EQU_SELF(input2);     REQUIRE(ba == emp::BitArray<8>("00001000"));
  ba.EQU_SELF(input3);     REQUIRE(ba == emp::BitArray<8>("10100010"));

  ba.NOT_SELF();           REQUIRE(ba == emp::BitArray<8>("01011101"));

  // Test regular Boolean Logic functions.
  ba.Clear();                            REQUIRE(ba == emp::BitArray<8>("00000000"));
  emp::BitArray<8> ba1 = ba.NOT();         REQUIRE(ba1 == emp::BitArray<8>("11111111"));

  ba1 = ba1.AND(input1);                 REQUIRE(ba1 == emp::BitArray<8>("00001111"));
  emp::BitArray<8> ba2 = ba1.AND(input1);  REQUIRE(ba2 == emp::BitArray<8>("00001111"));
  emp::BitArray<8> ba3 = ba2.AND(input2);  REQUIRE(ba3 == emp::BitArray<8>("00000011"));
  emp::BitArray<8> ba4 = ba3.AND(input3);  REQUIRE(ba4 == emp::BitArray<8>("00000001"));

  ba1 = ba4.OR(input1);      REQUIRE(ba1 == emp::BitArray<8>("00001111"));
  ba2 = ba1.OR(input1);      REQUIRE(ba2 == emp::BitArray<8>("00001111"));
  ba3 = ba2.OR(input3);      REQUIRE(ba3 == emp::BitArray<8>("01011111"));
  ba4 = ba3.OR(input2);      REQUIRE(ba4 == emp::BitArray<8>("01111111"));

  ba1 = ba4.NAND(input1);    REQUIRE(ba1 == emp::BitArray<8>("11110000"));
  ba2 = ba1.NAND(input1);    REQUIRE(ba2 == emp::BitArray<8>("11111111"));
  ba3 = ba2.NAND(input2);    REQUIRE(ba3 == emp::BitArray<8>("11001100"));
  ba4 = ba3.NAND(input3);    REQUIRE(ba4 == emp::BitArray<8>("10111011"));

  ba1 = ba4.NOR(input1);     REQUIRE(ba1 == emp::BitArray<8>("01000000"));
  ba2 = ba1.NOR(input1);     REQUIRE(ba2 == emp::BitArray<8>("10110000"));
  ba3 = ba2.NOR(input2);     REQUIRE(ba3 == emp::BitArray<8>("01001100"));
  ba4 = ba3.NOR(input3);     REQUIRE(ba4 == emp::BitArray<8>("10100010"));

  ba1 = ba4.XOR(input1);     REQUIRE(ba1 == emp::BitArray<8>("10101101"));
  ba2 = ba1.XOR(input1);     REQUIRE(ba2 == emp::BitArray<8>("10100010"));
  ba3 = ba2.XOR(input2);     REQUIRE(ba3 == emp::BitArray<8>("10010001"));
  ba4 = ba3.XOR(input3);     REQUIRE(ba4 == emp::BitArray<8>("11000100"));

  ba1 = ba4.EQU(input1);     REQUIRE(ba1 == emp::BitArray<8>("00110100"));
  ba2 = ba1.EQU(input1);     REQUIRE(ba2 == emp::BitArray<8>("11000100"));
  ba3 = ba2.EQU(input2);     REQUIRE(ba3 == emp::BitArray<8>("00001000"));
  ba4 = ba3.EQU(input3);     REQUIRE(ba4 == emp::BitArray<8>("10100010"));

  ba = ba4.NOT();            REQUIRE(ba == emp::BitArray<8>("01011101"));


  // Test Boolean Logic operators.
  ba.Clear();               REQUIRE(ba == emp::BitArray<8>("00000000"));
  ba1 = ~ba;                REQUIRE(ba1 == emp::BitArray<8>("11111111"));

  ba1 = ba1 & input1;       REQUIRE(ba1 == emp::BitArray<8>("00001111"));
  ba2 = ba1 & input1;       REQUIRE(ba2 == emp::BitArray<8>("00001111"));
  ba3 = ba2 & input2;       REQUIRE(ba3 == emp::BitArray<8>("00000011"));
  ba4 = ba3 & input3;       REQUIRE(ba4 == emp::BitArray<8>("00000001"));

  ba1 = ba4 | input1;       REQUIRE(ba1 == emp::BitArray<8>("00001111"));
  ba2 = ba1 | input1;       REQUIRE(ba2 == emp::BitArray<8>("00001111"));
  ba3 = ba2 | input3;       REQUIRE(ba3 == emp::BitArray<8>("01011111"));
  ba4 = ba3 | input2;       REQUIRE(ba4 == emp::BitArray<8>("01111111"));

  ba1 = ~(ba4 & input1);    REQUIRE(ba1 == emp::BitArray<8>("11110000"));
  ba2 = ~(ba1 & input1);    REQUIRE(ba2 == emp::BitArray<8>("11111111"));
  ba3 = ~(ba2 & input2);    REQUIRE(ba3 == emp::BitArray<8>("11001100"));
  ba4 = ~(ba3 & input3);    REQUIRE(ba4 == emp::BitArray<8>("10111011"));

  ba1 = ~(ba4 | input1);    REQUIRE(ba1 == emp::BitArray<8>("01000000"));
  ba2 = ~(ba1 | input1);    REQUIRE(ba2 == emp::BitArray<8>("10110000"));
  ba3 = ~(ba2 | input2);    REQUIRE(ba3 == emp::BitArray<8>("01001100"));
  ba4 = ~(ba3 | input3);    REQUIRE(ba4 == emp::BitArray<8>("10100010"));

  ba1 = ba4 ^ input1;       REQUIRE(ba1 == emp::BitArray<8>("10101101"));
  ba2 = ba1 ^ input1;       REQUIRE(ba2 == emp::BitArray<8>("10100010"));
  ba3 = ba2 ^ input2;       REQUIRE(ba3 == emp::BitArray<8>("10010001"));
  ba4 = ba3 ^ input3;       REQUIRE(ba4 == emp::BitArray<8>("11000100"));

  ba1 = ~(ba4 ^ input1);    REQUIRE(ba1 == emp::BitArray<8>("00110100"));
  ba2 = ~(ba1 ^ input1);    REQUIRE(ba2 == emp::BitArray<8>("11000100"));
  ba3 = ~(ba2 ^ input2);    REQUIRE(ba3 == emp::BitArray<8>("00001000"));
  ba4 = ~(ba3 ^ input3);    REQUIRE(ba4 == emp::BitArray<8>("10100010"));

  ba = ~ba4;                REQUIRE(ba == emp::BitArray<8>("01011101"));


  // Test COMPOUND Boolean Logic operators.
  ba = "11111111";    REQUIRE(ba == emp::BitArray<8>("11111111"));

  ba &= input1;       REQUIRE(ba == emp::BitArray<8>("00001111"));
  ba &= input1;       REQUIRE(ba == emp::BitArray<8>("00001111"));
  ba &= input2;       REQUIRE(ba == emp::BitArray<8>("00000011"));
  ba &= input3;       REQUIRE(ba == emp::BitArray<8>("00000001"));

  ba |= input1;       REQUIRE(ba == emp::BitArray<8>("00001111"));
  ba |= input1;       REQUIRE(ba == emp::BitArray<8>("00001111"));
  ba |= input3;       REQUIRE(ba == emp::BitArray<8>("01011111"));
  ba |= input2;       REQUIRE(ba == emp::BitArray<8>("01111111"));

  ba ^= input1;       REQUIRE(ba == emp::BitArray<8>("01110000"));
  ba ^= input1;       REQUIRE(ba == emp::BitArray<8>("01111111"));
  ba ^= input2;       REQUIRE(ba == emp::BitArray<8>("01001100"));
  ba ^= input3;       REQUIRE(ba == emp::BitArray<8>("00011001"));

  // Now some tests with BitArrays longer than one field.
  const emp::BitArray<80> bal80 =
    "00110111000101110001011100010111000101110001011100010111000101110001011100010111";
  REQUIRE( bal80.GetSize() == 80 );
  REQUIRE( bal80.CountOnes() == 41 );
  REQUIRE( (bal80 << 1) ==
           emp::BitArray<80>("00011011100010111000101110001011100010111000101110001011100010111000101110001011")
         );
  REQUIRE( (bal80 << 2) ==
           emp::BitArray<80>("00001101110001011100010111000101110001011100010111000101110001011100010111000101")
         );
  REQUIRE( (bal80 << 63) ==
           emp::BitArray<80>("00000000000000000000000000000000000000000000000000000000000000000110111000101110")
         );
  REQUIRE( (bal80 << 64) ==
           emp::BitArray<80>("00000000000000000000000000000000000000000000000000000000000000000011011100010111")
         );
  REQUIRE( (bal80 << 65) ==
           emp::BitArray<80>("00000000000000000000000000000000000000000000000000000000000000000001101110001011")
         );

  REQUIRE( (bal80 >> 1) ==
           emp::BitArray<80>("01101110001011100010111000101110001011100010111000101110001011100010111000101110")
         );
  REQUIRE( (bal80 >> 2) ==
           emp::BitArray<80>("11011100010111000101110001011100010111000101110001011100010111000101110001011100")
         );
  REQUIRE( (bal80 >> 63) ==
           emp::BitArray<80>("10001011100010111000000000000000000000000000000000000000000000000000000000000000")
         );
  REQUIRE( (bal80 >> 64) ==
           emp::BitArray<80>("00010111000101110000000000000000000000000000000000000000000000000000000000000000")
         );
  REQUIRE( (bal80 >> 65) ==
           emp::BitArray<80>("00101110001011100000000000000000000000000000000000000000000000000000000000000000")
         );
}



/// Ensures that
/// 1) A == B
/// 2) A and B can be constexprs or non-contexprs.
/// 3) A and B have the same values regardless of constexpr-ness.
#define CONSTEXPR_REQUIRE_EQ(A, B)       \
  {                                      \
    static_assert(A == B, #A " == " #B); \
    REQUIRE(A == B);                     \
  }



/**
 * Status booleans (Any, All, None)
 * as well as Clear and SetAll
 */
void test_status(){
  emp::BitArray<10> ba10;
  REQUIRE(!ba10.any());
  REQUIRE(ba10.none());
  REQUIRE(!ba10.all());
  ba10.SetAll();
  REQUIRE(ba10.all());
  ba10.Clear();
  REQUIRE(ba10.none());
}

/**
 * GetSize
 */
void test_size(){
  emp::BitArray<42> ba42;
  REQUIRE(ba42.size() == 42);

  emp::BitArray<35> ba35;
  REQUIRE(ba35.GetSize() == 35);

  emp::BitArray<1> ba1;
  REQUIRE(ba1.size() == 1);
}

/**
 * Flip and Toggle
 */
void test_flip(){
  emp::BitArray<2> ba2;  // ba2 = 00
  ba2.flip(0);        // ba2 = 01
  REQUIRE(ba2[0]);

  emp::BitArray<8> ba8;  // ba8 = 00000000
  ba8.flip(0,4);      // ba8 = 00001111
  REQUIRE(ba8[0]);
  REQUIRE(ba8[1]);
  REQUIRE(ba8[2]);
  REQUIRE(ba8[3]);
  REQUIRE(!ba8[4]);

  ba8[0].Toggle();    // ba8 = 00001110
  REQUIRE(!ba8[0]);

  emp::BitArray<4> ba4;  // ba4 = 0000
  ba4.flip();          // ba4 = 1111
  REQUIRE(ba4.all());
}

/**
 * FindOne and PopOne
 */
void test_find(){
  emp::BitArray<10> ba10;  // ba10 = 00 00000000
  ba10.flip(3);          // ba10 = 00 00001000
  REQUIRE(ba10.FindOne() == 3);
  ba10.PopOne();        // ba10 = 00 00000000
  REQUIRE(ba10.PopOne() == -1);
  ba10.flip(3);
  ba10.flip(1);
  REQUIRE(ba10.FindOne(2) == 3);
  REQUIRE(ba10.FindOne(4) == -1);
}

/**
 * GetByte and SetByte
 */
void test_byte(){
  emp::BitArray<10>  ba10;
  ba10.SetByte(0, 10);
  REQUIRE(ba10.GetByte(0) == 10);

  ba10.flip(0,4);
  REQUIRE(ba10.GetByte(0) == 5);
  ba10.SetByte(1, 3);
  REQUIRE(ba10.count() == 4);
}

/**
 * GetBytes
 */
// actual testing function
template <size_t Bits>
void do_byte_test() {
  emp::BitArray<Bits> ba;

  for (size_t i = 0; i < Bits / 8; ++i) {
    ba.SetByte(i, 10 * i);
  }

  const auto myspan = ba.GetBytes();
  for (size_t i = 0; i < Bits / 8; ++i) {
    REQUIRE(myspan[i] == static_cast<std::byte>(i * 10));
  }
}
// helper function that uses a fold expression to
// unpack the integer sequence of bits to test
// and then call the actual testing function with each as a template arg
template <typename T, T... Bits>
void do_byte_tests(const std::integer_sequence<T, Bits...>& sequence) {
  ((do_byte_test<Bits>()),...);
}
// function that holds what number of bits to test, and then calls
// the helper function with them
void test_bytes() {
  // sequence of number of bits to test
  std::index_sequence<16, 17, 32, 33, 64, 65, 128, 129> bits_to_test{};
  do_byte_tests(bits_to_test);
}

/**
 * Left and Right shifts
 */
void test_shift(){
  emp::BitArray<40> ba40;
  ba40.SetByte(0, 1);
  ba40 <<= 34;
  REQUIRE(ba40.GetByte(4) == 4);

  emp::BitArray<10> ba10;
  ba10.SetByte(0, 10);
  ba10 >>= 2;
  REQUIRE(ba10.GetByte(0) == 2);
}

/**
 * Count ones
 */
void test_count(){
  emp::BitArray<12> ba12;
  ba12.SetAll();
  REQUIRE(ba12.count() == 12);
  REQUIRE(ba12.CountOnes_Sparse() == 12);
  ba12.flip(0,5);
  REQUIRE(ba12.count() == 7);
}

/**
 * Get ones
 */
void test_get_ones(){
  emp::BitArray<5> ba5;
  ba5.flip(2); // 00100
  emp::vector<size_t> ones = ba5.GetOnes();
  REQUIRE(ones.size() == 1);
  REQUIRE(ones[0] == 2);
}


/**
 * Get and Set bits
 */
void test_bit(){
  emp::BitArray<8> ba8;
  ba8.Set(0, 1);       // ba8 = 00000001
  REQUIRE(ba8.Get(0));

  ba8.Set(7, 1);      // ba8 = 10000001
  ba8.Set(0, 0);      // ba8 = 10000000
  REQUIRE(!ba8.Get(0));
  REQUIRE(ba8.Get(7));
}

/**
 * Bitwise XOR ^
 */
void test_bitwise_xor(){
  emp::BitArray<4> ba4;
  ba4.Set(0, 1);
  emp::BitArray<4> ba4_1;
  ba4_1.SetByte(0,3);
  ba4 ^= ba4_1;                 // ba4 = 0001 ^ 0011 = 0010
  REQUIRE(ba4.GetByte(0) == 2);   // 0010 = 2
  ba4_1.PopOne();               // ba4_1 = 0010
  ba4 ^= ba4_1;                 // ba4 = 0010 ^ 0010 = 0000
  REQUIRE(ba4.GetByte(0) == 0);  // 0000 = 0
}

/**
 * Bitwise OR |
 */
void test_bitwise_or(){
  emp::BitArray<10> ba10;
  emp::BitArray<10> ba10_1;
  ba10.Set(1,1);                 // ba10 = 00 0000 0010
  ba10_1.Set(3,1);
  ba10_1.SetByte(1,3);
  REQUIRE(ba10_1.count() == 3);  // ba10_1 = 11 00001000
  ba10_1 |= ba10;                // ba10_1 = 11 00001000 | 00 00000010 = 11 00001010
  REQUIRE(ba10_1.GetByte(0) == 10);
  REQUIRE(ba10_1.GetByte(1) == 3);
}

/**
 * Bitwise AND &
 */
void test_bitwise_and(){
  emp::BitArray<8> ba8;
  emp::BitArray<8> ba8_1;
  ba8.SetByte(0,13);    // ba8 = 00001101
  ba8_1.SetByte(0,10);  // ba8_1 = 00001010
  ba8_1 &= ba8;          // ba8_1 = 00001010 & 00001101 = 00001000
  REQUIRE(ba8_1.GetByte(0) == 8);
}

/**
 * NAND, NOR, EQU & SELF
 */
void test_more_comparators(){
  // NAND
  emp::BitArray<8> ba8_1;
  emp::BitArray<8> ba8_2;
  ba8_1.SetAll();
  REQUIRE(ba8_1.NAND(ba8_2).All());
  ba8_2.flip(1);
  ba8_1.NAND_SELF(ba8_2);
  REQUIRE(ba8_1.Any());
  REQUIRE( !(ba8_1.Get(1)) );

  // NOR
  ba8_1.SetAll();
  ba8_2.Clear();
  REQUIRE(ba8_1.NOR(ba8_2).None());
  ba8_1.flip(1);
  ba8_1.NOR_SELF(ba8_2);
  REQUIRE(ba8_1.Get(1));

  // EQU
  ba8_1.Clear();
  ba8_2.SetAll();
  REQUIRE( (ba8_1.EQU(ba8_2).None()) );
  ba8_2.Clear();
  ba8_2.EQU_SELF(ba8_1);
  REQUIRE(ba8_2.All());
}

/**
 * Random bitset
 */
void test_random(){
  emp::Random random;
  emp::BitArray<8> ba8(random);
  ba8.Randomize(random, 1.0);
  REQUIRE(ba8.all());
  ba8.Randomize(random, 0.0);
  REQUIRE(ba8.none());
}

/**
 * Copy
 */
void test_copy(){
  emp::BitArray<10> ba10;
  ba10.SetAll();
  ba10.flip(0,5);

  emp::BitArray<10> ba10_1;
  ba10_1 = ba10;
  REQUIRE(ba10 == ba10_1);
}

/**
 * Comparators (>=,>,==,!=,<,<=)
 */
void test_comparators(){
  emp::BitArray<10> ba10;
  emp::BitArray<10> ba10_1;
  ba10_1.SetAll();
  REQUIRE(ba10_1 != ba10);
  REQUIRE(ba10_1 > ba10);
  ba10.SetAll();
  REQUIRE(ba10_1 >= ba10);
  REQUIRE(ba10_1 <= ba10);
  REQUIRE(ba10_1 == ba10);
  REQUIRE(!(ba10_1 < ba10));
  ba10.Clear();
  REQUIRE( (ba10 < ba10_1) );
}

/**
 * Export
 */
void test_export(){
  emp::BitArray<8> ba8;
  ba8.SetAll();
  REQUIRE(ba8.count() == 8);
  emp::BitArray<10> ba10 = ba8.Export<10>();
  REQUIRE(ba10.size() == 10);
  REQUIRE(ba10.GetByte(0) == 255);
  REQUIRE(ba10.GetByte(1) == 0);
}

/**
 * Import
 */
void test_import(){
  emp::BitArray<8> ba8;
  emp::BitArray<20> ba20;
  ba20[5] = 1;

  ba8.Import(ba20);
  REQUIRE(ba8[5]);

  emp::BitArray<10> ba10;
  ba10.SetAll();
  ba20.Import(ba10);
  REQUIRE(ba20.count() == 10);
}



TEST_CASE("Test BitArray", "[bits]")
{
  test_status();
  test_size();
  test_flip();
  test_bit();
  test_byte();
  test_bytes();
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
  test_export();
  test_import();
}


// For BitArray Import/Export
template <size_t SOURCE_BITS, size_t DEST_BITS>
struct ImportExportTester {

  static void test() {

    emp::Random rand(1);

    // using default parameter
    emp::BitArray<SOURCE_BITS> source(rand);
    emp::BitArray<DEST_BITS> dest(rand);

    dest.template Import(source);

    for (size_t i = 0; i < std::min(source.GetSize(), dest.GetSize()); ++i) {
      REQUIRE(source.Get(i) == dest.Get(i));
    }
    for (size_t i = source.GetSize(); i < dest.GetSize(); ++i) {
      REQUIRE(dest.Get(i) == 0);
    }

    dest.Clear();
    dest = source.template Export<dest.GetSize()>();

    for (size_t i = 0; i < std::min(source.GetSize(), dest.GetSize()); ++i) {
      REQUIRE(source.Get(i) == dest.Get(i));
    }
    for (size_t i = source.GetSize(); i < dest.GetSize(); ++i) {
      REQUIRE(dest.Get(i) == 0);
    }

    // using all from_bit's
    source.Randomize(rand);
    dest.Randomize(rand);

    for (size_t from_bit = 0; from_bit < source.GetSize(); ++from_bit) {
      // std::cout << "---------" << std::endl;
      // std::cout << source << std::endl;
      dest.template Import(source, from_bit);
      // std::cout << "=========" << std::endl;
      // std::cout << from_bit << std::endl;
      // std::cout << source << std::endl;
      // std::cout << dest << std::endl;
      for (size_t i = 0; i < std::min(source.GetSize() - from_bit, dest.GetSize()); ++i) {
        REQUIRE(source.Get(i+from_bit) == dest.Get(i));
      }
      for (size_t i = source.GetSize() - from_bit; i < dest.GetSize(); ++i) {
        REQUIRE(dest.Get(i) == 0);
      }

      dest.Clear();
      dest = source.template Export<dest.GetSize()>(from_bit);

      for (size_t i = 0; i < std::min(source.GetSize() - from_bit, dest.GetSize()); ++i) {
        REQUIRE(source.Get(i+from_bit) == dest.Get(i));
      }
      for (size_t i = source.GetSize() - from_bit; i < dest.GetSize(); ++i) {
        REQUIRE(dest.Get(i) == 0);
      }

    }
  }
};

// for BitArray ROTATE_SELF
// adapted from spraetor.github.io/2015/12/26/compile-time-loops.html
// TODO: replace with https://en.cppreference.com/w/cpp/utility/integer_sequence
template <size_t N>
struct MultiTester2 {

  template <size_t I>
  static void test() {

    emp::Random rand(1);

    constexpr int W = N - 2;
    emp::BitArray<W> ba;

    for (int j = 0; j < W; ++j) {
      ba.Clear(); ba.Set(j);
      ba.template ROTL_SELF<I>();
      REQUIRE(ba.CountOnes() == 1);
      REQUIRE(ba.Get(emp::Mod(j+I,W)));

      ba.SetAll(); ba.Set(j, false);
      ba.template ROTL_SELF<I>();
      REQUIRE(ba.CountOnes() == W-1);
      REQUIRE(!ba.Get(emp::Mod(j+I,W)));

      ba.Randomize(rand); ba.Set(j);
      const size_t c1 = ba.CountOnes();
      ba.template ROTL_SELF<I>();
      REQUIRE(ba.CountOnes() == c1);
      REQUIRE(ba.Get(emp::Mod(j+I,W)));

      ba.Randomize(rand); ba.Set(j, false);
      const size_t c2 = ba.CountOnes();
      ba.template ROTL_SELF<I>();
      REQUIRE(ba.CountOnes() == c2);
      REQUIRE(!ba.Get(emp::Mod(j+I,W)));

      ba.Clear(); ba.Set(j);
      ba.template ROTR_SELF<I>();
      REQUIRE(ba.CountOnes() == 1);
      REQUIRE(ba.Get(emp::Mod(j-I,W)));

      ba.SetAll(); ba.Set(j, false);
      ba.template ROTR_SELF<I>();
      REQUIRE(ba.CountOnes() == W-1);
      REQUIRE(!ba.Get(emp::Mod(j-I,W)));

      ba.Randomize(rand); ba.Set(j);
      const size_t c3 = ba.CountOnes();
      ba.template ROTR_SELF<I>();
      REQUIRE(ba.CountOnes() == c3);
      REQUIRE(ba.Get(emp::Mod(j-I,W)));

      ba.Randomize(rand); ba.Set(j, false);
      const size_t c4 = ba.CountOnes();
      ba.template ROTR_SELF<I>();
      REQUIRE(ba.CountOnes() == c4);
      REQUIRE(!ba.Get(emp::Mod(j-I,W)));
    }

    if constexpr (I+1 < N) {
      // recurse
      MultiTester2<N>::test<I+1>();
    }
  }

};

// TODO: replace with https://en.cppreference.com/w/cpp/utility/integer_sequence
template <int N>
struct MultiTester {

  template <int I>
  static void test() {

    constexpr int width = I;
    constexpr int STEP = (I <= 200) ? 1 : I/100;

    emp::Random rand(1);
    emp::BitArray<width> ba(rand);
    const emp::BitArray<width> ba_orig(ba);
    const size_t num_ones = ba.CountOnes();

    // Rotations should not change the number of ones.
    for (int i = -width - STEP - 1; i <= width + STEP + 1; i += STEP) {
      for (size_t rep = 0; rep < width; ++ rep) {
        ba.ROTATE_SELF(i);
        REQUIRE(ba.CountOnes() == num_ones);
      }
      REQUIRE(ba == ba_orig);
    }

    // Try each individual bit set with many possible rotations.
    for (int i = -width - STEP - 1; i <= width + STEP + 1; i += STEP) {
      // for large widths, just do one starting position
      for (int j = 0; j < (width < 200 ? width : 1); ++j) {
        ba.Clear(); ba.Set(j);
        ba.ROTATE_SELF(i);
        REQUIRE(ba.CountOnes() == 1);
        REQUIRE(ba.Get(emp::Mod(j-i,width)));

        ba.SetAll(); ba.Set(j, false);
        ba.ROTATE_SELF(i);
        REQUIRE(ba.CountOnes() == width-1);
        REQUIRE(!ba.Get(emp::Mod(j-i,width)));

        ba.Randomize(rand); ba.Set(j);
        const size_t c1 = ba.CountOnes();
        ba.ROTATE_SELF(i);
        REQUIRE(ba.CountOnes() == c1);
        REQUIRE(ba.Get(emp::Mod(j-i,width)));

        ba.Randomize(rand); ba.Set(j, false);
        const size_t c2 = ba.CountOnes();
        ba.ROTATE_SELF(i);
        REQUIRE(ba.CountOnes() == c2);
        REQUIRE(!ba.Get(emp::Mod(j-i,width)));
      }
    }

    if constexpr (N < 200) {
      // test templated rotates (only for small N)
      MultiTester2<I+2>::template test<0>();
    }

    if constexpr (I+1 < N && N < 200) {
      // recurse
      MultiTester<N>::test<I+1>();
    } else if constexpr (I+1 < N ) {
      // recurse
      MultiTester<N>::test<I+63>();
    }
  }

};

template class emp::BitArray<5>;
TEST_CASE("Another Test BitArray", "[bits]")
{

  // test BitArray GetSize, GetNumBytes
  {
    REQUIRE(emp::BitArray<2>{}.GetSize() == 2);
    REQUIRE(emp::BitArray<2>{}.GetNumBytes() == 1);

    REQUIRE(emp::BitArray<7>{}.GetSize() == 7);
    REQUIRE(emp::BitArray<7>{}.GetNumBytes() == 1);

    REQUIRE(emp::BitArray<8>{}.GetSize() == 8);
    REQUIRE(emp::BitArray<8>{}.GetNumBytes() == 1);

    REQUIRE(emp::BitArray<9>{}.GetSize() == 9);
    REQUIRE(emp::BitArray<9>{}.GetNumBytes() == 2);

    REQUIRE(emp::BitArray<16>{}.GetSize() == 16);
    REQUIRE(emp::BitArray<16>{}.GetNumBytes() == 2);

    REQUIRE(emp::BitArray<24>{}.GetSize() == 24);
    REQUIRE(emp::BitArray<24>{}.GetNumBytes() == 3);
  }

  // test BitArray reverse
  {

    REQUIRE(emp::BitArray<1>{0}.REVERSE_SELF() == emp::BitArray<1>{0});
    REQUIRE(emp::BitArray<1>{0}.REVERSE_SELF().CountOnes() == 0);
    REQUIRE(emp::BitArray<1>{1}.REVERSE_SELF() == emp::BitArray<1>{1});
    REQUIRE(emp::BitArray<1>{1}.REVERSE_SELF().CountOnes() == 1);

    REQUIRE(
      (emp::BitArray<2>{1,1}.REVERSE_SELF())
      ==
      (emp::BitArray<2>{1,1})
    );
    REQUIRE((emp::BitArray<2>{1,1}.REVERSE_SELF().CountOnes()) == 2);
    REQUIRE(
      (emp::BitArray<2>{0,1}.REVERSE_SELF())
      ==
      (emp::BitArray<2>{1,0})
    );
    REQUIRE((emp::BitArray<2>{0,1}.REVERSE_SELF().CountOnes()) == 1);
    REQUIRE(
      (emp::BitArray<2>{0,0}.REVERSE_SELF())
      ==
      (emp::BitArray<2>{0,0})
    );
    REQUIRE((emp::BitArray<2>{0,0}.REVERSE_SELF().CountOnes()) == 0);

    REQUIRE(
      (emp::BitArray<7>{1,1,0,0,0,0,1}.REVERSE_SELF())
      ==
      (emp::BitArray<7>{1,0,0,0,0,1,1})
    );
    REQUIRE((emp::BitArray<7>{1,1,0,0,0,0,1}.REVERSE_SELF().CountOnes()) == 3);
    REQUIRE(
      (emp::BitArray<7>{1,0,1,0,1,0,1}.REVERSE_SELF())
      ==
      (emp::BitArray<7>{1,0,1,0,1,0,1})
    );
    REQUIRE((emp::BitArray<7>{1,0,1,0,1,0,1}.REVERSE_SELF().CountOnes()) == 4);
    REQUIRE(
      (emp::BitArray<7>{1,1,1,1,1,0,1}.REVERSE_SELF())
      ==
      (emp::BitArray<7>{1,0,1,1,1,1,1})
    );
    REQUIRE((emp::BitArray<7>{1,1,1,1,1,0,1}.REVERSE_SELF().CountOnes()) == 6);

    REQUIRE(
      (emp::BitArray<8>{1,1,0,0,0,0,1,0}.REVERSE_SELF())
      ==
      (emp::BitArray<8>{0,1,0,0,0,0,1,1})
    );
    REQUIRE((emp::BitArray<8>{1,1,0,0,0,0,1,0}.REVERSE_SELF().CountOnes()) == 3);
    REQUIRE(
      (emp::BitArray<8>{1,0,1,0,1,0,1,0}.REVERSE_SELF())
      ==
      (emp::BitArray<8>{0,1,0,1,0,1,0,1})
    );
    REQUIRE((emp::BitArray<8>{0,1,0,1,0,1,0,1}.REVERSE_SELF().CountOnes()) == 4);
    REQUIRE(
      (emp::BitArray<8>{1,1,1,1,1,0,1,0}.REVERSE_SELF())
      ==
      (emp::BitArray<8>{0,1,0,1,1,1,1,1})
    );
    REQUIRE((emp::BitArray<8>{1,1,1,1,1,0,1,0}.REVERSE_SELF().CountOnes()) == 6);

    REQUIRE(
      (emp::BitArray<9>{1,1,0,0,0,0,1,0,0}.REVERSE_SELF())
      ==
      (emp::BitArray<9>{0,0,1,0,0,0,0,1,1})
    );
    REQUIRE(
      (emp::BitArray<9>{1,1,0,0,0,0,1,0,0}.REVERSE_SELF().CountOnes())
      ==
      3
    );
    REQUIRE(
      (emp::BitArray<9>{1,0,1,0,1,0,1,0,0}.REVERSE_SELF())
      ==
      (emp::BitArray<9>{0,0,1,0,1,0,1,0,1})
    );
    REQUIRE(
      (emp::BitArray<9>{0,0,1,0,1,0,1,0,1}.REVERSE_SELF().CountOnes())
      ==
      4
    );
    REQUIRE(
      (emp::BitArray<9>{1,1,1,1,1,0,1,0,0}.REVERSE_SELF())
      ==
      (emp::BitArray<9>{0,0,1,0,1,1,1,1,1})
    );
    REQUIRE(
      (emp::BitArray<9>{1,1,1,1,1,0,1,0,0}.REVERSE_SELF().CountOnes())
      ==
      6
    );

    emp::Random rand(1);
    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitArray<15> ba(rand);
      ba[0] = 0;
      ba[15-1] = 1;
      REQUIRE(ba.REVERSE() != ba);
      REQUIRE(ba.REVERSE().REVERSE() == ba);
      REQUIRE(ba.REVERSE().CountOnes() == ba.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitArray<16> ba(rand);
      ba[0] = 0;
      ba[16-1] = 1;
      REQUIRE(ba.REVERSE() != ba);
      REQUIRE(ba.REVERSE().REVERSE() == ba);
      REQUIRE(ba.REVERSE().CountOnes() == ba.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitArray<17> ba(rand);
      ba[0] = 0;
      ba[17-1] = 1;
      REQUIRE(ba.REVERSE() != ba);
      REQUIRE(ba.REVERSE().REVERSE() == ba);
      REQUIRE(ba.REVERSE().CountOnes() == ba.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitArray<31> ba(rand);
      ba[0] = 0;
      ba[31-1] = 1;
      REQUIRE(ba.REVERSE() != ba);
      REQUIRE(ba.REVERSE().REVERSE() == ba);
      REQUIRE(ba.REVERSE().CountOnes() == ba.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitArray<32> ba(rand);
      ba[0] = 0;
      ba[32-1] = 1;
      REQUIRE(ba.REVERSE() != ba);
      REQUIRE(ba.REVERSE().REVERSE() == ba);
      REQUIRE(ba.REVERSE().CountOnes() == ba.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitArray<33> ba(rand);
      ba[0] = 0;
      ba[33-1] = 1;
      REQUIRE(ba.REVERSE() != ba);
      REQUIRE(ba.REVERSE().REVERSE() == ba);
      REQUIRE(ba.REVERSE().CountOnes() == ba.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitArray<63> ba(rand);
      ba[0] = 0;
      ba[63-1] = 1;
      REQUIRE(ba.REVERSE() != ba);
      REQUIRE(ba.REVERSE().REVERSE() == ba);
      REQUIRE(ba.REVERSE().CountOnes() == ba.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitArray<64> ba(rand);
      ba[0] = 0;
      ba[64-1] = 1;
      REQUIRE(ba.REVERSE() != ba);
      REQUIRE(ba.REVERSE().REVERSE() == ba);
      REQUIRE(ba.REVERSE().CountOnes() == ba.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitArray<65> ba(rand);
      ba[0] = 0;
      ba[65-1] = 1;
      REQUIRE(ba.REVERSE() != ba);
      REQUIRE(ba.REVERSE().REVERSE() == ba);
      REQUIRE(ba.REVERSE().CountOnes() == ba.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitArray<127> ba(rand);
      ba[0] = 0;
      ba[127-1] = 1;
      REQUIRE(ba.REVERSE() != ba);
      REQUIRE(ba.REVERSE().REVERSE() == ba);
      REQUIRE(ba.REVERSE().CountOnes() == ba.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitArray<128> ba(rand);
      ba[0] = 0;
      ba[128-1] = 1;
      REQUIRE(ba.REVERSE() != ba);
      REQUIRE(ba.REVERSE().REVERSE() == ba);
      REQUIRE(ba.REVERSE().CountOnes() == ba.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitArray<129> ba(rand);
      ba[0] = 0;
      ba[129-1] = 1;
      REQUIRE(ba.REVERSE() != ba);
      REQUIRE(ba.REVERSE().REVERSE() == ba);
      REQUIRE(ba.REVERSE().CountOnes() == ba.CountOnes());
    }

  }

  // test BitArray addition
  {
  emp::BitArray<32> ba0;
  ba0.SetUInt(0, std::numeric_limits<uint32_t>::max() - 1);
  emp::BitArray<32> ba1;
  ba1.SetUInt(0,1);
  ba0+=ba1;
  REQUIRE (ba0.GetUInt(0) == 4294967295);
  REQUIRE ((ba0+ba1).GetUInt(0) == 0);
  REQUIRE ((ba0+ba0).GetUInt(0) == 4294967294);

  emp::BitArray<8> ba2;
  ba2.SetUInt(0, emp::IntPow(2UL, 8UL)-1);
  emp::BitArray<8> ba3;
  ba3.SetUInt(0, 1);
  REQUIRE((ba2+ba3).GetUInt(0) == 0);
  emp::BitArray<64> ba4;
  ba4.SetUInt(0, std::numeric_limits<uint32_t>::max()-1);
  ba4.SetUInt(1, std::numeric_limits<uint32_t>::max());
  emp::BitArray<64> ba5;
  ba5.SetUInt(0, 1);
  ba4+=ba5;
  REQUIRE(ba4.GetUInt(0) == pow((size_t)2, (size_t)32)-1);
  REQUIRE(ba4.GetUInt(1) == pow((size_t)2, (size_t)32)-1);
  ba4+=ba5;
  REQUIRE(ba4.GetUInt(0) == 0);
  REQUIRE(ba4.GetUInt(1) == 0);
  }

  // test BitArray subtraction
  {
  emp::BitArray<32> ba0;
  ba0.SetUInt(0, 1);
  emp::BitArray<32> ba1;
  ba1.SetUInt(0, 1);
  ba0 = ba0 - ba1;
  REQUIRE (ba0.GetUInt(0) == 0);
  REQUIRE ((ba0-ba1).GetUInt(0) == std::numeric_limits<uint32_t>::max());

  emp::BitArray<8> ba2;
  ba2.SetUInt(0, 1);
  emp::BitArray<8> ba3;
  ba3.SetUInt(0, 1);

  ba2-=ba3;
  REQUIRE (ba2.GetUInt(0) == 0);
  REQUIRE((ba2-ba3).GetUInt(0) == emp::IntPow(2UL,8UL)-1);

  emp::BitArray<64> ba4;
  ba4.SetUInt(0, 1);
  ba4.SetUInt(1, 0);

  emp::BitArray<64> ba5;
  ba5.SetUInt(0, 1);

  ba4-=ba5;
  REQUIRE(ba4.GetUInt(0) == 0);
  REQUIRE(ba4.GetUInt(1) == 0);

  ba4-=ba5;
  REQUIRE(ba4.GetUInt(0) == std::numeric_limits<uint32_t>::max());
  REQUIRE(ba4.GetUInt(1) == std::numeric_limits<uint32_t>::max());
  ba4 = ba4 - ba5;
  REQUIRE(ba4.GetUInt(0) == std::numeric_limits<uint32_t>::max() - 1);
  REQUIRE(ba4.GetUInt(1) == std::numeric_limits<uint32_t>::max());
  }

  // test addition and subtraction with multiple fields
  {
  emp::BitArray<65> ba1;
  emp::BitArray<65> ba2;

  /* PART 1 */
  ba1.Clear();
  ba2.Clear();

  ba1.Set(64); // 10000...
  ba2.Set(0);  // ...00001

  for (size_t i = 0; i < 64; ++i) REQUIRE((ba1 - ba2).Get(i));
  REQUIRE(!(ba1 - ba2).Get(64));

  ba1 -= ba2;

  for (size_t i = 0; i < 64; ++i) {
    REQUIRE(ba1.Get(i));
  }
  REQUIRE(!ba1.Get(64));

  /* PART 2 */
  ba1.Clear();
  ba2.Clear();

  ba2.Set(0);  // ...00001

  for (size_t i = 0; i < 65; ++i) REQUIRE((ba1 - ba2).Get(i));

  ba1 -= ba2;

  for (size_t i = 0; i < 65; ++i) REQUIRE(ba1.Get(i));

  /* PART 3 */
  ba1.Clear();
  ba2.Clear();

  for (size_t i = 0; i < 65; ++i) ba1.Set(i); // 11111...11111
  ba2.Set(0);  // ...00001

  for (size_t i = 0; i < 65; ++i) REQUIRE(!(ba1 + ba2).Get(i));
  for (size_t i = 0; i < 65; ++i) REQUIRE(!(ba2 + ba1).Get(i));

  ba1 += ba2;

  for (size_t i = 0; i < 65; ++i) REQUIRE(!ba1.Get(i));

  /* PART 4 */
  ba1.Clear();
  ba2.Clear();

  for (size_t i = 0; i < 64; ++i) ba1.Set(i); // 01111...11111
  ba2.Set(0);  // ...00001

  for (size_t i = 0; i < 64; ++i) REQUIRE(!(ba1 + ba2).Get(i));
  REQUIRE((ba1 + ba2).Get(64));
  for (size_t i = 0; i < 64; ++i) REQUIRE(!(ba2 + ba1).Get(i));
  REQUIRE((ba2 + ba1).Get(64));

  ba1 += ba2;

  for (size_t i = 0; i < 64; ++i) REQUIRE(!ba1.Get(i));
  REQUIRE((ba2 + ba1).Get(64));
  }

  {
  emp::BitArray<3> ba0{0,0,0};
  REQUIRE(ba0.GetUInt8(0) == 0);
  REQUIRE(ba0.GetUInt16(0) == 0);
  REQUIRE(ba0.GetUInt32(0) == 0);
  REQUIRE(ba0.GetUInt64(0) == 0);
  REQUIRE(ba0.GetNumStates() == 8);

  emp::BitArray<3> ba1{1,0,0};
  REQUIRE(ba1.GetUInt8(0) == 1);
  REQUIRE(ba1.GetUInt16(0) == 1);
  REQUIRE(ba1.GetUInt32(0) == 1);
  REQUIRE(ba1.GetUInt64(0) == 1);

  emp::BitArray<3> ba2{1,1,0};
  REQUIRE(ba2.GetUInt8(0) == 3);
  REQUIRE(ba2.GetUInt16(0) == 3);
  REQUIRE(ba2.GetUInt32(0) == 3);
  REQUIRE(ba2.GetUInt64(0) == 3);

  emp::BitArray<3> ba3{1,1,1};
  REQUIRE(ba3.GetUInt8(0) == 7);

  emp::BitArray<3> ba4{0,1,1};
  REQUIRE(ba4.GetUInt8(0) == 6);

  emp::BitArray<32> ba5;
  ba5.SetUInt(0, 1789156UL);
  REQUIRE(ba5.GetUInt64(0) == 1789156ULL);
  REQUIRE(ba5.GetNumStates() == 4294967296ULL);

  emp::BitArray<63> ba6;
  ba6.SetUInt64(0, 789156816848ULL);
  REQUIRE(ba6.GetUInt64(0) == 789156816848ULL);
  REQUIRE(ba6.GetNumStates() == 9223372036854775808ULL);


  // @CAO: Removed GetDouble() due to confusing name (GetUInt64() gives the same answer, but with
  //       the correct encoding.
  // emp::BitArray<65> ba7;
  // ba7.SetUInt64(0, 1789156816848ULL);
  // ba7.Set(64);
  // REQUIRE(ba7.GetDouble() == 1789156816848.0 + emp::Pow2(64.0));
  // REQUIRE(ba7.MaxDouble() == 36893488147419103231.0);

  // emp::BitArray<1027> ba8;
  // ba8.Set(1026);
  // REQUIRE(std::isinf(ba8.GetDouble()));
  // REQUIRE(std::isinf(ba8.MaxDouble()));
  }

  // test list initializer
  {
  emp::BitArray<3> ba_empty{0,0,0};
  emp::BitArray<3> ba_first{1,0,0};
  emp::BitArray<3> ba_last{0,0,1};
  emp::BitArray<3> ba_full{1,1,1};

  REQUIRE(ba_empty.CountOnes() == 0);
  REQUIRE(ba_first.CountOnes() == 1);
  REQUIRE(ba_last.CountOnes() == 1);
  REQUIRE(ba_full.CountOnes() == 3);
  }

  // test Import and Export
  {

    emp::Random rand(1);

    emp::BitArray<32> orig(rand);

    emp::array<emp::BitArray<32>, 1> d1;
    emp::array<emp::BitArray<16>, 2> d2;
    emp::array<emp::BitArray<8>, 4> d4;
    emp::array<emp::BitArray<4>, 8> d8;
    emp::array<emp::BitArray<2>, 16> d16;
    emp::array<emp::BitArray<1>, 32> d32;

    // Import

    d1[0].Import(orig, 0);
    for (size_t i = 0; i < 2; ++i) d2[i].Import(orig, i * 16);
    for (size_t i = 0; i < 4; ++i) d4[i].Import(orig, i * 8);
    for (size_t i = 0; i < 8; ++i) d8[i].Import(orig, i * 4);
    for (size_t i = 0; i < 16; ++i) d16[i].Import(orig, i * 2);
    for (size_t i = 0; i < 32; ++i) d32[i].Import(orig, i * 1);

    for (size_t i = 0; i < 32; ++i) {
      REQUIRE(orig[i] == d1[i/32][i%32]);
      REQUIRE(orig[i] == d2[i/16][i%16]);
      REQUIRE(orig[i] == d4[i/8][i%8]);
      REQUIRE(orig[i] == d8[i/4][i%4]);
      REQUIRE(orig[i] == d16[i/2][i%2]);
      REQUIRE(orig[i] == d32[i/1][i%1]);
    }

    // Export

    d1[0] = orig.Export<32>(0);
    for (size_t i = 0; i < 2; ++i) d2[i] = orig.Export<16>(i * 16);
    for (size_t i = 0; i < 4; ++i) d4[i] = orig.Export<8>(i * 8);
    for (size_t i = 0; i < 8; ++i) d8[i] = orig.Export<4>(i * 4);
    for (size_t i = 0; i < 16; ++i) d16[i] = orig.Export<2>(i * 2);
    for (size_t i = 0; i < 32; ++i) d32[i] = orig.Export<1>(i * 1);

    for (size_t i = 0; i < 32; ++i) {
      REQUIRE(orig[i] == d1[i/32][i%32]);
      REQUIRE(orig[i] == d2[i/16][i%16]);
      REQUIRE(orig[i] == d4[i/8][i%8]);
      REQUIRE(orig[i] == d8[i/4][i%4]);
      REQUIRE(orig[i] == d16[i/2][i%2]);
      REQUIRE(orig[i] == d32[i/1][i%1]);
    }

    // now test some funky imports and exports
    // interesting container sizes:
    // 1, 17, 29, 32, 33, 64, 65, 95, 128, 129

    ImportExportTester<1,1>::test();
    ImportExportTester<1,17>::test();
    ImportExportTester<1,29>::test();
    ImportExportTester<1,32>::test();
    ImportExportTester<1,33>::test();
    ImportExportTester<1,64>::test();
    ImportExportTester<1,65>::test();
    ImportExportTester<1,96>::test();
    ImportExportTester<1,128>::test();
    ImportExportTester<1,129>::test();

    ImportExportTester<17,1>::test();
    ImportExportTester<17,17>::test();
    ImportExportTester<17,29>::test();
    ImportExportTester<17,32>::test();
    ImportExportTester<17,33>::test();
    ImportExportTester<17,64>::test();
    ImportExportTester<17,65>::test();
    ImportExportTester<17,96>::test();
    ImportExportTester<17,128>::test();
    ImportExportTester<17,129>::test();

    ImportExportTester<29,1>::test();
    ImportExportTester<29,17>::test();
    ImportExportTester<29,29>::test();
    ImportExportTester<29,32>::test();
    ImportExportTester<29,33>::test();
    ImportExportTester<29,64>::test();
    ImportExportTester<29,65>::test();
    ImportExportTester<29,96>::test();
    ImportExportTester<29,128>::test();
    ImportExportTester<29,129>::test();

    ImportExportTester<32,1>::test();
    ImportExportTester<32,17>::test();
    ImportExportTester<32,29>::test();
    ImportExportTester<32,32>::test();
    ImportExportTester<32,33>::test();
    ImportExportTester<32,64>::test();
    ImportExportTester<32,65>::test();
    ImportExportTester<32,96>::test();
    ImportExportTester<32,128>::test();
    ImportExportTester<32,129>::test();

    ImportExportTester<33,1>::test();
    ImportExportTester<33,17>::test();
    ImportExportTester<33,29>::test();
    ImportExportTester<33,32>::test();
    ImportExportTester<33,33>::test();
    ImportExportTester<33,64>::test();
    ImportExportTester<33,65>::test();
    ImportExportTester<33,96>::test();
    ImportExportTester<33,128>::test();
    ImportExportTester<33,129>::test();

    ImportExportTester<64,1>::test();
    ImportExportTester<64,17>::test();
    ImportExportTester<64,29>::test();
    ImportExportTester<64,32>::test();
    ImportExportTester<64,33>::test();
    ImportExportTester<64,64>::test();
    ImportExportTester<64,65>::test();
    ImportExportTester<64,96>::test();
    ImportExportTester<64,128>::test();
    ImportExportTester<64,129>::test();

    ImportExportTester<65,1>::test();
    ImportExportTester<65,17>::test();
    ImportExportTester<65,29>::test();
    ImportExportTester<65,32>::test();
    ImportExportTester<65,33>::test();
    ImportExportTester<65,64>::test();
    ImportExportTester<65,65>::test();
    ImportExportTester<65,96>::test();
    ImportExportTester<65,128>::test();
    ImportExportTester<65,129>::test();

    ImportExportTester<96,1>::test();
    ImportExportTester<96,17>::test();
    ImportExportTester<96,29>::test();
    ImportExportTester<96,32>::test();
    ImportExportTester<96,33>::test();
    ImportExportTester<96,64>::test();
    ImportExportTester<96,65>::test();
    ImportExportTester<96,96>::test();
    ImportExportTester<96,128>::test();
    ImportExportTester<96,129>::test();

    ImportExportTester<128,1>::test();
    ImportExportTester<128,17>::test();
    ImportExportTester<128,29>::test();
    ImportExportTester<128,32>::test();
    ImportExportTester<128,33>::test();
    ImportExportTester<128,64>::test();
    ImportExportTester<128,65>::test();
    ImportExportTester<128,96>::test();
    ImportExportTester<128,128>::test();
    ImportExportTester<128,129>::test();

    ImportExportTester<129,1>::test();
    ImportExportTester<129,17>::test();
    ImportExportTester<129,29>::test();
    ImportExportTester<129,32>::test();
    ImportExportTester<129,33>::test();
    ImportExportTester<129,64>::test();
    ImportExportTester<129,65>::test();
    ImportExportTester<129,96>::test();
    ImportExportTester<129,128>::test();
    ImportExportTester<129,129>::test();

  }

  emp::BitArray<10> ba10;
  emp::BitArray<25> ba25;
  emp::BitArray<32> ba32;
  emp::BitArray<50> ba50;
  emp::BitArray<64> ba64;
  emp::BitArray<80> ba80;

  ba80[70] = 1;
  ba80 <<= 1;
  emp::BitArray<80> ba80c(ba80);

  for (size_t i = 0; i < 75; i++) {
    emp::BitArray<80> shift_set = ba80 >> i;
    REQUIRE((shift_set.CountOnes() == 1) == (i <= 71));
  }

  ba80.Clear();

  REQUIRE(ba10[2] == false);
  ba10.flip(2);
  REQUIRE(ba10[2] == true);

  REQUIRE(ba32[2] == false);
  ba32.flip(2);
  REQUIRE(ba32[2] == true);

  REQUIRE(ba80[2] == false);
  ba80.flip(2);
  REQUIRE(ba80[2] == true);

  for (size_t i = 3; i < 8; i++) {REQUIRE(ba10[i] == false);}
  ba10.flip(3, 8);
  for (size_t i = 3; i < 8; i++) {REQUIRE(ba10[i] == true);}
  REQUIRE(ba10[8] == false);

  for (size_t i = 3; i < 8; i++) {REQUIRE(ba32[i] == false);}
  ba32.flip(3, 8);
  for (size_t i = 3; i < 8; i++) {REQUIRE(ba32[i] == true);}
  REQUIRE(ba32[8] == false);

  for (size_t i = 3; i < 8; i++) {REQUIRE(ba80[i] == false);}
  ba80.flip(3, 8);
  for (size_t i = 3; i < 8; i++) {REQUIRE(ba80[i] == true);}
  REQUIRE(ba80[8] == false);

  ba80[70] = 1;

  REQUIRE(ba10.GetUInt(0) == 252);
  REQUIRE(ba10.GetUInt32(0) == 252);
  REQUIRE(ba10.GetUInt64(0) == 252);

  REQUIRE(ba32.GetUInt(0) == 252);
  REQUIRE(ba32.GetUInt32(0) == 252);
  REQUIRE(ba32.GetUInt64(0) == 252);

  REQUIRE(ba80.GetUInt(0) == 252);
  REQUIRE(ba80.GetUInt(1) == 0);
  REQUIRE(ba80.GetUInt(2) == 64);
  REQUIRE(ba80.GetUInt32(0) == 252);
  REQUIRE(ba80.GetUInt32(1) == 0);
  REQUIRE(ba80.GetUInt32(2) == 64);
  REQUIRE(ba80.GetUInt64(0) == 252);
  REQUIRE(ba80.GetUInt64(1) == 64);

  ba80 = ba80c;

  // Test arbitrary bit retrieval of UInts
  ba80[65] = 1;
  REQUIRE(ba80.GetUInt32(2) == 130);
  REQUIRE(ba80.GetUInt32AtBit(64) == 130);
  REQUIRE(ba80.GetUInt8AtBit(64) == 130);

  emp::BitArray<96> ba;

  REQUIRE (ba.LongestSegmentOnes() == 0);
  ba.SetUInt(2, 1);
  REQUIRE (ba.LongestSegmentOnes() == 1);
  ba.SetUInt(1, 3);
  REQUIRE (ba.LongestSegmentOnes() == 2);
  ba.SetUInt(0, 7);
  REQUIRE (ba.LongestSegmentOnes() == 3);

  ba.SetUInt(0, std::numeric_limits<uint32_t>::max());
  ba.SetUInt(1, std::numeric_limits<uint32_t>::max() - 1);
  ba.SetUInt(2, std::numeric_limits<uint32_t>::max() - 3);
  REQUIRE (ba.LongestSegmentOnes() == 32);

  // tests for ROTATE
  // ... with one set bit
  ba10.Clear(); ba10.Set(0);
  ba25.Clear(); ba25.Set(0);
  ba32.Clear(); ba32.Set(0);
  ba50.Clear(); ba50.Set(0);
  ba64.Clear(); ba64.Set(0);
  ba80.Clear(); ba80.Set(0);

  for (int rot = -100; rot < 101; ++rot) {

    REQUIRE( ba10.CountOnes() == ba10.ROTATE(rot).CountOnes() );
    REQUIRE( ba25.CountOnes() == ba25.ROTATE(rot).CountOnes() );
    REQUIRE( ba32.CountOnes() == ba32.ROTATE(rot).CountOnes() );
    REQUIRE( ba50.CountOnes() == ba50.ROTATE(rot).CountOnes() );
    REQUIRE( ba64.CountOnes() == ba64.ROTATE(rot).CountOnes() );
    REQUIRE( ba80.CountOnes() == ba80.ROTATE(rot).CountOnes() );

    if (rot % 10) REQUIRE( ba10 != ba10.ROTATE(rot) );
    else REQUIRE( ba10 == ba10.ROTATE(rot) );

    if (rot % 25) REQUIRE( ba25 != ba25.ROTATE(rot) );
    else REQUIRE( ba25 == ba25.ROTATE(rot) );

    if (rot % 32) REQUIRE( ba32 != ba32.ROTATE(rot) );
    else REQUIRE( ba32 == ba32.ROTATE(rot) );

    if (rot % 50) REQUIRE( ba50 != ba50.ROTATE(rot) );
    else REQUIRE( ba50 == ba50.ROTATE(rot) );

    if (rot % 64) REQUIRE( ba64 != ba64.ROTATE(rot) );
    else REQUIRE( ba64 == ba64.ROTATE(rot) );

    if (rot % 80) REQUIRE( ba80 != ba80.ROTATE(rot) );
    else REQUIRE( ba80 == ba80.ROTATE(rot) );

  }

  // ... with random set bits
  emp::Random rand(1);
  // no ba10 because there's a reasonable chance
  // of breaking the test's assumption of nonsymmetry
  ba25.Randomize(rand);
  ba32.Randomize(rand);
  ba50.Randomize(rand);
  ba64.Randomize(rand);
  ba80.Randomize(rand);

  for (int rot = -100; rot < 101; ++rot) {

    REQUIRE( ba25.CountOnes() == ba25.ROTATE(rot).CountOnes() );
    REQUIRE( ba32.CountOnes() == ba32.ROTATE(rot).CountOnes() );
    REQUIRE( ba50.CountOnes() == ba50.ROTATE(rot).CountOnes() );
    REQUIRE( ba64.CountOnes() == ba64.ROTATE(rot).CountOnes() );
    REQUIRE( ba80.CountOnes() == ba80.ROTATE(rot).CountOnes() );

    if (rot % 25) REQUIRE( ba25 != ba25.ROTATE(rot) );
    else REQUIRE( ba25 == ba25.ROTATE(rot) );

    if (rot % 32) REQUIRE( ba32 != ba32.ROTATE(rot) );
    else REQUIRE( ba32 == ba32.ROTATE(rot) );

    if (rot % 50) REQUIRE( ba50 != ba50.ROTATE(rot) );
    else REQUIRE( ba50 == ba50.ROTATE(rot) );

    if (rot % 64) REQUIRE( ba64 != ba64.ROTATE(rot) );
    else REQUIRE( ba64 == ba64.ROTATE(rot) );

    if (rot % 80) REQUIRE( ba80 != ba80.ROTATE(rot) );
    else REQUIRE( ba80 == ba80.ROTATE(rot) );

  }

  // tests for ROTATE_SELF, ROTR_SELF, ROTL_SELF
  MultiTester<2>::test<1>();
  MultiTester<18>::test<17>();
  MultiTester<34>::test<31>();
  MultiTester<51>::test<50>();
  MultiTester<66>::test<63>();
  MultiTester<96>::test<93>();
  MultiTester<161>::test<160>();
  MultiTester<2050>::test<2048>();

  // tests for RandomizeFixed
  {
    emp::Random random(1);
    emp::BitArray<25> ba_25;
    emp::BitArray<32> ba_32;
    emp::BitArray<50> ba_50;
    emp::BitArray<64> ba_64;
    emp::BitArray<80> ba_80;

    ba_25.FlipRandomCount(random, 0);
    REQUIRE(!ba_25.CountOnes());

    ba_32.FlipRandomCount(random, 0);
    REQUIRE(!ba_32.CountOnes());

    ba_50.FlipRandomCount(random, 0);
    REQUIRE(!ba_50.CountOnes());

    ba_64.FlipRandomCount(random, 0);
    REQUIRE(!ba_64.CountOnes());

    ba_80.FlipRandomCount(random, 0);
    REQUIRE(!ba_80.CountOnes());


    ba_25.FlipRandomCount(random, 1);
    REQUIRE( ba_25.CountOnes() == 1);

    ba_32.FlipRandomCount(random, 1);
    REQUIRE( ba_32.CountOnes() == 1);

    ba_50.FlipRandomCount(random, 1);
    REQUIRE( ba_50.CountOnes() == 1);

    ba_64.FlipRandomCount(random, 1);
    REQUIRE( ba_64.CountOnes() == 1);

    ba_80.FlipRandomCount(random, 1);
    REQUIRE( ba_80.CountOnes() == 1);

    ba_25.Clear();
    ba_32.Clear();
    ba_50.Clear();
    ba_64.Clear();
    ba_80.Clear();

    for (size_t i = 1; i < 5000; ++i) {
      ba_25.FlipRandomCount(random, 1);
      REQUIRE(ba_25.CountOnes() <= i);

      ba_32.FlipRandomCount(random, 1);
      REQUIRE(ba_32.CountOnes() <= i);

      ba_50.FlipRandomCount(random, 1);
      REQUIRE(ba_50.CountOnes() <= i);

      ba_64.FlipRandomCount(random, 1);
      REQUIRE(ba_64.CountOnes() <= i);

      ba_80.FlipRandomCount(random, 1);
      REQUIRE(ba_80.CountOnes() <= i);
    }

    REQUIRE(ba_25.CountOnes() > ba_25.size()/4);
    REQUIRE(ba_25.CountOnes() < 3*ba_25.size()/4);
    REQUIRE(ba_32.CountOnes() > ba_32.size()/4);
    REQUIRE(ba_32.CountOnes() < 3*ba_32.size()/4);
    REQUIRE(ba_50.CountOnes() > ba_50.size()/4);
    REQUIRE(ba_50.CountOnes() < 3*ba_50.size()/4);
    REQUIRE(ba_64.CountOnes() > ba_64.size()/4);
    REQUIRE(ba_64.CountOnes() < 3*ba_64.size()/4);
    REQUIRE(ba_80.CountOnes() > ba_80.size()/4);
    REQUIRE(ba_80.CountOnes() < 3*ba_80.size()/4);

    for (size_t i = 0; i < 10; ++i) {
      ba_25.FlipRandomCount(random, ba_25.size());
      REQUIRE(ba_25.CountOnes() > ba_25.size()/4);
      REQUIRE(ba_25.CountOnes() < 3*ba_25.size()/4);

      ba_32.FlipRandomCount(random, ba_32.size());
      REQUIRE(ba_32.CountOnes() > ba_32.size()/4);
      REQUIRE(ba_32.CountOnes() < 3*ba_32.size()/4);

      ba_50.FlipRandomCount(random, ba_50.size());
      REQUIRE(ba_50.CountOnes() > ba_50.size()/4);
      REQUIRE(ba_50.CountOnes() < 3*ba_50.size()/4);

      ba_64.FlipRandomCount(random, ba_64.size());
      REQUIRE(ba_64.CountOnes() > ba_64.size()/4);
      REQUIRE(ba_64.CountOnes() < 3*ba_64.size()/4);

      ba_80.FlipRandomCount(random, ba_80.size());
      REQUIRE(ba_80.CountOnes() > ba_80.size()/4);
      REQUIRE(ba_80.CountOnes() < 3*ba_80.size()/4);
    }
  }

  // serialize / deserialize
  {

    // set up
    emp::Random rand(1);
    emp::BitArray<10> ba10(rand);
    emp::BitArray<25> ba25(rand);
    emp::BitArray<32> ba32(rand);
    emp::BitArray<50> ba50(rand);
    emp::BitArray<64> ba64(rand);
    emp::BitArray<80> ba80(rand);

    emp::BitArray<10> ba10_deser;
    emp::BitArray<25> ba25_deser;
    emp::BitArray<32> ba32_deser;
    emp::BitArray<50> ba50_deser;
    emp::BitArray<64> ba64_deser;
    emp::BitArray<80> ba80_deser;

    std::stringstream ss;

    {
      // Create an output archive
      cereal::BinaryOutputArchive oarchive(ss);

      // Write the data to the archive
      oarchive(
        ba10,
        ba25,
        ba32,
        ba50,
        ba64,
        ba80
      );

    } // archive goes out of scope, ensuring all contents are flushed

    {
      cereal::BinaryInputArchive iarchive(ss); // Create an input archive

       // Read the data from the archive
       iarchive(
        ba10_deser,
        ba25_deser,
        ba32_deser,
        ba50_deser,
        ba64_deser,
        ba80_deser
      );

    }

    REQUIRE(ba10 == ba10_deser);
    REQUIRE(ba25 == ba25_deser);
    REQUIRE(ba32 == ba32_deser);
    REQUIRE(ba50 == ba50_deser);
    REQUIRE(ba64 == ba64_deser);
    REQUIRE(ba80 == ba80_deser);

  }

  {

    // set up
    emp::Random rand(1);
    emp::BitArray<10> ba10(rand);
    emp::BitArray<25> ba25(rand);
    emp::BitArray<32> ba32(rand);
    emp::BitArray<50> ba50(rand);
    emp::BitArray<64> ba64(rand);
    emp::BitArray<80> ba80(rand);

    emp::BitArray<10> ba10_deser;
    emp::BitArray<25> ba25_deser;
    emp::BitArray<32> ba32_deser;
    emp::BitArray<50> ba50_deser;
    emp::BitArray<64> ba64_deser;
    emp::BitArray<80> ba80_deser;

    std::stringstream ss;

    {
      // Create an output archive
      cereal::JSONOutputArchive oarchive(ss);

      // Write the data to the archive
      oarchive(
        ba10,
        ba25,
        ba32,
        ba50,
        ba64,
        ba80
      );

    } // archive goes out of scope, ensuring all contents are flushed

    {
      cereal::JSONInputArchive iarchive(ss); // Create an input archive

       // Read the data from the archive
       iarchive(
        ba10_deser,
        ba25_deser,
        ba32_deser,
        ba50_deser,
        ba64_deser,
        ba80_deser
      );

    }

    REQUIRE(ba10 == ba10_deser);
    REQUIRE(ba25 == ba25_deser);
    REQUIRE(ba32 == ba32_deser);
    REQUIRE(ba50 == ba50_deser);
    REQUIRE(ba64 == ba64_deser);
    REQUIRE(ba80 == ba80_deser);

  }

}


TEST_CASE("Test BitArray string construction", "[tools]") {

  REQUIRE( emp::BitArray<5>( "01001" ) == emp::BitArray<5>{0, 1, 0, 0, 1} );

  // std::bitset treats bits in the opposite direction of emp::BitArray.
  REQUIRE(
    emp::BitArray<5>( std::bitset<5>( "01001" ) )
    == emp::BitArray<5>{1, 0, 0, 1, 0}
  );

}
