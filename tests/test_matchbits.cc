#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch.hpp"

#include <string>
#include "tools/BitSet.h"
#include "tools/MatchBin.h"
#include "tools/matchbin_utils.h"
#include "tools/math.h"
#include <ratio>
#include <limits>
#include <cmath>

TEST_CASE("Test MatchBin", "[tools]")
{
  {
  emp::BitSet<32> bs0;
  bs0.SetUInt(0, pow((size_t)2, (size_t)32)-2);
  emp::BitSet<32> bs1;
  bs1.SetUInt(0,1);
  bs0+=bs1;
  REQUIRE (bs0.GetUInt(0) == 4294967295);
  REQUIRE ((bs0+bs1).GetUInt(0) == 0);
  REQUIRE ((bs0+bs0).GetUInt(0) == 4294967294);

  emp::BitSet<8> bs2;
  bs2.SetUInt(0,pow(2, 8)-1);
  emp::BitSet<8> bs3;
  bs3.SetUInt(0, 1);
  REQUIRE((bs2+bs3).GetUInt(0) == 0);

  emp::BitSet<64> bs4;
  bs4.SetUInt(0, pow((size_t)2, (size_t)32)-2);
  bs4.SetUInt(1, pow((size_t)2, (size_t)32)-1);
  emp::BitSet<64> bs5;
  bs5.SetUInt(0, 1);
  bs4+=bs5;
  REQUIRE(bs4.GetUInt(0) == pow((size_t)2, (size_t)32)-1);
  REQUIRE(bs4.GetUInt(1) == pow((size_t)2, (size_t)32)-1);
  bs4+=bs5;
  REQUIRE(bs4.GetUInt(0) == 0);
  REQUIRE(bs4.GetUInt(1) == 0);
  }

  {
  emp::BitSet<32> bs0;
  bs0.SetUInt(0, 1);
  emp::BitSet<32> bs1;
  bs1.SetUInt(0, 1);
  bs0 = bs0 - bs1;
  REQUIRE (bs0.GetUInt(0) == 0);
  REQUIRE ((bs0-bs1).GetUInt(0) == pow((size_t)2, (size_t)32)-1);

  emp::BitSet<8> bs2;
  bs2.SetUInt(0, 1);
  emp::BitSet<8> bs3;
  bs3.SetUInt(0, 1);

  bs2-=bs3;
  REQUIRE (bs2.GetUInt(0) == 0);
  REQUIRE((bs2-bs3).GetUInt(0) == pow(2,8)-1);

  emp::BitSet<64> bs4;
  bs4.SetUInt(0, 1);
  bs4.SetUInt(1, 0);

  emp::BitSet<64> bs5;
  bs5.SetUInt(0, 1);

  bs4-=bs5;
  REQUIRE(bs4.GetUInt(0) == 0);
  REQUIRE(bs4.GetUInt(1) == 0);

  bs4-=bs5;
  REQUIRE(bs4.GetUInt(0) == pow((size_t)2, (size_t)32)-1);
  REQUIRE(bs4.GetUInt(1) == pow((size_t)2, (size_t)32)-1);
  bs4 = bs4 - bs5;
  REQUIRE(bs4.GetUInt(0) == pow((size_t)2, (size_t)32)-2);
  REQUIRE(bs4.GetUInt(1) == pow((size_t)2, (size_t)32)-1);
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

    emp::BitSet<3> dup(bs_010);
    slide_streak(bs_010, bs_000);
    REQUIRE(bs_010 == dup);

    // slide takes a reference and rotates the BitSet around
    // (and back into place) so we need to make a copy if we have
    // equivalent left and right operands to operator()
    REQUIRE(
      slide_streak(bs_100, dup=bs_100)
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
      slide_hamming(bs_100, dup=bs_100)
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
      ==
      streak(bs_111, bs_111)
    );

    REQUIRE(
      anti_streak(bs_011, bs_000)
      ==
      streak(bs_011, bs_111)
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

    emp::BitSet<3> dup;

    emp::SlideMod<emp::StreakMetric<3>> slide_streak;
    emp::SlideMod<emp::AntiMod<emp::StreakMetric<3>>> slide_anti_streak;
    emp::SlideMod<emp::HammingMetric<3>> slide_hamming;
    emp::SlideMod<emp::AntiMod<emp::HammingMetric<3>>> slide_anti_hamming;

    // anti should be equivalent to 0 matching with 1 and vice versa
    // instead of 0 matching with 0 and 1 matching with 1

    REQUIRE(
      slide_streak(bs_000, bs_111)
      ==
      slide_anti_streak(dup=bs_111, bs_111)
    );
    // slide takes a reference and rotates the BitSet around
    // (and back into place) so we need to make a copy if we have
    // equivalent left and right operands to operator()

    REQUIRE(
      slide_streak(bs_011, bs_000)
      ==
      slide_anti_streak(bs_011, bs_111)
    );

    REQUIRE(
      slide_hamming(bs_000, bs_111)
      -
      slide_anti_hamming(dup=bs_111, bs_111)
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

  {

    emp::BitSet<3> bs0{0,0,0};
    REQUIRE(bs0.GetDouble() == 0.0);

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

    emp::BitSet<64> bs6;
    bs6.SetUInt64(0, 1789156816848ULL);
    REQUIRE(bs6.GetDouble() == 1789156816848ULL);

    emp::BitSet<65> bs7;
    bs7.SetUInt64(0, 1789156816848ULL);
    bs7.Set(64);
    REQUIRE(bs7.GetDouble() == 1789156816848.0 + emp::Pow2(64.0));

    emp::BitSet<1027> bs8;
    bs8.Set(1026);
    REQUIRE(std::isinf(bs8.GetDouble()));

  }

}
