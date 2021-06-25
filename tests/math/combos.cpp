#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/math/combos.hpp"

#include <sstream>

TEST_CASE("Test combos", "[math]")
{
  // constructor
  emp::ComboIDs cid(10, 3);

  // accessors
  REQUIRE((cid.GetNumCombos() == 120));
  REQUIRE((cid.size() == 120));
  REQUIRE((cid.GetComboSize() == 3));
  int first_max_combo = 7;
  for(size_t i=0;i<cid.GetComboSize();i++){
    REQUIRE(cid.GetMaxCombo()[i] == first_max_combo+i);
  }

  // stepping through combos
  REQUIRE((cid[0] == 0));
  REQUIRE((cid[1] == 1));
  REQUIRE((cid[2] == 2));
  cid++;
  REQUIRE((cid.GetCombo()[0] == 0));
  REQUIRE((cid.GetCombo()[1] == 1));
  REQUIRE((cid.GetCombo()[2] == 3));
  for(size_t i=0;i<10;i++){
    cid++; // test post fix ++
  }
  REQUIRE((cid.GetCombo()[0] == 0));
  REQUIRE((cid.GetCombo()[1] == 2));
  REQUIRE((cid.GetCombo()[2] == 6));

  for(size_t i=0;i<108;i++){
    ++cid; // test pre fix ++
  }
  REQUIRE(cid.GetCombo() == cid.GetMaxCombo());
  cid++;
  REQUIRE(cid.GetCombo()[0] == 0);
  REQUIRE(cid.GetCombo()[1] == 1);
  REQUIRE(cid.GetCombo()[2] == 2);

  // resize
  cid.ResizeCombos(2);
  REQUIRE(cid.size() == 45);
  REQUIRE(cid.GetCombo()[0] == 0);
  REQUIRE(cid.GetCombo()[1] == 1);

  // inverse combo 2 3 4 5 6 7 8 9 ( everything not in cur combo )
  int first_inverse = 2;
  for(size_t i=0;i<8;i++){
    REQUIRE(cid.GetInverseCombo()[i] == first_inverse + i);
  }
}