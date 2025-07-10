/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2025
*/
/**
 *  @file
 */

#include <iostream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/tools/GridSize.hpp"


TEST_CASE("Test GridPos", "[tools]")
{
  emp::GridPos gp1;       // Default Constructor
  emp::GridPos gp2(4,5);  // Direct Constructor
  emp::GridPos gp3(0,0);
  emp::GridPos gp4(gp2);  // Copy Constructor

  // Test basic comparisons.
  CHECK(gp1 == gp1);
  CHECK(gp1 != gp2);
  CHECK(gp1 == gp3);
  CHECK(gp1 != gp4);
  CHECK(gp2 == gp4);

  gp4 = emp::GridPos{3,4};
  CHECK(gp2 != gp4);
  gp2 = gp4;
  CHECK(gp2 == gp4);

  // Test Row() and Col()
  CHECK(gp2.Row() == 3);
  CHECK(gp2.Col() == 4);

  gp4 = emp::GridPos(1000, 2000);
  CHECK(gp4.Row() == 1000);
  CHECK(gp4.Col() == 2000);

  // SRow() and SCol() should return signed values.
  CHECK(gp2.SRow() - 5 == -2);
  CHECK(gp2.SCol() - 5 == -1);

  // Test dealing with invalid coordinates.
  emp::GridPos gp_inv = emp::GridPos::MakeInvalid();

  CHECK(gp_inv.IsValid() == false);
  CHECK(gp_inv.OK() == true);
  CHECK(gp1 != gp_inv);

  gp1 = gp_inv;

  CHECK(gp1 == gp_inv);
  CHECK(gp1.IsValid() == false);
  CHECK(gp1.OK() == true);

  // Test more direct setting of values.
  gp1.Set(7,9);
  CHECK(gp1.Row() == 7);
  CHECK(gp1.Col() == 9);

  gp1.SetRow(11);
  CHECK(gp1.Row() == 11);
  CHECK(gp1.Col() == 9);

  gp1.SetCol(2);
  CHECK(gp1.Row() == 11);
  CHECK(gp1.Col() == 2);

  gp1.Set(0, 0);
  CHECK(gp1.Row() == 0);
  CHECK(gp1.Col() == 0);

  // Other point tests.
  CHECK(gp1.AtOrigin() == true);
  CHECK(gp2.AtOrigin() == false);
  CHECK(gp_inv.AtOrigin() == false);

  CHECK(gp1.NonZero() == false);
  CHECK(gp2.NonZero() == true);
  CHECK(gp_inv.NonZero() == true);

  // Test math.
  CHECK(gp1.GetOffset(8, 8) == emp::GridPos{8, 8});
  CHECK(gp2.GetOffset(8, 8) == emp::GridPos{11, 12});
  CHECK(gp2.GetOffset(-2, -2) == emp::GridPos{1, 2});

  CHECK(gp2 + emp::GridPos{100, 200} == emp::GridPos{103, 204});
  gp3 = gp4 - emp::GridPos{111, 222};
  CHECK(gp3 == emp::GridPos{889, 1778});
  gp2 = gp3 % emp::GridPos{100,1000};
  CHECK(gp2 == emp::GridPos{89, 778});

  gp2.Offset(gp3);
  CHECK(gp2 == emp::GridPos{978, 2556});

  gp2.Offset(-500, 100);
  CHECK(gp2 == emp::GridPos{478, 2656});

  // Test bounding
  emp::GridPos bound{700, 2000};
  gp2.BoundLower(bound);
  CHECK(gp2 == emp::GridPos{700, 2656});
  gp2.BoundUpper(bound);
  CHECK(gp2 == emp::GridPos{700, 2000});

  gp3.BoundUpper(bound);
  CHECK(gp3 == emp::GridPos{700, 1778});
  gp3.BoundLower(bound);
  CHECK(gp3 == emp::GridPos{700, 2000});
 
  gp3 += emp::GridPos{7, 13};
  CHECK(gp3 == emp::GridPos{707, 2013});

  gp3 -= emp::GridPos{20, 30};
  CHECK(gp3 == emp::GridPos{687, 1983});

  gp3 %= emp::GridPos{25, 100};
  CHECK(gp3 == emp::GridPos{12, 83});
}
