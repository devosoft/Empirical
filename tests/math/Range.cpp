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
  REQUIRE(r1.Limit(0) == 5);

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
  REQUIRE(r2.CalcBin('g',26) == 6);

  // IntRange
  emp::Range<int> r3 = emp::IntRange(-5,5);
  REQUIRE(r3.GetLower() == -5);
  REQUIRE(r3.GetUpper() == 5);
  REQUIRE(r3.Limit(100) == 5);

  // DRange
  emp::Range<double> r4 = emp::DRange(0.1,23.5);
  REQUIRE(r4.GetLower() == 0.1);
  REQUIRE(r4.GetUpper() == 23.5);
  REQUIRE(!r4.Valid(0.0));
  REQUIRE(r4.CalcBin(4.7,5) == 0);
  REQUIRE(r4.CalcBin(4.8,5) == 1);
}
