/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2023.
 *
 *  @file RangeSet.cpp
 */

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/math/RangeSet.hpp"

TEST_CASE("Test integral RangeSets", "[math]")
{
  emp::RangeSet<int> rs1;

  CHECK(rs1.Has(27) == false);
  CHECK(rs1.Has(-1003) == false);
  CHECK(rs1.Has(0) == false);
  CHECK(rs1.GetNumRanges() == 0);
  CHECK(rs1.GetSize() == 0);

  rs1.Insert(27);

  CHECK(rs1.Has(26) == false);
  CHECK(rs1.Has(27) == true);
  CHECK(rs1.Has(28) == false);
  CHECK(rs1.Has(-1003) == false);
  CHECK(rs1.Has(0) == false);
  CHECK(rs1.GetStart() == 27);
  CHECK(rs1.GetEnd() == 28);
  CHECK(rs1.GetNumRanges() == 1);
  CHECK(rs1.GetSize() == 1);

  rs1.Insert(28);

  CHECK(rs1.Has(26) == false);
  CHECK(rs1.Has(27) == true);
  CHECK(rs1.Has(28) == true);
  CHECK(rs1.Has(29) == false);
  CHECK(rs1.Has(-1003) == false);
  CHECK(rs1.Has(0) == false);
  CHECK(rs1.GetStart() == 27);
  CHECK(rs1.GetEnd() == 29);
  CHECK(rs1.GetNumRanges() == 1);
  CHECK(rs1.GetSize() == 2);

  rs1.Insert(26);

  CHECK(rs1.Has(25) == false);
  CHECK(rs1.Has(26) == true);
  CHECK(rs1.Has(27) == true);
  CHECK(rs1.Has(28) == true);
  CHECK(rs1.Has(29) == false);
  CHECK(rs1.Has(-1003) == false);
  CHECK(rs1.Has(0) == false);
  CHECK(rs1.GetStart() == 26);
  CHECK(rs1.GetEnd() == 29);
  CHECK(rs1.GetNumRanges() == 1);
  CHECK(rs1.GetSize() == 3);

  rs1.Insert(emp::Range<int,false>(23,26));

  // Make sure RangeSets are identified as equal, even if constructed differently.
  emp::RangeSet<int> rs2(emp::Range<int,false>{23,29});
  CHECK(rs1 == rs2);
  CHECK(rs1.GetStart() == 23);
  CHECK(rs1.GetEnd() == 29);
  CHECK(rs1.GetNumRanges() == 1);
  CHECK(rs1.GetSize() == 6);

  // Make sure Remove works...
  CHECK(rs1.Has(26) == true);
  rs1.Remove(26);
  CHECK(rs1.Has(23) == true);
  CHECK(rs1.Has(24) == true);
  CHECK(rs1.Has(25) == true);
  CHECK(rs1.Has(26) == false);
  CHECK(rs1.Has(27) == true);
  CHECK(rs1.Has(28) == true);
  CHECK(rs1.Has(29) == false);
  CHECK(rs1.Has(-1003) == false);
  CHECK(rs1.Has(0) == false);
  CHECK(rs1 != rs2);
  CHECK(rs1.GetStart() == 23);
  CHECK(rs1.GetEnd() == 29);
  CHECK(rs1.GetNumRanges() == 2);
  CHECK(rs1.GetSize() == 5);

  // And the re-Insertion and merging.
  rs1.Insert(26);
  CHECK(rs1 == rs2);
  CHECK(rs1.GetNumRanges() == 1);
  CHECK(rs1.GetSize() == 6);

  rs2.Insert(-1);
  rs2.Insert(1000);
  CHECK(rs1 != rs2);
  CHECK(rs1.GetNumRanges() == 1);
  CHECK(rs2.GetNumRanges() == 3);
  CHECK(rs1.GetSize() == 6);
  CHECK(rs2.GetSize() == 8);
}

TEST_CASE("Test continuous RangeSets", "[math]")
{
  emp::RangeSet<double> rs1;
  using range_t = emp::RangeSet<double>::range_t;
  range_t range(0.4, 0.9);

  rs1.InsertRange(0.0, 100.0);
  CHECK(rs1.GetSize() == Approx(100.0));

  CHECK(rs1.HasOverlap(range) == true);
  rs1.Remove(range);
  CHECK(rs1.HasOverlap(range) == false);
  CHECK(rs1.GetSize() == Approx(99.5));
  rs1.Remove(range);
  CHECK(rs1.HasOverlap(range) == false);
  CHECK(rs1.GetSize() == Approx(99.5));

  for (size_t i = 0; i < 100; ++i) {
    range.Shift(1.0);
    rs1.Remove(range);  // Only the first 99 should matter -- then we're out of range.)
  }
  // From 0.0 to 100.0 we should have gaps from every *.4 to *.9

  CHECK(rs1.GetSize() == Approx(50.0));
  CHECK(rs1.GetNumRanges() == 101);

  // Remove exact range.
  CHECK(rs1.HasOverlap(range_t(1.9, 2.4)) == true);
  rs1.RemoveRange(1.9, 2.4);
  CHECK(rs1.HasOverlap(range_t(1.9, 2.4)) == false);
  CHECK(rs1.GetSize() == Approx(49.5));
  CHECK(rs1.GetNumRanges() == 100);

  // Remove range + gap before
  CHECK(rs1.HasOverlap(range_t(3.4, 4.4)) == true);
  rs1.RemoveRange(3.4, 4.4);
  CHECK(rs1.HasOverlap(range_t(3.4, 4.4)) == false);
  CHECK(rs1.GetSize() == Approx(49.0));
  CHECK(rs1.GetNumRanges() == 99);

  // Remove range + gap after
  CHECK(rs1.HasOverlap(range_t(5.9, 6.9)) == true);
  rs1.RemoveRange(5.9, 6.9);
  CHECK(rs1.HasOverlap(range_t(5.9, 6.9)) == false);
  CHECK(rs1.GetSize() == Approx(48.5));
  CHECK(rs1.GetNumRanges() == 98);

  // Remove range + both gaps
  CHECK(rs1.HasOverlap(range_t(7.4, 8.9)) == true);
  rs1.RemoveRange(7.4, 8.9);
  CHECK(rs1.HasOverlap(range_t(7.4, 8.9)) == false);
  CHECK(rs1.GetSize() == Approx(48.0));
  CHECK(rs1.GetNumRanges() == 97);

  // Remove range + to middle of both gaps
  CHECK(rs1.HasOverlap(range_t(10.6, 11.7)) == true);
  rs1.RemoveRange(10.6, 11.7);
  CHECK(rs1.HasOverlap(range_t(10.6, 11.7)) == false);
  CHECK(rs1.GetSize() == Approx(47.5));
  CHECK(rs1.GetNumRanges() == 96);

  // Try a bunch of failed removals.
  const emp::RangeSet<double> rs1_bak(rs1);
  rs1.RemoveRange(13.4, 13.9);  // Remove full gap.
  rs1.RemoveRange(13.7, 13.9);  // Remove middle to end of gap.
  rs1.RemoveRange(13.4, 13.6);  // Remove beginning to middle of gap.
  rs1.RemoveRange(13.5, 13.8);  // Remove chunk in middle of gap.
  CHECK(rs1 == rs1_bak);        // Nothing should have changed from removals.

  CHECK(rs1.GetSize() == Approx(47.5));
  CHECK(rs1.GetNumRanges() == 96);

  // Remove middle of one range through middle of next.
  CHECK(rs1.HasOverlap(range_t(14.15,15.15)) == true);
  rs1.RemoveRange(14.15,15.15);
  CHECK(rs1.HasOverlap(range_t(14.15,15.15)) == false);
  CHECK(rs1.GetSize() == Approx(47.0));
  CHECK(rs1.GetNumRanges() == 96);

  // Remove middle of one range through end of next.
  CHECK(rs1.HasOverlap(range_t(16.15,17.4)) == true);
  rs1.RemoveRange(16.15,17.4);
  CHECK(rs1.HasOverlap(range_t(16.15,17.4)) == false);
  CHECK(rs1.GetSize() == Approx(46.25));
  CHECK(rs1.GetNumRanges() == 95);

  // Remove beginning of one range through middle of next.
  CHECK(rs1.HasOverlap(range_t(18.9,20.15)) == true);
  rs1.RemoveRange(18.9,20.15);
  CHECK(rs1.HasOverlap(range_t(18.9,20.15)) == false);
  CHECK(rs1.GetSize() == Approx(45.5));
  CHECK(rs1.GetNumRanges() == 94);

  // Remove beginning of one range through end of next.
  CHECK(rs1.HasOverlap(range_t(21.9,23.4)) == true);
  rs1.RemoveRange(21.9,23.4);
  CHECK(rs1.HasOverlap(range_t(21.9,23.4)) == false);
  CHECK(rs1.GetSize() == Approx(44.5));
  CHECK(rs1.GetNumRanges() == 92);

  // Remove middle section of one range only
  CHECK(rs1.HasOverlap(range_t(26.0,26.25)) == true);
  rs1.RemoveRange(26.0,26.25);
  CHECK(rs1.HasOverlap(range_t(26.0,26.25)) == false);
  CHECK(rs1.GetSize() == Approx(44.25));
  CHECK(rs1.GetNumRanges() == 93);
  
  // Remove middle of one range through middle of two later.
  CHECK(rs1.HasOverlap(range_t(29.0,31.0)) == true);
  rs1.RemoveRange(29.0,31.0);
  CHECK(rs1.HasOverlap(range_t(29.0,31.0)) == false);
  CHECK(rs1.GetSize() == Approx(43.25));
  CHECK(rs1.GetNumRanges() == 92);

  // Remove middle of one range through middle of four later.
  CHECK(rs1.HasOverlap(range_t(34.0,38.0)) == true);
  rs1.RemoveRange(34.0,38.0);
  CHECK(rs1.HasOverlap(range_t(34.0,38.0)) == false);
  CHECK(rs1.GetSize() == Approx(41.25));
  CHECK(rs1.GetNumRanges() == 89);

}

TEST_CASE("Test RangeSet Inversion calculations", "[math]")
{
  emp::RangeSet<double> rsd(10.5, 20.5);

  CHECK(rsd.GetNumRanges() == 1);
  CHECK(rsd.Has(std::numeric_limits<double>::lowest()) == false);
  CHECK(rsd.Has(0.0) == false);
  CHECK(rsd.Has(10.0) == false);
  CHECK(rsd.Has(10.5) == true);
  CHECK(rsd.Has(15.0) == true);
  CHECK(rsd.Has(20.0) == true);
  CHECK(rsd.Has(20.5) == false);
  CHECK(rsd.Has(100.0) == false);
  CHECK(rsd.Has(150.0) == false);
  CHECK(rsd.Has(1000.0) == false);

  rsd.Invert();

  CHECK(rsd.GetNumRanges() == 2);
  CHECK(rsd.Has(std::numeric_limits<double>::lowest()) == true);
  CHECK(rsd.Has(0.0) == true);
  CHECK(rsd.Has(10.0) == true);
  CHECK(rsd.Has(10.5) == false);
  CHECK(rsd.Has(15.0) == false);
  CHECK(rsd.Has(20.0) == false);
  CHECK(rsd.Has(20.5) == true);
  CHECK(rsd.Has(100.0) == true);
  CHECK(rsd.Has(150.0) == true);
  CHECK(rsd.Has(1000.0) == true);

  rsd.RemoveRange(100.5, 200.5);
  CHECK(rsd.GetNumRanges() == 3);
  CHECK(rsd.Has(0.0) == true);
  CHECK(rsd.Has(10.0) == true);
  CHECK(rsd.Has(10.5) == false);
  CHECK(rsd.Has(15.0) == false);
  CHECK(rsd.Has(20.0) == false);
  CHECK(rsd.Has(20.5) == true);
  CHECK(rsd.Has(100.0) == true);
  CHECK(rsd.Has(150.0) == false);
  CHECK(rsd.Has(1000.0) == true);

  rsd.Invert();
  CHECK(rsd.GetNumRanges() == 2);
  CHECK(rsd.Has(0.0) == false);
  CHECK(rsd.Has(10.0) == false);
  CHECK(rsd.Has(10.5) == true);
  CHECK(rsd.Has(15.0) == true);
  CHECK(rsd.Has(20.0) == true);
  CHECK(rsd.Has(20.5) == false);
  CHECK(rsd.Has(100.0) == false);
  CHECK(rsd.Has(150.0) == true);
  CHECK(rsd.Has(1000.0) == false);
}

TEST_CASE("Test RangeSet operators to behave like Bits", "[math]")
{
  using set_t = emp::RangeSet<size_t>;
  const set_t input1("00001111");
  const set_t input2("00110011");
  const set_t input3("01010101");

  // Test *_SELF() Boolean Logic functions.
  set_t bv;      CHECK(bv.KeepOnly(0,8) == set_t("00000000"));
  bv.Invert();      CHECK(bv.KeepOnly(0,8) == set_t("11111111"));
  bv &= input1;     CHECK(bv.KeepOnly(0,8) == set_t("00001111"));
  bv &= input1;     CHECK(bv.KeepOnly(0,8) == set_t("00001111"));
  bv &= input2;     CHECK(bv.KeepOnly(0,8) == set_t("00000011"));
  bv &= input3;     CHECK(bv.KeepOnly(0,8) == set_t("00000001"));

  bv |= input1;      CHECK(bv.KeepOnly(0,8) == set_t("00001111"));
  bv |= input1;      CHECK(bv.KeepOnly(0,8) == set_t("00001111"));
  bv |= input3;      CHECK(bv.KeepOnly(0,8) == set_t("01011111"));
  bv |= input2;      CHECK(bv.KeepOnly(0,8) == set_t("01111111"));

  bv &= input1; bv.Invert();  CHECK(bv.KeepOnly(0,8) == set_t("11110000"));
  bv &= input1; bv.Invert();  CHECK(bv.KeepOnly(0,8) == set_t("11111111"));
  bv &= input2; bv.Invert();  CHECK(bv.KeepOnly(0,8) == set_t("11001100"));
  bv &= input3; bv.Invert();  CHECK(bv.KeepOnly(0,8) == set_t("10111011"));

  bv |= input1; bv.Invert();  CHECK(bv.KeepOnly(0,8) == set_t("01000000"));
  bv |= input1; bv.Invert();  CHECK(bv.KeepOnly(0,8) == set_t("10110000"));
  bv |= input2; bv.Invert();  CHECK(bv.KeepOnly(0,8) == set_t("01001100"));
  bv |= input3; bv.Invert();  CHECK(bv.KeepOnly(0,8) == set_t("10100010"));

  bv ^= input1;               CHECK(bv.KeepOnly(0,8) == set_t("10101101"));
  bv ^= input1;               CHECK(bv.KeepOnly(0,8) == set_t("10100010"));
  bv ^= input2;               CHECK(bv.KeepOnly(0,8) == set_t("10010001"));
  bv ^= input3;               CHECK(bv.KeepOnly(0,8) == set_t("11000100"));

  bv ^= input1; bv.Invert();  CHECK(bv.KeepOnly(0,8) == set_t("00110100"));
  bv ^= input1; bv.Invert();  CHECK(bv.KeepOnly(0,8) == set_t("11000100"));
  bv ^= input2; bv.Invert();  CHECK(bv.KeepOnly(0,8) == set_t("00001000"));
  bv ^= input3; bv.Invert();  CHECK(bv.KeepOnly(0,8) == set_t("10100010"));

  bv.Invert();                CHECK(bv.KeepOnly(0,8) == set_t("01011101"));

  // Test regular Boolean Logic functions.
  bv.Clear();                 CHECK(bv.KeepOnly(0,8) == set_t("00000000"));
  set_t bv1 = ~bv;            CHECK(bv1 == set_t("11111111"));

  bv1 = bv1 & input1;         CHECK(bv1 == set_t("00001111"));
  set_t bv2 = bv1 & input1;   CHECK(bv2 == set_t("00001111"));
  set_t bv3 = bv2 & input2;   CHECK(bv3 == set_t("00000011"));
  set_t bv4 = bv3 & input3;   CHECK(bv4 == set_t("00000001"));

  bv1 = bv4 | input1;         CHECK(bv1 == set_t("00001111"));
  bv2 = bv1 | input1;         CHECK(bv2 == set_t("00001111"));
  bv3 = bv2 | input3;         CHECK(bv3 == set_t("01011111"));
  bv4 = bv3 | input2;         CHECK(bv4 == set_t("01111111"));

  bv1 = bv4 & input1; bv1.Invert();   CHECK(bv1 == set_t("11110000"));
  bv2 = bv1 & input1; bv2.Invert();   CHECK(bv2 == set_t("11111111"));
  bv3 = bv2 & input2; bv3.Invert();   CHECK(bv3 == set_t("11001100"));
  bv4 = bv3 & input3; bv4.Invert();   CHECK(bv4 == set_t("10111011"));

  bv1 = bv4 | input1; bv1.Invert();   CHECK(bv1 == set_t("01000000"));
  bv2 = bv1 | input1; bv2.Invert();   CHECK(bv2 == set_t("10110000"));
  bv3 = bv2 | input2; bv3.Invert();   CHECK(bv3 == set_t("01001100"));
  bv4 = bv3 | input3; bv4.Invert();   CHECK(bv4 == set_t("10100010"));

  bv1 = bv4 ^ input1;                 CHECK(bv1 == set_t("10101101"));
  bv2 = bv1 ^ input1;                 CHECK(bv2 == set_t("10100010"));
  bv3 = bv2 ^ input2;                 CHECK(bv3 == set_t("10010001"));
  bv4 = bv3 ^ input3;                 CHECK(bv4 == set_t("11000100"));

  bv1 = bv4 ^ input1; bv1.Invert();   CHECK(bv1 == set_t("00110100"));
  bv2 = bv1 ^ input1; bv2.Invert();   CHECK(bv2 == set_t("11000100"));
  bv3 = bv2 ^ input2; bv3.Invert();   CHECK(bv3 == set_t("00001000"));
  bv4 = bv3 ^ input3; bv4.Invert();   CHECK(bv4 == set_t("10100010"));

  bv = ~bv4;                          CHECK(bv.KeepOnly(0,8) == set_t("01011101"));


  // Test Boolean Logic operators.
  bv.Clear();               CHECK(bv.KeepOnly(0,8) == set_t("00000000"));
  bv1 = ~bv;                CHECK(bv1 == set_t("11111111"));

  bv1 = bv1 & input1;       CHECK(bv1 == set_t("00001111"));
  bv2 = bv1 & input1;       CHECK(bv2 == set_t("00001111"));
  bv3 = bv2 & input2;       CHECK(bv3 == set_t("00000011"));
  bv4 = bv3 & input3;       CHECK(bv4 == set_t("00000001"));

  bv1 = bv4 | input1;       CHECK(bv1 == set_t("00001111"));
  bv2 = bv1 | input1;       CHECK(bv2 == set_t("00001111"));
  bv3 = bv2 | input3;       CHECK(bv3 == set_t("01011111"));
  bv4 = bv3 | input2;       CHECK(bv4 == set_t("01111111"));

  bv1 = ~(bv4 & input1);    CHECK(bv1 == set_t("11110000"));
  bv2 = ~(bv1 & input1);    CHECK(bv2 == set_t("11111111"));
  bv3 = ~(bv2 & input2);    CHECK(bv3 == set_t("11001100"));
  bv4 = ~(bv3 & input3);    CHECK(bv4 == set_t("10111011"));

  bv1 = ~(bv4 | input1);    CHECK(bv1 == set_t("01000000"));
  bv2 = ~(bv1 | input1);    CHECK(bv2 == set_t("10110000"));
  bv3 = ~(bv2 | input2);    CHECK(bv3 == set_t("01001100"));
  bv4 = ~(bv3 | input3);    CHECK(bv4 == set_t("10100010"));

  bv1 = bv4 ^ input1;       CHECK(bv1 == set_t("10101101"));
  bv2 = bv1 ^ input1;       CHECK(bv2 == set_t("10100010"));
  bv3 = bv2 ^ input2;       CHECK(bv3 == set_t("10010001"));
  bv4 = bv3 ^ input3;       CHECK(bv4 == set_t("11000100"));

  bv1 = ~(bv4 ^ input1);    CHECK(bv1 == set_t("00110100"));
  bv2 = ~(bv1 ^ input1);    CHECK(bv2 == set_t("11000100"));
  bv3 = ~(bv2 ^ input2);    CHECK(bv3 == set_t("00001000"));
  bv4 = ~(bv3 ^ input3);    CHECK(bv4 == set_t("10100010"));

  bv = ~bv4;                CHECK(bv.KeepOnly(0,8) == set_t("01011101"));


  // Test COMPOUND Boolean Logic operators.
  bv = std::string("11111111");    CHECK(bv.KeepOnly(0,8) == set_t("11111111"));

  bv &= input1;       CHECK(bv.KeepOnly(0,8) == set_t("00001111"));
  bv &= input1;       CHECK(bv.KeepOnly(0,8) == set_t("00001111"));
  bv &= input2;       CHECK(bv.KeepOnly(0,8) == set_t("00000011"));
  bv &= input3;       CHECK(bv.KeepOnly(0,8) == set_t("00000001"));

  bv |= input1;       CHECK(bv.KeepOnly(0,8) == set_t("00001111"));
  bv |= input1;       CHECK(bv.KeepOnly(0,8) == set_t("00001111"));
  bv |= input3;       CHECK(bv.KeepOnly(0,8) == set_t("01011111"));
  bv |= input2;       CHECK(bv.KeepOnly(0,8) == set_t("01111111"));

  bv ^= input1;       CHECK(bv.KeepOnly(0,8) == set_t("01110000"));
  bv ^= input1;       CHECK(bv.KeepOnly(0,8) == set_t("01111111"));
  bv ^= input2;       CHECK(bv.KeepOnly(0,8) == set_t("01001100"));
  bv ^= input3;       CHECK(bv.KeepOnly(0,8) == set_t("00011001"));

  // Shifting tests.
  CHECK( (bv << 1) == set_t("00001100"));
  CHECK( (bv << 2) == set_t("00000110"));
  CHECK( (bv << 3) == set_t("00000011"));
  CHECK( (bv << 4) == set_t("00000001"));

  CHECK( (bv >> 1) == set_t("00110010"));
  CHECK( (bv >> 2) == set_t("01100100"));
  CHECK( (bv >> 3) == set_t("11001000"));
  CHECK( (bv >> 4) == set_t("10010000"));

  // Now some tests with bitvectors longer than one field.
  const set_t bvl80("00110111000101110001011100010111000101110001011100010111000101110001011100010111");
  CHECK( bvl80.GetSize() == 41 );
  CHECK( bvl80 << 1 ==
           set_t("000110111000101110001011100010111000101110001011100010111000101110001011100010111")
         );
  CHECK( bvl80 << 2 ==
           set_t("0000110111000101110001011100010111000101110001011100010111000101110001011100010111")
         );
  CHECK( bvl80 << 63 ==
           set_t("00000000000000000000000000000000000000000000000000000000000000000110111000101110001011100010111000101110001011100010111000101110001011100010111")
         );
  CHECK( bvl80 << 64 ==
           set_t("000000000000000000000000000000000000000000000000000000000000000000110111000101110001011100010111000101110001011100010111000101110001011100010111")
         );
  CHECK( bvl80 << 65 ==
           set_t("0000000000000000000000000000000000000000000000000000000000000000000110111000101110001011100010111000101110001011100010111000101110001011100010111")
         );

  CHECK( bvl80 >> 1 ==
           set_t("01101110001011100010111000101110001011100010111000101110001011100010111000101110")
         );
  CHECK( bvl80 >> 2 ==
           set_t("11011100010111000101110001011100010111000101110001011100010111000101110001011100")
         );
  CHECK( bvl80 >> 63 ==
           set_t("10001011100010111000000000000000000000000000000000000000000000000000000000000000")
         );
}