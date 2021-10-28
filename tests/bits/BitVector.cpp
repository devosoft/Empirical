/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file BitVector.cpp
 */

#include <limits>
#include <map>
#include <ratio>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/base/vector.hpp"
#include "emp/bits/BitVector.hpp"
#include "emp/math/Random.hpp"

TEST_CASE("1: Test BitVector Constructors", "[bits]"){
  // Create a size 50 bit vector, default to all zeros.
  emp::BitVector bv1(50);
  REQUIRE( bv1.GetSize() == 50 );
  REQUIRE( bv1.CountOnes() == 0 );
  REQUIRE( (~bv1).CountOnes() == 50 );

  // Create a size 1000 BitVector, default to all ones.
  emp::BitVector bv2(1000, true);
  REQUIRE( bv2.GetSize() == 1000 );
  REQUIRE( bv2.CountOnes() == 1000 );

  // Try a range of BitVector sizes, from 0 to 200.
  for (size_t bv_size = 0; bv_size <= 200; bv_size++) {
    emp::BitVector bv3(bv_size);
    REQUIRE( bv3.GetSize() == bv_size );
    REQUIRE( bv3.CountOnes() == 0 );
    for (size_t i = 0; i < bv_size; i++) bv3[i] = true;
    REQUIRE( bv3.CountOnes() == bv_size );
  }

  // Build a relatively large BitVector.
  emp::BitVector bv4(1000000);
  for (size_t i = 0; i < bv4.GetSize(); i += 100) bv4[i].Toggle();
  REQUIRE( bv4.CountOnes() == 10000 );

  // Try out the copy constructor.
  emp::BitVector bv5(bv4);
  REQUIRE( bv5.GetSize() == 1000000 );
  REQUIRE( bv5.CountOnes() == 10000 );

  // And the move constructor.
  auto old_ptr = bv5.RawBytes();         // Grab a pointer to where bv5 currently has its bytes.
  emp::BitVector bv6( std::move(bv5) );  // Move bv5 bytes into bv6.
  REQUIRE( bv6.RawBytes() == old_ptr );
  REQUIRE( bv5.RawBytes() == nullptr );

  // Construct from std::bitset.
  std::bitset<6> bit_set;
  bit_set[1] = 1;   bit_set[2] = 1;   bit_set[4] = 1;
  emp::BitVector bv7(bit_set);
  REQUIRE( bv7.GetSize() == 6 );
  REQUIRE( bv7.CountOnes() == 3 );

  // Construct from string.
  std::string bit_string = "10011001010000011101";
  emp::BitVector bv8(bit_string);
  REQUIRE( bv8.GetSize() == 20 );
  REQUIRE( bv8.CountOnes() == 9 );

  // Some random BitVectors
  emp::Random random(1);
  emp::BitVector bv9(1000, random);            // 50/50 chance for each bit.
  const size_t bv9_ones = bv9.CountOnes();
  REQUIRE( bv9_ones >= 400 );
  REQUIRE( bv9_ones <= 600 );

  emp::BitVector bv10(1000, random, 0.8);      // 80% chance of ones.
  const size_t bv10_ones = bv10.CountOnes();
  REQUIRE( bv10_ones >= 750 );
  REQUIRE( bv10_ones <= 850 );

  emp::BitVector bv11(1000, random, 117);      // Exactly 117 ones, randomly placed.
  const size_t bv11_ones = bv11.CountOnes();
  REQUIRE( bv11_ones == 117 );

  emp::BitVector bv12(bv11, 500);              // Construct with just first half of bv11.
  const size_t bv12_ones = bv12.CountOnes();
  REQUIRE( bv12_ones >= 20 );
  REQUIRE( bv12_ones <= 90 );

  emp::BitVector bv13({1,0,0,0,1,1,1,0,0,0,1,1,1}); // Construct with initializer list.
  REQUIRE( bv13.GetSize() == 13 );
  REQUIRE( bv13.CountOnes() == 7 );
}

TEST_CASE("2: Test BitVector Assignemnts", "[bits]"){
  emp::vector< emp::BitVector > v;

  // Try all BitVector sizes from 0 to 128.
  // Lot's of move operators will trigger as vector grows.
  for (size_t i = 0; i <= 128; i++) {
    v.emplace_back(i);
  }

  // And a few larger BitVectors...
  v.emplace_back(1023);
  v.emplace_back(1024);
  v.emplace_back(1025);
  v.emplace_back(1000000);

  // Copy each BitVector into bv2 and do some manipulations then copy back.
  for (emp::BitVector & bv : v) {
    emp::BitVector bv2 = bv;
    for (size_t i = 1; i < bv2.GetSize(); i += 2) {
      bv2[i] = 1;
    }
    bv = bv2;
  }

  // Now make sure the we constructed bits correctly!
  for (const emp::BitVector & bv : v) {
    REQUIRE( bv.CountOnes() == bv.GetSize()/2 );
  }

  emp::vector< emp::BitVector > v2;
  v2.push_back( emp::BitVector({0,1,0,1,0,1}) );

  v2 = v; // Copy over all BitVectors.

  std::bitset<600> bit_set;
  bit_set[1] = 1;   bit_set[22] = 1;   bit_set[444] = 1;

  v[10] = bit_set;  // Copy in an std::bitset.

  REQUIRE( v[10].GetSize() == 600 );
  REQUIRE( v[10].CountOnes() == 3 );

  std::string bit_string = "100110010100000111011001100101000001110110011001010000011101";

  v[75] = bit_string;

  REQUIRE( v[75].GetSize() == 60 );
  REQUIRE( v[75].CountOnes() == 27 );

}


TEST_CASE("3: Test Simple BitVector Accessors", "[bits]"){
  emp::BitVector bv0(0);
  emp::BitVector bv1(1, true);
  emp::BitVector bv8( "10001101" );
  emp::BitVector bv32( "10001101100011011000110110001101" );
  emp::BitVector bv64( "1000110110001101100000011000110000001101100000000000110110001101" );
  emp::BitVector bv75( "010001011100010111110000011110100011111000001110100000111110010011111000011" );

  emp::Random random(1);
  emp::BitVector bv1k(1000, random, 0.75);

  // Make sure all sizes are correct.
  REQUIRE( bv0.GetSize() == 0 );
  REQUIRE( bv1.GetSize() == 1 );
  REQUIRE( bv8.GetSize() == 8 );
  REQUIRE( bv32.GetSize() == 32 );
  REQUIRE( bv64.GetSize() == 64 );
  REQUIRE( bv75.GetSize() == 75 );
  REQUIRE( bv1k.GetSize() == 1000 );

  // Check byte counts (should always round up!)
  REQUIRE( bv0.GetNumBytes() == 0 );
  REQUIRE( bv1.GetNumBytes() == 1 );     // round up!
  REQUIRE( bv8.GetNumBytes() == 1 );
  REQUIRE( bv32.GetNumBytes() == 4 );
  REQUIRE( bv64.GetNumBytes() == 8 );
  REQUIRE( bv75.GetNumBytes() == 10 );   // round up!
  REQUIRE( bv1k.GetNumBytes() == 125 );

  // How many states can be represented in each size of BitVector?
  REQUIRE( bv0.GetNumStates() == 1.0 );
  REQUIRE( bv1.GetNumStates() == 2.0 );
  REQUIRE( bv8.GetNumStates() == 256.0 );
  REQUIRE( bv32.GetNumStates() == 4294967296.0 );
  REQUIRE( bv64.GetNumStates() >= 18446744073709551610.0 );
  REQUIRE( bv64.GetNumStates() <= 18446744073709551720.0 );
  REQUIRE( bv75.GetNumStates() >= 37778931862957161709560.0 );
  REQUIRE( bv75.GetNumStates() <= 37778931862957161709570.0 );
  REQUIRE( bv1k.GetNumStates() == emp::Pow2(1000) );

  // Test Get()
  REQUIRE( bv1.Get(0) == 1 );
  REQUIRE( bv8.Get(0) == 1 );
  REQUIRE( bv8.Get(4) == 1 );
  REQUIRE( bv8.Get(6) == 0 );
  REQUIRE( bv8.Get(7) == 1 );
  REQUIRE( bv75.Get(0) == 0 );
  REQUIRE( bv75.Get(1) == 1 );
  REQUIRE( bv75.Get(72) == 0 );
  REQUIRE( bv75.Get(73) == 1 );
  REQUIRE( bv75.Get(74) == 1 );

  // Test Has() (including out of range)
  REQUIRE( bv0.Has(0) == false );
  REQUIRE( bv0.Has(1000000) == false );

  REQUIRE( bv1.Has(0) == true );
  REQUIRE( bv1.Has(1) == false );

  REQUIRE( bv8.Has(0) == true );
  REQUIRE( bv8.Has(4) == true );
  REQUIRE( bv8.Has(6) == false );
  REQUIRE( bv8.Has(7) == true );
  REQUIRE( bv8.Has(8) == false );

  REQUIRE( bv75.Has(0) == false );
  REQUIRE( bv75.Has(1) == true );
  REQUIRE( bv75.Has(72) == false );
  REQUIRE( bv75.Has(73) == true );
  REQUIRE( bv75.Has(74) == true );
  REQUIRE( bv75.Has(75) == false );
  REQUIRE( bv75.Has(79) == false );
  REQUIRE( bv75.Has(1000000) == false );

  // Test Set(), changing in most (but not all) cases.
  bv1.Set(0, 0);
  REQUIRE( bv1.Get(0) == 0 );
  bv8.Set(0, 1);                // Already a 1!
  REQUIRE( bv8.Get(0) == 1 );
  bv8.Set(4, 0);
  REQUIRE( bv8.Get(4) == 0 );
  bv8.Set(6, 1);
  REQUIRE( bv8.Get(6) == 1 );
  bv8.Set(7, 0);
  REQUIRE( bv8.Get(7) == 0 );
  bv75.Set(0, 0);               // Already a 0!
  REQUIRE( bv75.Get(0) == 0 );
  bv75.Set(1, 0);
  REQUIRE( bv75.Get(1) == 0 );
  bv75.Set(72);                 // No second arg!
  REQUIRE( bv75.Get(72) == 1 );
  bv75.Set(73);                 // No second arg AND already a 1!
  REQUIRE( bv75.Get(73) == 1 );
  bv75.Set(74, 0);
  REQUIRE( bv75.Get(74) == 0 );
}

TEST_CASE("4: Test BitVector Set*, Clear* and Toggle* Accessors", "[bits]") {
  // Make sure range-based accessors still work when there are no bits.
  emp::BitVector bv0(0);
  bv0.SetRange(0,0);
  bv0.SetAll();
  bv0.Clear();
  bv0.Clear(0,0);
  bv0.Toggle();
  bv0.Toggle(0,0);
  REQUIRE( bv0.GetSize() == 0 );

  // Now try range-based accessors on a single bit.
  emp::BitVector bv1(1, false);  REQUIRE( bv1[0] == false );   REQUIRE( bv1.CountOnes() == 0 );
  bv1.Set(0);                    REQUIRE( bv1[0] == true );    REQUIRE( bv1.CountOnes() == 1 );
  bv1.Clear(0);                  REQUIRE( bv1[0] == false );   REQUIRE( bv1.CountOnes() == 0 );
  bv1.Toggle(0);                 REQUIRE( bv1[0] == true );    REQUIRE( bv1.CountOnes() == 1 );
  bv1.Clear();                   REQUIRE( bv1[0] == false );   REQUIRE( bv1.CountOnes() == 0 );
  bv1.SetAll();                  REQUIRE( bv1[0] == true );    REQUIRE( bv1.CountOnes() == 1 );
  bv1.Toggle();                  REQUIRE( bv1[0] == false );   REQUIRE( bv1.CountOnes() == 0 );
  bv1.SetRange(0,1);             REQUIRE( bv1[0] == true );    REQUIRE( bv1.CountOnes() == 1 );
  bv1.Clear(0,1);                REQUIRE( bv1[0] == false );   REQUIRE( bv1.CountOnes() == 0 );
  bv1.Toggle(0,1);               REQUIRE( bv1[0] == true );    REQUIRE( bv1.CountOnes() == 1 );
  bv1.Set(0, false);             REQUIRE( bv1[0] == false );   REQUIRE( bv1.CountOnes() == 0 );
  bv1.SetRange(0,0);             REQUIRE( bv1[0] == false );   REQUIRE( bv1.CountOnes() == 0 );
  bv1.SetRange(1,1);             REQUIRE( bv1[0] == false );   REQUIRE( bv1.CountOnes() == 0 );

  // Test when a full byte is used.
  emp::BitVector bv8( "10001101" );   REQUIRE(bv8.GetValue() == 177.0);  // 10110001
  bv8.Set(2);                         REQUIRE(bv8.GetValue() == 181.0);  // 10110101
  bv8.Set(0, 0);                      REQUIRE(bv8.GetValue() == 180.0);  // 10110100
  bv8.SetRange(1, 4);                 REQUIRE(bv8.GetValue() == 190.0);  // 10111110
  bv8.SetAll();                       REQUIRE(bv8.GetValue() == 255.0);  // 11111111
  bv8.Clear(3);                       REQUIRE(bv8.GetValue() == 247.0);  // 11110111
  bv8.Clear(5,5);                     REQUIRE(bv8.GetValue() == 247.0);  // 11110111
  bv8.Clear(5,7);                     REQUIRE(bv8.GetValue() == 151.0);  // 10010111
  bv8.Clear();                        REQUIRE(bv8.GetValue() ==   0.0);  // 00000000
  bv8.Toggle(4);                      REQUIRE(bv8.GetValue() ==  16.0);  // 00010000
  bv8.Toggle(4,6);                    REQUIRE(bv8.GetValue() ==  32.0);  // 00100000
  bv8.Toggle(0,3);                    REQUIRE(bv8.GetValue() ==  39.0);  // 00100111
  bv8.Toggle(7,8);                    REQUIRE(bv8.GetValue() == 167.0);  // 10100111
  bv8.Toggle();                       REQUIRE(bv8.GetValue() ==  88.0);  // 01011000

  // Test a full field.
  constexpr double ALL_64 = (double) ((uint64_t) -1);
  emp::BitVector bv64( "11011000110110001101" ); REQUIRE(bv64.GetValue() == 727835.0);
  bv64.Resize(64);      REQUIRE(bv64.GetValue() == 727835.0);        // ...0 010110001101100011011
  bv64.Set(6);          REQUIRE(bv64.GetValue() == 727899.0);        // ...0 010110001101101011011
  bv64.Set(0, 0);       REQUIRE(bv64.GetValue() == 727898.0);        // ...0 010110001101101011010
  bv64.SetRange(4, 9);  REQUIRE(bv64.GetValue() == 728058.0);        // ...0 010110001101111111010
  bv64.SetAll();        REQUIRE(bv64.GetValue() == ALL_64);          // ...1 111111111111111111111
  bv64.Clear(2);        REQUIRE(bv64.GetValue() == ALL_64 - 4);      // ...1 111111111111111111011
  bv64.Clear(5,5);      REQUIRE(bv64.GetValue() == ALL_64 - 4);      // ...1 111111111111111111011
  bv64.Clear(5,7);      REQUIRE(bv64.GetValue() == ALL_64 - 100);    // ...1 111111111111110011011
  bv64.Clear();         REQUIRE(bv64.GetValue() == 0.0);             // ...0 000000000000000000000
  bv64.Toggle(19);      REQUIRE(bv64.GetValue() == emp::Pow2(19));   // ...0 010000000000000000000
  bv64.Toggle(15,20);   REQUIRE(bv64.GetValue() == 491520.0);        // ...0 001111000000000000000
  bv64.Toggle();        REQUIRE(bv64.GetValue() == ALL_64-491520.0); // ...1 110000111111111111111
  bv64.Toggle(0,64);    REQUIRE(bv64.GetValue() == 491520.0);        // ...0 001111000000000000000


  emp::BitVector bv75( "010001011100010111110000011110100011111000001110100000111110010011111000011" );

  // Test a full + partial field.
  constexpr double ALL_88 = ((double) ((uint64_t) -1)) * emp::Pow2(24);
  emp::BitVector bv88( "11011000110110001101" ); REQUIRE(bv88.GetValue() == 727835.0);
  bv88.Resize(88);      REQUIRE(bv88.GetValue() == 727835.0);        // ...0 010110001101100011011

  // Start with same tests as last time...
  bv88.Set(6);          REQUIRE(bv88.GetValue() == 727899.0);        // ...0 010110001101101011011
  bv88.Set(0, 0);       REQUIRE(bv88.GetValue() == 727898.0);        // ...0 010110001101101011010
  bv88.SetRange(4, 9);  REQUIRE(bv88.GetValue() == 728058.0);        // ...0 010110001101111111010
  bv88.SetAll();        REQUIRE(bv88.GetValue() == ALL_88);          // ...1 111111111111111111111
  bv88.Clear(2);        REQUIRE(bv88.GetValue() == ALL_88 - 4);      // ...1 111111111111111111011
  bv88.Clear(5,5);      REQUIRE(bv88.GetValue() == ALL_88 - 4);      // ...1 111111111111111111011
  bv88.Clear(5,7);      REQUIRE(bv88.GetValue() == ALL_88 - 100);    // ...1 111111111111110011011
  bv88.Clear();         REQUIRE(bv88.GetValue() == 0.0);             // ...0 000000000000000000000
  bv88.Toggle(19);      REQUIRE(bv88.GetValue() == emp::Pow2(19));   // ...0 010000000000000000000
  bv88.Toggle(15,20);   REQUIRE(bv88.GetValue() == 491520.0);        // ...0 001111000000000000000
  bv88.Toggle();        REQUIRE(bv88.GetValue() == ALL_88-491520.0); // ...1 110000111111111111111
  bv88.Toggle(0,88);    REQUIRE(bv88.GetValue() == 491520.0);        // ...0 001111000000000000000

  bv88 <<= 20;          REQUIRE(bv88.CountOnes() == 4);   // four ones, moved to bits 35-39
  bv88 <<= 27;          REQUIRE(bv88.CountOnes() == 4);   // four ones, moved to bits 62-65
  bv88 <<= 22;          REQUIRE(bv88.CountOnes() == 4);   // four ones, moved to bits 84-87
  bv88 <<= 1;           REQUIRE(bv88.CountOnes() == 3);   // three ones left, moved to bits 85-87
  bv88 <<= 2;           REQUIRE(bv88.CountOnes() == 1);   // one one left, at bit 87
  bv88 >>= 30;          REQUIRE(bv88.CountOnes() == 1);   // one one left, now at bit 57
  bv88.Toggle(50,80);   REQUIRE(bv88.CountOnes() == 29);  // Toggling 30 bits, only one was on.
  bv88.Clear(52,78);    REQUIRE(bv88.CountOnes() == 4);   // Leave two 1s on each side of range
  bv88.SetRange(64,66); REQUIRE(bv88.CountOnes() == 6);   // Set two more 1s, just into 2nd field.

  // A larger BitVector with lots of random tests.
  emp::Random random(1);
  emp::BitVector bv1k(1000, random, 0.65);
  size_t num_ones = bv1k.CountOnes();  REQUIRE(num_ones > 550);
  bv1k.Toggle();                       REQUIRE(bv1k.CountOnes() == 1000 - num_ones);

  for (size_t test_id = 0; test_id < 10000; ++test_id) {
    size_t val1 = random.GetUInt(1000);
    size_t val2 = random.GetUInt(1001);
    if (val1 > val2) std::swap(val1, val2);
    bv1k.Toggle(val1, val2);

    val1 = random.GetUInt(1000);
    val2 = random.GetUInt(1001);
    if (val1 > val2) std::swap(val1, val2);
    bv1k.Clear(val1, val2);

    val1 = random.GetUInt(1000);
    val2 = random.GetUInt(1001);
    if (val1 > val2) std::swap(val1, val2);
    bv1k.SetRange(val1, val2);
  }

  // Test Any(), All() and None()
  emp::BitVector bv_empty("000000");
  emp::BitVector bv_mixed("010101");
  emp::BitVector bv_full ("111111");

  REQUIRE(bv_empty.Any() == false);
  REQUIRE(bv_mixed.Any() == true);
  REQUIRE(bv_full.Any() == true);

  REQUIRE(bv_empty.All() == false);
  REQUIRE(bv_mixed.All() == false);
  REQUIRE(bv_full.All() == true);

  REQUIRE(bv_empty.None() == true);
  REQUIRE(bv_mixed.None() == false);
  REQUIRE(bv_full.None() == false);
}

TEST_CASE("5: Test Randomize() and variants", "[bits]") {
  emp::Random random(1);
  emp::BitVector bv(1000);

  REQUIRE(bv.None() == true);

  // Do all of the random tests 10 times.
  for (size_t test_num = 0; test_num < 10; test_num++) {
    bv.Randomize(random);
    size_t num_ones = bv.CountOnes();
    REQUIRE(num_ones > 300);
    REQUIRE(num_ones < 700);

    // 85% Chance of 1
    bv.Randomize(random, 0.85);
    num_ones = bv.CountOnes();
    REQUIRE(num_ones > 700);
    REQUIRE(num_ones < 950);

    // 15% Chance of 1
    bv.Randomize(random, 0.15);
    num_ones = bv.CountOnes();
    REQUIRE(num_ones > 50);
    REQUIRE(num_ones < 300);

    // Try randomizing only a portion of the genome.
    uint64_t first_bits = bv.GetUInt64(0);
    bv.Randomize(random, 0.7, 64, 1000);

    REQUIRE(bv.GetUInt64(0) == first_bits);  // Make sure first bits haven't changed

    num_ones = bv.CountOnes();
    REQUIRE(num_ones > 500);                 // Expected with new randomization is ~665 ones.
    REQUIRE(num_ones < 850);

    // Try randomizing using specific numbers of ones.
    bv.ChooseRandom(random, 1);       REQUIRE(bv.CountOnes() == 1);
    bv.ChooseRandom(random, 12);      REQUIRE(bv.CountOnes() == 12);
    bv.ChooseRandom(random, 128);     REQUIRE(bv.CountOnes() == 128);
    bv.ChooseRandom(random, 507);     REQUIRE(bv.CountOnes() == 507);
    bv.ChooseRandom(random, 999);     REQUIRE(bv.CountOnes() == 999);

    // Test the probabilistic CHANGE functions.
    bv.Clear();                     REQUIRE(bv.CountOnes() == 0);   // Set all bits to 0.

    bv.FlipRandom(random, 0.3);     // Exprected: 300 ones (from flipping zeros)
    num_ones = bv.CountOnes();      REQUIRE(num_ones > 230);  REQUIRE(num_ones < 375);

    bv.FlipRandom(random, 0.3);     // Exprected: 420 ones (hit by ONE but not both flips)
    num_ones = bv.CountOnes();      REQUIRE(num_ones > 345);  REQUIRE(num_ones < 495);

    bv.SetRandom(random, 0.5);      // Expected: 710 (already on OR newly turned on)
    num_ones = bv.CountOnes();      REQUIRE(num_ones > 625);  REQUIRE(num_ones < 775);

    bv.SetRandom(random, 0.8);      // Expected: 942 (already on OR newly turned on)
    num_ones = bv.CountOnes();      REQUIRE(num_ones > 900);  REQUIRE(num_ones < 980);

    bv.ClearRandom(random, 0.2);    // Expected 753.6 (20% of those on now off)
    num_ones = bv.CountOnes();      REQUIRE(num_ones > 675);  REQUIRE(num_ones < 825);

    bv.FlipRandom(random, 0.5);     // Exprected: 500 ones (each bit has a 50% chance of flipping)
    num_ones = bv.CountOnes();      REQUIRE(num_ones > 425);  REQUIRE(num_ones < 575);


    // Repeat with fixed-sized changes.
    bv.Clear();                        REQUIRE(bv.CountOnes() == 0);     // Set all bits to 0.

    bv.FlipRandomCount(random, 123);   // Flip exactly 123 bits to 1.
    num_ones = bv.CountOnes();         REQUIRE(num_ones == 123);

    bv.FlipRandomCount(random, 877);   // Flip exactly 877 bits; Expected 784.258 ones
    num_ones = bv.CountOnes();         REQUIRE(num_ones > 700);  REQUIRE(num_ones < 850);


    bv.SetAll();                       REQUIRE(bv.CountOnes() == 1000);  // Set all bits to 1.

    bv.ClearRandomCount(random, 123);
    num_ones = bv.CountOnes();         REQUIRE(num_ones == 877);

    bv.ClearRandomCount(random, 877);  // Clear exactly 877 bits; Expected 107.871 ones
    num_ones = bv.CountOnes();         REQUIRE(num_ones > 60);  REQUIRE(num_ones < 175);

    bv.SetRandomCount(random, 500);    // Half of the remaining ones should be set; 553.9355 expected.
    num_ones = bv.CountOnes();         REQUIRE(num_ones > 485);  REQUIRE(num_ones < 630);


    bv.Clear();                        REQUIRE(bv.CountOnes() == 0);     // Set all bits to 0.
    bv.SetRandomCount(random, 567);    // Half of the remaining ones should be set; 607.871 expected.
    num_ones = bv.CountOnes();         REQUIRE(num_ones == 567);
  }


  // During randomization, make sure each bit position is set appropriately.
  std::vector<size_t> one_counts(1000, 0);

  for (size_t test_num = 0; test_num < 1000; ++test_num) {
    // Set bits with different probabilities in different ranges.
    bv.Clear();
    bv.Randomize(random, 0.5,  100, 250);
    bv.Randomize(random, 0.25, 250, 400);
    bv.Randomize(random, 0.75, 400, 550);
    bv.Randomize(random, 0.10, 550, 700);
    bv.Randomize(random, 0.98, 700, 850);

    // Keep count of how many times each position was a one.
    for (size_t i = 0; i < bv.GetSize(); ++i) {
      if (bv.Get(i)) one_counts[i]++;
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

  emp::BitVector bv(num_bits);
  REQUIRE(bv.GetSize() == num_bits);
  REQUIRE(bv.GetNumBytes() == num_bytes);

  // All bytes should start out empty.
  for (size_t i = 0; i < num_bytes; i++) REQUIRE(bv.GetByte(i) == 0);

  bv.SetByte(2, 11);
  REQUIRE(bv.GetByte(2) == 11);

  REQUIRE(bv.GetValue() == 720896.0);

  bv.SetByte(5, 7);
  REQUIRE(bv.GetByte(0) == 0);
  REQUIRE(bv.GetByte(1) == 0);
  REQUIRE(bv.GetByte(2) == 11);
  REQUIRE(bv.GetByte(3) == 0);
  REQUIRE(bv.GetByte(4) == 0);
  REQUIRE(bv.GetByte(5) == 7);
  REQUIRE(bv.GetByte(6) == 0);
  REQUIRE(bv.CountOnes() == 6);

  for (size_t i = 0; i < num_bytes; i++) REQUIRE(bv.GetByte(i) == bv.GetUInt8(i));

  REQUIRE(bv.GetUInt16(0) == 0);
  REQUIRE(bv.GetUInt16(1) == 11);
  REQUIRE(bv.GetUInt16(2) == 1792);
  REQUIRE(bv.GetUInt16(3) == 0);

  REQUIRE(bv.GetUInt32(0) == 720896);
  REQUIRE(bv.GetUInt32(1) == 1792);
  REQUIRE(bv.GetUInt32(2) == 0);

  REQUIRE(bv.GetUInt64(0) == 7696582115328);
  REQUIRE(bv.GetUInt64(1) == 0);

  bv.SetUInt64(0, 12345678901234);
  bv.SetUInt32(2, 2000000);
  bv.SetUInt16(7, 7777);
  bv.SetUInt8(17, 17);

  REQUIRE(bv.GetUInt64(0) == 12345678901234);
  REQUIRE(bv.GetUInt32(2) == 2000000);
  REQUIRE(bv.GetUInt16(7) == 7777);
  REQUIRE(bv.GetUInt8(17) == 17);

  bv.Clear();
  bv.SetUInt16AtBit(40, 40);

  REQUIRE(bv.GetUInt16AtBit(40) == 40);

  REQUIRE(bv.GetUInt8(5) == 40);
  REQUIRE(bv.GetUInt8AtBit(40) == 40);
  REQUIRE(bv.GetUInt32AtBit(40) == 40);
  REQUIRE(bv.GetUInt64AtBit(40) == 40);

  REQUIRE(bv.GetUInt16AtBit(38) == 160);
  REQUIRE(bv.GetUInt16AtBit(39) == 80);
  REQUIRE(bv.GetUInt16AtBit(41) == 20);
  REQUIRE(bv.GetUInt16AtBit(42) == 10);

  REQUIRE(bv.GetUInt8AtBit(38) == 160);
  REQUIRE(bv.GetUInt8AtBit(37) == 64);
  REQUIRE(bv.GetUInt8AtBit(36) == 128);
  REQUIRE(bv.GetUInt8AtBit(35) == 0);
}

TEST_CASE("7: Test functions that analyze and manipulate ones", "[bits]") {

  emp::BitVector bv = "0001000100001110";

  REQUIRE(bv.GetSize() == 16);
  REQUIRE(bv.CountOnes() == 5);

  // Make sure we can find all of the ones.
  REQUIRE(bv.FindOne() == 3);
  REQUIRE(bv.FindOne(4) == 7);
  REQUIRE(bv.FindOne(5) == 7);
  REQUIRE(bv.FindOne(6) == 7);
  REQUIRE(bv.FindOne(7) == 7);
  REQUIRE(bv.FindOne(8) == 12);
  REQUIRE(bv.FindOne(13) == 13);
  REQUIRE(bv.FindOne(14) == 14);
  REQUIRE(bv.FindOne(15) == -1);

  // Get all of the ones at once and make sure they're there.
  emp::vector<size_t> ones = bv.GetOnes();
  REQUIRE(ones.size() == 5);
  REQUIRE(ones[0] == 3);
  REQUIRE(ones[1] == 7);
  REQUIRE(ones[2] == 12);
  REQUIRE(ones[3] == 13);
  REQUIRE(ones[4] == 14);

  // Identify the final one.
  REQUIRE(bv.FindMaxOne() == 14);

  // Try finding the length of the longest segment of ones.
  REQUIRE(bv.LongestSegmentOnes() == 3);

  // Pop all ones, one at a time.
  REQUIRE(bv.PopOne() == 3);
  REQUIRE(bv.PopOne() == 7);
  REQUIRE(bv.PopOne() == 12);
  REQUIRE(bv.PopOne() == 13);
  REQUIRE(bv.PopOne() == 14);
  REQUIRE(bv.PopOne() == -1);

  REQUIRE(bv.CountOnes() == 0);
  REQUIRE(bv.LongestSegmentOnes() == 0);
  REQUIRE(bv.FindMaxOne() == -1);

  bv.SetAll();                             // 1111111111111111
  REQUIRE(bv.LongestSegmentOnes() == 16);
  bv[8] = 0;                               // 1111111101111111
  REQUIRE(bv.LongestSegmentOnes() == 8);
  bv[4] = 0;                               // 1111011101111111
  REQUIRE(bv.LongestSegmentOnes() == 7);

  // Try again with Find, this time with a random sequence of ones.
  emp::Random random(1);
  bv.Randomize(random);
  size_t count = 0;
  for (int i = bv.FindOne(); i != -1; i = bv.FindOne(i+1)) count++;
  REQUIRE(count == bv.CountOnes());

}

TEST_CASE("8: Test printing and string functions.", "[bits]") {
  emp::BitVector bv6("000111");

  REQUIRE(bv6.ToString() == "000111");
  REQUIRE(bv6.ToBinaryString() == "111000");
  REQUIRE(bv6.ToIDString() == "3 4 5");
  REQUIRE(bv6.ToIDString() == "3 4 5");
  REQUIRE(bv6.ToRangeString() == "3-5");

  emp::BitVector bv64("0001110000000000000100000000000001000110000001000001000100000001");

  REQUIRE(bv64.ToString()       == "0001110000000000000100000000000001000110000001000001000100000001");
  REQUIRE(bv64.ToBinaryString() == "1000000010001000001000000110001000000000000010000000000000111000");
  REQUIRE(bv64.ToIDString() == "3 4 5 19 33 37 38 45 51 55 63");
  REQUIRE(bv64.ToIDString(",") == "3,4,5,19,33,37,38,45,51,55,63");
  REQUIRE(bv64.ToRangeString() == "3-5,19,33,37-38,45,51,55,63");

  emp::BitVector bv65("00011110000000000001000000000000010001100000010000010001000000111");

  REQUIRE(bv65.ToString()       == "00011110000000000001000000000000010001100000010000010001000000111");
  REQUIRE(bv65.ToBinaryString() == "11100000010001000001000000110001000000000000010000000000001111000");
  REQUIRE(bv65.ToIDString()     == "3 4 5 6 19 33 37 38 45 51 55 62 63 64");
  REQUIRE(bv65.ToIDString(",")  == "3,4,5,6,19,33,37,38,45,51,55,62,63,64");
  REQUIRE(bv65.ToRangeString()  == "3-6,19,33,37-38,45,51,55,62-64");
}

TEST_CASE("9: Test Boolean logic and shifting functions.", "[bits]") {
  const emp::BitVector input1 = "00001111";
  const emp::BitVector input2 = "00110011";
  const emp::BitVector input3 = "01010101";

  // Test *_SELF() Boolean Logic functions.
  emp::BitVector bv(8);    REQUIRE(bv == emp::BitVector("00000000"));
  bv.NOT_SELF();           REQUIRE(bv == emp::BitVector("11111111"));
  bv.AND_SELF(input1);     REQUIRE(bv == emp::BitVector("00001111"));
  bv.AND_SELF(input1);     REQUIRE(bv == emp::BitVector("00001111"));
  bv.AND_SELF(input2);     REQUIRE(bv == emp::BitVector("00000011"));
  bv.AND_SELF(input3);     REQUIRE(bv == emp::BitVector("00000001"));

  bv.OR_SELF(input1);      REQUIRE(bv == emp::BitVector("00001111"));
  bv.OR_SELF(input1);      REQUIRE(bv == emp::BitVector("00001111"));
  bv.OR_SELF(input3);      REQUIRE(bv == emp::BitVector("01011111"));
  bv.OR_SELF(input2);      REQUIRE(bv == emp::BitVector("01111111"));

  bv.NAND_SELF(input1);    REQUIRE(bv == emp::BitVector("11110000"));
  bv.NAND_SELF(input1);    REQUIRE(bv == emp::BitVector("11111111"));
  bv.NAND_SELF(input2);    REQUIRE(bv == emp::BitVector("11001100"));
  bv.NAND_SELF(input3);    REQUIRE(bv == emp::BitVector("10111011"));

  bv.NOR_SELF(input1);     REQUIRE(bv == emp::BitVector("01000000"));
  bv.NOR_SELF(input1);     REQUIRE(bv == emp::BitVector("10110000"));
  bv.NOR_SELF(input2);     REQUIRE(bv == emp::BitVector("01001100"));
  bv.NOR_SELF(input3);     REQUIRE(bv == emp::BitVector("10100010"));

  bv.XOR_SELF(input1);     REQUIRE(bv == emp::BitVector("10101101"));
  bv.XOR_SELF(input1);     REQUIRE(bv == emp::BitVector("10100010"));
  bv.XOR_SELF(input2);     REQUIRE(bv == emp::BitVector("10010001"));
  bv.XOR_SELF(input3);     REQUIRE(bv == emp::BitVector("11000100"));

  bv.EQU_SELF(input1);     REQUIRE(bv == emp::BitVector("00110100"));
  bv.EQU_SELF(input1);     REQUIRE(bv == emp::BitVector("11000100"));
  bv.EQU_SELF(input2);     REQUIRE(bv == emp::BitVector("00001000"));
  bv.EQU_SELF(input3);     REQUIRE(bv == emp::BitVector("10100010"));

  bv.NOT_SELF();           REQUIRE(bv == emp::BitVector("01011101"));

  // Test regular Boolean Logic functions.
  bv.Clear();                            REQUIRE(bv == emp::BitVector("00000000"));
  emp::BitVector bv1 = bv.NOT();         REQUIRE(bv1 == emp::BitVector("11111111"));

  bv1 = bv1.AND(input1);                 REQUIRE(bv1 == emp::BitVector("00001111"));
  emp::BitVector bv2 = bv1.AND(input1);  REQUIRE(bv2 == emp::BitVector("00001111"));
  emp::BitVector bv3 = bv2.AND(input2);  REQUIRE(bv3 == emp::BitVector("00000011"));
  emp::BitVector bv4 = bv3.AND(input3);  REQUIRE(bv4 == emp::BitVector("00000001"));

  bv1 = bv4.OR(input1);      REQUIRE(bv1 == emp::BitVector("00001111"));
  bv2 = bv1.OR(input1);      REQUIRE(bv2 == emp::BitVector("00001111"));
  bv3 = bv2.OR(input3);      REQUIRE(bv3 == emp::BitVector("01011111"));
  bv4 = bv3.OR(input2);      REQUIRE(bv4 == emp::BitVector("01111111"));

  bv1 = bv4.NAND(input1);    REQUIRE(bv1 == emp::BitVector("11110000"));
  bv2 = bv1.NAND(input1);    REQUIRE(bv2 == emp::BitVector("11111111"));
  bv3 = bv2.NAND(input2);    REQUIRE(bv3 == emp::BitVector("11001100"));
  bv4 = bv3.NAND(input3);    REQUIRE(bv4 == emp::BitVector("10111011"));

  bv1 = bv4.NOR(input1);     REQUIRE(bv1 == emp::BitVector("01000000"));
  bv2 = bv1.NOR(input1);     REQUIRE(bv2 == emp::BitVector("10110000"));
  bv3 = bv2.NOR(input2);     REQUIRE(bv3 == emp::BitVector("01001100"));
  bv4 = bv3.NOR(input3);     REQUIRE(bv4 == emp::BitVector("10100010"));

  bv1 = bv4.XOR(input1);     REQUIRE(bv1 == emp::BitVector("10101101"));
  bv2 = bv1.XOR(input1);     REQUIRE(bv2 == emp::BitVector("10100010"));
  bv3 = bv2.XOR(input2);     REQUIRE(bv3 == emp::BitVector("10010001"));
  bv4 = bv3.XOR(input3);     REQUIRE(bv4 == emp::BitVector("11000100"));

  bv1 = bv4.EQU(input1);     REQUIRE(bv1 == emp::BitVector("00110100"));
  bv2 = bv1.EQU(input1);     REQUIRE(bv2 == emp::BitVector("11000100"));
  bv3 = bv2.EQU(input2);     REQUIRE(bv3 == emp::BitVector("00001000"));
  bv4 = bv3.EQU(input3);     REQUIRE(bv4 == emp::BitVector("10100010"));

  bv = bv4.NOT();            REQUIRE(bv == emp::BitVector("01011101"));


  // Test Boolean Logic operators.
  bv.Clear();               REQUIRE(bv == emp::BitVector("00000000"));
  bv1 = ~bv;                REQUIRE(bv1 == emp::BitVector("11111111"));

  bv1 = bv1 & input1;       REQUIRE(bv1 == emp::BitVector("00001111"));
  bv2 = bv1 & input1;       REQUIRE(bv2 == emp::BitVector("00001111"));
  bv3 = bv2 & input2;       REQUIRE(bv3 == emp::BitVector("00000011"));
  bv4 = bv3 & input3;       REQUIRE(bv4 == emp::BitVector("00000001"));

  bv1 = bv4 | input1;       REQUIRE(bv1 == emp::BitVector("00001111"));
  bv2 = bv1 | input1;       REQUIRE(bv2 == emp::BitVector("00001111"));
  bv3 = bv2 | input3;       REQUIRE(bv3 == emp::BitVector("01011111"));
  bv4 = bv3 | input2;       REQUIRE(bv4 == emp::BitVector("01111111"));

  bv1 = ~(bv4 & input1);    REQUIRE(bv1 == emp::BitVector("11110000"));
  bv2 = ~(bv1 & input1);    REQUIRE(bv2 == emp::BitVector("11111111"));
  bv3 = ~(bv2 & input2);    REQUIRE(bv3 == emp::BitVector("11001100"));
  bv4 = ~(bv3 & input3);    REQUIRE(bv4 == emp::BitVector("10111011"));

  bv1 = ~(bv4 | input1);    REQUIRE(bv1 == emp::BitVector("01000000"));
  bv2 = ~(bv1 | input1);    REQUIRE(bv2 == emp::BitVector("10110000"));
  bv3 = ~(bv2 | input2);    REQUIRE(bv3 == emp::BitVector("01001100"));
  bv4 = ~(bv3 | input3);    REQUIRE(bv4 == emp::BitVector("10100010"));

  bv1 = bv4 ^ input1;       REQUIRE(bv1 == emp::BitVector("10101101"));
  bv2 = bv1 ^ input1;       REQUIRE(bv2 == emp::BitVector("10100010"));
  bv3 = bv2 ^ input2;       REQUIRE(bv3 == emp::BitVector("10010001"));
  bv4 = bv3 ^ input3;       REQUIRE(bv4 == emp::BitVector("11000100"));

  bv1 = ~(bv4 ^ input1);    REQUIRE(bv1 == emp::BitVector("00110100"));
  bv2 = ~(bv1 ^ input1);    REQUIRE(bv2 == emp::BitVector("11000100"));
  bv3 = ~(bv2 ^ input2);    REQUIRE(bv3 == emp::BitVector("00001000"));
  bv4 = ~(bv3 ^ input3);    REQUIRE(bv4 == emp::BitVector("10100010"));

  bv = ~bv4;                REQUIRE(bv == emp::BitVector("01011101"));


  // Test COMPOUND Boolean Logic operators.
  bv = "11111111";    REQUIRE(bv == emp::BitVector("11111111"));

  bv &= input1;       REQUIRE(bv == emp::BitVector("00001111"));
  bv &= input1;       REQUIRE(bv == emp::BitVector("00001111"));
  bv &= input2;       REQUIRE(bv == emp::BitVector("00000011"));
  bv &= input3;       REQUIRE(bv == emp::BitVector("00000001"));

  bv |= input1;       REQUIRE(bv == emp::BitVector("00001111"));
  bv |= input1;       REQUIRE(bv == emp::BitVector("00001111"));
  bv |= input3;       REQUIRE(bv == emp::BitVector("01011111"));
  bv |= input2;       REQUIRE(bv == emp::BitVector("01111111"));

  bv ^= input1;       REQUIRE(bv == emp::BitVector("01110000"));
  bv ^= input1;       REQUIRE(bv == emp::BitVector("01111111"));
  bv ^= input2;       REQUIRE(bv == emp::BitVector("01001100"));
  bv ^= input3;       REQUIRE(bv == emp::BitVector("00011001"));

  // Shifting tests.
  REQUIRE( (bv << 1) == emp::BitVector("00001100"));
  REQUIRE( (bv << 2) == emp::BitVector("00000110"));
  REQUIRE( (bv << 3) == emp::BitVector("00000011"));
  REQUIRE( (bv << 4) == emp::BitVector("00000001"));

  REQUIRE( (bv >> 1) == emp::BitVector("00110010"));
  REQUIRE( (bv >> 2) == emp::BitVector("01100100"));
  REQUIRE( (bv >> 3) == emp::BitVector("11001000"));
  REQUIRE( (bv >> 4) == emp::BitVector("10010000"));

  // Now some tests with bitvectors longer than one field.
  const emp::BitVector bvl80 =
    "00110111000101110001011100010111000101110001011100010111000101110001011100010111";
  REQUIRE( bvl80.GetSize() == 80 );
  REQUIRE( bvl80.CountOnes() == 41 );
  REQUIRE( bvl80 << 1 ==
           emp::BitVector("00011011100010111000101110001011100010111000101110001011100010111000101110001011")
         );
  REQUIRE( bvl80 << 2 ==
           emp::BitVector("00001101110001011100010111000101110001011100010111000101110001011100010111000101")
         );
  REQUIRE( bvl80 << 63 ==
           emp::BitVector("00000000000000000000000000000000000000000000000000000000000000000110111000101110")
         );
  REQUIRE( bvl80 << 64 ==
           emp::BitVector("00000000000000000000000000000000000000000000000000000000000000000011011100010111")
         );
  REQUIRE( bvl80 << 65 ==
           emp::BitVector("00000000000000000000000000000000000000000000000000000000000000000001101110001011")
         );

  REQUIRE( bvl80 >> 1 ==
           emp::BitVector("01101110001011100010111000101110001011100010111000101110001011100010111000101110")
         );
  REQUIRE( bvl80 >> 2 ==
           emp::BitVector("11011100010111000101110001011100010111000101110001011100010111000101110001011100")
         );
  REQUIRE( bvl80 >> 63 ==
           emp::BitVector("10001011100010111000000000000000000000000000000000000000000000000000000000000000")
         );
  REQUIRE( bvl80 >> 64 ==
           emp::BitVector("00010111000101110000000000000000000000000000000000000000000000000000000000000000")
         );
  REQUIRE( bvl80 >> 65 ==
           emp::BitVector("00101110001011100000000000000000000000000000000000000000000000000000000000000000")
         );
}

TEST_CASE("10: Test functions that trigger size changes", "[bits]") {
  emp::BitVector bv(10);
  REQUIRE(bv.GetSize() == 10);
  REQUIRE(bv.CountOnes() == 0);
  REQUIRE(bv.CountZeros() == 10);

  bv.Resize(1000);
  REQUIRE(bv.GetSize() == 1000);
  REQUIRE(bv.CountOnes() == 0);
  REQUIRE(bv.CountZeros() == 1000);

  bv.SetAll();
  REQUIRE(bv.GetSize() == 1000);
  REQUIRE(bv.CountOnes() == 1000);
  REQUIRE(bv.CountZeros() == 0);

  emp::Random random(1);
  bv.Randomize(random);
  REQUIRE(bv.CountOnes() == bv.CountOnes_Sparse());
  size_t num_ones = bv.CountOnes();
  size_t num_zeros = bv.CountZeros();
  REQUIRE(num_ones > 425);
  REQUIRE(num_zeros > 425);
  REQUIRE(num_ones + num_zeros == 1000);

  while (bv.GetSize()) {
    if (bv.PopBack()) num_ones--;
    else num_zeros--;
  }

  REQUIRE(num_ones == 0);
  REQUIRE(num_zeros == 0);

  for (size_t i = 0; i < 500; i++) {
    bv.PushBack(0);
    bv.PushBack(1);
  }

  REQUIRE(bv.GetSize() == 1000);
  REQUIRE(bv.CountOnes() == 500);
  REQUIRE(bv.CountZeros() == 500);

  bv.Insert(250, 0, 500); // Insert 500 zeros at index 250.

  REQUIRE(bv.GetSize() == 1500);
  REQUIRE(bv.CountOnes() == 500);
  REQUIRE(bv.CountZeros() == 1000);
  for (size_t i = 250; i < 750; i++) REQUIRE(bv[i] == 0);

  bv.Insert(1250, 1, 500); // Insert ones zeros at index 1250 (250 before end).

  REQUIRE(bv.GetSize() == 2000);
  REQUIRE(bv.CountOnes() == 1000);
  REQUIRE(bv.CountZeros() == 1000);
  for (size_t i = 1250; i < 1750; i++) REQUIRE(bv[i] == 1);

  bv.Delete(500,550);   // Delete 250 zeros and 300 pairs of zeros and ones.

  REQUIRE(bv.GetSize() == 1450);
  REQUIRE(bv.CountOnes() == 850);
  REQUIRE(bv.CountZeros() == 600);
}

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
  REQUIRE(bv1 == bv);
  REQUIRE(bv1.Get(0));
  emp::BitVector bv20(20);
  emp::BitVector bv30(30);
  bv20.Set(1);
  REQUIRE(bv20.Get(1));
  bv20 = bv;
  REQUIRE(bv20 == bv);
  REQUIRE(bv20.size()==bv.size());
  REQUIRE(!bv20.Get(1));
  bv20 = bv30;
  REQUIRE(!bv20.Get(1));
  REQUIRE(bv20 == bv30);

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
  REQUIRE((ss.str() == "1 7"));
  ss.str(std::string()); // clear ss

  bv3.PrintArray(ss);
  REQUIRE((ss.str() == "01000001"));
  ss.str(std::string()); // clear ss

  // Find & Pop Bit
  bv3.SetByte(0,74);
  REQUIRE((bv3.PopOne() == 1));
  REQUIRE((bv3.CountOnes() == 2));
  REQUIRE((bv3.GetByte(0) == 72));
  REQUIRE((bv3.FindOne() == 3));
  REQUIRE((bv3.FindOne(4) == 6));
  bv3.PopOne();
  bv3.PopOne();
  REQUIRE((bv3.FindOne() == -1));
  REQUIRE((bv3.FindOne(2) == -1));
  REQUIRE((bv3.PopOne() == -1));

  // Get Ones
  emp::vector<size_t> ones = bv3.GetOnes();
  REQUIRE((ones.size() == 0));
  bv3.SetByte(0,10);
  ones = bv3.GetOnes();
  REQUIRE((ones[0] == 1));
  REQUIRE((ones[1] == 3));

  // Larger BitVector
  emp::BitVector bv4(96);
  bv4.SetByte(1,1);
  bv4.PrintFields(ss);
  REQUIRE(ss.str() == "00000000000000000000000000000000 0000000000000000000000000000000000000000000000000000000100000000");

  // test single set.
  bv4[62] = 1;
  ss.str(std::string()); bv4.PrintFields(ss); // Clear & resend bits.
  REQUIRE(ss.str() == "00000000000000000000000000000000 0100000000000000000000000000000000000000000000000000000100000000");
  // test toggle of range (across boundary)
  bv4.Toggle(61, 70);
  ss.str(std::string()); bv4.PrintFields(ss); // Clear & resend bits.
  REQUIRE(ss.str() == "00000000000000000000000000111111 1010000000000000000000000000000000000000000000000000000100000000");
  // test clearing a range in a single field.
  bv4.Clear(65, 69);
  ss.str(std::string()); bv4.PrintFields(ss); // Clear & resend bits.
  REQUIRE(ss.str() == "00000000000000000000000000100001 1010000000000000000000000000000000000000000000000000000100000000");
  // test toggling a larger range
  bv4.Toggle(55, 75);
  ss.str(std::string()); bv4.PrintFields(ss); // Clear & resend bits.
  REQUIRE(ss.str() == "00000000000000000000011111011110 0101111110000000000000000000000000000000000000000000000100000000");
  // test clearing a field across bounderies
  bv4.Clear(56, 74);
  ss.str(std::string()); bv4.PrintFields(ss); // Clear & resend bits.
  REQUIRE(ss.str() == "00000000000000000000010000000000 0000000010000000000000000000000000000000000000000000000100000000");

  // Even longer bit vector (to test operations that span multiple fields)
  bv4.Resize(300);
  ss.str(std::string()); bv4.PrintFields(ss); // Clear & resend bits.
  REQUIRE(ss.str() == "00000000000000000000000000000000000000000000 0000000000000000000000000000000000000000000000000000000000000000 0000000000000000000000000000000000000000000000000000000000000000 0000000000000000000000000000000000000000000000000000010000000000 0000000010000000000000000000000000000000000000000000000100000000");
  // test setting a range that spans three fields.
  bv4.SetRange(100, 250);
  ss.str(std::string()); bv4.PrintFields(ss); // Clear & resend bits.
  REQUIRE(ss.str() == "00000000000000000000000000000000000000000000 0000001111111111111111111111111111111111111111111111111111111111 1111111111111111111111111111111111111111111111111111111111111111 1111111111111111111111111111000000000000000000000000010000000000 0000000010000000000000000000000000000000000000000000000100000000");
  // test clearing a full field.
  bv4.Clear(128,192);
  ss.str(std::string()); bv4.PrintFields(ss); // Clear & resend bits.
  REQUIRE(ss.str() == "00000000000000000000000000000000000000000000 0000001111111111111111111111111111111111111111111111111111111111 0000000000000000000000000000000000000000000000000000000000000000 1111111111111111111111111111000000000000000000000000010000000000 0000000010000000000000000000000000000000000000000000000100000000");
  // test clearing slightly more than a full field.
  bv4.Clear(127,193);
  ss.str(std::string()); bv4.PrintFields(ss); // Clear & resend bits.
  REQUIRE(ss.str() == "00000000000000000000000000000000000000000000 0000001111111111111111111111111111111111111111111111111111111110 0000000000000000000000000000000000000000000000000000000000000000 0111111111111111111111111111000000000000000000000000010000000000 0000000010000000000000000000000000000000000000000000000100000000");
  // test setting a full field.
  bv4.SetRange(128,192);
  ss.str(std::string()); bv4.PrintFields(ss); // Clear & resend bits.
  REQUIRE(ss.str() == "00000000000000000000000000000000000000000000 0000001111111111111111111111111111111111111111111111111111111110 1111111111111111111111111111111111111111111111111111111111111111 0111111111111111111111111111000000000000000000000000010000000000 0000000010000000000000000000000000000000000000000000000100000000");
  ss.str(std::string()); // clear ss


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

  // #ifdef TDEBUG
  // REQUIRE(emp::assert_last_fail == 0);
  // #endif

}

TEST_CASE("Test MaskHigh, MaskLow", "[bits]") {

  // #ifdef TDEBUG
  // REQUIRE(emp::assert_last_fail == 0);
  // #endif

  // Test MaskHigh, MaskLow
  emp::BitVector a(0);
  a.Insert(0,true, 7);
  REQUIRE(a.Get(0));
  REQUIRE(a.Get(1));
  REQUIRE(a.Get(2));

  // #ifdef TDEBUG
  // REQUIRE(emp::assert_last_fail == 0);
  // #endif

}

TEST_CASE("Test PopBack, PushBack, Insert, Delete", "[bits]") {

  // #ifdef TDEBUG
  // REQUIRE(emp::assert_last_fail == 0);
  // #endif

  // Pop Back and Push Back
  emp::BitVector bv_g(0);    // Empty BitVector
  bv_g.PushBack(true);       // 1
  bv_g.PushBack(true);       // 11
  bv_g.PushBack(false);      // 110
  REQUIRE(bv_g.Get(0));
  REQUIRE(bv_g.Get(1));
  REQUIRE(!bv_g.PopBack());  // 11
  REQUIRE(bv_g.size() == 2);

  // Insert and Delete
  bv_g.Insert(1, true);      // 111
  REQUIRE(bv_g.Get(0));
  REQUIRE(bv_g.Get(1));
  REQUIRE(bv_g.Get(2));
  REQUIRE(bv_g.size() == 3);

  bv_g.Insert(1, true);      // 1111
  REQUIRE(bv_g.Get(3));
  REQUIRE(bv_g.Get(2));
  REQUIRE(bv_g.Get(1));
  REQUIRE(bv_g.Get(0));
  REQUIRE(bv_g.size() == 4);

  bv_g.Insert(1, false);     // 10111
  REQUIRE(bv_g.Get(0));
  REQUIRE(!bv_g.Get(1));
  REQUIRE(bv_g.Get(2));
  REQUIRE(bv_g.Get(3));

  bv_g.Delete(0);            // 0111
  REQUIRE(bv_g.size() == 4);
  REQUIRE(!bv_g.Get(0));
  bv_g.Delete(1, 2);         // 01
  REQUIRE(bv_g.size() == 2);
  REQUIRE(bv_g.Get(1));

  // #ifdef TDEBUG
  // REQUIRE(emp::assert_last_fail == 0);
  // #endif

}

TEST_CASE("Another Test BitVector", "[bits]") {

  // #ifdef TDEBUG
  // REQUIRE(emp::assert_last_fail == 0);
  // #endif

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
  REQUIRE(bv80.GetUInt32(2) == 130);
  REQUIRE(bv80.GetUIntAtBit(64) == 130);
//  REQUIRE(bv80.GetValueAtBit<5>(64) == 2);

  // #ifdef TDEBUG
  // REQUIRE(emp::assert_last_fail == 0);
  // #endif

}

TEST_CASE("Test range of BitVector constructors.", "[bits]")
// test list initializer
{
  emp::BitVector bs_empty{0,0,0};
  emp::BitVector bs_first{1,0,0};
  emp::BitVector bs_last{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
  emp::BitVector bs_two{0,0,1,0,0,0,0,0,0,0,1,0,0};
  emp::BitVector bs_full{1,1,1,1,1,1,1,1};

  REQUIRE(bs_empty.CountOnes() == 0);
  REQUIRE(bs_first.CountOnes() == 1);
  REQUIRE(bs_last.CountOnes() == 1);
  REQUIRE(bs_two.CountOnes() == 2);
  REQUIRE(bs_full.CountOnes() == 8);

  REQUIRE(bs_empty.GetSize() == 3);
  REQUIRE(bs_first.GetSize() == 3);
  REQUIRE(bs_last.GetSize() == 25);
  REQUIRE(bs_two.GetSize() == 13);
  REQUIRE(bs_full.GetSize() == 8);
}

// TEST_CASE("BitVector padding bits protected", "[bits]") {
// #ifdef TDEBUG

//   REQUIRE(emp::assert_last_fail == 0);

//   for (size_t i = 1; i < 32; ++i) {

//     emp::BitVector vec(i);
//     REQUIRE(emp::assert_last_fail == 0);
//     vec.SetUInt(0, std::numeric_limits<uint32_t>::max());
//     REQUIRE(emp::assert_last_fail);
//     emp::assert_clear();

//   }

//   REQUIRE(emp::assert_last_fail == 0);

//   emp::BitVector vec(32);
//   vec.SetUInt(0, std::numeric_limits<uint32_t>::max());

//   REQUIRE(emp::assert_last_fail == 0);

// #endif
// }

TEST_CASE("BitVector regression test for #277", "[bits]") {
  // #ifdef TDEBUG
  // REQUIRE(emp::assert_last_fail == 0);
  // #endif

  emp::BitVector vec1(4);
  emp::BitVector vec2(4);

  for (size_t i = 0; i < 4; ++i) REQUIRE(!vec1[i]);
  for (size_t i = 0; i < 4; ++i) REQUIRE(!vec2[i]);
  vec1.SetUInt(0, 15);
  vec2.SetUIntAtBit(0, 15);
  for (size_t i = 0; i < 4; ++i) REQUIRE(vec1[i]);
  for (size_t i = 0; i < 4; ++i) REQUIRE(vec2[i]);

  // #ifdef TDEBUG
  // REQUIRE(emp::assert_last_fail == 0);
  // #endif

}
