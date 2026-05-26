/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2021
*/
/**
 *  @file
 */

#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/bits/bitset_utils.hpp"

TEST_CASE("Test bitset_utils", "[bits]")
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

  REQUIRE(emp::MaskUsed<size_t>(5) == 7);
  REQUIRE(emp::MaskUsed<size_t>(15) == 15);
  REQUIRE(emp::MaskUsed<uint32_t>(30000) == 32767);
  REQUIRE(emp::MaskUsed<uint64_t>(40000000000) == 0xfffffffff);
  REQUIRE(emp::MaskUsed<uint64_t>(5000000000000) == 0x7ffffffffff);

  unsigned long long large = 0x8000000000000000;
  REQUIRE(emp::MaskHigh<unsigned long long>(1) == large);

  // MaskHigh(0) must return 0 without UB.
  REQUIRE(emp::MaskHigh<uint64_t>(0) == 0);
  REQUIRE(emp::MaskHigh<uint32_t>(0) == 0);
}

TEST_CASE("BitMask", "[bits]")
{
  // All low bits set
  REQUIRE(emp::BitMask<uint8_t>(4, 0) == 0x0F);
  REQUIRE(emp::BitMask<uint8_t>(8, 0) == 0xFF);
  REQUIRE(emp::BitMask<uint8_t>(0, 0) == 0x00);

  // Shifted masks
  REQUIRE(emp::BitMask<uint8_t>(4, 4) == 0xF0);   // assert would have fired with < MAX_BITS
  REQUIRE(emp::BitMask<uint8_t>(1, 7) == 0x80);
  REQUIRE(emp::BitMask<uint8_t>(3, 2) == 0x1C);

  // 64-bit variants
  REQUIRE(emp::BitMask<uint64_t>(64, 0) == ~uint64_t{0});
  REQUIRE(emp::BitMask<uint64_t>(32, 0) == 0xFFFF'FFFF);
  REQUIRE(emp::BitMask<uint64_t>(32, 32) == 0xFFFF'FFFF'0000'0000);

  // zero mask_size always returns 0 regardless of offset
  REQUIRE(emp::BitMask<uint64_t>(0, 0) == 0);
  REQUIRE(emp::BitMask<uint64_t>(0, 10) == 0);
}

TEST_CASE("pop_bit 64-bit", "[bits]")
{
  // Bug fix: was ~(1 << pos) which is UB for pos >= 32.
  uint64_t val = uint64_t{1} << 33;   // bit 33 set
  REQUIRE(emp::pop_bit(val) == 33);
  REQUIRE(val == 0);

  uint64_t val2 = uint64_t{1} << 63;  // highest bit
  REQUIRE(emp::pop_bit(val2) == 63);
  REQUIRE(val2 == 0);
}

TEST_CASE("RotateBitsLeft and RotateBitsRight", "[bits]")
{
  // Full-field rotations.
  REQUIRE(emp::RotateBitsLeft<uint8_t>(0b00000001, 1) == 0b00000010);
  REQUIRE(emp::RotateBitsLeft<uint8_t>(0b10000000, 1) == 0b00000001);  // wraps
  REQUIRE(emp::RotateBitsLeft<uint8_t>(0b10110010, 3) == 0b10010101);

  REQUIRE(emp::RotateBitsRight<uint8_t>(0b00000010, 1) == 0b00000001);
  REQUIRE(emp::RotateBitsRight<uint8_t>(0b00000001, 1) == 0b10000000);  // wraps
  REQUIRE(emp::RotateBitsRight<uint8_t>(0b10110010, 3) == 0b01010110);

  // Zero rotation must return the original value (not UB).
  REQUIRE(emp::RotateBitsLeft<uint64_t>(0xDEAD'BEEF'CAFE'F00D, 0)  == 0xDEAD'BEEF'CAFE'F00D);
  REQUIRE(emp::RotateBitsRight<uint64_t>(0xDEAD'BEEF'CAFE'F00D, 0) == 0xDEAD'BEEF'CAFE'F00D);
  // Full-width rotation is equivalent to zero rotation.
  REQUIRE(emp::RotateBitsLeft<uint64_t>(0xDEAD'BEEF'CAFE'F00D, 64) == 0xDEAD'BEEF'CAFE'F00D);
  REQUIRE(emp::RotateBitsRight<uint64_t>(0xDEAD'BEEF'CAFE'F00D, 64)== 0xDEAD'BEEF'CAFE'F00D);

  // Partial-field rotations (bit_count variant).
  // Rotate the low 4 bits of 0b11001010 left by 1:
  //   low 4 = 1010 → rotated → 0101; high 4 unchanged = 1100
  REQUIRE(emp::RotateBitsLeft<uint8_t>(0b11001010, 1, 4) == 0b00000101);
  REQUIRE(emp::RotateBitsRight<uint8_t>(0b11000101, 1, 4) == 0b00001010);

  // Zero rotation in partial-field variant.
  REQUIRE(emp::RotateBitsLeft<uint64_t>(0xFF, 0, 64)  == 0xFF);
  REQUIRE(emp::RotateBitsRight<uint64_t>(0xFF, 0, 64) == 0xFF);
}
