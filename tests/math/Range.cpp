/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2021-23
*/
/**
 *  @file
 */

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/math/Range.hpp"

TEST_CASE("Test integral Ranges", "[math]")
{
  // Basic constructor
  [[maybe_unused]] emp::Range<int> rng;

  // GetLower GetUpper
  emp::Range<int> r(0,10);
  REQUIRE(r.GetLower() == 0);
  REQUIRE(r.GetUpper() == 10);
  REQUIRE(r.GetSize() == 11); // 0 through 10 inclusive.
  REQUIRE(r.Has(0) == true);
  REQUIRE(r.Has(10) == true);
  REQUIRE(r.Has(5) == true);
  REQUIRE(r.Has(-1) == false);
  REQUIRE(r.Has(11) == false);
  REQUIRE(r.Has(100000) == false);

  // CalcBin
  REQUIRE(r.CalcBin(5,10) == 5);
  REQUIRE(r.CalcBin(1,4) == 0);

  // Comparison
  emp::Range<int> r1 = r;
  REQUIRE(r1 == r);
  r1.SetLower(5);
  REQUIRE(r1 != r);

  // Valid Limit
  REQUIRE(!r1.Has(0));
  REQUIRE(r1.Clamp(0) == 5);

  // Spread
  emp::vector<int> s = r.Spread(6);
  int j=0;
  for(int i=0;i<6;i++){
    REQUIRE(s[i] == j);
    j+=2;
  }

  // SetLower SetUpper Set
  r1.SetLower(0);
  r1.SetUpper(15);
  r.Set(0,15);
  REQUIRE(r1 == r);
  REQUIRE(r.GetLower() == 0);
  REQUIRE(r.GetUpper() == 15);

  // SetMinLower SetMaxUpper
  r.SetMinLower();
  r.SetMaxUpper();
  REQUIRE(r.GetLower() == -2147483648);
  REQUIRE(r.GetUpper() == 2147483647);

  // MakeRange
  emp::Range<char> r2 = emp::MakeRange('a','z');
  REQUIRE(r2.GetLower() == 'a');
  REQUIRE(r2.GetUpper() == 'z');
  REQUIRE(r2.GetSize() == 26);
  REQUIRE(r2.CalcBin('g',26) == 6);
  REQUIRE(r2.Has('a') == true);
  REQUIRE(r2.Has('j') == true);
  REQUIRE(r2.Has('z') == true);
  REQUIRE(r2.Has('A') == false);
  REQUIRE(r2.Has('-') == false);
  REQUIRE(r2.Has('\n') == false);

  // IntRange
  emp::Range<int> r3 = emp::IntRange(-5,5);
  REQUIRE(r3.GetLower() == -5);
  REQUIRE(r3.GetUpper() == 5);
  REQUIRE(r3.GetSize() == 11);
  REQUIRE(r3.Clamp(100) == 5);
}

TEST_CASE("Test Range with non-inclusive endpoint", "[math]")
{
  // Basic constructor
  [[maybe_unused]] emp::Range<int, false> rng;

  // GetLower GetUpper
  emp::Range<int, false> r(0,10);
  REQUIRE(r.GetLower() == 0);
  REQUIRE(r.GetUpper() == 10);
  REQUIRE(r.GetSize() == 10); // 0 through 10 exclusive.
  REQUIRE(r.Has(0) == true);
  REQUIRE(r.Has(10) == false);
  REQUIRE(r.Has(5) == true);
  REQUIRE(r.Has(-1) == false);
  REQUIRE(r.Has(11) == false);
  REQUIRE(r.Has(100000) == false);

  // CalcBin
  REQUIRE(r.CalcBin(5,10) == 5);
  REQUIRE(r.CalcBin(1,4) == 0);

  // Comparison
  emp::Range<int, false> r1 = r;
  REQUIRE(r1 == r);
  r1.SetLower(5);
  REQUIRE(r1 != r);

  // Valid Limit
  REQUIRE(!r1.Has(0));
  REQUIRE(r1.Clamp(0) == 5);

  // Spread
  emp::vector<int> s = r.Spread(6);
  int j=0;
  for(int i=0;i<6;i++){
    REQUIRE(s[i] == j);
    j+=2;
  }

  // SetLower SetUpper Set
  r1.SetLower(0);
  r1.SetUpper(15);
  r.Set(0,15);
  REQUIRE(r1 == r);
  REQUIRE(r.GetLower() == 0);
  REQUIRE(r.GetUpper() == 15);

  // SetMinLower SetMaxUpper
  r.SetMinLower();
  r.SetMaxUpper();
  REQUIRE(r.GetLower() == -2147483648);
  REQUIRE(r.GetUpper() == 2147483647);

  // MakeRange
  emp::Range<char, false> r2 = emp::MakeRange<char,false>('a','z');
  REQUIRE(r2.GetLower() == 'a');
  REQUIRE(r2.GetUpper() == 'z');
  REQUIRE(r2.GetSize() == 25);   // 'z' is excluded.
  REQUIRE(r2.CalcBin('g',26) == 6);
  REQUIRE(r2.Has('a') == true);
  REQUIRE(r2.Has('j') == true);
  REQUIRE(r2.Has('z') == false);
  REQUIRE(r2.Has('A') == false);
  REQUIRE(r2.Has('-') == false);
  REQUIRE(r2.Has('\n') == false);

  // IntRange
  emp::Range<int, false> r3 = emp::IntRange<false>(-5,5);
  REQUIRE(r3.GetLower() == -5);
  REQUIRE(r3.GetUpper() == 5);
  REQUIRE(r3.GetSize() == 10);
  REQUIRE(r3.Clamp(100) == 4);
}

TEST_CASE("Test continuous ranges", "[math]")
{
  // DRange
  emp::Range<double> dr = emp::DRange(0.1,23.5);
  REQUIRE(dr.GetLower() == 0.1);
  REQUIRE(dr.GetUpper() == 23.5);
  REQUIRE(dr.GetSize() > 23.3999); REQUIRE(dr.GetSize() < 23.4001);
  REQUIRE(!dr.Has(0.0));
  REQUIRE(dr.CalcBin(4.7,5) == 0);
  REQUIRE(dr.CalcBin(4.8,5) == 1);
  REQUIRE(dr.Has(0.1) == true);
  REQUIRE(dr.Has(23.5) == true);
  REQUIRE(dr.Has(10.0) == true);
  REQUIRE(dr.Has(0.09) == false);
  REQUIRE(dr.Has(23.6) == false);
  REQUIRE(dr.Has(0.000000001) == false);
  REQUIRE(dr.Clamp(23.6) == 23.5);

  // DRange with excluded endpoint.
  emp::Range<double, false> dr2 = emp::DRange<false>(0.1,23.5);
  REQUIRE(dr2.GetLower() == 0.1);
  REQUIRE(dr2.GetUpper() == 23.5);
  REQUIRE(dr2.GetSize() > 23.3999); REQUIRE(dr2.GetSize() < 23.4001);
  REQUIRE(!dr2.Has(0.0));
  REQUIRE(dr2.CalcBin(4.7,5) == 0);
  REQUIRE(dr2.CalcBin(4.8,5) == 1);
  REQUIRE(dr2.Has(0.1) == true);
  REQUIRE(dr2.Has(23.5) == false);
  REQUIRE(dr2.Has(10.0) == true);
  REQUIRE(dr2.Has(0.09) == false);
  REQUIRE(dr2.Has(23.6) == false);
  REQUIRE(dr2.Has(0.000000001) == false);
  REQUIRE(dr2.Clamp(23.6) < 23.5);
}

TEST_CASE("Test Range constructors", "[math]")
{
  // Base constructor.
  emp::Range<int, true> r1;
  REQUIRE(r1.Has(100000) == true);   // Should have a large range.
  REQUIRE(r1.Has(-100000) == true);  // Should have a large negative range too.
  r1.Set(-100, 200000);
  REQUIRE(r1.Has(100000) == true);
  REQUIRE(r1.Has(-100000) == false);
  r1.Set(-100, 200);
  REQUIRE(r1.Has(100000) == false);
  REQUIRE(r1.Has(-100000) == false);

  // Single-value constructor.
  emp::Range<char> r2('j');
  REQUIRE(r2.Has('i') == false);
  REQUIRE(r2.Has('j') == true);
  REQUIRE(r2.Has('k') == false);
  REQUIRE(r2.GetLower() == 'j');
  REQUIRE(r2.GetUpper() == 'j');

  // Single-value constructor exclusive of limit.
  emp::Range<char, false> r3('j');
  REQUIRE(r3.Has('i') == false);
  REQUIRE(r3.Has('j') == true);
  REQUIRE(r3.Has('k') == false);
  REQUIRE(r3.GetLower() == 'j');
  REQUIRE(r3.GetUpper() == 'k');

  // Single-value constructor with continuous value, exclusive of limit.
  emp::Range<double, false> r4(12345.67);
  REQUIRE(r4.Has(12345.66) == false);
  REQUIRE(r4.Has(12345.67) == true);
  REQUIRE(r4.Has(12345.68) == false);
  REQUIRE(r4.GetLower() == 12345.67);
  REQUIRE(r4.GetUpper() > 12345.67);

  // Let's try a string-base range.
  emp::Range<std::string> r5("abc", "def");
  REQUIRE(r5.Has("aardvark") == false);
  REQUIRE(r5.Has("beta") == true);
  REQUIRE(r5.Has("central") == true);
  REQUIRE(r5.Has("divided") == false);
}
