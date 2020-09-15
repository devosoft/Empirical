#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "tools/bitset_utils.h"

#include <sstream>

TEST_CASE("Test bitset_utils", "[tools]")
{
  uint32_t three = emp::UIntMaskFirst<2>();
  REQUIRE(three == 3);
  uint32_t zero = emp::UIntMaskFirst<0>();
  REQUIRE(emp::count_bits(zero) == 0);
  REQUIRE(emp::find_bit(zero) == 32); // no 1 bit in 0
  uint64_t one = 2;
  REQUIRE(emp::count_bits(one) == 1);
  REQUIRE(emp::find_bit(one) == 1);
  
  uint64_t two = 2;
  REQUIRE(emp::pop_bit(two) == 1);
  REQUIRE(emp::count_bits(two) == 0);
  uint32_t four = 4;
  REQUIRE(emp::count_bits(four) == 1);
  REQUIRE(emp::pop_bit(four) == 2);
  REQUIRE(emp::count_bits(four) == 0);
}