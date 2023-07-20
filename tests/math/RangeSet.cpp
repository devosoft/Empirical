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
  emp::RangeSet<double>::range_t range(0.4, 0.9);

  rs1.InsertRange(0.0, 100.0);
  CHECK(rs1.GetSize() == Approx(100.0));

  CHECK(rs1.Remove(range) == true);
  CHECK(rs1.GetSize() == Approx(99.5));
  CHECK(rs1.Remove(range) == false);
  CHECK(rs1.GetSize() == Approx(99.5));

  for (size_t i = 0; i < 100; ++i) {
    range.Shift(1.0);
    rs1.Remove(range);  // Only the first 99 should matter -- then we're out of range.)
  }
  // From 0.0 to 100.0 we should have gaps from every *.4 to *.9

  CHECK(rs1.GetSize() == Approx(50.0));
  CHECK(rs1.GetNumRanges() == 101);

  // Remove exact range.
  CHECK(rs1.RemoveRange(1.9, 2.4) == true);
  CHECK(rs1.GetSize() == Approx(49.5));
  CHECK(rs1.GetNumRanges() == 100);

  // Remove range + gap before
  CHECK(rs1.RemoveRange(3.4, 4.4) == true);
  CHECK(rs1.GetSize() == Approx(49.0));
  CHECK(rs1.GetNumRanges() == 99);

  // Remove range + gap after
  CHECK(rs1.RemoveRange(5.9, 6.9) == true);
  CHECK(rs1.GetSize() == Approx(48.5));
  CHECK(rs1.GetNumRanges() == 98);

  // Remove range + both gaps
  CHECK(rs1.RemoveRange(7.4, 8.9) == true);
  CHECK(rs1.GetSize() == Approx(48.0));
  CHECK(rs1.GetNumRanges() == 97);

  // Remove range + to middle of both gaps
  CHECK(rs1.RemoveRange(10.6, 11.7) == true);
  CHECK(rs1.GetSize() == Approx(47.5));
  CHECK(rs1.GetNumRanges() == 96);

  // Remove middle of one range through middle of next.
  // Remove middle of one range through end of next.
  // Remove beginning of one range through middle of next.
  // Remove beginning of one range through end of next.

  // Remove middle section of one range only
  
  // Remove middle of one range through middle of two later.
  // Remove middle of one range through middle of three later.

}