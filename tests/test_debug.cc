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
#include <string>
#include <deque>
#include <algorithm>
#include <limits>
#include <numeric>
#include <climits>
#include <unordered_set>
#include <ratio>

#include "data/DataNode.h"

#include "tools/Binomial.h"
#include "tools/BitMatrix.h"
#include "tools/BitSet.h"
#include "tools/BitVector.h"
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


template class emp::BitSet<5>;
TEST_CASE("Test BitSet", "[tools]")
{

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

  bs4.Print();
  std::cout << std::endl;

  emp::BitSet<64> bs5;
  bs5.SetUInt(0, 1);

  bs5.Print();
  std::cout << std::endl;

  bs4-=bs5;
  REQUIRE(bs4.GetUInt(0) == 0);
  REQUIRE(bs4.GetUInt(1) == 0);

  bs4-=bs5;
  REQUIRE(bs4.GetUInt(0) == std::numeric_limits<uint32_t>::max());
  REQUIRE(bs4.GetUInt(1) == std::numeric_limits<uint32_t>::max());

  bs4.Print();
  std::cout << std::endl;

  emp::BitSet<64> bs6(bs4 - bs5);
  bs6.Print();
  std::cout << std::endl;

  REQUIRE(bs6.GetUInt(0) == std::numeric_limits<uint32_t>::max() - 1);
  REQUIRE(bs6.GetUInt(1) == std::numeric_limits<uint32_t>::max());

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

  std::cout << "============" << std::endl;
  bs1.Print(); std::cout << std::endl;
  bs2.Print(); std::cout << std::endl;
  (bs1 - bs2).Print();  std::cout << std::endl;
  std::cout << "============" << std::endl;

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

}
