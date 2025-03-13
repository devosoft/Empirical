/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2024
*/
/**
 *  @file
 */

#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/math/ComboSet.hpp"

TEST_CASE("Test combos", "[math]")
{
  // constructor
  emp::ComboSet<10,3> cid;

  // accessors
  REQUIRE((cid.size() == 120));
  REQUIRE((cid.GetCombo().size() == 3));
  REQUIRE(cid.GetMaxCombo()[0] == 7); // Check maximum combo.
  REQUIRE(cid.GetMaxCombo()[1] == 8);
  REQUIRE(cid.GetMaxCombo()[2] == 9);

  // stepping through combos
  REQUIRE((cid[0] == 0));             // Check starting combo
  REQUIRE((cid[1] == 1));
  REQUIRE((cid[2] == 2));
  cid++;
  REQUIRE((cid.GetCombo()[0] == 0));  // Check first step into combos
  REQUIRE((cid.GetCombo()[1] == 1));
  REQUIRE((cid.GetCombo()[2] == 3));
  REQUIRE(cid.Next() == true);   // Make sure Next returns true/false correctly
  for(size_t i=0;i<9;i++){
    cid++; // test post fix ++
  }
  REQUIRE((cid[0] == 0));
  REQUIRE((cid[1] == 2));
  REQUIRE((cid[2] == 6));

  for(size_t i=0;i<108;i++){          // Step all the way to final combo
    ++cid; // test pre fix ++
  }
  REQUIRE(cid.GetCombo() == cid.GetMaxCombo());
  REQUIRE(cid.Next() == false);  // Flip-over should have Next() return false
  REQUIRE(cid[0] == 0);
  REQUIRE(cid[1] == 1);
  REQUIRE(cid[2] == 2);
}
