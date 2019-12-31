//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Tests for files in the tools/ folder.


#define EMP_DECORATE(X) [X]
#define EMP_DECORATE_PAIR(X,Y) [X-Y]
#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include <sstream>
#include <fstream>
#include <string>
#include <deque>
#include <algorithm>
#include <limits>
#include <numeric>
#include <climits>
#include <unordered_set>
#include <ratio>

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>

#include "data/DataNode.h"

#include "tools/Binomial.h"
#include "tools/BitMatrix.h"
#include "tools/BitSet.h"
#include "tools/BitVector.h"
#include "tools/ContiguousStream.h"
#include "tools/DFA.h"
#include "tools/DynamicString.h"
#include "tools/FunctionSet.h"
#include "tools/Graph.h"
#include "tools/IndexMap.h"
#include "tools/Lexer.h"
#include "tools/MatchBin.h"
#include "tools/NFA.h"
#include "tools/RegEx.h"
#include "tools/Random.h"
#include "tools/TypeTracker.h"
#include "tools/attrs.h"

#include "tools/flex_function.h"
#include "tools/functions.h"
#include "tools/graph_utils.h"
#include "tools/hash_utils.h"
//#include "tools/grid.h"
#include "tools/info_theory.h"
#include "tools/keyname_utils.h"
#include "tools/lexer_utils.h"
#include "tools/map_utils.h"
#include "tools/math.h"
#include "tools/matchbin_utils.h"
#include "tools/mem_track.h"
#include "tools/memo_function.h"
#include "tools/NullStream.h"
#include "tools/sequence_utils.h"
// #include "tools/serialize.h"
#include "tools/set_utils.h"
#include "tools/stats.h"
#include "tools/string_utils.h"
#include "tools/tuple_struct.h"
#include "tools/vector_utils.h"

// currently these have no coveage; we include them so we get metrics on them
// this doesn't actually work--TODO: figure out why this doesn't work
#include "tools/alert.h"
#include "tools/const.h"
#include "tools/SolveState.h"
#include "tools/serialize_macros.h"

/// Ensures that
/// 1) A == B
/// 2) A and B can be constexprs or non-contexprs.
/// 3) A and B have the same values regardless of constexpr-ness.
#define CONSTEXPR_REQUIRE_EQ(A, B)       \
  {                                      \
    static_assert(A == B, #A " == " #B); \
    REQUIRE(A == B);                     \
  }


TEST_CASE("Test Binomial", "[tools]")
{
  // test over a consistent set of seeds
  for (int s = 1; s <= 200; ++s) {

  REQUIRE(s > 0);
  emp::Random random(s);

  const double flip_prob = 0.03;
  const size_t num_flips = 100;

  const size_t num_tests = 100000;
  const size_t view_count = 10;

  emp::Binomial bi100(flip_prob, num_flips);

  emp::vector<size_t> counts(num_flips+1, 0);

  for (size_t test_id = 0; test_id < num_tests; test_id++) {
    size_t win_count = 0;
    for (size_t i = 0; i < num_flips; i++) {
      if (random.P(0.03)) win_count++;
    }
    counts[win_count]++;
  }

  // Print out the first values in the distribution.
  for (size_t i = 0; i < view_count; i++) {
    // std::cout << "bi100[" << i << "] = " << bi100[i]
    //           << "  test_count = " << counts[i]
    //           << "\n";
    REQUIRE(bi100[i] < ((double) counts[i]) / (double) num_tests + 0.02);
    REQUIRE(bi100[i] > ((double) counts[i]) / (double) num_tests - 0.02);
  }
  // std::cout << "Total = " << bi100.GetTotalProb() << std::endl;

  // // Pick some random values...
  // std::cout << "\nSome random values:";
  // for (size_t i = 0; i < 100; i++) {
  //   std::cout << " " << bi100.PickRandom(random);
  // }
  // std::cout << std::endl;
  }
}

// this templating is necessary to force full coverage of templated classes.
// Since c++ doesn't generate code for templated methods if those methods aren't
// explicitly called (and thus our profiling doesn't see them), we have to
// force them all to be included in the comilation.
template class emp::BitMatrix<4, 5>;
TEST_CASE("Test BitMatrix", "[tools]")
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
  REQUIRE(bm45.FindBit() == bm45.ToID(1,2));

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

// For BitSet Import/Export
template <size_t SOURCE_BITS, size_t DEST_BITS>
struct ImportExportTester {

  static void test() {

    emp::Random rand(1);

    // using default parameter
    emp::BitSet<SOURCE_BITS> source(rand);
    emp::BitSet<DEST_BITS> dest(rand);

    dest.template Import(source);

    for(size_t i = 0; i < std::min(source.GetSize(), dest.GetSize()); ++i) {
      REQUIRE(source.Get(i) == dest.Get(i));
    }
    for(size_t i = source.GetSize(); i < dest.GetSize(); ++i) {
      REQUIRE(dest.Get(i) == 0);
    }

    dest.Clear();
    dest = source.template Export<dest.GetSize()>();

    for(size_t i = 0; i < std::min(source.GetSize(), dest.GetSize()); ++i) {
      REQUIRE(source.Get(i) == dest.Get(i));
    }
    for(size_t i = source.GetSize(); i < dest.GetSize(); ++i) {
      REQUIRE(dest.Get(i) == 0);
    }

    // using all from_bit's
    source.Randomize(rand);
    dest.Randomize(rand);

    for(size_t from_bit = 0; from_bit < source.GetSize(); ++from_bit) {
      // std::cout << "---------" << std::endl;
      // std::cout << source << std::endl;
      dest.template Import(source, from_bit);
      // std::cout << "=========" << std::endl;
      // std::cout << from_bit << std::endl;
      // std::cout << source << std::endl;
      // std::cout << dest << std::endl;
      for(size_t i = 0; i < std::min(source.GetSize() - from_bit, dest.GetSize()); ++i) {
        REQUIRE(source.Get(i+from_bit) == dest.Get(i));
      }
      for(size_t i = source.GetSize() - from_bit; i < dest.GetSize(); ++i) {
        REQUIRE(dest.Get(i) == 0);
      }

      dest.Clear();
      dest = source.template Export<dest.GetSize()>(from_bit);

      for(size_t i = 0; i < std::min(source.GetSize() - from_bit, dest.GetSize()); ++i) {
        REQUIRE(source.Get(i+from_bit) == dest.Get(i));
      }
      for(size_t i = source.GetSize() - from_bit; i < dest.GetSize(); ++i) {
        REQUIRE(dest.Get(i) == 0);
      }

    }
  }
};

// for BitSet ROTATE_SELF
// adapted from spraetor.github.io/2015/12/26/compile-time-loops.html
template <size_t N>
struct MultiTester2 {

  template <size_t I>
  static void test() {

    emp::Random rand(1);

    constexpr int W = N - 2;
    emp::BitSet<W> bs;

    for (int j = 0; j < W; ++j) {
      bs.Clear(); bs.Set(j);
      bs.template ROTL_SELF<I>();
      REQUIRE(bs.CountOnes() == 1);
      REQUIRE(bs.Get(emp::Mod(j+I,W)));

      bs.SetAll(); bs.Set(j, false);
      bs.template ROTL_SELF<I>();
      REQUIRE(bs.CountOnes() == W-1);
      REQUIRE(!bs.Get(emp::Mod(j+I,W)));

      bs.Randomize(rand); bs.Set(j);
      const size_t c1 = bs.CountOnes();
      bs.template ROTL_SELF<I>();
      REQUIRE(bs.CountOnes() == c1);
      REQUIRE(bs.Get(emp::Mod(j+I,W)));

      bs.Randomize(rand); bs.Set(j, false);
      const size_t c2 = bs.CountOnes();
      bs.template ROTL_SELF<I>();
      REQUIRE(bs.CountOnes() == c2);
      REQUIRE(!bs.Get(emp::Mod(j+I,W)));

      bs.Clear(); bs.Set(j);
      bs.template ROTR_SELF<I>();
      REQUIRE(bs.CountOnes() == 1);
      REQUIRE(bs.Get(emp::Mod(j-I,W)));

      bs.SetAll(); bs.Set(j, false);
      bs.template ROTR_SELF<I>();
      REQUIRE(bs.CountOnes() == W-1);
      REQUIRE(!bs.Get(emp::Mod(j-I,W)));

      bs.Randomize(rand); bs.Set(j);
      const size_t c3 = bs.CountOnes();
      bs.template ROTR_SELF<I>();
      REQUIRE(bs.CountOnes() == c3);
      REQUIRE(bs.Get(emp::Mod(j-I,W)));

      bs.Randomize(rand); bs.Set(j, false);
      const size_t c4 = bs.CountOnes();
      bs.template ROTR_SELF<I>();
      REQUIRE(bs.CountOnes() == c4);
      REQUIRE(!bs.Get(emp::Mod(j-I,W)));
    }

    if constexpr (I+1 < N) {
      // recurse
      MultiTester2<N>::test<I+1>();
    }
  }

};

template <int N>
struct MultiTester {

  template <int I>
  static void test() {

    constexpr int width = I;

    emp::Random rand(1);
    emp::BitSet<width> bs(rand);
    const emp::BitSet<width> bs_orig(bs);
    const size_t num_ones = bs.CountOnes();

    for (int i = -width - 1; i <= width + 1; ++i) {
      for (size_t rep = 0; rep < width; ++ rep) {
        bs.ROTATE_SELF(i);
        REQUIRE(bs.CountOnes() == num_ones);
      }
      REQUIRE(bs == bs_orig);
    }

    for (int i = -width - 1; i <= width + 1; ++i) {
      // for large widths, just do one starting position
      for (int j = 0; j < (width < 200 ? width : 1); ++j) {
        bs.Clear(); bs.Set(j);
        bs.ROTATE_SELF(i);
        REQUIRE(bs.CountOnes() == 1);
        REQUIRE(bs.Get(emp::Mod(j-i,width)));

        bs.SetAll(); bs.Set(j, false);
        bs.ROTATE_SELF(i);
        REQUIRE(bs.CountOnes() == width-1);
        REQUIRE(!bs.Get(emp::Mod(j-i,width)));

        bs.Randomize(rand); bs.Set(j);
        const size_t c1 = bs.CountOnes();
        bs.ROTATE_SELF(i);
        REQUIRE(bs.CountOnes() == c1);
        REQUIRE(bs.Get(emp::Mod(j-i,width)));

        bs.Randomize(rand); bs.Set(j, false);
        const size_t c2 = bs.CountOnes();
        bs.ROTATE_SELF(i);
        REQUIRE(bs.CountOnes() == c2);
        REQUIRE(!bs.Get(emp::Mod(j-i,width)));
      }
    }

    if constexpr (N < 200) {
      // test templated rotates (only for small N)
      MultiTester2<I+2>::template test<0>();
    }

    if constexpr (I+1 < N && N < 200) {
      // recurse
      MultiTester<N>::test<I+1>();
    } else if constexpr (I+1 < N ) {
      // recurse
      MultiTester<N>::test<I+63>();
    }
  }

};

template class emp::BitSet<5>;
TEST_CASE("Test BitSet", "[tools]")
{

  // test BitSet reverse
  {

    REQUIRE(emp::BitSet<1>{0}.REVERSE_SELF() == emp::BitSet<1>{0});
    REQUIRE(emp::BitSet<1>{0}.REVERSE_SELF().CountOnes() == 0);
    REQUIRE(emp::BitSet<1>{1}.REVERSE_SELF() == emp::BitSet<1>{1});
    REQUIRE(emp::BitSet<1>{1}.REVERSE_SELF().CountOnes() == 1);

    REQUIRE(
      (emp::BitSet<2>{1,1}.REVERSE_SELF())
      ==
      (emp::BitSet<2>{1,1})
    );
    REQUIRE((emp::BitSet<2>{1,1}.REVERSE_SELF().CountOnes()) == 2);
    REQUIRE(
      (emp::BitSet<2>{0,1}.REVERSE_SELF())
      ==
      (emp::BitSet<2>{1,0})
    );
    REQUIRE((emp::BitSet<2>{0,1}.REVERSE_SELF().CountOnes()) == 1);
    REQUIRE(
      (emp::BitSet<2>{0,0}.REVERSE_SELF())
      ==
      (emp::BitSet<2>{0,0})
    );
    REQUIRE((emp::BitSet<2>{0,0}.REVERSE_SELF().CountOnes()) == 0);

    REQUIRE(
      (emp::BitSet<7>{1,1,0,0,0,0,1}.REVERSE_SELF())
      ==
      (emp::BitSet<7>{1,0,0,0,0,1,1})
    );
    REQUIRE((emp::BitSet<7>{1,1,0,0,0,0,1}.REVERSE_SELF().CountOnes()) == 3);
    REQUIRE(
      (emp::BitSet<7>{1,0,1,0,1,0,1}.REVERSE_SELF())
      ==
      (emp::BitSet<7>{1,0,1,0,1,0,1})
    );
    REQUIRE((emp::BitSet<7>{1,0,1,0,1,0,1}.REVERSE_SELF().CountOnes()) == 4);
    REQUIRE(
      (emp::BitSet<7>{1,1,1,1,1,0,1}.REVERSE_SELF())
      ==
      (emp::BitSet<7>{1,0,1,1,1,1,1})
    );
    REQUIRE((emp::BitSet<7>{1,1,1,1,1,0,1}.REVERSE_SELF().CountOnes()) == 6);

    REQUIRE(
      (emp::BitSet<8>{1,1,0,0,0,0,1,0}.REVERSE_SELF())
      ==
      (emp::BitSet<8>{0,1,0,0,0,0,1,1})
    );
    REQUIRE((emp::BitSet<8>{1,1,0,0,0,0,1,0}.REVERSE_SELF().CountOnes()) == 3);
    REQUIRE(
      (emp::BitSet<8>{1,0,1,0,1,0,1,0}.REVERSE_SELF())
      ==
      (emp::BitSet<8>{0,1,0,1,0,1,0,1})
    );
    REQUIRE((emp::BitSet<8>{0,1,0,1,0,1,0,1}.REVERSE_SELF().CountOnes()) == 4);
    REQUIRE(
      (emp::BitSet<8>{1,1,1,1,1,0,1,0}.REVERSE_SELF())
      ==
      (emp::BitSet<8>{0,1,0,1,1,1,1,1})
    );
    REQUIRE((emp::BitSet<8>{1,1,1,1,1,0,1,0}.REVERSE_SELF().CountOnes()) == 6);

    REQUIRE(
      (emp::BitSet<9>{1,1,0,0,0,0,1,0,0}.REVERSE_SELF())
      ==
      (emp::BitSet<9>{0,0,1,0,0,0,0,1,1})
    );
    REQUIRE(
      (emp::BitSet<9>{1,1,0,0,0,0,1,0,0}.REVERSE_SELF().CountOnes())
      ==
      3
    );
    REQUIRE(
      (emp::BitSet<9>{1,0,1,0,1,0,1,0,0}.REVERSE_SELF())
      ==
      (emp::BitSet<9>{0,0,1,0,1,0,1,0,1})
    );
    REQUIRE(
      (emp::BitSet<9>{0,0,1,0,1,0,1,0,1}.REVERSE_SELF().CountOnes())
      ==
      4
    );
    REQUIRE(
      (emp::BitSet<9>{1,1,1,1,1,0,1,0,0}.REVERSE_SELF())
      ==
      (emp::BitSet<9>{0,0,1,0,1,1,1,1,1})
    );
    REQUIRE(
      (emp::BitSet<9>{1,1,1,1,1,0,1,0,0}.REVERSE_SELF().CountOnes())
      ==
      6
    );

    emp::Random rand(1);
    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<15> bs(rand);
      bs[0] = 0;
      bs[15-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<16> bs(rand);
      bs[0] = 0;
      bs[16-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<17> bs(rand);
      bs[0] = 0;
      bs[17-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<31> bs(rand);
      bs[0] = 0;
      bs[31-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<32> bs(rand);
      bs[0] = 0;
      bs[32-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<33> bs(rand);
      bs[0] = 0;
      bs[33-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<63> bs(rand);
      bs[0] = 0;
      bs[63-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<64> bs(rand);
      bs[0] = 0;
      bs[64-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<65> bs(rand);
      bs[0] = 0;
      bs[65-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<127> bs(rand);
      bs[0] = 0;
      bs[127-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<128> bs(rand);
      bs[0] = 0;
      bs[128-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

    for (size_t rep = 0; rep < 100; ++rep) {
      emp::BitSet<129> bs(rand);
      bs[0] = 0;
      bs[129-1] = 1;
      REQUIRE(bs.REVERSE() != bs);
      REQUIRE(bs.REVERSE().REVERSE() == bs);
      REQUIRE(bs.REVERSE().CountOnes() == bs.CountOnes());
    }

  }

  // test BitSet addition
  {
  emp::BitSet<32> bs0;
  bs0.SetUInt(0, std::numeric_limits<uint32_t>::max() - 1);
  emp::BitSet<32> bs1;
  bs1.SetUInt(0,1);
  bs0+=bs1;
  REQUIRE (bs0.GetUInt(0) == 4294967295);
  REQUIRE ((bs0+bs1).GetUInt(0) == 0);
  REQUIRE ((bs0+bs0).GetUInt(0) == 4294967294);

  emp::BitSet<8> bs2;
  bs2.SetUInt(0, emp::IntPow(2UL, 8UL)-1);
  emp::BitSet<8> bs3;
  bs3.SetUInt(0, 1);
  REQUIRE((bs2+bs3).GetUInt(0) == 0);
  emp::BitSet<64> bs4;
  bs4.SetUInt(0, std::numeric_limits<uint32_t>::max()-1);
  bs4.SetUInt(1, std::numeric_limits<uint32_t>::max());
  emp::BitSet<64> bs5;
  bs5.SetUInt(0, 1);
  bs4+=bs5;
  REQUIRE(bs4.GetUInt(0) == pow((size_t)2, (size_t)32)-1);
  REQUIRE(bs4.GetUInt(1) == pow((size_t)2, (size_t)32)-1);
  bs4+=bs5;
  REQUIRE(bs4.GetUInt(0) == 0);
  REQUIRE(bs4.GetUInt(1) == 0);
  }

  // test BitSet subtraction
  {
  emp::BitSet<32> bs0;
  bs0.SetUInt(0, 1);
  emp::BitSet<32> bs1;
  bs1.SetUInt(0, 1);
  bs0 = bs0 - bs1;
  REQUIRE (bs0.GetUInt(0) == 0);
  REQUIRE ((bs0-bs1).GetUInt(0) == std::numeric_limits<uint32_t>::max());

  emp::BitSet<8> bs2;
  bs2.SetUInt(0, 1);
  emp::BitSet<8> bs3;
  bs3.SetUInt(0, 1);

  bs2-=bs3;
  REQUIRE (bs2.GetUInt(0) == 0);
  REQUIRE((bs2-bs3).GetUInt(0) == emp::IntPow(2UL,8UL)-1);

  emp::BitSet<64> bs4;
  bs4.SetUInt(0, 1);
  bs4.SetUInt(1, 0);

  emp::BitSet<64> bs5;
  bs5.SetUInt(0, 1);

  bs4-=bs5;
  REQUIRE(bs4.GetUInt(0) == 0);
  REQUIRE(bs4.GetUInt(1) == 0);

  bs4-=bs5;
  REQUIRE(bs4.GetUInt(0) == std::numeric_limits<uint32_t>::max());
  REQUIRE(bs4.GetUInt(1) == std::numeric_limits<uint32_t>::max());
  bs4 = bs4 - bs5;
  REQUIRE(bs4.GetUInt(0) == std::numeric_limits<uint32_t>::max() - 1);
  REQUIRE(bs4.GetUInt(1) == std::numeric_limits<uint32_t>::max());
  }

  // test addition and subtraction with multiple fields
  {
  emp::BitSet<65> bs1;
  emp::BitSet<65> bs2;

  /* PART 1 */
  bs1.Clear();
  bs2.Clear();

  bs1.Set(64); // 10000...
  bs2.Set(0);  // ...00001

  for(size_t i = 0; i < 64; ++i) REQUIRE((bs1 - bs2).Get(i));
  REQUIRE(!(bs1 - bs2).Get(64));

  bs1 -= bs2;

  for(size_t i = 0; i < 64; ++i) {
    REQUIRE(bs1.Get(i));
  }
  REQUIRE(!bs1.Get(64));

  /* PART 2 */
  bs1.Clear();
  bs2.Clear();

  bs2.Set(0);  // ...00001

  for(size_t i = 0; i < 65; ++i) REQUIRE((bs1 - bs2).Get(i));

  bs1 -= bs2;

  for(size_t i = 0; i < 65; ++i) REQUIRE(bs1.Get(i));

  /* PART 3 */
  bs1.Clear();
  bs2.Clear();

  for(size_t i = 0; i < 65; ++i) bs1.Set(i); // 11111...11111
  bs2.Set(0);  // ...00001

  for(size_t i = 0; i < 65; ++i) REQUIRE(!(bs1 + bs2).Get(i));
  for(size_t i = 0; i < 65; ++i) REQUIRE(!(bs2 + bs1).Get(i));

  bs1 += bs2;

  for(size_t i = 0; i < 65; ++i) REQUIRE(!bs1.Get(i));

  /* PART 4 */
  bs1.Clear();
  bs2.Clear();

  for(size_t i = 0; i < 64; ++i) bs1.Set(i); // 01111...11111
  bs2.Set(0);  // ...00001

  for(size_t i = 0; i < 64; ++i) REQUIRE(!(bs1 + bs2).Get(i));
  REQUIRE((bs1 + bs2).Get(64));
  for(size_t i = 0; i < 64; ++i) REQUIRE(!(bs2 + bs1).Get(i));
  REQUIRE((bs2 + bs1).Get(64));

  bs1 += bs2;

  for(size_t i = 0; i < 64; ++i) REQUIRE(!bs1.Get(i));
  REQUIRE((bs2 + bs1).Get(64));
  }

  // test GetDouble() and MaxDouble()
  {
  emp::BitSet<3> bs0{0,0,0};
  REQUIRE(bs0.GetDouble() == 0.0);
  REQUIRE(bs0.MaxDouble() == 7.0);

  emp::BitSet<3> bs1{0,0,1};
  REQUIRE(bs1.GetDouble() == 1.0);

  emp::BitSet<3> bs2{0,1,1};
  REQUIRE(bs2.GetDouble() == 3.0);

  emp::BitSet<3> bs3{1,1,1};
  REQUIRE(bs3.GetDouble() == 7.0);

  emp::BitSet<3> bs4{1,1,0};
  REQUIRE(bs4.GetDouble() == 6.0);

  emp::BitSet<32> bs5;
  bs5.SetUInt(0, 1789156UL);
  REQUIRE(bs5.GetDouble() == 1789156ULL);
  REQUIRE(bs5.MaxDouble() == 4294967295.0);

  emp::BitSet<64> bs6;
  bs6.SetUInt64(0, 1789156816848ULL);
  REQUIRE(bs6.GetDouble() == 1789156816848ULL);
  REQUIRE(bs6.MaxDouble() == 18446744073709551615.0);

  emp::BitSet<65> bs7;
  bs7.SetUInt64(0, 1789156816848ULL);
  bs7.Set(64);
  REQUIRE(bs7.GetDouble() == 1789156816848.0 + emp::Pow2(64.0));
  REQUIRE(bs7.MaxDouble() == 36893488147419103231.0);

  emp::BitSet<1027> bs8;
  bs8.Set(1026);
  REQUIRE(std::isinf(bs8.GetDouble()));
  REQUIRE(std::isinf(bs8.MaxDouble()));
  }

  // test list initializer
  {
  emp::BitSet<3> bs_empty{0,0,0};
  emp::BitSet<3> bs_first{1,0,0};
  emp::BitSet<3> bs_last{0,0,1};
  emp::BitSet<3> bs_full{1,1,1};

  REQUIRE(bs_empty.CountOnes() == 0);
  REQUIRE(bs_first.CountOnes() == 1);
  REQUIRE(bs_last.CountOnes() == 1);
  REQUIRE(bs_full.CountOnes() == 3);
  }

  // test Import and Export
  {

    emp::Random rand(1);

    emp::BitSet<32> orig(rand);

    emp::array<emp::BitSet<32>, 1> d1;
    emp::array<emp::BitSet<16>, 2> d2;
    emp::array<emp::BitSet<8>, 4> d4;
    emp::array<emp::BitSet<4>, 8> d8;
    emp::array<emp::BitSet<2>, 16> d16;
    emp::array<emp::BitSet<1>, 32> d32;

    // Import

    d1[0].Import(orig, 0);
    for(size_t i = 0; i < 2; ++i) d2[i].Import(orig, i * 16);
    for(size_t i = 0; i < 4; ++i) d4[i].Import(orig, i * 8);
    for(size_t i = 0; i < 8; ++i) d8[i].Import(orig, i * 4);
    for(size_t i = 0; i < 16; ++i) d16[i].Import(orig, i * 2);
    for(size_t i = 0; i < 32; ++i) d32[i].Import(orig, i * 1);

    for (size_t i = 0; i < 32; ++i) {
      REQUIRE(orig[i] == d1[i/32][i%32]);
      REQUIRE(orig[i] == d2[i/16][i%16]);
      REQUIRE(orig[i] == d4[i/8][i%8]);
      REQUIRE(orig[i] == d8[i/4][i%4]);
      REQUIRE(orig[i] == d16[i/2][i%2]);
      REQUIRE(orig[i] == d32[i/1][i%1]);
    }

    // Export

    d1[0] = orig.Export<32>(0);
    for(size_t i = 0; i < 2; ++i) d2[i] = orig.Export<16>(i * 16);
    for(size_t i = 0; i < 4; ++i) d4[i] = orig.Export<8>(i * 8);
    for(size_t i = 0; i < 8; ++i) d8[i] = orig.Export<4>(i * 4);
    for(size_t i = 0; i < 16; ++i) d16[i] = orig.Export<2>(i * 2);
    for(size_t i = 0; i < 32; ++i) d32[i] = orig.Export<1>(i * 1);

    for (size_t i = 0; i < 32; ++i) {
      REQUIRE(orig[i] == d1[i/32][i%32]);
      REQUIRE(orig[i] == d2[i/16][i%16]);
      REQUIRE(orig[i] == d4[i/8][i%8]);
      REQUIRE(orig[i] == d8[i/4][i%4]);
      REQUIRE(orig[i] == d16[i/2][i%2]);
      REQUIRE(orig[i] == d32[i/1][i%1]);
    }

    // now test some funky imports and exports
    // interesting container sizes:
    // 1, 17, 29, 32, 33, 64, 65, 95, 128, 129

    ImportExportTester<1,1>::test();
    ImportExportTester<1,17>::test();
    ImportExportTester<1,29>::test();
    ImportExportTester<1,32>::test();
    ImportExportTester<1,33>::test();
    ImportExportTester<1,64>::test();
    ImportExportTester<1,65>::test();
    ImportExportTester<1,96>::test();
    ImportExportTester<1,128>::test();
    ImportExportTester<1,129>::test();

    ImportExportTester<17,1>::test();
    ImportExportTester<17,17>::test();
    ImportExportTester<17,29>::test();
    ImportExportTester<17,32>::test();
    ImportExportTester<17,33>::test();
    ImportExportTester<17,64>::test();
    ImportExportTester<17,65>::test();
    ImportExportTester<17,96>::test();
    ImportExportTester<17,128>::test();
    ImportExportTester<17,129>::test();

    ImportExportTester<29,1>::test();
    ImportExportTester<29,17>::test();
    ImportExportTester<29,29>::test();
    ImportExportTester<29,32>::test();
    ImportExportTester<29,33>::test();
    ImportExportTester<29,64>::test();
    ImportExportTester<29,65>::test();
    ImportExportTester<29,96>::test();
    ImportExportTester<29,128>::test();
    ImportExportTester<29,129>::test();

    ImportExportTester<32,1>::test();
    ImportExportTester<32,17>::test();
    ImportExportTester<32,29>::test();
    ImportExportTester<32,32>::test();
    ImportExportTester<32,33>::test();
    ImportExportTester<32,64>::test();
    ImportExportTester<32,65>::test();
    ImportExportTester<32,96>::test();
    ImportExportTester<32,128>::test();
    ImportExportTester<32,129>::test();

    ImportExportTester<33,1>::test();
    ImportExportTester<33,17>::test();
    ImportExportTester<33,29>::test();
    ImportExportTester<33,32>::test();
    ImportExportTester<33,33>::test();
    ImportExportTester<33,64>::test();
    ImportExportTester<33,65>::test();
    ImportExportTester<33,96>::test();
    ImportExportTester<33,128>::test();
    ImportExportTester<33,129>::test();

    ImportExportTester<64,1>::test();
    ImportExportTester<64,17>::test();
    ImportExportTester<64,29>::test();
    ImportExportTester<64,32>::test();
    ImportExportTester<64,33>::test();
    ImportExportTester<64,64>::test();
    ImportExportTester<64,65>::test();
    ImportExportTester<64,96>::test();
    ImportExportTester<64,128>::test();
    ImportExportTester<64,129>::test();

    ImportExportTester<65,1>::test();
    ImportExportTester<65,17>::test();
    ImportExportTester<65,29>::test();
    ImportExportTester<65,32>::test();
    ImportExportTester<65,33>::test();
    ImportExportTester<65,64>::test();
    ImportExportTester<65,65>::test();
    ImportExportTester<65,96>::test();
    ImportExportTester<65,128>::test();
    ImportExportTester<65,129>::test();

    ImportExportTester<96,1>::test();
    ImportExportTester<96,17>::test();
    ImportExportTester<96,29>::test();
    ImportExportTester<96,32>::test();
    ImportExportTester<96,33>::test();
    ImportExportTester<96,64>::test();
    ImportExportTester<96,65>::test();
    ImportExportTester<96,96>::test();
    ImportExportTester<96,128>::test();
    ImportExportTester<96,129>::test();

    ImportExportTester<128,1>::test();
    ImportExportTester<128,17>::test();
    ImportExportTester<128,29>::test();
    ImportExportTester<128,32>::test();
    ImportExportTester<128,33>::test();
    ImportExportTester<128,64>::test();
    ImportExportTester<128,65>::test();
    ImportExportTester<128,96>::test();
    ImportExportTester<128,128>::test();
    ImportExportTester<128,129>::test();

    ImportExportTester<129,1>::test();
    ImportExportTester<129,17>::test();
    ImportExportTester<129,29>::test();
    ImportExportTester<129,32>::test();
    ImportExportTester<129,33>::test();
    ImportExportTester<129,64>::test();
    ImportExportTester<129,65>::test();
    ImportExportTester<129,96>::test();
    ImportExportTester<129,128>::test();
    ImportExportTester<129,129>::test();

  }

  emp::BitSet<10> bs10;
  emp::BitSet<25> bs25;
  emp::BitSet<32> bs32;
  emp::BitSet<50> bs50;
  emp::BitSet<64> bs64;
  emp::BitSet<80> bs80;

  bs80[70] = 1;
  bs80 <<= 1;
  emp::BitSet<80> bs80c(bs80);

  for (size_t i = 0; i < 75; i++) {
    emp::BitSet<80> shift_set = bs80 >> i;
    REQUIRE((shift_set.CountOnes() == 1) == (i <= 71));
  }

  bs80.Clear();

  REQUIRE(bs10[2] == false);
  bs10.flip(2);
  REQUIRE(bs10[2] == true);

  REQUIRE(bs32[2] == false);
  bs32.flip(2);
  REQUIRE(bs32[2] == true);

  REQUIRE(bs80[2] == false);
  bs80.flip(2);
  REQUIRE(bs80[2] == true);

  for (size_t i = 3; i < 8; i++) {REQUIRE(bs10[i] == false);}
  bs10.flip(3, 8);
  for (size_t i = 3; i < 8; i++) {REQUIRE(bs10[i] == true);}
  REQUIRE(bs10[8] == false);

  for (size_t i = 3; i < 8; i++) {REQUIRE(bs32[i] == false);}
  bs32.flip(3, 8);
  for (size_t i = 3; i < 8; i++) {REQUIRE(bs32[i] == true);}
  REQUIRE(bs32[8] == false);

  for (size_t i = 3; i < 8; i++) {REQUIRE(bs80[i] == false);}
  bs80.flip(3, 8);
  for (size_t i = 3; i < 8; i++) {REQUIRE(bs80[i] == true);}
  REQUIRE(bs80[8] == false);

  bs80[70] = 1;

  REQUIRE(bs10.GetUInt(0) == 252);
  REQUIRE(bs10.GetUInt32(0) == 252);
  REQUIRE(bs10.GetUInt64(0) == 252);

  REQUIRE(bs32.GetUInt(0) == 252);
  REQUIRE(bs32.GetUInt32(0) == 252);
  REQUIRE(bs32.GetUInt64(0) == 252);

  REQUIRE(bs80.GetUInt(0) == 252);
  REQUIRE(bs80.GetUInt(1) == 0);
  REQUIRE(bs80.GetUInt(2) == 64);
  REQUIRE(bs80.GetUInt32(0) == 252);
  REQUIRE(bs80.GetUInt32(1) == 0);
  REQUIRE(bs80.GetUInt32(2) == 64);
  REQUIRE(bs80.GetUInt64(0) == 252);
  REQUIRE(bs80.GetUInt64(1) == 64);

  bs80 = bs80c;

  // Test arbitrary bit retrieval of UInts
  bs80[65] = 1;
  REQUIRE(bs80.GetUIntAtBit(64) == 130);
  REQUIRE(bs80.GetValueAtBit<5>(64) == 2);

  emp::BitSet<96> bs;

  REQUIRE (bs.LongestSegmentOnes() == 0);
  bs.SetUInt(2, 1);
  REQUIRE (bs.LongestSegmentOnes() == 1);
  bs.SetUInt(1, 3);
  REQUIRE (bs.LongestSegmentOnes() == 2);
  bs.SetUInt(0, 7);
  REQUIRE (bs.LongestSegmentOnes() == 3);

  bs.SetUInt(0, std::numeric_limits<uint32_t>::max());
  bs.SetUInt(1, std::numeric_limits<uint32_t>::max() - 1);
  bs.SetUInt(2, std::numeric_limits<uint32_t>::max() - 3);
  REQUIRE (bs.LongestSegmentOnes() == 32);

  // tests for ROTATE
  // ... with one set bit
  bs10.Clear(); bs10.Set(0);
  bs25.Clear(); bs25.Set(0);
  bs32.Clear(); bs32.Set(0);
  bs50.Clear(); bs50.Set(0);
  bs64.Clear(); bs64.Set(0);
  bs80.Clear(); bs80.Set(0);

  for (int rot = -100; rot < 101; ++rot) {

    REQUIRE( bs10.CountOnes() == bs10.ROTATE(rot).CountOnes() );
    REQUIRE( bs25.CountOnes() == bs25.ROTATE(rot).CountOnes() );
    REQUIRE( bs32.CountOnes() == bs32.ROTATE(rot).CountOnes() );
    REQUIRE( bs50.CountOnes() == bs50.ROTATE(rot).CountOnes() );
    REQUIRE( bs64.CountOnes() == bs64.ROTATE(rot).CountOnes() );
    REQUIRE( bs80.CountOnes() == bs80.ROTATE(rot).CountOnes() );

    if (rot % 10) REQUIRE( bs10 != bs10.ROTATE(rot) );
    else REQUIRE( bs10 == bs10.ROTATE(rot) );

    if (rot % 25) REQUIRE( bs25 != bs25.ROTATE(rot) );
    else REQUIRE( bs25 == bs25.ROTATE(rot) );

    if (rot % 32) REQUIRE( bs32 != bs32.ROTATE(rot) );
    else REQUIRE( bs32 == bs32.ROTATE(rot) );

    if (rot % 50) REQUIRE( bs50 != bs50.ROTATE(rot) );
    else REQUIRE( bs50 == bs50.ROTATE(rot) );

    if (rot % 64) REQUIRE( bs64 != bs64.ROTATE(rot) );
    else REQUIRE( bs64 == bs64.ROTATE(rot) );

    if (rot % 80) REQUIRE( bs80 != bs80.ROTATE(rot) );
    else REQUIRE( bs80 == bs80.ROTATE(rot) );

  }

  // ... with random set bits
  emp::Random rand(1);
  // no bs10 because there's a reasonable chance
  // of breaking the test's assumption of nonsymmetry
  bs25.Randomize(rand);
  bs32.Randomize(rand);
  bs50.Randomize(rand);
  bs64.Randomize(rand);
  bs80.Randomize(rand);

  for (int rot = -100; rot < 101; ++rot) {

    REQUIRE( bs25.CountOnes() == bs25.ROTATE(rot).CountOnes() );
    REQUIRE( bs32.CountOnes() == bs32.ROTATE(rot).CountOnes() );
    REQUIRE( bs50.CountOnes() == bs50.ROTATE(rot).CountOnes() );
    REQUIRE( bs64.CountOnes() == bs64.ROTATE(rot).CountOnes() );
    REQUIRE( bs80.CountOnes() == bs80.ROTATE(rot).CountOnes() );

    if (rot % 25) REQUIRE( bs25 != bs25.ROTATE(rot) );
    else REQUIRE( bs25 == bs25.ROTATE(rot) );

    if (rot % 32) REQUIRE( bs32 != bs32.ROTATE(rot) );
    else REQUIRE( bs32 == bs32.ROTATE(rot) );

    if (rot % 50) REQUIRE( bs50 != bs50.ROTATE(rot) );
    else REQUIRE( bs50 == bs50.ROTATE(rot) );

    if (rot % 64) REQUIRE( bs64 != bs64.ROTATE(rot) );
    else REQUIRE( bs64 == bs64.ROTATE(rot) );

    if (rot % 80) REQUIRE( bs80 != bs80.ROTATE(rot) );
    else REQUIRE( bs80 == bs80.ROTATE(rot) );

  }

  // tests for ROTATE_SELF, ROTR_SELF, ROTL_SELF
  MultiTester<2>::test<1>();
  MultiTester<18>::test<17>();
  MultiTester<34>::test<31>();
  MultiTester<51>::test<50>();
  MultiTester<66>::test<63>();
  MultiTester<96>::test<93>();
  MultiTester<161>::test<160>();
  MultiTester<2050>::test<2048>();

  // tests for Mutate
  {
    emp::Random rando(1);
    emp::BitSet<25> bs_25;
    emp::BitSet<32> bs_32;
    emp::BitSet<50> bs_50;
    emp::BitSet<64> bs_64;
    emp::BitSet<80> bs_80;

    bs_25.Mutate(rando, 0);
    REQUIRE(!bs_25.CountOnes());

    bs_32.Mutate(rando, 0);
    REQUIRE(!bs_32.CountOnes());

    bs_50.Mutate(rando, 0);
    REQUIRE(!bs_50.CountOnes());

    bs_64.Mutate(rando, 0);
    REQUIRE(!bs_64.CountOnes());

    bs_80.Mutate(rando, 0);
    REQUIRE(!bs_80.CountOnes());


    bs_25.Mutate(rando, 1);
    REQUIRE( bs_25.CountOnes() == 1);

    bs_32.Mutate(rando, 1);
    REQUIRE( bs_32.CountOnes() == 1);

    bs_50.Mutate(rando, 1);
    REQUIRE( bs_50.CountOnes() == 1);

    bs_64.Mutate(rando, 1);
    REQUIRE( bs_64.CountOnes() == 1);

    bs_80.Mutate(rando, 1);
    REQUIRE( bs_80.CountOnes() == 1);

    bs_25.Clear();
    bs_32.Clear();
    bs_50.Clear();
    bs_64.Clear();
    bs_80.Clear();

    for (size_t i = 1; i < 5000; ++i) {
      bs_25.Mutate(rando, 1);
      REQUIRE(bs_25.CountOnes() <= i);

      bs_32.Mutate(rando, 1);
      REQUIRE(bs_32.CountOnes() <= i);

      bs_50.Mutate(rando, 1);
      REQUIRE(bs_50.CountOnes() <= i);

      bs_64.Mutate(rando, 1);
      REQUIRE(bs_64.CountOnes() <= i);

      bs_80.Mutate(rando, 1);
      REQUIRE(bs_80.CountOnes() <= i);
    }

    REQUIRE(bs_25.CountOnes() > bs_25.size()/4);
    REQUIRE(bs_25.CountOnes() < 3*bs_25.size()/4);
    REQUIRE(bs_32.CountOnes() > bs_32.size()/4);
    REQUIRE(bs_32.CountOnes() < 3*bs_32.size()/4);
    REQUIRE(bs_50.CountOnes() > bs_50.size()/4);
    REQUIRE(bs_50.CountOnes() < 3*bs_50.size()/4);
    REQUIRE(bs_64.CountOnes() > bs_64.size()/4);
    REQUIRE(bs_64.CountOnes() < 3*bs_64.size()/4);
    REQUIRE(bs_80.CountOnes() > bs_80.size()/4);
    REQUIRE(bs_80.CountOnes() < 3*bs_80.size()/4);

    for (size_t i = 0; i < 10; ++i) {
      bs_25.Mutate(rando, bs_25.size());
      REQUIRE(bs_25.CountOnes() > bs_25.size()/4);
      REQUIRE(bs_25.CountOnes() < 3*bs_25.size()/4);

      bs_32.Mutate(rando, bs_32.size());
      REQUIRE(bs_32.CountOnes() > bs_32.size()/4);
      REQUIRE(bs_32.CountOnes() < 3*bs_32.size()/4);

      bs_50.Mutate(rando, bs_50.size());
      REQUIRE(bs_50.CountOnes() > bs_50.size()/4);
      REQUIRE(bs_50.CountOnes() < 3*bs_50.size()/4);

      bs_64.Mutate(rando, bs_64.size());
      REQUIRE(bs_64.CountOnes() > bs_64.size()/4);
      REQUIRE(bs_64.CountOnes() < 3*bs_64.size()/4);

      bs_80.Mutate(rando, bs_80.size());
      REQUIRE(bs_80.CountOnes() > bs_80.size()/4);
      REQUIRE(bs_80.CountOnes() < 3*bs_80.size()/4);
    }
  }

  // serialize / deserialize
  {

    // set up
    emp::Random rand(1);
    emp::BitSet<10> bs10(rand);
    emp::BitSet<25> bs25(rand);
    emp::BitSet<32> bs32(rand);
    emp::BitSet<50> bs50(rand);
    emp::BitSet<64> bs64(rand);
    emp::BitSet<80> bs80(rand);

    emp::BitSet<10> bs10_deser;
    emp::BitSet<25> bs25_deser;
    emp::BitSet<32> bs32_deser;
    emp::BitSet<50> bs50_deser;
    emp::BitSet<64> bs64_deser;
    emp::BitSet<80> bs80_deser;

    std::stringstream ss;

    {
      // Create an output archive
      cereal::BinaryOutputArchive oarchive(ss);

      // Write the data to the archive
      oarchive(
        bs10,
        bs25,
        bs32,
        bs50,
        bs64,
        bs80
      );

    } // archive goes out of scope, ensuring all contents are flushed

    {
      cereal::BinaryInputArchive iarchive(ss); // Create an input archive

       // Read the data from the archive
       iarchive(
        bs10_deser,
        bs25_deser,
        bs32_deser,
        bs50_deser,
        bs64_deser,
        bs80_deser
      );

    }

    REQUIRE(bs10 == bs10_deser);
    REQUIRE(bs25 == bs25_deser);
    REQUIRE(bs32 == bs32_deser);
    REQUIRE(bs50 == bs50_deser);
    REQUIRE(bs64 == bs64_deser);
    REQUIRE(bs80 == bs80_deser);

  }

  {

    // set up
    emp::Random rand(1);
    emp::BitSet<10> bs10(rand);
    emp::BitSet<25> bs25(rand);
    emp::BitSet<32> bs32(rand);
    emp::BitSet<50> bs50(rand);
    emp::BitSet<64> bs64(rand);
    emp::BitSet<80> bs80(rand);

    emp::BitSet<10> bs10_deser;
    emp::BitSet<25> bs25_deser;
    emp::BitSet<32> bs32_deser;
    emp::BitSet<50> bs50_deser;
    emp::BitSet<64> bs64_deser;
    emp::BitSet<80> bs80_deser;

    std::stringstream ss;

    {
      // Create an output archive
      cereal::JSONOutputArchive oarchive(ss);

      // Write the data to the archive
      oarchive(
        bs10,
        bs25,
        bs32,
        bs50,
        bs64,
        bs80
      );

    } // archive goes out of scope, ensuring all contents are flushed

    {
      cereal::JSONInputArchive iarchive(ss); // Create an input archive

       // Read the data from the archive
       iarchive(
        bs10_deser,
        bs25_deser,
        bs32_deser,
        bs50_deser,
        bs64_deser,
        bs80_deser
      );

    }

    REQUIRE(bs10 == bs10_deser);
    REQUIRE(bs25 == bs25_deser);
    REQUIRE(bs32 == bs32_deser);
    REQUIRE(bs50 == bs50_deser);
    REQUIRE(bs64 == bs64_deser);
    REQUIRE(bs80 == bs80_deser);

  }

}


TEST_CASE("Test BitSet timing", "[tools]")
{
  const size_t set_size = 100000;
  typedef emp::BitSet<set_size> TEST_TYPE;

  TEST_TYPE set1;
  TEST_TYPE set2;

  for (size_t i = 0; i < set_size; i++) {
    if (!(i%2) && (i%5)) set1[i] = 1;
    if (!(i%3) && (i%7)) set2.Set(i, true);
  }

  // TIMING!!!!!
  std::clock_t emp_start_time = std::clock();

  TEST_TYPE set3(set1 & set2);
  TEST_TYPE set4 = (set1 | set2);
  size_t total = 0;

  // should probably assert that this does what we want it to do...
  for (size_t i = 0; i < 100000; i++) {
    set3 |= (set4 << 3);
    set4 &= (set3 >> 3);
    auto set5 = set3 & set4;
    total += set5.CountOnes();
  }

  std::clock_t emp_tot_time = std::clock() - emp_start_time;
  double time = 1000.0 * ((double) emp_tot_time) / (double) CLOCKS_PER_SEC;
  //REQUIRE(time < 13000); // WARNING: WILL VARY ON DIFFERENT SYSTEMS

  // END TIMING!!!
}


TEST_CASE("Test BitVector", "[tools]")
{
  emp::BitVector bv10(10);
  emp::BitVector bv32(32);
  emp::BitVector bv50(50);
  emp::BitVector bv64(64);
  emp::BitVector bv80(80);

  bv80[70] = 1;
  emp::BitVector bv80c(bv80);

  bv80 <<= 1;

  for (size_t i = 0; i < 75; i += 2) {
    emp::BitVector shift_vector = bv80 >> i;
    REQUIRE((shift_vector.CountOnes() == 1) == (i <= 71));
  }

  bv10 = (bv80 >> 70);

  // Test arbitrary bit retrieval of UInts
  bv80[65] = 1;
  REQUIRE(bv80.GetUIntAtBit(64) == 130);
  REQUIRE(bv80.GetValueAtBit<5>(64) == 2);
}


TEST_CASE("Test DFA", "[tools]")
{
  emp::DFA dfa(10);
  dfa.SetTransition(0, 1, 'a');
  dfa.SetTransition(1, 2, 'a');
  dfa.SetTransition(2, 0, 'a');
  dfa.SetTransition(0, 3, 'b');

  int state = 0;
  REQUIRE( (state = dfa.Next(state, 'a')) == 1 );
  REQUIRE( (state = dfa.Next(state, 'a')) == 2 );
  REQUIRE( (state = dfa.Next(state, 'a')) == 0 );
  REQUIRE( (state = dfa.Next(state, 'b')) == 3 );
  REQUIRE( (state = dfa.Next(state, 'b')) == -1 );
  REQUIRE( (state = dfa.Next(state, 'b')) == -1 );
  REQUIRE( (state = dfa.Next(state, 'b')) == -1 );

  REQUIRE(dfa.Next(0, "aaaaaab") == 3);
  REQUIRE(dfa.Next(0, "aaaaab") == -1);
  REQUIRE(dfa.Next(0, "aaaaaabb") == -1);
  REQUIRE(dfa.Next(0, "a") == 1);
  REQUIRE(dfa.Next(0, "aa") == 2);
  REQUIRE(dfa.Next(0, "aaa") == 0);
  REQUIRE(dfa.Next(0, "b")  == 3);
}

TEST_CASE("Test ContiguousStreamBuf", "[tools]")
{
  emp::Random rand(1);

  std::stringstream ss;
  emp::ContiguousStream cs1;
  emp::ContiguousStream cs2(1);
  emp::ContiguousStream cs3(0);
  emp::ContiguousStream cs4(3);

  std::string temp;

  for (size_t i = 0; i < 3; ++i) {
    temp = ss.str();

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs1.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs1.GetSize() );

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs2.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs2.GetSize() );

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs3.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs3.GetSize() );

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs3.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs4.GetSize() );

    // Put in some letters
    ss << "Hello_World";
    cs1 << "Hello_World";
    cs2 << "Hello_World";
    cs3 << "Hello_World";
    cs4 << "Hello_World";
    temp = ss.str();

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs1.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs1.GetSize() );

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs2.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs2.GetSize() );

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs3.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs3.GetSize() );

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs4.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs4.GetSize() );

    // Make sure flush doesn't break it
    ss << "Will it flush?" << std::flush << "beep";
    cs1 << "Will it flush?" << std::flush << "beep";
    cs2 << "Will it flush?" << std::flush << "beep";
    cs3 << "Will it flush?" << std::flush << "beep";
    cs4 << "Will it flush?" << std::flush << "beep";
    temp = ss.str();

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs1.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs1.GetSize() );

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs2.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs2.GetSize() );

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs3.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs3.GetSize() );

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs4.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs4.GetSize() );

    // Put in random data
    for (size_t i = 0; i < 1024; ++i) {
      const auto draw = rand.GetUInt();
      ss << draw;
      cs1 << draw;
      cs2 << draw;
      cs3 << draw;
      cs4 << draw;
    }
    temp = ss.str();

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs1.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs1.GetSize() );

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs2.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs2.GetSize() );

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs3.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs3.GetSize() );

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs4.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs4.GetSize() );

    // Make sure reset works!
    cs1.Reset();
    cs2.Reset();
    cs3.Reset();
    cs4.Reset();
    ss.str("");
    ss.clear();

  }

}

TEST_CASE("Test DynamicString", "[tools]")
{
  emp::DynamicString test_set;

  test_set.Append("Line Zero");  // Test regular append
  test_set << "Line One";        // Test stream append

  // Test append-to-stream
  std::stringstream ss;
  ss << test_set;
  REQUIRE(ss.str() == "Line ZeroLine One");

  // Test direct conversion to string.
  REQUIRE(test_set.str() == "Line ZeroLine One");

  // Test appending functions.
  test_set.Append( [](){ return std::string("Line Two"); } );
  test_set.Append( [](){ return "Line Three"; } );

  // Test appending functions with variable output
  int line_no = 20;
  test_set.Append( [&line_no](){ return std::string("Line ") + std::to_string(line_no); } );

  REQUIRE(test_set[4] == "Line 20");
  line_no = 4;
  REQUIRE(test_set[4] == "Line 4");

  // Make sure we can change an existing line.
  test_set.Set(0, "Line 0");
  REQUIRE(test_set[0] == "Line 0");

  // Make sure all lines are what we expect.
  REQUIRE(test_set[0] == "Line 0");
  REQUIRE(test_set[1] == "Line One");
  REQUIRE(test_set[2] == "Line Two");
  REQUIRE(test_set[3] == "Line Three");
  REQUIRE(test_set[4] == "Line 4");
}

int Sum4(int a1, int a2, int a3, int a4) {
  return a1 + a2 + a3 + a4;
}

TEST_CASE("Test flex_function", "[tools]")
{
  emp::flex_function<int(int,int,int,int)> ff = Sum4;
  ff.SetDefaults(10, 100,1000,10000);

  REQUIRE( ff(1,2,3,4) == 10 );
  REQUIRE( ff(1,2,3) == 10006 );
  REQUIRE( ff(1,2) == 11003 );
  REQUIRE( ff(1) == 11101 );
  REQUIRE( ff() == 11110 );
}

TEST_CASE("Test functions", "[tools]")
{

  bool test_bool = true;
  emp::Toggle(test_bool);
  REQUIRE(test_bool == false);

  REQUIRE(emp::ToRange(-10000, 10, 20) == 10);
  REQUIRE(emp::ToRange(9, 10, 20) == 10);
  REQUIRE(emp::ToRange(10, 10, 20) == 10);
  REQUIRE(emp::ToRange(11, 10, 20) == 11);
  REQUIRE(emp::ToRange(17, 10, 20) == 17);
  REQUIRE(emp::ToRange(20, 10, 20) == 20);
  REQUIRE(emp::ToRange(21, 10, 20) == 20);
  REQUIRE(emp::ToRange(12345678, 10, 20) == 20);
  REQUIRE(emp::ToRange<double>(12345678, 10, 20.1) == 20.1);
  REQUIRE(emp::ToRange(12345678.0, 10.7, 20.1) == 20.1);
}

// should migrate these inside the test case, probably

// utility funcs for FunctionSet test case
int global_var1;
int global_var2;
int global_var3;
int global_var4;

void fun1(int in1, int in2) { global_var1 = in1 + in2; }
void fun2(int in1, int in2) { global_var2 = in1 - in2; }
void fun3(int in1, int in2) { global_var3 = in1 * in2; }
void fun4(int in1, int in2) { global_var4 = in1 / in2; }

double fun5(double input) { return input; }
double fun6(double input) { return input * input; }
double fun7(double input) { return input * input * input; }

TEST_CASE("Test FunctionSet", "[tools]")
{
  // TEST 1: Functions with VOID returns.
  emp::FunctionSet<void(int,int)> fun_set;
  fun_set.Add(&fun1);
  fun_set.Add(&fun2);
  fun_set.Add(&fun3);
  fun_set.Add(&fun4);

  fun_set.Run(10, 2);

  REQUIRE(global_var1 == 12);
  REQUIRE(global_var2 == 8);
  REQUIRE(global_var3 == 20);
  REQUIRE(global_var4 == 5);

  fun_set.Remove(2);           // What if we remove a function

  fun_set.Run(20, 5);

  REQUIRE(global_var1 == 25);
  REQUIRE(global_var2 == 15);
  REQUIRE(global_var3 == 20);  // Same as last time!!
  REQUIRE(global_var4 == 4);

  // Test 2: Functions with non-void returns.
  emp::FunctionSet<double(double)> fun_set2;
  fun_set2.Add(&fun5);
  fun_set2.Add(&fun6);
  fun_set2.Add(&fun7);

  emp::vector<double> out_vals;

  out_vals = fun_set2.Run(10.0);

  REQUIRE(out_vals[0] == 10.0);
  REQUIRE(out_vals[1] == 100.0);
  REQUIRE(out_vals[2] == 1000.0);

  out_vals = fun_set2.Run(-10.0);

  REQUIRE(out_vals[0] == -10.0);
  REQUIRE(out_vals[1] == 100.0);
  REQUIRE(out_vals[2] == -1000.0);

  out_vals = fun_set2.Run(0.1);

  REQUIRE(out_vals[0] == 0.1);
  REQUIRE(out_vals[1] < 0.01000001);
  REQUIRE(out_vals[1] > 0.00999999);
  REQUIRE(out_vals[2] < 0.00100001);
  REQUIRE(out_vals[2] > 0.00099999);

  out_vals = fun_set2.Run(-0.1);

  REQUIRE(out_vals[0] == -0.1);
  REQUIRE(out_vals[1] < 0.01000001);
  REQUIRE(out_vals[1] > 0.00999999);
  REQUIRE(out_vals[2] > -0.00100001);
  REQUIRE(out_vals[2] < -0.00099999);


  // TEST 3: Running functions and auto-processing return values.
  REQUIRE(fun_set2.FindMax(10.0) == 1000.0);
  REQUIRE(fun_set2.FindMax(-10.0) == 100.0);
  REQUIRE(fun_set2.FindMax(0.1) == 0.1);
  REQUIRE(fun_set2.FindMax(-0.1) < 0.0100000001);
  REQUIRE(fun_set2.FindMax(-0.1) > 0.0099999999);

  REQUIRE(fun_set2.FindMin(10.0) == 10.0);
  REQUIRE(fun_set2.FindMin(-10.0) == -1000.0);
  REQUIRE(fun_set2.FindMin(0.1) < 0.0010000001);
  REQUIRE(fun_set2.FindMin(0.1) > 0.0009999999);
  REQUIRE(fun_set2.FindMin(-0.1) == -0.1);

  REQUIRE(fun_set2.FindSum(10.0) == 1110.0);
  REQUIRE(fun_set2.FindSum(-10.0) == -910.0);
  REQUIRE(fun_set2.FindSum(0.1) < 0.1110000001);
  REQUIRE(fun_set2.FindSum(0.1) > 0.1109999999);
  REQUIRE(fun_set2.FindSum(-0.1) == -0.091);


  // TEST 4: Manually-entered aggregation function.
  std::function<double(double,double)> test_fun =
    [](double i1, double i2){ return std::max(i1,i2); };

  REQUIRE(fun_set2.Run(10.0, test_fun) == 1000.0);
  REQUIRE(fun_set2.Run(-10.0, test_fun) == 100.0);
  REQUIRE(fun_set2.Run(0.1, test_fun) == 0.1);
  REQUIRE(fun_set2.Run(-0.1, test_fun) < 0.01000000001);
  REQUIRE(fun_set2.Run(-0.1, test_fun) > 0.00999999999);
}

// TODO: add moar asserts
TEST_CASE("Test graph", "[tools]")
{

  emp::Graph graph(20);

  REQUIRE(graph.GetSize() == 20);

  graph.AddEdgePair(0, 1);
  graph.AddEdgePair(0, 2);
  graph.AddEdgePair(0, 3);

}

// // TODO: add asserts
// emp::Random grand;
TEST_CASE("Test Graph utils", "[tools]")
{
  emp::Random random(1);
  // emp::Graph graph( emp::build_graph_tree(20, random) );
  // emp::Graph graph( emp::build_graph_random(20, 40, random) );
  emp::Graph graph( emp::build_graph_grid(5, 4, random) );

  // graph.PrintSym();
}

// // TODO: add asserts
// emp::Random grand;
TEST_CASE("Test hash_utils", "[tools]")
{
  {
  REQUIRE(emp::szudzik_hash((uint32_t)0, (uint32_t)0) == (uint64_t)0);

  REQUIRE(emp::szudzik_hash((uint32_t)0, (uint32_t)1) == (uint64_t)1);

  REQUIRE(emp::szudzik_hash((uint32_t)1, (uint32_t)0) == (uint64_t)2);
  REQUIRE(emp::szudzik_hash((uint32_t)1, (uint32_t)1) == (uint64_t)3);

  REQUIRE(emp::szudzik_hash((uint32_t)0, (uint32_t)2) == (uint64_t)4);
  REQUIRE(emp::szudzik_hash((uint32_t)1, (uint32_t)2) == (uint64_t)5);

  REQUIRE(emp::szudzik_hash((uint32_t)2, (uint32_t)0) == (uint64_t)6);
  REQUIRE(emp::szudzik_hash((uint32_t)2, (uint32_t)1) == (uint64_t)7);
  REQUIRE(emp::szudzik_hash((uint32_t)2, (uint32_t)2) == (uint64_t)8);

  REQUIRE(emp::szudzik_hash((uint32_t)0, (uint32_t)3) == (uint64_t)9);
  REQUIRE(emp::szudzik_hash((uint32_t)1, (uint32_t)3) == (uint64_t)10);
  REQUIRE(emp::szudzik_hash((uint32_t)2, (uint32_t)3) == (uint64_t)11);

  REQUIRE(emp::szudzik_hash((uint32_t)3, (uint32_t)0) == (uint64_t)12);
  REQUIRE(emp::szudzik_hash((uint32_t)3, (uint32_t)1) == (uint64_t)13);
  REQUIRE(emp::szudzik_hash((uint32_t)3, (uint32_t)2) == (uint64_t)14);
  REQUIRE(emp::szudzik_hash((uint32_t)3, (uint32_t)3) == (uint64_t)15);


  REQUIRE(emp::szudzik_hash((uint32_t)0, (uint32_t)0) == (uint64_t)0);

  REQUIRE(emp::szudzik_hash((uint32_t)0, (uint32_t)1) == (uint64_t)1);

  REQUIRE(emp::szudzik_hash((uint32_t)1, (uint32_t)0) == (uint64_t)2);
  REQUIRE(emp::szudzik_hash((uint32_t)1, (uint32_t)1) == (uint64_t)3);

  REQUIRE(emp::szudzik_hash((uint32_t)0, (uint32_t)2) == (uint64_t)4);
  REQUIRE(emp::szudzik_hash((uint32_t)1, (uint32_t)2) == (uint64_t)5);

  REQUIRE(emp::szudzik_hash((uint32_t)2, (uint32_t)0) == (uint64_t)6);
  REQUIRE(emp::szudzik_hash((uint32_t)2, (uint32_t)1) == (uint64_t)7);
  REQUIRE(emp::szudzik_hash((uint32_t)2, (uint32_t)2) == (uint64_t)8);

  REQUIRE(emp::szudzik_hash((uint32_t)0, (uint32_t)3) == (uint64_t)9);
  REQUIRE(emp::szudzik_hash((uint32_t)1, (uint32_t)3) == (uint64_t)10);
  REQUIRE(emp::szudzik_hash((uint32_t)2, (uint32_t)3) == (uint64_t)11);

  REQUIRE(emp::szudzik_hash((uint32_t)3, (uint32_t)0) == (uint64_t)12);
  REQUIRE(emp::szudzik_hash((uint32_t)3, (uint32_t)1) == (uint64_t)13);
  REQUIRE(emp::szudzik_hash((uint32_t)3, (uint32_t)2) == (uint64_t)14);
  REQUIRE(emp::szudzik_hash((uint32_t)3, (uint32_t)3) == (uint64_t)15);

  emp::vector<uint64_t> hash_vec;

  for(uint32_t i = 0; i < 10; ++i) {
    for(uint32_t j = 0; j < 10; ++j) {
      for(uint32_t s : { 0, 100, 100000 }) {
        hash_vec.push_back(emp::szudzik_hash(s+i,s+j));
      }
    }
  }

  std::unordered_set<uint64_t> hash_set(hash_vec.begin(), hash_vec.end());

  REQUIRE(hash_vec.size() == hash_set.size());
  }
  {
  REQUIRE(emp::hash_combine((size_t) 0, (size_t) 0) == (size_t) 0);
  REQUIRE(emp::hash_combine((size_t) 0, (size_t) 1) == (size_t) 2654435769);
  REQUIRE(emp::hash_combine((size_t) 0, (size_t) 2) == (size_t) 5308871538);
  REQUIRE(emp::hash_combine((size_t) 1, (size_t) 0) == (size_t) 65);
  REQUIRE(emp::hash_combine((size_t) 2, (size_t) 0) == (size_t) 130);

  REQUIRE(emp::hash_combine((size_t) 1, (size_t) 1) == (size_t) 2654435832);
  REQUIRE(emp::hash_combine((size_t) 2, (size_t) 2) == (size_t) 5308871664);
  REQUIRE(emp::hash_combine((size_t) 3, (size_t) 3) == (size_t) 7963307496);

  REQUIRE(emp::hash_combine((size_t) 1, (size_t) 3) == (size_t) 7963307370);
  REQUIRE(emp::hash_combine((size_t) 3, (size_t) 1) == (size_t) 2654435962);
  }
}


/*TEST_CASE("Test grid", "[tools]")
{
  emp::Grid::Layout layout(10,10);

  REQUIRE(layout.GetNumRegions() == 0);
  layout.AddRegion({0,1,2,3,4,5,6,7,8});

  REQUIRE(layout.GetNumRegions() == 1);

  emp::Grid::Board<void, void, void> board1(layout);
  emp::Grid::Board<bool, bool, bool> board2(layout);
  emp::Grid::Board<int, int, int> board3(layout);
  emp::Grid::Board<int, void, void> board4(layout);
  emp::Grid::Board<void, int, void> board5(layout);
  emp::Grid::Board<void, void, int> board6(layout);
  //TODO: moar asserts
}*/


// TODO: add moar asserts
TEST_CASE("Test IndexMap", "[tools]")
{
  emp::IndexMap imap(8);
  imap[0] = 1.0;
  imap[1] = 1.0;
  imap[2] = 1.0;
  imap[3] = 1.0;
  imap[4] = 2.0;
  imap[5] = 2.0;
  imap[6] = 0.0;
  imap[7] = 8.0;

  REQUIRE(imap.GetSize() == 8);
  REQUIRE(imap.GetWeight() == 16.0);
  REQUIRE(imap.GetWeight(2) == 1.0);
  REQUIRE(imap.GetWeight(5) == 2.0);
  REQUIRE(imap.GetWeight(7) == 8.0);
  REQUIRE(imap[5] == 2.0);
  REQUIRE(imap.GetProb(4) == 0.125);
  REQUIRE(imap.GetProb(7) == 0.5);
  REQUIRE(imap.Index(7.1) == 5);

  // Add a new element to the end of the map that takes up half of the weight.
  imap.PushBack(16.0);

  REQUIRE(imap.GetSize() == 9);
  REQUIRE(imap.GetWeight() == 32.0);
  REQUIRE(imap.GetWeight(2) == 1.0);
  REQUIRE(imap.GetWeight(5) == 2.0);
  REQUIRE(imap.GetWeight(7) == 8.0);
  REQUIRE(imap.GetWeight(8) == 16.0);
  REQUIRE(imap[5] == 2.0);
  REQUIRE(imap.GetProb(7) == 0.25);
  REQUIRE(imap.Index(7.1) == 5);
  REQUIRE(imap.Index(17.1) == 8);

  emp::IndexMap imap_bak(imap);
  imap.AdjustAll(10.0);

  REQUIRE(imap.GetSize() == 9);
  REQUIRE(imap.GetWeight() == 90.0);
  REQUIRE(imap.GetWeight(2) == 10.0);
  REQUIRE(imap.GetWeight(8) == 10.0);
  REQUIRE(imap[5] == 10.0);
  REQUIRE(imap.Index(7.1) == 0);
  REQUIRE(imap.Index(75.0) == 7);

  // Did the backup copy work correctly?
  REQUIRE(imap_bak.GetSize() == 9);
  REQUIRE(imap_bak.GetWeight() == 32.0);
  REQUIRE(imap_bak.GetWeight(2) == 1.0);
  REQUIRE(imap_bak.GetWeight(5) == 2.0);
  REQUIRE(imap_bak.GetWeight(7) == 8.0);
  REQUIRE(imap_bak.GetWeight(8) == 16.0);
  REQUIRE(imap_bak[5] == 2.0);
  REQUIRE(imap_bak.GetProb(7) == 0.25);
  REQUIRE(imap_bak.Index(7.1) == 5);
  REQUIRE(imap_bak.Index(17.1) == 8);

  // Can we add on values from one index map to another?
  imap += imap_bak;

  REQUIRE(imap.GetSize() == 9);
  REQUIRE(imap.GetWeight() == 122.0);
  REQUIRE(imap.GetWeight(2) == 11.0);
  REQUIRE(imap.GetWeight(5) == 12.0);
  REQUIRE(imap.GetWeight(7) == 18.0);
  REQUIRE(imap.GetWeight(8) == 26.0);
  REQUIRE(imap[5] == 12.0);
  REQUIRE(imap.Index(7.1) == 0);
  REQUIRE(imap.Index(90.0) == 7);

  // And subtraction?
  imap -= imap_bak;

  REQUIRE(imap.GetSize() == 9);
  REQUIRE(imap.GetWeight() == 90.0);
  REQUIRE(imap.GetWeight(2) == 10.0);
  REQUIRE(imap.GetWeight(8) == 10.0);
  REQUIRE(imap[5] == 10.0);
  REQUIRE(imap.Index(7.1) == 0);
  REQUIRE(imap.Index(75.0) == 7);
}

TEST_CASE("Test info_theory", "[tools]")
{
  emp::vector<int> weights = { 100, 100, 200 };
  REQUIRE( emp::Entropy(weights) == 1.5 );

  emp::vector<double> dweights = { 10.5, 10.5, 10.5, 10.5, 21.0, 21.0 };
  REQUIRE( emp::Entropy(dweights) == 2.5 );

  REQUIRE( emp::Entropy2(0.5) == 1.0 );
}

TEST_CASE("Test lexer_utils", "[tools]")
{
  emp::NFA nfa2c(3);  // Must have zero or two c's with any number of a's or b's.
  nfa2c.AddTransition(0,0,"ab");
  nfa2c.AddTransition(0,1,"c");
  nfa2c.AddTransition(1,1,"ab");
  nfa2c.AddTransition(1,2,"c");
  nfa2c.AddTransition(2,2,"ab");
  nfa2c.AddFreeTransition(0,2);
  nfa2c.SetStop(2);

  emp::RegEx re2f("[de]*f[de]*f[de]*");
  // emp::RegEx re2f("([de]*)f([de]*)f([de]*)");
  emp::NFA nfa2f = to_NFA(re2f);
  emp::DFA dfa2f = to_DFA(nfa2f);
  REQUIRE( nfa2f.GetSize() == 15 );
  REQUIRE( dfa2f.GetSize() == 4 );

  int state;
  state = dfa2f.Next(0, "a");        REQUIRE(dfa2f.IsStop(state) == false);
  state = dfa2f.Next(0, "d");        REQUIRE(dfa2f.IsStop(state) == false);
  state = dfa2f.Next(0, "defdef");   REQUIRE(dfa2f.IsStop(state) == true);
  state = dfa2f.Next(0, "fedfed");   REQUIRE(dfa2f.IsStop(state) == true);
  state = dfa2f.Next(0, "ffed");     REQUIRE(dfa2f.IsStop(state) == true);
  state = dfa2f.Next(0, "edffed");   REQUIRE(dfa2f.IsStop(state) == true);
  state = dfa2f.Next(0, "edffedf");  REQUIRE(dfa2f.IsStop(state) == false);
  state = dfa2f.Next(0, "defed");    REQUIRE(dfa2f.IsStop(state) == false);
  state = dfa2f.Next(0, "ff");       REQUIRE(dfa2f.IsStop(state) == true);

  emp::RegEx re_lower("[a-z]+");
  emp::RegEx re_upper("[A-Z]+");
  emp::RegEx re_inc("[a-z]+[A-Z]+");
  emp::NFA nfa_lower = to_NFA(re_lower);
  emp::NFA nfa_upper = to_NFA(re_upper);
  emp::NFA nfa_inc = to_NFA(re_inc);
  emp::NFA nfa_all = MergeNFA(nfa_lower, nfa_upper, nfa_inc);
  emp::DFA dfa_lower = to_DFA(nfa_lower);
  emp::DFA dfa_upper = to_DFA(nfa_upper);
  emp::DFA dfa_inc = to_DFA(nfa_inc);
  emp::DFA dfa_all = to_DFA(nfa_all);

  emp::NFA_State lstate(nfa_lower);
  lstate.Reset(); lstate.Next("abc");      REQUIRE(lstate.IsActive() == true);
  lstate.Reset(); lstate.Next("DEF");      REQUIRE(lstate.IsActive() == false);
  lstate.Reset(); lstate.Next("abcDEF");   REQUIRE(lstate.IsActive() == false);
  lstate.Reset(); lstate.Next("ABDdef");   REQUIRE(lstate.IsActive() == false);
  lstate.Reset(); lstate.Next("ABCDEF");   REQUIRE(lstate.IsActive() == false);
  lstate.Reset(); lstate.Next("abcdefghijklmnopqrstuvwxyz");  REQUIRE(lstate.IsActive() == true);
  lstate.Reset(); lstate.Next("ABC-DEF");  REQUIRE(lstate.IsActive() == false);

  REQUIRE( dfa_all.Next(0, "abc") == 2 );
  REQUIRE( dfa_all.Next(0, "DEF") == 1 );
  REQUIRE( dfa_all.Next(0, "abcDEF") == 3 );
  REQUIRE( dfa_all.Next(0, "ABDdef") == -1 );
  REQUIRE( dfa_all.Next(0, "ABCDEF") == 1 );
  REQUIRE( dfa_all.Next(0, "abcdefghijklmnopqrstuvwxyz") == 2 );
  REQUIRE( dfa_all.Next(0, "ABC-DEF") == -1 );
}

TEST_CASE("Test keyname_utils", "[tools]")
{

  // test unpack

  emp::keyname::unpack_t goal{
      {"seed", "100"},
      {"foobar", "20"},
      {"_hash", "asdf"},
      {"ext", ".txt"}
  };

  std::string name;

  name = "seed=100+foobar=20+_hash=asdf+ext=.txt";
  goal["_"] = name;
  REQUIRE( emp::keyname::unpack(name) == goal );


  // reorderings
  name = "foobar=20+seed=100+_hash=asdf+ext=.txt";
  goal["_"] = name;
  REQUIRE( emp::keyname::unpack(name) == goal );


  name = "_hash=asdf+foobar=20+seed=100+ext=.txt";
  goal["_"] = name;
  REQUIRE( emp::keyname::unpack(name) == goal );


  // should ignore path
  name = "path/seed=100+foobar=20+_hash=asdf+ext=.txt";
  goal["_"] = name;
  REQUIRE( emp::keyname::unpack(name) == goal );


  name = "~/more=path/+blah/seed=100+foobar=20+_hash=asdf+ext=.txt";
  goal["_"] = name;
  REQUIRE( emp::keyname::unpack(name) == goal );


  name = "just/a/regular/file.pdf";
  REQUIRE( emp::keyname::unpack(name) == (emp::keyname::unpack_t{
    {"file.pdf", ""},
    {"_", "just/a/regular/file.pdf"}
  }));

  name = "key/with/no+=value/file+ext=.pdf";
  REQUIRE( emp::keyname::unpack(name) == (emp::keyname::unpack_t{
    {"file", ""},
    {"ext", ".pdf"},
    {"_", "key/with/no+=value/file+ext=.pdf"}
  }));

  name = "multiple/=s/file=biz=blah+ext=.pdf";
  REQUIRE( emp::keyname::unpack(name) == (emp::keyname::unpack_t{
    {"file", "biz=blah"},
    {"ext", ".pdf"},
    {"_", "multiple/=s/file=biz=blah+ext=.pdf"}
  }));

  // test pack
  // reorderings
  REQUIRE( (emp::keyname::pack({
     {"seed", "100"},
     {"foobar", "20"},
     {"_hash", "asdf"},
     {"ext", ".txt"}
    })) == "foobar=20+seed=100+_hash=asdf+ext=.txt"
   );

  REQUIRE( (emp::keyname::pack({
      {"_hash", "asdf"},
      {"seed", "100"},
      {"foobar", "20"},
      {"ext", ".txt"}
   })) == "foobar=20+seed=100+_hash=asdf+ext=.txt"
  );

  REQUIRE( (emp::keyname::pack({
      {"_hash", "asdf"},
      {"foobar", "20"},
      {"ext", ".txt"},
      {"seed", "100"}
   })) == "foobar=20+seed=100+_hash=asdf+ext=.txt"
  );

  // different values
  REQUIRE( (emp::keyname::pack({
      {"seed", "100"},
      {"foobar", "blip"},
      {"_hash", "asdf"},
      {"ext", ".txt"}
   })) == "foobar=blip+seed=100+_hash=asdf+ext=.txt"
  );

  REQUIRE( (emp::keyname::pack({
      {"seed", "a100"},
      {"foobar", "blip"},
      {"_hash", "asdf"},
      {"ext", ".txt"}
   })) == "foobar=blip+seed=a100+_hash=asdf+ext=.txt"
  );

  REQUIRE( (emp::keyname::pack({
      {"aseed", "a100"},
      {"foobar", "blip"},
      {"_hash", "asdf"},
      {"ext", ".txt"}
   })) == "aseed=a100+foobar=blip+_hash=asdf+ext=.txt"
  );

  // should ignore "_" key
  REQUIRE( (emp::keyname::pack({
      {"seed", "100"},
      {"foobar", "20"},
      {"_hash", "asdf"},
      {"ext", ".txt"},
      {"_", "foobar=20+seed=100+_hash=asdf+ext=.txt"}
   })) == "foobar=20+seed=100+_hash=asdf+ext=.txt"
  );

  REQUIRE( (emp::keyname::pack({
      {"seed", "100"},
      {"foobar", "20"},
      {"_hash", "asdf"},
      {"ext", ".txt"},
      {"_", "path/seed=100+foobar=20+_hash=asdf+ext=.txt"}
   })) == "foobar=20+seed=100+_hash=asdf+ext=.txt"
  );

  REQUIRE( (emp::keyname::pack({
      {"seed", "100"},
      {"foobar", "20"},
      {"_hash", "asdf"},
      {"ext", ".txt"},
      {"_", "~/more=path/+blah/seed=100+foobar=20+_hash=asdf+ext=.txt"}
   })) == "foobar=20+seed=100+_hash=asdf+ext=.txt"
  );

  REQUIRE( (emp::keyname::pack({
      {"seed", "100"},
      {"foobar", "20"},
      {"_hash", "asdf"},
      {"ext", ".txt"},
      {"_", "\"whatever+=/\""}
   })) == "foobar=20+seed=100+_hash=asdf+ext=.txt"
  );

  // missing extension
  REQUIRE( (emp::keyname::pack({
      {"_hash", "asdf"},
      {"foobar", "20"},
      {"seed", "100"}
   })) == "foobar=20+seed=100+_hash=asdf"
  );


}

TEST_CASE("Test Lexer", "[tools]")
{
  emp::Lexer lexer;
  lexer.AddToken("Integer", "[0-9]+");
  lexer.AddToken("Float", "[0-9]*\\.[0-9]+");
  lexer.AddToken("Lower", "[a-z]+");
  lexer.AddToken("Upper", "[A-Z]+");
  lexer.AddToken("Mixed", "[a-zA-Z]+");
  lexer.AddToken("Whitespace", "[ \t\n\r]");
  lexer.AddToken("Other", ".");

  std::stringstream ss;
  ss << "This is a 123 TEST.  It should also have 1. .2 123.456 789 FLOATING point NUMbers!";

  REQUIRE(lexer.Process(ss).lexeme == "This");
  REQUIRE(lexer.Process(ss).lexeme == " ");
  REQUIRE(lexer.Process(ss).lexeme == "is");
  REQUIRE(lexer.Process(ss).lexeme == " ");
  REQUIRE(lexer.Process(ss).lexeme == "a");
  REQUIRE(lexer.Process(ss).lexeme == " ");
  REQUIRE(lexer.Process(ss).lexeme == "123");
  REQUIRE(lexer.Process(ss).lexeme == " ");
  REQUIRE(lexer.Process(ss).lexeme == "TEST");
  REQUIRE(lexer.Process(ss).lexeme == ".");
  REQUIRE(lexer.Process(ss).lexeme == " ");
  REQUIRE(lexer.Process(ss).lexeme == " ");

  REQUIRE(lexer.GetTokenName(lexer.Process(ss)) == "Mixed");
  REQUIRE(lexer.GetTokenName(lexer.Process(ss)) == "Whitespace");
  REQUIRE(lexer.GetTokenName(lexer.Process(ss)) == "Lower");
}


TEST_CASE("Test map_utils", "[tools]")
{
  std::map<int, char> test_map;
  test_map[0] = 'a';
  test_map[4] = 'e';
  test_map[8] = 'i';
  test_map[14] = 'o';
  test_map[20] = 'u';

  REQUIRE( emp::Has(test_map, 8) == true );
  REQUIRE( emp::Has(test_map, 18) == false );
  REQUIRE( emp::Find(test_map, 14, 'x') == 'o'); // 14 should be there as 'o'
  REQUIRE( emp::Find(test_map, 15, 'x') == 'x'); // 15 shouldn't be there, so return default.
  REQUIRE( emp::Has(test_map, 15) == false );    // Make sure 15 hasn't been added to the map.

  auto flipped = emp::flip_map(test_map);        // Make sure we can reverse the map.
  REQUIRE( emp::Has(flipped, 'u') == true);      // And the reversed map should have proper info.
  REQUIRE( emp::Has(flipped, 'x') == false);

  // Testing for bug #123
  std::map<std::string, std::string> test_123;
  test_123["1"] = "1";
  test_123["12"] = "12";

  REQUIRE( emp::Find(test_123, "0", "nothing") == "nothing" );
  REQUIRE( emp::Find(test_123, "1", "nothing") == "1" );
  REQUIRE( emp::FindRef(test_123, "1", "nothing") == "1" );
}

TEST_CASE("Test matchbin_utils", "[tools]")
{

  // test ExactStreakDistribution
  {
    emp::ExactStreakDistribution<4> dist;

    REQUIRE( dist.StreakProbability(2,2) == 0.25 );
    REQUIRE( dist.StreakProbability(2,3) == 0.375 );
    REQUIRE( dist.StreakProbability(2,4) == 8.0/16.0 );

    REQUIRE( dist.StreakProbability(0) == 16.0/16.0 );
    REQUIRE( dist.StreakProbability(1) == 15.0/16.0 );
    REQUIRE( dist.StreakProbability(2) == 8.0/16.0 );
    REQUIRE( dist.StreakProbability(3) == 3.0/16.0 );
    REQUIRE( dist.StreakProbability(4) == 1.0/16.0 );

  }

  // test ApproxSingleStreakMetric
  {

    emp::ApproxSingleStreakMetric<4> metric;

    REQUIRE( metric({0,0,0,0},{0,0,0,0}) < metric({0,0,0,0},{1,0,0,0}) );
    REQUIRE( metric({0,0,0,0},{0,0,0,1}) < metric({0,0,0,0},{0,1,0,0}) );
    // REQUIRE( metric({0,0,0,0},{1,1,0,1}) < metric({0,0,0,0},{1,1,1,1}) );
    REQUIRE( metric({0,0,0,0},{1,1,0,1}) == 1.0 ); // in lieu
    REQUIRE( metric({0,0,0,0},{1,1,1,1}) == 1.0 ); // in lieu
    REQUIRE( metric({0,0,0,0},{1,1,0,0}) < metric({0,0,0,0},{1,1,0,1}) );
    // REQUIRE( metric({0,0,0,0},{1,1,0,1}) < metric({0,0,0,0},{1,1,1,0}) );
    REQUIRE( metric({0,0,0,0},{1,1,0,1}) == 1.0 ); // in lieu
    REQUIRE( metric({0,0,0,0},{1,1,1,0}) == 1.0 ); // in lieu
    REQUIRE( metric({0,0,0,0},{0,0,1,1}) == metric({0,0,0,0},{0,0,1,0}) );

    emp::Random rand(1);
    for (size_t i = 0; i < 1000; ++i) {
      emp::BitSet<4> a(rand);
      emp::BitSet<4> b(rand);
      REQUIRE(metric(a,b) <= 1.0);
      REQUIRE(metric(a,b) >= 0.0);
    }

  }

  // test ApproxDualStreakMetric
  emp::ExactDualStreakMetric<4> metric;

  REQUIRE( metric({0,0,0,0},{0,0,0,0}) < metric({0,0,0,0},{1,0,0,0}) );
  REQUIRE( metric({0,0,0,0},{0,0,0,1}) < metric({0,0,0,0},{0,1,0,0}) );
  REQUIRE( metric({0,0,0,0},{1,1,0,1}) < metric({0,0,0,0},{1,1,1,1}) );
  REQUIRE( metric({0,0,0,0},{1,1,0,0}) < metric({0,0,0,0},{1,1,0,1}) );
  REQUIRE( metric({0,0,0,0},{1,1,0,1}) < metric({0,0,0,0},{1,1,1,0}) );
  REQUIRE( metric({0,0,0,0},{0,0,1,1}) > metric({0,0,0,0},{0,0,1,0}) );

  emp::Random rand(1);
  for (size_t i = 0; i < 1000; ++i) {
    emp::BitSet<4> a(rand);
    emp::BitSet<4> b(rand);
    REQUIRE(metric(a,b) <= 1.0);
    REQUIRE(metric(a,b) >= 0.0);
  }


  // test ExactSingleStreakMetric
  {

    emp::ExactSingleStreakMetric<4> metric;

    REQUIRE( metric({0,0,0,0},{0,0,0,0}) < metric({0,0,0,0},{1,0,0,0}) );
    REQUIRE( metric({0,0,0,0},{0,0,0,1}) < metric({0,0,0,0},{0,1,0,0}) );
    REQUIRE( metric({0,0,0,0},{1,1,0,1}) < metric({0,0,0,0},{1,1,1,1}) );
    REQUIRE( metric({0,0,0,0},{1,1,0,0}) < metric({0,0,0,0},{1,1,0,1}) );
    REQUIRE( metric({0,0,0,0},{1,1,0,1}) == metric({0,0,0,0},{1,1,1,0}) );
    REQUIRE( metric({0,0,0,0},{0,0,1,1}) == metric({0,0,0,0},{0,0,1,0}) );

    emp::Random rand(1);
    for (size_t i = 0; i < 1000; ++i) {
      emp::BitSet<4> a(rand);
      emp::BitSet<4> b(rand);
      REQUIRE(metric(a,b) <= 1.0);
      REQUIRE(metric(a,b) >= 0.0);
    }

  }

  // test ExactDualStreakMetric
  {

    emp::ExactDualStreakMetric<4> metric;

    REQUIRE( metric({0,0,0,0},{0,0,0,0}) < metric({0,0,0,0},{1,0,0,0}) );
    REQUIRE( metric({0,0,0,0},{0,0,0,1}) < metric({0,0,0,0},{0,1,0,0}) );
    REQUIRE( metric({0,0,0,0},{1,1,0,1}) < metric({0,0,0,0},{1,1,1,1}) );
    REQUIRE( metric({0,0,0,0},{1,1,0,0}) < metric({0,0,0,0},{1,1,0,1}) );
    REQUIRE( metric({0,0,0,0},{1,1,0,1}) < metric({0,0,0,0},{1,1,1,0}) );
    REQUIRE( metric({0,0,0,0},{0,0,1,1}) > metric({0,0,0,0},{0,0,1,0}) );

    emp::Random rand(1);
    for (size_t i = 0; i < 1000; ++i) {
      emp::BitSet<4> a(rand);
      emp::BitSet<4> b(rand);
      REQUIRE(metric(a,b) <= 1.0);
      REQUIRE(metric(a,b) >= 0.0);
    }

  }

  // test SieveSelector with auto adjust
  {
  emp::Random rand(1);
  emp::MatchBin<
    std::string,
    emp::NextUpMetric<>,
    emp::SieveSelector<>,
    emp::AdditiveCountdownRegulator<>
  > bin(rand);

  bin.Put("one", 1);

  bin.Put("two-two-seven", 227);

  bin.Put("nine-two-eight", 928);

  bin.Put("fifteen", 15);

  bin.Put("one-fifteen", 115);

  const size_t nrep = 1000;

  std::unordered_map<std::string, size_t> res;
  for (size_t rep = 0; rep < nrep; ++rep) {

    const auto matches = bin.GetVals(bin.Match(2));
    REQUIRE(matches.size() >= 2);

    std::unordered_set<std::string> uniques;

    for (const auto & val : matches) {
      ++res[val];
      uniques.insert(val);
    }

    REQUIRE(uniques.size() == matches.size());

  }

  REQUIRE(res["one"] == 0);
  REQUIRE(res["two-two-seven"] > 0);
  REQUIRE(res["two-two-seven"] < nrep);
  REQUIRE(res["nine-two-eight"] == 0);
  REQUIRE(res["one-fifteen"] == nrep);
  REQUIRE(res["fifteen"] == nrep);

  bin.Put(emp::to_string(0), 0);
  for (size_t i = 0; i < 45; ++i) {
    bin.Put(emp::to_string(i*10), i*10);
  }

  res.clear();

  for (size_t rep = 0; rep < nrep; ++rep) {
    for (const auto & val : bin.GetVals(bin.Match(2))) {
      ++res[val];
    }
  }

  REQUIRE(res["one"] == 0);
  REQUIRE(res["two-two-seven"] == 0);
  REQUIRE(res["nine-two-eight"] == 0);
  REQUIRE(res["one-fifteen"] > 0);
  REQUIRE(res["one-fifteen"] < nrep);
  REQUIRE(res["fifteen"] == nrep);
  }

  // test SieveSelector with no stochastic
  {
  emp::Random rand(1);
  emp::MatchBin<
    std::string,
    emp::NextUpMetric<>,
    emp::SieveSelector<std::ratio<0,1>>,
    emp::AdditiveCountdownRegulator<>
  > bin(rand);

  bin.Put("one", 1);

  bin.Put("two-two-seven", 227);

  bin.Put("nine-two-eight", 928);

  bin.Put("fifteen", 15);

  bin.Put("one-fifteen", 115);

  const size_t nrep = 1000;

  std::unordered_map<std::string, size_t> res;
  for (size_t rep = 0; rep < nrep; ++rep) {

    const auto matches = bin.GetVals(bin.Match(2));
    REQUIRE(matches.size() >= 2);

    std::unordered_set<std::string> uniques;

    for (const auto & val : matches) {
      ++res[val];
      uniques.insert(val);
    }

    REQUIRE(uniques.size() == matches.size());

  }

  REQUIRE(res["one"] == 0);
  REQUIRE(res["two-two-seven"] == 0);
  REQUIRE(res["nine-two-eight"] == 0);
  REQUIRE(res["one-fifteen"] == nrep);
  REQUIRE(res["fifteen"] == nrep);

  bin.Put(emp::to_string(0), 0);
  for (size_t i = 0; i < 45; ++i) {
    bin.Put(emp::to_string(i*10), i*10);
  }

  res.clear();

  for (size_t rep = 0; rep < nrep; ++rep) {
    for (const auto & val : bin.GetVals(bin.Match(2))) {
      ++res[val];
    }
  }

  REQUIRE(res["one"] == 0);
  REQUIRE(res["two-two-seven"] == 0);
  REQUIRE(res["nine-two-eight"] == 0);
  REQUIRE(res["one-fifteen"] == 0);
  REQUIRE(res["fifteen"] == nrep);
  }

  // test SieveSelector with no auto adjust
  {
  emp::Random rand(1);
  emp::MatchBin<
    std::string,
    emp::NextUpMetric<>,
    emp::SieveSelector<
      std::ratio<1, 10>,
      std::ratio<1, 5>
    >,
    emp::AdditiveCountdownRegulator<>
  > bin(rand);

  bin.Put("one", 1);

  bin.Put("two-two-seven", 227);

  bin.Put("nine-two-eight", 928);

  bin.Put("fifteen", 15);

  bin.Put("one-fifteen", 115);

  const size_t nrep = 1000;

  std::unordered_map<std::string, size_t> res;
  for (size_t rep = 0; rep < nrep; ++rep) {

    const auto matches = bin.GetVals(bin.Match(2));
    REQUIRE(matches.size() >= 2);

    std::unordered_set<std::string> uniques;

    for (const auto & val : matches) {
      ++res[val];
      uniques.insert(val);
    }

    REQUIRE(uniques.size() == matches.size());

  }

  REQUIRE(res["one"] == 0);
  REQUIRE(res["two-two-seven"] > 0);
  REQUIRE(res["two-two-seven"] < nrep);
  REQUIRE(res["nine-two-eight"] == 0);
  REQUIRE(res["one-fifteen"] == nrep);
  REQUIRE(res["fifteen"] == nrep);

  bin.Put(emp::to_string(0), 0);
  for (size_t i = 0; i < 45; ++i) {
    bin.Put(emp::to_string(i*10), i*10);
  }

  res.clear();

  for (size_t rep = 0; rep < nrep; ++rep) {
    for (const auto & val : bin.GetVals(bin.Match(2))) {
      ++res[val];
    }
  }

  REQUIRE(res["one"] == 0);
  REQUIRE(res["two-two-seven"] > 0);
  REQUIRE(res["two-two-seven"] < nrep);
  REQUIRE(res["nine-two-eight"] == 0);
  REQUIRE(res["one-fifteen"] == nrep);
  REQUIRE(res["fifteen"] == nrep);
  }

  // test PowMod, LogMod
  {
  emp::HammingMetric<4> baseline;

  emp::PowMod<emp::HammingMetric<4>, std::ratio<3>> squish_pow;
  emp::PowMod<emp::HammingMetric<4>, std::ratio<1>> same_pow;
  emp::PowMod<emp::HammingMetric<4>, std::ratio<1,3>> stretch_pow;

  emp::LogMod<emp::HammingMetric<4>, std::ratio<1,3>> squish_log;
  emp::LogMod<emp::HammingMetric<4>, std::ratio<1>> same_log;
  emp::LogMod<emp::HammingMetric<4>, std::ratio<3>> stretch_log;

  REQUIRE( squish_pow({0,0,0,0},{0,0,0,0}) == baseline({0,0,0,0},{0,0,0,0}) );
  REQUIRE( squish_pow({0,0,0,0},{0,0,0,1}) > baseline({0,0,0,0},{0,0,0,1}) );
  REQUIRE( squish_pow({0,0,0,0},{0,0,1,1}) == baseline({0,0,0,0},{0,0,1,1}) );
  REQUIRE( squish_pow({0,0,0,0},{0,1,1,1}) < baseline({0,0,0,0},{0,1,1,1}) );
  REQUIRE( squish_pow({0,0,0,0},{1,1,1,1}) == baseline({0,0,0,0},{1,1,1,1}) );

  REQUIRE( same_pow({0,0,0,0},{0,0,0,0}) == baseline({0,0,0,0},{0,0,0,0}) );
  REQUIRE( same_pow({0,0,0,0},{0,0,0,1}) == baseline({0,0,0,0},{0,0,0,1}) );
  REQUIRE( same_pow({0,0,0,0},{0,0,1,1}) == baseline({0,0,0,0},{0,0,1,1}) );
  REQUIRE( same_pow({0,0,0,0},{0,1,1,1}) == baseline({0,0,0,0},{0,1,1,1}) );
  REQUIRE( same_pow({0,0,0,0},{1,1,1,1}) == baseline({0,0,0,0},{1,1,1,1}) );

  REQUIRE( stretch_pow({0,0,0,0},{0,0,0,0}) == baseline({0,0,0,0},{0,0,0,0}) );
  REQUIRE( stretch_pow({0,0,0,0},{0,0,0,1}) < baseline({0,0,0,0},{0,0,0,1}) );
  REQUIRE( stretch_pow({0,0,0,0},{0,0,1,1}) == baseline({0,0,0,0},{0,0,1,1}) );
  REQUIRE( stretch_pow({0,0,0,0},{0,1,1,1}) > baseline({0,0,0,0},{0,1,1,1}) );
  REQUIRE( stretch_pow({0,0,0,0},{1,1,1,1}) == baseline({0,0,0,0},{1,1,1,1}) );

  REQUIRE( squish_log({0,0,0,0},{0,0,0,0}) == baseline({0,0,0,0},{0,0,0,0}) );
  REQUIRE( squish_log({0,0,0,0},{0,0,0,1}) > baseline({0,0,0,0},{0,0,0,1}) );
  REQUIRE( squish_log({0,0,0,0},{0,0,1,1}) == baseline({0,0,0,0},{0,0,1,1}) );
  REQUIRE( squish_log({0,0,0,0},{0,1,1,1}) < baseline({0,0,0,0},{0,1,1,1}) );
  REQUIRE( squish_log({0,0,0,0},{1,1,1,1}) == baseline({0,0,0,0},{1,1,1,1}) );

  REQUIRE( same_log({0,0,0,0},{0,0,0,0}) == baseline({0,0,0,0},{0,0,0,0}) );
  REQUIRE( same_log({0,0,0,0},{0,0,0,1}) == baseline({0,0,0,0},{0,0,0,1}) );
  REQUIRE( same_log({0,0,0,0},{0,0,1,1}) == baseline({0,0,0,0},{0,0,1,1}) );
  REQUIRE( same_log({0,0,0,0},{0,1,1,1}) == baseline({0,0,0,0},{0,1,1,1}) );
  REQUIRE( same_log({0,0,0,0},{1,1,1,1}) == baseline({0,0,0,0},{1,1,1,1}) );

  REQUIRE( stretch_log({0,0,0,0},{0,0,0,0}) == baseline({0,0,0,0},{0,0,0,0}) );
  REQUIRE( stretch_log({0,0,0,0},{0,0,0,1}) < baseline({0,0,0,0},{0,0,0,1}) );
  REQUIRE( stretch_log({0,0,0,0},{0,0,1,1}) == baseline({0,0,0,0},{0,0,1,1}) );
  REQUIRE( stretch_log({0,0,0,0},{0,1,1,1}) > baseline({0,0,0,0},{0,1,1,1}) );
  REQUIRE( stretch_log({0,0,0,0},{1,1,1,1}) == baseline({0,0,0,0},{1,1,1,1}) );
  }

  // more tests for PowMod, LogMod
  {

  emp::PowMod<emp::HashMetric<32>, std::ratio<5>> squish_pow;
  emp::PowMod<emp::HashMetric<32>, std::ratio<1>> same_pow;
  emp::PowMod<emp::HashMetric<32>, std::ratio<1,5>> stretch_pow;

  emp::LogMod<emp::HashMetric<32>, std::ratio<1,5>> squish_log;
  emp::LogMod<emp::HashMetric<32>, std::ratio<1>> same_log;
  emp::LogMod<emp::HashMetric<32>, std::ratio<5>> stretch_log;

  emp::Random rand(1);
  for (size_t rep = 0; rep < 1000; ++rep) {
    emp::BitSet<32> a(rand);
    emp::BitSet<32> b(rand);
    REQUIRE(squish_pow(a,b) >= 0.0);
    REQUIRE(squish_pow(a,b) <= 1.0);

    REQUIRE(same_pow(a,b) >= 0.0);
    REQUIRE(same_pow(a,b) <= 1.0);

    REQUIRE(stretch_pow(a,b) >= 0.0);
    REQUIRE(stretch_pow(a,b) <= 1.0);

    REQUIRE(squish_log(a,b) >= 0.0);
    REQUIRE(squish_log(a,b) <= 1.0);

    REQUIRE(same_log(a,b) >= 0.0);
    REQUIRE(same_log(a,b) <= 1.0);

    REQUIRE(stretch_log(a,b) >= 0.0);
    REQUIRE(stretch_log(a,b) <= 1.0);
  }

  }

  // test CacheMod
  // test PowMod, LogMod
  {
  emp::HammingMetric<4> baseline;

  emp::PowMod<emp::HammingMetric<4>, std::ratio<3>> squish;

  emp::CacheMod<emp::PowMod<
    emp::HammingMetric<4>,
    std::ratio<3>
  >> cache_squish;

  emp::CacheMod<emp::PowMod<
    emp::HammingMetric<4>,
    std::ratio<3>
  >, 2> small_cache_squish;

  // put in cache
  REQUIRE( squish({0,0,0,0},{0,0,0,0}) == cache_squish({0,0,0,0},{0,0,0,0}) );
  REQUIRE( squish({0,0,0,0},{0,0,0,1}) == cache_squish({0,0,0,0},{0,0,0,1}) );
  REQUIRE( squish({0,0,0,0},{0,0,1,1}) == cache_squish({0,0,0,0},{0,0,1,1}) );
  REQUIRE( squish({0,0,0,0},{0,1,1,1}) == cache_squish({0,0,0,0},{0,1,1,1}) );
  REQUIRE( squish({0,0,0,0},{1,1,1,1}) == cache_squish({0,0,0,0},{1,1,1,1}) );

  // hit cache
  REQUIRE( squish({0,0,0,0},{0,0,0,0}) == cache_squish({0,0,0,0},{0,0,0,0}) );
  REQUIRE( squish({0,0,0,0},{0,0,0,1}) == cache_squish({0,0,0,0},{0,0,0,1}) );
  REQUIRE( squish({0,0,0,0},{0,0,1,1}) == cache_squish({0,0,0,0},{0,0,1,1}) );
  REQUIRE( squish({0,0,0,0},{0,1,1,1}) == cache_squish({0,0,0,0},{0,1,1,1}) );
  REQUIRE( squish({0,0,0,0},{1,1,1,1}) == cache_squish({0,0,0,0},{1,1,1,1}) );

  // put in cache
  REQUIRE(
    squish({0,0,0,0},{0,0,0,0}) == small_cache_squish({0,0,0,0},{0,0,0,0})
  );
  REQUIRE(
    squish({0,0,0,0},{0,0,0,1}) == small_cache_squish({0,0,0,0},{0,0,0,1})
  );
  REQUIRE(
    squish({0,0,0,0},{0,0,1,1}) == small_cache_squish({0,0,0,0},{0,0,1,1})
  );
  REQUIRE(
    squish({0,0,0,0},{0,1,1,1}) == small_cache_squish({0,0,0,0},{0,1,1,1})
  );
  REQUIRE(
    squish({0,0,0,0},{1,1,1,1}) == small_cache_squish({0,0,0,0},{1,1,1,1})
  );

  // hit cache
  REQUIRE(
    squish({0,0,0,0},{0,0,0,0}) == small_cache_squish({0,0,0,0},{0,0,0,0})
  );
  REQUIRE(
    squish({0,0,0,0},{0,0,0,1}) == small_cache_squish({0,0,0,0},{0,0,0,1})
  );
  REQUIRE(
    squish({0,0,0,0},{0,0,1,1}) == small_cache_squish({0,0,0,0},{0,0,1,1})
  );
  REQUIRE(
    squish({0,0,0,0},{0,1,1,1}) == small_cache_squish({0,0,0,0},{0,1,1,1})
  );
  REQUIRE(
    squish({0,0,0,0},{1,1,1,1}) == small_cache_squish({0,0,0,0},{1,1,1,1})
  );

  }


  // test UnifMod
  {

  emp::HashMetric<32> hash;
  emp::UnifMod<emp::HashMetric<32>> unif_hash;
  emp::UnifMod<emp::HashMetric<32>, 1> unif_hash_small;

  emp::HammingMetric<32> hamming;
  emp::UnifMod<emp::HammingMetric<32>> unif_hamming;
  emp::UnifMod<emp::HammingMetric<32>, 1> unif_hamming_small;

  emp::Random rand(1);

  for (size_t rep = 0; rep < 5000; ++rep) {

    emp::BitSet<32> a(rand);
    emp::BitSet<32> b(rand);

    emp::BitSet<32> c(rand);
    emp::BitSet<32> d(rand);

    REQUIRE(unif_hash(a,b) >= 0.0);
    REQUIRE(unif_hash(a,b) <= 1.0);
    if (unif_hash(a,b) > unif_hash(c,d)) {
      REQUIRE(hash(a,b) > hash(c,d));
    } else if (unif_hash(a,b) < unif_hash(c,d)) {
      REQUIRE(hash(a,b) < hash(c,d));
    } else {
      // unif_hash(a,b) == unif_hash(c,d)
      REQUIRE(hash(a,b) == hash(c,d));
    }

    REQUIRE(unif_hash_small(a,b) >= 0.0);
    REQUIRE(unif_hash_small(a,b) <= 1.0);
    if (unif_hash_small(a,b) > unif_hash_small(c,d)) {
      REQUIRE(hash(a,b) > hash(c,d));
    } else if (unif_hash_small(a,b) < unif_hash_small(c,d)) {
      REQUIRE(hash(a,b) < hash(c,d));
    } else {
      // unif_hash_small(a,b) == unif_hash_small(c,d)
      REQUIRE(hash(a,b) == hash(c,d));
    }

    REQUIRE(unif_hamming(a,b) >= 0.0);
    REQUIRE(unif_hamming(a,b) <= 1.0);
    if (unif_hamming(a,b) > unif_hamming(c,d)) {
      REQUIRE(hamming(a,b) > hamming(c,d));
    } else if (unif_hamming(a,b) < unif_hamming(c,d)) {
      REQUIRE(hamming(a,b) < hamming(c,d));
    } else {
      // unif_hamming(a,b) == unif_hamming(c,d)
      REQUIRE(hamming(a,b) == hamming(c,d));
    }

    REQUIRE(unif_hamming_small(a,b) >= 0.0);
    REQUIRE(unif_hamming_small(a,b) <= 1.0);
    if (unif_hamming_small(a,b) > unif_hamming_small(c,d)) {
      REQUIRE(hamming(a,b) > hamming(c,d));
    } else if (unif_hamming_small(a,b) < unif_hamming_small(c,d)) {
      REQUIRE(hamming(a,b) < hamming(c,d));
    } else {
      // unif_hamming_small(a,b) == unif_hamming_small(c,d)
      REQUIRE(hamming(a,b) == hamming(c,d));
    }

  }

  }

  // test EuclideanDimMod
  {
  emp::Random rand(1);

  emp::BitSet<32> a1(rand);
  emp::BitSet<32> b1(rand);

  emp::HammingMetric<32> hamming;

  emp::FlatMod<
    emp::MeanDimMod<
      typename emp::HammingMetric<32>,
      1
    >
  > d_hamming1;
  REQUIRE(d_hamming1.width() == hamming.width());

  REQUIRE(hamming(a1, b1) == d_hamming1(a1, b1));
  }

  // test EuclideanDimMod
  {
  emp::Random rand(1);

  emp::BitSet<32> a1(rand);
  emp::BitSet<32> b1(rand);

  emp::HammingMetric<32> hamming;

  emp::FlatMod<
    emp::MeanDimMod<
      typename emp::HammingMetric<32>,
      1
    >
  > d_hamming1;
  REQUIRE(d_hamming1.width() == hamming.width());

  REQUIRE(hamming(a1, b1) == d_hamming1(a1, b1));
  }

  // more tests for EuclideanDimMod
  {
    emp::HammingMetric<4> hamming;

    emp::FlatMod<
      emp::EuclideanDimMod<
        typename emp::HammingMetric<2>,
        2
      >
    > d_hamming2;
    REQUIRE(d_hamming2.width() == hamming.width());

    REQUIRE(d_hamming2({0,0,0,0}, {0,0,0,0}) == 0.0);

    REQUIRE(d_hamming2({0,0,1,1}, {0,0,0,0}) == std::sqrt(0.5));
    REQUIRE(d_hamming2({0,0,0,0}, {1,1,0,0}) == std::sqrt(0.5));
    REQUIRE(d_hamming2({0,0,1,1}, {1,1,1,1}) == std::sqrt(0.5));
    REQUIRE(d_hamming2({1,1,1,1}, {0,0,1,1}) == std::sqrt(0.5));

    REQUIRE(d_hamming2({0,0,1,1}, {0,1,1,0}) == 0.5);
    REQUIRE(d_hamming2({0,0,1,1}, {0,1,1,0}) == 0.5);
    REQUIRE(d_hamming2({0,0,0,0}, {0,1,1,0}) == 0.5);
    REQUIRE(d_hamming2({0,1,1,1}, {1,1,1,0}) == 0.5);

    REQUIRE(d_hamming2({0,0,0,0}, {1,1,1,1}) == 1.0);
    REQUIRE(d_hamming2({1,1,1,1}, {0,0,0,0}) == 1.0);
  }

  // more tests for EuclideanDimMod
  {

    emp::FlatMod<
      emp::MeanDimMod<
        typename emp::HammingMetric<8>,
        4
      >
    > metric;

  emp::Random rand(1);
  for (size_t rep = 0; rep < 1000; ++rep) {
    emp::BitSet<32> a(rand);
    emp::BitSet<32> b(rand);
    REQUIRE(metric(a,b) >= 0.0);
    REQUIRE(metric(a,b) <= 1.0);
  }

  }

  // tests for AdditiveCountdownRegulator
  {

  emp::Random rand(1);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RouletteSelector<>,
    emp::AdditiveCountdownRegulator<>
  >bin(rand);

  const size_t ndraws = 100000;

  const size_t hi = bin.Put("hi", std::numeric_limits<int>::max()/5);
  REQUIRE( bin.GetVal(hi) == "hi" );
  const size_t salut = bin.Put("salut", std::numeric_limits<int>::max()/100);
  REQUIRE( bin.GetVal(salut) == "salut" );

  REQUIRE( bin.Size() == 2 );
  REQUIRE( bin.ViewRegulator(hi) == 0.0 );
  REQUIRE( bin.ViewRegulator(salut) == 0.0 );

  auto res = bin.GetVals(bin.Match(0, ndraws));
  const size_t count = std::count(std::begin(res), std::end(res), "salut");
  REQUIRE( count > ndraws/2);
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  bin.AdjRegulator(salut, 20.0); // downregulate
  REQUIRE( bin.ViewRegulator(salut) == 20.0 );
  REQUIRE( bin.ViewRegulator(hi) == 0.0 );
  res = bin.GetVals(bin.Match(0, ndraws));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > 0 );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > ndraws/2 );

  bin.AdjRegulator(hi, -20.0); // upregulate
  bin.AdjRegulator(salut, -20.0); // upregulate
  REQUIRE( bin.ViewRegulator(salut) == 0.0 );
  REQUIRE( bin.ViewRegulator(hi) == -20.0 );
  res = bin.GetVals(bin.Match(0, ndraws));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > 0 );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > ndraws/2 );

  bin.SetRegulator(salut, 2.0); // downregulate
  bin.SetRegulator(hi, -2.0); // upregulate
  REQUIRE( bin.ViewRegulator(salut) == 2.0 );
  REQUIRE( bin.ViewRegulator(hi) == -2.0 );

  res = bin.GetVals(bin.Match(0, ndraws));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > 0 );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > ndraws/2 );

  bin.SetRegulator(salut, -1.0); // upregulate
  bin.SetRegulator(hi, 1.0); // downregulate
  REQUIRE( bin.ViewRegulator(salut) == -1.0 );
  REQUIRE( bin.ViewRegulator(hi) == 1.0 );
  res = bin.GetVals(bin.Match(0, ndraws));
  const size_t hi_count = std::count(std::begin(res), std::end(res), "salut");
  REQUIRE( hi_count > count );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  bin.DecayRegulator(salut, -2);
  REQUIRE( bin.ViewRegulator(salut) == -1.0 );
  REQUIRE( bin.ViewRegulator(hi) == 1.0 );

  res = bin.GetVals(bin.Match(0, ndraws));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > count );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  bin.DecayRegulator(salut, 1);
  bin.DecayRegulator(hi, 0);
  REQUIRE( bin.ViewRegulator(salut) == -1.0 );
  REQUIRE( bin.ViewRegulator(hi) == 1.0 );

  res = bin.GetVals(bin.Match(0, ndraws));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > count );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  bin.DecayRegulator(salut, 500);
  bin.DecayRegulators();
  REQUIRE( bin.ViewRegulator(salut) == 0.0 );
  REQUIRE( bin.ViewRegulator(hi) == 0.0 );
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > ndraws/2 );
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") < hi_count );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  }

  // tests for MultiplicativeCountdownRegulator
  {

  emp::Random rand(1);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RouletteSelector<>,
    emp::MultiplicativeCountdownRegulator<>
  >bin(rand);

  const size_t ndraws = 1000000;

  const size_t hi = bin.Put("hi", std::numeric_limits<int>::max()/2);
  REQUIRE( bin.GetVal(hi) == "hi" );
  const size_t salut = bin.Put("salut", std::numeric_limits<int>::max()/10);
  REQUIRE( bin.GetVal(salut) == "salut" );

  REQUIRE( bin.Size() == 2 );
  REQUIRE( bin.ViewRegulator(hi) == 0.0 );
  REQUIRE( bin.ViewRegulator(salut) == 0.0 );

  auto res = bin.GetVals(bin.Match(0, ndraws));
  const size_t count = std::count(std::begin(res), std::end(res), "salut");
  REQUIRE( count > ndraws/2);
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  bin.AdjRegulator(salut, 20.0); // downregulate
  REQUIRE( bin.ViewRegulator(salut) == 20.0 );
  REQUIRE( bin.ViewRegulator(hi) == 0.0 );
  res = bin.GetVals(bin.Match(0, ndraws));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > 0 );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > ndraws/2 );

  bin.AdjRegulator(hi, -20.0); // upregulate
  bin.AdjRegulator(salut, -20.0); // restore
  REQUIRE( bin.ViewRegulator(salut) == 0.0 );
  REQUIRE( bin.ViewRegulator(hi) == -20.0 );
  res = bin.GetVals(bin.Match(0, ndraws));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > 0 );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > ndraws/2 );

  bin.SetRegulator(salut, 5.0); // downregulate
  bin.SetRegulator(hi, -5.0); // upregulate
  REQUIRE( bin.ViewRegulator(salut) == 5.0 );
  REQUIRE( bin.ViewRegulator(hi) == -5.0 );

  bin.SetRegulator(salut, -1.0); // upregulate
  bin.SetRegulator(hi, 1.0); // downregulate
  REQUIRE( bin.ViewRegulator(salut) == -1.0 );
  REQUIRE( bin.ViewRegulator(hi) == 1.0 );
  res = bin.GetVals(bin.Match(0, ndraws));
  const size_t hi_count = std::count(std::begin(res), std::end(res), "salut");
  REQUIRE( hi_count > count );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  bin.DecayRegulator(salut, -2);
  REQUIRE( bin.ViewRegulator(salut) == -1.0 );
  REQUIRE( bin.ViewRegulator(hi) == 1.0 );

  res = bin.GetVals(bin.Match(0, ndraws));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > count );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  bin.DecayRegulator(salut, 1);
  bin.DecayRegulator(hi, 0);
  REQUIRE( bin.ViewRegulator(salut) == -1.0 );
  REQUIRE( bin.ViewRegulator(hi) == 1.0 );

  res = bin.GetVals(bin.Match(0, ndraws));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > count );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  bin.DecayRegulator(salut, 500);
  bin.DecayRegulator(hi, 1);
  REQUIRE( bin.ViewRegulator(salut) == 0.0 );
  REQUIRE( bin.ViewRegulator(hi) == 0.0 );
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > ndraws/2 );
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") < hi_count );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  }


}

TEST_CASE("Test MatchBin", "[tools]")
{

  // test baseline default N (1)
  {
  emp::Random rand(1);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RouletteSelector<>,
    emp::LegacyRegulator
  >bin_rts(rand);

  bin_rts.Put("hi", 0);
  bin_rts.Put("salut", 0);
  REQUIRE(bin_rts.Match(0).size() == 1);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::ExpRouletteSelector<>,
    emp::LegacyRegulator
  >bin_xrs(rand);

  bin_xrs.Put("hi", 0);
  bin_xrs.Put("salut", 0);
  REQUIRE(bin_xrs.Match(0).size() == 1);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RankedSelector<>,
    emp::LegacyRegulator
  >bin_rks(rand);

  bin_rks.Put("hi", 0);
  bin_rks.Put("salut", 0);
  REQUIRE(bin_rks.Match(0).size() == 1);

  }

  // test setting different default N
  {
  emp::Random rand(1);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RouletteSelector<
      std::ratio<-1,1>,
      std::ratio<1000,1>,
      std::ratio<1, 1>,
      2
    >,
    emp::LegacyRegulator
  >bin_rts(rand);

  bin_rts.Put("hi", 0);
  bin_rts.Put("salut", 0);
  REQUIRE(bin_rts.Match(0).size() == 2);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::ExpRouletteSelector<
      std::ratio<13, 10>,
      std::ratio<1, 100>,
      std::ratio<4, 1>,
      std::ratio<4, 1>,
      std::ratio<5, 4>,
      2
    >,
    emp::LegacyRegulator
  >bin_xrs(rand);

  bin_xrs.Put("hi", 0);
  bin_xrs.Put("salut", 0);
  REQUIRE(bin_xrs.Match(0).size() == 2);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RankedSelector<std::ratio<-1,1>, 2>,
    emp::LegacyRegulator
  >bin_rks(rand);

  bin_rks.Put("hi", 0);
  bin_rks.Put("salut", 0);
  REQUIRE(bin_rks.Match(0).size() == 2);

  }

  {
    emp::Random rand(1);
  // We care about numbers less than 6 (<=5.99) away from what we're matching.
  // The AbsDiffMetric will normalize our result to distance / Max_Int.
  // We multiply both sides by 100 to get rid of floats for std::ratio
  // Add Max_INT * 100 in the numerator to offset the regulator function.
  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RankedSelector<std::ratio<214748364700+599,214748364700>, 2>,
    emp::LegacyRegulator
  > bin(rand);

  const size_t hi = bin.Put("hi", 1);
  REQUIRE( bin.GetVal(hi) == "hi" );
  const size_t salut = bin.Put("salut", 0);
  REQUIRE( bin.GetVal(salut) == "salut" );

  REQUIRE( bin.GetVal(bin.Put("bonjour", 6)) == "bonjour" );
  REQUIRE( bin.GetVal(bin.Put("yo", -4)) == "yo" );
  REQUIRE( bin.GetVal(bin.Put("konichiwa", -6)) == "konichiwa" );

  REQUIRE( bin.Size() == 5 );

  // 0 = use Selector default n of 2
  REQUIRE(
    bin.GetVals(bin.Match(0, 0)) == (emp::vector<std::string>{"salut", "hi"})
  );
  REQUIRE( bin.GetTags(bin.Match(0, 0)) == (emp::vector<int>{0, 1}) );

  REQUIRE( bin.GetVals(bin.Match(0, 1)) == emp::vector<std::string>{"salut"} );
  REQUIRE( bin.GetTags(bin.Match(0, 1)) == emp::vector<int>{0} );

  REQUIRE(
    bin.GetVals(bin.Match(0, 2)) == (emp::vector<std::string>{"salut", "hi"})
  );
  REQUIRE( bin.GetTags(bin.Match(0, 2)) == (emp::vector<int>{0, 1}) );

  REQUIRE(
    bin.GetVals(bin.Match(0, 3)) == (emp::vector<std::string>{"salut", "hi", "yo"})
  );
  REQUIRE( bin.GetTags(bin.Match(0, 3)) == (emp::vector<int>{0, 1, -4}) );

  REQUIRE(
    bin.GetVals(bin.Match(0, 4)) == (emp::vector<std::string>{"salut", "hi", "yo"})
  );
  REQUIRE( bin.GetTags(bin.Match(0, 4)) == (emp::vector<int>{0, 1, -4}) );

  REQUIRE( bin.GetVals(bin.Match(15, 8)) == emp::vector<std::string>{} );
  REQUIRE( bin.GetTags(bin.Match(15, 8)) == (emp::vector<int>{}) );

  REQUIRE( bin.GetVals(bin.Match(10, 2)) == emp::vector<std::string>{"bonjour"} );
  REQUIRE( bin.GetTags(bin.Match(10, 2)) == (emp::vector<int>{6}) );

  bin.SetRegulator(hi, 0.1);
  REQUIRE( bin.GetVals(bin.Match(0, 1)) == emp::vector<std::string>{"hi"} );
  REQUIRE( bin.GetTags(bin.Match(0, 1)) == emp::vector<int>{1} );
  REQUIRE(
    bin.GetVals(bin.Match(0, 2)) == (emp::vector<std::string>{"hi", "salut"})
  );
  REQUIRE( bin.GetTags(bin.Match(0, 2)) == (emp::vector<int>{1, 0}) );

  bin.Delete(hi);
  REQUIRE( bin.GetVals(bin.Match(0, 1)) == emp::vector<std::string>{"salut"} );
  REQUIRE( bin.GetTags(bin.Match(0, 1)) == emp::vector<int>{0} );
  REQUIRE(
    bin.GetVals(bin.Match(0, 2)) == (emp::vector<std::string>{"salut", "yo"})
  );
  REQUIRE( bin.GetTags(bin.Match(0, 2)) == (emp::vector<int>{0, -4}) );

  bin.Put("hi", 1);
  // 0 = use Selector default of 2
  REQUIRE(
    bin.GetVals(bin.Match(0, 0)) == (emp::vector<std::string>{"salut", "hi"})
  );
  REQUIRE( bin.GetVals(bin.Match(0, 1)) == emp::vector<std::string>{"salut"} );
  REQUIRE(
    bin.GetVals(bin.Match(0, 2)) == (emp::vector<std::string>{"salut", "hi"})
  );
  REQUIRE(
    bin.GetVals(bin.Match(0, 3)) == (emp::vector<std::string>{"salut", "hi", "yo"})
  );


  }

  // test infinite thresh
  {
  emp::Random rand(1);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RankedSelector<>,
    emp::LegacyRegulator
  > bin(rand);

  const size_t hi = bin.Put("hi", 1);
  REQUIRE( bin.GetVal(hi) == "hi" );
  const size_t salut = bin.Put("salut", 0);
  REQUIRE( bin.GetVal(salut) == "salut" );
  const size_t bonjour = bin.Put("bonjour", std::numeric_limits<int>::max());
  REQUIRE(bin.GetVal(bonjour) == "bonjour");

  REQUIRE( bin.Size() == 3 );

  // 0 = use Selector default, which is 1
  REQUIRE( bin.GetVals(bin.Match(0, 0)) == emp::vector<std::string>{"salut"} );
  REQUIRE( bin.GetTags(bin.Match(0, 0)) == emp::vector<int>{0} );

  REQUIRE( bin.GetVals(bin.Match(0, 1)) == emp::vector<std::string>{"salut"} );
  REQUIRE( bin.GetTags(bin.Match(0, 1)) == emp::vector<int>{0} );

  REQUIRE(
    bin.GetVals(bin.Match(0, 2)) == (emp::vector<std::string>{"salut", "hi"})
  );
  REQUIRE( bin.GetTags(bin.Match(0, 2)) == (emp::vector<int>{0, 1}) );

  REQUIRE(
    bin.GetVals(bin.Match(0, 3)) == (emp::vector<std::string>{"salut", "hi", "bonjour"})
  );
  REQUIRE(
    bin.GetVals(bin.Match(0, 4)) == (emp::vector<std::string>{"salut", "hi", "bonjour"})
  );

  bin.SetRegulator(bonjour, std::numeric_limits<double>::infinity());
  // 0 = use Selector default, which is 1
  REQUIRE( bin.GetVals(bin.Match(0, 0)) == emp::vector<std::string>{"salut"} );
  REQUIRE( bin.GetTags(bin.Match(0, 0)) == emp::vector<int>{0} );

  REQUIRE( bin.GetVals(bin.Match(0, 1)) == emp::vector<std::string>{"salut"} );
  REQUIRE( bin.GetTags(bin.Match(0, 1)) == emp::vector<int>{0} );

  REQUIRE(
    bin.GetVals(bin.Match(0, 2)) == (emp::vector<std::string>{"salut", "hi"})
  );
  REQUIRE( bin.GetTags(bin.Match(0, 2)) == (emp::vector<int>{0, 1}) );

  REQUIRE(
    bin.GetVals(bin.Match(0, 3)) == (emp::vector<std::string>{"salut", "hi", "bonjour"})
  );
  REQUIRE(
    bin.GetVals(bin.Match(0, 4)) == (emp::vector<std::string>{"salut", "hi", "bonjour"})
  );

  }

  {

  emp::Random rand(1);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RouletteSelector<>,
    emp::LegacyRegulator
  >bin(rand);

  const size_t hi = bin.Put("hi", std::numeric_limits<int>::max()-1);
  REQUIRE( bin.GetVal(hi) == "hi" );
  const size_t salut = bin.Put("salut", 0);
  REQUIRE( bin.GetVal(salut) == "salut" );

  REQUIRE( bin.Size() == 2 );

  auto res = bin.GetVals(bin.Match(0, 100000));
  const size_t count = std::count(std::begin(res), std::end(res), "salut");
  REQUIRE( count > 50000);
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  bin.AdjRegulator(salut, 10.0);
  bin.SetRegulator(hi, 0.5);
  res = bin.GetVals(bin.Match(0, 100000));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > 0 );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 50000 );

  bin.SetRegulator(salut, 0.5);
  bin.SetRegulator(hi, 2.0);
  res = bin.GetVals(bin.Match(0, 100000));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > count );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );
  }


  //test roulette selector with threshold
  {

  emp::Random rand(1);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RouletteSelector<
      std::ratio<(size_t)std::numeric_limits<int>::max()+1000000, std::numeric_limits<int>::max()>
    >,
    emp::LegacyRegulator
  >bin(rand);

  const size_t hi = bin.Put("hi", 1000000);
  REQUIRE( bin.GetVal(hi) == "hi" );
  const size_t yo = bin.Put("yo", std::numeric_limits<int>::max());
  REQUIRE( bin.GetVal(yo) == "yo" );
  const size_t salut = bin.Put("salut", 0);
  REQUIRE( bin.GetVal(salut) == "salut" );

  REQUIRE( bin.Size() == 3 );

  auto res = bin.GetVals(bin.Match(0, 100000));
  const size_t count = std::count(std::begin(res), std::end(res), "salut");
  REQUIRE( count > 50000);
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );
  REQUIRE( std::count(std::begin(res), std::end(res), "yo") == 0 );

  bin.SetRegulator(hi, 0.5);
  res = bin.GetVals(bin.Match(0, 100000));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > 0 );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 50000 );
  REQUIRE( std::count(std::begin(res), std::end(res), "yo") == 0 );

  bin.SetRegulator(salut, 0.5);
  bin.SetRegulator(hi, 0.9);
  res = bin.GetVals(bin.Match(0, 100000));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > count );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );
  REQUIRE( std::count(std::begin(res), std::end(res), "yo") == 0 );

  bin.SetRegulator(yo, 0.0);
  res = bin.GetVals(bin.Match(0, 100000));
  REQUIRE( std::count(std::begin(res), std::end(res), "yo") > 0 );

  bin.SetRegulator(salut, 20.0);
  bin.SetRegulator(hi, 20.0);
  res = bin.GetVals(bin.Match(0, 100000));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") == 0 );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") == 0 );

  }

  // test RouletteSelector skew
  {

  emp::Random rand(1);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RouletteSelector<std::ratio<-1,1>,std::ratio<1000,1>>,
    emp::LegacyRegulator
  >bin_softskew(rand);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RouletteSelector<std::ratio<-1,1>,std::ratio<1,1000>>,
    emp::LegacyRegulator
  >bin_hardskew(rand);

  const size_t hi1 = bin_softskew.Put("hi", 100000000);
  REQUIRE( bin_softskew.GetVal(hi1) == "hi" );
  const size_t salut1 = bin_softskew.Put("salut", 0);
  REQUIRE( bin_softskew.GetVal(salut1) == "salut" );

  REQUIRE( bin_softskew.Size() == 2 );

  const size_t hi2 = bin_hardskew.Put("hi", 100000000);
  REQUIRE( bin_hardskew.GetVal(hi2) == "hi" );
  const size_t salut2 = bin_hardskew.Put("salut", 0);
  REQUIRE( bin_hardskew.GetVal(salut2) == "salut" );

  REQUIRE( bin_hardskew.Size() == 2 );


  auto res_softskew = bin_softskew.GetVals(bin_softskew.Match(0, 100000));
  const size_t count_soft = std::count(
    std::begin(res_softskew), std::end(res_softskew), "salut"
  );
  REQUIRE(count_soft > 45000);
  REQUIRE(
    std::count(std::begin(res_softskew), std::end(res_softskew), "hi") > 40000
  );

  auto res_hardskew = bin_hardskew.GetVals(bin_hardskew.Match(0, 100000));
  const size_t count_hard = std::count(
    std::begin(res_hardskew), std::end(res_hardskew), "salut"
  );
  REQUIRE( count_hard > 90000);
  REQUIRE( count_hard > count_soft);

  bin_softskew.AdjRegulator(salut1, 4.0);
  bin_softskew.SetRegulator(hi1, 0.5);
  res_softskew = bin_softskew.GetVals(bin_softskew.Match(0, 100000));
  REQUIRE(
    std::count(std::begin(res_softskew), std::end(res_softskew), "salut") > 45000
  );
  REQUIRE(
    std::count(std::begin(res_softskew), std::end(res_softskew), "hi") > 45000
  );

  bin_hardskew.AdjRegulator(salut2, 4.0);
  bin_hardskew.SetRegulator(hi2, 0.5);
  res_hardskew = bin_hardskew.GetVals(bin_hardskew.Match(0, 100000));
  REQUIRE(
    std::count(std::begin(res_hardskew), std::end(res_hardskew), "hi") > 90000
  );

  }

  // test RouletteSelector MaxBaselineRatio
  {

  emp::Random rand(1);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RouletteSelector<
      std::ratio<-1,1>,
      std::ratio<1,10>,
      std::ratio<0,1>
    >,
    emp::LegacyRegulator
  >bin_lobase(rand);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RouletteSelector<
      std::ratio<-1,1>,
      std::ratio<1,10>,
      std::ratio<-1,1>
    >,
    emp::LegacyRegulator
  >bin_hibase(rand);

  const size_t hi1 = bin_lobase.Put("hi", std::numeric_limits<int>::max());
  REQUIRE( bin_lobase.GetVal(hi1) == "hi" );
  const size_t salut1 = bin_lobase.Put("salut", 0);
  REQUIRE( bin_lobase.GetVal(salut1) == "salut" );

  REQUIRE( bin_lobase.Size() == 2 );

  const size_t hi2 = bin_hibase.Put("hi", std::numeric_limits<int>::max());
  REQUIRE( bin_hibase.GetVal(hi2) == "hi" );
  const size_t salut2 = bin_hibase.Put("salut", 0);
  REQUIRE( bin_hibase.GetVal(salut2) == "salut" );

  REQUIRE( bin_hibase.Size() == 2 );

  auto res_lobase = bin_lobase.GetVals(bin_lobase.Match(0, 100000));
  const size_t count_lobase = std::count(
    std::begin(res_lobase), std::end(res_lobase), "salut"
  );
  REQUIRE(count_lobase < 70000);
  REQUIRE(
    std::count(std::begin(res_lobase), std::end(res_lobase), "hi") > 30000
  );

  auto res_hibase = bin_hibase.GetVals(bin_hibase.Match(0, 100000));
  const size_t count_hibase = std::count(
    std::begin(res_hibase), std::end(res_hibase), "salut"
  );
  REQUIRE( count_hibase > 90000);
  REQUIRE( count_hibase > count_lobase);

  bin_lobase.AdjRegulator(salut1, 9.0);
  bin_lobase.SetRegulator(hi1, 5.0);
  res_lobase = bin_lobase.GetVals(bin_lobase.Match(0, 100000));
  REQUIRE(
    std::count(std::begin(res_lobase), std::end(res_lobase), "salut") > 45000
  );
  REQUIRE(
    std::count(std::begin(res_lobase), std::end(res_lobase), "hi") > 45000
  );

  bin_hibase.AdjRegulator(salut2, 4.0);
  bin_hibase.SetRegulator(hi2, 2.0);
  res_hibase = bin_hibase.GetVals(bin_hibase.Match(0, 100000));
  REQUIRE(
    std::count(std::begin(res_hibase), std::end(res_hibase), "hi") > 90000
  );

  bin_lobase.AdjRegulator(hi1, -5.0);
  bin_lobase.SetRegulator(salut1, 1.2);
  res_lobase = bin_lobase.GetVals(bin_lobase.Match(0, 100000));
  REQUIRE(
    std::count(std::begin(res_lobase), std::end(res_lobase), "hi") > 90000
  );

  }

  {
  emp::Random rand(1);
  emp::MatchBin<
    std::string,
    emp::HammingMetric<32>,
    emp::RankedSelector<std::ratio<32 + 3, 32>>,
    emp::LegacyRegulator
  > bitBin(rand);

  emp::BitSet<32> bs3;
  bs3.SetUInt(0,3); // 0000 0011

  const size_t three = bitBin.Put("three", bs3);
  REQUIRE( bitBin.GetVal(three) == "three");

  emp::BitSet<32> bs1;
  bs1.SetUInt(0,1); // 0000 0001

  const size_t one = bitBin.Put("one", bs1);
  REQUIRE( bitBin.GetVal(one) == "one");

  emp::BitSet<32> bs11;
  bs11.SetUInt(0,11); //0000 1011

  REQUIRE(bitBin.GetVal(bitBin.Put("eleven", bs11)) == "eleven");

  emp::BitSet<32> bs0;//0000 0000

  // rely on MatchBin default, which is 1
  REQUIRE(bitBin.GetVals(bitBin.Match(bs0, 0)) == emp::vector<std::string>{"one"});
  REQUIRE(bitBin.GetTags(bitBin.Match(bs0, 0)) == emp::vector<emp::BitSet<32>>{bs1});

  REQUIRE(bitBin.GetVals(bitBin.Match(bs0, 1)) == emp::vector<std::string>{"one"});
  REQUIRE(bitBin.GetTags(bitBin.Match(bs0, 1)) == emp::vector<emp::BitSet<32>>{bs1});

  REQUIRE(bitBin.GetVals(bitBin.Match(bs11, 2)) == (emp::vector<std::string>{"eleven", "three"}));
  REQUIRE(bitBin.GetTags(bitBin.Match(bs11, 2)) == (emp::vector<emp::BitSet<32>>{bs11, bs3}));

  REQUIRE(bitBin.GetVals(bitBin.Match(bs3, 5)) == (emp::vector<std::string> {"three","one","eleven"}));
  REQUIRE(bitBin.GetTags(bitBin.Match(bs3, 5)) == (emp::vector<emp::BitSet<32>> {bs3, bs1, bs11}));

  REQUIRE (bitBin.Size() == 3);

  bitBin.SetRegulator(one, .1); //1 0 1 2 --> .2 .1 .2 .3

  REQUIRE(bitBin.GetVals(bitBin.Match(bs3, 5)) == (emp::vector<std::string> {"one","three","eleven"}));
  REQUIRE(bitBin.GetTags(bitBin.Match(bs3, 5)) == (emp::vector<emp::BitSet<32>> {bs1, bs3, bs11}));

  bitBin.SetRegulator(one, 1);
  bitBin.SetRegulator(three, 4); // 2 1 0 1 --> 12 8 4 8

  REQUIRE(bitBin.GetVals(bitBin.Match(bs3, 5)) == (emp::vector<std::string> {"eleven", "one"}));
  REQUIRE(bitBin.GetTags(bitBin.Match(bs3, 5)) == (emp::vector<emp::BitSet<32>> {bs11, bs1}));

  }

  {

  emp::Random rand(1);

  emp::MatchBin<
    std::string,
    emp::HammingMetric<32>,
    emp::RouletteSelector<>,
    emp::LegacyRegulator
  >bitBin(rand);

  emp::BitSet<32> bs2;
  bs2.SetUInt(0, 2);//0000 0010
  const size_t elementary = bitBin.Put("elementary", bs2);
  REQUIRE(bitBin.GetVal(elementary) == "elementary");

  emp::BitSet<32> bs6;
  bs6.SetUInt(0,6);//0000 0110
  const size_t fedora = bitBin.Put("fedora", bs6);
  REQUIRE	(bitBin.GetVal(fedora) == "fedora");

  REQUIRE(bitBin.Size() == 2);

  auto res = bitBin.GetVals(bitBin.Match(bs2, 100000));
  const size_t count = std::count(std::begin(res), std::end(res), "elementary");
  REQUIRE(count > 50000);
  REQUIRE (std::count(std::begin(res), std::end(res), "fedora") > 0);

  bitBin.AdjRegulator(elementary, 10);
  bitBin.SetRegulator(fedora, 0.5);
  res = bitBin.GetVals(bitBin.Match(bs2, 100000));

  REQUIRE(std::count(std::begin(res), std::end(res), "elementary") > 0);
  REQUIRE(std::count(std::begin(res), std::end(res), "fedora") > 50000);

  bitBin.SetRegulator(elementary, 0.5);
  bitBin.SetRegulator(fedora, 2.0);
  res = bitBin.GetVals(bitBin.Match(bs2, 100000));

  REQUIRE( std::count(std::begin(res), std::end(res), "elementary") > count);
  REQUIRE( std::count(std::begin(res), std::end(res), "fedora") >  0);

  }

  {
  emp::Random rand(1);
  const int max_value = 1000;
  emp::MatchBin<
    std::string,
    emp::NextUpMetric<1000>,
    emp::RankedSelector<std::ratio<max_value + max_value,max_value>>,
    emp::LegacyRegulator
  > bin(rand);

  const size_t hi = bin.Put("hi", 1);
  REQUIRE( bin.GetVal(hi) == "hi" );
  const size_t salut = bin.Put("salut", 0);
  REQUIRE( bin.GetVal(salut) == "salut" );

  REQUIRE( bin.GetVal(bin.Put("bonjour", 6)) == "bonjour" );
  REQUIRE( bin.GetVal(bin.Put("yo", 10)) == "yo" );
  REQUIRE( bin.GetVal(bin.Put("konichiwa", max_value)) == "konichiwa" );

  REQUIRE( bin.Size() == 5 );

  // 0 = use Selector default, which is 1
  REQUIRE( bin.GetVals(bin.Match(0, 1)) == emp::vector<std::string>{"salut"} );
  REQUIRE( bin.GetTags(bin.Match(0, 1)) == emp::vector<size_t>{0} );

  REQUIRE( bin.GetVals(bin.Match(0, 1)) == emp::vector<std::string>{"salut"} );
  REQUIRE( bin.GetTags(bin.Match(0, 1)) == emp::vector<size_t>{0} );

  REQUIRE(
    bin.GetVals(bin.Match(0, 2)) == (emp::vector<std::string>{"salut", "hi"})
  );
  REQUIRE( bin.GetTags(bin.Match(0, 2)) == (emp::vector<size_t>{0, 1}) );

  REQUIRE(
    bin.GetVals(bin.Match(7, 3)) == (emp::vector<std::string>{"yo", "konichiwa", "salut"})
  );
  REQUIRE( bin.GetTags(bin.Match(7, 3)) == (emp::vector<size_t>{10, max_value, 0}) );

  REQUIRE(
    bin.GetVals(bin.Match(0, 4)) == (emp::vector<std::string>{"salut", "hi", "bonjour", "yo"})
  );
  REQUIRE( bin.GetTags(bin.Match(0, 4)) == (emp::vector<size_t>{0, 1, 6, 10}) );

  REQUIRE( bin.GetVals(bin.Match(15, 8)) == (emp::vector<std::string>{"konichiwa", "salut", "hi", "bonjour", "yo"}) );
  REQUIRE( bin.GetTags(bin.Match(15, 8)) == (emp::vector<size_t>{max_value, 0, 1, 6, 10}) );

  REQUIRE( bin.GetVals(bin.Match(10, 2)) == (emp::vector<std::string>{"yo", "konichiwa"}) );
  REQUIRE( bin.GetTags(bin.Match(10, 2)) == (emp::vector<size_t>{10, max_value}) );

  bin.SetRegulator(hi, 1.004);
  REQUIRE( bin.GetVals(bin.Match(1, 1)) == emp::vector<std::string>{"hi"} );
  REQUIRE( bin.GetTags(bin.Match(1, 1)) == emp::vector<size_t>{1} );

  bin.SetRegulator(hi, 2.0);
  REQUIRE(
    bin.GetVals(bin.Match(1, 1)) == (emp::vector<std::string>{"bonjour"})
  );
  REQUIRE( bin.GetTags(bin.Match(1, 1)) == (emp::vector<size_t>{6}) );

  }

  {

  emp::Random rand(1);

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RouletteSelector<>,
    emp::LegacyRegulator
  >bin(rand);

  const size_t hi = bin.Put("hi", 100000000);
  REQUIRE( bin.GetVal(hi) == "hi" );
  const size_t salut = bin.Put("salut", 0);
  REQUIRE( bin.GetVal(salut) == "salut" );

  REQUIRE( bin.Size() == 2 );

  auto res = bin.GetVals(bin.Match(0, 100000));
  const size_t count = std::count(std::begin(res), std::end(res), "salut");
  REQUIRE( count > 50000);
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  bin.AdjRegulator(salut, 10.0);
  bin.SetRegulator(hi, 0.5);
  res = bin.GetVals(bin.Match(0, 100000));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > 0 );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 50000 );

  bin.SetRegulator(salut, 0.5);
  bin.SetRegulator(hi, 2.0);
  res = bin.GetVals(bin.Match(0, 100000));
  REQUIRE( std::count(std::begin(res), std::end(res), "salut") > count );
  REQUIRE( std::count(std::begin(res), std::end(res), "hi") > 0 );

  }
  {
  emp::Random rand(1);
  emp::MatchBin<
    std::string,
    emp::StreakMetric<8>,
    emp::RankedSelector<std::ratio<1+1, 1>>,
    emp::LegacyRegulator
  > bitBin(rand);

  emp::BitSet<8> bs1;
  bs1.SetUInt(0,1); // 0000 0001

  const size_t one = bitBin.Put("one", bs1);
  REQUIRE( bitBin.GetVal(one) == "one");

  emp::BitSet<8> bs128;
  bs128.SetUInt(0,128); // 1000 000

  const size_t oneTwoEight = bitBin.Put("one-two-eight", bs128);
  REQUIRE( bitBin.GetVal(oneTwoEight) == "one-two-eight");

  emp::BitSet<8> bs127;
  bs127.SetUInt(0,127); //0111 1111

  REQUIRE(bitBin.GetVal(bitBin.Put("one-two-seven", bs127)) == "one-two-seven");

  emp::BitSet<8> bs15;
  bs15.SetUInt(0,15); //0000 1111

  REQUIRE(bitBin.GetVal(bitBin.Put("fifteen", bs15)) == "fifteen");

  emp::BitSet<8> bs2;//0000 0010
  bs2.SetUInt(0,2);

  // 0 = use Selector default, which is 1
  REQUIRE(bitBin.GetVals(bitBin.Match(bs2, 1)) == emp::vector<std::string>{"one"});
  REQUIRE(bitBin.GetTags(bitBin.Match(bs2, 1)) == emp::vector<emp::BitSet<8>>{bs1});

  REQUIRE(bitBin.GetVals(bitBin.Match(bs2, 1)) == emp::vector<std::string>{"one"});
  REQUIRE(bitBin.GetTags(bitBin.Match(bs2, 1)) == emp::vector<emp::BitSet<8>>{bs1});

  REQUIRE(bitBin.GetVals(bitBin.Match(bs128, 2)) == (emp::vector<std::string>{"one-two-eight", "one"}));
  REQUIRE(bitBin.GetTags(bitBin.Match(bs128, 2)) == (emp::vector<emp::BitSet<8>>{bs128, bs1}));

  REQUIRE(bitBin.GetVals(bitBin.Match(bs127, 5)) == (emp::vector<std::string> {"one-two-seven", "fifteen","one","one-two-eight"}));
  REQUIRE(bitBin.GetTags(bitBin.Match(bs127, 5)) == (emp::vector<emp::BitSet<8>> {bs127, bs15, bs1, bs128}));

  REQUIRE (bitBin.Size() == 4);

  bitBin.SetRegulator(one, .1); //1 0 1 2 --> .2 .1 .2 .3

  REQUIRE(bitBin.GetVals(bitBin.Match(bs128, 2)) == (emp::vector<std::string> {"one","one-two-eight"}));
  REQUIRE(bitBin.GetTags(bitBin.Match(bs128, 2)) == (emp::vector<emp::BitSet<8>> {bs1,bs128}));

  }

  {
  emp::Random rand(1);
  emp::MatchBin<
    std::string,
    emp::StreakMetric<64>,
    emp::RankedSelector<std::ratio<1+1, 1>>,
    emp::LegacyRegulator
  > bitBin64(rand);

  emp::BitSet<64> bs7;
  bs7.SetUInt(
    1,
    emp::IntPow(2UL, 18UL) + emp::IntPow(2UL, 19UL) + emp::IntPow(2UL, 20UL)
  );

  const size_t id_seven = bitBin64.Put("seven", bs7);
  REQUIRE( bitBin64.GetVal(id_seven) == "seven");

  emp::BitSet<64> bs1;
  bs1.SetUInt(
    1,
    emp::IntPow(2UL, 16UL) + emp::IntPow(2UL, 17UL) + emp::IntPow(2UL, 18UL)
  );

  const size_t id_one  = bitBin64.Put("one", bs1);
  REQUIRE( bitBin64.GetVal(id_one) == "one");

  emp::BitSet<64> bs9;
  bs9.SetUInt(
    1,
    emp::IntPow(2UL, 15UL) + emp::IntPow(2UL, 16UL) + emp::IntPow(2UL, 17UL)
  );

  const size_t id_nine  = bitBin64.Put("nine", bs9);
  REQUIRE( bitBin64.GetVal(id_nine) == "nine");

  REQUIRE (bitBin64.Size() == 3);

  // 0 = use Selector default, which is 1
  REQUIRE(bitBin64.GetVals(bitBin64.Match(bs9, 0)) == (emp::vector<std::string> {"nine"}));
  REQUIRE(bitBin64.GetTags(bitBin64.Match(bs9, 0)) == (emp::vector<emp::BitSet<64>> {bs9}));

  REQUIRE(bitBin64.GetVals(bitBin64.Match(bs9, 5)) == (emp::vector<std::string> {"nine","one","seven"}));
  REQUIRE(bitBin64.GetTags(bitBin64.Match(bs9, 5)) == (emp::vector<emp::BitSet<64>> {bs9, bs1, bs7}));

  }

  {
  emp::Random rand(1);
  emp::MatchBin<
    std::string,
    emp::SymmetricNoWrapMetric<8>,
    emp::RankedSelector<std::ratio<256 + 40, 256>>,
    emp::LegacyRegulator
  > bitBin(rand);

  emp::BitSet<8> bs1;
  bs1.SetUInt(0,1); // 0000 0001

  const size_t one = bitBin.Put("one", bs1);

  emp::BitSet<8> bs128;
  bs128.SetUInt(0,128); // 1000 000

  bitBin.Put("one-two-eight", bs128);

  emp::BitSet<8> bs127;
  bs127.SetUInt(0,127); //0111 1111

  bitBin.Put("one-two-seven", bs127);

  emp::BitSet<8> bs15;
  bs15.SetUInt(0,15); //0000 1111

  bitBin.Put("fifteen", bs15);

  emp::BitSet<8> bs2;//0000 0010
  bs2.SetUInt(0,2);

  // 0 = use Selector default, which is 1
  REQUIRE(bitBin.GetVals(bitBin.Match(bs2, 0)) == emp::vector<std::string>{"one"});
  REQUIRE(bitBin.GetTags(bitBin.Match(bs2, 0)) == emp::vector<emp::BitSet<8>>{bs1});

  REQUIRE(bitBin.GetVals(bitBin.Match(bs2, 1)) == emp::vector<std::string>{"one"});
  REQUIRE(bitBin.GetTags(bitBin.Match(bs2, 1)) == emp::vector<emp::BitSet<8>>{bs1});

  REQUIRE(bitBin.GetVals(bitBin.Match(bs128, 2)) == (emp::vector<std::string>{"one-two-eight", "one-two-seven"}));
  REQUIRE(bitBin.GetTags(bitBin.Match(bs128, 2)) == (emp::vector<emp::BitSet<8>>{bs128, bs127}));

  REQUIRE(bitBin.GetVals(bitBin.Match(bs127, 5)) == (emp::vector<std::string> {"one-two-seven", "one-two-eight"}));
  REQUIRE(bitBin.GetTags(bitBin.Match(bs127, 5)) == (emp::vector<emp::BitSet<8>> {bs127, bs128}));

  REQUIRE (bitBin.Size() == 4);

  bitBin.SetRegulator(one, .001);

  REQUIRE(bitBin.GetVals(bitBin.Match(bs128, 2)) == (emp::vector<std::string> {"one","one-two-eight"}));
  REQUIRE(bitBin.GetTags(bitBin.Match(bs128, 2)) == (emp::vector<emp::BitSet<8>> {bs1,bs128}));

  bitBin.SetRegulator(one, 1.05);

  REQUIRE(bitBin.GetVals(bitBin.Match(bs2, 5)) == (emp::vector<std::string> {"fifteen", "one"}));
  REQUIRE(bitBin.GetTags(bitBin.Match(bs2, 5)) == (emp::vector<emp::BitSet<8>> {bs15, bs1}));

  }

// test HashMetric

  {

  const size_t N_SAMPLES = 1000000;
  const size_t N_BINS = 10;

  emp::HashMetric<32> metric;

  emp::BitSet<32> query;
  emp::BitSet<32> tag;

  emp::DataNode<double, emp::data::Range, emp::data::Histogram> scores;
  scores.SetupBins(0.0, 1.0, 10);

  emp::Random rand(2);

  // test with sequential numbers
  for(size_t i = 0; i < N_SAMPLES; ++i) {
    query.SetUInt(0, i);
    tag.SetUInt(0, i);
    scores.Add(
      metric(query, tag)
    );
  }

  REQUIRE(scores.GetMean() - 0.5 < 0.01);
  REQUIRE(scores.GetMin() < 0.01);
  REQUIRE(scores.GetMax() > 0.01);
  for (auto & c : scores.GetHistCounts()) {
    REQUIRE(c > N_SAMPLES / N_BINS - 20000);
    REQUIRE(c < N_SAMPLES / N_BINS + 20000);
  }

  // test with sequential numbers
  scores.Reset();
  for(size_t i = 0; i < N_SAMPLES; ++i) {
    query.SetUInt(0, 0);
    tag.SetUInt(0, i);
    scores.Add(
      metric(query, tag)
    );
  }

  REQUIRE(scores.GetMean() - 0.5 < 0.01);
  REQUIRE(scores.GetMin() < 0.01);
  REQUIRE(scores.GetMax() > 0.01);
  for (auto & c : scores.GetHistCounts()) {
    REQUIRE(c > N_SAMPLES / N_BINS - 20000);
    REQUIRE(c < N_SAMPLES / N_BINS + 20000);
  }

  // test with random numbers
  scores.Reset();
  for(size_t i = 0; i < N_SAMPLES; ++i) {
    query.Randomize(rand);
    tag.Randomize(rand);
    scores.Add(
      metric(query, tag)
    );
  }

  REQUIRE(scores.GetMean() - 0.5 < 0.01);
  REQUIRE(scores.GetMin() < 0.01);
  REQUIRE(scores.GetMax() > 0.01);
  for (auto & c : scores.GetHistCounts()) {
    REQUIRE(c > N_SAMPLES / N_BINS - 20000);
    REQUIRE(c < N_SAMPLES / N_BINS + 20000);
  }

  // test with weighted random numbers
  scores.Reset();
  for(size_t i = 0; i < N_SAMPLES; ++i) {
    query.Randomize(rand, 0.75);
    tag.Randomize(rand, 0.75);
    scores.Add(
      metric(query, tag)
    );
  }

  REQUIRE(scores.GetMean() - 0.5 < 0.01);
  REQUIRE(scores.GetMin() < 0.01);
  REQUIRE(scores.GetMax() > 0.01);
  for (auto & c : scores.GetHistCounts()) {
    REQUIRE(c > N_SAMPLES / N_BINS - 20000);
    REQUIRE(c < N_SAMPLES / N_BINS + 20000);
  }

  }


  // test SlideMod
  {
  emp::BitSet<3> bs_000{0,0,0};
  emp::BitSet<3> bs_100{1,0,0};
  emp::BitSet<3> bs_010{0,1,0};
  emp::BitSet<3> bs_001{0,0,1};
  emp::BitSet<3> bs_111{1,1,1};

  emp::SlideMod<emp::StreakMetric<3>> slide_streak;
  emp::SlideMod<emp::HammingMetric<3>> slide_hamming;
  emp::StreakMetric<3> streak;

  REQUIRE(
    slide_streak(bs_100, bs_100)
    ==
    slide_streak(bs_100, bs_010)
  );

  REQUIRE(
    streak(bs_100, bs_100)
    ==
    slide_streak(bs_100, bs_010)
  );

  REQUIRE(
    slide_streak(bs_000, bs_111)
    >
    slide_streak(bs_100, bs_000)
  );

  REQUIRE(
    slide_streak(bs_000, bs_111)
    >
    streak(bs_100, bs_000)
  );

  REQUIRE(
    slide_hamming(bs_100, bs_100)
    ==
    slide_hamming(bs_100, bs_010)
  );
  }

  // test AntiMod
  {
  emp::BitSet<3> bs_000{0,0,0};
  emp::BitSet<3> bs_100{1,0,0};
  emp::BitSet<3> bs_010{0,1,0};
  emp::BitSet<3> bs_001{0,0,1};
  emp::BitSet<3> bs_011{0,1,1};
  emp::BitSet<3> bs_111{1,1,1};

  emp::AntiMod<emp::StreakMetric<3>> anti_streak;
  emp::StreakMetric<3> streak;
  emp::AntiMod<emp::HammingMetric<3>> anti_hamming;
  emp::HammingMetric<3> hamming;


  // anti should be equivalent to 0 matching with 1 and vice versa
  // instead of 0 matching with 0 and 1 matching with 1
  REQUIRE(
    anti_streak(bs_000, bs_111)
    -
    streak(bs_111, bs_111)
    <= std::numeric_limits<double>::epsilon()
    // instead of doing == because of floating imprecision
  );

  REQUIRE(
    anti_streak(bs_011, bs_000)
    -
    streak(bs_011, bs_111)
    <= std::numeric_limits<double>::epsilon()
    // instead of doing == because of floating imprecision
  );

  REQUIRE(
    anti_hamming(bs_000, bs_111)
    -
    hamming(bs_111, bs_111)
    <= std::numeric_limits<double>::epsilon()
    // instead of doing == because of floating imprecision
  );

  REQUIRE(
    anti_hamming(bs_011, bs_000)
    -
    hamming(bs_011, bs_111)
    <= std::numeric_limits<double>::epsilon()
    // instead of doing == because of floating imprecision
  );

  }

  // text AntiMod x SlideMod
  {
  emp::BitSet<3> bs_000{0,0,0};
  emp::BitSet<3> bs_100{1,0,0};
  emp::BitSet<3> bs_010{0,1,0};
  emp::BitSet<3> bs_001{0,0,1};
  emp::BitSet<3> bs_011{0,1,1};
  emp::BitSet<3> bs_111{1,1,1};

  emp::SlideMod<emp::StreakMetric<3>> slide_streak;
  emp::SlideMod<emp::AntiMod<emp::StreakMetric<3>>> slide_anti_streak;
  emp::SlideMod<emp::HammingMetric<3>> slide_hamming;
  emp::SlideMod<emp::AntiMod<emp::HammingMetric<3>>> slide_anti_hamming;

  // anti should be equivalent to 0 matching with 1 and vice versa
  // instead of 0 matching with 0 and 1 matching with 1

  REQUIRE(
    slide_streak(bs_000, bs_111)
    ==
    slide_anti_streak(bs_111, bs_111)
  );

  REQUIRE(
    slide_streak(bs_011, bs_000)
    ==
    slide_anti_streak(bs_011, bs_111)
  );

  REQUIRE(
    slide_hamming(bs_000, bs_111)
    -
    slide_anti_hamming(bs_111, bs_111)
    <= std::numeric_limits<double>::epsilon()
    // instead of doing == because of floating imprecision
  );

  REQUIRE(
    slide_hamming(bs_011, bs_000)
    -
    slide_anti_hamming(bs_011, bs_111)
    <= std::numeric_limits<double>::epsilon()
    // instead of doing == because of floating imprecision
  );
  }

  // test MeanDimMod
  {
  emp::Random rand(1);

  emp::array<emp::BitSet<32>, 1> a1 = {
    emp::BitSet<32>(rand)
  };
  emp::array<emp::BitSet<32>, 1> b1 = {
    emp::BitSet<32>(rand)
  };

  emp::StreakMetric<32> streak;
  emp::AntiMod<emp::StreakMetric<32>> anti_streak;
  emp::HammingMetric<32> hamming;
  emp::SlideMod<emp::HammingMetric<32>> slide_hamming;

  emp::MeanDimMod<emp::StreakMetric<32>, 1> d_streak1;
  REQUIRE(d_streak1.width() == streak.width());

  emp::MeanDimMod<emp::AntiMod<emp::StreakMetric<32>>, 1> d_anti_streak1;
  REQUIRE(d_anti_streak1.width() == anti_streak.width());

  emp::MeanDimMod<emp::HammingMetric<32>, 1> d_hamming1;
  REQUIRE(d_hamming1.width() == hamming.width());

  emp::MeanDimMod<emp::SlideMod<emp::HammingMetric<32>>, 1> d_slide_hamming1;
  REQUIRE(d_slide_hamming1.width() == slide_hamming.width());

  REQUIRE(streak(a1[0], b1[0]) == d_streak1(a1, b1));
  REQUIRE(anti_streak(a1[0], b1[0]) == d_anti_streak1(a1, b1));
  REQUIRE(hamming(a1[0], b1[0]) == d_hamming1(a1, b1));
  REQUIRE(slide_hamming(a1[0], b1[0]) == d_slide_hamming1(a1, b1));

  emp::array<emp::BitSet<32>, 3> a3 = {
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand)
  };
  emp::array<emp::BitSet<32>, 3> b3 = {
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand)
  };

  emp::MeanDimMod<emp::StreakMetric<32>, 3> d_streak3;
  REQUIRE(d_streak3.width() == streak.width() * 3);

  emp::MeanDimMod<emp::AntiMod<emp::StreakMetric<32>>, 3> d_anti_streak3;
  REQUIRE(d_anti_streak3.width() == anti_streak.width() * 3);

  emp::MeanDimMod<emp::HammingMetric<32>, 3> d_hamming3;
  REQUIRE(d_hamming3.width() == hamming.width() * 3);

  emp::MeanDimMod<emp::SlideMod<emp::HammingMetric<32>>, 3> d_slide_hamming3;
  REQUIRE(d_slide_hamming3.width() == slide_hamming.width() * 3);


  REQUIRE(
    streak(a3[0], b3[0]) + streak(a3[1], b3[1]) + streak(a3[2], b3[2])
    -
    d_streak3(a3,b3) * 3.0
    <= std::numeric_limits<double>::epsilon()
  );
  REQUIRE(
    anti_streak(a3[0], b3[0]) + anti_streak(a3[1], b3[1]) + anti_streak(a3[2], b3[2])
    -
    d_anti_streak3(a3,b3)  * 3.0
    <= std::numeric_limits<double>::epsilon()
  );
  REQUIRE(
    hamming(a3[0], b3[0]) + hamming(a3[1], b3[1]) + hamming(a3[2], b3[2])
    -
    d_hamming3(a3,b3)  * 3.0
    <= std::numeric_limits<double>::epsilon()
  );
  REQUIRE(
    slide_hamming(a3[0], b3[0]) + slide_hamming(a3[1], b3[1]) + slide_hamming(a3[2], b3[2])
    -
    d_slide_hamming3(a3,b3)  * 3.0
    <= std::numeric_limits<double>::epsilon()
  );

  }

  // test MinDimMod
  {
  emp::Random rand(1);

  emp::array<emp::BitSet<32>, 1> a1 = {
    emp::BitSet<32>(rand)
  };
  emp::array<emp::BitSet<32>, 1> b1 = {
    emp::BitSet<32>(rand)
  };

  emp::StreakMetric<32> streak;
  emp::AntiMod<emp::StreakMetric<32>> anti_streak;
  emp::HammingMetric<32> hamming;
  emp::SlideMod<emp::HammingMetric<32>> slide_hamming;

  emp::MinDimMod<emp::StreakMetric<32>, 1> d_streak1;
  REQUIRE(d_streak1.width() == streak.width());

  emp::MinDimMod<emp::AntiMod<emp::StreakMetric<32>>, 1> d_anti_streak1;
  REQUIRE(d_anti_streak1.width() == anti_streak.width());

  emp::MinDimMod<emp::HammingMetric<32>, 1> d_hamming1;
  REQUIRE(d_hamming1.width() == hamming.width());

  emp::MinDimMod<emp::SlideMod<emp::HammingMetric<32>>, 1> d_slide_hamming1;
  REQUIRE(d_slide_hamming1.width() == slide_hamming.width());

  REQUIRE(streak(a1[0], b1[0]) == d_streak1(a1, b1));
  REQUIRE(anti_streak(a1[0], b1[0]) == d_anti_streak1(a1, b1));
  REQUIRE(hamming(a1[0], b1[0]) == d_hamming1(a1, b1));
  REQUIRE(slide_hamming(a1[0], b1[0]) == d_slide_hamming1(a1, b1));

  emp::array<emp::BitSet<32>, 3> a3 = {
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand)
  };
  emp::array<emp::BitSet<32>, 3> b3 = {
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand)
  };

  emp::MinDimMod<emp::StreakMetric<32>, 3> d_streak3;
  REQUIRE(d_streak3.width() == streak.width() * 3);

  emp::MinDimMod<emp::AntiMod<emp::StreakMetric<32>>, 3> d_anti_streak3;
  REQUIRE(d_anti_streak3.width() == anti_streak.width() * 3);

  emp::MinDimMod<emp::HammingMetric<32>, 3> d_hamming3;
  REQUIRE(d_hamming3.width() == hamming.width() * 3);

  emp::MinDimMod<emp::SlideMod<emp::HammingMetric<32>>, 3> d_slide_hamming3;
  REQUIRE(d_slide_hamming3.width() == slide_hamming.width() * 3);


  REQUIRE(
    std::min({streak(a3[0], b3[0]), streak(a3[1], b3[1]), streak(a3[2], b3[2])})
    -
    d_streak3(a3,b3)
    <= std::numeric_limits<double>::epsilon()
  );
  REQUIRE(
    std::min({anti_streak(a3[0], b3[0]), anti_streak(a3[1], b3[1]), anti_streak(a3[2], b3[2])})
    -
    d_anti_streak3(a3,b3)
    <= std::numeric_limits<double>::epsilon()
  );
  REQUIRE(
    std::min({hamming(a3[0], b3[0]), hamming(a3[1], b3[1]), hamming(a3[2], b3[2])})
    -
    d_hamming3(a3,b3)
    <= std::numeric_limits<double>::epsilon()
  );
  REQUIRE(
    std::min({slide_hamming(a3[0], b3[0]), slide_hamming(a3[1], b3[1]), slide_hamming(a3[2], b3[2])})
    -
    d_slide_hamming3(a3,b3)
    <= std::numeric_limits<double>::epsilon()
  );

  }

  // test FlatMod x MeanDimMod
  {
  emp::Random rand(1);

  emp::BitSet<32> a1(rand);

  emp::BitSet<32> b1(rand);

  emp::StreakMetric<32> streak;
  emp::AntiMod<emp::StreakMetric<32>> anti_streak;
  emp::HammingMetric<32> hamming;
  emp::SlideMod<emp::HammingMetric<32>> slide_hamming;

  emp::FlatMod<
    emp::MeanDimMod<
      emp::StreakMetric<32>,
      1
    >
  > d_streak1;
  REQUIRE(d_streak1.width() == streak.width());

  emp::FlatMod<
    emp::MeanDimMod<
      emp::AntiMod<
        typename emp::StreakMetric<32>
      >,
      1
    >
  > d_anti_streak1;
  REQUIRE(d_anti_streak1.width() == anti_streak.width());

  emp::FlatMod<
    emp::MeanDimMod<
      typename emp::HammingMetric<32>,
      1
    >
  > d_hamming1;
  REQUIRE(d_hamming1.width() == hamming.width());

  emp::FlatMod<
    emp::MeanDimMod<
      emp::SlideMod<
        typename emp::HammingMetric<32>
      >,
      1
    >
  > d_slide_hamming1;
  REQUIRE(d_slide_hamming1.width() == slide_hamming.width());

  REQUIRE(streak(a1, b1) == d_streak1(a1, b1));
  REQUIRE(anti_streak(a1, b1) == d_anti_streak1(a1, b1));
  REQUIRE(hamming(a1, b1) == d_hamming1(a1, b1));
  REQUIRE(slide_hamming(a1, b1) == d_slide_hamming1(a1, b1));

  emp::array<emp::BitSet<32>, 3> a3 = {
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand)
  };

  emp::array<emp::BitSet<32>, 3> b3 = {
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand)
  };

  emp::BitSet<96> flat_a3;
  emp::BitSet<96> flat_b3;

  for (size_t i = 0; i < 96; ++i) {
    flat_a3[i] = a3[i/32][i%32];
    flat_b3[i] = b3[i/32][i%32];
  }

  emp::FlatMod<
    emp::MeanDimMod<
      emp::StreakMetric<32>,
      3
    >
  > d_streak3;
  REQUIRE(d_streak3.width() == streak.width() * 3);

  emp::FlatMod<
    emp::MeanDimMod<
      emp::AntiMod<
        emp::StreakMetric<32>
      >,
      3
    >
  > d_anti_streak3;
  REQUIRE(d_anti_streak3.width() == anti_streak.width() * 3);

  emp::FlatMod<
    emp::MeanDimMod<
      emp::HammingMetric<32>,
      3
    >
  > d_hamming3;
  REQUIRE(d_hamming3.width() == hamming.width() * 3);

  emp::FlatMod<
    emp::MeanDimMod<
      emp::SlideMod<
        emp::HammingMetric<32>
      >,
      3
    >
  > d_slide_hamming3;
  REQUIRE(d_slide_hamming3.width() == slide_hamming.width() * 3);


  REQUIRE(
    streak(a3[0], b3[0]) + streak(a3[1], b3[1]) + streak(a3[2], b3[2])
    -
    d_streak3(flat_a3,flat_b3) * 3.0
    <= std::numeric_limits<double>::epsilon()
  );
  REQUIRE(
    anti_streak(a3[0], b3[0]) + anti_streak(a3[1], b3[1]) + anti_streak(a3[2], b3[2])
    -
    d_anti_streak3(flat_a3,flat_b3)  * 3.0
    <= std::numeric_limits<double>::epsilon()
  );
  REQUIRE(
    hamming(a3[0], b3[0]) + hamming(a3[1], b3[1]) + hamming(a3[2], b3[2])
    -
    d_hamming3(flat_a3,flat_b3)  * 3.0
    <= std::numeric_limits<double>::epsilon()
  );
  REQUIRE(
    slide_hamming(a3[0], b3[0]) + slide_hamming(a3[1], b3[1]) + slide_hamming(a3[2], b3[2])
    -
    d_slide_hamming3(flat_a3,flat_b3)  * 3.0
    <= std::numeric_limits<double>::epsilon()
  );

  }

  // test FlatMod x MinDimMod
  {
  emp::Random rand(1);

  emp::BitSet<32> a1(rand);

  emp::BitSet<32> b1(rand);

  emp::StreakMetric<32> streak;
  emp::AntiMod<emp::StreakMetric<32>> anti_streak;
  emp::HammingMetric<32> hamming;
  emp::SlideMod<emp::HammingMetric<32>> slide_hamming;

  emp::FlatMod<
    emp::MinDimMod<
      emp::StreakMetric<32>,
      1
    >
  > d_streak1;
  REQUIRE(d_streak1.width() == streak.width());

  emp::FlatMod<
    emp::MinDimMod<
      emp::AntiMod<
        typename emp::StreakMetric<32>
      >,
      1
    >
  > d_anti_streak1;
  REQUIRE(d_anti_streak1.width() == anti_streak.width());

  emp::FlatMod<
    emp::MinDimMod<
      typename emp::HammingMetric<32>,
      1
    >
  > d_hamming1;
  REQUIRE(d_hamming1.width() == hamming.width());

  emp::FlatMod<
    emp::MinDimMod<
      emp::SlideMod<
        typename emp::HammingMetric<32>
      >,
      1
    >
  > d_slide_hamming1;
  REQUIRE(d_slide_hamming1.width() == slide_hamming.width());

  REQUIRE(streak(a1, b1) == d_streak1(a1, b1));
  REQUIRE(anti_streak(a1, b1) == d_anti_streak1(a1, b1));
  REQUIRE(hamming(a1, b1) == d_hamming1(a1, b1));
  REQUIRE(slide_hamming(a1, b1) == d_slide_hamming1(a1, b1));

  emp::array<emp::BitSet<32>, 3> a3 = {
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand)
  };

  emp::array<emp::BitSet<32>, 3> b3 = {
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand),
    emp::BitSet<32>(rand)
  };

  emp::BitSet<96> flat_a3;
  emp::BitSet<96> flat_b3;

  for (size_t i = 0; i < 96; ++i) {
    flat_a3[i] = a3[i/32][i%32];
    flat_b3[i] = b3[i/32][i%32];
  }

  emp::FlatMod<
    emp::MinDimMod<
      emp::StreakMetric<32>,
      3
    >
  > d_streak3;
  REQUIRE(d_streak3.width() == streak.width() * 3);

  emp::FlatMod<
    emp::MinDimMod<
      emp::AntiMod<
        emp::StreakMetric<32>
      >,
      3
    >
  > d_anti_streak3;
  REQUIRE(d_anti_streak3.width() == anti_streak.width() * 3);

  emp::FlatMod<
    emp::MinDimMod<
      emp::HammingMetric<32>,
      3
    >
  > d_hamming3;
  REQUIRE(d_hamming3.width() == hamming.width() * 3);

  emp::FlatMod<
    emp::MinDimMod<
      emp::SlideMod<
        emp::HammingMetric<32>
      >,
      3
    >
  > d_slide_hamming3;
  REQUIRE(d_slide_hamming3.width() == slide_hamming.width() * 3);


  REQUIRE(
    std::min({streak(a3[0], b3[0]), streak(a3[1], b3[1]), streak(a3[2], b3[2])})
    -
    d_streak3(flat_a3,flat_b3)
    <= std::numeric_limits<double>::epsilon()
  );
  REQUIRE(
    std::min({anti_streak(a3[0], b3[0]), anti_streak(a3[1], b3[1]), anti_streak(a3[2], b3[2])})
    -
    d_anti_streak3(flat_a3,flat_b3)
    <= std::numeric_limits<double>::epsilon()
  );
  REQUIRE(
    std::min({hamming(a3[0], b3[0]), hamming(a3[1], b3[1]), hamming(a3[2], b3[2])})
    -
    d_hamming3(flat_a3,flat_b3)
    <= std::numeric_limits<double>::epsilon()
  );
  REQUIRE(
    std::min({slide_hamming(a3[0], b3[0]), slide_hamming(a3[1], b3[1]), slide_hamming(a3[2], b3[2])})
    -
    d_slide_hamming3(flat_a3,flat_b3)
    <= std::numeric_limits<double>::epsilon()
  );

  }

  // test SymmetricWrapMetric
  {
  const double norm = 8.0;
  const emp::BitSet<4> bs_0{0,0,0,0};
  const emp::BitSet<4> bs_1{0,0,0,1};
  const emp::BitSet<4> bs_7{0,1,1,1};
  const emp::BitSet<4> bs_8{1,0,0,0};
  const emp::BitSet<4> bs_11{1,0,1,1};
  const emp::BitSet<4> bs_15{1,1,1,1};

  emp::SymmetricWrapMetric<4> metric;

  REQUIRE(metric(bs_0, bs_1) == 1.0/norm);
  REQUIRE(metric(bs_1, bs_0) == 1.0/norm);

  REQUIRE(metric(bs_0, bs_7) == 7.0/norm);
  REQUIRE(metric(bs_7, bs_0) == 7.0/norm);

  REQUIRE(metric(bs_0, bs_8) == 8.0/norm);
  REQUIRE(metric(bs_8, bs_0) == 8.0/norm);

  REQUIRE(metric(bs_0, bs_11) == 5.0/norm);
  REQUIRE(metric(bs_11, bs_0) == 5.0/norm);

  REQUIRE(metric(bs_0, bs_15) == 1.0/norm);
  REQUIRE(metric(bs_15, bs_0) == 1.0/norm);

  REQUIRE(metric(bs_1, bs_7) == 6.0/norm);
  REQUIRE(metric(bs_7, bs_1) == 6.0/norm);

  REQUIRE(metric(bs_1, bs_8) == 7.0/norm);
  REQUIRE(metric(bs_8, bs_1) == 7.0/norm);

  REQUIRE(metric(bs_1, bs_11) == 6.0/norm);
  REQUIRE(metric(bs_11, bs_1) == 6.0/norm);

  REQUIRE(metric(bs_1, bs_15) == 2.0/norm);
  REQUIRE(metric(bs_15, bs_1) == 2.0/norm);

  REQUIRE(metric(bs_7, bs_8) == 1.0/norm);
  REQUIRE(metric(bs_8, bs_7) == 1.0/norm);

  REQUIRE(metric(bs_7, bs_11) == 4.0/norm);
  REQUIRE(metric(bs_11, bs_7) == 4.0/norm);

  REQUIRE(metric(bs_7, bs_15) == 8.0/norm);
  REQUIRE(metric(bs_15, bs_7) == 8.0/norm);

  REQUIRE(metric(bs_8, bs_11) == 3.0/norm);
  REQUIRE(metric(bs_11, bs_8) == 3.0/norm);

  REQUIRE(metric(bs_8, bs_15) == 7.0/norm);
  REQUIRE(metric(bs_15, bs_8) == 7.0/norm);

  REQUIRE(metric(bs_11, bs_15) == 4.0/norm);
  REQUIRE(metric(bs_15, bs_11) == 4.0/norm);
  }

  // test SymmetricNoWrapMetric
  {
  const double norm = 15.0;
  const emp::BitSet<4> bs_0{0,0,0,0};
  const emp::BitSet<4> bs_1{0,0,0,1};
  const emp::BitSet<4> bs_7{0,1,1,1};
  const emp::BitSet<4> bs_8{1,0,0,0};
  const emp::BitSet<4> bs_11{1,0,1,1};
  const emp::BitSet<4> bs_15{1,1,1,1};

  emp::SymmetricNoWrapMetric<4> metric;

  REQUIRE(metric(bs_0, bs_1) == 1.0/norm);
  REQUIRE(metric(bs_1, bs_0) == 1.0/norm);

  REQUIRE(metric(bs_0, bs_7) == 7.0/norm);
  REQUIRE(metric(bs_7, bs_0) == 7.0/norm);

  REQUIRE(metric(bs_0, bs_8) == 8.0/norm);
  REQUIRE(metric(bs_8, bs_0) == 8.0/norm);

  REQUIRE(metric(bs_0, bs_11) == 11.0/norm);
  REQUIRE(metric(bs_11, bs_0) == 11.0/norm);

  REQUIRE(metric(bs_0, bs_15) == 15.0/norm);
  REQUIRE(metric(bs_15, bs_0) == 15.0/norm);

  REQUIRE(metric(bs_1, bs_7) == 6.0/norm);
  REQUIRE(metric(bs_7, bs_1) == 6.0/norm);

  REQUIRE(metric(bs_1, bs_8) == 7.0/norm);
  REQUIRE(metric(bs_8, bs_1) == 7.0/norm);

  REQUIRE(metric(bs_1, bs_11) == 10.0/norm);
  REQUIRE(metric(bs_11, bs_1) == 10.0/norm);

  REQUIRE(metric(bs_1, bs_15) == 14.0/norm);
  REQUIRE(metric(bs_15, bs_1) == 14.0/norm);

  REQUIRE(metric(bs_7, bs_8) == 1.0/norm);
  REQUIRE(metric(bs_8, bs_7) == 1.0/norm);

  REQUIRE(metric(bs_7, bs_11) == 4.0/norm);
  REQUIRE(metric(bs_11, bs_7) == 4.0/norm);

  REQUIRE(metric(bs_7, bs_15) == 8.0/norm);
  REQUIRE(metric(bs_15, bs_7) == 8.0/norm);

  REQUIRE(metric(bs_8, bs_11) == 3.0/norm);
  REQUIRE(metric(bs_11, bs_8) == 3.0/norm);

  REQUIRE(metric(bs_8, bs_15) == 7.0/norm);
  REQUIRE(metric(bs_15, bs_8) == 7.0/norm);

  REQUIRE(metric(bs_11, bs_15) == 4.0/norm);
  REQUIRE(metric(bs_15, bs_11) == 4.0/norm);
  }

  // test AsymmetricWrapMetric
  {
  const double norm = 15.0;
  const emp::BitSet<4> bs_0{0,0,0,0};
  const emp::BitSet<4> bs_1{0,0,0,1};
  const emp::BitSet<4> bs_7{0,1,1,1};
  const emp::BitSet<4> bs_8{1,0,0,0};
  const emp::BitSet<4> bs_11{1,0,1,1};
  const emp::BitSet<4> bs_15{1,1,1,1};

  emp::AsymmetricWrapMetric<4> metric;

  REQUIRE(metric(bs_0, bs_1) == 1.0/norm);
  REQUIRE(metric(bs_1, bs_0) == 15.0/norm);

  REQUIRE(metric(bs_0, bs_7) == 7.0/norm);
  REQUIRE(metric(bs_7, bs_0) == 9.0/norm);

  REQUIRE(metric(bs_0, bs_8) == 8.0/norm);
  REQUIRE(metric(bs_8, bs_0) == 8.0/norm);

  REQUIRE(metric(bs_0, bs_11) == 11.0/norm);
  REQUIRE(metric(bs_11, bs_0) == 5.0/norm);

  REQUIRE(metric(bs_0, bs_15) == 15.0/norm);
  REQUIRE(metric(bs_15, bs_0) == 1.0/norm);

  REQUIRE(metric(bs_1, bs_7) == 6.0/norm);
  REQUIRE(metric(bs_7, bs_1) == 10.0/norm);

  REQUIRE(metric(bs_1, bs_8) == 7.0/norm);
  REQUIRE(metric(bs_8, bs_1) == 9.0/norm);

  REQUIRE(metric(bs_1, bs_11) == 10.0/norm);
  REQUIRE(metric(bs_11, bs_1) == 6.0/norm);

  REQUIRE(metric(bs_1, bs_15) == 14.0/norm);
  REQUIRE(metric(bs_15, bs_1) == 2.0/norm);

  REQUIRE(metric(bs_7, bs_8) == 1.0/norm);
  REQUIRE(metric(bs_8, bs_7) == 15.0/norm);

  REQUIRE(metric(bs_7, bs_11) == 4.0/norm);
  REQUIRE(metric(bs_11, bs_7) == 12.0/norm);

  REQUIRE(metric(bs_7, bs_15) == 8.0/norm);
  REQUIRE(metric(bs_15, bs_7) == 8.0/norm);

  REQUIRE(metric(bs_8, bs_11) == 3.0/norm);
  REQUIRE(metric(bs_11, bs_8) == 13.0/norm);

  REQUIRE(metric(bs_8, bs_15) == 7.0/norm);
  REQUIRE(metric(bs_15, bs_8) == 9.0/norm);

  REQUIRE(metric(bs_11, bs_15) == 4.0/norm);
  REQUIRE(metric(bs_15, bs_11) == 12.0/norm);
  }

  // test AsymmetricNoWrapMetric
  {
  const double norm = 16.0;
  const emp::BitSet<4> bs_0{0,0,0,0};
  const emp::BitSet<4> bs_1{0,0,0,1};
  const emp::BitSet<4> bs_7{0,1,1,1};
  const emp::BitSet<4> bs_8{1,0,0,0};
  const emp::BitSet<4> bs_11{1,0,1,1};
  const emp::BitSet<4> bs_15{1,1,1,1};

  emp::AsymmetricNoWrapMetric<4> metric;

  REQUIRE(metric(bs_0, bs_1) == 1.0/norm);
  REQUIRE(metric(bs_1, bs_0) == 16.0/norm);

  REQUIRE(metric(bs_0, bs_7) == 7.0/norm);
  REQUIRE(metric(bs_7, bs_0) == 16.0/norm);

  REQUIRE(metric(bs_0, bs_8) == 8.0/norm);
  REQUIRE(metric(bs_8, bs_0) == 16.0/norm);

  REQUIRE(metric(bs_0, bs_11) == 11.0/norm);
  REQUIRE(metric(bs_11, bs_0) == 16.0/norm);

  REQUIRE(metric(bs_0, bs_15) == 15.0/norm);
  REQUIRE(metric(bs_15, bs_0) == 16.0/norm);

  REQUIRE(metric(bs_1, bs_7) == 6.0/norm);
  REQUIRE(metric(bs_7, bs_1) == 16.0/norm);

  REQUIRE(metric(bs_1, bs_8) == 7.0/norm);
  REQUIRE(metric(bs_8, bs_1) == 16.0/norm);

  REQUIRE(metric(bs_1, bs_11) == 10.0/norm);
  REQUIRE(metric(bs_11, bs_1) == 16.0/norm);

  REQUIRE(metric(bs_1, bs_15) == 14.0/norm);
  REQUIRE(metric(bs_15, bs_1) == 16.0/norm);

  REQUIRE(metric(bs_7, bs_8) == 1.0/norm);
  REQUIRE(metric(bs_8, bs_7) == 16.0/norm);

  REQUIRE(metric(bs_7, bs_11) == 4.0/norm);
  REQUIRE(metric(bs_11, bs_7) == 16.0/norm);

  REQUIRE(metric(bs_7, bs_15) == 8.0/norm);
  REQUIRE(metric(bs_15, bs_7) == 16.0/norm);

  REQUIRE(metric(bs_8, bs_11) == 3.0/norm);
  REQUIRE(metric(bs_11, bs_8) == 16.0/norm);

  REQUIRE(metric(bs_8, bs_15) == 7.0/norm);
  REQUIRE(metric(bs_15, bs_8) == 16.0/norm);

  REQUIRE(metric(bs_11, bs_15) == 4.0/norm);
  REQUIRE(metric(bs_15, bs_11) == 16.0/norm);
  }
  {
  // Cache Testing
  struct DummySelector: public emp::RankedSelector<std::ratio<2,1>>{

    DummySelector(emp::Random &rand) : emp::RankedSelector<std::ratio<2,1>>(rand) { ; }

    size_t opCount = 0;

    emp::RankedCacheState operator()(
      emp::vector<size_t>& uids,
      std::unordered_map<size_t, double>& scores,
      size_t n
    ){
      opCount+=1;
      return emp::RankedSelector<std::ratio<2,1>>::operator()(uids, scores, n);
    }
  };

  using parent_t = emp::MatchBin<
    emp::BitSet<32>,
    emp::HammingMetric<32>,
    DummySelector,
    emp::LegacyRegulator
  >;
  class MatchBinTest : public parent_t {
  public:
    MatchBinTest(emp::Random & rand) : parent_t(rand) { ; }

    size_t GetCacheSize(){ return cache_regulated.size(); }
    size_t GetSelectCount(){ return selector.opCount; }
  };

  emp::Random rand(1);
  MatchBinTest bin(rand);
  std::vector<size_t> ids;

  for(unsigned int i = 0; i < 1000; ++i){
    emp::BitSet<32> bs;
    bs.SetUInt32(0, i);
    ids.push_back(bin.Put(bs,bs));
  }

  REQUIRE( bin.GetCacheSize() == 0);
  REQUIRE( bin.GetSelectCount() == 0);
  emp::vector<size_t> uncached = bin.Match(emp::BitSet<32>(), 10);// first match caches
  emp::vector<size_t> cached = bin.Match(emp::BitSet<32>(), 10);// second already cached
  REQUIRE( bin.GetCacheSize() == 1);
  REQUIRE( bin.GetSelectCount() == 1);
  REQUIRE( cached == uncached );
  bin.DeactivateCaching();
  REQUIRE(bin.GetCacheSize() == 0 );
  bin.Match(emp::BitSet<32>(),10);//second cache
  bin.Match(emp::BitSet<32>(),10);//third cache
  REQUIRE(bin.GetCacheSize() == 0 );
  REQUIRE(bin.GetSelectCount() == 3);

  bin.ActivateCaching();
  REQUIRE(bin.GetCacheSize() == 0 );


  for(unsigned int i = 0; i < 1000; ++i){
    emp::BitSet<32> bs;
    bs.SetUInt32(0, i);

    uncached = bin.Match(bs, 3);
    REQUIRE(bin.GetCacheSize() == i + 1);
    REQUIRE(bin.GetSelectCount() == 3 + i + 1);

    cached = bin.Match(bs, 3);
    REQUIRE(bin.GetCacheSize() == i + 1); //shouldnt change
    REQUIRE(bin.GetSelectCount() == 3 + i + 1); //shouldnt change

    REQUIRE(cached == uncached);
  }

  emp::BitSet<32> bs;
  bs.SetUInt32(0,1001);
  bin.SetTag(ids[0], bs);
  REQUIRE(bin.GetCacheSize() == 0);

  bin.Match(emp::BitSet<32>(), 3);
  REQUIRE(bin.GetCacheSize() == 1);
  REQUIRE(bin.GetSelectCount() == 1000 + 3 + 1);

  bin.Match(emp::BitSet<32>(), 4); //Asking for more than last time so we recache.
  REQUIRE(bin.GetCacheSize() == 1); //replace the current one so same size.
  REQUIRE(bin.GetSelectCount() == 1000 + 3 + 2);
  }

  // serialization / deserialization
  {
  // set up
  emp::Random rand(1);
  std::stringstream ss;

  {
    // Create an output archive
    cereal::JSONOutputArchive oarchive(ss);

    emp::MatchBin<
      std::string,
      emp::AbsDiffMetric,
      emp::RouletteSelector<>,
      emp::LegacyRegulator
    > bin(rand);

    const size_t hi = bin.Put("hi", 1);
    REQUIRE( bin.GetVal(hi) == "hi" );
    const size_t salut = bin.Put("salut", 0);
    REQUIRE( bin.GetVal(salut) == "salut" );

    REQUIRE( bin.GetVal(bin.Put("bonjour", 6)) == "bonjour" );
    REQUIRE( bin.GetVal(bin.Put("yo", -4)) == "yo" );
    REQUIRE( bin.GetVal(bin.Put("konichiwa", -6)) == "konichiwa" );

    // Write the data to the archive
    oarchive(bin.GetState());

  } // archive goes out of scope, ensuring all contents are flushed

  emp::MatchBin<
    std::string,
    emp::AbsDiffMetric,
    emp::RankedSelector<std::ratio<214748364700+599,214748364700>>,
    emp::LegacyRegulator
  > bin(rand);

  {
    cereal::JSONInputArchive iarchive(ss); // Create an input archive

    // Read the data from the archive
    decltype(bin)::state_t state;
    iarchive(state);
    bin.SetState(state);

  }

  REQUIRE( bin.Size() == 5 );

  // 0 = use Selector default, which is 1
  REQUIRE( bin.GetVals(bin.Match(0, 0)) == emp::vector<std::string>{"salut"} );
  REQUIRE( bin.GetTags(bin.Match(0, 0)) == emp::vector<int>{0} );

  REQUIRE( bin.GetVals(bin.Match(0, 1)) == emp::vector<std::string>{"salut"} );
  REQUIRE( bin.GetTags(bin.Match(0, 1)) == emp::vector<int>{0} );

  REQUIRE(
    bin.GetVals(bin.Match(0, 2)) == (emp::vector<std::string>{"salut", "hi"})
  );
  REQUIRE( bin.GetTags(bin.Match(0, 2)) == (emp::vector<int>{0, 1}) );

  REQUIRE(
    bin.GetVals(bin.Match(0, 3)) == (emp::vector<std::string>{"salut", "hi", "yo"})
  );
  REQUIRE( bin.GetTags(bin.Match(0, 3)) == (emp::vector<int>{0, 1, -4}) );

  REQUIRE(
    bin.GetVals(bin.Match(0, 4)) == (emp::vector<std::string>{"salut", "hi", "yo"})
  );
  REQUIRE( bin.GetTags(bin.Match(0, 4)) == (emp::vector<int>{0, 1, -4}) );

  REQUIRE( bin.GetVals(bin.Match(15, 8)) == emp::vector<std::string>{} );
  REQUIRE( bin.GetTags(bin.Match(15, 8)) == (emp::vector<int>{}) );

  REQUIRE( bin.GetVals(bin.Match(10, 2)) == emp::vector<std::string>{"bonjour"} );
  REQUIRE( bin.GetTags(bin.Match(10, 2)) == (emp::vector<int>{6}) );


}

}



TEST_CASE("Test math", "[tools]")
{
  constexpr auto a1 = emp::Log2(3.14);           REQUIRE( a1 > 1.650);   REQUIRE( a1 < 1.651);
  constexpr auto a2 = emp::Log2(0.125);          REQUIRE( a2 == -3.0 );
  constexpr auto a3 = emp::Log(1000, 10);        REQUIRE( a3 == 3.0 );
  constexpr auto a4 = emp::Log(10, 1000);        REQUIRE( a4 > 0.333 );  REQUIRE( a4 < 0.334 );
  constexpr auto a5 = emp::Log10(100);           REQUIRE( a5 == 2.0 );
  constexpr auto a6 = emp::Ln(3.33);             REQUIRE( a6 > 1.202 );  REQUIRE( a6 < 1.204 );
  constexpr auto a7 = emp::Pow2(2.345);          REQUIRE( a7 > 5.080 );  REQUIRE( a7 < 5.081 );
  constexpr auto a8 = emp::Pow(emp::PI, emp::E); REQUIRE( a8 > 22.440 ); REQUIRE( a8 < 22.441 );
  constexpr auto a9 = emp::Pow(7, 10);           REQUIRE( a9 == 282475249 );

  REQUIRE(emp::Mod(10, 7) == 3);
  REQUIRE(emp::Mod(3, 7) == 3);
  REQUIRE(emp::Mod(-4, 7) == 3);
  REQUIRE(emp::Mod(-11, 7) == 3);

  REQUIRE(emp::Mod(-11, 11) == 0);
  REQUIRE(emp::Mod(0, 11) == 0);
  REQUIRE(emp::Mod(11, 11) == 0);

  REQUIRE(emp::Pow(2,3) == 8);
  REQUIRE(emp::Pow(-2,2) == 4);
  REQUIRE(emp::IntPow(3,4) == 81);

  REQUIRE(emp::Min(5) == 5);
  REQUIRE(emp::Min(5,10) == 5);
  REQUIRE(emp::Min(10,5) == 5);
  REQUIRE(emp::Min(40,30,20,10,5,15,25,35) == 5);

  REQUIRE(emp::Max(5) == 5);
  REQUIRE(emp::Max(5,10) == 10);
  REQUIRE(emp::Max(10,5) == 10);
  REQUIRE(emp::Max(40,30,20,10,45,15,25,35) == 45);

  REQUIRE(emp::FloorDivide(0,4) == 0);
  REQUIRE(emp::FloorDivide(1,4) == 0);
  REQUIRE(emp::FloorDivide(2,4) == 0);
  REQUIRE(emp::FloorDivide(3,4) == 0);
  REQUIRE(emp::FloorDivide(4,4) == 1);
  REQUIRE(emp::FloorDivide(6,4) == 1);
  REQUIRE(emp::FloorDivide(5,3) == 1);
  REQUIRE(emp::FloorDivide(6,3) == 2);
  REQUIRE(emp::FloorDivide(7,3) == 2);

  REQUIRE(emp::FloorDivide((size_t)0,(size_t)4) == 0);
  REQUIRE(emp::FloorDivide((size_t)1,(size_t)4) == 0);
  REQUIRE(emp::FloorDivide((size_t)2,(size_t)4) == 0);
  REQUIRE(emp::FloorDivide((size_t)3,(size_t)4) == 0);
  REQUIRE(emp::FloorDivide((size_t)4,(size_t)4) == 1);
  REQUIRE(emp::FloorDivide((size_t)6,(size_t)4) == 1);
  REQUIRE(emp::FloorDivide((size_t)5,(size_t)3) == 1);
  REQUIRE(emp::FloorDivide((size_t)6,(size_t)3) == 2);
  REQUIRE(emp::FloorDivide((size_t)7,(size_t)3) == 2);

  REQUIRE(emp::FloorDivide(-1,4) == -1);
  REQUIRE(emp::FloorDivide(-2,4) == -1);
  REQUIRE(emp::FloorDivide(-3,4) == -1);
  REQUIRE(emp::FloorDivide(-4,4) == -1);
  REQUIRE(emp::FloorDivide(-6,4) == -2);
  REQUIRE(emp::FloorDivide(-5,3) == -2);
  REQUIRE(emp::FloorDivide(-6,3) == -2);
  REQUIRE(emp::FloorDivide(-7,3) == -3);

  REQUIRE(emp::FloorDivide(0,-4) == 0);
  REQUIRE(emp::FloorDivide(1,-4) == -1);
  REQUIRE(emp::FloorDivide(2,-4) == -1);
  REQUIRE(emp::FloorDivide(3,-4) == -1);
  REQUIRE(emp::FloorDivide(4,-4) == -1);
  REQUIRE(emp::FloorDivide(6,-4) == -2);
  REQUIRE(emp::FloorDivide(5,-3) == -2);
  REQUIRE(emp::FloorDivide(6,-3) == -2);
  REQUIRE(emp::FloorDivide(7,-3) == -3);

  REQUIRE(emp::FloorDivide(-1,-4) == 0);
  REQUIRE(emp::FloorDivide(-2,-4) == 0);
  REQUIRE(emp::FloorDivide(-3,-4) == 0);
  REQUIRE(emp::FloorDivide(-4,-4) == 1);
  REQUIRE(emp::FloorDivide(-6,-4) == 1);
  REQUIRE(emp::FloorDivide(-5,-3) == 1);
  REQUIRE(emp::FloorDivide(-6,-3) == 2);
  REQUIRE(emp::FloorDivide(-7,-3) == 2);

  REQUIRE(emp::RoundedDivide(0,4) == 0);
  REQUIRE(emp::RoundedDivide(1,4) == 0);
  REQUIRE(emp::RoundedDivide(2,4) == 1);
  REQUIRE(emp::RoundedDivide(3,4) == 1);
  REQUIRE(emp::RoundedDivide(4,4) == 1);
  REQUIRE(emp::RoundedDivide(6,4) == 2);
  REQUIRE(emp::RoundedDivide(5,3) == 2);
  REQUIRE(emp::RoundedDivide(6,3) == 2);
  REQUIRE(emp::RoundedDivide(7,3) == 2);

  REQUIRE(emp::RoundedDivide((size_t)0,(size_t)4) == 0);
  REQUIRE(emp::RoundedDivide((size_t)1,(size_t)4) == 0);
  REQUIRE(emp::RoundedDivide((size_t)2,(size_t)4) == 1);
  REQUIRE(emp::RoundedDivide((size_t)3,(size_t)4) == 1);
  REQUIRE(emp::RoundedDivide((size_t)4,(size_t)4) == 1);
  REQUIRE(emp::RoundedDivide((size_t)6,(size_t)4) == 2);
  REQUIRE(emp::RoundedDivide((size_t)5,(size_t)3) == 2);
  REQUIRE(emp::RoundedDivide((size_t)6,(size_t)3) == 2);
  REQUIRE(emp::RoundedDivide((size_t)7,(size_t)3) == 2);

  REQUIRE(emp::RoundedDivide(-1,4) == 0);
  REQUIRE(emp::RoundedDivide(-2,4) == 0);
  REQUIRE(emp::RoundedDivide(-3,4) == -1);
  REQUIRE(emp::RoundedDivide(-4,4) == -1);
  REQUIRE(emp::RoundedDivide(-6,4) == -1);
  REQUIRE(emp::RoundedDivide(-5,3) == -2);
  REQUIRE(emp::RoundedDivide(-6,3) == -2);
  REQUIRE(emp::RoundedDivide(-7,3) == -2);

  REQUIRE(emp::RoundedDivide(0,-4) == 0);
  REQUIRE(emp::RoundedDivide(1,-4) == 0);
  REQUIRE(emp::RoundedDivide(2,-4) == 0);
  REQUIRE(emp::RoundedDivide(3,-4) == -1);
  REQUIRE(emp::RoundedDivide(4,-4) == -1);
  REQUIRE(emp::RoundedDivide(6,-4) == -1);
  REQUIRE(emp::RoundedDivide(5,-3) == -2);
  REQUIRE(emp::RoundedDivide(6,-3) == -2);
  REQUIRE(emp::RoundedDivide(7,-3) == -2);

  REQUIRE(emp::RoundedDivide(-1,-4) == 0);
  REQUIRE(emp::RoundedDivide(-2,-4) == 1);
  REQUIRE(emp::RoundedDivide(-3,-4) == 1);
  REQUIRE(emp::RoundedDivide(-4,-4) == 1);
  REQUIRE(emp::RoundedDivide(-6,-4) == 2);
  REQUIRE(emp::RoundedDivide(-5,-3) == 2);
  REQUIRE(emp::RoundedDivide(-6,-3) == 2);
  REQUIRE(emp::RoundedDivide(-7,-3) == 2);

  REQUIRE(emp::RoundedDivide((size_t)0,(size_t)4) == 0);
  REQUIRE(emp::RoundedDivide((size_t)1,(size_t)4) == 0);
  REQUIRE(emp::RoundedDivide((size_t)2,(size_t)4) == 1);
  REQUIRE(emp::RoundedDivide((size_t)3,(size_t)4) == 1);
  REQUIRE(emp::RoundedDivide((size_t)4,(size_t)4) == 1);
  REQUIRE(emp::RoundedDivide((size_t)6,(size_t)4) == 2);
  REQUIRE(emp::RoundedDivide((size_t)5,(size_t)3) == 2);
  REQUIRE(emp::RoundedDivide((size_t)6,(size_t)3) == 2);
  REQUIRE(emp::RoundedDivide((size_t)7,(size_t)3) == 2);

  auto MeanUnbiasedDivide = [](int dividend, int divisor, size_t rc){
    emp::Random r = emp::Random(1);
    emp::DataNode<double, emp::data::Current, emp::data::Range, emp::data::Log> data;
    for(size_t i=0;i<rc;++i) data.Add(emp::UnbiasedDivide(dividend,divisor,r));
    return data.GetMean();
  };

  REQUIRE(MeanUnbiasedDivide(0,4,100) == 0);
  REQUIRE(MeanUnbiasedDivide(1,4,100) == 0);
  REQUIRE(MeanUnbiasedDivide(2,4,100) > 0);
  REQUIRE(MeanUnbiasedDivide(2,4,100) < 1);
  REQUIRE(MeanUnbiasedDivide(3,4,100) == 1);
  REQUIRE(MeanUnbiasedDivide(4,4,100) == 1);
  REQUIRE(MeanUnbiasedDivide(6,4,100) > 1);
  REQUIRE(MeanUnbiasedDivide(6,4,100) < 2);
  REQUIRE(MeanUnbiasedDivide(5,3,100) == 2);
  REQUIRE(MeanUnbiasedDivide(6,3,100) == 2);
  REQUIRE(MeanUnbiasedDivide(7,3,100) == 2);

  REQUIRE(MeanUnbiasedDivide(-1,4,100) == 0);
  REQUIRE(MeanUnbiasedDivide(-2,4,100) < 0);
  REQUIRE(MeanUnbiasedDivide(-2,4,100) > -1);
  REQUIRE(MeanUnbiasedDivide(-3,4,100) == -1);
  REQUIRE(MeanUnbiasedDivide(-4,4,100) == -1);
  REQUIRE(MeanUnbiasedDivide(-6,4,100) < -1);
  REQUIRE(MeanUnbiasedDivide(-6,4,100) > -2);
  REQUIRE(MeanUnbiasedDivide(-5,3,100) == -2);
  REQUIRE(MeanUnbiasedDivide(-6,3,100) == -2);
  REQUIRE(MeanUnbiasedDivide(-7,3,100) == -2);

  REQUIRE(MeanUnbiasedDivide(0,-4,100) == 0);
  REQUIRE(MeanUnbiasedDivide(1,-4,100) == 0);
  REQUIRE(MeanUnbiasedDivide(2,-4,100) < 0);
  REQUIRE(MeanUnbiasedDivide(2,-4,100) > -1);
  REQUIRE(MeanUnbiasedDivide(3,-4,100) == -1);
  REQUIRE(MeanUnbiasedDivide(4,-4,100) == -1);
  REQUIRE(MeanUnbiasedDivide(6,-4,100) < -1);
  REQUIRE(MeanUnbiasedDivide(6,-4,100) > -2);
  REQUIRE(MeanUnbiasedDivide(5,-3,100) == -2);
  REQUIRE(MeanUnbiasedDivide(6,-3,100) == -2);
  REQUIRE(MeanUnbiasedDivide(7,-3,100) == -2);

  REQUIRE(MeanUnbiasedDivide(-1,-4,100) == 0);
  REQUIRE(MeanUnbiasedDivide(-2,-4,100) > 0);
  REQUIRE(MeanUnbiasedDivide(-2,-4,100) < 1);
  REQUIRE(MeanUnbiasedDivide(-3,-4,100) == 1);
  REQUIRE(MeanUnbiasedDivide(-4,-4,100) == 1);
  REQUIRE(MeanUnbiasedDivide(-6,-4,100) > 1);
  REQUIRE(MeanUnbiasedDivide(-6,-4,100) < 2);
  REQUIRE(MeanUnbiasedDivide(-5,-3,100) == 2);
  REQUIRE(MeanUnbiasedDivide(-6,-3,100) == 2);
  REQUIRE(MeanUnbiasedDivide(-7,-3,100) == 2);

  auto SztMeanUnbiasedDivide = [](size_t dividend, size_t divisor, size_t rc){
    emp::Random r = emp::Random(1);
    emp::DataNode<double, emp::data::Current, emp::data::Range, emp::data::Log> data;
    for(size_t i=0;i<rc;++i) data.Add(emp::UnbiasedDivide(dividend,divisor,r));
    return data.GetMean();
  };

  REQUIRE(SztMeanUnbiasedDivide((size_t)0,(size_t)4,100) == 0);
  REQUIRE(SztMeanUnbiasedDivide((size_t)1,(size_t)4,100) == 0);
  REQUIRE(SztMeanUnbiasedDivide((size_t)2,(size_t)4,100) > 0);
  REQUIRE(SztMeanUnbiasedDivide((size_t)2,(size_t)4,100) < 1);
  REQUIRE(SztMeanUnbiasedDivide((size_t)3,(size_t)4,100) == 1);
  REQUIRE(SztMeanUnbiasedDivide((size_t)4,(size_t)4,100) == 1);
  REQUIRE(SztMeanUnbiasedDivide((size_t)6,(size_t)4,100) > 1);
  REQUIRE(SztMeanUnbiasedDivide((size_t)6,(size_t)4,100) < 2);
  REQUIRE(SztMeanUnbiasedDivide((size_t)5,(size_t)3,100) == 2);
  REQUIRE(SztMeanUnbiasedDivide((size_t)6,(size_t)3,100) == 2);
  REQUIRE(SztMeanUnbiasedDivide((size_t)7,(size_t)3,100) == 2);

  REQUIRE(emp::Sgn(1) == 1);
  REQUIRE(emp::Sgn(2) == 1);
  REQUIRE(emp::Sgn(3) == 1);
  REQUIRE(emp::Sgn(102) == 1);
  REQUIRE(emp::Sgn(0) == 0);
  REQUIRE(emp::Sgn(-1) == -1);
  REQUIRE(emp::Sgn(-2) == -1);
  REQUIRE(emp::Sgn(-3) == -1);
  REQUIRE(emp::Sgn(-102) == -1);

  REQUIRE(emp::Sgn((size_t)1) == 1);
  REQUIRE(emp::Sgn((size_t)2) == 1);
  REQUIRE(emp::Sgn((size_t)3) == 1);
  REQUIRE(emp::Sgn((size_t)102) == 1);
  REQUIRE(emp::Sgn((size_t)0) == 0);

  REQUIRE(emp::Sgn(1.0) == 1);
  REQUIRE(emp::Sgn(2.1) == 1);
  REQUIRE(emp::Sgn(3.0) == 1);
  REQUIRE(emp::Sgn(102.5) == 1);
  REQUIRE(emp::Sgn(0.0) == 0);
  REQUIRE(emp::Sgn(-1.0) == -1);
  REQUIRE(emp::Sgn(-2.1) == -1);
  REQUIRE(emp::Sgn(-3.0) == -1);
  REQUIRE(emp::Sgn(-102.5) == -1);

}


struct TestClass1 {
  TestClass1() {
    EMP_TRACK_CONSTRUCT(TestClass1);
  }
  ~TestClass1() {
    EMP_TRACK_DESTRUCT(TestClass1);
  }
};

struct TestClass2 {
  TestClass2() {
    EMP_TRACK_CONSTRUCT(TestClass2);
  }
  ~TestClass2() {
    EMP_TRACK_DESTRUCT(TestClass2);
  }
};

TEST_CASE("Test mem_track", "[tools]")
{
  emp::vector<TestClass1 *> test_v;
  TestClass2 class2_mem;

  #ifdef EMP_TRACK_MEM
  REQUIRE(EMP_TRACK_COUNT(TestClass1) == 0);
  #endif

  for (int i = 0; i < 1000; i++) {
    test_v.push_back( new TestClass1 );
  }

  #ifdef EMP_TRACK_MEM
  REQUIRE(EMP_TRACK_COUNT(TestClass1) == 1000);
  #endif

  for (size_t i = 500; i < 1000; i++) {
    delete test_v[i];
  }

  #ifdef EMP_TRACK_MEM
  REQUIRE(EMP_TRACK_COUNT(TestClass1) == 500);
  //REQUIRE(EMP_TRACK_STATUS == 0);
  #endif

}


TEST_CASE("Test emp::memo_function", "[tools]")
{
  emp::memo_function<uint64_t(int)> test_fun;

  // Build a Fibonacchi function...
  test_fun = [&test_fun](int N) {
    if (N<=1) return (uint64_t) N;
    return test_fun(N-1) + test_fun(N-2);
  };

  REQUIRE( test_fun(80) == 0x533163ef0321e5 );
}


TEST_CASE("Test NFA", "[tools]")
{
  emp::NFA nfa(10);
  nfa.AddTransition(0, 1, 'a');
  nfa.AddTransition(0, 2, 'a');
  nfa.AddTransition(0, 3, 'a');
  nfa.AddTransition(0, 4, 'a');

  nfa.AddTransition(1, 2, 'b');
  nfa.AddTransition(2, 3, 'c');
  nfa.AddTransition(3, 4, 'd');

  nfa.AddTransition(0, 1, 'e');
  nfa.AddTransition(0, 1, 'f');
  nfa.AddTransition(0, 1, 'g');

  nfa.AddTransition(2, 3, 'a');
  nfa.AddTransition(3, 4, 'a');
  nfa.AddTransition(2, 4, 'a');

  nfa.AddTransition(2, 2, 'e');
  nfa.AddTransition(3, 3, 'e');
  nfa.AddTransition(4, 4, 'e');

  nfa.AddFreeTransition(1,5);

  nfa.AddTransition(5, 6, 'a');

  nfa.AddFreeTransition(6,7);
  nfa.AddFreeTransition(6,8);
  nfa.AddFreeTransition(6,9);
  nfa.AddFreeTransition(9,0);

  emp::NFA_State state(nfa);
  REQUIRE(state.GetSize() == 1);
  state.Next('a');
  REQUIRE(state.GetSize() == 5);
  state.Next('a');
  REQUIRE(state.GetSize() == 7);

  emp::NFA_State state2(nfa);
  REQUIRE(state2.GetSize() == 1);
  state2.Next("aaaa");
  REQUIRE(state2.GetSize() == 7);
}


TEST_CASE("Test NullStream", "[tools]")
{
  emp::NullStream ns;
  ns << "abcdefg";
  ns << std::endl;
  ns << 123;
  ns << 123.456;
  ns.flush();
}


TEST_CASE("Test random", "[tools]")
{

  std::unordered_map<std::string, std::pair<size_t, size_t>> n_fails;

  // test over a consistent set of seeds
  for(int s = 1; s < 251; ++s) {

  REQUIRE(s > 0); // tests should be replicable
  emp::Random rng(s);

  // HERE'S THE MATH
  // Var(Unif) = 1/12 (1 - 0)^2 = 1/12
  // Std(Unif) = sqrt(1/12) = 0.28867513459481287
  // by central limit theorem,
  // Std(mean) =  Std(observation) / sqrt(num observs)
  // Std(mean) = 0.28867513459481287 / sqrt(100000) = 0.0009128709291752767
  // 0.0035 / 0.0009128709291752767 = 4 standard deviations
  // from WolframAlpha, 6.334×10^-5 observations outside 5.4 standard deviations
  // with 500 reps fail rate is 1 - (1 - 1E-8) ^ 500 = 5E-6
  const size_t num_tests = 100000;
  const double error_thresh = 0.0035;
  const double min_value = 2.5;
  const double max_value = 8.7;

  double total = 0.0;

  for (size_t i = 0; i < num_tests; i++) {
    const double cur_value = (
      (rng.GetDouble(min_value, max_value) - min_value)
      / (max_value - min_value)
    );
    total += cur_value;
  }

  {
    const double expected_mean = 0.5;
    const double min_threshold = (expected_mean-error_thresh);
    const double max_threshold = (expected_mean+error_thresh);
    double mean_value = total/(double) num_tests;

    REQUIRE(mean_value > min_threshold);
    REQUIRE(mean_value < max_threshold);
  }

  // Test GetInt
  total = 0.0;
  for (size_t i = 0; i < num_tests; i++) {
    const size_t cur_value = rng.GetInt(min_value, max_value);
    total += cur_value;
  }

  {
    const double expected_mean = static_cast<double>(
      static_cast<int>(min_value) + static_cast<int>(max_value) - 1
    ) / 2.0;
    const double min_threshold = (expected_mean*0.995);
    const double max_threshold = (expected_mean*1.005);
    double mean_value = total/(double) num_tests;

    n_fails["GetInt"].first += !(mean_value > min_threshold);
    n_fails["GetInt"].second += !(mean_value < max_threshold);
  }

  // Test GetUInt()
  emp::vector<uint32_t> uint32_draws;
  total = 0.0;
  for (size_t i = 0; i < num_tests; i++) {
    const uint32_t cur_value = rng.GetUInt();
    total += (
      cur_value / static_cast<double>(std::numeric_limits<uint32_t>::max())
    );
    uint32_draws.push_back(cur_value);
  }

  {
  const double expected_mean = 0.5;
  const double min_threshold = expected_mean-error_thresh;
  const double max_threshold = expected_mean+error_thresh;
  const double mean_value = total / static_cast<double>(num_tests);
  // std::cout << mean_value * 1000 << std::endl;

  n_fails["GetUInt"].first += !(mean_value > min_threshold);
  n_fails["GetUInt"].second += !(mean_value < max_threshold);
  // ensure that all bits are set at least once and unset at least once
  REQUIRE(std::numeric_limits<uint32_t>::max() == std::accumulate(
      std::begin(uint32_draws),
      std::end(uint32_draws),
      static_cast<uint32_t>(0),
      [](uint32_t accumulator, uint32_t val){ return accumulator | val; }
    )
  );
  REQUIRE(std::numeric_limits<uint32_t>::max() == std::accumulate(
      std::begin(uint32_draws),
      std::end(uint32_draws),
      static_cast<uint32_t>(0),
      [](uint32_t accumulator, uint32_t val){ return accumulator | (~val); }
    )
  );
  }

  // Test RandFill()
  uint32_t randfill_draws[num_tests];
  rng.RandFill(
    reinterpret_cast<unsigned char*>(randfill_draws),
    sizeof(randfill_draws)
  );

  total = 0.0;
  for (size_t i = 0; i < num_tests; i++) {
    total += (
      randfill_draws[i]
      / static_cast<double>(std::numeric_limits<uint32_t>::max())
    );
  }

  {
  const double expected_mean = 0.5;
  const double min_threshold = expected_mean-error_thresh;
  const double max_threshold = expected_mean+error_thresh;
  double mean_value = total / static_cast<double>(num_tests);

  n_fails["RandFill"].first += !(mean_value > min_threshold);
  n_fails["RandFill"].second += !(mean_value < max_threshold);
  // ensure that all bits are set at least once and unset at least once
  REQUIRE(std::numeric_limits<uint32_t>::max() == std::accumulate(
      std::begin(randfill_draws),
      std::end(randfill_draws),
      (uint32_t)0,
      [](uint32_t accumulator, uint32_t val){ return accumulator | val; }
    )
  );
  REQUIRE(std::numeric_limits<uint32_t>::max() == std::accumulate(
      std::begin(randfill_draws),
      std::end(randfill_draws),
      static_cast<uint32_t>(0),
      [](uint32_t accumulator, uint32_t val){ return accumulator | (~val); }
    )
  );
  }

  // Test GetUInt64
  emp::vector<uint64_t> uint64_draws;
  total = 0.0;
  double total2 = 0.0;
  for (size_t i = 0; i < num_tests; i++) {
    const uint64_t cur_value = rng.GetUInt64();
    uint64_draws.push_back(cur_value);

    uint32_t temp;
    std::memcpy(&temp, &cur_value, sizeof(temp));
    total += temp / static_cast<double>(std::numeric_limits<uint32_t>::max());
    std::memcpy(
      &temp,
      reinterpret_cast<const unsigned char *>(&cur_value) + sizeof(temp),
      sizeof(temp)
    );
    total2 += temp / static_cast<double>(std::numeric_limits<uint32_t>::max());

  }

  {
  const double expected_mean = 0.5;
  const double min_threshold = expected_mean-error_thresh;
  const double max_threshold = expected_mean+error_thresh;

  const double mean_value = total / static_cast<double>(num_tests);
  n_fails["GetUInt64"].first += !(mean_value > min_threshold);
  n_fails["GetUInt64"].second += !(mean_value < max_threshold);

  const double mean_value2 = total2 / static_cast<double>(num_tests);
  n_fails["GetUInt64"].first += !(mean_value2 > min_threshold);
  n_fails["GetUInt64"].second += !(mean_value2 < max_threshold);

  // ensure that all bits are set at least once and unset at least once
  REQUIRE(std::numeric_limits<uint64_t>::max() == std::accumulate(
      std::begin(uint64_draws),
      std::end(uint64_draws),
      static_cast<uint64_t>(0),
      [](uint64_t accumulator, uint64_t val){ return accumulator | val; }
    )
  );
  REQUIRE(std::numeric_limits<uint64_t>::max() == std::accumulate(
      std::begin(uint64_draws),
      std::end(uint64_draws),
      static_cast<uint64_t>(0),
      [](uint64_t accumulator, uint64_t val){ return accumulator | (~val); }
    )
  );

  }

  // Test P
  double flip_prob = 0.56789;
  int hit_count = 0;
  for (size_t i = 0; i < num_tests; i++) {
    if (rng.P(flip_prob)) hit_count++;
  }

  double actual_prob = ((double) hit_count) / (double) num_tests;

  REQUIRE(actual_prob < flip_prob + 0.01);
  REQUIRE(actual_prob > flip_prob - 0.01);


  // Mimimal test of Choose()
  emp::vector<size_t> choices = Choose(rng,100,10);

  REQUIRE(choices.size() == 10);

  }

  for (const auto & [k, v] : n_fails) {
    // std::cout << k << ": " << v.first << ", " << v.second << std::endl;
    REQUIRE(v.first + v.second == 0);
  }
}


TEST_CASE("Test regular expressions (RegEx)", "[tools]")
{
  emp::RegEx re1("a|bcdef");
  REQUIRE(re1.Test("a") == true);
  REQUIRE(re1.Test("bc") == false);
  REQUIRE(re1.Test("bcdef") == true);
  REQUIRE(re1.Test("bcdefg") == false);

  emp::RegEx re2("#[abcdefghijklm]*abc");
  REQUIRE(re2.Test("") == false);
  REQUIRE(re2.Test("#a") == false);
  REQUIRE(re2.Test("#aaaabc") == true);
  REQUIRE(re2.Test("#abcabc") == true);
  REQUIRE(re2.Test("#abcabcd") == false);

  emp::RegEx re3("xx(y|(z*)?)+xx");
  REQUIRE(re3.Test("xxxx") == true);
  REQUIRE(re3.Test("xxxxx") == false);
  REQUIRE(re3.Test("xxyxx") == true);
  REQUIRE(re3.Test("xxyyxx") == true);
  REQUIRE(re3.Test("xxzzzxx") == true);

  emp::RegEx re_WHITESPACE("[ \t\r]");
  emp::RegEx re_COMMENT("#.*");
  emp::RegEx re_INT_LIT("[0-9]+");
  emp::RegEx re_FLOAT_LIT("[0-9]+[.][0-9]+");
  emp::RegEx re_CHAR_LIT("'(.|(\\\\[\\\\'nt]))'");
  emp::RegEx re_STRING_LIT("[\"]((\\\\[nt\"\\\\])|[^\"])*\\\"");
  emp::RegEx re_ID("[a-zA-Z0-9_]+");

  REQUIRE(re_INT_LIT.Test("1234") == true);
  REQUIRE(re_FLOAT_LIT.Test("1234") == false);
  REQUIRE(re_ID.Test("1234") == true);
  REQUIRE(re_INT_LIT.Test("1234.56") == false);
  REQUIRE(re_FLOAT_LIT.Test("1234.56") == true);
  REQUIRE(re_ID.Test("1234.56") == false);

  std::string test_str = "\"1234\"";
  REQUIRE(re_STRING_LIT.Test(test_str) == true);
  REQUIRE(re_INT_LIT.Test(test_str) == false);

  std::string test_str2 = "\"1234\", \"5678\"";
  REQUIRE(re_STRING_LIT.Test(test_str2) == false);
}

TEST_CASE("Test sequence utils", "[tools]")
{
  std::string s1 = "This is the first test string.";
  std::string s2 = "This is the second test string.";

  REQUIRE(emp::calc_hamming_distance(s1,s2) == 19);
  REQUIRE(emp::calc_edit_distance(s1,s2) == 6);

  // std::string s3 = "abcdefghijklmnopqrstuvwWxyz";
  // std::string s4 = "abBcdefghijXXmnopqrstuvwxyz";

  // std::string s3 = "lmnopqrstuv";
  // std::string s4 = "abcdefghijklmnopqrstuvwxyz";

  std::string s3 = "adhlmnopqrstuvxy";
  std::string s4 = "abcdefghijklmnopqrstuvwxyz";

  emp::align(s3, s4, '_');

  REQUIRE(s3 == "a__d___h___lmnopqrstuv_xy_");

  emp::vector<int> v1 = { 1,2,3,4,5,6,7,8,9 };
  emp::vector<int> v2 = { 1,4,5,6,8 };

  emp::align(v1,v2,0);

  REQUIRE((v2 == emp::vector<int>({1,0,0,4,5,6,0,8,0})));
}



// struct SerializeTest {
//   int a;
//   float b;        // unimportant data!
//   std::string c;

//   SerializeTest(int _a, float _b, std::string _c) : a(_a), b(_b), c(_c) { ; }
//   EMP_SETUP_DATAPOD(SerializeTest, a, c);
// };

// struct SerializeTest_D : public SerializeTest {
//   char d = '$';

//   SerializeTest_D(int _a, float _b, std::string _c, char _d)
//     : SerializeTest(_a, _b, _c), d(_d) { ; }
//   EMP_SETUP_DATAPOD_D(SerializeTest_D, SerializeTest, d);
// };

// struct ExtraBase {
//   double e;

//   ExtraBase(double _e) : e(_e) { ; }
//   EMP_SETUP_DATAPOD(ExtraBase, e);
// };

// struct MultiTest : public SerializeTest, public ExtraBase {
//   bool f;

//   MultiTest(int _a, float _b, std::string _c, double _e, bool _f)
//     : SerializeTest(_a, _b, _c), ExtraBase(_e), f(_f) { ; }
//   EMP_SETUP_DATAPOD_D2(MultiTest, SerializeTest, ExtraBase, f);
// };

// struct NestedTest {
//   SerializeTest st;
//   std::string name;
//   SerializeTest_D std;
//   MultiTest mt;

//   NestedTest(int a1, float b1, std::string c1,
//              int a2, float b2, std::string c2, char d2,
//              int a3, float b3, std::string c3, double e3, bool f3)
//     : st(a1, b1, c1), name("my_class"), std(a2, b2, c2, d2), mt(a3, b3, c3, e3, f3) { ; }

//   EMP_SETUP_DATAPOD(NestedTest, st, name, std, mt);
// };

// struct BuiltInTypesTest {
//   const int a;
//   emp::vector<int> int_v;

//   BuiltInTypesTest(int _a, size_t v_size) : a(_a), int_v(v_size) {
//     for (size_t i = 0; i < v_size; i++) int_v[i] = (int)(i*i);
//   }

//   EMP_SETUP_DATAPOD(BuiltInTypesTest, a, int_v);
// };

// TEST_CASE("Test serialize", "[tools]")
// {
//   std::stringstream ss;
//   emp::serialize::DataPod pod(ss);


//   // Basic test...

//   SerializeTest st(7, 2.34, "my_test_string");
//   st.EMP_Store(pod);

//   SerializeTest st2(pod);

//   REQUIRE(st2.a == 7);                 // Make sure a was reloaded correctly.
//   REQUIRE(st2.c == "my_test_string");  // Make sure c was reloaded correctly.


//   // Derived class Test

//   SerializeTest_D stD(10,0.2,"three",'D');
//   stD.EMP_Store(pod);

//   SerializeTest_D stD2(pod);

//   REQUIRE(stD2.a == 10);
//   REQUIRE(stD2.c == "three");
//   REQUIRE(stD2.d == 'D');

//   // Multiply-derived class Test

//   MultiTest stM(111,2.22,"ttt",4.5,true);
//   stM.EMP_Store(pod);

//   MultiTest stM2(pod);


//   REQUIRE(stM2.a == 111);
//   REQUIRE(stM2.c == "ttt");
//   REQUIRE(stM2.e == 4.5);
//   REQUIRE(stM2.f == true);


//   // Nested objects test...

//   NestedTest nt(91, 3.14, "magic numbers",
//                 100, 0.01, "powers of 10", '1',
//                 1001, 1.001, "ones and zeros", 0.125, true);
//   nt.EMP_Store(pod);

//   NestedTest nt2(pod);

//   REQUIRE(nt2.st.a == 91);
//   REQUIRE(nt2.st.c == "magic numbers");
//   REQUIRE(nt2.name == "my_class");
//   REQUIRE(nt2.std.a == 100);
//   REQUIRE(nt2.std.c == "powers of 10");
//   REQUIRE(nt2.std.d == '1');
//   REQUIRE(nt2.mt.a == 1001);
//   REQUIRE(nt2.mt.c == "ones and zeros");
//   REQUIRE(nt2.mt.e == 0.125);
//   REQUIRE(nt2.mt.f == true);


//   // If we made it this far, everything must have worked!;

//   const int v_size = 43;
//   BuiltInTypesTest bitt(91, v_size);
//   bitt.EMP_Store(pod);


//   BuiltInTypesTest bitt2(pod);
// }



TEST_CASE("Test string_utils", "[tools]")
{

  // TEST1: lets test our conversion to an escaped string.
  const std::string special_string = "This\t5tr1ng\nis\non THREE (3) \"lines\".";
  std::string escaped_string = emp::to_escaped_string(special_string);

  // note: we had to double-escape the test to make sure this worked.
  REQUIRE(escaped_string == "This\\t5tr1ng\\nis\\non THREE (3) \\\"lines\\\".");

  // TEST2: Test more general conversion to literals.
  REQUIRE(emp::to_literal(42) == "42");
  REQUIRE(emp::to_literal('a') == "'a'");
  REQUIRE(emp::to_literal('\t') == "'\\t'");
  REQUIRE(emp::to_literal(1.234) == "1.234000");

  // TEST3: Make sure that we can properly identify different types of characters.
  int num_ws = 0;
  int num_cap = 0;
  int num_lower = 0;
  int num_let = 0;
  int num_num = 0;
  int num_alphanum = 0;
  int num_i = 0;
  int num_vowel = 0;
  for (char cur_char : special_string) {
    if (emp::is_whitespace(cur_char)) num_ws++;
    if (emp::is_upper_letter(cur_char)) num_cap++;
    if (emp::is_lower_letter(cur_char)) num_lower++;
    if (emp::is_letter(cur_char)) num_let++;
    if (emp::is_digit(cur_char)) num_num++;
    if (emp::is_alphanumeric(cur_char)) num_alphanum++;
    if (emp::is_valid(cur_char, [](char c){ return c=='i'; })) num_i++;
    if (emp::is_valid(cur_char, [](char c){return c=='a' || c=='A';},
                      [](char c){return c=='e' || c=='E';},
                      [](char c){return c=='i' || c=='I';},
                      [](char c){return c=='o' || c=='O';},
                      [](char c){return c=='u' || c=='U';},
                      [](char c){return c=='y';}
                      )) num_vowel++;
  }
  int num_other = ((int) special_string.size()) - num_alphanum - num_ws;


  REQUIRE(num_ws == 6);
  REQUIRE(num_cap == 6);
  REQUIRE(num_lower == 16);
  REQUIRE(num_let == 22);
  REQUIRE(num_num == 3);
  REQUIRE(num_alphanum == 25);
  REQUIRE(num_other == 5);
  REQUIRE(num_i == 3);
  REQUIRE(num_vowel == 7);

  std::string base_string = "This is an okay string.\n  \tThis\nis   -MY-    very best string!!!!   ";

  REQUIRE(
    emp::slugify(base_string)
    == "this-is-an-okay-string-this-is-my-very-best-string"
  );

  std::string first_line = emp::string_pop_line(base_string);

  REQUIRE(first_line == "This is an okay string.");
  REQUIRE(emp::string_get_word(first_line) == "This");

  emp::string_pop_word(first_line);

  REQUIRE(first_line == "is an okay string.");

  emp::remove_whitespace(first_line);

  REQUIRE(first_line == "isanokaystring.");

  std::string popped_str = emp::string_pop(first_line, "ns");

  REQUIRE(popped_str == "i");
  REQUIRE(first_line == "anokaystring.");


  popped_str = emp::string_pop(first_line, "ns");


  REQUIRE(popped_str == "a");
  REQUIRE(first_line == "okaystring.");


  popped_str = emp::string_pop(first_line, 'y');

  REQUIRE(popped_str == "oka");
  REQUIRE(first_line == "string.");

  emp::left_justify(base_string);
  REQUIRE(base_string == "This\nis   -MY-    very best string!!!!   ");

  emp::right_justify(base_string);
  REQUIRE(base_string == "This\nis   -MY-    very best string!!!!");

  emp::compress_whitespace(base_string);
  REQUIRE(base_string == "This is -MY- very best string!!!!");


  std::string view_test = "This is my view test!";
  REQUIRE( emp::view_string(view_test) == "This is my view test!" );
  REQUIRE( emp::view_string(view_test, 5) == "is my view test!" );
  REQUIRE( emp::view_string(view_test, 8, 2) == "my" );
  REQUIRE( emp::view_string_front(view_test,4) == "This" );
  REQUIRE( emp::view_string_back(view_test, 5) == "test!" );
  REQUIRE( emp::view_string_range(view_test, 11, 15) == "view" );
  REQUIRE( emp::view_string_to(view_test, ' ') == "This" );
  REQUIRE( emp::view_string_to(view_test, ' ', 5) == "is" );

  emp::vector<std::string_view> slice_view = emp::view_slices(view_test, ' ');
  REQUIRE( slice_view.size() == 5 );
  REQUIRE( slice_view[0] == "This" );
  REQUIRE( slice_view[1] == "is" );
  REQUIRE( slice_view[2] == "my" );
  REQUIRE( slice_view[3] == "view" );
  REQUIRE( slice_view[4] == "test!" );


  auto slices = emp::slice(
    "This is a test of a different version of slice.",
    ' '
  );
  REQUIRE(slices.size() == 10);
  REQUIRE(slices[8] == "of");

  slices = emp::slice(
    "This is a test of a different version of slice.",
    ' ',
    101
  );
  REQUIRE(slices.size() == 10);
  REQUIRE(slices[8] == "of");

  slices = emp::slice(
    "This is a test.",
    ' ',
    0
  );
  REQUIRE(slices.size() == 1);
  REQUIRE(slices[0] == "This is a test.");

  slices = emp::slice(
    "This is a test.",
    ' ',
    1
  );
  REQUIRE(slices.size() == 2);
  REQUIRE(slices[0] == "This");
  REQUIRE(slices[1] == "is a test.");

  slices = emp::slice(
    "This is a test.",
    ' ',
    2
  );
  REQUIRE(slices.size() == 3);
  REQUIRE(slices[0] == "This");
  REQUIRE(slices[1] == "is");
  REQUIRE(slices[2] == "a test.");

  // Try other ways of using slice().
  emp::slice(base_string, slices, 's');

  REQUIRE(slices.size() == 5);
  REQUIRE(slices[1] == " i");
  REQUIRE(slices[3] == "t ");


  // Some tests of to_string() function.
  REQUIRE(emp::to_string((int) 1) == "1");
  REQUIRE(emp::to_string("2") == "2");
  REQUIRE(emp::to_string(std::string("3")) == "3");
  REQUIRE(emp::to_string('4') == "4");
  REQUIRE(emp::to_string((int16_t) 5) == "5");
  REQUIRE(emp::to_string((int32_t) 6) == "6");
  REQUIRE(emp::to_string((int64_t) 7) == "7");
  REQUIRE(emp::to_string((uint16_t) 8) == "8");
  REQUIRE(emp::to_string((uint32_t) 9) == "9");
  REQUIRE(emp::to_string((uint64_t) 10) == "10");
  REQUIRE(emp::to_string((size_t) 11) == "11");
  REQUIRE(emp::to_string((long) 12) == "12");
  REQUIRE(emp::to_string((unsigned long) 13) == "13");
  REQUIRE(emp::to_string((float) 14.0) == "14");
  REQUIRE(emp::to_string((float) 14.1) == "14.1");
  REQUIRE(emp::to_string((float) 14.1234) == "14.1234");
  REQUIRE(emp::to_string((double) 15.0) == "15");
  REQUIRE(emp::to_string(16.0) == "16");
  REQUIRE(emp::to_string(emp::vector<size_t>({17,18,19})) == "[ 17 18 19 ]");
  REQUIRE(emp::to_string((char) 32) == " ");
  REQUIRE(emp::to_string((unsigned char) 33) == "!");

  std::string cat_a = "ABC";
  bool cat_b = true;
  char cat_c = '2';
  int cat_d = 3;

  std::string cat_full = emp::to_string(cat_a, cat_b, cat_c, cat_d);

  REQUIRE(cat_full == "ABC123");
  emp::array<int, 3> test_arr({{ 4, 2, 5 }});
  REQUIRE(emp::to_string(test_arr) == "[ 4 2 5 ]");

  // tests adapted from https://stackoverflow.com/questions/5288396/c-ostream-out-manipulation/5289170#5289170
  std::string els[] = { "aap", "noot", "mies" };

  typedef emp::vector<std::string> strings;

  REQUIRE( ""  == emp::join_on(strings(), "") );
  REQUIRE( "" == emp::join_on(strings(), "bla") );
  REQUIRE( "aap" == emp::join_on(strings(els, els + 1), "") );
  REQUIRE( "aap" == emp::join_on(strings(els, els + 1), "#") );
  REQUIRE( "aap" == emp::join_on(strings(els, els + 1), "##") );
  REQUIRE( "aapnoot" == emp::join_on(strings(els, els + 2), "") );
  REQUIRE( "aap#noot" == emp::join_on(strings(els, els + 2), "#") );
  REQUIRE( "aap##noot" == emp::join_on(strings(els, els + 2), "##") );
  REQUIRE( "aapnootmies" == emp::join_on(strings(els, els + 3), "") );
  REQUIRE( "aap#noot#mies" == emp::join_on(strings(els, els + 3), "#") );
  REQUIRE( "aap##noot##mies" == emp::join_on(strings(els, els + 3), "##") );
  REQUIRE( "aap  noot  mies" == emp::join_on(strings(els, els + 3), "  ") );
  REQUIRE( "aapnootmies" == emp::join_on(strings(els, els + 3), "\0"));
  REQUIRE(
    "aapnootmies"
    ==
    emp::join_on(strings(els, els + 3), std::string("\0" , 1).c_str())
  );
  REQUIRE(
    "aapnootmies"
    ==
    emp::join_on(strings(els, els + 3), std::string("\0+", 2).c_str())
  );
  REQUIRE(
    "aap+noot+mies"
    ==
    emp::join_on(strings(els, els + 3), std::string("+\0", 2).c_str())
  );

}



TEST_CASE("Test stats", "[tools]") {
  emp::vector<int> vec1({1,2,1,1,2,3});
  double i1 = 1;
  double i2 = 1;
  double i3 = 1;
  double i4 = 2;

  emp::vector<double*> vec2({&i1, &i2, &i3, &i4});

  std::deque<double> deque1({5,4,3,5,4,6});

  REQUIRE(emp::ShannonEntropy(vec1) == Approx(1.459324));
  REQUIRE(emp::ShannonEntropy(vec2) == Approx(0.81128));
  REQUIRE(emp::ShannonEntropy(deque1) == Approx(1.918648));

  REQUIRE(emp::Variance(vec1) == Approx(0.55539));
  REQUIRE(emp::Variance(vec2) == Approx(0.1875));
  REQUIRE(emp::Variance(deque1) == Approx(0.9166666667));

  REQUIRE(emp::StandardDeviation(vec1) == Approx(0.745245));
  REQUIRE(emp::StandardDeviation(vec2) == Approx(0.433013));
  REQUIRE(emp::StandardDeviation(deque1) == Approx(0.957427));

  REQUIRE(emp::Sum(vec1) == 10);
  REQUIRE(emp::Sum(vec2) == 5);
  REQUIRE(emp::Sum(deque1) == 27);

  REQUIRE(emp::UniqueCount(vec1) == 3);
  REQUIRE(emp::UniqueCount(vec2) == 2);
  REQUIRE(emp::UniqueCount(deque1) == 4);

  REQUIRE(emp::Mean(vec1) == Approx(1.6666666666667));
  REQUIRE(emp::Mean(vec2) == Approx(1.25));
  REQUIRE(emp::Mean(deque1) == 4.5);

  std::function<int(int)> invert = [](int i){return i*-1;};

  REQUIRE(emp::MaxResult(invert, vec1) == -1);
  REQUIRE(emp::MinResult(invert, vec1) == -3);
  REQUIRE(emp::MeanResult(invert, vec1) == Approx(-1.666666667));
  REQUIRE(emp::ApplyFunction(invert, vec1) == emp::vector<int>({-1,-2,-1,-1,-2,-3}));

}

TEST_CASE("Test set utils", "[tools]") {
  std::set<int> s1;
  std::set<int> s2;
  std::set<int> comp_set;
  emp::vector<int> v1;
  emp::vector<int> v2;

  s1.insert(1);
  s1.insert(2);
  s2.insert(2);
  s2.insert(3);
  v1.push_back(1);
  v1.push_back(3);
  v2.push_back(4);
  v2.push_back(1);

  REQUIRE(emp::Has(s1, 1));
  REQUIRE(!emp::Has(s1, 3));

  comp_set.insert(1);
  REQUIRE(emp::difference(s1, s2) == comp_set);
  comp_set.clear();
  comp_set.insert(3);
  REQUIRE(emp::difference(s2, s1) == comp_set);
  comp_set.clear();
  comp_set.insert(2);
  REQUIRE(emp::intersection(s1, s2) == comp_set);
  REQUIRE(emp::intersection(s2, s1) == comp_set);
  comp_set.clear();
  comp_set.insert(2);
  REQUIRE(emp::difference(s1, v1) == comp_set);
  comp_set.clear();
  comp_set.insert(1);
  REQUIRE(emp::intersection(s1, v1) == comp_set);
  REQUIRE(emp::intersection(v1, s1) == comp_set);
  REQUIRE(emp::intersection(v2, v1) == comp_set);
  REQUIRE(emp::intersection(v1, v2) == comp_set);
  comp_set.clear();
  comp_set.insert(4);
  REQUIRE(emp::difference(v2, v1) == comp_set);
  comp_set.clear();
  comp_set.insert(1);
  comp_set.insert(2);
  comp_set.insert(3);
  REQUIRE(emp::set_union(s1, s2) == comp_set);
  REQUIRE(emp::set_union(s2, s1) == comp_set);
  comp_set.clear();
  comp_set.insert(1);
  comp_set.insert(2);
  comp_set.insert(3);
  comp_set.insert(4);
  REQUIRE(emp::set_union(v2, s2) == comp_set);
  REQUIRE(emp::set_union(s2, v2) == comp_set);
  comp_set.clear();
  comp_set.insert(1);
  comp_set.insert(3);
  comp_set.insert(4);
  REQUIRE(emp::set_union(v2, v1) == comp_set);
  REQUIRE(emp::set_union(v1, v2) == comp_set);
  comp_set.clear();
  comp_set.insert(1);
  comp_set.insert(3);
  REQUIRE(emp::symmetric_difference(s1, s2) == comp_set);
  REQUIRE(emp::symmetric_difference(s2, s1) == comp_set);
  comp_set.clear();
  comp_set.insert(4);
  comp_set.insert(3);
  REQUIRE(emp::symmetric_difference(v1, v2) == comp_set);
  REQUIRE(emp::symmetric_difference(v2, v1) == comp_set);
  comp_set.clear();
  comp_set.insert(2);
  comp_set.insert(3);
  REQUIRE(emp::symmetric_difference(v1, s1) == comp_set);
  REQUIRE(emp::symmetric_difference(s1, v1) == comp_set);

}

std::string tt_result;

// Some functions to print a single type and its value
void fun_int(int x) { tt_result = emp::to_string("int:", x); }
void fun_double(double x) { tt_result = emp::to_string("double:", x); }
void fun_string(std::string x) { tt_result = emp::to_string("string:", x); }

// And some silly ways to combine types.
void fun_int_int(int x, int y) { tt_result = emp::to_string(x+y); }
void fun_int_double(int x, double y) { tt_result = emp::to_string(y * (double) x); }
void fun_string_int(std::string x, int y) {
  tt_result = "";
  for (int i=0; i < y; i++) tt_result += x;
}
void fun_5ints(int v, int w, int x, int y, int z) {
  tt_result = emp::to_string(v, '+', w, '+', x, '+', y, '+', z, '=', v+w+x+y+z);
}

TEST_CASE("Test TypeTracker", "[tools]") {
  using tt_t = emp::TypeTracker<int, std::string, double>;   // Setup the tracker type.
  tt_t tt;                                                   // Build the tracker.

  // Add some functions.
  tt.AddFunction( [](int x){ tt_result = emp::to_string("int:", x); } );
  tt.AddFunction(fun_double);
  tt.AddFunction(fun_string);
  tt.AddFunction(fun_int_int);
  tt.AddFunction(fun_int_double);
  tt.AddFunction(fun_string_int);
  tt.AddFunction(fun_5ints);

  emp::TrackedVar tt_int1 = tt.Convert<int>(1);
  emp::TrackedVar tt_int2 = tt.Convert<int>(2);
  emp::TrackedVar tt_int3 = tt.Convert<int>(3);

  emp::TrackedVar tt_str  = tt.Convert<std::string>("FOUR");
  emp::TrackedVar tt_doub = tt.Convert<double>(5.5);

  tt.RunFunction(tt_int1, tt_int2);  // An int and another int should add.
  REQUIRE( tt_result == "3" );

  tt.RunFunction(tt_int3, tt_doub);  // An int and a double should multiply.
  REQUIRE( tt_result == "16.5" );

  tt.RunFunction(tt_doub, tt_int2); // A double and an int is unknown; should leave old result.
  REQUIRE( tt_result == "16.5" );

  tt.RunFunction(tt_str, tt_int3);    // A string an an int should duplicate the string.
  REQUIRE( tt_result == "FOURFOURFOUR" );

  tt.RunFunction(tt_int1, tt_int2, tt_int3, tt_int2, tt_int1);  // Add five ints!
  REQUIRE( tt_result == "1+2+3+2+1=9" );


  // Load all types into a vector and then experiment with them.
  emp::vector<emp::TrackedVar> vars;
  vars.push_back(tt_int1);
  vars.push_back(tt_int2);
  vars.push_back(tt_int3);
  vars.push_back(tt_str);
  vars.push_back(tt_doub);

  emp::vector<std::string> results = { "int:1", "int:2", "int:3", "string:FOUR", "double:5.5" };

  for (size_t i = 0; i < vars.size(); i++) {
    tt(vars[i]);
    REQUIRE(tt_result == results[i]);
  }

  // Make sure TypeTracker can determine consistant IDs.
  REQUIRE( (tt_t::GetID<int,std::string,double>()) == (tt_t::GetTrackedID(tt_int1, tt_str, tt_doub)) );
  REQUIRE( (tt_t::GetComboID<int,std::string,double>()) == (tt_t::GetTrackedComboID(tt_int1, tt_str, tt_doub)) );

  // Make sure a TypeTracker can work with a single type.
  size_t num_args = 0;
  emp::TypeTracker<int> tt1;
  tt1.AddFunction( [&num_args](int){ num_args=1; } );
  tt1.AddFunction( [&num_args](int,int){ num_args=2; } );
  tt1.AddFunction( [&num_args](int,int,int){ num_args=3; } );

  tt_int1 = tt1.Convert<int>(1);
  tt_int2 = tt1.Convert<int>(2);
  tt_int3 = tt1.Convert<int>(3);

  tt1.RunFunction(tt_int1);
  REQUIRE(num_args == 1);
  tt1(tt_int2, tt_int3);
  REQUIRE(num_args == 2);
  tt1(tt_int1, tt_int2, tt_int3);
  REQUIRE(num_args == 3);

}

TEST_CASE("Test vector utils", "[tools]") {
  emp::vector<int> v1({6,2,5,1,3});
  emp::Sort(v1);
  REQUIRE(v1 == emp::vector<int>({1,2,3,5,6}));
  REQUIRE(emp::FindValue(v1, 3) == 2);
  REQUIRE(emp::Sum(v1) == 17);
  REQUIRE(emp::Has(v1, 3));
  REQUIRE(!emp::Has(v1, 4));
  REQUIRE(emp::Product(v1) == 180);
  REQUIRE(emp::Slice(v1,1,3) == emp::vector<int>({2,3}));

  // Test handling vector-of-vectors.
  using vv_int_t = emp::vector< emp::vector< int > >;
  vv_int_t vv = {{1,2,3},{4,5,6},{7,8,9}};
  vv_int_t vv2 = emp::Transpose(vv);
  REQUIRE(vv[0][2] == 3);
  REQUIRE(vv[1][0] == 4);
  REQUIRE(vv2[0][2] == 7);
  REQUIRE(vv2[1][0] == 2);
}

// DEFINE_ATTR(Foo);
// DEFINE_ATTR(Bar);
// DEFINE_ATTR(Bazz);

struct ident_t {
  template <typename T>
  constexpr decltype(auto) operator()(T&& value) const {
    return std::forward<T>(value);
  }
};

constexpr ident_t ident{};

template <typename T>
struct Callable {
  T value;

  constexpr decltype(auto) operator()() & { return value; }
  constexpr decltype(auto) operator()() const & { return value; }
  constexpr decltype(auto) operator()() && { return std::move(value); }
  constexpr decltype(auto) operator()() const && { return std::move(value); }
};
template <typename T>
constexpr Callable<std::decay_t<T>> callable(T&& value) {
  return {std::forward<T>(value)};
}

struct NoCopy {
  int value;
  constexpr NoCopy(int value) : value(value) {}
  constexpr NoCopy(const NoCopy&) = delete;
  constexpr NoCopy(NoCopy&&) = default;

  constexpr NoCopy& operator=(const NoCopy&) = delete;
  constexpr NoCopy& operator=(NoCopy&&) = default;
};
constexpr bool operator==(const NoCopy& a, const NoCopy& b) {
  return a.value == b.value;
}
std::ostream& operator<<(std::ostream& out, const NoCopy& nc) {
  return out << "NoCopy{" << nc.value << "}";
}

struct {
  template <typename I, typename T>
  constexpr auto operator()(I&& init, T&& value) const {
    return std::forward<I>(init) + std::forward<T>(value).Get();
  }

  template <typename I, typename T>
  constexpr auto operator()(const char* name, I&& init, T&& value) const {
    return std::forward<I>(init) + std::forward<T>(value);
  }
} sum;

struct {
  template <typename I, typename A, typename B>
  constexpr auto operator()(I&& init, A&& a, B&& b) const {
    return std::forward<I>(init) +
           (std::forward<A>(a).Get() * std::forward<B>(b).Get());
  }
} dot;

struct {
  template <typename I, typename T>
  constexpr NoCopy operator()(I&& init, T&& value) const {
    return {std::forward<I>(init).value + std::forward<T>(value).Get().value};
  }

  template <typename I, typename T>
  constexpr NoCopy operator()(const char* name, I&& init, T&& value) const {
    return {std::forward<I>(init).value + std::forward<T>(value).value};
  }
} sum_nocopy;

// TEST_CASE("Test Attribute Packs", "[tools]") {
//   using namespace emp::tools;
//   // Test Construction & access
//   CONSTEXPR_REQUIRE_EQ(Foo::CallOrGetAttribute(Foo(6)).Get(), 6);
//   CONSTEXPR_REQUIRE_EQ(Foo::CallOrGetAttribute(Foo(callable(7))).Get(), 7);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrElse(Foo(7), callable(0)), 7);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrElse(Merge(Foo(7), Bar(6)), callable(0)), 7);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrElse(Merge(Bazz(7), Bar(6)), callable(0)), 0);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrGetIn(Bazz(1), Bar(2), Foo(3)), 3);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrGetIn(Bazz(1), Foo(3), Foo(2)), 3);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrGetIn(Foo(3), Bar(2), Bazz(1)), 3);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrGetIn(Foo(3), Bar(2)), 3);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrGetIn(Bar(2), Foo(3)), 3);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrGetIn(Foo(3)), 3);

//   CONSTEXPR_REQUIRE_EQ(Foo::GetOr(Foo(7), 0), 7);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOr(Merge(Foo(7), Bar(6)), 0), 7);
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOr(Merge(Bazz(7), Bar(6)), 0), 0);

//   CONSTEXPR_REQUIRE_EQ(Merge(Foo(5), Bar(6)), Merge(Foo(5), Bar(6)));

//   // Test NoCopy
//   CONSTEXPR_REQUIRE_EQ(Foo::CallOrGetAttribute(Foo(NoCopy{7})).Get(),
//                        NoCopy{7});
//   CONSTEXPR_REQUIRE_EQ(Foo::CallOrGetAttribute(Foo(callable(NoCopy{7}))).Get(),
//                        NoCopy{7});
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrElse(Foo(NoCopy{7}), callable(NoCopy{0})),
//                        NoCopy{7});
//   CONSTEXPR_REQUIRE_EQ(
//     Foo::GetOrElse(Merge(Foo(NoCopy{7}), Bar(NoCopy{6})), callable(NoCopy{7})),
//     NoCopy{7});
//   CONSTEXPR_REQUIRE_EQ(
//     Foo::GetOrElse(Merge(Bazz(NoCopy{7}), Bar(NoCopy{6})), callable(NoCopy{0})),
//     NoCopy{0});

//   CONSTEXPR_REQUIRE_EQ(
//     Foo::GetOrGetIn(Bazz(NoCopy{1}), Bar(NoCopy{2}), Foo(NoCopy{3})),
//     NoCopy{3});
//   CONSTEXPR_REQUIRE_EQ(
//     Foo::GetOrGetIn(Bazz(NoCopy{1}), Foo(NoCopy{3}), Foo(NoCopy{2})),
//     NoCopy{3});
//   CONSTEXPR_REQUIRE_EQ(
//     Foo::GetOrGetIn(Foo(NoCopy{3}), Bar(NoCopy{2}), Bazz(NoCopy{1})),
//     NoCopy{3});
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrGetIn(Foo(NoCopy{3}), Bar(NoCopy{2})),
//                        NoCopy{3});
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrGetIn(Bar(NoCopy{2}), Foo(NoCopy{3})),
//                        NoCopy{3});
//   CONSTEXPR_REQUIRE_EQ(Foo::GetOrGetIn(Foo(NoCopy{3})), NoCopy{3});

//   CONSTEXPR_REQUIRE_EQ(Foo::GetOr(Foo(NoCopy{7}), NoCopy{0}), NoCopy{7});
//   CONSTEXPR_REQUIRE_EQ(
//     Foo::GetOr(Merge(Foo(NoCopy{7}), Bar(NoCopy{6})), NoCopy{0}), NoCopy{7});
//   CONSTEXPR_REQUIRE_EQ(
//     Foo::GetOr(Merge(Bazz(NoCopy{7}), Bar(NoCopy{6})), NoCopy{0}), NoCopy{0});

//   CONSTEXPR_REQUIRE_EQ(Merge(Foo(NoCopy{5}), Bar(NoCopy{6})),
//                        Merge(Foo(NoCopy{5}), Bar(NoCopy{6})));

//   // Test Mapping
//   CONSTEXPR_REQUIRE_EQ(Merge(Foo(ident), Bar(6))(5), Merge(Foo(5), Bar(6)));
//   CONSTEXPR_REQUIRE_EQ(Merge(Foo(ident), Bar(6))(5), Merge(Foo(5), Bar(6)));

//   CONSTEXPR_REQUIRE_EQ(Merge(Bar(6), Foo(ident))(NoCopy{5}),
//                        Merge(Foo(NoCopy{5}), Bar(6)));

//   CONSTEXPR_REQUIRE_EQ(Merge(Bar(NoCopy{6}), Foo(ident))(5),
//                        Merge(Foo(5), Bar(NoCopy{6})));

//   CONSTEXPR_REQUIRE_EQ(Merge(Bar(5), Foo(6)).Reduce(0, sum), 11);
//   CONSTEXPR_REQUIRE_EQ(
//     Merge(Bar(NoCopy{5}), Foo(NoCopy{6})).Reduce(NoCopy{0}, sum_nocopy),
//     NoCopy{11});
//   CONSTEXPR_REQUIRE_EQ(MergeReduce(0, sum, Bar(6), Foo(7)), 6 + 7);
//   // CONSTEXPR_REQUIRE_EQ(MergeReduce(0, [](auto init, auto& a, auto& b) {return
//   // init + a.Get() * b.Get();}, Bar(6) + Foo(7), Bar(11) + Foo(12)),
//   //                      6 + 7);
// }
