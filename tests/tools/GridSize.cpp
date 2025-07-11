/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2025
*/
/**
 *  @file
 */

#include <iostream>
#include <vector>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/base/vector.hpp"
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
  emp::GridPos gp_inv = emp::GridSize::InvalidPos();

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

TEST_CASE("Test GridSize", "[tools]")
{
  emp::GridSize gs0;

  CHECK(gs0.NumRows() == 0);
  CHECK(gs0.NumCols() == 0);
  CHECK(gs0.NumCells() == 0);

  emp::GridSize gs1{10,11};

  CHECK(gs1.NumRows() == 10);
  CHECK(gs1.NumCols() == 11);
  CHECK(gs1.NumCells() == 110);

  emp::GridSize gs2{gs1};


  CHECK(gs2.NumRows() == 10);
  CHECK(gs2.NumCols() == 11);
  CHECK(gs2.NumCells() == 110);

  emp::vector< emp::vector<int> > matrix(20, emp::vector<int>(7,2));
  emp::GridSize gs3(matrix);

  CHECK(gs3.NumRows() == 20);
  CHECK(gs3.NumCols() == 7);
  CHECK(gs3.NumCells() == 140);

  std::vector< std::vector<int> > matrix2(13, std::vector<int>(14,3));
  emp::GridSize gs4(matrix2);

  CHECK(gs4.NumRows() == 13);
  CHECK(gs4.NumCols() == 14);
  CHECK(gs4.NumCells() == 182);

  // Check position testing.
  emp::GridPos test_pos{15, 9};

  CHECK(gs0.IsInside(test_pos) == false);
  CHECK(gs1.IsInside(test_pos) == false);
  CHECK(gs2.IsInside(test_pos) == false);
  CHECK(gs3.IsInside(test_pos) == false);
  CHECK(gs4.IsInside(test_pos) == false);

  test_pos.Set(4,4);

  CHECK(gs0.IsInside(test_pos) == false);
  CHECK(gs1.IsInside(test_pos) == true);
  CHECK(gs2.IsInside(test_pos) == true);
  CHECK(gs3.IsInside(test_pos) == true);
  CHECK(gs4.IsInside(test_pos) == true);

  int top_count = 0;
  int bottom_count = 0;
  int left_count = 0;
  int right_count = 0;
  int cell_count = 0;
  for (size_t row = 0; row < 30; ++row) {
    for (size_t col = 0; col < 30; ++col) {
      test_pos.Set(row, col);
      if (gs3.AtTopEdge(test_pos)) top_count++;
      if (gs3.AtBottomEdge(test_pos)) bottom_count++;
      if (gs3.AtLeftEdge(test_pos)) left_count++;
      if (gs3.AtRightEdge(test_pos)) right_count++;
      if (gs3.IsInside(test_pos)) cell_count++;
    }
  }

  CHECK(top_count == 7);
  CHECK(bottom_count == 7);
  CHECK(left_count == 20);
  CHECK(right_count == 20);
  CHECK(cell_count == 140);

  // Check index conversions
  CHECK(gs1.ToIndex({2,2}) == 24);
  CHECK(gs2.ToIndex({2,2}) == 24);
  CHECK(gs3.ToIndex({2,2}) == 16);
  CHECK(gs4.ToIndex({2,2}) == 30);

  CHECK(gs1.FromIndex(50) == emp::GridPos{4,6});
  CHECK(gs2.FromIndex(50) == emp::GridPos{4,6});
  CHECK(gs3.FromIndex(50) == emp::GridPos{7,1});
  CHECK(gs4.FromIndex(50) == emp::GridPos{3,8});

  // Relative positioning
  test_pos.Set(3,3);
  CHECK(gs1.PosUp(test_pos) == emp::GridPos{2,3});
  CHECK(gs1.PosDown(test_pos) == emp::GridPos{4,3});
  CHECK(gs1.PosLeft(test_pos) == emp::GridPos{3,2});
  CHECK(gs1.PosRight(test_pos) == emp::GridPos{3,4});
  CHECK(gs1.PosUL(test_pos) == emp::GridPos{2,2});
  CHECK(gs1.PosUR(test_pos) == emp::GridPos{2,4});
  CHECK(gs1.PosDL(test_pos) == emp::GridPos{4,2});
  CHECK(gs1.PosDR(test_pos) == emp::GridPos{4,4});

  test_pos.Set(0,0);  // Top Left
  CHECK(gs1.PosUp(test_pos) == emp::GridSize::InvalidPos());
  CHECK(gs1.PosDown(test_pos) == emp::GridPos{1,0});
  CHECK(gs1.PosLeft(test_pos) == emp::GridSize::InvalidPos());
  CHECK(gs1.PosRight(test_pos) == emp::GridPos{0,1});
  CHECK(gs1.PosUL(test_pos) == emp::GridSize::InvalidPos());
  CHECK(gs1.PosUR(test_pos) == emp::GridSize::InvalidPos());
  CHECK(gs1.PosDL(test_pos) == emp::GridSize::InvalidPos());
  CHECK(gs1.PosDR(test_pos) == emp::GridPos{1,1});

  test_pos.Set(9,0);  // Bottom Left
  CHECK(gs1.PosUp(test_pos) == emp::GridPos{8,0});
  CHECK(gs1.PosDown(test_pos) == emp::GridSize::InvalidPos());
  CHECK(gs1.PosLeft(test_pos) == emp::GridSize::InvalidPos());
  CHECK(gs1.PosRight(test_pos) == emp::GridPos{9,1});
  CHECK(gs1.PosUL(test_pos) == emp::GridSize::InvalidPos());
  CHECK(gs1.PosUR(test_pos) == emp::GridPos{8,1});
  CHECK(gs1.PosDL(test_pos) == emp::GridSize::InvalidPos());
  CHECK(gs1.PosDR(test_pos) == emp::GridSize::InvalidPos());

  test_pos.Set(0,10);  // Top Right
  CHECK(gs1.PosUp(test_pos) == emp::GridSize::InvalidPos());
  CHECK(gs1.PosDown(test_pos) == emp::GridPos{1,10});
  CHECK(gs1.PosLeft(test_pos) == emp::GridPos{0,9});
  CHECK(gs1.PosRight(test_pos) == emp::GridSize::InvalidPos());
  CHECK(gs1.PosUL(test_pos) == emp::GridSize::InvalidPos());
  CHECK(gs1.PosUR(test_pos) == emp::GridSize::InvalidPos());
  CHECK(gs1.PosDL(test_pos) == emp::GridPos{1,9});
  CHECK(gs1.PosDR(test_pos) == emp::GridSize::InvalidPos());

  test_pos.Set(9,10);  // Bottom Right
  CHECK(gs1.PosUp(test_pos) == emp::GridPos{8,10});
  CHECK(gs1.PosDown(test_pos) == emp::GridSize::InvalidPos());
  CHECK(gs1.PosLeft(test_pos) == emp::GridPos{9,9});
  CHECK(gs1.PosRight(test_pos) == emp::GridSize::InvalidPos());
  CHECK(gs1.PosUL(test_pos) == emp::GridPos{8,9});
  CHECK(gs1.PosUR(test_pos) == emp::GridSize::InvalidPos());
  CHECK(gs1.PosDL(test_pos) == emp::GridSize::InvalidPos());
  CHECK(gs1.PosDR(test_pos) == emp::GridSize::InvalidPos());
}