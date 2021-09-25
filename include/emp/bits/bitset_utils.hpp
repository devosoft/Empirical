/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2020.
 *
 *  @file bitset_utils.hpp
 *  @brief A set of simple functions to manipulate bitsets.
 *  @note Status: BETA
 */

#ifndef EMP_BITS_BITSET_UTILS_HPP_INCLUDE
#define EMP_BITS_BITSET_UTILS_HPP_INCLUDE

namespace emp {

  /// Create a series of a specified number of ones (at compile time) in a uint.
  template <int NUM_BITS>
  constexpr uint32_t UIntMaskFirst() { return (UIntMaskFirst<NUM_BITS-1>() << 1) | 1; }

  /// Create an empty bit mask (all zeros)
  template <>
  constexpr uint32_t UIntMaskFirst<0>() { return 0; }

  /// How many bits are set to one in each possible byte?
  constexpr size_t ByteCount[256] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
  };

  /// Count the number of bits in a 64-bit unsigned integer.
  inline constexpr size_t count_bits(uint64_t val) {
    return
      ByteCount[  val >> 56         ] +
      ByteCount[ (val >> 48) & 0xFF ] +
      ByteCount[ (val >> 40) & 0xFF ] +
      ByteCount[ (val >> 32) & 0xFF ] +
      ByteCount[ (val >> 24) & 0xFF ] +
      ByteCount[ (val >> 16) & 0xFF ] +
      ByteCount[ (val >>  8) & 0xFF ] +
      ByteCount[  val        & 0xFF ];
  }

  // /// Count the number of bits in a 32-bit unsigned integer.
  // inline constexpr size_t count_bits(uint32_t val) {
  //   return
  //     ByteCount[  val >> 24         ] +
  //     ByteCount[ (val >> 16) & 0xFF ] +
  //     ByteCount[ (val >>  8) & 0xFF ] +
  //     ByteCount[  val        & 0xFF ];
  // }

  /// Return the position of the first one bit
  template <typename T>
  inline constexpr size_t find_bit(T val) { return count_bits( (~val) & (val-1) ); }

  /// Return the position of the first one bit AND REMOVE IT.
  template <typename T>
  inline size_t pop_bit(T & val) {
    const size_t pos = find_bit(val);
    val &= ~(1 << pos);
    return pos;
  }

  /// A compile-time bit counter.
  template <typename TYPE>
  static constexpr int CountOnes(TYPE x) { return x == 0 ? 0 : (CountOnes(x/2) + (x&1)); }

  /// Quick bit-mask generator for low bits.
  template <typename TYPE>
  static constexpr TYPE MaskLow(std::size_t num_bits) {
    return (num_bits == 8*sizeof(TYPE)) ? ((TYPE)-1) : ((((TYPE)1) << num_bits) - 1);
  }

  /// Quick bit-mask generator for high bits.
  template <typename TYPE>
  static constexpr TYPE MaskHigh(std::size_t num_bits) {
    return MaskLow<TYPE>(num_bits) << (8*sizeof(TYPE)-num_bits);
  }

  template <typename TYPE>
  static constexpr TYPE MaskUsed(TYPE val) {
    size_t shift = 1;
    TYPE last = 0;
    while (val != last) {     // While the shift is making progress...
      last = val;             // Backup current value as 'last'
      val |= (val >> shift);  // Copy 1's over to the shifted position of val.
      shift <<= 1;            // Double the size of the shift for the next loop.
      if (shift == 0) return 0;
    }
    return val;
  }

  /*
  // Returns the position of the first set (one) bit or a -1 if none exist.
  template <size_t BITS>
  int find_bit(const std::bitset<BITS> & in) {
    int offset = 0;
    uint64_t tmp_bits = 0ULL;
    while (offset < BITS && ((tmp_bits = (in >> offset).to_ullong()) == 0ULL)) {
      offset += 64;
    }
    return tmp_bits ? find_bit(tmp_bits) + offset : -1;
  }

  template <size_t BITS1, size_t BITS2>
  std::bitset<BITS1+BITS2> concat_bits(std::bitset<BITS1> in1, std::bitset<BITS2> in2) {
    constexpr int BITS_OUT = BITS1 + BITS2;  // How many bits are in the output?
    constexpr int FULL_ULLS1 = BITS1 >> 6;   // How many 64-bit groups of bits fit into in1?
    constexpr int FULL_ULLS2 = BITS2 >> 6;   // How many 64-bit groups of bits fit into in2?
    // constexpr int EXTRA_BITS1 = BITS1 & 63;  // How many bits are leftover in BITS1?
    constexpr int EXTRA_BITS2 = BITS2 & 63;  // How many bits are leftover in BITS2?
    std::bitset<BITS_OUT> out_bits;

    // Copy over in1...
    for (int i = FULL_ULLS1; i >= 0; --i) {
      out_bits <<= 64;                          // Make room for the next group of bits.
      out_bits |= (in1 >> (i*64)).to_ullong();  // Put them in place.
    }
  }

  template <size_t BITS>
  constexpr std::bitset<BITS> mask_bit(uint32_t id) {
    return (std::bitset<BITS>(1)) << id;
  }

  template <size_t BITS>
  constexpr std::bitset<BITS> mask_pattern(size_t start, size_t step, size_t end) {
    return mask_bit<BITS>(start) | std::bitset<BITS>(mask_pattern<BITS*(start<end)>(start+step,step,end));
  }

  template <>
  constexpr size_t mask_pattern<0>(size_t start, size_t step, size_t end) {
    return 0;
  }
  */
}

#endif // #ifndef EMP_BITS_BITSET_UTILS_HPP_INCLUDE
