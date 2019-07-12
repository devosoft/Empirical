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

  {

    emp::Random rand(1);

    emp::array<emp::BitSet<32>,1> a1 = {
      emp::BitSet<32>(rand)
    };
    emp::array<emp::BitSet<32>,1> b1 = {
      emp::BitSet<32>(rand)
    };

    emp::StreakMetric<32> streak;
    emp::AntiMod<emp::StreakMetric<32>> anti_streak;
    emp::HammingMetric<32> hamming;
    emp::SlideMod<emp::HammingMetric<32>> slide_hamming;

    emp::DimMod<emp::StreakMetric<32>, 1> d_streak1;
    REQUIRE(d_streak1.width == streak.width);

    emp::DimMod<emp::AntiMod<emp::StreakMetric<32>>, 1> d_anti_streak1;
    REQUIRE(d_anti_streak1.width == anti_streak.width);

    emp::DimMod<emp::HammingMetric<32>, 1> d_hamming1;
    REQUIRE(d_hamming1.width == hamming.width);

    emp::DimMod<emp::SlideMod<emp::HammingMetric<32>>, 1> d_slide_hamming1;
    REQUIRE(d_slide_hamming1.width == slide_hamming.width);

    REQUIRE(streak(a1[0], b1[0]) == d_streak1(a1, b1));
    REQUIRE(anti_streak(a1[0], b1[0]) == d_anti_streak1(a1, b1));
    REQUIRE(hamming(a1[0], b1[0]) == d_hamming1(a1, b1));
    REQUIRE(slide_hamming(a1[0], b1[0]) == d_slide_hamming1(a1, b1));

    emp::array<emp::BitSet<32>,3> a3 = {
      emp::BitSet<32>(rand),
      emp::BitSet<32>(rand),
      emp::BitSet<32>(rand)
    };
    emp::array<emp::BitSet<32>,3> b3 = {
      emp::BitSet<32>(rand),
      emp::BitSet<32>(rand),
      emp::BitSet<32>(rand)
    };

    emp::DimMod<emp::StreakMetric<32>, 3> d_streak3;
    REQUIRE(d_streak3.width == streak.width * 3);

    emp::DimMod<emp::AntiMod<emp::StreakMetric<32>>, 3> d_anti_streak3;
    REQUIRE(d_anti_streak3.width == anti_streak.width * 3);

    emp::DimMod<emp::HammingMetric<32>, 3> d_hamming3;
    REQUIRE(d_hamming3.width == hamming.width * 3);

    emp::DimMod<emp::SlideMod<emp::HammingMetric<32>>, 3> d_slide_hamming3;
    REQUIRE(d_slide_hamming3.width == slide_hamming.width * 3);


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

}
