/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file Range.cpp
 */

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/math/Range.hpp"

TEST_CASE("Test Range", "[math]")
{
  // Basic constructor
  [[maybe_unused]] emp::Range<int> rng;

  // GetLower GetUpper
  emp::Range<int> r(0,10);
  REQUIRE(r.GetLower() == 0);
  REQUIRE(r.GetUpper() == 10);
  REQUIRE(r.GetSize() == 11); // 0 through 10 inclusive.
  REQUIRE(r.Valid(0) == true);
  REQUIRE(r.Valid(10) == true);
  REQUIRE(r.Valid(5) == true);
  REQUIRE(r.Valid(-1) == false);
  REQUIRE(r.Valid(11) == false);
  REQUIRE(r.Valid(100000) == false);

  // CalcBin
  REQUIRE(r.CalcBin(5,10) == 5);
  REQUIRE(r.CalcBin(1,4) == 0);

  // Comparison
  emp::Range<int> r1 = r;
  REQUIRE(r1 == r);
  r1.SetLower(5);
  REQUIRE(r1 != r);

  // Valid Limit
  REQUIRE(!r1.Valid(0));
  REQUIRE(r1.LimitValue(0) == 5);

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

  // SetMaxLower SetMaxUpper
  r.SetMaxLower();
  r.SetMaxUpper();
  REQUIRE(r.GetLower() == -2147483648);
  REQUIRE(r.GetUpper() == 2147483647);

  // MakeRange
  emp::Range<char> r2 = emp::MakeRange('a','z');
  REQUIRE(r2.GetLower() == 'a');
  REQUIRE(r2.GetUpper() == 'z');
  REQUIRE(r2.GetSize() == 26);
  REQUIRE(r2.CalcBin('g',26) == 6);
  REQUIRE(r2.Valid('a') == true);
  REQUIRE(r2.Valid('j') == true);
  REQUIRE(r2.Valid('z') == true);
  REQUIRE(r2.Valid('A') == false);
  REQUIRE(r2.Valid('-') == false);
  REQUIRE(r2.Valid('\n') == false);

  // IntRange
  emp::Range<int> r3 = emp::IntRange(-5,5);
  REQUIRE(r3.GetLower() == -5);
  REQUIRE(r3.GetUpper() == 5);
  REQUIRE(r3.GetSize() == 11);
  REQUIRE(r3.LimitValue(100) == 5);

  // DRange
  emp::Range<double> r4 = emp::DRange(0.1,23.5);
  REQUIRE(r4.GetLower() == 0.1);
  REQUIRE(r4.GetUpper() == 23.5);
  REQUIRE(r4.GetSize() > 23.3999); REQUIRE(r4.GetSize() < 23.4001);
  REQUIRE(!r4.Valid(0.0));
  REQUIRE(r4.CalcBin(4.7,5) == 0);
  REQUIRE(r4.CalcBin(4.8,5) == 1);
  REQUIRE(r4.Valid(0.1) == true);
  REQUIRE(r4.Valid(23.5) == true);
  REQUIRE(r4.Valid(10.0) == true);
  REQUIRE(r4.Valid(0.09) == false);
  REQUIRE(r4.Valid(23.6) == false);
  REQUIRE(r4.Valid(0.000000001) == false);
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
  REQUIRE(r.Valid(0) == true);
  REQUIRE(r.Valid(10) == false);
  REQUIRE(r.Valid(5) == true);
  REQUIRE(r.Valid(-1) == false);
  REQUIRE(r.Valid(11) == false);
  REQUIRE(r.Valid(100000) == false);

  // CalcBin
  REQUIRE(r.CalcBin(5,10) == 5);
  REQUIRE(r.CalcBin(1,4) == 0);

  // Comparison
  emp::Range<int, false> r1 = r;
  REQUIRE(r1 == r);
  r1.SetLower(5);
  REQUIRE(r1 != r);

  // Valid Limit
  REQUIRE(!r1.Valid(0));
  REQUIRE(r1.LimitValue(0) == 5);

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

  // SetMaxLower SetMaxUpper
  r.SetMaxLower();
  r.SetMaxUpper();
  REQUIRE(r.GetLower() == -2147483648);
  REQUIRE(r.GetUpper() == 2147483647);

  // MakeRange
  emp::Range<char, false> r2 = emp::MakeRange<char,false>('a','z');
  REQUIRE(r2.GetLower() == 'a');
  REQUIRE(r2.GetUpper() == 'z');
  REQUIRE(r2.GetSize() == 25);   // 'z' is excluded.
  REQUIRE(r2.CalcBin('g',26) == 6);
  REQUIRE(r2.Valid('a') == true);
  REQUIRE(r2.Valid('j') == true);
  REQUIRE(r2.Valid('z') == false);
  REQUIRE(r2.Valid('A') == false);
  REQUIRE(r2.Valid('-') == false);
  REQUIRE(r2.Valid('\n') == false);

  // IntRange
  emp::Range<int, false> r3 = emp::IntRange<false>(-5,5);
  REQUIRE(r3.GetLower() == -5);
  REQUIRE(r3.GetUpper() == 5);
  REQUIRE(r3.GetSize() == 10);
  REQUIRE(r3.LimitValue(100) == 5);

  // DRange
  emp::Range<double, false> r4 = emp::DRange<false>(0.1,23.5);
  REQUIRE(r4.GetLower() == 0.1);
  REQUIRE(r4.GetUpper() == 23.5);
  REQUIRE(r4.GetSize() > 23.3999); REQUIRE(r4.GetSize() < 23.4001);
  REQUIRE(!r4.Valid(0.0));
  REQUIRE(r4.CalcBin(4.7,5) == 0);
  REQUIRE(r4.CalcBin(4.8,5) == 1);
  REQUIRE(r4.Valid(0.1) == true);
  REQUIRE(r4.Valid(23.5) == false);
  REQUIRE(r4.Valid(10.0) == true);
  REQUIRE(r4.Valid(0.09) == false);
  REQUIRE(r4.Valid(23.6) == false);
  REQUIRE(r4.Valid(0.000000001) == false);
}