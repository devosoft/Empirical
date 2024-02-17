/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022-24.
 *
 *  @file Bits.cpp
 */

#include <limits>
#include <map>
#include <ratio>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>

#include "emp/base/vector.hpp"
#include "emp/bits/Bits.hpp"
#include "emp/math/Random.hpp"
#include "emp/meta/macros.hpp"
#include "emp/tools/string_utils.hpp"

#define BITS_TEST(TYPE, CONSTRUCT, ...) { TYPE CONSTRUCT; __VA_ARGS__ }

#define BITS_TEST_ALL(BASE_SIZE, CONSTRUCT, ...)                      \
  BITS_TEST(emp::BitVector, CONSTRUCT, __VA_ARGS__)                   \
  BITS_TEST(emp::BitValue, CONSTRUCT, __VA_ARGS__)                    \
  BITS_TEST(emp::StaticBitVector<BASE_SIZE>, CONSTRUCT, __VA_ARGS__)  \
  BITS_TEST(emp::StaticBitValue<BASE_SIZE>, CONSTRUCT, __VA_ARGS__)   \
  BITS_TEST(emp::BitArray<BASE_SIZE>, CONSTRUCT, __VA_ARGS__)         \
  BITS_TEST(emp::BitSet<BASE_SIZE>, CONSTRUCT, __VA_ARGS__)

#define BITS_TEST_ALL_ZEROS(SIZE) BITS_TEST_ALL(SIZE, bits(SIZE, false), TestBasics( bits, SIZE, "Zeros");)
#define BITS_TEST_ALL_ONES(SIZE) BITS_TEST_ALL(SIZE, bits(SIZE, true), TestBasics( bits, SIZE, "Ones");)

template<typename T>
void TestBasics(const T & bits, size_t _size, std::string vals="") {

  CHECK( bits.GetSize() == _size);
  if (vals == "") { return; } // No values to check.
  else if (vals == "Zeros") { CHECK(bits.CountOnes() == 0); }
  else if (vals == "Ones")  { CHECK(bits.CountOnes() == _size); }
  else if (vals == "Mixed") { CHECK(bits.CountOnes() > 0); CHECK(bits.CountOnes() < _size); }

  // If vals is a number, use that as the expected number of ones.
  else if (emp::is_digits(vals)) {
    CHECK(bits.CountOnes() == emp::from_string<size_t>(vals));
  }

  // If vals is a range in the format "[100,200]" then make sure in that range.
  else if (vals[0] == '[') {
    vals.erase(0,1); // erase open '['
    size_t start = emp::string_pop_uint(vals);
    emp_assert(vals.size() && vals[0] == ',');
    vals.erase(0,1); // erase ','
    size_t end = emp::string_pop_uint(vals);
    emp_assert(vals.size() && vals[0] == ']');

    size_t count = bits.CountOnes();
    CHECK(count >= start);
    CHECK(count <= end);
  }
}

TEST_CASE("1: Test Bits Constructors", "[bits]"){
  // Default constructors.
  emp::BitVector           bv0;    TestBasics( bv0, 0, "Zeros" );
  emp::BitValue            bvl0;   TestBasics( bvl0, 0, "Zeros" );
  emp::StaticBitVector<50> sbv0;   TestBasics( sbv0, 0, "Zeros" );
  emp::StaticBitValue<50>  sbvl0;  TestBasics( sbvl0, 0, "Zeros" );
  emp::BitArray<50>        ba0;    TestBasics( ba0, 50, "Zeros" );
  emp::BitSet<50>          bs0;    TestBasics( bs0, 50, "Zeros" );

  // Create size 50 bits objects, default to all zeros.
  BITS_TEST_ALL(50, bits(50), TestBasics( bits, 50, "Zeros");)

  // Create larger (size 5000) bits objects, default to all ones.
  BITS_TEST_ALL(5000, bits(5000, true), TestBasics( bits, 5000, "Ones");)

  // Create huge (size 100,000) bits objects.
  BITS_TEST_ALL(100000, bits(100000), TestBasics( bits, 100000, "Zeros");)

  // Try a full range of BitVector sizes, from 1 to 200.
  EMP_LAYOUT( BITS_TEST_ALL_ZEROS, , 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50 )
  EMP_LAYOUT( BITS_TEST_ALL_ZEROS, , 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100 )
  EMP_LAYOUT( BITS_TEST_ALL_ZEROS, , 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150 )
  EMP_LAYOUT( BITS_TEST_ALL_ZEROS, , 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 100 )

  EMP_LAYOUT( BITS_TEST_ALL_ONES, , 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50 )
  EMP_LAYOUT( BITS_TEST_ALL_ONES, , 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100 )
  EMP_LAYOUT( BITS_TEST_ALL_ONES, , 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150 )
  EMP_LAYOUT( BITS_TEST_ALL_ONES, , 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 100 )


  // Build a relatively large BitVector and populate it with scatters ones.
  BITS_TEST_ALL(100000, bits(100000),
    for (size_t i = 0; i < bits.GetSize(); i += 91) bits[i].Toggle();
    TestBasics( bits, 100000, "1099");
  )

  // Try out all combinations of the copy constructor.
  BITS_TEST_ALL(100, b1(100), b1.Set(50); emp::BitVector b2(b1); TestBasics(b2, 100, "1");)
  BITS_TEST_ALL(100, b1(100), b1.Set(50); emp::BitValue  b2(b1); TestBasics(b2, 100, "1");)
  BITS_TEST_ALL(100, b1(100), b1.Set(50); emp::BitArray<100> b2(b1); TestBasics(b2, 100, "1");)
  BITS_TEST_ALL(100, b1(100), b1.Set(50); emp::BitSet<100> b2(b1); TestBasics(b2, 100, "1");)
  BITS_TEST_ALL(100, b1(100), b1.Set(50); emp::StaticBitVector<105> b2(b1); TestBasics(b2, 100, "1");)
  BITS_TEST_ALL(100, b1(100), b1.Set(50); emp::StaticBitValue<105> b2(b1); TestBasics(b2, 100, "1");)


  // And the move constructor (on BitVector only since it's movable.)
  emp::BitVector bv1(1000);
  bv1 = "0,0,0,1,1,1,1,1,1,0,0,1,1,0";
  auto old_ptr = bv1.RawBytes();         // Grab a pointer to where bv5 currently has its bytes.
  emp::BitVector bv2( std::move(bv1) );  // Move bv5 bytes into bv6.
  CHECK( bv2.RawBytes() == old_ptr );
  CHECK( bv1.RawBytes() == nullptr );


  // Construct from std::bitset.
  std::bitset<6> bit_set;
  bit_set[1] = 1;   bit_set[2] = 1;   bit_set[4] = 1;
  BITS_TEST_ALL(6, bits(bit_set),
    TestBasics( bits, 6, "3" );
  )

  // Construct from string.
  std::string bit_string = "10011001010000011101";
  BITS_TEST_ALL(20, bits(bit_string),
    TestBasics( bits, 20, "9" );
  )


  // Some random Bits objects
  emp::Random random(1);
  BITS_TEST_ALL(1000, bits(1000, random),
    TestBasics( bits, 1000, "[400,600]" );
  )

  // Random Bits objects with 80% chance of ones.
  BITS_TEST_ALL(1000, bits(1000, random, 0.8),
    TestBasics( bits, 1000, "[750,850]" );
  )

  // Random Bits objects with exactly 117 ones, randomly placed.
  BITS_TEST_ALL(1000, bits(1000, random, 117),
    TestBasics( bits, 1000, "117" );
  )

  // Construct with just the first half of another Bits object.
  emp::BitVector bits_base(1000, random);
  BITS_TEST_ALL(500, bits(bits_base, 500),
    TestBasics( bits, 500, "[180,320]" );
  )

  BITS_TEST_ALL(13, bits({1,0,0,0,1,1,1,0,0,0,1,1,1}),
    TestBasics( bits, 13, "7" );
  )
}

TEST_CASE("2: Test Bits Assignments", "[bits]"){
  emp::vector< emp::BitVector > b_vec;
  emp::vector< emp::BitValue > b_val;
  emp::vector< emp::StaticBitVector<128> > b_svec;
  emp::vector< emp::StaticBitValue<128> > b_sval;
  emp::vector< emp::BitArray<128> > b_arr;
  emp::vector< emp::BitSet<128> > b_set;

  // For resizable Bits types, try all sizes from 0 to 128.
  // Lot's of move operators will trigger as vector grows.
  for (size_t i = 0; i <= 128; i++) {
    b_vec.emplace_back(i);
    b_val.emplace_back(i);
    b_svec.emplace_back(i);
    b_sval.emplace_back(i);
  }

  // And a few larger BitVectors...
  b_vec.emplace_back(1023);
  b_vec.emplace_back(1024);
  b_vec.emplace_back(1025);
  b_vec.emplace_back(1000000);

  // Copy each BitVector into bv2 and do some manipulations then copy back.
  for (emp::BitVector & bv : b_vec) {
    emp::BitVector bv2 = bv;
    for (size_t i = 1; i < bv2.GetSize(); i += 2) {
      bv2[i] = 1;
    }
    bv = bv2;
  }

  // Now make sure the we constructed bits correctly!
  for (const emp::BitVector & bv : b_vec) {
    CHECK( bv.CountOnes() == bv.GetSize()/2 );
  }

  emp::vector< emp::BitVector > v2;
  v2.push_back( emp::BitVector({0,1,0,1,0,1}) );

  v2 = b_vec; // Copy over all BitVectors.

  std::bitset<600> bit_set;
  bit_set[1] = 1;   bit_set[22] = 1;   bit_set[444] = 1;

  b_vec[10] = bit_set;  // Copy in an std::bitset.

  CHECK( b_vec[10].GetSize() == 600 );
  CHECK( b_vec[10].CountOnes() == 3 );

  std::string bit_string = "100110010100000111011001100101000001110110011001010000011101";

  b_vec[75] = bit_string;

  CHECK( b_vec[75].GetSize() == 60 );
  CHECK( b_vec[75].CountOnes() == 27 );

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
  CHECK( bv0.GetSize() == 0 );
  CHECK( bv1.GetSize() == 1 );
  CHECK( bv8.GetSize() == 8 );
  CHECK( bv32.GetSize() == 32 );
  CHECK( bv64.GetSize() == 64 );
  CHECK( bv75.GetSize() == 75 );
  CHECK( bv1k.GetSize() == 1000 );

  // Check byte counts (should always round up!)
  CHECK( bv0.GetNumBytes() == 0 );
  CHECK( bv1.GetNumBytes() == 1 );     // round up!
  CHECK( bv8.GetNumBytes() == 1 );
  CHECK( bv32.GetNumBytes() == 4 );
  CHECK( bv64.GetNumBytes() == 8 );
  CHECK( bv75.GetNumBytes() == 10 );   // round up!
  CHECK( bv1k.GetNumBytes() == 125 );

  // How many states can be represented in each size of BitVector?
  CHECK( bv0.GetNumStates() == 1.0 );
  CHECK( bv1.GetNumStates() == 2.0 );
  CHECK( bv8.GetNumStates() == 256.0 );
  CHECK( bv32.GetNumStates() == 4294967296.0 );
  CHECK( bv64.GetNumStates() >= 18446744073709551610.0 );
  CHECK( bv64.GetNumStates() <= 18446744073709551720.0 );
  CHECK( bv75.GetNumStates() >= 37778931862957161709560.0 );
  CHECK( bv75.GetNumStates() <= 37778931862957161709570.0 );
  CHECK( bv1k.GetNumStates() == emp::Pow2(1000) );

  // Test Get()
  CHECK( bv1.Get(0) == 1 );
  CHECK( bv8.Get(0) == 1 );
  CHECK( bv8.Get(4) == 1 );
  CHECK( bv8.Get(6) == 0 );
  CHECK( bv8.Get(7) == 1 );
  CHECK( bv75.Get(0) == 0 );
  CHECK( bv75.Get(1) == 1 );
  CHECK( bv75.Get(72) == 0 );
  CHECK( bv75.Get(73) == 1 );
  CHECK( bv75.Get(74) == 1 );

  // Test Has() (including out of range)
  CHECK( bv0.Has(0) == false );
  CHECK( bv0.Has(1000000) == false );

  CHECK( bv1.Has(0) == true );
  CHECK( bv1.Has(1) == false );

  CHECK( bv8.Has(0) == true );
  CHECK( bv8.Has(4) == true );
  CHECK( bv8.Has(6) == false );
  CHECK( bv8.Has(7) == true );
  CHECK( bv8.Has(8) == false );

  CHECK( bv75.Has(0) == false );
  CHECK( bv75.Has(1) == true );
  CHECK( bv75.Has(72) == false );
  CHECK( bv75.Has(73) == true );
  CHECK( bv75.Has(74) == true );
  CHECK( bv75.Has(75) == false );
  CHECK( bv75.Has(79) == false );
  CHECK( bv75.Has(1000000) == false );

  // Test Set(), changing in most (but not all) cases.
  bv1.Set(0, 0);
  CHECK( bv1.Get(0) == 0 );
  bv8.Set(0, 1);                // Already a 1!
  CHECK( bv8.Get(0) == 1 );
  bv8.Set(4, 0);
  CHECK( bv8.Get(4) == 0 );
  bv8.Set(6, 1);
  CHECK( bv8.Get(6) == 1 );
  bv8.Set(7, 0);
  CHECK( bv8.Get(7) == 0 );
  bv75.Set(0, 0);               // Already a 0!
  CHECK( bv75.Get(0) == 0 );
  bv75.Set(1, 0);
  CHECK( bv75.Get(1) == 0 );
  bv75.Set(72);                 // No second arg!
  CHECK( bv75.Get(72) == 1 );
  bv75.Set(73);                 // No second arg AND already a 1!
  CHECK( bv75.Get(73) == 1 );
  bv75.Set(74, 0);
  CHECK( bv75.Get(74) == 0 );
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
  CHECK( bv0.GetSize() == 0 );

  // Now try range-based accessors on a single bit.
  emp::BitVector bv1(1, false);  CHECK( bv1[0] == false );   CHECK( bv1.CountOnes() == 0 );
  bv1.Set(0);                    CHECK( bv1[0] == true );    CHECK( bv1.CountOnes() == 1 );
  bv1.Clear(0);                  CHECK( bv1[0] == false );   CHECK( bv1.CountOnes() == 0 );
  bv1.Toggle(0);                 CHECK( bv1[0] == true );    CHECK( bv1.CountOnes() == 1 );
  bv1.Clear();                   CHECK( bv1[0] == false );   CHECK( bv1.CountOnes() == 0 );
  bv1.SetAll();                  CHECK( bv1[0] == true );    CHECK( bv1.CountOnes() == 1 );
  bv1.Toggle();                  CHECK( bv1[0] == false );   CHECK( bv1.CountOnes() == 0 );
  bv1.SetRange(0,1);             CHECK( bv1[0] == true );    CHECK( bv1.CountOnes() == 1 );
  bv1.Clear(0,1);                CHECK( bv1[0] == false );   CHECK( bv1.CountOnes() == 0 );
  bv1.Toggle(0,1);               CHECK( bv1[0] == true );    CHECK( bv1.CountOnes() == 1 );
  bv1.Set(0, false);             CHECK( bv1[0] == false );   CHECK( bv1.CountOnes() == 0 );
  bv1.SetRange(0,0);             CHECK( bv1[0] == false );   CHECK( bv1.CountOnes() == 0 );
  bv1.SetRange(1,1);             CHECK( bv1[0] == false );   CHECK( bv1.CountOnes() == 0 );

  // Test when a full byte is used.
  emp::BitVector bv8( "10001101" );   CHECK(bv8.GetValue() == 177.0);  // 10110001
  bv8.Set(2);                         CHECK(bv8.GetValue() == 181.0);  // 10110101
  bv8.Set(0, 0);                      CHECK(bv8.GetValue() == 180.0);  // 10110100
  bv8.SetRange(1, 4);                 CHECK(bv8.GetValue() == 190.0);  // 10111110
  bv8.SetAll();                       CHECK(bv8.GetValue() == 255.0);  // 11111111
  bv8.Clear(3);                       CHECK(bv8.GetValue() == 247.0);  // 11110111
  bv8.Clear(5,5);                     CHECK(bv8.GetValue() == 247.0);  // 11110111
  bv8.Clear(5,7);                     CHECK(bv8.GetValue() == 151.0);  // 10010111
  bv8.Clear();                        CHECK(bv8.GetValue() ==   0.0);  // 00000000
  bv8.Toggle(4);                      CHECK(bv8.GetValue() ==  16.0);  // 00010000
  bv8.Toggle(4,6);                    CHECK(bv8.GetValue() ==  32.0);  // 00100000
  bv8.Toggle(0,3);                    CHECK(bv8.GetValue() ==  39.0);  // 00100111
  bv8.Toggle(7,8);                    CHECK(bv8.GetValue() == 167.0);  // 10100111
  bv8.Toggle();                       CHECK(bv8.GetValue() ==  88.0);  // 01011000

  // Test a full field.
  constexpr double ALL_64 = (double) ((uint64_t) -1);
  emp::BitVector bv64( "11011000110110001101" ); CHECK(bv64.GetValue() == 727835.0);
  bv64.Resize(64);      CHECK(bv64.GetValue() == 727835.0);        // ...0 010110001101100011011
  bv64.Set(6);          CHECK(bv64.GetValue() == 727899.0);        // ...0 010110001101101011011
  bv64.Set(0, 0);       CHECK(bv64.GetValue() == 727898.0);        // ...0 010110001101101011010
  bv64.SetRange(4, 9);  CHECK(bv64.GetValue() == 728058.0);        // ...0 010110001101111111010
  bv64.SetAll();        CHECK(bv64.GetValue() == ALL_64);          // ...1 111111111111111111111
  bv64.Clear(2);        CHECK(bv64.GetValue() == ALL_64 - 4);      // ...1 111111111111111111011
  bv64.Clear(5,5);      CHECK(bv64.GetValue() == ALL_64 - 4);      // ...1 111111111111111111011
  bv64.Clear(5,7);      CHECK(bv64.GetValue() == ALL_64 - 100);    // ...1 111111111111110011011
  bv64.Clear();         CHECK(bv64.GetValue() == 0.0);             // ...0 000000000000000000000
  bv64.Toggle(19);      CHECK(bv64.GetValue() == emp::Pow2(19));   // ...0 010000000000000000000
  bv64.Toggle(15,20);   CHECK(bv64.GetValue() == 491520.0);        // ...0 001111000000000000000
  bv64.Toggle();        CHECK(bv64.GetValue() == ALL_64-491520.0); // ...1 110000111111111111111
  bv64.Toggle(0,64);    CHECK(bv64.GetValue() == 491520.0);        // ...0 001111000000000000000


  emp::BitVector bv75( "010001011100010111110000011110100011111000001110100000111110010011111000011" );

  // Test a full + partial field.
  constexpr double ALL_88 = ((double) ((uint64_t) -1)) * emp::Pow2(24);
  emp::BitVector bv88( "11011000110110001101" ); CHECK(bv88.GetValue() == 727835.0);
  bv88.Resize(88);      CHECK(bv88.GetValue() == 727835.0);        // ...0 010110001101100011011

  // Start with same tests as last time...
  bv88.Set(6);          CHECK(bv88.GetValue() == 727899.0);        // ...0 010110001101101011011
  bv88.Set(0, 0);       CHECK(bv88.GetValue() == 727898.0);        // ...0 010110001101101011010
  bv88.SetRange(4, 9);  CHECK(bv88.GetValue() == 728058.0);        // ...0 010110001101111111010
  bv88.SetAll();        CHECK(bv88.GetValue() == ALL_88);          // ...1 111111111111111111111
  bv88.Clear(2);        CHECK(bv88.GetValue() == ALL_88 - 4);      // ...1 111111111111111111011
  bv88.Clear(5,5);      CHECK(bv88.GetValue() == ALL_88 - 4);      // ...1 111111111111111111011
  bv88.Clear(5,7);      CHECK(bv88.GetValue() == ALL_88 - 100);    // ...1 111111111111110011011
  bv88.Clear();         CHECK(bv88.GetValue() == 0.0);             // ...0 000000000000000000000
  bv88.Toggle(19);      CHECK(bv88.GetValue() == emp::Pow2(19));   // ...0 010000000000000000000
  bv88.Toggle(15,20);   CHECK(bv88.GetValue() == 491520.0);        // ...0 001111000000000000000
  bv88.Toggle();        CHECK(bv88.GetValue() == ALL_88-491520.0); // ...1 110000111111111111111
  bv88.Toggle(0,88);    CHECK(bv88.GetValue() == 491520.0);        // ...0 001111000000000000000

  bv88 <<= 20;          CHECK(bv88.CountOnes() == 4);   // four ones, moved to bits 35-39
  bv88 <<= 27;          CHECK(bv88.CountOnes() == 4);   // four ones, moved to bits 62-65
  bv88 <<= 22;          CHECK(bv88.CountOnes() == 4);   // four ones, moved to bits 84-87
  bv88 <<= 1;           CHECK(bv88.CountOnes() == 3);   // three ones left, moved to bits 85-87
  bv88 <<= 2;           CHECK(bv88.CountOnes() == 1);   // one one left, at bit 87
  bv88 >>= 30;          CHECK(bv88.CountOnes() == 1);   // one one left, now at bit 57
  bv88.Toggle(50,80);   CHECK(bv88.CountOnes() == 29);  // Toggling 30 bits, only one was on.
  bv88.Clear(52,78);    CHECK(bv88.CountOnes() == 4);   // Leave two 1s on each side of range
  bv88.SetRange(64,66); CHECK(bv88.CountOnes() == 6);   // Set two more 1s, just into 2nd field.

  // A larger BitVector with lots of random tests.
  emp::Random random(1);
  emp::BitVector bv1k(1000, random, 0.65);
  size_t num_ones = bv1k.CountOnes();  CHECK(num_ones > 550);
  bv1k.Toggle();                       CHECK(bv1k.CountOnes() == 1000 - num_ones);

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

  CHECK(bv_empty.Any() == false);
  CHECK(bv_mixed.Any() == true);
  CHECK(bv_full.Any() == true);

  CHECK(bv_empty.All() == false);
  CHECK(bv_mixed.All() == false);
  CHECK(bv_full.All() == true);

  CHECK(bv_empty.None() == true);
  CHECK(bv_mixed.None() == false);
  CHECK(bv_full.None() == false);
}

TEST_CASE("5: Test Randomize() and variants", "[bits]") {
  emp::Random random(1);
  emp::BitVector bv(1000);

  CHECK(bv.None() == true);

  // Do all of the random tests 10 times.
  for (size_t test_num = 0; test_num < 10; test_num++) {
    bv.Randomize(random);
    size_t num_ones = bv.CountOnes();
    CHECK(num_ones > 300);
    CHECK(num_ones < 700);

    // 85% Chance of 1
    bv.Randomize(random, 0.85);
    num_ones = bv.CountOnes();
    CHECK(num_ones > 700);
    CHECK(num_ones < 950);

    // 15% Chance of 1
    bv.Randomize(random, 0.15);
    num_ones = bv.CountOnes();
    CHECK(num_ones > 50);
    CHECK(num_ones < 300);

    // Try randomizing only a portion of the genome.
    uint64_t first_bits = bv.GetUInt64(0);
    bv.Randomize(random, 0.7, 64, 1000);

    CHECK(bv.GetUInt64(0) == first_bits);  // Make sure first bits haven't changed

    num_ones = bv.CountOnes();
    CHECK(num_ones > 500);                 // Expected with new randomization is ~665 ones.
    CHECK(num_ones < 850);

    // Try randomizing using specific numbers of ones.
    bv.ChooseRandom(random, 1);       CHECK(bv.CountOnes() == 1);
    bv.ChooseRandom(random, 12);      CHECK(bv.CountOnes() == 12);
    bv.ChooseRandom(random, 128);     CHECK(bv.CountOnes() == 128);
    bv.ChooseRandom(random, 507);     CHECK(bv.CountOnes() == 507);
    bv.ChooseRandom(random, 999);     CHECK(bv.CountOnes() == 999);

    // Test the probabilistic CHANGE functions.
    bv.Clear();                     CHECK(bv.CountOnes() == 0);   // Set all bits to 0.

    bv.FlipRandom(random, 0.3);     // Expected: 300 ones (from flipping zeros)
    num_ones = bv.CountOnes();      CHECK(num_ones > 230);  CHECK(num_ones < 375);

    bv.FlipRandom(random, 0.3);     // Expected: 420 ones (hit by ONE but not both flips)
    num_ones = bv.CountOnes();      CHECK(num_ones > 345);  CHECK(num_ones < 495);

    bv.SetRandom(random, 0.5);      // Expected: 710 (already on OR newly turned on)
    num_ones = bv.CountOnes();      CHECK(num_ones > 625);  CHECK(num_ones < 775);

    bv.SetRandom(random, 0.8);      // Expected: 942 (already on OR newly turned on)
    num_ones = bv.CountOnes();      CHECK(num_ones > 900);  CHECK(num_ones < 980);

    bv.ClearRandom(random, 0.2);    // Expected 753.6 (20% of those on now off)
    num_ones = bv.CountOnes();      CHECK(num_ones > 675);  CHECK(num_ones < 825);

    bv.FlipRandom(random, 0.5);     // Expected: 500 ones (each bit has a 50% chance of flipping)
    num_ones = bv.CountOnes();      CHECK(num_ones > 425);  CHECK(num_ones < 575);


    // Repeat with fixed-sized changes.
    bv.Clear();                        CHECK(bv.CountOnes() == 0);     // Set all bits to 0.

    bv.FlipRandomCount(random, 123);   // Flip exactly 123 bits to 1.
    num_ones = bv.CountOnes();         CHECK(num_ones == 123);

    bv.FlipRandomCount(random, 877);   // Flip exactly 877 bits; Expected 784.258 ones
    num_ones = bv.CountOnes();         CHECK(num_ones > 700);  CHECK(num_ones < 850);


    bv.SetAll();                       CHECK(bv.CountOnes() == 1000);  // Set all bits to 1.

    bv.ClearRandomCount(random, 123);
    num_ones = bv.CountOnes();         CHECK(num_ones == 877);

    bv.ClearRandomCount(random, 877);  // Clear exactly 877 bits; Expected 107.871 ones
    num_ones = bv.CountOnes();         CHECK(num_ones > 60);  CHECK(num_ones < 175);

    bv.SetRandomCount(random, 500);    // Half of the remaining ones should be set; 553.9355 expected.
    num_ones = bv.CountOnes();         CHECK(num_ones > 485);  CHECK(num_ones < 630);


    bv.Clear();                        CHECK(bv.CountOnes() == 0);     // Set all bits to 0.
    bv.SetRandomCount(random, 567);    // Half of the remaining ones should be set; 607.871 expected.
    num_ones = bv.CountOnes();         CHECK(num_ones == 567);
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
  for (size_t i = 0;   i < 100; i++)  { CHECK(one_counts[i] == 0); }
  for (size_t i = 100; i < 250; i++)  { CHECK(one_counts[i] > 420);  CHECK(one_counts[i] < 580); }
  for (size_t i = 250; i < 400; i++)  { CHECK(one_counts[i] > 190);  CHECK(one_counts[i] < 320); }
  for (size_t i = 400; i < 550; i++)  { CHECK(one_counts[i] > 680);  CHECK(one_counts[i] < 810); }
  for (size_t i = 550; i < 700; i++)  { CHECK(one_counts[i] >  60);  CHECK(one_counts[i] < 150); }
  for (size_t i = 700; i < 850; i++)  { CHECK(one_counts[i] > 950);  CHECK(one_counts[i] < 999); }
  for (size_t i = 850; i < 1000; i++) { CHECK(one_counts[i] == 0); }
}

TEST_CASE("6: Test getting and setting whole chunks of bits", "[bits]") {
  constexpr size_t num_bits = 145;
  constexpr size_t num_bytes = 19;

  emp::BitVector bv(num_bits);
  CHECK(bv.GetSize() == num_bits);
  CHECK(bv.GetNumBytes() == num_bytes);

  // All bytes should start out empty.
  for (size_t i = 0; i < num_bytes; i++) CHECK(bv.GetByte(i) == 0);

  bv.SetByte(2, 11);
  CHECK(bv.GetByte(2) == 11);

  CHECK(bv.GetValue() == 720896.0);

  bv.SetByte(5, 7);
  CHECK(bv.GetByte(0) == 0);
  CHECK(bv.GetByte(1) == 0);
  CHECK(bv.GetByte(2) == 11);
  CHECK(bv.GetByte(3) == 0);
  CHECK(bv.GetByte(4) == 0);
  CHECK(bv.GetByte(5) == 7);
  CHECK(bv.GetByte(6) == 0);
  CHECK(bv.CountOnes() == 6);

  for (size_t i = 0; i < num_bytes; i++) CHECK(bv.GetByte(i) == bv.GetUInt8(i));

  CHECK(bv.GetUInt16(0) == 0);
  CHECK(bv.GetUInt16(1) == 11);
  CHECK(bv.GetUInt16(2) == 1792);
  CHECK(bv.GetUInt16(3) == 0);

  CHECK(bv.GetUInt32(0) == 720896);
  CHECK(bv.GetUInt32(1) == 1792);
  CHECK(bv.GetUInt32(2) == 0);

  CHECK(bv.GetUInt64(0) == 7696582115328);
  CHECK(bv.GetUInt64(1) == 0);

  bv.SetUInt64(0, 12345678901234);
  bv.SetUInt32(2, 2000000);
  bv.SetUInt16(7, 7777);
  bv.SetUInt8(17, 17);

  CHECK(bv.GetUInt64(0) == 12345678901234);
  CHECK(bv.GetUInt32(2) == 2000000);
  CHECK(bv.GetUInt16(7) == 7777);
  CHECK(bv.GetUInt8(17) == 17);

  bv.Clear();
  bv.SetUInt16AtBit(40, 40);

  CHECK(bv.GetUInt16AtBit(40) == 40);

  CHECK(bv.GetUInt8(5) == 40);
  CHECK(bv.GetUInt8AtBit(40) == 40);
  CHECK(bv.GetUInt32AtBit(40) == 40);
  CHECK(bv.GetUInt64AtBit(40) == 40);

  CHECK(bv.GetUInt16AtBit(38) == 160);
  CHECK(bv.GetUInt16AtBit(39) == 80);
  CHECK(bv.GetUInt16AtBit(41) == 20);
  CHECK(bv.GetUInt16AtBit(42) == 10);

  CHECK(bv.GetUInt8AtBit(38) == 160);
  CHECK(bv.GetUInt8AtBit(37) == 64);
  CHECK(bv.GetUInt8AtBit(36) == 128);
  CHECK(bv.GetUInt8AtBit(35) == 0);
}

TEST_CASE("7: Test functions that analyze and manipulate ones", "[bits]") {

  emp::BitVector bv = "0001000100001110";

  CHECK(bv.GetSize() == 16);
  CHECK(bv.CountOnes() == 5);

  // Make sure we can find all of the ones.
  CHECK(bv.FindOne() == 3);
  CHECK(bv.FindOne(4) == 7);
  CHECK(bv.FindOne(5) == 7);
  CHECK(bv.FindOne(6) == 7);
  CHECK(bv.FindOne(7) == 7);
  CHECK(bv.FindOne(8) == 12);
  CHECK(bv.FindOne(13) == 13);
  CHECK(bv.FindOne(14) == 14);
  CHECK(bv.FindOne(15) == -1);

  // Also check that we find the zeroes.
  CHECK(bv.FindZero() == 0);
  CHECK(bv.FindZero(1) == 1);
  CHECK(bv.FindZero(2) == 2);
  CHECK(bv.FindZero(3) == 4);
  CHECK(bv.FindZero(4) == 4);
  CHECK(bv.FindZero(5) == 5);
  CHECK(bv.FindZero(7) == 8);
  CHECK(bv.FindZero(11) == 11);
  CHECK(bv.FindZero(12) == 15);
  CHECK(bv.FindZero(14) == 15);
  CHECK(bv.FindZero(15) == 15);
  CHECK(bv.FindZero(16) == -1);

  // Get all of the ones at once and make sure they're there.
  emp::vector<size_t> ones = bv.GetOnes();
  CHECK(ones.size() == 5);
  CHECK(ones[0] == 3);
  CHECK(ones[1] == 7);
  CHECK(ones[2] == 12);
  CHECK(ones[3] == 13);
  CHECK(ones[4] == 14);

  // Identify the final one.
  CHECK(bv.FindMaxOne() == 14);

  // Try finding the length of the longest segment of ones.
  CHECK(bv.LongestSegmentOnes() == 3);

  // Pop all ones, one at a time.
  CHECK(bv.PopOne() == 3);
  CHECK(bv.PopOne() == 7);
  CHECK(bv.PopOne() == 12);
  CHECK(bv.PopOne() == 13);
  CHECK(bv.PopOne() == 14);
  CHECK(bv.PopOne() == -1);

  CHECK(bv.CountOnes() == 0);
  CHECK(bv.LongestSegmentOnes() == 0);
  CHECK(bv.FindMaxOne() == -1);

  bv.SetAll();                             // 1111111111111111
  CHECK(bv.LongestSegmentOnes() == 16);
  bv[8] = 0;                               // 1111111101111111
  CHECK(bv.LongestSegmentOnes() == 8);
  bv[4] = 0;                               // 1111011101111111
  CHECK(bv.LongestSegmentOnes() == 7);

  // Try again with Find, this time with a random sequence of ones.
  emp::Random random(1);
  bv.Randomize(random);
  size_t count = 0;
  for (int i = bv.FindOne(); i != -1; i = bv.FindOne(i+1)) count++;
  CHECK(count == bv.CountOnes());

}

TEST_CASE("8: Test printing and string functions.", "[bits]") {
  emp::BitVector bv6("000111");

  CHECK(bv6.ToString() == "000111");
  CHECK(bv6.ToBinaryString() == "111000");
  CHECK(bv6.ToIDString() == "3 4 5");
  CHECK(bv6.ToIDString() == "3 4 5");
  CHECK(bv6.ToRangeString() == "3-5");

  emp::BitVector bv64("0001110000000000000100000000000001000110000001000001000100000001");

  CHECK(bv64.ToString()       == "0001110000000000000100000000000001000110000001000001000100000001");
  CHECK(bv64.ToBinaryString() == "1000000010001000001000000110001000000000000010000000000000111000");
  CHECK(bv64.ToIDString() == "3 4 5 19 33 37 38 45 51 55 63");
  CHECK(bv64.ToIDString(",") == "3,4,5,19,33,37,38,45,51,55,63");
  CHECK(bv64.ToRangeString() == "3-5,19,33,37-38,45,51,55,63");

  emp::BitVector bv65("00011110000000000001000000000000010001100000010000010001000000111");

  CHECK(bv65.ToString()       == "00011110000000000001000000000000010001100000010000010001000000111");
  CHECK(bv65.ToBinaryString() == "11100000010001000001000000110001000000000000010000000000001111000");
  CHECK(bv65.ToIDString()     == "3 4 5 6 19 33 37 38 45 51 55 62 63 64");
  CHECK(bv65.ToIDString(",")  == "3,4,5,6,19,33,37,38,45,51,55,62,63,64");
  CHECK(bv65.ToRangeString()  == "3-6,19,33,37-38,45,51,55,62-64");
}

TEST_CASE("9: Test Boolean logic and shifting functions.", "[bits]") {
  const emp::BitVector input1 = "00001111";
  const emp::BitVector input2 = "00110011";
  const emp::BitVector input3 = "01010101";

  // Test *_SELF() Boolean Logic functions.
  emp::BitVector bv(8);    CHECK(bv == emp::BitVector("00000000"));
  bv.NOT_SELF();           CHECK(bv == emp::BitVector("11111111"));
  bv.AND_SELF(input1);     CHECK(bv == emp::BitVector("00001111"));
  bv.AND_SELF(input1);     CHECK(bv == emp::BitVector("00001111"));
  bv.AND_SELF(input2);     CHECK(bv == emp::BitVector("00000011"));
  bv.AND_SELF(input3);     CHECK(bv == emp::BitVector("00000001"));

  bv.OR_SELF(input1);      CHECK(bv == emp::BitVector("00001111"));
  bv.OR_SELF(input1);      CHECK(bv == emp::BitVector("00001111"));
  bv.OR_SELF(input3);      CHECK(bv == emp::BitVector("01011111"));
  bv.OR_SELF(input2);      CHECK(bv == emp::BitVector("01111111"));

  bv.NAND_SELF(input1);    CHECK(bv == emp::BitVector("11110000"));
  bv.NAND_SELF(input1);    CHECK(bv == emp::BitVector("11111111"));
  bv.NAND_SELF(input2);    CHECK(bv == emp::BitVector("11001100"));
  bv.NAND_SELF(input3);    CHECK(bv == emp::BitVector("10111011"));

  bv.NOR_SELF(input1);     CHECK(bv == emp::BitVector("01000000"));
  bv.NOR_SELF(input1);     CHECK(bv == emp::BitVector("10110000"));
  bv.NOR_SELF(input2);     CHECK(bv == emp::BitVector("01001100"));
  bv.NOR_SELF(input3);     CHECK(bv == emp::BitVector("10100010"));

  bv.XOR_SELF(input1);     CHECK(bv == emp::BitVector("10101101"));
  bv.XOR_SELF(input1);     CHECK(bv == emp::BitVector("10100010"));
  bv.XOR_SELF(input2);     CHECK(bv == emp::BitVector("10010001"));
  bv.XOR_SELF(input3);     CHECK(bv == emp::BitVector("11000100"));

  bv.EQU_SELF(input1);     CHECK(bv == emp::BitVector("00110100"));
  bv.EQU_SELF(input1);     CHECK(bv == emp::BitVector("11000100"));
  bv.EQU_SELF(input2);     CHECK(bv == emp::BitVector("00001000"));
  bv.EQU_SELF(input3);     CHECK(bv == emp::BitVector("10100010"));

  bv.NOT_SELF();           CHECK(bv == emp::BitVector("01011101"));

  // Test regular Boolean Logic functions.
  bv.Clear();                            CHECK(bv == emp::BitVector("00000000"));
  emp::BitVector bv1 = bv.NOT();         CHECK(bv1 == emp::BitVector("11111111"));

  bv1 = bv1.AND(input1);                 CHECK(bv1 == emp::BitVector("00001111"));
  emp::BitVector bv2 = bv1.AND(input1);  CHECK(bv2 == emp::BitVector("00001111"));
  emp::BitVector bv3 = bv2.AND(input2);  CHECK(bv3 == emp::BitVector("00000011"));
  emp::BitVector bv4 = bv3.AND(input3);  CHECK(bv4 == emp::BitVector("00000001"));

  bv1 = bv4.OR(input1);      CHECK(bv1 == emp::BitVector("00001111"));
  bv2 = bv1.OR(input1);      CHECK(bv2 == emp::BitVector("00001111"));
  bv3 = bv2.OR(input3);      CHECK(bv3 == emp::BitVector("01011111"));
  bv4 = bv3.OR(input2);      CHECK(bv4 == emp::BitVector("01111111"));

  bv1 = bv4.NAND(input1);    CHECK(bv1 == emp::BitVector("11110000"));
  bv2 = bv1.NAND(input1);    CHECK(bv2 == emp::BitVector("11111111"));
  bv3 = bv2.NAND(input2);    CHECK(bv3 == emp::BitVector("11001100"));
  bv4 = bv3.NAND(input3);    CHECK(bv4 == emp::BitVector("10111011"));

  bv1 = bv4.NOR(input1);     CHECK(bv1 == emp::BitVector("01000000"));
  bv2 = bv1.NOR(input1);     CHECK(bv2 == emp::BitVector("10110000"));
  bv3 = bv2.NOR(input2);     CHECK(bv3 == emp::BitVector("01001100"));
  bv4 = bv3.NOR(input3);     CHECK(bv4 == emp::BitVector("10100010"));

  bv1 = bv4.XOR(input1);     CHECK(bv1 == emp::BitVector("10101101"));
  bv2 = bv1.XOR(input1);     CHECK(bv2 == emp::BitVector("10100010"));
  bv3 = bv2.XOR(input2);     CHECK(bv3 == emp::BitVector("10010001"));
  bv4 = bv3.XOR(input3);     CHECK(bv4 == emp::BitVector("11000100"));

  bv1 = bv4.EQU(input1);     CHECK(bv1 == emp::BitVector("00110100"));
  bv2 = bv1.EQU(input1);     CHECK(bv2 == emp::BitVector("11000100"));
  bv3 = bv2.EQU(input2);     CHECK(bv3 == emp::BitVector("00001000"));
  bv4 = bv3.EQU(input3);     CHECK(bv4 == emp::BitVector("10100010"));

  bv = bv4.NOT();            CHECK(bv == emp::BitVector("01011101"));


  // Test Boolean Logic operators.
  bv.Clear();               CHECK(bv == emp::BitVector("00000000"));
  bv1 = ~bv;                CHECK(bv1 == emp::BitVector("11111111"));

  bv1 = bv1 & input1;       CHECK(bv1 == emp::BitVector("00001111"));
  bv2 = bv1 & input1;       CHECK(bv2 == emp::BitVector("00001111"));
  bv3 = bv2 & input2;       CHECK(bv3 == emp::BitVector("00000011"));
  bv4 = bv3 & input3;       CHECK(bv4 == emp::BitVector("00000001"));

  bv1 = bv4 | input1;       CHECK(bv1 == emp::BitVector("00001111"));
  bv2 = bv1 | input1;       CHECK(bv2 == emp::BitVector("00001111"));
  bv3 = bv2 | input3;       CHECK(bv3 == emp::BitVector("01011111"));
  bv4 = bv3 | input2;       CHECK(bv4 == emp::BitVector("01111111"));

  bv1 = ~(bv4 & input1);    CHECK(bv1 == emp::BitVector("11110000"));
  bv2 = ~(bv1 & input1);    CHECK(bv2 == emp::BitVector("11111111"));
  bv3 = ~(bv2 & input2);    CHECK(bv3 == emp::BitVector("11001100"));
  bv4 = ~(bv3 & input3);    CHECK(bv4 == emp::BitVector("10111011"));

  bv1 = ~(bv4 | input1);    CHECK(bv1 == emp::BitVector("01000000"));
  bv2 = ~(bv1 | input1);    CHECK(bv2 == emp::BitVector("10110000"));
  bv3 = ~(bv2 | input2);    CHECK(bv3 == emp::BitVector("01001100"));
  bv4 = ~(bv3 | input3);    CHECK(bv4 == emp::BitVector("10100010"));

  bv1 = bv4 ^ input1;       CHECK(bv1 == emp::BitVector("10101101"));
  bv2 = bv1 ^ input1;       CHECK(bv2 == emp::BitVector("10100010"));
  bv3 = bv2 ^ input2;       CHECK(bv3 == emp::BitVector("10010001"));
  bv4 = bv3 ^ input3;       CHECK(bv4 == emp::BitVector("11000100"));

  bv1 = ~(bv4 ^ input1);    CHECK(bv1 == emp::BitVector("00110100"));
  bv2 = ~(bv1 ^ input1);    CHECK(bv2 == emp::BitVector("11000100"));
  bv3 = ~(bv2 ^ input2);    CHECK(bv3 == emp::BitVector("00001000"));
  bv4 = ~(bv3 ^ input3);    CHECK(bv4 == emp::BitVector("10100010"));

  bv = ~bv4;                CHECK(bv == emp::BitVector("01011101"));


  // Test COMPOUND Boolean Logic operators.
  bv = "11111111";    CHECK(bv == emp::BitVector("11111111"));

  bv &= input1;       CHECK(bv == emp::BitVector("00001111"));
  bv &= input1;       CHECK(bv == emp::BitVector("00001111"));
  bv &= input2;       CHECK(bv == emp::BitVector("00000011"));
  bv &= input3;       CHECK(bv == emp::BitVector("00000001"));

  bv |= input1;       CHECK(bv == emp::BitVector("00001111"));
  bv |= input1;       CHECK(bv == emp::BitVector("00001111"));
  bv |= input3;       CHECK(bv == emp::BitVector("01011111"));
  bv |= input2;       CHECK(bv == emp::BitVector("01111111"));

  bv ^= input1;       CHECK(bv == emp::BitVector("01110000"));
  bv ^= input1;       CHECK(bv == emp::BitVector("01111111"));
  bv ^= input2;       CHECK(bv == emp::BitVector("01001100"));
  bv ^= input3;       CHECK(bv == emp::BitVector("00011001"));

  // Shifting tests.
  CHECK( (bv << 1) == emp::BitVector("00001100"));
  CHECK( (bv << 2) == emp::BitVector("00000110"));
  CHECK( (bv << 3) == emp::BitVector("00000011"));
  CHECK( (bv << 4) == emp::BitVector("00000001"));

  CHECK( (bv >> 1) == emp::BitVector("00110010"));
  CHECK( (bv >> 2) == emp::BitVector("01100100"));
  CHECK( (bv >> 3) == emp::BitVector("11001000"));
  CHECK( (bv >> 4) == emp::BitVector("10010000"));

  // Now some tests with bitvectors longer than one field.
  const emp::BitVector bvl80 =
    "00110111000101110001011100010111000101110001011100010111000101110001011100010111";
  CHECK( bvl80.GetSize() == 80 );
  CHECK( bvl80.CountOnes() == 41 );
  CHECK( bvl80 << 1 ==
           emp::BitVector("00011011100010111000101110001011100010111000101110001011100010111000101110001011")
         );
  CHECK( bvl80 << 2 ==
           emp::BitVector("00001101110001011100010111000101110001011100010111000101110001011100010111000101")
         );
  CHECK( bvl80 << 63 ==
           emp::BitVector("00000000000000000000000000000000000000000000000000000000000000000110111000101110")
         );
  CHECK( bvl80 << 64 ==
           emp::BitVector("00000000000000000000000000000000000000000000000000000000000000000011011100010111")
         );
  CHECK( bvl80 << 65 ==
           emp::BitVector("00000000000000000000000000000000000000000000000000000000000000000001101110001011")
         );

  CHECK( bvl80 >> 1 ==
           emp::BitVector("01101110001011100010111000101110001011100010111000101110001011100010111000101110")
         );
  CHECK( bvl80 >> 2 ==
           emp::BitVector("11011100010111000101110001011100010111000101110001011100010111000101110001011100")
         );
  CHECK( bvl80 >> 63 ==
           emp::BitVector("10001011100010111000000000000000000000000000000000000000000000000000000000000000")
         );
  CHECK( bvl80 >> 64 ==
           emp::BitVector("00010111000101110000000000000000000000000000000000000000000000000000000000000000")
         );
  CHECK( bvl80 >> 65 ==
           emp::BitVector("00101110001011100000000000000000000000000000000000000000000000000000000000000000")
         );
}

TEST_CASE("10: Test functions that trigger size changes", "[bits]") {
  emp::BitVector bv(10);
  CHECK(bv.GetSize() == 10);
  CHECK(bv.CountOnes() == 0);
  CHECK(bv.CountZeros() == 10);

  bv.Resize(1000);
  CHECK(bv.GetSize() == 1000);
  CHECK(bv.CountOnes() == 0);
  CHECK(bv.CountZeros() == 1000);

  bv.SetAll();
  CHECK(bv.GetSize() == 1000);
  CHECK(bv.CountOnes() == 1000);
  CHECK(bv.CountZeros() == 0);

  for (size_t i = 0; i < 100; i++) {
    bv.Resize(1000+i+1);
    CHECK(bv.GetSize() == 1000+i+1);
    CHECK(bv.CountOnes() == 1000);
    bv[1000+i] = 1;
    CHECK(bv.CountOnes() == 1001);
    bv[1000+i] = 0;
    CHECK(bv.CountOnes() == 1000);
  }
  bv.Resize(1000);
  CHECK(bv.GetSize() == 1000);
  CHECK(bv.CountOnes() == 1000);

  emp::Random random(1);
  bv.Randomize(random);
  CHECK(bv.CountOnes() == bv.CountOnes_Sparse());
  size_t num_ones = bv.CountOnes();
  size_t num_zeros = bv.CountZeros();
  CHECK(num_ones > 425);
  CHECK(num_zeros > 425);
  CHECK(num_ones + num_zeros == 1000);

  while (bv.GetSize()) {
    if (bv.PopBack()) num_ones--;
    else num_zeros--;
  }

  CHECK(num_ones == 0);
  CHECK(num_zeros == 0);
  CHECK(bv.CountOnes() == 0);
  CHECK(bv.CountZeros() == 0);

  // Size is now zero.  Raise it back up to one and make sure it initializes to zero ones correctly.
  bv.Resize(1);
  CHECK(bv.GetSize() == 1);
  CHECK(bv.CountOnes() == 0);
  bv.SetRange(0,1);
  CHECK(bv.GetSize() == 1);
  CHECK(bv.CountOnes() == 1);

  bv.Resize(0);
  // Push (100*2=) 200 bits with intensive checking.
  for (size_t i = 0; i < 100; i++) {
    CHECK(bv.GetSize() == i*2);
    CHECK(bv.CountOnes() == i);
    bv.PushBack(0);
    CHECK(bv.GetSize() == i*2+1);
    CHECK(bv.CountOnes() == i);
    bv.PushBack(1);
    CHECK(bv.GetSize() == i*2+2);
    CHECK(bv.CountOnes() == i+1);
  }

  // Push another (400*2=) 800 bits with checking afterward.
  for (size_t i = 100; i < 500; i++) {
    bv.PushBack(0);
    bv.PushBack(1);
  }

  CHECK(bv.GetSize() == 1000);
  CHECK(bv.CountOnes() == 500);
  CHECK(bv.CountZeros() == 500);

  bv.Insert(250, 0, 500); // Insert 500 zeros at index 250.

  // bv should now be:
  // [0-250): 0,1 repeating
  // [250-750): all 0
  // [750-1500): 0,1 repeating

  CHECK(bv.GetSize() == 1500);
  CHECK(bv.CountOnes() == 500);
  CHECK(bv.CountZeros() == 1000);
  CHECK(bv.CountOnes(250,750) == 0);
  CHECK(bv.CountOnes(0,750) == 125);
  for (size_t i = 250; i < 750; i++) CHECK(bv[i] == 0);

  bv.Insert(1250, 1, 500); // Insert 500 ones at index 1250 (250 before end).

  // bv should now be:
  // [0-250): 0,1 repeating
  // [250-750): all 0
  // [750-1250): 0,1 repeating
  // [1250-1750): all 1
  // [1750-2000): 0,1 repeating

  CHECK(bv.GetSize() == 2000);
  CHECK(bv.CountOnes() == 1000);
  CHECK(bv.CountZeros() == 1000);
  CHECK(bv.CountOnes(250,750) == 0);
  CHECK(bv.CountOnes(1250,1750) == 500);
  for (size_t i = 1250; i < 1750; i++) CHECK(bv[i] == 1);

  bv.Delete(500, 550);   // Delete 250 zeros and 300 pairs of zeros and ones.

  // bv should now be:
  // [0-250): 0,1 repeating      (125 ones)
  // [250-500): all 0
  // [500-700): 0,1 repeating    (100 ones)
  // [700-1200): all 1           (500 ones)
  // [1200-1450): 0,1 repeating  (125 ones)

  CHECK(bv.GetSize() == 1450);
  CHECK(bv.CountOnes() == 850);
  CHECK(bv.CountOnes(0,250) == 125);
  CHECK(bv.CountOnes(250,500) == 0);
  CHECK(bv.CountOnes(500,700) == 100);
  CHECK(bv.CountOnes(700,1200) == 500);
  CHECK(bv.CountOnes(1200,1450) == 125);
}

TEST_CASE("11: Test BitVector", "[bits]")
{

  // Constructor
  emp::BitVector bv(10);

  // Get Size
  CHECK( (bv.GetSize() == 10) );
  CHECK( (bv.size() == 10) );

  // Set & Get
  bv.Set(0);
  CHECK(bv.Get(0));
  bv.Set(1, false);
  CHECK(!bv.Get(1));

  // Assignment operator
  emp::BitVector bv1(10);
  bv1 = bv;
  CHECK(bv1 == bv);
  CHECK(bv1.Get(0));
  emp::BitVector bv20(20);
  emp::BitVector bv30(30);
  bv20.Set(1);
  CHECK(bv20.Get(1));
  bv20 = bv;
  CHECK(bv20 == bv);
  CHECK(bv20.size()==bv.size());
  CHECK(!bv20.Get(1));
  bv20 = bv30;
  CHECK(!bv20.Get(1));
  CHECK(bv20 == bv30);

  // Resize
  bv1.Set(9);
  bv1.resize(8);
  CHECK( (bv1.GetSize() == 8) );
  CHECK( (bv1.GetByte(0) == 1) );
  bv1.resize(128);
  CHECK( (bv1.GetSize() == 128) );
  CHECK( (bv1.GetByte(1) == 0) );

  // Comparison operators
  CHECK((bv1 != bv));
  bv1.Resize(10);
  CHECK((bv1 == bv));
  CHECK((bv1 >= bv));
  CHECK((bv1 <= bv));
  bv.Set(1);
  CHECK((bv > bv1));
  CHECK((bv >= bv1));

  // Set & Get Byte
  emp::BitVector bv2(32);
  bv2.SetByte(0, 128);
  bv2.SetByte(1, 255);
  CHECK((bv2.GetByte(0) == 128));
  CHECK((bv2.GetByte(1) == 255));

  // Count Ones
  CHECK((bv2.CountOnes() == 9));
  CHECK((bv2.CountOnes_Sparse() == 9));
  CHECK((bv2.count() == 9));

  // Any All None SetAll Clear
  CHECK(bool(bv2)); // operator bool()
  CHECK(bool(bv2[7])); // bool operator[]
  CHECK(bv2.any());
  CHECK(!bv2.all());
  CHECK(!bv2.none());
  bv2.SetAll();
  CHECK(!bv2.none());
  CHECK(bv2.all());
  bv2.Clear();
  CHECK(bv2.none());
  CHECK(!bv2.all());

  // Prints
  std::stringstream ss;
  emp::BitVector bv3(8);
  bv3.SetByte(0,255);
  bv3.Print(ss);
  CHECK((ss.str() == "11111111"));
  ss.str(std::string()); // clear ss

  ss << bv3;
  CHECK((ss.str() == "11111111"));
  ss.str(std::string()); // clear ss

  bv3.SetByte(0,130);
  bv3.PrintOneIDs(ss);
  CHECK((ss.str() == "1 7"));
  ss.str(std::string()); // clear ss

  bv3.PrintArray(ss);
  CHECK((ss.str() == "01000001"));
  ss.str(std::string()); // clear ss

  // Find & Pop Bit
  bv3.SetByte(0,74);
  CHECK((bv3.PopOne() == 1));
  CHECK((bv3.CountOnes() == 2));
  CHECK((bv3.GetByte(0) == 72));
  CHECK((bv3.FindOne() == 3));
  CHECK((bv3.FindOne(4) == 6));
  bv3.PopOne();
  bv3.PopOne();
  CHECK((bv3.FindOne() == -1));
  CHECK((bv3.FindOne(2) == -1));
  CHECK((bv3.PopOne() == -1));

  // Get Ones
  emp::vector<size_t> ones = bv3.GetOnes();
  CHECK((ones.size() == 0));
  bv3.SetByte(0,10);
  ones = bv3.GetOnes();
  CHECK((ones[0] == 1));
  CHECK((ones[1] == 3));

  // Larger BitVector
  emp::BitVector bv4(96);
  bv4.SetByte(1,1);
  bv4.PrintFields(ss);
  CHECK(ss.str() == "00000000000000000000000000000000 0000000000000000000000000000000000000000000000000000000100000000");

  // test single set.
  bv4[62] = 1;
  ss.str(std::string()); bv4.PrintFields(ss); // Clear & resend bits.
  CHECK(ss.str() == "00000000000000000000000000000000 0100000000000000000000000000000000000000000000000000000100000000");
  // test toggle of range (across boundary)
  bv4.Toggle(61, 70);
  ss.str(std::string()); bv4.PrintFields(ss); // Clear & resend bits.
  CHECK(ss.str() == "00000000000000000000000000111111 1010000000000000000000000000000000000000000000000000000100000000");
  // test clearing a range in a single field.
  bv4.Clear(65, 69);
  ss.str(std::string()); bv4.PrintFields(ss); // Clear & resend bits.
  CHECK(ss.str() == "00000000000000000000000000100001 1010000000000000000000000000000000000000000000000000000100000000");
  // test toggling a larger range
  bv4.Toggle(55, 75);
  ss.str(std::string()); bv4.PrintFields(ss); // Clear & resend bits.
  CHECK(ss.str() == "00000000000000000000011111011110 0101111110000000000000000000000000000000000000000000000100000000");
  // test clearing a field across boundaries
  bv4.Clear(56, 74);
  ss.str(std::string()); bv4.PrintFields(ss); // Clear & resend bits.
  CHECK(ss.str() == "00000000000000000000010000000000 0000000010000000000000000000000000000000000000000000000100000000");

  // Even longer bit vector (to test operations that span multiple fields)
  bv4.Resize(300);
  ss.str(std::string()); bv4.PrintFields(ss); // Clear & resend bits.
  CHECK(ss.str() == "00000000000000000000000000000000000000000000 0000000000000000000000000000000000000000000000000000000000000000 0000000000000000000000000000000000000000000000000000000000000000 0000000000000000000000000000000000000000000000000000010000000000 0000000010000000000000000000000000000000000000000000000100000000");
  // test setting a range that spans three fields.
  bv4.SetRange(100, 250);
  ss.str(std::string()); bv4.PrintFields(ss); // Clear & resend bits.
  CHECK(ss.str() == "00000000000000000000000000000000000000000000 0000001111111111111111111111111111111111111111111111111111111111 1111111111111111111111111111111111111111111111111111111111111111 1111111111111111111111111111000000000000000000000000010000000000 0000000010000000000000000000000000000000000000000000000100000000");
  // test clearing a full field.
  bv4.Clear(128,192);
  ss.str(std::string()); bv4.PrintFields(ss); // Clear & resend bits.
  CHECK(ss.str() == "00000000000000000000000000000000000000000000 0000001111111111111111111111111111111111111111111111111111111111 0000000000000000000000000000000000000000000000000000000000000000 1111111111111111111111111111000000000000000000000000010000000000 0000000010000000000000000000000000000000000000000000000100000000");
  // test clearing slightly more than a full field.
  bv4.Clear(127,193);
  ss.str(std::string()); bv4.PrintFields(ss); // Clear & resend bits.
  CHECK(ss.str() == "00000000000000000000000000000000000000000000 0000001111111111111111111111111111111111111111111111111111111110 0000000000000000000000000000000000000000000000000000000000000000 0111111111111111111111111111000000000000000000000000010000000000 0000000010000000000000000000000000000000000000000000000100000000");
  // test setting a full field.
  bv4.SetRange(128,192);
  ss.str(std::string()); bv4.PrintFields(ss); // Clear & resend bits.
  CHECK(ss.str() == "00000000000000000000000000000000000000000000 0000001111111111111111111111111111111111111111111111111111111110 1111111111111111111111111111111111111111111111111111111111111111 0111111111111111111111111111000000000000000000000000010000000000 0000000010000000000000000000000000000000000000000000000100000000");
  ss.str(std::string()); // clear ss


  // Logic operators
  emp::BitVector bv5(8);
  bv5.SetByte(0,28);
  CHECK((bv3.CountOnes() == 8-((~bv3).CountOnes())));
  CHECK(((bv3 & bv5).GetByte(0) == 8));
  CHECK(((bv3 | bv5).GetByte(0) == 30));
  CHECK(((bv3 ^ bv5).GetByte(0) == 22));
  CHECK(((bv3 << 2).GetByte(0) == 40));
  CHECK(((bv5 >> 2).GetByte(0) == 7));

  // Compound operators
  bv5 &= bv3;
  CHECK((bv5.GetByte(0) == 8));
  bv5 |= bv3;
  CHECK((bv5.GetByte(0) == 10));
  bv5 ^= bv3;
  CHECK((bv5.GetByte(0) == 0));
  bv3 >>= 2;
  CHECK((bv3.GetByte(0) == 2));
  bv3 <<= 4;
  CHECK((bv3.GetByte(0) == 32));

  // Hash
  emp::BitVector bv_a(2);
  bv_a.Set(0);
  emp::BitVector bv_b(2);
  bv_b.Set(0);
  CHECK(bv_a.Hash() == bv_b.Hash());
  bv_b.Set(0, false);
  CHECK(bv_a.Hash() != bv_b.Hash());
  bv_b.Set(0, true);

  // EQU_SELF
  CHECK(bv_a.EQU_SELF(bv_b).all());
  // bv_a = 01, bv_b = 01, ~(01 ^ 01) = 11
  CHECK(bv_a.GetByte(0) == 3);
  CHECK(bv_b.GetByte(0) == 1);
  CHECK(!(bv_a.EQU_SELF(bv_b).all()));
  // bv_a = 11, bv_b = 01, ~(11 ^ 01) = 01
  CHECK(bv_a.GetByte(0) == 1);
  CHECK(bv_b.GetByte(0) == 1);

  // NAND SELF
  // bv_a = 01, bv_b = 01, ~(01 & 01) = 10
  CHECK(bv_a.NAND_SELF(bv_b) == ~bv_b);
  CHECK(bv_a.GetByte(0) == 2);

  // NOR SELF
  // bv_a = 10, bv_b = 01, ~(10 | 01) = 00
  CHECK(bv_a.NOR_SELF(bv_b).none());
  CHECK(bv_a.GetByte(0) == 0);

  // NOT SELF
  CHECK(bv_a.NOT_SELF().all());

  // EQU
  emp::BitVector bv_c(3);
  bv_c.SetByte(0,2);
  emp::BitVector bv_d(3);
  bv_d.SetByte(0,2);
  CHECK(bv_c.EQU(bv_d).all());
  CHECK(bv_c.GetByte(0) == 2);

  // NAND
  CHECK(bv_c.NAND(bv_d) == ~bv_c);
  CHECK(bv_c.GetByte(0) == 2);

  // NOR
  CHECK(bv_c.NOR(bv_d) == ~bv_c);
  CHECK(bv_c.GetByte(0) == 2);

  // Bit proxy compound assignment operators
  // AND
  // bv_c = 010
  bv_c[0] &= 1;
  CHECK(bv_c[0] == 0);
  CHECK(bv_c[1] == 1);
  bv_c[1] &= 0;
  CHECK(bv_c[1] == 0);
  // OR
  // bv_d = 010
  bv_d[1] |= 0;
  CHECK(bv_d[1] == 1);
  bv_d[0] |= 1;
  CHECK(bv_d[0] == 1);
  bv_d[2] |= 0;
  CHECK(bv_d[2] == 0);
  // XOR
  // bv_c = 000
  bv_c[0] ^= 1;
  CHECK(bv_c[0] == 1);
  bv_c[0] ^= 1;
  CHECK(bv_c[0] == 0);
  //PLUS
  // bv_d = 011
  bv_d[2] += 1;
  CHECK(bv_d[2] == 1);
  // MINUS
  // bv_d = 111
  bv_d[1] -= 1;
  CHECK(bv_d[1] == 0);
  // TIMES
  //bv_d = 101
  bv_d[2] *= 1;
  CHECK(bv_d[2] == 1);
  bv_d[0] *= 0;
  CHECK(bv_d[0] == 0);
  // DIV
  // bv_c = 000
  bv_c[0] /= 1;
  CHECK(bv_c[0] == 0);

  // GetUInt SetUInt
  emp::BitVector bv_e(5);
  bv_e.SetUInt(0, 16);
  CHECK(bv_e.GetUInt(0) == 16);

  // Shift Left
  emp::BitVector bv_f(128);
  bv_f.SetAll();
  CHECK(bv_f.all());
  bv_f <<= 127;
  CHECK(bv_f.count() == 1);
  bv_f <<= 1;
  CHECK(bv_f.none());

  // #ifdef TDEBUG
  // CHECK(emp::assert_last_fail == 0);
  // #endif

}

TEST_CASE("12: Test MaskHigh, MaskLow", "[bits]") {

  // #ifdef TDEBUG
  // CHECK(emp::assert_last_fail == 0);
  // #endif

  // Test MaskHigh, MaskLow
  emp::BitVector a(0);
  a.Insert(0,true, 7);
  CHECK(a.Get(0));
  CHECK(a.Get(1));
  CHECK(a.Get(2));

  // #ifdef TDEBUG
  // CHECK(emp::assert_last_fail == 0);
  // #endif

}

TEST_CASE("13: Test PopBack, PushBack, Insert, Delete", "[bits]") {

  // #ifdef TDEBUG
  // CHECK(emp::assert_last_fail == 0);
  // #endif

  // Pop Back and Push Back
  emp::BitVector bv_g(0);    // Empty BitVector
  bv_g.PushBack(true);       // 1
  bv_g.PushBack(true);       // 11
  bv_g.PushBack(false);      // 110
  CHECK(bv_g.Get(0));
  CHECK(bv_g.Get(1));
  CHECK(!bv_g.PopBack());  // 11
  CHECK(bv_g.size() == 2);

  // Insert and Delete
  bv_g.Insert(1, true);      // 111
  CHECK(bv_g.Get(0));
  CHECK(bv_g.Get(1));
  CHECK(bv_g.Get(2));
  CHECK(bv_g.size() == 3);

  bv_g.Insert(1, true);      // 1111
  CHECK(bv_g.Get(3));
  CHECK(bv_g.Get(2));
  CHECK(bv_g.Get(1));
  CHECK(bv_g.Get(0));
  CHECK(bv_g.size() == 4);

  bv_g.Insert(1, false);     // 10111
  CHECK(bv_g.Get(0));
  CHECK(!bv_g.Get(1));
  CHECK(bv_g.Get(2));
  CHECK(bv_g.Get(3));

  bv_g.Delete(0);            // 0111
  CHECK(bv_g.size() == 4);
  CHECK(!bv_g.Get(0));
  bv_g.Delete(1, 2);         // 01
  CHECK(bv_g.size() == 2);
  CHECK(bv_g.Get(1));

  // #ifdef TDEBUG
  // CHECK(emp::assert_last_fail == 0);
  // #endif

}

TEST_CASE("14: Another Test BitVector", "[bits]") {

  // #ifdef TDEBUG
  // CHECK(emp::assert_last_fail == 0);
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
    CHECK((shift_vector.CountOnes() == 1) == (i <= 71));
  }

  bv10 = (bv80 >> 70);

  // Test arbitrary bit retrieval of UInts
  bv80[65] = 1;
  CHECK(bv80.GetUInt32(2) == 130);
  CHECK(bv80.GetUIntAtBit(64) == 130);
//  CHECK(bv80.GetValueAtBit<5>(64) == 2);

  // #ifdef TDEBUG
  // CHECK(emp::assert_last_fail == 0);
  // #endif

}

TEST_CASE("15: Test range of BitVector constructors.", "[bits]")
// test list initializer
{
  emp::BitVector bs_empty{0,0,0};
  emp::BitVector bs_first{1,0,0};
  emp::BitVector bs_last{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
  emp::BitVector bs_two{0,0,1,0,0,0,0,0,0,0,1,0,0};
  emp::BitVector bs_full{1,1,1,1,1,1,1,1};

  CHECK(bs_empty.CountOnes() == 0);
  CHECK(bs_first.CountOnes() == 1);
  CHECK(bs_last.CountOnes() == 1);
  CHECK(bs_two.CountOnes() == 2);
  CHECK(bs_full.CountOnes() == 8);

  CHECK(bs_empty.GetSize() == 3);
  CHECK(bs_first.GetSize() == 3);
  CHECK(bs_last.GetSize() == 25);
  CHECK(bs_two.GetSize() == 13);
  CHECK(bs_full.GetSize() == 8);
}

// TEST_CASE("BitVector padding bits protected", "[bits]") {
// #ifdef TDEBUG

//   CHECK(emp::assert_last_fail == 0);

//   for (size_t i = 1; i < 32; ++i) {

//     emp::BitVector vec(i);
//     CHECK(emp::assert_last_fail == 0);
//     vec.SetUInt(0, std::numeric_limits<uint32_t>::max());
//     CHECK(emp::assert_last_fail);
//     emp::assert_clear();

//   }

//   CHECK(emp::assert_last_fail == 0);

//   emp::BitVector vec(32);
//   vec.SetUInt(0, std::numeric_limits<uint32_t>::max());

//   CHECK(emp::assert_last_fail == 0);

// #endif
// }

TEST_CASE("16: BitVector regression test for #277", "[bits]") {
  // #ifdef TDEBUG
  // CHECK(emp::assert_last_fail == 0);
  // #endif

  emp::BitVector vec1(4);
  emp::BitVector vec2(4);

  for (size_t i = 0; i < 4; ++i) CHECK(!vec1[i]);
  for (size_t i = 0; i < 4; ++i) CHECK(!vec2[i]);
  vec1.SetUInt(0, 15);
  vec2.SetUIntAtBit(0, 15);
  for (size_t i = 0; i < 4; ++i) CHECK(vec1[i]);
  for (size_t i = 0; i < 4; ++i) CHECK(vec2[i]);

  // #ifdef TDEBUG
  // CHECK(emp::assert_last_fail == 0);
  // #endif
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  ---------------------------  BitArray tests ---------------------------  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


template <size_t... VALS> struct TestBitArrayConstruct;

template <size_t VAL1, size_t... VALS>
struct TestBitArrayConstruct<VAL1, VALS...> {
  static void Run() {
    emp::BitArray<VAL1> bit_array;
    CHECK( bit_array.GetSize() == VAL1 );
    CHECK( bit_array.CountOnes() == 0 );
    for (size_t i = 0; i < VAL1; i++) bit_array[i] = true;
    CHECK( bit_array.CountOnes() == VAL1 );

    TestBitArrayConstruct<VALS...>::Run();
  }
};

// Base case for constructors...
template <>
struct TestBitArrayConstruct<> {
  static void Run(){}
};


TEST_CASE("17: Test BitArray Constructors", "[bits]"){
  // Create a size 50 bit vector, default to all zeros.
  emp::BitArray<50> ba1;
  CHECK( ba1.GetSize() == 50 );
  CHECK( ba1.CountOnes() == 0 );
  CHECK( (~ba1).CountOnes() == 50 );

  // Create a size 1000 BitArray, default to all ones.
  emp::BitArray<1000> ba2(true);
  CHECK( ba2.GetSize() == 1000 );
  CHECK( ba2.CountOnes() == 1000 );

  // Try a range of BitArray sizes, from 1 to 200.
  TestBitArrayConstruct<1,2,7,8,9,15,16,17,31,32,33,63,64,65,127,128,129,191,192,193,200>::Run();

  // Build a relatively large BitArray.
  emp::BitArray<1000000> ba4;
  for (size_t i = 0; i < ba4.GetSize(); i += 100) ba4[i].Toggle();
  CHECK( ba4.CountOnes() == 10000 );

  // Try out the copy constructor.
  emp::BitArray<1000000> ba5(ba4);
  CHECK( ba5.GetSize() == 1000000 );
  CHECK( ba5.CountOnes() == 10000 );

  // Construct from std::bitset.
  std::bitset<6> bit_set;
  bit_set[1] = 1;   bit_set[2] = 1;   bit_set[4] = 1;
  emp::BitArray<6> ba7(bit_set);
  CHECK( ba7.GetSize() == 6 );
  CHECK( ba7.CountOnes() == 3 );

  // Construct from string.
  std::string bit_string = "10011001010000011101";
  emp::BitArray<20> ba8(bit_string);
  CHECK( ba8.GetSize() == 20 );
  CHECK( ba8.CountOnes() == 9 );

  // Some random BitArrays
  emp::Random random;
  emp::BitArray<1000> ba9(random);            // 50/50 chance for each bit.
  const size_t ba9_ones = ba9.CountOnes();
  CHECK( ba9_ones >= 400 );
  CHECK( ba9_ones <= 600 );

  emp::BitArray<1000> ba10(random, 0.8);      // 80% chance of ones.
  const size_t ba10_ones = ba10.CountOnes();
  CHECK( ba10_ones >= 750 );
  CHECK( ba10_ones <= 850 );

  emp::BitArray<1000> ba11(random, 117);      // Exactly 117 ones, randomly placed.
  const size_t ba11_ones = ba11.CountOnes();
  CHECK( ba11_ones == 117 );

  emp::BitArray<13> ba12({1,0,0,0,1,1,1,0,0,0,1,1,1}); // Construct with initializer list.
  CHECK( ba12.GetSize() == 13 );
  CHECK( ba12.CountOnes() == 7 );
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

    CHECK( ba.CountOnes() == ba.GetSize()/2 );

    // Try copying in from an std::bitset.
    std::bitset<VAL1> bit_set;
    size_t num_ones = 0;
    if constexpr (VAL1 > 1)   { bit_set[1] = 1; num_ones++; }
    if constexpr (VAL1 > 22)  { bit_set[22] = 1; num_ones++; }
    if constexpr (VAL1 > 444) { bit_set[444] = 1; num_ones++; }

    ba2 = bit_set;  // Copy in an std::bitset.

    CHECK( ba2.GetSize() == VAL1 );
    CHECK( ba2.CountOnes() == num_ones );

    // Try copying from an std::string
    std::string bit_string = "100110010100000111011001100101000001110110011001010000011101";
    while (bit_string.size() < VAL1) bit_string += bit_string;
    bit_string.resize(VAL1);

    num_ones = 0;
    for (char x : bit_string) if (x == '1') num_ones++;

    ba2 = bit_string;

    CHECK( ba2.GetSize() == VAL1 );
    CHECK( ba2.CountOnes() == num_ones );

    TestBVAssign<VALS...>::Run();
  }
};

// Base case for constructors...
template<> struct TestBVAssign<> { static void Run(){} };

TEST_CASE("18: Test BitArray Assignments", "[bits]"){
  // Try a range of BitArray sizes, from 1 to 200.
  TestBVAssign<1,2,7,8,9,15,16,17,31,32,33,63,64,65,127,128,129,191,192,193,200,1023,1024,1025,1000000>::Run();
}


TEST_CASE("19: Test Simple BitArray Accessors", "[bits]"){
  emp::BitArray<1>  ba1(true);
  emp::BitArray<8>  ba8( "10001101" );
  emp::BitArray<32> ba32( "10001101100011011000110110001101" );
  emp::BitArray<64> ba64( "1000110110001101100000011000110000001101100000000000110110001101" );
  emp::BitArray<75> ba75( "010001011100010111110000011110100011111000001110100000111110010011111000011" );

  emp::Random random;
  emp::BitArray<1000> ba1k(random, 0.75);

  // Make sure all sizes are correct.
  CHECK( ba1.GetSize() == 1 );
  CHECK( ba8.GetSize() == 8 );
  CHECK( ba32.GetSize() == 32 );
  CHECK( ba64.GetSize() == 64 );
  CHECK( ba75.GetSize() == 75 );
  CHECK( ba1k.GetSize() == 1000 );

  // Check byte counts (should always round up!)
  CHECK( ba1.GetNumBytes() == 1 );     // round up!
  CHECK( ba8.GetNumBytes() == 1 );
  CHECK( ba32.GetNumBytes() == 4 );
  CHECK( ba64.GetNumBytes() == 8 );
  CHECK( ba75.GetNumBytes() == 10 );   // round up!
  CHECK( ba1k.GetNumBytes() == 125 );

  // How many states can be represented in each size of BitArray?
  CHECK( ba1.GetNumStates() == 2.0 );
  CHECK( ba8.GetNumStates() == 256.0 );
  CHECK( ba32.GetNumStates() == 4294967296.0 );
  CHECK( ba64.GetNumStates() >= 18446744073709551610.0 );
  CHECK( ba64.GetNumStates() <= 18446744073709551720.0 );
  CHECK( ba75.GetNumStates() >= 37778931862957161709560.0 );
  CHECK( ba75.GetNumStates() <= 37778931862957161709570.0 );
  CHECK( ba1k.GetNumStates() == emp::Pow2(1000) );

  // Test Get()
  CHECK( ba1.Get(0) == 1 );
  CHECK( ba8.Get(0) == 1 );
  CHECK( ba8.Get(4) == 1 );
  CHECK( ba8.Get(6) == 0 );
  CHECK( ba8.Get(7) == 1 );
  CHECK( ba75.Get(0) == 0 );
  CHECK( ba75.Get(1) == 1 );
  CHECK( ba75.Get(72) == 0 );
  CHECK( ba75.Get(73) == 1 );
  CHECK( ba75.Get(74) == 1 );

  // Test Has() (including out of range)
  CHECK( ba1.Has(0) == true );
  CHECK( ba1.Has(1) == false );
  CHECK( ba1.Has(1000000) == false );

  CHECK( ba8.Has(0) == true );
  CHECK( ba8.Has(4) == true );
  CHECK( ba8.Has(6) == false );
  CHECK( ba8.Has(7) == true );
  CHECK( ba8.Has(8) == false );

  CHECK( ba75.Has(0) == false );
  CHECK( ba75.Has(1) == true );
  CHECK( ba75.Has(72) == false );
  CHECK( ba75.Has(73) == true );
  CHECK( ba75.Has(74) == true );
  CHECK( ba75.Has(75) == false );
  CHECK( ba75.Has(79) == false );
  CHECK( ba75.Has(1000000) == false );

  // Test Set(), changing in most (but not all) cases.
  ba1.Set(0, 0);
  CHECK( ba1.Get(0) == 0 );
  ba8.Set(0, 1);                // Already a 1!
  CHECK( ba8.Get(0) == 1 );
  ba8.Set(4, 0);
  CHECK( ba8.Get(4) == 0 );
  ba8.Set(6, 1);
  CHECK( ba8.Get(6) == 1 );
  ba8.Set(7, 0);
  CHECK( ba8.Get(7) == 0 );
  ba75.Set(0, 0);               // Already a 0!
  CHECK( ba75.Get(0) == 0 );
  ba75.Set(1, 0);
  CHECK( ba75.Get(1) == 0 );
  ba75.Set(72);                 // No second arg!
  CHECK( ba75.Get(72) == 1 );
  ba75.Set(73);                 // No second arg AND already a 1!
  CHECK( ba75.Get(73) == 1 );
  ba75.Set(74, 0);
  CHECK( ba75.Get(74) == 0 );
}

TEST_CASE("20: Test BitArray Set*, Clear* and Toggle* Accessors", "[bits]") {
  // Now try range-based accessors on a single bit.
  emp::BitArray<1> ba1(false);  CHECK( ba1[0] == false );   CHECK( ba1.CountOnes() == 0 );
  ba1.Set(0);           CHECK( ba1[0] == true );    CHECK( ba1.CountOnes() == 1 );
  ba1.Clear(0);         CHECK( ba1[0] == false );   CHECK( ba1.CountOnes() == 0 );
  ba1.Toggle(0);        CHECK( ba1[0] == true );    CHECK( ba1.CountOnes() == 1 );
  ba1.Clear();          CHECK( ba1[0] == false );   CHECK( ba1.CountOnes() == 0 );
  ba1.SetAll();         CHECK( ba1[0] == true );    CHECK( ba1.CountOnes() == 1 );
  ba1.Toggle();         CHECK( ba1[0] == false );   CHECK( ba1.CountOnes() == 0 );
  ba1.SetRange(0,1);    CHECK( ba1[0] == true );    CHECK( ba1.CountOnes() == 1 );
  ba1.Clear(0,1);       CHECK( ba1[0] == false );   CHECK( ba1.CountOnes() == 0 );
  ba1.Toggle(0,1);      CHECK( ba1[0] == true );    CHECK( ba1.CountOnes() == 1 );
  ba1.Set(0, false);    CHECK( ba1[0] == false );   CHECK( ba1.CountOnes() == 0 );
  ba1.SetRange(0,0);    CHECK( ba1[0] == false );   CHECK( ba1.CountOnes() == 0 );
  ba1.SetRange(1,1);    CHECK( ba1[0] == false );   CHECK( ba1.CountOnes() == 0 );

  // Test when a full byte is used.
  emp::BitArray<8> ba8( "10001101" );   CHECK(ba8.GetValue() == 177.0);  // 10110001
  ba8.Set(2);           CHECK(ba8.GetValue() == 181.0);  // 10110101
  ba8.Set(0, 0);        CHECK(ba8.GetValue() == 180.0);  // 10110100
  ba8.SetRange(1, 4);   CHECK(ba8.GetValue() == 190.0);  // 10111110
  ba8.SetAll();         CHECK(ba8.GetValue() == 255.0);  // 11111111
  ba8.Clear(3);         CHECK(ba8.GetValue() == 247.0);  // 11110111
  ba8.Clear(5,5);       CHECK(ba8.GetValue() == 247.0);  // 11110111
  ba8.Clear(5,7);       CHECK(ba8.GetValue() == 151.0);  // 10010111
  ba8.Clear();          CHECK(ba8.GetValue() ==   0.0);  // 00000000
  ba8.Toggle(4);        CHECK(ba8.GetValue() ==  16.0);  // 00010000
  ba8.Toggle(4,6);      CHECK(ba8.GetValue() ==  32.0);  // 00100000
  ba8.Toggle(0,3);      CHECK(ba8.GetValue() ==  39.0);  // 00100111
  ba8.Toggle(7,8);      CHECK(ba8.GetValue() == 167.0);  // 10100111
  ba8.Toggle();         CHECK(ba8.GetValue() ==  88.0);  // 01011000

  // Test a full field.
  constexpr double ALL_64 = (double) ((uint64_t) -1);
  emp::BitArray<64> ba64( "11011000110110001101" );
  CHECK(ba64.GetValue() == 727835.0);
  ba64.Set(6);          CHECK(ba64.GetValue() == 727899.0);        // ...0 010110001101101011011
  ba64.Set(0, 0);       CHECK(ba64.GetValue() == 727898.0);        // ...0 010110001101101011010
  ba64.SetRange(4, 9);  CHECK(ba64.GetValue() == 728058.0);        // ...0 010110001101111111010
  ba64.SetAll();        CHECK(ba64.GetValue() == ALL_64);          // ...1 111111111111111111111
  ba64.Clear(2);        CHECK(ba64.GetValue() == ALL_64 - 4);      // ...1 111111111111111111011
  ba64.Clear(5,5);      CHECK(ba64.GetValue() == ALL_64 - 4);      // ...1 111111111111111111011
  ba64.Clear(5,7);      CHECK(ba64.GetValue() == ALL_64 - 100);    // ...1 111111111111110011011
  ba64.Clear();         CHECK(ba64.GetValue() == 0.0);             // ...0 000000000000000000000
  ba64.Toggle(19);      CHECK(ba64.GetValue() == emp::Pow2(19));   // ...0 010000000000000000000
  ba64.Toggle(15,20);   CHECK(ba64.GetValue() == 491520.0);        // ...0 001111000000000000000
  ba64.Toggle();        CHECK(ba64.GetValue() == ALL_64-491520.0); // ...1 110000111111111111111
  ba64.Toggle(0,64);    CHECK(ba64.GetValue() == 491520.0);        // ...0 001111000000000000000


  emp::BitArray<75> ba75( "010001011100010111110000011110100011111000001110100000111110010011111000011" );

  // Test a full + partial field.
  constexpr double ALL_88 = ((double) ((uint64_t) -1)) * emp::Pow2(24);
  emp::BitArray<88> ba88( "11011000110110001101" ); CHECK(ba88.GetValue() == 727835.0);
  CHECK(ba88.GetValue() == 727835.0);                              // ...0 010110001101100011011

  // Start with same tests as last time...
  ba88.Set(6);          CHECK(ba88.GetValue() == 727899.0);        // ...0 010110001101101011011
  ba88.Set(0, 0);       CHECK(ba88.GetValue() == 727898.0);        // ...0 010110001101101011010
  ba88.SetRange(4, 9);  CHECK(ba88.GetValue() == 728058.0);        // ...0 010110001101111111010
  ba88.SetAll();        CHECK(ba88.GetValue() == ALL_88);          // ...1 111111111111111111111
  ba88.Clear(2);        CHECK(ba88.GetValue() == ALL_88 - 4);      // ...1 111111111111111111011
  ba88.Clear(5,5);      CHECK(ba88.GetValue() == ALL_88 - 4);      // ...1 111111111111111111011
  ba88.Clear(5,7);      CHECK(ba88.GetValue() == ALL_88 - 100);    // ...1 111111111111110011011
  ba88.Clear();         CHECK(ba88.GetValue() == 0.0);             // ...0 000000000000000000000
  ba88.Toggle(19);      CHECK(ba88.GetValue() == emp::Pow2(19));   // ...0 010000000000000000000
  ba88.Toggle(15,20);   CHECK(ba88.GetValue() == 491520.0);        // ...0 001111000000000000000
  ba88.Toggle();        CHECK(ba88.GetValue() == ALL_88-491520.0); // ...1 110000111111111111111
  ba88.Toggle(0,88);    CHECK(ba88.GetValue() == 491520.0);        // ...0 001111000000000000000

  ba88 <<= 20;          CHECK(ba88.CountOnes() == 4);   // four ones, moved to bits 35-39
  ba88 <<= 27;          CHECK(ba88.CountOnes() == 4);   // four ones, moved to bits 62-65
  ba88 <<= 22;          CHECK(ba88.CountOnes() == 4);   // four ones, moved to bits 84-87
  ba88 <<= 1;           CHECK(ba88.CountOnes() == 3);   // three ones left, moved to bits 85-87
  ba88 <<= 2;           CHECK(ba88.CountOnes() == 1);   // one one left, at bit 87
  ba88 >>= 30;          CHECK(ba88.CountOnes() == 1);   // one one left, now at bit 57
  ba88.Toggle(50,80);   CHECK(ba88.CountOnes() == 29);  // Toggling 30 bits, only one was on.
  ba88.Clear(52,78);    CHECK(ba88.CountOnes() == 4);   // Leave two 1s on each side of range
  ba88.SetRange(64,66); CHECK(ba88.CountOnes() == 6);   // Set two more 1s, just into 2nd field.

  // A larger BitArray with lots of random tests.
  emp::Random random;

  emp::BitArray<40> ba40(random, 0.65);
  size_t num_ones = ba40.CountOnes();  CHECK(num_ones >= 14); CHECK(num_ones < 39);
  ba40.Toggle();                       CHECK(ba40.CountOnes() == 40 - num_ones);

  emp::BitArray<1000> ba1k(random, 0.65);
  num_ones = ba1k.CountOnes();         CHECK(num_ones > 560); CHECK(num_ones < 760);
  ba1k.Toggle();                       CHECK(ba1k.CountOnes() == 1000 - num_ones);

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

  CHECK(ba_empty.Any() == false);
  CHECK(ba_mixed.Any() == true);
  CHECK(ba_full.Any() == true);

  CHECK(ba_empty.All() == false);
  CHECK(ba_mixed.All() == false);
  CHECK(ba_full.All() == true);

  CHECK(ba_empty.None() == true);
  CHECK(ba_mixed.None() == false);
  CHECK(ba_full.None() == false);
}


TEST_CASE("21: Test Randomize() and variants", "[bits]") {
  emp::Random random;
  emp::BitArray<1000> ba;

  CHECK(ba.None() == true);

  // Do all of the random tests 10 times.
  for (size_t test_num = 0; test_num < 10; test_num++) {
    ba.Randomize(random);
    size_t num_ones = ba.CountOnes();
    CHECK(num_ones > 300);
    CHECK(num_ones < 700);

    // 85% Chance of 1
    ba.Randomize(random, 0.85);
    num_ones = ba.CountOnes();
    CHECK(num_ones > 700);
    CHECK(num_ones < 950);

    // 15% Chance of 1
    ba.Randomize(random, 0.15);
    num_ones = ba.CountOnes();
    CHECK(num_ones > 50);
    CHECK(num_ones < 300);

    // Try randomizing only a portion of the genome.
    uint64_t first_bits = ba.GetUInt64(0);
    ba.Randomize(random, 0.7, 64, 1000);

    CHECK(ba.GetUInt64(0) == first_bits);  // Make sure first bits haven't changed

    num_ones = ba.CountOnes();
    CHECK(num_ones > 500);                 // Expected with new randomization is ~665 ones.
    CHECK(num_ones < 850);

    // Try randomizing using specific numbers of ones.
    ba.ChooseRandom(random, 1);       CHECK(ba.CountOnes() == 1);
    ba.ChooseRandom(random, 12);      CHECK(ba.CountOnes() == 12);
    ba.ChooseRandom(random, 128);     CHECK(ba.CountOnes() == 128);
    ba.ChooseRandom(random, 507);     CHECK(ba.CountOnes() == 507);
    ba.ChooseRandom(random, 999);     CHECK(ba.CountOnes() == 999);

    // Test the probabilistic CHANGE functions.
    ba.Clear();                     CHECK(ba.CountOnes() == 0);   // Set all bits to 0.

    ba.FlipRandom(random, 0.3);     // Expected: 300 ones (from flipping zeros)
    num_ones = ba.CountOnes();      CHECK(num_ones > 230);  CHECK(num_ones < 375);

    ba.FlipRandom(random, 0.3);     // Expected: 420 ones (hit by ONE but not both flips)
    num_ones = ba.CountOnes();      CHECK(num_ones > 345);  CHECK(num_ones < 495);

    ba.SetRandom(random, 0.5);      // Expected: 710 (already on OR newly turned on)
    num_ones = ba.CountOnes();      CHECK(num_ones > 625);  CHECK(num_ones < 775);

    ba.SetRandom(random, 0.8);      // Expected: 942 (already on OR newly turned on)
    num_ones = ba.CountOnes();      CHECK(num_ones > 900);  CHECK(num_ones < 980);

    ba.ClearRandom(random, 0.2);    // Expected 753.6 (20% of those on now off)
    num_ones = ba.CountOnes();      CHECK(num_ones > 675);  CHECK(num_ones < 825);

    ba.FlipRandom(random, 0.5);     // Expected: 500 ones (each bit has a 50% chance of flipping)
    num_ones = ba.CountOnes();      CHECK(num_ones > 425);  CHECK(num_ones < 575);


    // Repeat with fixed-sized changes.
    ba.Clear();                        CHECK(ba.CountOnes() == 0);     // Set all bits to 0.

    ba.FlipRandomCount(random, 123);   // Flip exactly 123 bits to 1.
    num_ones = ba.CountOnes();         CHECK(num_ones == 123);

    ba.FlipRandomCount(random, 877);   // Flip exactly 877 bits; Expected 784.258 ones
    num_ones = ba.CountOnes();         CHECK(num_ones > 700);  CHECK(num_ones < 850);


    ba.SetAll();                       CHECK(ba.CountOnes() == 1000);  // Set all bits to 1.

    ba.ClearRandomCount(random, 123);
    num_ones = ba.CountOnes();         CHECK(num_ones == 877);

    ba.ClearRandomCount(random, 877);  // Clear exactly 877 bits; Expected 107.871 ones
    num_ones = ba.CountOnes();         CHECK(num_ones > 60);  CHECK(num_ones < 175);

    ba.SetRandomCount(random, 500);    // Half of the remaining ones should be set; 553.9355 expected.
    num_ones = ba.CountOnes();         CHECK(num_ones > 485);  CHECK(num_ones < 630);


    ba.Clear();                        CHECK(ba.CountOnes() == 0);     // Set all bits to 0.
    ba.SetRandomCount(random, 567);    // Half of the remaining ones should be set; 607.871 expected.
    num_ones = ba.CountOnes();         CHECK(num_ones == 567);
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
  for (size_t i = 0;   i < 100; i++)  { CHECK(one_counts[i] == 0); }
  for (size_t i = 100; i < 250; i++)  { CHECK(one_counts[i] > 420);  CHECK(one_counts[i] < 580); }
  for (size_t i = 250; i < 400; i++)  { CHECK(one_counts[i] > 190);  CHECK(one_counts[i] < 320); }
  for (size_t i = 400; i < 550; i++)  { CHECK(one_counts[i] > 680);  CHECK(one_counts[i] < 810); }
  for (size_t i = 550; i < 700; i++)  { CHECK(one_counts[i] >  60);  CHECK(one_counts[i] < 150); }
  for (size_t i = 700; i < 850; i++)  { CHECK(one_counts[i] > 950);  CHECK(one_counts[i] < 999); }
  for (size_t i = 850; i < 1000; i++) { CHECK(one_counts[i] == 0); }
}

TEST_CASE("22: Test getting and setting whole chunks of bits", "[bits]") {
  constexpr size_t num_bits = 145;
  constexpr size_t num_bytes = 19;

  emp::BitArray<num_bits> ba;
  CHECK(ba.GetSize() == num_bits);
  CHECK(ba.GetNumBytes() == num_bytes);

  // All bytes should start out empty.
  for (size_t i = 0; i < num_bytes; i++) CHECK(ba.GetByte(i) == 0);

  ba.SetByte(2, 11);
  CHECK(ba.GetByte(2) == 11);

  CHECK(ba.GetValue() == 720896.0);

  ba.SetByte(5, 7);
  CHECK(ba.GetByte(0) == 0);
  CHECK(ba.GetByte(1) == 0);
  CHECK(ba.GetByte(2) == 11);
  CHECK(ba.GetByte(3) == 0);
  CHECK(ba.GetByte(4) == 0);
  CHECK(ba.GetByte(5) == 7);
  CHECK(ba.GetByte(6) == 0);
  CHECK(ba.CountOnes() == 6);

  for (size_t i = 0; i < num_bytes; i++) CHECK(ba.GetByte(i) == ba.GetUInt8(i));

  CHECK(ba.GetUInt16(0) == 0);
  CHECK(ba.GetUInt16(1) == 11);
  CHECK(ba.GetUInt16(2) == 1792);
  CHECK(ba.GetUInt16(3) == 0);

  CHECK(ba.GetUInt32(0) == 720896);
  CHECK(ba.GetUInt32(1) == 1792);
  CHECK(ba.GetUInt32(2) == 0);

  CHECK(ba.GetUInt64(0) == 7696582115328);
  CHECK(ba.GetUInt64(1) == 0);

  ba.SetUInt64(0, 12345678901234);
  ba.SetUInt32(2, 2000000);
  ba.SetUInt16(7, 7777);
  ba.SetUInt8(17, 17);

  CHECK(ba.GetUInt64(0) == 12345678901234);
  CHECK(ba.GetUInt32(2) == 2000000);
  CHECK(ba.GetUInt16(7) == 7777);
  CHECK(ba.GetUInt8(17) == 17);

  ba.Clear();
  ba.SetUInt16AtBit(40, 40);

  CHECK(ba.GetUInt16AtBit(40) == 40);

  CHECK(ba.GetUInt8(5) == 40);
  CHECK(ba.GetUInt8AtBit(40) == 40);
  CHECK(ba.GetUInt32AtBit(40) == 40);
  CHECK(ba.GetUInt64AtBit(40) == 40);

  CHECK(ba.GetUInt16AtBit(38) == 160);
  CHECK(ba.GetUInt16AtBit(39) == 80);
  CHECK(ba.GetUInt16AtBit(41) == 20);
  CHECK(ba.GetUInt16AtBit(42) == 10);

  CHECK(ba.GetUInt8AtBit(38) == 160);
  CHECK(ba.GetUInt8AtBit(37) == 64);
  CHECK(ba.GetUInt8AtBit(36) == 128);
  CHECK(ba.GetUInt8AtBit(35) == 0);
}

TEST_CASE("23: Test functions that analyze and manipulate ones", "[bits]") {

  emp::BitArray<16> ba = "0001000100001110";

  CHECK(ba.GetSize() == 16);
  CHECK(ba.CountOnes() == 5);

  // Make sure we can find all of the ones.
  CHECK(ba.FindOne() == 3);
  CHECK(ba.FindOne(4) == 7);
  CHECK(ba.FindOne(5) == 7);
  CHECK(ba.FindOne(6) == 7);
  CHECK(ba.FindOne(7) == 7);
  CHECK(ba.FindOne(8) == 12);
  CHECK(ba.FindOne(13) == 13);
  CHECK(ba.FindOne(14) == 14);
  CHECK(ba.FindOne(15) == -1);

  // Get all of the ones at once and make sure they're there.
  emp::vector<size_t> ones = ba.GetOnes();
  CHECK(ones.size() == 5);
  CHECK(ones[0] == 3);
  CHECK(ones[1] == 7);
  CHECK(ones[2] == 12);
  CHECK(ones[3] == 13);
  CHECK(ones[4] == 14);

  // Make sure that the ones can be identified in ranges correctly.
  auto ranges = ba.GetRanges();
  CHECK(ranges.size() == 3);
  CHECK(ranges[0] == emp::Range<size_t>(3,3));
  CHECK(ranges[1] == emp::Range<size_t>(7,7));
  CHECK(ranges[2] == emp::Range<size_t>(12,14));

  // Try finding the length of the longest segment of ones.
  CHECK(ba.LongestSegmentOnes() == 3);

  // Identify the final one.
  CHECK(ba.FindMaxOne() == 14);

  // Pop all ones, one at a time.
  CHECK(ba.PopOne() == 3);
  CHECK(ba.PopOne() == 7);
  CHECK(ba.PopOne() == 12);
  CHECK(ba.PopOne() == 13);
  CHECK(ba.PopOne() == 14);
  CHECK(ba.PopOne() == -1);

  CHECK(ba.CountOnes() == 0);
  CHECK(ba.LongestSegmentOnes() == 0);
  CHECK(ba.FindMaxOne() == -1);


  ba.SetAll();                             // 1111111111111111
  CHECK(ba.LongestSegmentOnes() == 16);
  ba[8] = 0;                               // 1111111101111111
  CHECK(ba.LongestSegmentOnes() == 8);
  ba[4] = 0;                               // 1111011101111111
  CHECK(ba.LongestSegmentOnes() == 7);

  // Try getting ranges of one values from this BitArray.
  ranges = ba.GetRanges();
  CHECK(ranges.size() == 3);
  CHECK(ranges[0] == emp::Range<size_t>(0,3));
  CHECK(ranges[1] == emp::Range<size_t>(5,7));
  CHECK(ranges[2] == emp::Range<size_t>(9,15));

  // Try again with Find, this time with a random sequence of ones.
  emp::Random random;
  ba.Randomize(random);
  size_t count = 0;
  for (int i = ba.FindOne(); i != -1; i = ba.FindOne(i+1)) count++;
  CHECK(count == ba.CountOnes());

}

TEST_CASE("24: Test printing and string functions.", "[bits]") {
  emp::BitArray<6> ba6("000111");

  CHECK(ba6.ToString() == "000111");
  CHECK(ba6.ToBinaryString() == "111000");
  CHECK(ba6.ToIDString() == "3 4 5");
  CHECK(ba6.ToIDString() == "3 4 5");
  CHECK(ba6.ToRangeString() == "3-5");

  emp::BitArray<64> ba64("0001110000000000000100000000000001000110000001000001000100000001");

  CHECK(ba64.ToString()       == "0001110000000000000100000000000001000110000001000001000100000001");
  CHECK(ba64.ToBinaryString() == "1000000010001000001000000110001000000000000010000000000000111000");
  CHECK(ba64.ToIDString() == "3 4 5 19 33 37 38 45 51 55 63");
  CHECK(ba64.ToIDString(",") == "3,4,5,19,33,37,38,45,51,55,63");
  CHECK(ba64.ToRangeString() == "3-5,19,33,37-38,45,51,55,63");

  emp::BitArray<65> ba65("00011110000000000001000000000000010001100000010000010001000000111");

  CHECK(ba65.ToString()       == "00011110000000000001000000000000010001100000010000010001000000111");
  CHECK(ba65.ToBinaryString() == "11100000010001000001000000110001000000000000010000000000001111000");
  CHECK(ba65.ToIDString()     == "3 4 5 6 19 33 37 38 45 51 55 62 63 64");
  CHECK(ba65.ToIDString(",")  == "3,4,5,6,19,33,37,38,45,51,55,62,63,64");
  CHECK(ba65.ToRangeString()  == "3-6,19,33,37-38,45,51,55,62-64");
}

TEST_CASE("25: Test Boolean logic and shifting functions.", "[bits]") {
  const emp::BitArray<8> input1 = "00001111";
  const emp::BitArray<8> input2 = "00110011";
  const emp::BitArray<8> input3 = "01010101";

  // Test *_SELF() Boolean Logic functions.
  emp::BitArray<8> ba;       CHECK(ba == emp::BitArray<8>("00000000"));
  ba.NOT_SELF();           CHECK(ba == emp::BitArray<8>("11111111"));
  ba.AND_SELF(input1);     CHECK(ba == emp::BitArray<8>("00001111"));
  ba.AND_SELF(input1);     CHECK(ba == emp::BitArray<8>("00001111"));
  ba.AND_SELF(input2);     CHECK(ba == emp::BitArray<8>("00000011"));
  ba.AND_SELF(input3);     CHECK(ba == emp::BitArray<8>("00000001"));

  ba.OR_SELF(input1);      CHECK(ba == emp::BitArray<8>("00001111"));
  ba.OR_SELF(input1);      CHECK(ba == emp::BitArray<8>("00001111"));
  ba.OR_SELF(input3);      CHECK(ba == emp::BitArray<8>("01011111"));
  ba.OR_SELF(input2);      CHECK(ba == emp::BitArray<8>("01111111"));

  ba.NAND_SELF(input1);    CHECK(ba == emp::BitArray<8>("11110000"));
  ba.NAND_SELF(input1);    CHECK(ba == emp::BitArray<8>("11111111"));
  ba.NAND_SELF(input2);    CHECK(ba == emp::BitArray<8>("11001100"));
  ba.NAND_SELF(input3);    CHECK(ba == emp::BitArray<8>("10111011"));

  ba.NOR_SELF(input1);     CHECK(ba == emp::BitArray<8>("01000000"));
  ba.NOR_SELF(input1);     CHECK(ba == emp::BitArray<8>("10110000"));
  ba.NOR_SELF(input2);     CHECK(ba == emp::BitArray<8>("01001100"));
  ba.NOR_SELF(input3);     CHECK(ba == emp::BitArray<8>("10100010"));

  ba.XOR_SELF(input1);     CHECK(ba == emp::BitArray<8>("10101101"));
  ba.XOR_SELF(input1);     CHECK(ba == emp::BitArray<8>("10100010"));
  ba.XOR_SELF(input2);     CHECK(ba == emp::BitArray<8>("10010001"));
  ba.XOR_SELF(input3);     CHECK(ba == emp::BitArray<8>("11000100"));

  ba.EQU_SELF(input1);     CHECK(ba == emp::BitArray<8>("00110100"));
  ba.EQU_SELF(input1);     CHECK(ba == emp::BitArray<8>("11000100"));
  ba.EQU_SELF(input2);     CHECK(ba == emp::BitArray<8>("00001000"));
  ba.EQU_SELF(input3);     CHECK(ba == emp::BitArray<8>("10100010"));

  ba.NOT_SELF();           CHECK(ba == emp::BitArray<8>("01011101"));

  // Test regular Boolean Logic functions.
  ba.Clear();                            CHECK(ba == emp::BitArray<8>("00000000"));
  emp::BitArray<8> ba1 = ba.NOT();         CHECK(ba1 == emp::BitArray<8>("11111111"));

  ba1 = ba1.AND(input1);                 CHECK(ba1 == emp::BitArray<8>("00001111"));
  emp::BitArray<8> ba2 = ba1.AND(input1);  CHECK(ba2 == emp::BitArray<8>("00001111"));
  emp::BitArray<8> ba3 = ba2.AND(input2);  CHECK(ba3 == emp::BitArray<8>("00000011"));
  emp::BitArray<8> ba4 = ba3.AND(input3);  CHECK(ba4 == emp::BitArray<8>("00000001"));

  ba1 = ba4.OR(input1);      CHECK(ba1 == emp::BitArray<8>("00001111"));
  ba2 = ba1.OR(input1);      CHECK(ba2 == emp::BitArray<8>("00001111"));
  ba3 = ba2.OR(input3);      CHECK(ba3 == emp::BitArray<8>("01011111"));
  ba4 = ba3.OR(input2);      CHECK(ba4 == emp::BitArray<8>("01111111"));

  ba1 = ba4.NAND(input1);    CHECK(ba1 == emp::BitArray<8>("11110000"));
  ba2 = ba1.NAND(input1);    CHECK(ba2 == emp::BitArray<8>("11111111"));
  ba3 = ba2.NAND(input2);    CHECK(ba3 == emp::BitArray<8>("11001100"));
  ba4 = ba3.NAND(input3);    CHECK(ba4 == emp::BitArray<8>("10111011"));

  ba1 = ba4.NOR(input1);     CHECK(ba1 == emp::BitArray<8>("01000000"));
  ba2 = ba1.NOR(input1);     CHECK(ba2 == emp::BitArray<8>("10110000"));
  ba3 = ba2.NOR(input2);     CHECK(ba3 == emp::BitArray<8>("01001100"));
  ba4 = ba3.NOR(input3);     CHECK(ba4 == emp::BitArray<8>("10100010"));

  ba1 = ba4.XOR(input1);     CHECK(ba1 == emp::BitArray<8>("10101101"));
  ba2 = ba1.XOR(input1);     CHECK(ba2 == emp::BitArray<8>("10100010"));
  ba3 = ba2.XOR(input2);     CHECK(ba3 == emp::BitArray<8>("10010001"));
  ba4 = ba3.XOR(input3);     CHECK(ba4 == emp::BitArray<8>("11000100"));

  ba1 = ba4.EQU(input1);     CHECK(ba1 == emp::BitArray<8>("00110100"));
  ba2 = ba1.EQU(input1);     CHECK(ba2 == emp::BitArray<8>("11000100"));
  ba3 = ba2.EQU(input2);     CHECK(ba3 == emp::BitArray<8>("00001000"));
  ba4 = ba3.EQU(input3);     CHECK(ba4 == emp::BitArray<8>("10100010"));

  ba = ba4.NOT();            CHECK(ba == emp::BitArray<8>("01011101"));


  // Test Boolean Logic operators.
  ba.Clear();               CHECK(ba == emp::BitArray<8>("00000000"));
  ba1 = ~ba;                CHECK(ba1 == emp::BitArray<8>("11111111"));

  ba1 = ba1 & input1;       CHECK(ba1 == emp::BitArray<8>("00001111"));
  ba2 = ba1 & input1;       CHECK(ba2 == emp::BitArray<8>("00001111"));
  ba3 = ba2 & input2;       CHECK(ba3 == emp::BitArray<8>("00000011"));
  ba4 = ba3 & input3;       CHECK(ba4 == emp::BitArray<8>("00000001"));

  ba1 = ba4 | input1;       CHECK(ba1 == emp::BitArray<8>("00001111"));
  ba2 = ba1 | input1;       CHECK(ba2 == emp::BitArray<8>("00001111"));
  ba3 = ba2 | input3;       CHECK(ba3 == emp::BitArray<8>("01011111"));
  ba4 = ba3 | input2;       CHECK(ba4 == emp::BitArray<8>("01111111"));

  ba1 = ~(ba4 & input1);    CHECK(ba1 == emp::BitArray<8>("11110000"));
  ba2 = ~(ba1 & input1);    CHECK(ba2 == emp::BitArray<8>("11111111"));
  ba3 = ~(ba2 & input2);    CHECK(ba3 == emp::BitArray<8>("11001100"));
  ba4 = ~(ba3 & input3);    CHECK(ba4 == emp::BitArray<8>("10111011"));

  ba1 = ~(ba4 | input1);    CHECK(ba1 == emp::BitArray<8>("01000000"));
  ba2 = ~(ba1 | input1);    CHECK(ba2 == emp::BitArray<8>("10110000"));
  ba3 = ~(ba2 | input2);    CHECK(ba3 == emp::BitArray<8>("01001100"));
  ba4 = ~(ba3 | input3);    CHECK(ba4 == emp::BitArray<8>("10100010"));

  ba1 = ba4 ^ input1;       CHECK(ba1 == emp::BitArray<8>("10101101"));
  ba2 = ba1 ^ input1;       CHECK(ba2 == emp::BitArray<8>("10100010"));
  ba3 = ba2 ^ input2;       CHECK(ba3 == emp::BitArray<8>("10010001"));
  ba4 = ba3 ^ input3;       CHECK(ba4 == emp::BitArray<8>("11000100"));

  ba1 = ~(ba4 ^ input1);    CHECK(ba1 == emp::BitArray<8>("00110100"));
  ba2 = ~(ba1 ^ input1);    CHECK(ba2 == emp::BitArray<8>("11000100"));
  ba3 = ~(ba2 ^ input2);    CHECK(ba3 == emp::BitArray<8>("00001000"));
  ba4 = ~(ba3 ^ input3);    CHECK(ba4 == emp::BitArray<8>("10100010"));

  ba = ~ba4;                CHECK(ba == emp::BitArray<8>("01011101"));


  // Test COMPOUND Boolean Logic operators.
  ba = "11111111";    CHECK(ba == emp::BitArray<8>("11111111"));

  ba &= input1;       CHECK(ba == emp::BitArray<8>("00001111"));
  ba &= input1;       CHECK(ba == emp::BitArray<8>("00001111"));
  ba &= input2;       CHECK(ba == emp::BitArray<8>("00000011"));
  ba &= input3;       CHECK(ba == emp::BitArray<8>("00000001"));

  ba |= input1;       CHECK(ba == emp::BitArray<8>("00001111"));
  ba |= input1;       CHECK(ba == emp::BitArray<8>("00001111"));
  ba |= input3;       CHECK(ba == emp::BitArray<8>("01011111"));
  ba |= input2;       CHECK(ba == emp::BitArray<8>("01111111"));

  ba ^= input1;       CHECK(ba == emp::BitArray<8>("01110000"));
  ba ^= input1;       CHECK(ba == emp::BitArray<8>("01111111"));
  ba ^= input2;       CHECK(ba == emp::BitArray<8>("01001100"));
  ba ^= input3;       CHECK(ba == emp::BitArray<8>("00011001"));

  // Now some tests with BitArrays longer than one field.
  const emp::BitArray<80> bal80 =
    "00110111000101110001011100010111000101110001011100010111000101110001011100010111";
  CHECK( bal80.GetSize() == 80 );
  CHECK( bal80.CountOnes() == 41 );
  CHECK( (bal80 << 1) ==
           emp::BitArray<80>("00011011100010111000101110001011100010111000101110001011100010111000101110001011")
         );
  CHECK( (bal80 << 2) ==
           emp::BitArray<80>("00001101110001011100010111000101110001011100010111000101110001011100010111000101")
         );
  CHECK( (bal80 << 63) ==
           emp::BitArray<80>("00000000000000000000000000000000000000000000000000000000000000000110111000101110")
         );
  CHECK( (bal80 << 64) ==
           emp::BitArray<80>("00000000000000000000000000000000000000000000000000000000000000000011011100010111")
         );
  CHECK( (bal80 << 65) ==
           emp::BitArray<80>("00000000000000000000000000000000000000000000000000000000000000000001101110001011")
         );

  CHECK( (bal80 >> 1) ==
           emp::BitArray<80>("01101110001011100010111000101110001011100010111000101110001011100010111000101110")
         );
  CHECK( (bal80 >> 2) ==
           emp::BitArray<80>("11011100010111000101110001011100010111000101110001011100010111000101110001011100")
         );
  CHECK( (bal80 >> 63) ==
           emp::BitArray<80>("10001011100010111000000000000000000000000000000000000000000000000000000000000000")
         );
  CHECK( (bal80 >> 64) ==
           emp::BitArray<80>("00010111000101110000000000000000000000000000000000000000000000000000000000000000")
         );
  CHECK( (bal80 >> 65) ==
           emp::BitArray<80>("00101110001011100000000000000000000000000000000000000000000000000000000000000000")
         );
}



/// Ensures that
/// 1) A == B
/// 2) A and B can be constexpr or non-constexpr.
/// 3) A and B have the same values regardless of constexpr-ness.
#define CONSTEXPR_CHECK_EQ(A, B)       \
  {                                      \
    static_assert(A == B, #A " == " #B); \
    CHECK(A == B);                     \
  }



/**
 * Status booleans (Any, All, None)
 * as well as Clear and SetAll
 */
void test_status(){
  emp::BitArray<10> ba10;
  CHECK(!ba10.any());
  CHECK(ba10.none());
  CHECK(!ba10.all());
  ba10.SetAll();
  CHECK(ba10.all());
  ba10.Clear();
  CHECK(ba10.none());
}

/**
 * GetSize
 */
void test_size(){
  emp::BitArray<42> ba42;
  CHECK(ba42.size() == 42);

  emp::BitArray<35> ba35;
  CHECK(ba35.GetSize() == 35);

  emp::BitArray<1> ba1;
  CHECK(ba1.size() == 1);
}

/**
 * Flip and Toggle
 */
void test_flip(){
  emp::BitArray<2> ba2;  // ba2 = 00
  ba2.flip(0);        // ba2 = 01
  CHECK(ba2[0]);

  emp::BitArray<8> ba8;  // ba8 = 00000000
  ba8.flip(0,4);      // ba8 = 00001111
  CHECK(ba8[0]);
  CHECK(ba8[1]);
  CHECK(ba8[2]);
  CHECK(ba8[3]);
  CHECK(!ba8[4]);

  ba8[0].Toggle();    // ba8 = 00001110
  CHECK(!ba8[0]);

  emp::BitArray<4> ba4;  // ba4 = 0000
  ba4.flip();          // ba4 = 1111
  CHECK(ba4.all());
}

/**
 * FindOne and PopOne
 */
void test_find(){
  emp::BitArray<10> ba10;  // ba10 = 00 00000000
  ba10.flip(3);          // ba10 = 00 00001000
  CHECK(ba10.FindOne() == 3);
  ba10.PopOne();        // ba10 = 00 00000000
  CHECK(ba10.PopOne() == -1);
  ba10.flip(3);
  ba10.flip(1);
  CHECK(ba10.FindOne(2) == 3);
  CHECK(ba10.FindOne(4) == -1);
}

/**
 * GetByte and SetByte
 */
void test_byte(){
  emp::BitArray<10>  ba10;
  ba10.SetByte(0, 10);
  CHECK(ba10.GetByte(0) == 10);

  ba10.flip(0,4);
  CHECK(ba10.GetByte(0) == 5);
  ba10.SetByte(1, 3);
  CHECK(ba10.count() == 4);
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

  const auto my_span = ba.GetBytes();
  for (size_t i = 0; i < Bits / 8; ++i) {
    CHECK(my_span[i] == static_cast<std::byte>(i * 10));
  }
}
// helper function that uses a fold expression to
// unpack the integer sequence of bits to test
// and then call the actual testing function with each as a template arg
template <typename T, T... Bits>
void do_byte_tests(const std::integer_sequence<T, Bits...>& /* sequence */) {
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
  CHECK(ba40.GetByte(4) == 4);

  emp::BitArray<10> ba10;
  ba10.SetByte(0, 10);
  ba10 >>= 2;
  CHECK(ba10.GetByte(0) == 2);
}

/**
 * Count ones
 */
void test_count(){
  emp::BitArray<12> ba12;
  ba12.SetAll();
  CHECK(ba12.count() == 12);
  CHECK(ba12.CountOnes_Sparse() == 12);
  ba12.flip(0,5);
  CHECK(ba12.count() == 7);
}

/**
 * Get ones
 */
void test_get_ones(){
  emp::BitArray<5> ba5;
  ba5.flip(2); // 00100
  emp::vector<size_t> ones = ba5.GetOnes();
  CHECK(ones.size() == 1);
  CHECK(ones[0] == 2);
}


/**
 * Get and Set bits
 */
void test_bit(){
  emp::BitArray<8> ba8;
  ba8.Set(0, 1);       // ba8 = 00000001
  CHECK(ba8.Get(0));

  ba8.Set(7, 1);      // ba8 = 10000001
  ba8.Set(0, 0);      // ba8 = 10000000
  CHECK(!ba8.Get(0));
  CHECK(ba8.Get(7));
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
  CHECK(ba4.GetByte(0) == 2);   // 0010 = 2
  ba4_1.PopOne();               // ba4_1 = 0010
  ba4 ^= ba4_1;                 // ba4 = 0010 ^ 0010 = 0000
  CHECK(ba4.GetByte(0) == 0);  // 0000 = 0
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
  CHECK(ba10_1.count() == 3);  // ba10_1 = 11 00001000
  ba10_1 |= ba10;                // ba10_1 = 11 00001000 | 00 00000010 = 11 00001010
  CHECK(ba10_1.GetByte(0) == 10);
  CHECK(ba10_1.GetByte(1) == 3);
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
  CHECK(ba8_1.GetByte(0) == 8);
}

/**
 * NAND, NOR, EQU & SELF
 */
void test_more_comparators(){
  // NAND
  emp::BitArray<8> ba8_1;
  emp::BitArray<8> ba8_2;
  ba8_1.SetAll();
  CHECK(ba8_1.NAND(ba8_2).All());
  ba8_2.flip(1);
  ba8_1.NAND_SELF(ba8_2);
  CHECK(ba8_1.Any());
  CHECK( !(ba8_1.Get(1)) );

  // NOR
  ba8_1.SetAll();
  ba8_2.Clear();
  CHECK(ba8_1.NOR(ba8_2).None());
  ba8_1.flip(1);
  ba8_1.NOR_SELF(ba8_2);
  CHECK(ba8_1.Get(1));

  // EQU
  ba8_1.Clear();
  ba8_2.SetAll();
  CHECK( (ba8_1.EQU(ba8_2).None()) );
  ba8_2.Clear();
  ba8_2.EQU_SELF(ba8_1);
  CHECK(ba8_2.All());
}

/**
 * Random bitset
 */
void test_random(){
  emp::Random random;
  emp::BitArray<8> ba8(random);
  ba8.Randomize(random, 1.0);
  CHECK(ba8.all());
  ba8.Randomize(random, 0.0);
  CHECK(ba8.none());
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
  CHECK(ba10 == ba10_1);
}

/**
 * Comparators (>=,>,==,!=,<,<=)
 */
void test_comparators(){
  emp::BitArray<10> ba10;
  emp::BitArray<10> ba10_1;
  ba10_1.SetAll();
  CHECK(ba10_1 != ba10);
  CHECK(ba10_1 > ba10);
  ba10.SetAll();
  CHECK(ba10_1 >= ba10);
  CHECK(ba10_1 <= ba10);
  CHECK(ba10_1 == ba10);
  CHECK(!(ba10_1 < ba10));
  ba10.Clear();
  CHECK( (ba10 < ba10_1) );
}

/**
 * Export
 */
void test_export(){
  emp::BitArray<8> ba8;
  ba8.SetAll();
  CHECK(ba8.count() == 8);
  emp::BitArray<10> ba10 = ba8.ExportArray<10>();
  CHECK(ba10.size() == 10);
  CHECK(ba10.GetByte(0) == 255);
  CHECK(ba10.GetByte(1) == 0);
}

/**
 * Import
 */
void test_import(){
  emp::BitArray<8> ba8;
  emp::BitArray<20> ba20;
  ba20[5] = 1;

  ba8.Import(ba20);
  CHECK(ba8[5]);

  emp::BitArray<10> ba10;
  ba10.SetAll();
  ba20.Import(ba10);
  CHECK(ba20.count() == 10);
}



TEST_CASE("26: Test BitArray", "[bits]")
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
      CHECK(source.Get(i) == dest.Get(i));
    }
    for (size_t i = source.GetSize(); i < dest.GetSize(); ++i) {
      CHECK(dest.Get(i) == 0);
    }

    dest.Clear();
    dest = source.template ExportArray<DEST_BITS>();

    for (size_t i = 0; i < std::min(source.GetSize(), dest.GetSize()); ++i) {
      CHECK(source.Get(i) == dest.Get(i));
    }
    for (size_t i = source.GetSize(); i < dest.GetSize(); ++i) {
      CHECK(dest.Get(i) == 0);
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
        CHECK(source.Get(i+from_bit) == dest.Get(i));
      }
      for (size_t i = source.GetSize() - from_bit; i < dest.GetSize(); ++i) {
        CHECK(dest.Get(i) == 0);
      }

      dest.Clear();
      dest = source.template ExportArray<DEST_BITS>(from_bit);

      for (size_t i = 0; i < std::min(source.GetSize() - from_bit, dest.GetSize()); ++i) {
        CHECK(source.Get(i+from_bit) == dest.Get(i));
      }
      for (size_t i = source.GetSize() - from_bit; i < dest.GetSize(); ++i) {
        CHECK(dest.Get(i) == 0);
      }

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
        CHECK(ba.CountOnes() == num_ones);
      }
      CHECK(ba == ba_orig);
    }

    // Try each individual bit set with many possible rotations.
    for (int i = -width - STEP - 1; i <= width + STEP + 1; i += STEP) {
      // for large widths, just do one starting position
      for (int j = 0; j < (width < 200 ? width : 1); ++j) {
        ba.Clear(); ba.Set(j);
        ba.ROTATE_SELF(i);
        CHECK(ba.CountOnes() == 1);
        CHECK(ba.Get(emp::Mod(j-i,width)));

        ba.SetAll(); ba.Set(j, false);
        ba.ROTATE_SELF(i);
        CHECK(ba.CountOnes() == width-1);
        CHECK(!ba.Get(emp::Mod(j-i,width)));

        ba.Randomize(rand); ba.Set(j);
        const size_t c1 = ba.CountOnes();
        ba.ROTATE_SELF(i);
        CHECK(ba.CountOnes() == c1);
        CHECK(ba.Get(emp::Mod(j-i,width)));

        ba.Randomize(rand); ba.Set(j, false);
        const size_t c2 = ba.CountOnes();
        ba.ROTATE_SELF(i);
        CHECK(ba.CountOnes() == c2);
        CHECK(!ba.Get(emp::Mod(j-i,width)));
      }
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

TEST_CASE("27: Another Test BitArray", "[bits]")
{

  // test BitArray GetSize, GetNumBytes
  {
    CHECK(emp::BitArray<2>{}.GetSize() == 2);
    CHECK(emp::BitArray<2>{}.GetNumBytes() == 1);

    CHECK(emp::BitArray<7>{}.GetSize() == 7);
    CHECK(emp::BitArray<7>{}.GetNumBytes() == 1);

    CHECK(emp::BitArray<8>{}.GetSize() == 8);
    CHECK(emp::BitArray<8>{}.GetNumBytes() == 1);

    CHECK(emp::BitArray<9>{}.GetSize() == 9);
    CHECK(emp::BitArray<9>{}.GetNumBytes() == 2);

    CHECK(emp::BitArray<16>{}.GetSize() == 16);
    CHECK(emp::BitArray<16>{}.GetNumBytes() == 2);

    CHECK(emp::BitArray<24>{}.GetSize() == 24);
    CHECK(emp::BitArray<24>{}.GetNumBytes() == 3);
  }

  // test BitArray reverse
  {

    CHECK(emp::BitArray<1>{0}.REVERSE_SELF() == emp::BitArray<1>{0});
    CHECK(emp::BitArray<1>{0}.REVERSE_SELF().CountOnes() == 0);
    CHECK(emp::BitArray<1>{1}.REVERSE_SELF() == emp::BitArray<1>{1});
    CHECK(emp::BitArray<1>{1}.REVERSE_SELF().CountOnes() == 1);

    CHECK(
      (emp::BitArray<2>{1,1}.REVERSE_SELF())
      ==
      (emp::BitArray<2>{1,1})
    );
    CHECK((emp::BitArray<2>{1,1}.REVERSE_SELF().CountOnes()) == 2);
    CHECK(
      (emp::BitArray<2>{0,1}.REVERSE_SELF())
      ==
      (emp::BitArray<2>{1,0})
    );
    CHECK((emp::BitArray<2>{0,1}.REVERSE_SELF().CountOnes()) == 1);
    CHECK(
      (emp::BitArray<2>{0,0}.REVERSE_SELF())
      ==
      (emp::BitArray<2>{0,0})
    );
    CHECK((emp::BitArray<2>{0,0}.REVERSE_SELF().CountOnes()) == 0);

    CHECK(
      (emp::BitArray<7>{1,1,0,0,0,0,1}.REVERSE_SELF())
      ==
      (emp::BitArray<7>{1,0,0,0,0,1,1})
    );
    CHECK((emp::BitArray<7>{1,1,0,0,0,0,1}.REVERSE_SELF().CountOnes()) == 3);
    CHECK(
      (emp::BitArray<7>{1,0,1,0,1,0,1}.REVERSE_SELF())
      ==
      (emp::BitArray<7>{1,0,1,0,1,0,1})
    );
    CHECK((emp::BitArray<7>{1,0,1,0,1,0,1}.REVERSE_SELF().CountOnes()) == 4);
    CHECK(
      (emp::BitArray<7>{1,1,1,1,1,0,1}.REVERSE_SELF())
      ==
      (emp::BitArray<7>{1,0,1,1,1,1,1})
    );
    CHECK((emp::BitArray<7>{1,1,1,1,1,0,1}.REVERSE_SELF().CountOnes()) == 6);

    CHECK(
      (emp::BitArray<8>{1,1,0,0,0,0,1,0}.REVERSE_SELF())
      ==
      (emp::BitArray<8>{0,1,0,0,0,0,1,1})
    );
    CHECK((emp::BitArray<8>{1,1,0,0,0,0,1,0}.REVERSE_SELF().CountOnes()) == 3);
    CHECK(
      (emp::BitArray<8>{1,0,1,0,1,0,1,0}.REVERSE_SELF())
      ==
      (emp::BitArray<8>{0,1,0,1,0,1,0,1})
    );
    CHECK((emp::BitArray<8>{0,1,0,1,0,1,0,1}.REVERSE_SELF().CountOnes()) == 4);
    CHECK(
      (emp::BitArray<8>{1,1,1,1,1,0,1,0}.REVERSE_SELF())
      ==
      (emp::BitArray<8>{0,1,0,1,1,1,1,1})
    );
    CHECK((emp::BitArray<8>{1,1,1,1,1,0,1,0}.REVERSE_SELF().CountOnes()) == 6);

    CHECK(
      (emp::BitArray<9>{1,1,0,0,0,0,1,0,0}.REVERSE_SELF())
      ==
      (emp::BitArray<9>{0,0,1,0,0,0,0,1,1})
    );
    CHECK(
      (emp::BitArray<9>{1,1,0,0,0,0,1,0,0}.REVERSE_SELF().CountOnes())
      ==
      3
    );
    CHECK(
      (emp::BitArray<9>{1,0,1,0,1,0,1,0,0}.REVERSE_SELF())
      ==
      (emp::BitArray<9>{0,0,1,0,1,0,1,0,1})
    );
    CHECK(
      (emp::BitArray<9>{0,0,1,0,1,0,1,0,1}.REVERSE_SELF().CountOnes())
      ==
      4
    );
    CHECK(
      (emp::BitArray<9>{1,1,1,1,1,0,1,0,0}.REVERSE_SELF())
      ==
      (emp::BitArray<9>{0,0,1,0,1,1,1,1,1})
    );
    CHECK(
      (emp::BitArray<9>{1,1,1,1,1,0,1,0,0}.REVERSE_SELF().CountOnes())
      ==
      6
    );

    emp::Random rand(1);
    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitArray<15> ba(rand);
      ba[0] = 0;
      ba[15-1] = 1;
      CHECK(ba.REVERSE() != ba);
      CHECK(ba.REVERSE().REVERSE() == ba);
      CHECK(ba.REVERSE().CountOnes() == ba.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitArray<16> ba(rand);
      ba[0] = 0;
      ba[16-1] = 1;
      CHECK(ba.REVERSE() != ba);
      CHECK(ba.REVERSE().REVERSE() == ba);
      CHECK(ba.REVERSE().CountOnes() == ba.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitArray<17> ba(rand);
      ba[0] = 0;
      ba[17-1] = 1;
      CHECK(ba.REVERSE() != ba);
      CHECK(ba.REVERSE().REVERSE() == ba);
      CHECK(ba.REVERSE().CountOnes() == ba.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitArray<31> ba(rand);
      ba[0] = 0;
      ba[31-1] = 1;
      CHECK(ba.REVERSE() != ba);
      CHECK(ba.REVERSE().REVERSE() == ba);
      CHECK(ba.REVERSE().CountOnes() == ba.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitArray<32> ba(rand);
      ba[0] = 0;
      ba[32-1] = 1;
      CHECK(ba.REVERSE() != ba);
      CHECK(ba.REVERSE().REVERSE() == ba);
      CHECK(ba.REVERSE().CountOnes() == ba.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitArray<33> ba(rand);
      ba[0] = 0;
      ba[33-1] = 1;
      CHECK(ba.REVERSE() != ba);
      CHECK(ba.REVERSE().REVERSE() == ba);
      CHECK(ba.REVERSE().CountOnes() == ba.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitArray<63> ba(rand);
      ba[0] = 0;
      ba[63-1] = 1;
      CHECK(ba.REVERSE() != ba);
      CHECK(ba.REVERSE().REVERSE() == ba);
      CHECK(ba.REVERSE().CountOnes() == ba.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitArray<64> ba(rand);
      ba[0] = 0;
      ba[64-1] = 1;
      CHECK(ba.REVERSE() != ba);
      CHECK(ba.REVERSE().REVERSE() == ba);
      CHECK(ba.REVERSE().CountOnes() == ba.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitArray<65> ba(rand);
      ba[0] = 0;
      ba[65-1] = 1;
      CHECK(ba.REVERSE() != ba);
      CHECK(ba.REVERSE().REVERSE() == ba);
      CHECK(ba.REVERSE().CountOnes() == ba.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitArray<127> ba(rand);
      ba[0] = 0;
      ba[127-1] = 1;
      CHECK(ba.REVERSE() != ba);
      CHECK(ba.REVERSE().REVERSE() == ba);
      CHECK(ba.REVERSE().CountOnes() == ba.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitArray<128> ba(rand);
      ba[0] = 0;
      ba[128-1] = 1;
      CHECK(ba.REVERSE() != ba);
      CHECK(ba.REVERSE().REVERSE() == ba);
      CHECK(ba.REVERSE().CountOnes() == ba.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitArray<129> ba(rand);
      ba[0] = 0;
      ba[129-1] = 1;
      CHECK(ba.REVERSE() != ba);
      CHECK(ba.REVERSE().REVERSE() == ba);
      CHECK(ba.REVERSE().CountOnes() == ba.CountOnes());
    }

  }

  // test BitArray addition
  {
  emp::BitArray<32> ba0;
  ba0.SetUInt(0, std::numeric_limits<uint32_t>::max() - 1);
  emp::BitArray<32> ba1;
  ba1.SetUInt(0,1);
  ba0+=ba1;
  CHECK (ba0.GetUInt(0) == 4294967295);
  CHECK ((ba0+ba1).GetUInt(0) == 0);
  CHECK ((ba0+ba0).GetUInt(0) == 4294967294);

  emp::BitArray<8> ba2;
  ba2.SetUInt(0, emp::IntPow(2UL, 8UL)-1);
  emp::BitArray<8> ba3;
  ba3.SetUInt(0, 1);
  CHECK((ba2+ba3).GetUInt(0) == 0);
  emp::BitArray<64> ba4;
  ba4.SetUInt(0, std::numeric_limits<uint32_t>::max()-1);
  ba4.SetUInt(1, std::numeric_limits<uint32_t>::max());
  emp::BitArray<64> ba5;
  ba5.SetUInt(0, 1);
  ba4+=ba5;
  CHECK(ba4.GetUInt(0) == pow((size_t)2, (size_t)32)-1);
  CHECK(ba4.GetUInt(1) == pow((size_t)2, (size_t)32)-1);
  ba4+=ba5;
  CHECK(ba4.GetUInt(0) == 0);
  CHECK(ba4.GetUInt(1) == 0);
  }

  // test BitArray subtraction
  {
  emp::BitArray<32> ba0;
  ba0.SetUInt(0, 1);
  emp::BitArray<32> ba1;
  ba1.SetUInt(0, 1);
  ba0 = ba0 - ba1;
  CHECK (ba0.GetUInt(0) == 0);
  CHECK ((ba0-ba1).GetUInt(0) == std::numeric_limits<uint32_t>::max());

  emp::BitArray<8> ba2;
  ba2.SetUInt(0, 1);
  emp::BitArray<8> ba3;
  ba3.SetUInt(0, 1);

  ba2-=ba3;
  CHECK (ba2.GetUInt(0) == 0);
  CHECK((ba2-ba3).GetUInt(0) == emp::IntPow(2UL,8UL)-1);

  emp::BitArray<64> ba4;
  ba4.SetUInt(0, 1);
  ba4.SetUInt(1, 0);

  emp::BitArray<64> ba5;
  ba5.SetUInt(0, 1);

  ba4-=ba5;
  CHECK(ba4.GetUInt(0) == 0);
  CHECK(ba4.GetUInt(1) == 0);

  ba4-=ba5;
  CHECK(ba4.GetUInt(0) == std::numeric_limits<uint32_t>::max());
  CHECK(ba4.GetUInt(1) == std::numeric_limits<uint32_t>::max());
  ba4 = ba4 - ba5;
  CHECK(ba4.GetUInt(0) == std::numeric_limits<uint32_t>::max() - 1);
  CHECK(ba4.GetUInt(1) == std::numeric_limits<uint32_t>::max());
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

  for (size_t i = 0; i < 64; ++i) CHECK((ba1 - ba2).Get(i));
  CHECK(!(ba1 - ba2).Get(64));

  ba1 -= ba2;

  for (size_t i = 0; i < 64; ++i) {
    CHECK(ba1.Get(i));
  }
  CHECK(!ba1.Get(64));

  /* PART 2 */
  ba1.Clear();
  ba2.Clear();

  ba2.Set(0);  // ...00001

  for (size_t i = 0; i < 65; ++i) CHECK((ba1 - ba2).Get(i));

  ba1 -= ba2;

  for (size_t i = 0; i < 65; ++i) CHECK(ba1.Get(i));

  /* PART 3 */
  ba1.Clear();
  ba2.Clear();

  for (size_t i = 0; i < 65; ++i) ba1.Set(i); // 11111...11111
  ba2.Set(0);  // ...00001

  for (size_t i = 0; i < 65; ++i) CHECK(!(ba1 + ba2).Get(i));
  for (size_t i = 0; i < 65; ++i) CHECK(!(ba2 + ba1).Get(i));

  ba1 += ba2;

  for (size_t i = 0; i < 65; ++i) CHECK(!ba1.Get(i));

  /* PART 4 */
  ba1.Clear();
  ba2.Clear();

  for (size_t i = 0; i < 64; ++i) ba1.Set(i); // 01111...11111
  ba2.Set(0);  // ...00001

  for (size_t i = 0; i < 64; ++i) CHECK(!(ba1 + ba2).Get(i));
  CHECK((ba1 + ba2).Get(64));
  for (size_t i = 0; i < 64; ++i) CHECK(!(ba2 + ba1).Get(i));
  CHECK((ba2 + ba1).Get(64));

  ba1 += ba2;

  for (size_t i = 0; i < 64; ++i) CHECK(!ba1.Get(i));
  CHECK((ba2 + ba1).Get(64));
  }

  {
  emp::BitArray<3> ba0{0,0,0};
  CHECK(ba0.GetUInt8(0) == 0);
  CHECK(ba0.GetUInt16(0) == 0);
  CHECK(ba0.GetUInt32(0) == 0);
  CHECK(ba0.GetUInt64(0) == 0);
  CHECK(ba0.GetNumStates() == 8);

  emp::BitArray<3> ba1{1,0,0};
  CHECK(ba1.GetUInt8(0) == 1);
  CHECK(ba1.GetUInt16(0) == 1);
  CHECK(ba1.GetUInt32(0) == 1);
  CHECK(ba1.GetUInt64(0) == 1);

  emp::BitArray<3> ba2{1,1,0};
  CHECK(ba2.GetUInt8(0) == 3);
  CHECK(ba2.GetUInt16(0) == 3);
  CHECK(ba2.GetUInt32(0) == 3);
  CHECK(ba2.GetUInt64(0) == 3);

  emp::BitArray<3> ba3{1,1,1};
  CHECK(ba3.GetUInt8(0) == 7);

  emp::BitArray<3> ba4{0,1,1};
  CHECK(ba4.GetUInt8(0) == 6);

  emp::BitArray<32> ba5;
  ba5.SetUInt(0, 1789156UL);
  CHECK(ba5.GetUInt64(0) == 1789156ULL);
  CHECK(ba5.GetNumStates() == 4294967296ULL);

  emp::BitArray<63> ba6;
  ba6.SetUInt64(0, 789156816848ULL);
  CHECK(ba6.GetUInt64(0) == 789156816848ULL);
  CHECK(ba6.GetNumStates() == 9223372036854775808ULL);


  // @CAO: Removed GetDouble() due to confusing name (GetUInt64() gives the same answer, but with
  //       the correct encoding.
  // emp::BitArray<65> ba7;
  // ba7.SetUInt64(0, 1789156816848ULL);
  // ba7.Set(64);
  // CHECK(ba7.GetDouble() == 1789156816848.0 + emp::Pow2(64.0));
  // CHECK(ba7.MaxDouble() == 36893488147419103231.0);

  // emp::BitArray<1027> ba8;
  // ba8.Set(1026);
  // CHECK(std::isinf(ba8.GetDouble()));
  // CHECK(std::isinf(ba8.MaxDouble()));
  }

  // test list initializer
  {
  emp::BitArray<3> ba_empty{0,0,0};
  emp::BitArray<3> ba_first{1,0,0};
  emp::BitArray<3> ba_last{0,0,1};
  emp::BitArray<3> ba_full{1,1,1};

  CHECK(ba_empty.CountOnes() == 0);
  CHECK(ba_first.CountOnes() == 1);
  CHECK(ba_last.CountOnes() == 1);
  CHECK(ba_full.CountOnes() == 3);
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
      CHECK(orig[i] == d1[i/32][i%32]);
      CHECK(orig[i] == d2[i/16][i%16]);
      CHECK(orig[i] == d4[i/8][i%8]);
      CHECK(orig[i] == d8[i/4][i%4]);
      CHECK(orig[i] == d16[i/2][i%2]);
      CHECK(orig[i] == d32[i/1][i%1]);
    }

    // Export

    d1[0] = orig.ExportArray<32>(0);
    for (size_t i = 0; i < 2; ++i) d2[i] = orig.ExportArray<16>(i * 16);
    for (size_t i = 0; i < 4; ++i) d4[i] = orig.ExportArray<8>(i * 8);
    for (size_t i = 0; i < 8; ++i) d8[i] = orig.ExportArray<4>(i * 4);
    for (size_t i = 0; i < 16; ++i) d16[i] = orig.ExportArray<2>(i * 2);
    for (size_t i = 0; i < 32; ++i) d32[i] = orig.ExportArray<1>(i * 1);

    for (size_t i = 0; i < 32; ++i) {
      CHECK(orig[i] == d1[i/32][i%32]);
      CHECK(orig[i] == d2[i/16][i%16]);
      CHECK(orig[i] == d4[i/8][i%8]);
      CHECK(orig[i] == d8[i/4][i%4]);
      CHECK(orig[i] == d16[i/2][i%2]);
      CHECK(orig[i] == d32[i/1][i%1]);
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
    CHECK((shift_set.CountOnes() == 1) == (i <= 71));
  }

  ba80.Clear();

  CHECK(ba10[2] == false);
  ba10.flip(2);
  CHECK(ba10[2] == true);

  CHECK(ba32[2] == false);
  ba32.flip(2);
  CHECK(ba32[2] == true);

  CHECK(ba80[2] == false);
  ba80.flip(2);
  CHECK(ba80[2] == true);

  for (size_t i = 3; i < 8; i++) {CHECK(ba10[i] == false);}
  ba10.flip(3, 8);
  for (size_t i = 3; i < 8; i++) {CHECK(ba10[i] == true);}
  CHECK(ba10[8] == false);

  for (size_t i = 3; i < 8; i++) {CHECK(ba32[i] == false);}
  ba32.flip(3, 8);
  for (size_t i = 3; i < 8; i++) {CHECK(ba32[i] == true);}
  CHECK(ba32[8] == false);

  for (size_t i = 3; i < 8; i++) {CHECK(ba80[i] == false);}
  ba80.flip(3, 8);
  for (size_t i = 3; i < 8; i++) {CHECK(ba80[i] == true);}
  CHECK(ba80[8] == false);

  ba80[70] = 1;

  CHECK(ba10.GetUInt(0) == 252);
  CHECK(ba10.GetUInt32(0) == 252);
  CHECK(ba10.GetUInt64(0) == 252);

  CHECK(ba32.GetUInt(0) == 252);
  CHECK(ba32.GetUInt32(0) == 252);
  CHECK(ba32.GetUInt64(0) == 252);

  CHECK(ba80.GetUInt(0) == 252);
  CHECK(ba80.GetUInt(1) == 0);
  CHECK(ba80.GetUInt(2) == 64);
  CHECK(ba80.GetUInt32(0) == 252);
  CHECK(ba80.GetUInt32(1) == 0);
  CHECK(ba80.GetUInt32(2) == 64);
  CHECK(ba80.GetUInt64(0) == 252);
  CHECK(ba80.GetUInt64(1) == 64);

  ba80 = ba80c;

  // Test arbitrary bit retrieval of UInts
  ba80[65] = 1;
  CHECK(ba80.GetUInt32(2) == 130);
  CHECK(ba80.GetUInt32AtBit(64) == 130);
  CHECK(ba80.GetUInt8AtBit(64) == 130);

  emp::BitArray<96> ba;

  CHECK (ba.LongestSegmentOnes() == 0);
  ba.SetUInt(2, 1);
  CHECK (ba.LongestSegmentOnes() == 1);
  ba.SetUInt(1, 3);
  CHECK (ba.LongestSegmentOnes() == 2);
  ba.SetUInt(0, 7);
  CHECK (ba.LongestSegmentOnes() == 3);

  ba.SetUInt(0, std::numeric_limits<uint32_t>::max());
  ba.SetUInt(1, std::numeric_limits<uint32_t>::max() - 1);
  ba.SetUInt(2, std::numeric_limits<uint32_t>::max() - 3);
  CHECK (ba.LongestSegmentOnes() == 32);

  // tests for ROTATE
  // ... with one set bit
  ba10.Clear(); ba10.Set(0);
  ba25.Clear(); ba25.Set(0);
  ba32.Clear(); ba32.Set(0);
  ba50.Clear(); ba50.Set(0);
  ba64.Clear(); ba64.Set(0);
  ba80.Clear(); ba80.Set(0);

  for (int rot = -100; rot < 101; ++rot) {

    CHECK( ba10.CountOnes() == ba10.ROTATE(rot).CountOnes() );
    CHECK( ba25.CountOnes() == ba25.ROTATE(rot).CountOnes() );
    CHECK( ba32.CountOnes() == ba32.ROTATE(rot).CountOnes() );
    CHECK( ba50.CountOnes() == ba50.ROTATE(rot).CountOnes() );
    CHECK( ba64.CountOnes() == ba64.ROTATE(rot).CountOnes() );
    CHECK( ba80.CountOnes() == ba80.ROTATE(rot).CountOnes() );

    if (rot % 10) CHECK( ba10 != ba10.ROTATE(rot) );
    else CHECK( ba10 == ba10.ROTATE(rot) );

    if (rot % 25) CHECK( ba25 != ba25.ROTATE(rot) );
    else CHECK( ba25 == ba25.ROTATE(rot) );

    if (rot % 32) CHECK( ba32 != ba32.ROTATE(rot) );
    else CHECK( ba32 == ba32.ROTATE(rot) );

    if (rot % 50) CHECK( ba50 != ba50.ROTATE(rot) );
    else CHECK( ba50 == ba50.ROTATE(rot) );

    if (rot % 64) CHECK( ba64 != ba64.ROTATE(rot) );
    else CHECK( ba64 == ba64.ROTATE(rot) );

    if (rot % 80) CHECK( ba80 != ba80.ROTATE(rot) );
    else CHECK( ba80 == ba80.ROTATE(rot) );

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

    CHECK( ba25.CountOnes() == ba25.ROTATE(rot).CountOnes() );
    CHECK( ba32.CountOnes() == ba32.ROTATE(rot).CountOnes() );
    CHECK( ba50.CountOnes() == ba50.ROTATE(rot).CountOnes() );
    CHECK( ba64.CountOnes() == ba64.ROTATE(rot).CountOnes() );
    CHECK( ba80.CountOnes() == ba80.ROTATE(rot).CountOnes() );

    if (rot % 25) CHECK( ba25 != ba25.ROTATE(rot) );
    else CHECK( ba25 == ba25.ROTATE(rot) );

    if (rot % 32) CHECK( ba32 != ba32.ROTATE(rot) );
    else CHECK( ba32 == ba32.ROTATE(rot) );

    if (rot % 50) CHECK( ba50 != ba50.ROTATE(rot) );
    else CHECK( ba50 == ba50.ROTATE(rot) );

    if (rot % 64) CHECK( ba64 != ba64.ROTATE(rot) );
    else CHECK( ba64 == ba64.ROTATE(rot) );

    if (rot % 80) CHECK( ba80 != ba80.ROTATE(rot) );
    else CHECK( ba80 == ba80.ROTATE(rot) );

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
    CHECK(!ba_25.CountOnes());

    ba_32.FlipRandomCount(random, 0);
    CHECK(!ba_32.CountOnes());

    ba_50.FlipRandomCount(random, 0);
    CHECK(!ba_50.CountOnes());

    ba_64.FlipRandomCount(random, 0);
    CHECK(!ba_64.CountOnes());

    ba_80.FlipRandomCount(random, 0);
    CHECK(!ba_80.CountOnes());


    ba_25.FlipRandomCount(random, 1);
    CHECK( ba_25.CountOnes() == 1);

    ba_32.FlipRandomCount(random, 1);
    CHECK( ba_32.CountOnes() == 1);

    ba_50.FlipRandomCount(random, 1);
    CHECK( ba_50.CountOnes() == 1);

    ba_64.FlipRandomCount(random, 1);
    CHECK( ba_64.CountOnes() == 1);

    ba_80.FlipRandomCount(random, 1);
    CHECK( ba_80.CountOnes() == 1);

    ba_25.Clear();
    ba_32.Clear();
    ba_50.Clear();
    ba_64.Clear();
    ba_80.Clear();

    for (size_t i = 1; i < 5000; ++i) {
      ba_25.FlipRandomCount(random, 1);
      CHECK(ba_25.CountOnes() <= i);

      ba_32.FlipRandomCount(random, 1);
      CHECK(ba_32.CountOnes() <= i);

      ba_50.FlipRandomCount(random, 1);
      CHECK(ba_50.CountOnes() <= i);

      ba_64.FlipRandomCount(random, 1);
      CHECK(ba_64.CountOnes() <= i);

      ba_80.FlipRandomCount(random, 1);
      CHECK(ba_80.CountOnes() <= i);
    }

    CHECK(ba_25.CountOnes() > ba_25.size()/4);
    CHECK(ba_25.CountOnes() < 3*ba_25.size()/4);
    CHECK(ba_32.CountOnes() > ba_32.size()/4);
    CHECK(ba_32.CountOnes() < 3*ba_32.size()/4);
    CHECK(ba_50.CountOnes() > ba_50.size()/4);
    CHECK(ba_50.CountOnes() < 3*ba_50.size()/4);
    CHECK(ba_64.CountOnes() > ba_64.size()/4);
    CHECK(ba_64.CountOnes() < 3*ba_64.size()/4);
    CHECK(ba_80.CountOnes() > ba_80.size()/4);
    CHECK(ba_80.CountOnes() < 3*ba_80.size()/4);

    for (size_t i = 0; i < 10; ++i) {
      ba_25.FlipRandomCount(random, ba_25.size());
      CHECK(ba_25.CountOnes() > ba_25.size()/4);
      CHECK(ba_25.CountOnes() < 3*ba_25.size()/4);

      ba_32.FlipRandomCount(random, ba_32.size());
      CHECK(ba_32.CountOnes() > ba_32.size()/4);
      CHECK(ba_32.CountOnes() < 3*ba_32.size()/4);

      ba_50.FlipRandomCount(random, ba_50.size());
      CHECK(ba_50.CountOnes() > ba_50.size()/4);
      CHECK(ba_50.CountOnes() < 3*ba_50.size()/4);

      ba_64.FlipRandomCount(random, ba_64.size());
      CHECK(ba_64.CountOnes() > ba_64.size()/4);
      CHECK(ba_64.CountOnes() < 3*ba_64.size()/4);

      ba_80.FlipRandomCount(random, ba_80.size());
      CHECK(ba_80.CountOnes() > ba_80.size()/4);
      CHECK(ba_80.CountOnes() < 3*ba_80.size()/4);
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

    CHECK(ba10 == ba10_deser);
    CHECK(ba25 == ba25_deser);
    CHECK(ba32 == ba32_deser);
    CHECK(ba50 == ba50_deser);
    CHECK(ba64 == ba64_deser);
    CHECK(ba80 == ba80_deser);

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

    CHECK(ba10 == ba10_deser);
    CHECK(ba25 == ba25_deser);
    CHECK(ba32 == ba32_deser);
    CHECK(ba50 == ba50_deser);
    CHECK(ba64 == ba64_deser);
    CHECK(ba80 == ba80_deser);

  }

}

TEST_CASE("28: Test BitArray string construction", "[tools]") {

  CHECK( emp::BitArray<5>( "01001" ) == emp::BitArray<5>{0, 1, 0, 0, 1} );

  // std::bitset treats bits in the opposite direction of emp::BitArray.
  CHECK(
    emp::BitArray<5>( std::bitset<5>( "01001" ) )
    == emp::BitArray<5>{1, 0, 0, 1, 0}
  );

}

TEST_CASE("29: Test Append()", "[tools]") {
  emp::BitVector bv1 = {0,1,1,1,0,0,0,1};
  emp::BitArray<8> ba1 = {0,0,0,1,0,0,0,1};
  bv1.Append(ba1);
  emp::BitVector exp_result = {0,1,1,1,0,0,0,1,0,0,0,1,0,0,0,1};

  CHECK( bv1 == exp_result );
}
