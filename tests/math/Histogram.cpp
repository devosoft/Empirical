/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2026
*/
/**
 *  @file
 */

#include <cmath>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/math/Histogram.hpp"

TEST_CASE("Histogram basics", "[math]")
{
  emp::Histogram h;

  // Empty state
  REQUIRE( h.size()      == 0 );
  REQUIRE( h.GetNumBins() == 0 );
  REQUIRE( h.GetNumVals() == 0 );
  REQUIRE( h.empty() );

  // Out-of-bounds Get returns 0
  REQUIRE( h.Get(5) == 0 );
  REQUIRE( h[3]     == 0 );

  // Single insert grows the vector and tracks values
  h.Insert(3);
  REQUIRE( h.GetNumBins() > 3 );
  REQUIRE( h.GetNumVals() == 1 );
  REQUIRE( h[3] == 1 );
  REQUIRE( h[0] == 0 );

  // Insert at same bin
  h.Insert(3);
  h.Insert(3);
  REQUIRE( h[3] == 3 );
  REQUIRE( h.GetNumVals() == 3 );

  // Insert extends the vector
  h.Insert(7);
  REQUIRE( h.GetNumBins() > 7 );
  REQUIRE( h[7] == 1 );
  REQUIRE( h.GetNumVals() == 4 );

  // Remove decrements
  h.Remove(3);
  REQUIRE( h[3] == 2 );
  REQUIRE( h.GetNumVals() == 3 );

  // Reset clears everything
  h.Reset();
  REQUIRE( h.GetNumBins() == 0 );
  REQUIRE( h.GetNumVals() == 0 );
  REQUIRE( h.empty() );
}

TEST_CASE("Histogram Min and Max", "[math]")
{
  emp::Histogram h;

  // Empty edge cases
  REQUIRE( h.Max() == 0 );

  h.Insert(0);
  REQUIRE( h.Min() == 0 );
  REQUIRE( h.Max() == 0 );

  h.Insert(5);
  REQUIRE( h.Min() == 0 );
  REQUIRE( h.Max() == 5 );

  h.Remove(0);
  REQUIRE( h.Min() == 5 );  // bin 0 is now empty; first value is at 5

  // Shrink: trailing zeros don't inflate Max
  h.Insert(10);
  h.Remove(10);
  REQUIRE( h.Max() == 5 );
}

TEST_CASE("Histogram operator+= and operator+", "[math]")
{
  emp::Histogram a, b;
  a.Insert(1); a.Insert(2); a.Insert(2);   // bin 1: 1, bin 2: 2
  b.Insert(2); b.Insert(3);               // bin 2: 1, bin 3: 1

  a += b;
  REQUIRE( a.GetNumVals() == 5 );
  REQUIRE( a[1] == 1 );
  REQUIRE( a[2] == 3 );
  REQUIRE( a[3] == 1 );

  // operator+ leaves originals intact
  emp::Histogram c, d;
  c.Insert(0); c.Insert(0);
  d.Insert(4);
  emp::Histogram e = c + d;
  REQUIRE( e.GetNumVals() == 3 );
  REQUIRE( e[0] == 2 );
  REQUIRE( e[4] == 1 );
  REQUIRE( c.GetNumVals() == 2 );  // unchanged
  REQUIRE( d.GetNumVals() == 1 );  // unchanged
}

TEST_CASE("Histogram FindPosition", "[math]")
{
  emp::Histogram h;
  // counts: bin 0 = 2, bin 1 = 3, bin 2 = 1  →  values [0,0, 1,1,1, 2]
  h.Insert(0); h.Insert(0);
  h.Insert(1); h.Insert(1); h.Insert(1);
  h.Insert(2);

  REQUIRE( h.FindPosition(0) == 0 );  // 1st value is at bin 0
  REQUIRE( h.FindPosition(1) == 0 );  // 2nd value is at bin 0
  REQUIRE( h.FindPosition(2) == 1 );  // 3rd value is at bin 1
  REQUIRE( h.FindPosition(4) == 1 );  // 5th value is at bin 1
  REQUIRE( h.FindPosition(5) == 2 );  // 6th value is at bin 2

  // Bin with 0 counts at the front is skipped
  emp::Histogram h2;
  h2.Insert(3); h2.Insert(3); h2.Insert(5);
  REQUIRE( h2.FindPosition(0) == 3 );  // 1st value is at bin 3
  REQUIRE( h2.FindPosition(2) == 5 );  // 3rd value is at bin 5
}

TEST_CASE("Histogram CalcMean", "[math]")
{
  emp::Histogram h;

  REQUIRE( std::isnan(h.CalcMean()) );  // empty

  h.Insert(0);
  REQUIRE( h.CalcMean() == Approx(0.0) );

  h.Insert(4);
  REQUIRE( h.CalcMean() == Approx(2.0) );  // (0 + 4) / 2

  // counts: bin 1 = 2, bin 3 = 2  →  mean = (1+1+3+3)/4 = 2
  emp::Histogram h2;
  h2.Insert(1); h2.Insert(1); h2.Insert(3); h2.Insert(3);
  REQUIRE( h2.CalcMean() == Approx(2.0) );

  // counts: bin 0=1, bin 1=2, bin 2=3  → (0 + 1 + 1 + 2 + 2 + 2) / 6 = 8/6
  emp::Histogram h3;
  h3.Insert(0);
  h3.Insert(1); h3.Insert(1);
  h3.Insert(2); h3.Insert(2); h3.Insert(2);
  REQUIRE( h3.CalcMean() == Approx(8.0 / 6.0) );
}

TEST_CASE("Histogram CalcMedian", "[math]")
{
  emp::Histogram h;
  REQUIRE( std::isnan(h.CalcMedian()) );  // empty

  // === Odd number of values ===

  // n=1: single value at bin 5
  h.Insert(5);
  REQUIRE( h.CalcMedian() == Approx(5.0) );
  h.Reset();

  // n=3: values at bins 1, 2, 3 → median = 2
  h.Insert(1); h.Insert(2); h.Insert(3);
  REQUIRE( h.CalcMedian() == Approx(2.0) );
  h.Reset();

  // n=5: [1, 2, 2, 2, 3] → median = 2 (0-indexed position 2)
  h.Insert(1); h.Insert(2); h.Insert(2); h.Insert(2); h.Insert(3);
  REQUIRE( h.CalcMedian() == Approx(2.0) );
  h.Reset();

  // n=5 with gap: [1, 1, 3, 3, 3] → median = bin of 3rd value = 3
  h.Insert(1); h.Insert(1); h.Insert(3); h.Insert(3); h.Insert(3);
  REQUIRE( h.CalcMedian() == Approx(3.0) );
  h.Reset();

  // === Even number of values ===

  // n=2: values at 1 and 3 → median = 2.0
  h.Insert(1); h.Insert(3);
  REQUIRE( h.CalcMedian() == Approx(2.0) );
  h.Reset();

  // n=4: [1, 1, 3, 3] → median = (1+3)/2 = 2.0
  h.Insert(1); h.Insert(1); h.Insert(3); h.Insert(3);
  REQUIRE( h.CalcMedian() == Approx(2.0) );
  h.Reset();

  // n=4: [2, 2, 2, 4] → median = (2+2)/2 = 2.0
  h.Insert(2); h.Insert(2); h.Insert(2); h.Insert(4);
  REQUIRE( h.CalcMedian() == Approx(2.0) );
  h.Reset();

  // n=6: [0, 0, 1, 2, 3, 3] → median = (1+2)/2 = 1.5
  h.Insert(0); h.Insert(0);
  h.Insert(1);
  h.Insert(2);
  h.Insert(3); h.Insert(3);
  REQUIRE( h.CalcMedian() == Approx(1.5) );
}

TEST_CASE("Histogram CalcMode", "[math]")
{
  emp::Histogram h;
  REQUIRE( std::isnan(h.CalcMode()) );  // empty

  h.Insert(2); h.Insert(2); h.Insert(2);
  h.Insert(4); h.Insert(4);
  REQUIRE( h.CalcMode() == Approx(2.0) );  // bin 2 has the most

  h.Insert(4); h.Insert(4); h.Insert(4);  // now bin 4 has the most
  REQUIRE( h.CalcMode() == Approx(4.0) );
}

// Local settings for Empecable file checker.
// empecable_words: CalcMode CalcMean CalcMedian FindPosition isnan
