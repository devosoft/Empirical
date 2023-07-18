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

  rs1.Insert(27);

  CHECK(rs1.Has(26) == false);
  CHECK(rs1.Has(27) == true);
  CHECK(rs1.Has(28) == false);
  CHECK(rs1.Has(-1003) == false);
  CHECK(rs1.Has(0) == false);

  rs1.Insert(28);

  CHECK(rs1.Has(26) == false);
  CHECK(rs1.Has(27) == true);
  CHECK(rs1.Has(28) == true);
  CHECK(rs1.Has(29) == false);
  CHECK(rs1.Has(-1003) == false);
  CHECK(rs1.Has(0) == false);

  rs1.Insert(26);

  CHECK(rs1.Has(25) == false);
  CHECK(rs1.Has(26) == true);
  CHECK(rs1.Has(27) == true);
  CHECK(rs1.Has(28) == true);
  CHECK(rs1.Has(29) == false);
  CHECK(rs1.Has(-1003) == false);
  CHECK(rs1.Has(0) == false);

  rs1.Insert(emp::Range<int,false>(23,26));

  // Make sure RangeSets are identified as equal, even if constructed differently.
  emp::RangeSet<int> rs2(emp::Range<int,false>{23,29});
  CHECK(rs1 == rs2);
}
