//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Tests for files in the tools/ folder.


#define EMP_DECORATE(X) [X]
#define EMP_DECORATE_PAIR(X,Y) [X-Y]
#define CATCH_CONFIG_MAIN


#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <sstream>
#include <string>

#include "constexpr/ce_string.h"

#include "tools/BitSet.h"

TEST_CASE("Test ce_string", "[constexpr]")
{
  constexpr emp::ce_string s = "abc";
  constexpr emp::ce_string s2 = "abc";
  constexpr emp::ce_string s3 = "abcdef";
  constexpr emp::ce_string s4 = "aba";
  emp::BitSet<s.size()> b1;
  emp::BitSet<(size_t) s[0]> b2;

  REQUIRE(b2.size() == 97);
  REQUIRE(s.size() == 3);

  constexpr bool x1 = (s == s2);
  constexpr bool x2 = (s != s2);
  constexpr bool x3 = (s < s2);
  constexpr bool x4 = (s > s2);
  constexpr bool x5 = (s <= s2);
  constexpr bool x6 = (s >= s2);

  REQUIRE(x1 == true);
  REQUIRE(x2 == false);
  REQUIRE(x3 == false);
  REQUIRE(x4 == false);
  REQUIRE(x5 == true);
  REQUIRE(x6 == true);

  constexpr bool y1 = (s == s3);
  constexpr bool y2 = (s != s3);
  constexpr bool y3 = (s < s3);
  constexpr bool y4 = (s > s3);
  constexpr bool y5 = (s <= s3);
  constexpr bool y6 = (s >= s3);

  REQUIRE(y1 == false);
  REQUIRE(y2 == true);
  REQUIRE(y3 == true);
  REQUIRE(y4 == false);
  REQUIRE(y5 == true);
  REQUIRE(y6 == false);

  constexpr bool z1 = (s == s4);
  constexpr bool z2 = (s != s4);
  constexpr bool z3 = (s < s4);
  constexpr bool z4 = (s > s4);
  constexpr bool z5 = (s <= s4);
  constexpr bool z6 = (s >= s4);

  REQUIRE(z1 == false);
  REQUIRE(z2 == true);
  REQUIRE(z3 == false);
  REQUIRE(z4 == true);
  REQUIRE(z5 == false);
  REQUIRE(z6 == true);
}
