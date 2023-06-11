/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file BitSet.cpp
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
#include "emp/bits/BitSet.hpp"
#include "emp/compiler/RegEx.hpp"
#include "emp/data/DataNode.hpp"
#include "emp/functional/FunctionSet.hpp"
#include "emp/math/Random.hpp"
#include "emp/tools/attrs.hpp"
#include "emp/tools/TypeTracker.hpp"


template <size_t... VALS> struct TestBVConstruct;

template <size_t VAL1, size_t... VALS>
struct TestBVConstruct<VAL1, VALS...> {
  static void Run() {
    emp::BitSet<VAL1> bs;
    REQUIRE( bs.GetSize() == VAL1 );
    REQUIRE( bs.CountOnes() == 0 );
    for (size_t i = 0; i < VAL1; i++) bs[i] = true;
    REQUIRE( bs.CountOnes() == VAL1 );

    TestBVConstruct<VALS...>::Run();
  }
};

// Base case for constructors...
template <>
struct TestBVConstruct<> {
  static void Run(){}
};

TEST_CASE("1: Test BitSet Constructors", "[bits]"){
  // Create a size 50 bit vector, default to all zeros.
  emp::BitSet<50> bs1;
  REQUIRE( bs1.GetSize() == 50 );
  REQUIRE( bs1.CountOnes() == 0 );
  REQUIRE( (~bs1).CountOnes() == 50 );

  // Create a size 1000 BitSet, default to all ones.
  emp::BitSet<1000> bs2(true);
  REQUIRE( bs2.GetSize() == 1000 );
  REQUIRE( bs2.CountOnes() == 1000 );

  // Try a range of BitSet sizes, from 1 to 200.
  TestBVConstruct<1,2,7,8,9,15,16,17,31,32,33,63,64,65,127,128,129,191,192,193,200>::Run();

  // Build a relatively large BitSet.
  emp::BitSet<1000000> bs4;
  for (size_t i = 0; i < bs4.GetSize(); i += 100) bs4[i].Toggle();
  REQUIRE( bs4.CountOnes() == 10000 );

  // Try out the copy constructor.
  emp::BitSet<1000000> bs5(bs4);
  REQUIRE( bs5.GetSize() == 1000000 );
  REQUIRE( bs5.CountOnes() == 10000 );

  // Construct from std::bitset.
  std::bitset<6> bit_set;
  bit_set[1] = 1;   bit_set[2] = 1;   bit_set[4] = 1;
  emp::BitSet<6> bs7(bit_set);
  REQUIRE( bs7.GetSize() == 6 );
  REQUIRE( bs7.CountOnes() == 3 );

  // Construct from string.
  std::string bit_string = "10011001010000011101";
  emp::BitSet<20> bs8(bit_string);
  REQUIRE( bs8.GetSize() == 20 );
  REQUIRE( bs8.CountOnes() == 9 );

  // Some random BitSets
  emp::Random random;
  emp::BitSet<1000> bs9(random);            // 50/50 chance for each bit.
  const size_t bs9_ones = bs9.CountOnes();
  REQUIRE( bs9_ones >= 400 );
  REQUIRE( bs9_ones <= 600 );

  emp::BitSet<1000> bs10(random, 0.8);      // 80% chance of ones.
  const size_t bs10_ones = bs10.CountOnes();
  REQUIRE( bs10_ones >= 750 );
  REQUIRE( bs10_ones <= 850 );

  emp::BitSet<1000> bs11(random, 117);      // Exactly 117 ones, randomly placed.
  const size_t bs11_ones = bs11.CountOnes();
  REQUIRE( bs11_ones == 117 );

  emp::BitSet<13> bs12({1,0,0,0,1,1,1,0,0,0,1,1,1}); // Construct with initializer list.
  REQUIRE( bs12.GetSize() == 13 );
  REQUIRE( bs12.CountOnes() == 7 );
}


template <size_t... VALS> struct TestBVAssign;

template <size_t VAL1, size_t... VALS>
struct TestBVAssign<VAL1, VALS...> {
  static void Run() {
    emp::BitSet<VAL1> bs;

    // Copy to a second bs, make changes, then copy back.
    emp::BitSet<VAL1> bs2;

    for (size_t i = 1; i < bs2.GetSize(); i += 2) {
      bs2[i] = 1;
    }

    bs = bs2;

    REQUIRE( bs.CountOnes() == bs.GetSize()/2 );

    // Try copying in from an std::bitset.
    std::bitset<VAL1> bit_set;
    size_t num_ones = 0;
    if constexpr (VAL1 > 1)   { bit_set[1] = 1; num_ones++; }
    if constexpr (VAL1 > 22)  { bit_set[22] = 1; num_ones++; }
    if constexpr (VAL1 > 444) { bit_set[444] = 1; num_ones++; }

    bs2 = bit_set;  // Copy in an std::bitset.

    REQUIRE( bs2.GetSize() == VAL1 );
    REQUIRE( bs2.CountOnes() == num_ones );

    // Try copying from an std::string
    std::string bit_string = "100110010100000111011001100101000001110110011001010000011101";
    while (bit_string.size() < VAL1) bit_string += bit_string;
    bit_string.resize(VAL1);

    num_ones = 0;
    for (char x : bit_string) if (x == '1') num_ones++;

    bs2 = bit_string;

    REQUIRE( bs2.GetSize() == VAL1 );
    REQUIRE( bs2.CountOnes() == num_ones );

    TestBVAssign<VALS...>::Run();
  }
};

// Base case for constructors...
template<> struct TestBVAssign<> { static void Run(){} };

TEST_CASE("2: Test BitSet Assignemnts", "[bits]"){
  // Try a range of BitSet sizes, from 1 to 200.
  TestBVAssign<1,2,7,8,9,15,16,17,31,32,33,63,64,65,127,128,129,191,192,193,200,1023,1024,1025,1000000>::Run();
}


TEST_CASE("3: Test Simple BitSet Accessors", "[bits]"){
  emp::BitSet<1>  bs1(true);
  emp::BitSet<8>  bs8( "10001101" );
  emp::BitSet<32> bs32( "10001101100011011000110110001101" );
  emp::BitSet<64> bs64( "1000110110001101100000011000110000001101100000000000110110001101" );
  emp::BitSet<75> bs75( "010001011100010111110000011110100011111000001110100000111110010011111000011" );

  emp::Random random(1);
  emp::BitSet<1000> bs1k(random, 0.75);

  // Make sure all sizes are correct.
  REQUIRE( bs1.GetSize() == 1 );
  REQUIRE( bs8.GetSize() == 8 );
  REQUIRE( bs32.GetSize() == 32 );
  REQUIRE( bs64.GetSize() == 64 );
  REQUIRE( bs75.GetSize() == 75 );
  REQUIRE( bs1k.GetSize() == 1000 );

  // Check byte counts (should always round up!)
  REQUIRE( bs1.GetNumBytes() == 1 );     // round up!
  REQUIRE( bs8.GetNumBytes() == 1 );
  REQUIRE( bs32.GetNumBytes() == 4 );
  REQUIRE( bs64.GetNumBytes() == 8 );
  REQUIRE( bs75.GetNumBytes() == 10 );   // round up!
  REQUIRE( bs1k.GetNumBytes() == 125 );

  // How many states can be represented in each size of BitSet?
  REQUIRE( bs1.GetNumStates() == 2.0 );
  REQUIRE( bs8.GetNumStates() == 256.0 );
  REQUIRE( bs32.GetNumStates() == 4294967296.0 );
  REQUIRE( bs64.GetNumStates() >= 18446744073709551610.0 );
  REQUIRE( bs64.GetNumStates() <= 18446744073709551720.0 );
  REQUIRE( bs75.GetNumStates() >= 37778931862957161709560.0 );
  REQUIRE( bs75.GetNumStates() <= 37778931862957161709570.0 );
  REQUIRE( bs1k.GetNumStates() == emp::Pow2(1000) );

  // Test Get()
  REQUIRE( bs1.Get(0) == 1 );
  REQUIRE( bs8.Get(0) == 1 );
  REQUIRE( bs8.Get(1) == 0 );
  REQUIRE( bs8.Get(3) == 1 );
  REQUIRE( bs8.Get(7) == 1 );
  REQUIRE( bs75.Get(0) == 1 );
  REQUIRE( bs75.Get(1) == 1 );
  REQUIRE( bs75.Get(2) == 0 );
  REQUIRE( bs75.Get(72) == 0 );
  REQUIRE( bs75.Get(73) == 1 );
  REQUIRE( bs75.Get(74) == 0 );

  // Test Has() (including out of range)
  REQUIRE( bs1.Has(0) == true );
  REQUIRE( bs1.Has(1) == false );
  REQUIRE( bs1.Has(1000000) == false );

  REQUIRE( bs8.Has(0) == true );
  REQUIRE( bs8.Has(1) == false );
  REQUIRE( bs8.Has(4) == false );
  REQUIRE( bs8.Has(7) == true );
  REQUIRE( bs8.Has(8) == false );

  REQUIRE( bs75.Has(0) == true );
  REQUIRE( bs75.Has(1) == true );
  REQUIRE( bs75.Has(2) == false );
  REQUIRE( bs75.Has(72) == false );
  REQUIRE( bs75.Has(73) == true );
  REQUIRE( bs75.Has(74) == false );
  REQUIRE( bs75.Has(75) == false );       // Out of bounds (which Has() is okay with...)
  REQUIRE( bs75.Has(79) == false );
  REQUIRE( bs75.Has(1000000) == false );

  // Test Set(), changing in most (but not all) cases.
  bs1.Set(0, 0);
  REQUIRE( bs1.Get(0) == 0 );
  bs8.Set(0, 1);                // Already a 1!
  REQUIRE( bs8.Get(0) == 1 );
  bs8.Set(4, 0);
  REQUIRE( bs8.Get(4) == 0 );
  bs8.Set(6, 1);
  REQUIRE( bs8.Get(6) == 1 );
  bs8.Set(7, 0);
  REQUIRE( bs8.Get(7) == 0 );
  bs75.Set(0, 0);               // Already a 0!
  REQUIRE( bs75.Get(0) == 0 );
  bs75.Set(1, 0);
  REQUIRE( bs75.Get(1) == 0 );
  bs75.Set(72);                 // No second arg!
  REQUIRE( bs75.Get(72) == 1 );
  bs75.Set(73);                 // No second arg AND already a 1!
  REQUIRE( bs75.Get(73) == 1 );
  bs75.Set(74, 0);
  REQUIRE( bs75.Get(74) == 0 );
}

TEST_CASE("4: Test BitSet Set*, Clear* and Toggle* Accessors", "[bits]") {
  // Now try range-based accessors on a single bit.
  emp::BitSet<1> bs1(false);  REQUIRE( bs1[0] == false );   REQUIRE( bs1.CountOnes() == 0 );
  bs1.Set(0);                 REQUIRE( bs1[0] == true );    REQUIRE( bs1.CountOnes() == 1 );
  bs1.Clear(0);               REQUIRE( bs1[0] == false );   REQUIRE( bs1.CountOnes() == 0 );
  bs1.Toggle(0);              REQUIRE( bs1[0] == true );    REQUIRE( bs1.CountOnes() == 1 );
  bs1.Clear();                REQUIRE( bs1[0] == false );   REQUIRE( bs1.CountOnes() == 0 );
  bs1.SetAll();               REQUIRE( bs1[0] == true );    REQUIRE( bs1.CountOnes() == 1 );
  bs1.Toggle();               REQUIRE( bs1[0] == false );   REQUIRE( bs1.CountOnes() == 0 );
  bs1.SetRange(0,1);          REQUIRE( bs1[0] == true );    REQUIRE( bs1.CountOnes() == 1 );
  bs1.Clear(0,1);             REQUIRE( bs1[0] == false );   REQUIRE( bs1.CountOnes() == 0 );
  bs1.Toggle(0,1);            REQUIRE( bs1[0] == true );    REQUIRE( bs1.CountOnes() == 1 );
  bs1.Set(0, false);          REQUIRE( bs1[0] == false );   REQUIRE( bs1.CountOnes() == 0 );
  bs1.SetRange(0,0);          REQUIRE( bs1[0] == false );   REQUIRE( bs1.CountOnes() == 0 );
  bs1.SetRange(1,1);          REQUIRE( bs1[0] == false );   REQUIRE( bs1.CountOnes() == 0 );

  // Test when a full byte is used.
  emp::BitSet<8> bs8( "10110001" );   REQUIRE(bs8.GetValue() == 177.0);  // 10110001
  bs8.Set(2);                         REQUIRE(bs8.GetValue() == 181.0);  // 10110101
  bs8.Set(0, 0);                      REQUIRE(bs8.GetValue() == 180.0);  // 10110100
  bs8.SetRange(1, 4);                 REQUIRE(bs8.GetValue() == 190.0);  // 10111110
  bs8.SetAll();                       REQUIRE(bs8.GetValue() == 255.0);  // 11111111
  bs8.Clear(3);                       REQUIRE(bs8.GetValue() == 247.0);  // 11110111
  bs8.Clear(5,5);                     REQUIRE(bs8.GetValue() == 247.0);  // 11110111
  bs8.Clear(5,7);                     REQUIRE(bs8.GetValue() == 151.0);  // 10010111
  bs8.Clear();                        REQUIRE(bs8.GetValue() ==   0.0);  // 00000000
  bs8.Toggle(4);                      REQUIRE(bs8.GetValue() ==  16.0);  // 00010000
  bs8.Toggle(4,6);                    REQUIRE(bs8.GetValue() ==  32.0);  // 00100000
  bs8.Toggle(0,3);                    REQUIRE(bs8.GetValue() ==  39.0);  // 00100111
  bs8.Toggle(7,8);                    REQUIRE(bs8.GetValue() == 167.0);  // 10100111
  bs8.Toggle();                       REQUIRE(bs8.GetValue() ==  88.0);  // 01011000

  // Test a full field.
  constexpr double ALL_64 = (double) ((uint64_t) -1);
  emp::BitSet<64> bs64( "10110001101100011011" );
  REQUIRE(bs64.GetValue() == 727835.0);
  bs64.Set(6);          REQUIRE(bs64.GetValue() == 727899.0);        // ...0 010110001101101011011
  bs64.Set(0, 0);       REQUIRE(bs64.GetValue() == 727898.0);        // ...0 010110001101101011010
  bs64.SetRange(4, 9);  REQUIRE(bs64.GetValue() == 728058.0);        // ...0 010110001101111111010
  bs64.SetAll();        REQUIRE(bs64.GetValue() == ALL_64);          // ...1 111111111111111111111
  bs64.Clear(2);        REQUIRE(bs64.GetValue() == ALL_64 - 4);      // ...1 111111111111111111011
  bs64.Clear(5,5);      REQUIRE(bs64.GetValue() == ALL_64 - 4);      // ...1 111111111111111111011
  bs64.Clear(5,7);      REQUIRE(bs64.GetValue() == ALL_64 - 100);    // ...1 111111111111110011011
  bs64.Clear();         REQUIRE(bs64.GetValue() == 0.0);             // ...0 000000000000000000000
  bs64.Toggle(19);      REQUIRE(bs64.GetValue() == emp::Pow2(19));   // ...0 010000000000000000000
  bs64.Toggle(15,20);   REQUIRE(bs64.GetValue() == 491520.0);        // ...0 001111000000000000000
  bs64.Toggle();        REQUIRE(bs64.GetValue() == ALL_64-491520.0); // ...1 110000111111111111111
  bs64.Toggle(0,64);    REQUIRE(bs64.GetValue() == 491520.0);        // ...0 001111000000000000000


  emp::BitSet<75> bs75( "110000111110010011111000001011100000111110001011110000011111010001110100010" );

  // Test a full + partial field.
  constexpr double ALL_88 = ((double) ((uint64_t) -1)) * emp::Pow2(24);
  emp::BitSet<88> bs88( "11011000110110001101" );
  REQUIRE(bs88.GetValue() == 888205.0);                              // ...0 010110001101100011011

  // Start with same tests as last time...
  bs88.Set(6);          REQUIRE(bs88.GetValue() == 888269.0);        // ...0 010110001101101011011
  bs88.Set(0, 0);       REQUIRE(bs88.GetValue() == 888268.0);        // ...0 010110001101101011010
  bs88.SetRange(4, 9);  REQUIRE(bs88.GetValue() == 888316.0);        // ...0 010110001101111111010
  bs88.SetAll();        REQUIRE(bs88.GetValue() == ALL_88);          // ...1 111111111111111111111
  bs88.Clear(2);        REQUIRE(bs88.GetValue() == ALL_88 - 4);      // ...1 111111111111111111011
  bs88.Clear(5,5);      REQUIRE(bs88.GetValue() == ALL_88 - 4);      // ...1 111111111111111111011
  bs88.Clear(5,7);      REQUIRE(bs88.GetValue() == ALL_88 - 100);    // ...1 111111111111110011011
  bs88.Clear();         REQUIRE(bs88.GetValue() == 0.0);             // ...0 000000000000000000000
  bs88.Toggle(19);      REQUIRE(bs88.GetValue() == emp::Pow2(19));   // ...0 010000000000000000000
  bs88.Toggle(15,20);   REQUIRE(bs88.GetValue() == 491520.0);        // ...0 001111000000000000000
  bs88.Toggle();        REQUIRE(bs88.GetValue() == ALL_88-491520.0); // ...1 110000111111111111111
  bs88.Toggle(0,88);    REQUIRE(bs88.GetValue() == 491520.0);        // ...0 001111000000000000000

  bs88 <<= 20;          REQUIRE(bs88.CountOnes() == 4);   // four ones, moved to bits 35-39
  bs88 <<= 27;          REQUIRE(bs88.CountOnes() == 4);   // four ones, moved to bits 62-65
  bs88 <<= 22;          REQUIRE(bs88.CountOnes() == 4);   // four ones, moved to bits 84-87
  bs88 <<= 1;           REQUIRE(bs88.CountOnes() == 3);   // three ones left, moved to bits 85-87
  bs88 <<= 2;           REQUIRE(bs88.CountOnes() == 1);   // one one left, at bit 87
  bs88 >>= 30;          REQUIRE(bs88.CountOnes() == 1);   // one one left, now at bit 57
  bs88.Toggle(50,80);   REQUIRE(bs88.CountOnes() == 29);  // Toggling 30 bits, only one was on.
  bs88.Clear(52,78);    REQUIRE(bs88.CountOnes() == 4);   // Leave two 1s on each side of range
  bs88.SetRange(64,66); REQUIRE(bs88.CountOnes() == 6);   // Set two more 1s, just into 2nd field.

  // A larger BitSet with lots of random tests.
  emp::Random random(1);
  emp::BitSet<1000> bs1k(random, 0.65);
  size_t num_ones = bs1k.CountOnes();  REQUIRE(num_ones > 550);
  bs1k.Toggle();                       REQUIRE(bs1k.CountOnes() == 1000 - num_ones);

  for (size_t test_id = 0; test_id < 10000; ++test_id) {
    size_t val1 = random.GetUInt(1000);
    size_t val2 = random.GetUInt(1001);
    if (val1 > val2) std::swap(val1, val2);
    bs1k.Toggle(val1, val2);

    val1 = random.GetUInt(1000);
    val2 = random.GetUInt(1001);
    if (val1 > val2) std::swap(val1, val2);
    bs1k.Clear(val1, val2);

    val1 = random.GetUInt(1000);
    val2 = random.GetUInt(1001);
    if (val1 > val2) std::swap(val1, val2);
    bs1k.SetRange(val1, val2);
  }

  // Test Any(), All() and None()
  emp::BitSet<6> bs_empty = "000000";
  emp::BitSet<6> bs_mixed = "010101";
  emp::BitSet<6> bs_full  = "111111";

  REQUIRE(bs_empty.Any() == false);
  REQUIRE(bs_mixed.Any() == true);
  REQUIRE(bs_full.Any() == true);

  REQUIRE(bs_empty.All() == false);
  REQUIRE(bs_mixed.All() == false);
  REQUIRE(bs_full.All() == true);

  REQUIRE(bs_empty.None() == true);
  REQUIRE(bs_mixed.None() == false);
  REQUIRE(bs_full.None() == false);
}


TEST_CASE("5: Test Randomize() and variants", "[bits]") {
  emp::Random random(1);
  emp::BitSet<1000> bs;

  REQUIRE(bs.None() == true);

  // Do all of the random tests 10 times.
  for (size_t test_num = 0; test_num < 10; test_num++) {
    bs.Randomize(random);
    size_t num_ones = bs.CountOnes();
    REQUIRE(num_ones > 300);
    REQUIRE(num_ones < 700);

    // 85% Chance of 1
    bs.Randomize(random, 0.85);
    num_ones = bs.CountOnes();
    REQUIRE(num_ones > 700);
    REQUIRE(num_ones < 950);

    // 15% Chance of 1
    bs.Randomize(random, 0.15);
    num_ones = bs.CountOnes();
    REQUIRE(num_ones > 50);
    REQUIRE(num_ones < 300);

    // Try randomizing only a portion of the genome.
    uint64_t first_bits = bs.GetUInt64(0);
    bs.Randomize(random, 0.7, 64, 1000);

    REQUIRE(bs.GetUInt64(0) == first_bits);  // Make sure first bits haven't changed

    num_ones = bs.CountOnes();
    REQUIRE(num_ones > 500);                 // Expected with new randomization is ~665 ones.
    REQUIRE(num_ones < 850);

    // Try randomizing using specific numbers of ones.
    bs.ChooseRandom(random, 1);       REQUIRE(bs.CountOnes() == 1);
    bs.ChooseRandom(random, 12);      REQUIRE(bs.CountOnes() == 12);
    bs.ChooseRandom(random, 128);     REQUIRE(bs.CountOnes() == 128);
    bs.ChooseRandom(random, 507);     REQUIRE(bs.CountOnes() == 507);
    bs.ChooseRandom(random, 999);     REQUIRE(bs.CountOnes() == 999);

    // Test the probabilistic CHANGE functions.
    bs.Clear();                     REQUIRE(bs.CountOnes() == 0);   // Set all bits to 0.

    bs.FlipRandom(random, 0.3);     // Exprected: 300 ones (from flipping zeros)
    num_ones = bs.CountOnes();      REQUIRE(num_ones > 230);  REQUIRE(num_ones < 375);

    bs.FlipRandom(random, 0.3);     // Exprected: 420 ones (hit by ONE but not both flips)
    num_ones = bs.CountOnes();      REQUIRE(num_ones > 345);  REQUIRE(num_ones < 495);

    bs.SetRandom(random, 0.5);      // Expected: 710 (already on OR newly turned on)
    num_ones = bs.CountOnes();      REQUIRE(num_ones > 625);  REQUIRE(num_ones < 775);

    bs.SetRandom(random, 0.8);      // Expected: 942 (already on OR newly turned on)
    num_ones = bs.CountOnes();      REQUIRE(num_ones > 900);  REQUIRE(num_ones < 980);

    bs.ClearRandom(random, 0.2);    // Expected 753.6 (20% of those on now off)
    num_ones = bs.CountOnes();      REQUIRE(num_ones > 675);  REQUIRE(num_ones < 825);

    bs.FlipRandom(random, 0.5);     // Exprected: 500 ones (each bit has a 50% chance of flipping)
    num_ones = bs.CountOnes();      REQUIRE(num_ones > 425);  REQUIRE(num_ones < 575);


    // Repeat with fixed-sized changes.
    bs.Clear();                        REQUIRE(bs.CountOnes() == 0);     // Set all bits to 0.

    bs.FlipRandomCount(random, 123);   // Flip exactly 123 bits to 1.
    num_ones = bs.CountOnes();         REQUIRE(num_ones == 123);

    bs.FlipRandomCount(random, 877);   // Flip exactly 877 bits; Expected 784.258 ones
    num_ones = bs.CountOnes();         REQUIRE(num_ones > 700);  REQUIRE(num_ones < 850);


    bs.SetAll();                       REQUIRE(bs.CountOnes() == 1000);  // Set all bits to 1.

    bs.ClearRandomCount(random, 123);
    num_ones = bs.CountOnes();         REQUIRE(num_ones == 877);

    bs.ClearRandomCount(random, 877);  // Clear exactly 877 bits; Expected 107.871 ones
    num_ones = bs.CountOnes();         REQUIRE(num_ones > 60);  REQUIRE(num_ones < 175);

    bs.SetRandomCount(random, 500);    // Half of the remaining ones should be set; 553.9355 expected.
    num_ones = bs.CountOnes();         REQUIRE(num_ones > 485);  REQUIRE(num_ones < 630);


    bs.Clear();                        REQUIRE(bs.CountOnes() == 0);     // Set all bits to 0.
    bs.SetRandomCount(random, 567);    // Half of the remaining ones should be set; 607.871 expected.
    num_ones = bs.CountOnes();         REQUIRE(num_ones == 567);
  }

  // During randomization, make sure each bit position is set appropriately.
  std::vector<size_t> one_counts(1000, 0);

  for (size_t test_num = 0; test_num < 1000; ++test_num) {
    // Set bits with different probabilities in different ranges.
    bs.Clear();
    bs.Randomize(random, 0.5,  100, 250);
    bs.Randomize(random, 0.25, 250, 400);
    bs.Randomize(random, 0.75, 400, 550);
    bs.Randomize(random, 0.10, 550, 700);
    bs.Randomize(random, 0.98, 700, 850);

    // Keep count of how many times each position was a one.
    for (size_t i = 0; i < bs.GetSize(); ++i) {
      if (bs.Get(i)) one_counts[i]++;
    }
  }

  // Check if the counts are reasonable.
  for (size_t i = 0;   i < 100; i++)  { REQUIRE(one_counts[i] == 0); }
  for (size_t i = 100; i < 250; i++)  { REQUIRE(one_counts[i] > 410);  REQUIRE(one_counts[i] < 590); }
  for (size_t i = 250; i < 400; i++)  { REQUIRE(one_counts[i] > 190);  REQUIRE(one_counts[i] < 320); }
  for (size_t i = 400; i < 550; i++)  { REQUIRE(one_counts[i] > 680);  REQUIRE(one_counts[i] < 810); }
  for (size_t i = 550; i < 700; i++)  { REQUIRE(one_counts[i] >  60);  REQUIRE(one_counts[i] < 150); }
  for (size_t i = 700; i < 850; i++)  { REQUIRE(one_counts[i] > 950);  REQUIRE(one_counts[i] < 999); }
  for (size_t i = 850; i < 1000; i++) { REQUIRE(one_counts[i] == 0); }
}

TEST_CASE("6: Test getting and setting whole chunks of bits", "[bits]") {
  constexpr size_t num_bits = 145;
  constexpr size_t num_bytes = 19;

  emp::BitSet<num_bits> bs;
  REQUIRE(bs.GetSize() == num_bits);
  REQUIRE(bs.GetNumBytes() == num_bytes);

  // All bytes should start out empty.
  for (size_t i = 0; i < num_bytes; i++) REQUIRE(bs.GetByte(i) == 0);

  bs.SetByte(2, 11);
  REQUIRE(bs.GetByte(2) == 11);

  REQUIRE(bs.GetValue() == 720896.0);

  bs.SetByte(5, 7);
  REQUIRE(bs.GetByte(0) == 0);
  REQUIRE(bs.GetByte(1) == 0);
  REQUIRE(bs.GetByte(2) == 11);
  REQUIRE(bs.GetByte(3) == 0);
  REQUIRE(bs.GetByte(4) == 0);
  REQUIRE(bs.GetByte(5) == 7);
  REQUIRE(bs.GetByte(6) == 0);
  REQUIRE(bs.CountOnes() == 6);

  for (size_t i = 0; i < num_bytes; i++) REQUIRE(bs.GetByte(i) == bs.GetUInt8(i));

  REQUIRE(bs.GetUInt16(0) == 0);
  REQUIRE(bs.GetUInt16(1) == 11);
  REQUIRE(bs.GetUInt16(2) == 1792);
  REQUIRE(bs.GetUInt16(3) == 0);

  REQUIRE(bs.GetUInt32(0) == 720896);
  REQUIRE(bs.GetUInt32(1) == 1792);
  REQUIRE(bs.GetUInt32(2) == 0);

  REQUIRE(bs.GetUInt64(0) == 7696582115328);
  REQUIRE(bs.GetUInt64(1) == 0);

  bs.SetUInt64(0, 12345678901234);
  bs.SetUInt32(2, 2000000);
  bs.SetUInt16(7, 7777);
  bs.SetUInt8(17, 17);

  REQUIRE(bs.GetUInt64(0) == 12345678901234);
  REQUIRE(bs.GetUInt32(2) == 2000000);
  REQUIRE(bs.GetUInt16(7) == 7777);
  REQUIRE(bs.GetUInt8(17) == 17);

  bs.Clear();
  bs.SetUInt16AtBit(40, 40);

  REQUIRE(bs.GetUInt16AtBit(40) == 40);

  REQUIRE(bs.GetUInt8(5) == 40);
  REQUIRE(bs.GetUInt8AtBit(40) == 40);
  REQUIRE(bs.GetUInt32AtBit(40) == 40);
  REQUIRE(bs.GetUInt64AtBit(40) == 40);

  REQUIRE(bs.GetUInt16AtBit(38) == 160);
  REQUIRE(bs.GetUInt16AtBit(39) == 80);
  REQUIRE(bs.GetUInt16AtBit(41) == 20);
  REQUIRE(bs.GetUInt16AtBit(42) == 10);

  REQUIRE(bs.GetUInt8AtBit(38) == 160);
  REQUIRE(bs.GetUInt8AtBit(37) == 64);
  REQUIRE(bs.GetUInt8AtBit(36) == 128);
  REQUIRE(bs.GetUInt8AtBit(35) == 0);
}

TEST_CASE("7: Test functions that analyze and manipulate ones", "[bits]") {

  emp::BitSet<16> bs = "0111000010001000";

  REQUIRE(bs.GetSize() == 16);
  REQUIRE(bs.CountOnes() == 5);

  // Make sure we can find all of the ones.
  REQUIRE(bs.FindOne() == 3);
  REQUIRE(bs.FindOne(4) == 7);
  REQUIRE(bs.FindOne(5) == 7);
  REQUIRE(bs.FindOne(6) == 7);
  REQUIRE(bs.FindOne(7) == 7);
  REQUIRE(bs.FindOne(8) == 12);
  REQUIRE(bs.FindOne(13) == 13);
  REQUIRE(bs.FindOne(14) == 14);
  REQUIRE(bs.FindOne(15) == -1);

  // Get all of the ones at once and make sure they're there.
  emp::vector<size_t> ones = bs.GetOnes();
  REQUIRE(ones.size() == 5);
  REQUIRE(ones[0] == 3);
  REQUIRE(ones[1] == 7);
  REQUIRE(ones[2] == 12);
  REQUIRE(ones[3] == 13);
  REQUIRE(ones[4] == 14);

  // Try finding the length of the longest segment of ones.
  REQUIRE(bs.LongestSegmentOnes() == 3);

  // Identify the final one.
  REQUIRE(bs.FindMaxOne() == 14);

  // Pop all ones, one at a time.
  REQUIRE(bs.PopOne() == 3);
  REQUIRE(bs.PopOne() == 7);
  REQUIRE(bs.PopOne() == 12);
  REQUIRE(bs.PopOne() == 13);
  REQUIRE(bs.PopOne() == 14);
  REQUIRE(bs.PopOne() == -1);

  REQUIRE(bs.CountOnes() == 0);
  REQUIRE(bs.LongestSegmentOnes() == 0);
  REQUIRE(bs.FindMaxOne() == -1);


  bs.SetAll();                             // 1111111111111111
  REQUIRE(bs.LongestSegmentOnes() == 16);
  bs[8] = 0;                               // 1111111101111111
  REQUIRE(bs.LongestSegmentOnes() == 8);
  bs[4] = 0;                               // 1111011101111111
  REQUIRE(bs.LongestSegmentOnes() == 7);

  // Try again with Find, this time with a random sequence of ones.
  emp::Random random(1);
  bs.Randomize(random);
  size_t count = 0;
  for (int i = bs.FindOne(); i != -1; i = bs.FindOne(i+1)) count++;
  REQUIRE(count == bs.CountOnes());

}

TEST_CASE("8: Test printing and string functions.", "[bits]") {
  emp::BitSet<6> bs6("111000");

  REQUIRE(bs6.ToString() == "111000");
  REQUIRE(bs6.ToBinaryString() == "111000");
  REQUIRE(bs6.ToArrayString() == "000111");
  REQUIRE(bs6.ToIDString() == "3 4 5");
  REQUIRE(bs6.ToIDString() == "3 4 5");
  REQUIRE(bs6.ToRangeString() == "3-5");

  emp::BitSet<64> bs64("1000000010001000001000000110001000000000000010000000000000111000");

  REQUIRE(bs64.ToArrayString()  == "0001110000000000000100000000000001000110000001000001000100000001");
  REQUIRE(bs64.ToBinaryString() == "1000000010001000001000000110001000000000000010000000000000111000");
  REQUIRE(bs64.ToIDString() == "3 4 5 19 33 37 38 45 51 55 63");
  REQUIRE(bs64.ToIDString(",") == "3,4,5,19,33,37,38,45,51,55,63");
  REQUIRE(bs64.ToRangeString() == "3-5,19,33,37-38,45,51,55,63");

//  emp::BitSet<65> bs65("00011110000000000001000000000000010001100000010000010001000000111");
  emp::BitSet<65> bs65("11100000010001000001000000110001000000000000010000000000001111000");

  REQUIRE(bs65.ToArrayString()  == "00011110000000000001000000000000010001100000010000010001000000111");
  REQUIRE(bs65.ToBinaryString() == "11100000010001000001000000110001000000000000010000000000001111000");
  REQUIRE(bs65.ToIDString()     == "3 4 5 6 19 33 37 38 45 51 55 62 63 64");
  REQUIRE(bs65.ToIDString(",")  == "3,4,5,6,19,33,37,38,45,51,55,62,63,64");
  REQUIRE(bs65.ToRangeString()  == "3-6,19,33,37-38,45,51,55,62-64");
}

TEST_CASE("9: Test Boolean logic and shifting functions.", "[bits]") {
  const emp::BitSet<8> input1 = "00001111";
  const emp::BitSet<8> input2 = "00110011";
  const emp::BitSet<8> input3 = "01010101";

  // Test *_SELF() Boolean Logic functions.
  emp::BitSet<8> bs;       REQUIRE(bs == emp::BitSet<8>("00000000"));
  bs.NOT_SELF();           REQUIRE(bs == emp::BitSet<8>("11111111"));
  bs.AND_SELF(input1);     REQUIRE(bs == emp::BitSet<8>("00001111"));
  bs.AND_SELF(input1);     REQUIRE(bs == emp::BitSet<8>("00001111"));
  bs.AND_SELF(input2);     REQUIRE(bs == emp::BitSet<8>("00000011"));
  bs.AND_SELF(input3);     REQUIRE(bs == emp::BitSet<8>("00000001"));

  bs.OR_SELF(input1);      REQUIRE(bs == emp::BitSet<8>("00001111"));
  bs.OR_SELF(input1);      REQUIRE(bs == emp::BitSet<8>("00001111"));
  bs.OR_SELF(input3);      REQUIRE(bs == emp::BitSet<8>("01011111"));
  bs.OR_SELF(input2);      REQUIRE(bs == emp::BitSet<8>("01111111"));

  bs.NAND_SELF(input1);    REQUIRE(bs == emp::BitSet<8>("11110000"));
  bs.NAND_SELF(input1);    REQUIRE(bs == emp::BitSet<8>("11111111"));
  bs.NAND_SELF(input2);    REQUIRE(bs == emp::BitSet<8>("11001100"));
  bs.NAND_SELF(input3);    REQUIRE(bs == emp::BitSet<8>("10111011"));

  bs.NOR_SELF(input1);     REQUIRE(bs == emp::BitSet<8>("01000000"));
  bs.NOR_SELF(input1);     REQUIRE(bs == emp::BitSet<8>("10110000"));
  bs.NOR_SELF(input2);     REQUIRE(bs == emp::BitSet<8>("01001100"));
  bs.NOR_SELF(input3);     REQUIRE(bs == emp::BitSet<8>("10100010"));

  bs.XOR_SELF(input1);     REQUIRE(bs == emp::BitSet<8>("10101101"));
  bs.XOR_SELF(input1);     REQUIRE(bs == emp::BitSet<8>("10100010"));
  bs.XOR_SELF(input2);     REQUIRE(bs == emp::BitSet<8>("10010001"));
  bs.XOR_SELF(input3);     REQUIRE(bs == emp::BitSet<8>("11000100"));

  bs.EQU_SELF(input1);     REQUIRE(bs == emp::BitSet<8>("00110100"));
  bs.EQU_SELF(input1);     REQUIRE(bs == emp::BitSet<8>("11000100"));
  bs.EQU_SELF(input2);     REQUIRE(bs == emp::BitSet<8>("00001000"));
  bs.EQU_SELF(input3);     REQUIRE(bs == emp::BitSet<8>("10100010"));

  bs.NOT_SELF();           REQUIRE(bs == emp::BitSet<8>("01011101"));

  // Test regular Boolean Logic functions.
  bs.Clear();                            REQUIRE(bs == emp::BitSet<8>("00000000"));
  emp::BitSet<8> bs1 = bs.NOT();         REQUIRE(bs1 == emp::BitSet<8>("11111111"));

  bs1 = bs1.AND(input1);                 REQUIRE(bs1 == emp::BitSet<8>("00001111"));
  emp::BitSet<8> bs2 = bs1.AND(input1);  REQUIRE(bs2 == emp::BitSet<8>("00001111"));
  emp::BitSet<8> bs3 = bs2.AND(input2);  REQUIRE(bs3 == emp::BitSet<8>("00000011"));
  emp::BitSet<8> bs4 = bs3.AND(input3);  REQUIRE(bs4 == emp::BitSet<8>("00000001"));

  bs1 = bs4.OR(input1);      REQUIRE(bs1 == emp::BitSet<8>("00001111"));
  bs2 = bs1.OR(input1);      REQUIRE(bs2 == emp::BitSet<8>("00001111"));
  bs3 = bs2.OR(input3);      REQUIRE(bs3 == emp::BitSet<8>("01011111"));
  bs4 = bs3.OR(input2);      REQUIRE(bs4 == emp::BitSet<8>("01111111"));

  bs1 = bs4.NAND(input1);    REQUIRE(bs1 == emp::BitSet<8>("11110000"));
  bs2 = bs1.NAND(input1);    REQUIRE(bs2 == emp::BitSet<8>("11111111"));
  bs3 = bs2.NAND(input2);    REQUIRE(bs3 == emp::BitSet<8>("11001100"));
  bs4 = bs3.NAND(input3);    REQUIRE(bs4 == emp::BitSet<8>("10111011"));

  bs1 = bs4.NOR(input1);     REQUIRE(bs1 == emp::BitSet<8>("01000000"));
  bs2 = bs1.NOR(input1);     REQUIRE(bs2 == emp::BitSet<8>("10110000"));
  bs3 = bs2.NOR(input2);     REQUIRE(bs3 == emp::BitSet<8>("01001100"));
  bs4 = bs3.NOR(input3);     REQUIRE(bs4 == emp::BitSet<8>("10100010"));

  bs1 = bs4.XOR(input1);     REQUIRE(bs1 == emp::BitSet<8>("10101101"));
  bs2 = bs1.XOR(input1);     REQUIRE(bs2 == emp::BitSet<8>("10100010"));
  bs3 = bs2.XOR(input2);     REQUIRE(bs3 == emp::BitSet<8>("10010001"));
  bs4 = bs3.XOR(input3);     REQUIRE(bs4 == emp::BitSet<8>("11000100"));

  bs1 = bs4.EQU(input1);     REQUIRE(bs1 == emp::BitSet<8>("00110100"));
  bs2 = bs1.EQU(input1);     REQUIRE(bs2 == emp::BitSet<8>("11000100"));
  bs3 = bs2.EQU(input2);     REQUIRE(bs3 == emp::BitSet<8>("00001000"));
  bs4 = bs3.EQU(input3);     REQUIRE(bs4 == emp::BitSet<8>("10100010"));

  bs = bs4.NOT();            REQUIRE(bs == emp::BitSet<8>("01011101"));


  // Test Boolean Logic operators.
  bs.Clear();               REQUIRE(bs == emp::BitSet<8>("00000000"));
  bs1 = ~bs;                REQUIRE(bs1 == emp::BitSet<8>("11111111"));

  bs1 = bs1 & input1;       REQUIRE(bs1 == emp::BitSet<8>("00001111"));
  bs2 = bs1 & input1;       REQUIRE(bs2 == emp::BitSet<8>("00001111"));
  bs3 = bs2 & input2;       REQUIRE(bs3 == emp::BitSet<8>("00000011"));
  bs4 = bs3 & input3;       REQUIRE(bs4 == emp::BitSet<8>("00000001"));

  bs1 = bs4 | input1;       REQUIRE(bs1 == emp::BitSet<8>("00001111"));
  bs2 = bs1 | input1;       REQUIRE(bs2 == emp::BitSet<8>("00001111"));
  bs3 = bs2 | input3;       REQUIRE(bs3 == emp::BitSet<8>("01011111"));
  bs4 = bs3 | input2;       REQUIRE(bs4 == emp::BitSet<8>("01111111"));

  bs1 = ~(bs4 & input1);    REQUIRE(bs1 == emp::BitSet<8>("11110000"));
  bs2 = ~(bs1 & input1);    REQUIRE(bs2 == emp::BitSet<8>("11111111"));
  bs3 = ~(bs2 & input2);    REQUIRE(bs3 == emp::BitSet<8>("11001100"));
  bs4 = ~(bs3 & input3);    REQUIRE(bs4 == emp::BitSet<8>("10111011"));

  bs1 = ~(bs4 | input1);    REQUIRE(bs1 == emp::BitSet<8>("01000000"));
  bs2 = ~(bs1 | input1);    REQUIRE(bs2 == emp::BitSet<8>("10110000"));
  bs3 = ~(bs2 | input2);    REQUIRE(bs3 == emp::BitSet<8>("01001100"));
  bs4 = ~(bs3 | input3);    REQUIRE(bs4 == emp::BitSet<8>("10100010"));

  bs1 = bs4 ^ input1;       REQUIRE(bs1 == emp::BitSet<8>("10101101"));
  bs2 = bs1 ^ input1;       REQUIRE(bs2 == emp::BitSet<8>("10100010"));
  bs3 = bs2 ^ input2;       REQUIRE(bs3 == emp::BitSet<8>("10010001"));
  bs4 = bs3 ^ input3;       REQUIRE(bs4 == emp::BitSet<8>("11000100"));

  bs1 = ~(bs4 ^ input1);    REQUIRE(bs1 == emp::BitSet<8>("00110100"));
  bs2 = ~(bs1 ^ input1);    REQUIRE(bs2 == emp::BitSet<8>("11000100"));
  bs3 = ~(bs2 ^ input2);    REQUIRE(bs3 == emp::BitSet<8>("00001000"));
  bs4 = ~(bs3 ^ input3);    REQUIRE(bs4 == emp::BitSet<8>("10100010"));

  bs = ~bs4;                REQUIRE(bs == emp::BitSet<8>("01011101"));


  // Test COMPOUND Boolean Logic operators.
  bs = "11111111";    REQUIRE(bs == emp::BitSet<8>("11111111"));

  bs &= input1;       REQUIRE(bs == emp::BitSet<8>("00001111"));
  bs &= input1;       REQUIRE(bs == emp::BitSet<8>("00001111"));
  bs &= input2;       REQUIRE(bs == emp::BitSet<8>("00000011"));
  bs &= input3;       REQUIRE(bs == emp::BitSet<8>("00000001"));

  bs |= input1;       REQUIRE(bs == emp::BitSet<8>("00001111"));
  bs |= input1;       REQUIRE(bs == emp::BitSet<8>("00001111"));
  bs |= input3;       REQUIRE(bs == emp::BitSet<8>("01011111"));
  bs |= input2;       REQUIRE(bs == emp::BitSet<8>("01111111"));

  bs ^= input1;       REQUIRE(bs == emp::BitSet<8>("01110000"));
  bs ^= input1;       REQUIRE(bs == emp::BitSet<8>("01111111"));
  bs ^= input2;       REQUIRE(bs == emp::BitSet<8>("01001100"));
  bs ^= input3;       REQUIRE(bs == emp::BitSet<8>("00011001"));

  // Now some tests with BitSets longer than one field.
  const emp::BitSet<80> bsl80 =
    "00110111000101110001011100010111000101110001011100010111000101110001011100010111";
  REQUIRE( bsl80.GetSize() == 80 );
  REQUIRE( bsl80.CountOnes() == 41 );
  REQUIRE( (bsl80 >> 1) ==
           emp::BitSet<80>("00011011100010111000101110001011100010111000101110001011100010111000101110001011")
         );
  REQUIRE( (bsl80 >> 2) ==
           emp::BitSet<80>("00001101110001011100010111000101110001011100010111000101110001011100010111000101")
         );
  REQUIRE( (bsl80 >> 63) ==
           emp::BitSet<80>("00000000000000000000000000000000000000000000000000000000000000000110111000101110")
         );
  REQUIRE( (bsl80 >> 64) ==
           emp::BitSet<80>("00000000000000000000000000000000000000000000000000000000000000000011011100010111")
         );
  REQUIRE( (bsl80 >> 65) ==
           emp::BitSet<80>("00000000000000000000000000000000000000000000000000000000000000000001101110001011")
         );

  REQUIRE( (bsl80 << 1) ==
           emp::BitSet<80>("01101110001011100010111000101110001011100010111000101110001011100010111000101110")
         );
  REQUIRE( (bsl80 << 2) ==
           emp::BitSet<80>("11011100010111000101110001011100010111000101110001011100010111000101110001011100")
         );
  REQUIRE( (bsl80 << 63) ==
           emp::BitSet<80>("10001011100010111000000000000000000000000000000000000000000000000000000000000000")
         );
  REQUIRE( (bsl80 << 64) ==
           emp::BitSet<80>("00010111000101110000000000000000000000000000000000000000000000000000000000000000")
         );
  REQUIRE( (bsl80 << 65) ==
           emp::BitSet<80>("00101110001011100000000000000000000000000000000000000000000000000000000000000000")
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
  emp::BitSet<2> bs2;  // bs2 = 00
  bs2.flip(0);        // bs2 = 01
  REQUIRE(bs2[0]);

  emp::BitSet<8> bs8;  // bs8 = 00000000
  bs8.flip(0,4);      // bs8 = 00001111
  REQUIRE(bs8[0]);
  REQUIRE(bs8[1]);
  REQUIRE(bs8[2]);
  REQUIRE(bs8[3]);
  REQUIRE(!bs8[4]);

  bs8[0].Toggle();    // bs8 = 00001110
  REQUIRE(!bs8[0]);

  emp::BitSet<4> bs4;  // bs4 = 0000
  bs4.flip();          // bs4 = 1111
  REQUIRE(bs4.all());
}

/**
 * FindOne and PopOne
 */
void test_find(){
  emp::BitSet<10> bs10;  // bs10 = 00 00000000
  bs10.flip(3);          // bs10 = 00 00001000
  REQUIRE(bs10.FindOne() == 3);
  bs10.PopOne();        // bs10 = 00 00000000
  REQUIRE(bs10.PopOne() == -1);
  bs10.flip(3);
  bs10.flip(1);
  REQUIRE(bs10.FindOne(2) == 3);
  REQUIRE(bs10.FindOne(4) == -1);
}

/**
 * GetByte and SetByte
 */
void test_byte(){
  emp::BitSet<10>  bs10;
  bs10.SetByte(0, 10);
  REQUIRE(bs10.GetByte(0) == 10);

  bs10.flip(0,4);
  REQUIRE(bs10.GetByte(0) == 5);
  bs10.SetByte(1, 3);
  REQUIRE(bs10.count() == 4);
}

/**
 * GetBytes
 */
// actual testing function
template <size_t Bits>
void do_byte_test() {
  emp::BitSet<Bits> bs;

  for (size_t i = 0; i < Bits / 8; ++i) {
    bs.SetByte(i, 10 * i);
  }

  const auto myspan = bs.GetBytes();
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
  bs8.Set(0, 1);       // bs8 = 00000001
  REQUIRE(bs8.Get(0));

  bs8.Set(7, 1);      // bs8 = 10000001
  bs8.Set(0, 0);      // bs8 = 10000000
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
  bs4 ^= bs4_1;                 // bs4 = 0001 ^ 0011 = 0010
  REQUIRE(bs4.GetByte(0) == 2);   // 0010 = 2
  bs4_1.PopOne();               // bs4_1 = 0010
  bs4 ^= bs4_1;                 // bs4 = 0010 ^ 0010 = 0000
  REQUIRE(bs4.GetByte(0) == 0);  // 0000 = 0
}

/**
 * Bitwise OR |
 */
void test_bitwise_or(){
  emp::BitSet<10> bs10;
  emp::BitSet<10> bs10_1;
  bs10.Set(1,1);                 // bs10 = 00 0000 0010
  bs10_1.Set(3,1);
  bs10_1.SetByte(1,3);
  REQUIRE(bs10_1.count() == 3);  // bs10_1 = 11 00001000
  bs10_1 |= bs10;                // bs10_1 = 11 00001000 | 00 00000010 = 11 00001010
  REQUIRE(bs10_1.GetByte(0) == 10);
  REQUIRE(bs10_1.GetByte(1) == 3);
}

/**
 * Bitwise AND &
 */
void test_bitwise_and(){
  emp::BitSet<8> bs8;
  emp::BitSet<8> bs8_1;
  bs8.SetByte(0,13);    // bs8 = 00001101
  bs8_1.SetByte(0,10);  // bs8_1 = 00001010
  bs8_1 &= bs8;          // bs8_1 = 00001010 & 00001101 = 00001000
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
  emp::Random random(1);
  emp::BitSet<8> bs8(random);
  bs8.Randomize(random, 1.0);
  REQUIRE(bs8.all());
  bs8.Randomize(random, 0.0);
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



TEST_CASE("Test BitSet", "[bits]")
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


// For BitSet Import/Export
template <size_t SOURCE_BITS, size_t DEST_BITS>
struct ImportExportTester {

  static void test() {

    emp::Random rand(1);

    // using default parameter
    emp::BitSet<SOURCE_BITS> source(rand);
    emp::BitSet<DEST_BITS> dest(rand);

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

// for BitSet ROTATE_SELF
// adapted from spraetor.github.io/2015/12/26/compile-time-loops.html
// TODO: replace with https://en.cppreference.com/w/cpp/utility/integer_sequence
template <size_t N>
struct MultiTester2 {

  template <size_t I>
  static void test() {

    emp::Random rand(1);

    constexpr int W = N - 2;
    emp::BitSet<W> bs;

    for (int j = 0; j < W; ++j) {
      bs.Clear(); bs.Set(j);
      bs.template ROTL_SELF<I>();
      REQUIRE(bs.CountOnes() == 1);
      REQUIRE(bs.Get(emp::Mod(j+I,W)));

      bs.SetAll(); bs.Set(j, false);
      bs.template ROTL_SELF<I>();
      REQUIRE(bs.CountOnes() == W-1);
      REQUIRE(!bs.Get(emp::Mod(j+I,W)));

      bs.Randomize(rand); bs.Set(j);
      const size_t c1 = bs.CountOnes();
      bs.template ROTL_SELF<I>();
      REQUIRE(bs.CountOnes() == c1);
      REQUIRE(bs.Get(emp::Mod(j+I,W)));

      bs.Randomize(rand); bs.Set(j, false);
      const size_t c2 = bs.CountOnes();
      bs.template ROTL_SELF<I>();
      REQUIRE(bs.CountOnes() == c2);
      REQUIRE(!bs.Get(emp::Mod(j+I,W)));

      bs.Clear(); bs.Set(j);
      bs.template ROTR_SELF<I>();
      REQUIRE(bs.CountOnes() == 1);
      REQUIRE(bs.Get(emp::Mod(j-I,W)));

      bs.SetAll(); bs.Set(j, false);
      bs.template ROTR_SELF<I>();
      REQUIRE(bs.CountOnes() == W-1);
      REQUIRE(!bs.Get(emp::Mod(j-I,W)));

      bs.Randomize(rand); bs.Set(j);
      const size_t c3 = bs.CountOnes();
      bs.template ROTR_SELF<I>();
      REQUIRE(bs.CountOnes() == c3);
      REQUIRE(bs.Get(emp::Mod(j-I,W)));

      bs.Randomize(rand); bs.Set(j, false);
      const size_t c4 = bs.CountOnes();
      bs.template ROTR_SELF<I>();
      REQUIRE(bs.CountOnes() == c4);
      REQUIRE(!bs.Get(emp::Mod(j-I,W)));
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

    emp::Random rand(1);
    emp::BitSet<width> bs(rand);
    const emp::BitSet<width> bs_orig(bs);
    const size_t num_ones = bs.CountOnes();

    for (int i = -width - 1; i <= width + 1; ++i) {
      for (size_t rep = 0; rep < width; ++ rep) {
        bs.ROTATE_SELF(i);
        REQUIRE(bs.CountOnes() == num_ones);
      }
      REQUIRE(bs == bs_orig);
    }

    for (int i = -width - 1; i <= width + 1; ++i) {
      // for large widths, just do one starting position
      for (int j = 0; j < (width < 200 ? width : 1); ++j) {
        bs.Clear(); bs.Set(j);
        bs.ROTATE_SELF(i);
        REQUIRE(bs.CountOnes() == 1);
        REQUIRE(bs.Get(emp::Mod(j-i,width)));

        bs.SetAll(); bs.Set(j, false);
        bs.ROTATE_SELF(i);
        REQUIRE(bs.CountOnes() == width-1);
        REQUIRE(!bs.Get(emp::Mod(j-i,width)));

        bs.Randomize(rand); bs.Set(j);
        const size_t c1 = bs.CountOnes();
        bs.ROTATE_SELF(i);
        REQUIRE(bs.CountOnes() == c1);
        REQUIRE(bs.Get(emp::Mod(j-i,width)));

        bs.Randomize(rand); bs.Set(j, false);
        const size_t c2 = bs.CountOnes();
        bs.ROTATE_SELF(i);
        REQUIRE(bs.CountOnes() == c2);
        REQUIRE(!bs.Get(emp::Mod(j-i,width)));
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

TEST_CASE("Another Test BitSet", "[bits]")
{

  // test BitSet GetSize, GetNumBytes
  {
    REQUIRE(emp::BitSet<2>{}.GetSize() == 2);
    REQUIRE(emp::BitSet<2>{}.GetNumBytes() == 1);

    REQUIRE(emp::BitSet<7>{}.GetSize() == 7);
    REQUIRE(emp::BitSet<7>{}.GetNumBytes() == 1);

    REQUIRE(emp::BitSet<8>{}.GetSize() == 8);
    REQUIRE(emp::BitSet<8>{}.GetNumBytes() == 1);

    REQUIRE(emp::BitSet<9>{}.GetSize() == 9);
    REQUIRE(emp::BitSet<9>{}.GetNumBytes() == 2);

    REQUIRE(emp::BitSet<16>{}.GetSize() == 16);
    REQUIRE(emp::BitSet<16>{}.GetNumBytes() == 2);

    REQUIRE(emp::BitSet<24>{}.GetSize() == 24);
    REQUIRE(emp::BitSet<24>{}.GetNumBytes() == 3);
  }

  // test BitSet reverse
  {

    REQUIRE(emp::BitSet<1>{0}.REVERSE_SELF() == emp::BitSet<1>{0});
    REQUIRE(emp::BitSet<1>{0}.REVERSE_SELF().CountOnes() == 0);
    REQUIRE(emp::BitSet<1>{1}.REVERSE_SELF() == emp::BitSet<1>{1});
    REQUIRE(emp::BitSet<1>{1}.REVERSE_SELF().CountOnes() == 1);

    REQUIRE(
      (emp::BitSet<2>{1,1}.REVERSE_SELF())
      ==
      (emp::BitSet<2>{1,1})
    );
    REQUIRE((emp::BitSet<2>{1,1}.REVERSE_SELF().CountOnes()) == 2);
    REQUIRE(
      (emp::BitSet<2>{0,1}.REVERSE_SELF())
      ==
      (emp::BitSet<2>{1,0})
    );
    REQUIRE((emp::BitSet<2>{0,1}.REVERSE_SELF().CountOnes()) == 1);
    REQUIRE(
      (emp::BitSet<2>{0,0}.REVERSE_SELF())
      ==
      (emp::BitSet<2>{0,0})
    );
    REQUIRE((emp::BitSet<2>{0,0}.REVERSE_SELF().CountOnes()) == 0);

    REQUIRE(
      (emp::BitSet<7>{1,1,0,0,0,0,1}.REVERSE_SELF())
      ==
      (emp::BitSet<7>{1,0,0,0,0,1,1})
    );
    REQUIRE((emp::BitSet<7>{1,1,0,0,0,0,1}.REVERSE_SELF().CountOnes()) == 3);
    REQUIRE(
      (emp::BitSet<7>{1,0,1,0,1,0,1}.REVERSE_SELF())
      ==
      (emp::BitSet<7>{1,0,1,0,1,0,1})
    );
    REQUIRE((emp::BitSet<7>{1,0,1,0,1,0,1}.REVERSE_SELF().CountOnes()) == 4);
    REQUIRE(
      (emp::BitSet<7>{1,1,1,1,1,0,1}.REVERSE_SELF())
      ==
      (emp::BitSet<7>{1,0,1,1,1,1,1})
    );
    REQUIRE((emp::BitSet<7>{1,1,1,1,1,0,1}.REVERSE_SELF().CountOnes()) == 6);

    REQUIRE(
      (emp::BitSet<8>{1,1,0,0,0,0,1,0}.REVERSE_SELF())
      ==
      (emp::BitSet<8>{0,1,0,0,0,0,1,1})
    );
    REQUIRE((emp::BitSet<8>{1,1,0,0,0,0,1,0}.REVERSE_SELF().CountOnes()) == 3);
    REQUIRE(
      (emp::BitSet<8>{1,0,1,0,1,0,1,0}.REVERSE_SELF())
      ==
      (emp::BitSet<8>{0,1,0,1,0,1,0,1})
    );
    REQUIRE((emp::BitSet<8>{0,1,0,1,0,1,0,1}.REVERSE_SELF().CountOnes()) == 4);
    REQUIRE(
      (emp::BitSet<8>{1,1,1,1,1,0,1,0}.REVERSE_SELF())
      ==
      (emp::BitSet<8>{0,1,0,1,1,1,1,1})
    );
    REQUIRE((emp::BitSet<8>{1,1,1,1,1,0,1,0}.REVERSE_SELF().CountOnes()) == 6);

    REQUIRE(
      (emp::BitSet<9>{1,1,0,0,0,0,1,0,0}.REVERSE_SELF())
      ==
      (emp::BitSet<9>{0,0,1,0,0,0,0,1,1})
    );
    REQUIRE(
      (emp::BitSet<9>{1,1,0,0,0,0,1,0,0}.REVERSE_SELF().CountOnes())
      ==
      3
    );
    REQUIRE(
      (emp::BitSet<9>{1,0,1,0,1,0,1,0,0}.REVERSE_SELF())
      ==
      (emp::BitSet<9>{0,0,1,0,1,0,1,0,1})
    );
    REQUIRE(
      (emp::BitSet<9>{0,0,1,0,1,0,1,0,1}.REVERSE_SELF().CountOnes())
      ==
      4
    );
    REQUIRE(
      (emp::BitSet<9>{1,1,1,1,1,0,1,0,0}.REVERSE_SELF())
      ==
      (emp::BitSet<9>{0,0,1,0,1,1,1,1,1})
    );
    REQUIRE(
      (emp::BitSet<9>{1,1,1,1,1,0,1,0,0}.REVERSE_SELF().CountOnes())
      ==
      6
    );

    emp::Random rand(1);
    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<15> bs(rand);
      bs[0] = 0;
      bs[15-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<16> bs(rand);
      bs[0] = 0;
      bs[16-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<17> bs(rand);
      bs[0] = 0;
      bs[17-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<31> bs(rand);
      bs[0] = 0;
      bs[31-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<32> bs(rand);
      bs[0] = 0;
      bs[32-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<33> bs(rand);
      bs[0] = 0;
      bs[33-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<63> bs(rand);
      bs[0] = 0;
      bs[63-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<64> bs(rand);
      bs[0] = 0;
      bs[64-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<65> bs(rand);
      bs[0] = 0;
      bs[65-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<127> bs(rand);
      bs[0] = 0;
      bs[127-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<128> bs(rand);
      bs[0] = 0;
      bs[128-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<129> bs(rand);
      bs[0] = 0;
      bs[129-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

  }

  // test BitSet addition
  {
  emp::BitSet<32> bs0;
  bs0.SetUInt(0, std::numeric_limits<uint32_t>::max() - 1);
  emp::BitSet<32> bs1;
  bs1.SetUInt(0,1);
  bs0+=bs1;
  REQUIRE (bs0.GetUInt(0) == 4294967295);
  REQUIRE ((bs0+bs1).GetUInt(0) == 0);
  REQUIRE ((bs0+bs0).GetUInt(0) == 4294967294);

  emp::BitSet<8> bs2;
  bs2.SetUInt(0, emp::IntPow(2UL, 8UL)-1);
  emp::BitSet<8> bs3;
  bs3.SetUInt(0, 1);
  REQUIRE((bs2+bs3).GetUInt(0) == 0);
  emp::BitSet<64> bs4;
  bs4.SetUInt(0, std::numeric_limits<uint32_t>::max()-1);
  bs4.SetUInt(1, std::numeric_limits<uint32_t>::max());
  emp::BitSet<64> bs5;
  bs5.SetUInt(0, 1);
  bs4+=bs5;
  REQUIRE(bs4.GetUInt(0) == pow((size_t)2, (size_t)32)-1);
  REQUIRE(bs4.GetUInt(1) == pow((size_t)2, (size_t)32)-1);
  bs4+=bs5;
  REQUIRE(bs4.GetUInt(0) == 0);
  REQUIRE(bs4.GetUInt(1) == 0);
  }

  // test BitSet subtraction
  {
  emp::BitSet<32> bs0;
  bs0.SetUInt(0, 1);
  emp::BitSet<32> bs1;
  bs1.SetUInt(0, 1);
  bs0 = bs0 - bs1;
  REQUIRE (bs0.GetUInt(0) == 0);
  REQUIRE ((bs0-bs1).GetUInt(0) == std::numeric_limits<uint32_t>::max());

  emp::BitSet<8> bs2;
  bs2.SetUInt(0, 1);
  emp::BitSet<8> bs3;
  bs3.SetUInt(0, 1);

  bs2-=bs3;
  REQUIRE (bs2.GetUInt(0) == 0);
  REQUIRE((bs2-bs3).GetUInt(0) == emp::IntPow(2UL,8UL)-1);

  emp::BitSet<64> bs4;
  bs4.SetUInt(0, 1);
  bs4.SetUInt(1, 0);

  emp::BitSet<64> bs5;
  bs5.SetUInt(0, 1);

  bs4-=bs5;
  REQUIRE(bs4.GetUInt(0) == 0);
  REQUIRE(bs4.GetUInt(1) == 0);

  bs4-=bs5;
  REQUIRE(bs4.GetUInt(0) == std::numeric_limits<uint32_t>::max());
  REQUIRE(bs4.GetUInt(1) == std::numeric_limits<uint32_t>::max());
  bs4 = bs4 - bs5;
  REQUIRE(bs4.GetUInt(0) == std::numeric_limits<uint32_t>::max() - 1);
  REQUIRE(bs4.GetUInt(1) == std::numeric_limits<uint32_t>::max());
  }

  // test addition and subtraction with multiple fields
  {
  emp::BitSet<65> bs1;
  emp::BitSet<65> bs2;

  /* PART 1 */
  bs1.Clear();
  bs2.Clear();

  bs1.Set(64); // 10000...
  bs2.Set(0);  // ...00001

  for (size_t i = 0; i < 64; ++i) REQUIRE((bs1 - bs2).Get(i));
  REQUIRE(!(bs1 - bs2).Get(64));

  bs1 -= bs2;

  for (size_t i = 0; i < 64; ++i) {
    REQUIRE(bs1.Get(i));
  }
  REQUIRE(!bs1.Get(64));

  /* PART 2 */
  bs1.Clear();
  bs2.Clear();

  bs2.Set(0);  // ...00001

  for (size_t i = 0; i < 65; ++i) REQUIRE((bs1 - bs2).Get(i));

  bs1 -= bs2;

  for (size_t i = 0; i < 65; ++i) REQUIRE(bs1.Get(i));

  /* PART 3 */
  bs1.Clear();
  bs2.Clear();

  for (size_t i = 0; i < 65; ++i) bs1.Set(i); // 11111...11111
  bs2.Set(0);  // ...00001

  for (size_t i = 0; i < 65; ++i) REQUIRE(!(bs1 + bs2).Get(i));
  for (size_t i = 0; i < 65; ++i) REQUIRE(!(bs2 + bs1).Get(i));

  bs1 += bs2;

  for (size_t i = 0; i < 65; ++i) REQUIRE(!bs1.Get(i));

  /* PART 4 */
  bs1.Clear();
  bs2.Clear();

  for (size_t i = 0; i < 64; ++i) bs1.Set(i); // 01111...11111
  bs2.Set(0);  // ...00001

  for (size_t i = 0; i < 64; ++i) REQUIRE(!(bs1 + bs2).Get(i));
  REQUIRE((bs1 + bs2).Get(64));
  for (size_t i = 0; i < 64; ++i) REQUIRE(!(bs2 + bs1).Get(i));
  REQUIRE((bs2 + bs1).Get(64));

  bs1 += bs2;

  for (size_t i = 0; i < 64; ++i) REQUIRE(!bs1.Get(i));
  REQUIRE((bs2 + bs1).Get(64));
  }

  {
  emp::BitSet<3> bs0{0,0,0};
  REQUIRE(bs0.GetUInt8(0) == 0);
  REQUIRE(bs0.GetUInt16(0) == 0);
  REQUIRE(bs0.GetUInt32(0) == 0);
  REQUIRE(bs0.GetUInt64(0) == 0);
  REQUIRE(bs0.GetNumStates() == 8);

  emp::BitSet<3> bs1{0,0,1};
  REQUIRE(bs1.GetUInt8(0) == 1);
  REQUIRE(bs1.GetUInt16(0) == 1);
  REQUIRE(bs1.GetUInt32(0) == 1);
  REQUIRE(bs1.GetUInt64(0) == 1);

  emp::BitSet<3> bs2{0,1,1};
  REQUIRE(bs2.GetUInt8(0) == 3);
  REQUIRE(bs2.GetUInt16(0) == 3);
  REQUIRE(bs2.GetUInt32(0) == 3);
  REQUIRE(bs2.GetUInt64(0) == 3);

  emp::BitSet<3> bs3{1,1,1};
  REQUIRE(bs3.GetUInt8(0) == 7);

  emp::BitSet<3> bs4{1,1,0};
  REQUIRE(bs4.GetUInt8(0) == 6);

  emp::BitSet<32> bs5;
  bs5.SetUInt(0, 1789156UL);
  REQUIRE(bs5.GetUInt64(0) == 1789156ULL);
  REQUIRE(bs5.GetNumStates() == 4294967296ULL);

  emp::BitSet<63> bs6;
  bs6.SetUInt64(0, 789156816848ULL);
  REQUIRE(bs6.GetUInt64(0) == 789156816848ULL);
  REQUIRE(bs6.GetNumStates() == 9223372036854775808ULL);


  // @CAO: Removed GetDouble() due to confusing name (GetUInt64() gives the same answer, but with
  //       the correct encoding.
  // emp::BitSet<65> bs7;
  // bs7.SetUInt64(0, 1789156816848ULL);
  // bs7.Set(64);
  // REQUIRE(bs7.GetDouble() == 1789156816848.0 + emp::Pow2(64.0));
  // REQUIRE(bs7.MaxDouble() == 36893488147419103231.0);

  // emp::BitSet<1027> bs8;
  // bs8.Set(1026);
  // REQUIRE(std::isinf(bs8.GetDouble()));
  // REQUIRE(std::isinf(bs8.MaxDouble()));
  }

  // test list initializer
  {
  emp::BitSet<3> bs_empty{0,0,0};
  emp::BitSet<3> bs_first{1,0,0};
  emp::BitSet<3> bs_last{0,0,1};
  emp::BitSet<3> bs_full{1,1,1};

  REQUIRE(bs_empty.CountOnes() == 0);
  REQUIRE(bs_first.CountOnes() == 1);
  REQUIRE(bs_last.CountOnes() == 1);
  REQUIRE(bs_full.CountOnes() == 3);
  }

  // test Import and Export
  {

    emp::Random rand(1);

    emp::BitSet<32> orig(rand);

    emp::array<emp::BitSet<32>, 1> d1;
    emp::array<emp::BitSet<16>, 2> d2;
    emp::array<emp::BitSet<8>, 4> d4;
    emp::array<emp::BitSet<4>, 8> d8;
    emp::array<emp::BitSet<2>, 16> d16;
    emp::array<emp::BitSet<1>, 32> d32;

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

  emp::BitSet<10> bs10;
  emp::BitSet<25> bs25;
  emp::BitSet<32> bs32;
  emp::BitSet<50> bs50;
  emp::BitSet<64> bs64;
  emp::BitSet<80> bs80;

  bs80[70] = 1;
  bs80 <<= 1;
  emp::BitSet<80> bs80c(bs80);

  for (size_t i = 0; i < 75; i++) {
    emp::BitSet<80> shift_set = bs80 >> i;
    REQUIRE((shift_set.CountOnes() == 1) == (i <= 71));
  }

  bs80.Clear();

  REQUIRE(bs10[2] == false);
  bs10.flip(2);
  REQUIRE(bs10[2] == true);

  REQUIRE(bs32[2] == false);
  bs32.flip(2);
  REQUIRE(bs32[2] == true);

  REQUIRE(bs80[2] == false);
  bs80.flip(2);
  REQUIRE(bs80[2] == true);

  for (size_t i = 3; i < 8; i++) {REQUIRE(bs10[i] == false);}
  bs10.flip(3, 8);
  for (size_t i = 3; i < 8; i++) {REQUIRE(bs10[i] == true);}
  REQUIRE(bs10[8] == false);

  for (size_t i = 3; i < 8; i++) {REQUIRE(bs32[i] == false);}
  bs32.flip(3, 8);
  for (size_t i = 3; i < 8; i++) {REQUIRE(bs32[i] == true);}
  REQUIRE(bs32[8] == false);

  for (size_t i = 3; i < 8; i++) {REQUIRE(bs80[i] == false);}
  bs80.flip(3, 8);
  for (size_t i = 3; i < 8; i++) {REQUIRE(bs80[i] == true);}
  REQUIRE(bs80[8] == false);

  bs80[70] = 1;

  REQUIRE(bs10.GetUInt(0) == 252);
  REQUIRE(bs10.GetUInt32(0) == 252);
  REQUIRE(bs10.GetUInt64(0) == 252);

  REQUIRE(bs32.GetUInt(0) == 252);
  REQUIRE(bs32.GetUInt32(0) == 252);
  REQUIRE(bs32.GetUInt64(0) == 252);

  REQUIRE(bs80.GetUInt(0) == 252);
  REQUIRE(bs80.GetUInt(1) == 0);
  REQUIRE(bs80.GetUInt(2) == 64);
  REQUIRE(bs80.GetUInt32(0) == 252);
  REQUIRE(bs80.GetUInt32(1) == 0);
  REQUIRE(bs80.GetUInt32(2) == 64);
  REQUIRE(bs80.GetUInt64(0) == 252);
  REQUIRE(bs80.GetUInt64(1) == 64);

  bs80 = bs80c;

  // Test arbitrary bit retrieval of UInts
  bs80[65] = 1;
  REQUIRE(bs80.GetUInt32(2) == 130);
  REQUIRE(bs80.GetUInt32AtBit(64) == 130);
  REQUIRE(bs80.GetUInt8AtBit(64) == 130);

  emp::BitSet<96> bs;

  REQUIRE (bs.LongestSegmentOnes() == 0);
  bs.SetUInt(2, 1);
  REQUIRE (bs.LongestSegmentOnes() == 1);
  bs.SetUInt(1, 3);
  REQUIRE (bs.LongestSegmentOnes() == 2);
  bs.SetUInt(0, 7);
  REQUIRE (bs.LongestSegmentOnes() == 3);

  bs.SetUInt(0, std::numeric_limits<uint32_t>::max());
  bs.SetUInt(1, std::numeric_limits<uint32_t>::max() - 1);
  bs.SetUInt(2, std::numeric_limits<uint32_t>::max() - 3);
  REQUIRE (bs.LongestSegmentOnes() == 32);

  // tests for ROTATE
  // ... with one set bit
  bs10.Clear(); bs10.Set(0);
  bs25.Clear(); bs25.Set(0);
  bs32.Clear(); bs32.Set(0);
  bs50.Clear(); bs50.Set(0);
  bs64.Clear(); bs64.Set(0);
  bs80.Clear(); bs80.Set(0);

  for (int rot = -100; rot < 101; ++rot) {

    REQUIRE( bs10.CountOnes() == bs10.ROTATE(rot).CountOnes() );
    REQUIRE( bs25.CountOnes() == bs25.ROTATE(rot).CountOnes() );
    REQUIRE( bs32.CountOnes() == bs32.ROTATE(rot).CountOnes() );
    REQUIRE( bs50.CountOnes() == bs50.ROTATE(rot).CountOnes() );
    REQUIRE( bs64.CountOnes() == bs64.ROTATE(rot).CountOnes() );
    REQUIRE( bs80.CountOnes() == bs80.ROTATE(rot).CountOnes() );

    if (rot % 10) REQUIRE( bs10 != bs10.ROTATE(rot) );
    else REQUIRE( bs10 == bs10.ROTATE(rot) );

    if (rot % 25) REQUIRE( bs25 != bs25.ROTATE(rot) );
    else REQUIRE( bs25 == bs25.ROTATE(rot) );

    if (rot % 32) REQUIRE( bs32 != bs32.ROTATE(rot) );
    else REQUIRE( bs32 == bs32.ROTATE(rot) );

    if (rot % 50) REQUIRE( bs50 != bs50.ROTATE(rot) );
    else REQUIRE( bs50 == bs50.ROTATE(rot) );

    if (rot % 64) REQUIRE( bs64 != bs64.ROTATE(rot) );
    else REQUIRE( bs64 == bs64.ROTATE(rot) );

    if (rot % 80) REQUIRE( bs80 != bs80.ROTATE(rot) );
    else REQUIRE( bs80 == bs80.ROTATE(rot) );

  }

  // ... with random set bits
  emp::Random rand(1);
  // no bs10 because there's a reasonable chance
  // of breaking the test's assumption of nonsymmetry
  bs25.Randomize(rand);
  bs32.Randomize(rand);
  bs50.Randomize(rand);
  bs64.Randomize(rand);
  bs80.Randomize(rand);

  for (int rot = -100; rot < 101; ++rot) {

    REQUIRE( bs25.CountOnes() == bs25.ROTATE(rot).CountOnes() );
    REQUIRE( bs32.CountOnes() == bs32.ROTATE(rot).CountOnes() );
    REQUIRE( bs50.CountOnes() == bs50.ROTATE(rot).CountOnes() );
    REQUIRE( bs64.CountOnes() == bs64.ROTATE(rot).CountOnes() );
    REQUIRE( bs80.CountOnes() == bs80.ROTATE(rot).CountOnes() );

    if (rot % 25) REQUIRE( bs25 != bs25.ROTATE(rot) );
    else REQUIRE( bs25 == bs25.ROTATE(rot) );

    if (rot % 32) REQUIRE( bs32 != bs32.ROTATE(rot) );
    else REQUIRE( bs32 == bs32.ROTATE(rot) );

    if (rot % 50) REQUIRE( bs50 != bs50.ROTATE(rot) );
    else REQUIRE( bs50 == bs50.ROTATE(rot) );

    if (rot % 64) REQUIRE( bs64 != bs64.ROTATE(rot) );
    else REQUIRE( bs64 == bs64.ROTATE(rot) );

    if (rot % 80) REQUIRE( bs80 != bs80.ROTATE(rot) );
    else REQUIRE( bs80 == bs80.ROTATE(rot) );

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
    emp::BitSet<25> bs_25;
    emp::BitSet<32> bs_32;
    emp::BitSet<50> bs_50;
    emp::BitSet<64> bs_64;
    emp::BitSet<80> bs_80;

    bs_25.FlipRandomCount(random, 0);
    REQUIRE(!bs_25.CountOnes());

    bs_32.FlipRandomCount(random, 0);
    REQUIRE(!bs_32.CountOnes());

    bs_50.FlipRandomCount(random, 0);
    REQUIRE(!bs_50.CountOnes());

    bs_64.FlipRandomCount(random, 0);
    REQUIRE(!bs_64.CountOnes());

    bs_80.FlipRandomCount(random, 0);
    REQUIRE(!bs_80.CountOnes());


    bs_25.FlipRandomCount(random, 1);
    REQUIRE( bs_25.CountOnes() == 1);

    bs_32.FlipRandomCount(random, 1);
    REQUIRE( bs_32.CountOnes() == 1);

    bs_50.FlipRandomCount(random, 1);
    REQUIRE( bs_50.CountOnes() == 1);

    bs_64.FlipRandomCount(random, 1);
    REQUIRE( bs_64.CountOnes() == 1);

    bs_80.FlipRandomCount(random, 1);
    REQUIRE( bs_80.CountOnes() == 1);

    bs_25.Clear();
    bs_32.Clear();
    bs_50.Clear();
    bs_64.Clear();
    bs_80.Clear();

    for (size_t i = 1; i < 5000; ++i) {
      bs_25.FlipRandomCount(random, 1);
      REQUIRE(bs_25.CountOnes() <= i);

      bs_32.FlipRandomCount(random, 1);
      REQUIRE(bs_32.CountOnes() <= i);

      bs_50.FlipRandomCount(random, 1);
      REQUIRE(bs_50.CountOnes() <= i);

      bs_64.FlipRandomCount(random, 1);
      REQUIRE(bs_64.CountOnes() <= i);

      bs_80.FlipRandomCount(random, 1);
      REQUIRE(bs_80.CountOnes() <= i);
    }

    REQUIRE(bs_25.CountOnes() > bs_25.size()/4);
    REQUIRE(bs_25.CountOnes() < 3*bs_25.size()/4);
    REQUIRE(bs_32.CountOnes() > bs_32.size()/4);
    REQUIRE(bs_32.CountOnes() < 3*bs_32.size()/4);
    REQUIRE(bs_50.CountOnes() > bs_50.size()/4);
    REQUIRE(bs_50.CountOnes() < 3*bs_50.size()/4);
    REQUIRE(bs_64.CountOnes() > bs_64.size()/4);
    REQUIRE(bs_64.CountOnes() < 3*bs_64.size()/4);
    REQUIRE(bs_80.CountOnes() > bs_80.size()/4);
    REQUIRE(bs_80.CountOnes() < 3*bs_80.size()/4);

    for (size_t i = 0; i < 10; ++i) {
      bs_25.FlipRandomCount(random, bs_25.size());
      REQUIRE(bs_25.CountOnes() > bs_25.size()/4);
      REQUIRE(bs_25.CountOnes() < 3*bs_25.size()/4);

      bs_32.FlipRandomCount(random, bs_32.size());
      REQUIRE(bs_32.CountOnes() > bs_32.size()/4);
      REQUIRE(bs_32.CountOnes() < 3*bs_32.size()/4);

      bs_50.FlipRandomCount(random, bs_50.size());
      REQUIRE(bs_50.CountOnes() > bs_50.size()/4);
      REQUIRE(bs_50.CountOnes() < 3*bs_50.size()/4);

      bs_64.FlipRandomCount(random, bs_64.size());
      REQUIRE(bs_64.CountOnes() > bs_64.size()/4);
      REQUIRE(bs_64.CountOnes() < 3*bs_64.size()/4);

      bs_80.FlipRandomCount(random, bs_80.size());
      REQUIRE(bs_80.CountOnes() > bs_80.size()/4);
      REQUIRE(bs_80.CountOnes() < 3*bs_80.size()/4);
    }
  }

  // serialize / deserialize
  {

    // set up
    emp::Random rand(1);
    emp::BitSet<10> bs10(rand);
    emp::BitSet<25> bs25(rand);
    emp::BitSet<32> bs32(rand);
    emp::BitSet<50> bs50(rand);
    emp::BitSet<64> bs64(rand);
    emp::BitSet<80> bs80(rand);

    emp::BitSet<10> bs10_deser;
    emp::BitSet<25> bs25_deser;
    emp::BitSet<32> bs32_deser;
    emp::BitSet<50> bs50_deser;
    emp::BitSet<64> bs64_deser;
    emp::BitSet<80> bs80_deser;

    std::stringstream ss;

    {
      // Create an output archive
      cereal::BinaryOutputArchive oarchive(ss);

      // Write the data to the archive
      oarchive(
        bs10,
        bs25,
        bs32,
        bs50,
        bs64,
        bs80
      );

    } // archive goes out of scope, ensuring all contents are flushed

    {
      cereal::BinaryInputArchive iarchive(ss); // Create an input archive

       // Read the data from the archive
       iarchive(
        bs10_deser,
        bs25_deser,
        bs32_deser,
        bs50_deser,
        bs64_deser,
        bs80_deser
      );

    }

    REQUIRE(bs10 == bs10_deser);
    REQUIRE(bs25 == bs25_deser);
    REQUIRE(bs32 == bs32_deser);
    REQUIRE(bs50 == bs50_deser);
    REQUIRE(bs64 == bs64_deser);
    REQUIRE(bs80 == bs80_deser);

  }

  {

    // set up
    emp::Random rand(1);
    emp::BitSet<10> bs10(rand);
    emp::BitSet<25> bs25(rand);
    emp::BitSet<32> bs32(rand);
    emp::BitSet<50> bs50(rand);
    emp::BitSet<64> bs64(rand);
    emp::BitSet<80> bs80(rand);

    emp::BitSet<10> bs10_deser;
    emp::BitSet<25> bs25_deser;
    emp::BitSet<32> bs32_deser;
    emp::BitSet<50> bs50_deser;
    emp::BitSet<64> bs64_deser;
    emp::BitSet<80> bs80_deser;

    std::stringstream ss;

    {
      // Create an output archive
      cereal::JSONOutputArchive oarchive(ss);

      // Write the data to the archive
      oarchive(
        bs10,
        bs25,
        bs32,
        bs50,
        bs64,
        bs80
      );

    } // archive goes out of scope, ensuring all contents are flushed

    {
      cereal::JSONInputArchive iarchive(ss); // Create an input archive

       // Read the data from the archive
       iarchive(
        bs10_deser,
        bs25_deser,
        bs32_deser,
        bs50_deser,
        bs64_deser,
        bs80_deser
      );

    }

    REQUIRE(bs10 == bs10_deser);
    REQUIRE(bs25 == bs25_deser);
    REQUIRE(bs32 == bs32_deser);
    REQUIRE(bs50 == bs50_deser);
    REQUIRE(bs64 == bs64_deser);
    REQUIRE(bs80 == bs80_deser);

  }

}


TEST_CASE("Test BitSet string construction", "[tools]") {

  REQUIRE( emp::BitSet<5>( "01001" ) == emp::BitSet<5>{0, 1, 0, 0, 1} );

  // std::bitset treats bits in the opposite direction of emp::BitSet.
  REQUIRE(
    emp::BitSet<5>( std::bitset<5>( "10010" ) )
    == emp::BitSet<5>{1, 0, 0, 1, 0}
  );

}
