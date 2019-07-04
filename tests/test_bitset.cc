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

#include "tools/BitSet.h"
#include "tools/Random.h"

template class emp::BitSet<5>;
TEST_CASE("Test BitSet", "[tools]")
{
  emp::BitSet<10> bs10;
  emp::BitSet<32> bs32;
  emp::BitSet<50> bs50;
  emp::BitSet<64> bs64;
  emp::BitSet<80> bs80;

  bs80[70] = 1;
  emp::BitSet<80> bs80c(bs80);
  bs80 <<= 1;

  for (size_t i = 0; i < 75; i++) {
    emp::BitSet<80> shift_set = bs80 >> i;
    REQUIRE((shift_set.CountOnes() == 1) == (i <= 71));
  }

  REQUIRE(bs10[2] == false);
  bs10.flip(2);
  REQUIRE(bs10[2] == true);

  for (size_t i = 3; i < 8; i++) {REQUIRE(bs10[i] == false);}
  bs10.flip(3, 8);
  for (size_t i = 3; i < 8; i++) {REQUIRE(bs10[i] == true);}

  // Test importing....
  bs10.Import(bs80 >> 70);

  REQUIRE(bs10.GetUInt(0) == 2);

  // Test arbitrary bit retrieval of UInts
  bs80[65] = 1;
  REQUIRE(bs80.GetUIntAtBit(64) == 130);
  REQUIRE(bs80.GetValueAtBit<5>(64) == 2);

  emp::Random rand(1);

  {
    emp::BitSet<1> bs(rand);
    const emp::BitSet<1> bs_orig(bs);
    const size_t num_ones = bs.CountOnes();

    for (int i = -1 - 1; i <= 1 + 1; ++i) {
      for (size_t rep = 0; rep < 1; ++ rep) {
        bs.ROTATE_SELF(i);
        REQUIRE(bs.CountOnes() == num_ones);
      }
      REQUIRE(bs == bs_orig);
    }
  }

  {
    emp::BitSet<2> bs(rand);
    const emp::BitSet<2> bs_orig(bs);
    const size_t num_ones = bs.CountOnes();

    for (int i = -2 - 1; i <= 2 + 1; ++i) {
      for (size_t rep = 0; rep < 2; ++ rep) {
        bs.ROTATE_SELF(i);
        REQUIRE(bs.CountOnes() == num_ones);
      }
      REQUIRE(bs == bs_orig);
    }
  }

  {
    emp::BitSet<3> bs(rand);
    const emp::BitSet<3> bs_orig(bs);
    const size_t num_ones = bs.CountOnes();

    for (int i = -3 - 1; i <= 3 + 1; ++i) {
      for (size_t rep = 0; rep < 3; ++ rep) {
        bs.ROTATE_SELF(i);
        REQUIRE(bs.CountOnes() == num_ones);
      }
      REQUIRE(bs == bs_orig);
    }
  }

  {
    emp::BitSet<10> bs(rand);
    const emp::BitSet<10> bs_orig(bs);
    const size_t num_ones = bs.CountOnes();

    for (int i = -10 - 1; i <= 10 + 1; ++i) {
      for (size_t rep = 0; rep < 10; ++ rep) {
        bs.ROTATE_SELF(i);
        REQUIRE(bs.CountOnes() == num_ones);
      }
      REQUIRE(bs == bs_orig);
    }
  }

  {
    emp::BitSet<32> bs(rand);
    const emp::BitSet<32> bs_orig(bs);
    const size_t num_ones = bs.CountOnes();

    for (int i = -32 - 1; i <= 32 + 1; ++i) {
      for (size_t rep = 0; rep < 32; ++ rep) {
        bs.ROTATE_SELF(i);
        REQUIRE(bs.CountOnes() == num_ones);
      }
      REQUIRE(bs == bs_orig);
    }
  }

  {
    emp::BitSet<50> bs(rand);
    const emp::BitSet<50> bs_orig(bs);
    const size_t num_ones = bs.CountOnes();

    for (int i = -18; i <= 18; ++i) {
      for (size_t rep = 0; rep < 50; ++ rep) {
        bs.ROTATE_SELF(i);
        REQUIRE(bs.CountOnes() == num_ones);
      }
      REQUIRE(bs == bs_orig);
    }

    for (int i = -50 - 1; i <= -32; ++i) {
      for (size_t rep = 0; rep < 50; ++ rep) {
        bs.ROTATE_SELF(i);
        REQUIRE(bs.CountOnes() == num_ones);
      }
      REQUIRE(bs == bs_orig);
    }

    for (int i = -50 - 1; i <= 50 + 1; ++i) {
      std::cout << i << std::endl;
      for (size_t rep = 0; rep < 50; ++ rep) {
        bs.ROTATE_SELF(i);
        REQUIRE(bs.CountOnes() == num_ones);
      }
      REQUIRE(bs == bs_orig);
    }
  }

}
