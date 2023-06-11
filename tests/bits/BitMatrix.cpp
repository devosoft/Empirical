/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file BitMatrix.cpp
 *
 */

#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/bits/BitMatrix.hpp"

/**
 * Constructor and basic getters
 */
void test_matrix(){
  emp::BitMatrix<2,3> bm;
  REQUIRE((bm.NumRows() == 3));
  REQUIRE((bm.NumCols() == 2));
  REQUIRE((bm.GetSize() == 6));
}

/**
 * ToID
 */
void test_to_id(){
  emp::BitMatrix<2,3> bm;
  REQUIRE((bm.ToID(0,2) == 4));
  REQUIRE((bm.ToID(1,2) == 5));
  REQUIRE((bm.ToID(1,0) == 1));
}

/**
 * ToRow and ToCol
 */
void test_to_row_col(){
  emp::BitMatrix<5,5> bm;
  REQUIRE((bm.ToRow(13) == 2));
  REQUIRE((bm.ToRow(22) == 4));
  REQUIRE((bm.ToRow(9) == 1));
  REQUIRE((bm.ToCol(9) == 4));
  REQUIRE((bm.ToCol(0) == 0));
  REQUIRE((bm.ToCol(16) == 1));
}

/**
 * Set, Unset, Get
 */
void test_get_set(){
  emp::BitMatrix<5,5> bm;
  bm.Set(2,2);
  REQUIRE(bm.Get(12));
  bm.Unset(2,2);
  REQUIRE(!bm.Get(2,2));
  bm.Set(18);
  REQUIRE(bm.Get(3,3));
  bm.Unset(18);
  REQUIRE(!bm.Get(18));
}

/**
 * Flip
 */
void test_flip(){
  emp::BitMatrix<3,3> bm;
  bm.Flip(1,1);
  REQUIRE(bm.Get(1,1));
  bm.Flip(4);
  REQUIRE(!bm.Get(1,1));
}

/**
 * SetAll, SetCol, SetRow,
 * Clear, ClearCol, ClearRow
 */
void test_setall_clear(){
  emp::BitMatrix<3,3> bm;
  bm.SetAll();
  REQUIRE(bm.All());
  bm.ClearCol(0);
  REQUIRE(bm.Any());
  REQUIRE( (!bm.Get(0) && !bm.Get(3) && !bm.Get(6)) );
  bm.ClearRow(2);
  REQUIRE( (!bm.Get(6) && !bm.Get(7) && !bm.Get(8)) );
  bm.Clear();
  REQUIRE(bm.None());
  bm.SetCol(1);
  REQUIRE( (bm.Get(1) && bm.Get(4) && bm.Get(7)) );
  bm.SetRow(0);
  REQUIRE( (bm.Get(0) && bm.Get(1) && bm.Get(2)) );
}

/**
 * CountOnes, FindOne
 */
void test_count_find(){
  emp::BitMatrix<2,3> bm;
  bm.SetCol(0);
  REQUIRE((bm.CountOnes() == 3));
  bm.SetRow(2);
  REQUIRE((bm.CountOnes() == 4));
  REQUIRE((bm.FindOne() == 0));
  bm.Unset(0);
  bm.Unset(2);
  REQUIRE((bm.FindOne() == 4));
}

/**
 * LeftShift, RightShift, DownShift, UpShift
 */
void test_shifts_one(){
  emp::BitMatrix<5,5> bm;
  bm.SetCol(2);
  bm = bm.LeftShift();
  REQUIRE( (bm.Get(1,0) && bm.Get(1,1) && bm.Get(1,2) && bm.Get(1,3) && bm.Get(1,4)) );
  bm.SetRow(2);
  bm = bm.DownShift();
  REQUIRE(bm.Get(16));
  REQUIRE(!bm.Get(12));
  REQUIRE((bm.FindOne() == 6));

  emp::BitMatrix<3,3> bm1;
  bm1.SetCol(2);
  bm1 = bm1.RightShift();
  REQUIRE(bm1.None());

  bm1.SetRow(0);
  bm1 = bm1.UpShift();
  REQUIRE(bm1.None());
}

/**
 * ULShift, DLShift, URShift, DRShift
 */
void test_shifts_two(){
  emp::BitMatrix<3,3> bm;
  bm.Set(1,1);
  bm = bm.DRShift();
  REQUIRE( (bm.Get(2,2) && !bm.Get(1,1)) );

  bm = bm.ULShift();
  bm = bm.ULShift();
  REQUIRE( (!bm.Get(2,2) && bm.Get(0,0)) );

  bm.Set(1,1);
  bm = bm.URShift();
  REQUIRE( (bm.Get(2,0) && !bm.Get(0,0)) );

  bm = bm.DLShift();
  REQUIRE( (bm.Get(1,1) && !bm.Get(2,0)) );
}

/**
 * Reach
 */
void test_reach(){
  emp::BitMatrix<3,3> bm;
  bm.Set(1,1);
  emp::BitMatrix<3,3> bm_reach = bm.GetReach();
  REQUIRE( (bm_reach.Get(1) && bm_reach.Get(3) && bm_reach.Get(4) && bm_reach.Get(5) && bm_reach.Get(7)) );
}

/**
 * Region
 */
void test_region(){
  emp::BitMatrix<3,3> bm;
  // nothing is set on this matrix, so get region will be empty
  REQUIRE( bm.GetRegion(0).None() );

  bm.Set(3);
  bm.Set(4);
  bm.Set(8);
  emp::BitMatrix<3,3> bm_region = bm.GetRegion(4);
  REQUIRE( (bm_region.Get(3) && bm_region.Get(4) && !bm_region.Get(8)) );

  bm.Set(2);
  bm.Set(5);
  bm_region = bm.GetRegion(1,1);
  REQUIRE( (bm_region == bm) );
}

/**
 * IsConnected
 */
void test_connected(){
  emp::BitMatrix<3,3> bm;
  bm.SetRow(1);
  bm.Set(8);
  REQUIRE(bm.IsConnected());

  bm.Unset(4);
  REQUIRE(!bm.IsConnected());
}

/**
 * Has 2x2
 */
void test_two_by_two(){
  emp::BitMatrix<4,4> bm;
  bm.Set(2,0);
  bm.Set(3,0);
  REQUIRE(!bm.Has2x2());
  bm.Set(2,1);
  bm.Set(3,1);
  REQUIRE(bm.Has2x2());
}

/**
 * Print
 */
void test_print(){
  emp::BitMatrix<3,3> bm;
  bm.SetCol(1);
  std::stringstream ss;
  bm.Print(ss);
  REQUIRE( (ss.str() == "010\n010\n010\n") );
}

/**
 * Assignments
 */
void test_assignments(){
  emp::BitMatrix<2,2> bm0;
  bm0.Set(1);
  bm0.Set(2);
  emp::BitMatrix<2,2> bm1 = bm0;
  REQUIRE( (!bm1.Get(0) && bm1.Get(1) && bm1.Get(2) && !bm1.Get(3)) );

  emp::BitMatrix<2,2> bm2;
  bm2.Set(0);
  bm2 |= bm1;
  REQUIRE( (!bm2.Get(3) && bm2.Get(2)) );

  bm2 &= bm0;
  REQUIRE( (!bm2.Get(0) && bm2.Get(1) && bm2.Get(2)) );
  bm0.Set(0);
  bm0 ^= bm2;
  REQUIRE( (bm0.Get(0) && !bm0.Get(1) && !bm0.Get(2) && !bm0.Get(3)) );
}

/**
 * Comparisons
 */
void test_comparisons(){
  emp::BitMatrix<3,3> bm;
  bm.Set(0);
  emp::BitMatrix<3,3> bm1;
  bm1.Set(0);
  REQUIRE( (bm == bm1) );
  bm1.Set(1);
  REQUIRE( (bm != bm1) );
}

/**
 * Logic
 */
void test_logic(){
  emp::BitMatrix<3,3> bm;
  bm.SetRow(0);
  bm.SetCol(1);
  emp::BitMatrix<3,3> bm1 = ~bm;
  REQUIRE( (bm1.Get(3) && bm1.Get(5)) );
  REQUIRE( (bm1 & bm).None() );
  REQUIRE( (bm1 | bm).All() );
  REQUIRE( (bm1 ^ bm).All() );
  bm1.Set(4);
  REQUIRE( !(bm1 ^ bm).Get(4) );
}

/**
 * to BitSet
 */
void test_conversions(){
  emp::BitMatrix<2,2> bm;
  bm.Set(0);
  emp::BitSet<4> bs = bm.to_bitset();
  REQUIRE(bs.Get(0));
}

/**
 * Mask
 */
void test_mask(){
  emp::BitMatrix<5,10> bm;
  emp::BitSet<50> bs = bm.Mask<0,1,10>();
  REQUIRE(bs.size() == (5*10));
}

TEST_CASE("Test BitMatrix", "[bits]")
{
  test_matrix();
  test_to_id();
  test_to_row_col();
  test_get_set();
  test_flip();
  test_setall_clear();
  test_count_find();
  test_shifts_one();
  test_shifts_two();
  test_reach();
  test_region();
  test_connected();
  test_two_by_two();
  test_print();
  test_assignments();
  test_comparisons();
  test_logic();
  test_conversions();
  test_mask();
}

// this templating is necessary to force full coverage of templated classes.
// Since c++ doesn't generate code for templated methods if those methods aren't
// explicitly called (and thus our profiling doesn't see them), we have to
// force them all to be included in the comilation.
template class emp::BitMatrix<4, 5>;
TEST_CASE("Another test BitMatrix", "[bits]")
{

  emp::BitMatrix<4,5> bm45;

  REQUIRE(bm45.NumCols() == 4);
  REQUIRE(bm45.NumRows() == 5);
  REQUIRE(bm45.GetSize() == 20);

  REQUIRE(bm45.Any() == false);
  REQUIRE(bm45.None() == true);
  REQUIRE(bm45.All() == false);
  REQUIRE(bm45.Get(1,1) == 0);
  REQUIRE(bm45.Get(1,2) == 0);
  REQUIRE(bm45.CountOnes() == 0);

  bm45.Set(1,2);  // Try setting a single bit!

  REQUIRE(bm45.Any() == true);
  REQUIRE(bm45.None() == false);
  REQUIRE(bm45.All() == false);
  REQUIRE(bm45.Get(1,1) == 0);
  REQUIRE(bm45.Get(1,2) == 1);
  REQUIRE(bm45.CountOnes() == 1);
  REQUIRE(bm45.FindOne() == bm45.ToID(1,2));

  bm45.SetAll();
  REQUIRE(bm45.All() == true);
  REQUIRE(bm45.None() == false);
  bm45.ClearRow(2);
  REQUIRE(bm45.Get(2,2) == 0);
  REQUIRE(bm45.Get(2,1) == 1);
  bm45.ClearCol(1);
  REQUIRE(bm45.Get(1,1) == 0);
  bm45.Clear();
  REQUIRE(bm45.Get(0,2) == 0);
  bm45.SetRow(2);
  REQUIRE(bm45.Get(0,2) == 1);
  REQUIRE(bm45.Get(0,0) == 0);
  bm45.SetCol(0);
  REQUIRE(bm45.Get(0,0) == 1);
  bm45.Clear();
  bm45.SetRow(2);
  REQUIRE(bm45.Get(0,2) == 1);
  REQUIRE(bm45.Get(0,1) == 0);
  bm45.UpShift();
  // TODO: figure out how upshift actually works and write a real test for it



/* This block needs asserts
  bm45 = bm45.GetReach().GetReach();

  bm45 = bm45.DownShift();

  bm45 = bm45.RightShift();

  bm45 = bm45.URShift();

  bm45 = bm45.UpShift();

  bm45 = bm45.ULShift();

  bm45 = bm45.LeftShift();

  bm45 = bm45.DLShift();

  bm45 = bm45.DRShift();

  emp::BitMatrix<10,10> bm100;
  bm100.Set(9,9);
*/
}
