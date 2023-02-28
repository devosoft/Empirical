/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2022.
 *
 *  @file bitset_utils.hpp
 *  @brief A set of simple functions to manipulate bitsets.
 *  @note Status: BETA
 */

#ifndef EMP_BITS_BITSET_UTILS_HPP_INCLUDE
#define EMP_BITS_BITSET_UTILS_HPP_INCLUDE

#include <sstream>
#include <string>

#include "../base/Ptr.hpp"

namespace emp {

  /// @brief Use size_t as the default bits field type.
  using bits_field_t = size_t;

  /// @brief Track the number of bits in a single bit field.
  static constexpr size_t NUM_FIELD_BITS = sizeof(bits_field_t)*8;

  /// @brief Convert a bit count to the number of fields needed to store them.
  [[nodiscard]] constexpr size_t NumBitFields(size_t num_bits) noexcept {
    return num_bits ? (1 + ((num_bits - 1) / NUM_FIELD_BITS)) : 0;
  }

  /// @brief Convert a single bit field to a string.
  /// @param field A single bit field to convert to a string.
  [[nodiscard]] std::string BitFieldToString(bits_field_t field) {
    std::stringstream ss;
    ss << '[' << std::hex << field << ']';
    return ss.str();
  }

  /// @brief Convert a series of bit fields to a string.
  /// @param field A single bit field to convert to a string.
  [[nodiscard]] std::string BitFieldsToString(emp::Ptr<bits_field_t> bits, size_t count) {
    std::stringstream ss;
    for (size_t i = 0; i < count; ++i) {
      if (i) ss << ' ';
      ss << BitFieldToString(bits[i]);
    }
    return ss.str();
  }

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

  /// Count the number of bits in an unsigned integer.
  template <typename T>
  [[nodiscard]] inline constexpr size_t count_bits(T val) {
    static_assert( std::is_unsigned_v<T>, "Bit manipulation requires unsigned values." );
    constexpr size_t num_bytes = sizeof(T);
    static_assert(num_bytes <= 8, "count_bits() requires 8 or fewer bytes.");

    size_t out_ones = ByteCount[ val & 0xFF ];
    if constexpr (num_bytes > 1) {
      out_ones += ByteCount[ (val >>  8) & 0xFF ];
    }
    if constexpr (num_bytes > 2) {
      out_ones += ByteCount[ (val >> 24) & 0xFF ] +
                  ByteCount[ (val >> 16) & 0xFF ];
    }
    if constexpr (num_bytes > 4) {
      out_ones += ByteCount[  val >> 56         ] +
                  ByteCount[ (val >> 48) & 0xFF ] +
                  ByteCount[ (val >> 40) & 0xFF ] +
                  ByteCount[ (val >> 32) & 0xFF ];
    }
    return out_ones;
  }


  /// Return the position of the first one bit
  template <typename T>
  [[nodiscard]] inline constexpr size_t find_bit(T val) {
    static_assert( std::is_unsigned_v<T>, "Bit manipulation requires unsigned values." );
    return count_bits( (~val) & (val-1) );
  }

  /// Return the position of the first one bit AND REMOVE IT.
  template <typename T>
  inline size_t pop_bit(T & val) {
    static_assert( std::is_unsigned_v<T>, "Bit manipulation requires unsigned values." );
    const size_t pos = find_bit(val);
    val &= ~(1 << pos);
    return pos;
  }

  /// Quick bit-mask generator for low bits.
  template <typename TYPE=size_t>
  [[nodiscard]] static constexpr TYPE MaskLow(std::size_t num_bits) {
    static_assert( std::is_unsigned_v<TYPE>, "Bit manipulation requires unsigned values." );
    return (num_bits == 8*sizeof(TYPE)) ? ((TYPE)-1) : ((((TYPE)1) << num_bits) - 1);
  }

  /// Quick bit-mask generator for high bits.
  template <typename TYPE=size_t>
  [[nodiscard]] static constexpr TYPE MaskHigh(std::size_t num_bits) {
    static_assert( std::is_unsigned_v<TYPE>, "Bit manipulation requires unsigned values." );
    return MaskLow<TYPE>(num_bits) << (8*sizeof(TYPE)-num_bits);
  }

  template <typename TYPE=size_t>
  [[nodiscard]] static constexpr TYPE MaskUsed(TYPE val) {
    static_assert( std::is_unsigned_v<TYPE>, "Bit manipulation requires unsigned values." );
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

  template <typename T>
  [[nodiscard]] constexpr T ReverseBits(T in) {
    constexpr size_t num_bytes = sizeof(T);

    static_assert( std::is_unsigned_v<T>, "Bit manipulation requires unsigned values." );
    static_assert( num_bytes == 1 || num_bytes == 2 || num_bytes == 4 || num_bytes == 8,
                   "ReverseBits() currently requires 1, 2, 4, or 8-byte values." );

    if constexpr (num_bytes == 1) {
      in = static_cast<T>( (in & 0xF0) >> 4 | (in & 0x0F) << 4 );
      in = static_cast<T>( (in & 0xCC) >> 2 | (in & 0x33) << 2 );
      in = static_cast<T>( (in & 0xAA) >> 1 | (in & 0x55) << 1 );
    }
    else if constexpr (num_bytes == 2) {
      in = static_cast<T>( (in & 0xFF00) >> 8 | (in & 0x00FF) << 8 );
      in = static_cast<T>( (in & 0xF0F0) >> 4 | (in & 0x0F0F) << 4 );
      in = static_cast<T>( (in & 0xCCCC) >> 2 | (in & 0x3333) << 2 );
      in = static_cast<T>( (in & 0xAAAA) >> 1 | (in & 0x5555) << 1 );
    }
    else if constexpr (num_bytes == 4) {
      in = static_cast<T>( (in & 0xFFFF0000) >> 16 | (in & 0x0000FFFF) << 16 );
      in = static_cast<T>( (in & 0xFF00FF00) >> 8  | (in & 0x00FF00FF) << 8 );
      in = static_cast<T>( (in & 0xF0F0F0F0) >> 4  | (in & 0x0F0F0F0F) << 4 );
      in = static_cast<T>( (in & 0xCCCCCCCC) >> 2  | (in & 0x33333333) << 2 );
      in = static_cast<T>( (in & 0xAAAAAAAA) >> 1  | (in & 0x55555555) << 1 );
    }
    else /* if constexpr (num_bytes == 8) */ {
      in = static_cast<T>( (in & 0xFFFFFFFF00000000) >> 32 | (in & 0x00000000FFFFFFFF) << 32 );
      in = static_cast<T>( (in & 0xFFFF0000FFFF0000) >> 16 | (in & 0x0000FFFF0000FFFF) << 16 );
      in = static_cast<T>( (in & 0xFF00FF00FF00FF00) >> 8  | (in & 0x00FF00FF00FF00FF) << 8 );
      in = static_cast<T>( (in & 0xF0F0F0F0F0F0F0F0) >> 4  | (in & 0x0F0F0F0F0F0F0F0F) << 4 );
      in = static_cast<T>( (in & 0xCCCCCCCCCCCCCCCC) >> 2  | (in & 0x3333333333333333) << 2 );
      in = static_cast<T>( (in & 0xAAAAAAAAAAAAAAAA) >> 1  | (in & 0x5555555555555555) << 1 );
    }

    return in;
  }

  // Rotate all bits to the left (looping around) in a provided field.
  template <typename T>
  [[nodiscard]] constexpr T RotateBitsLeft(
    T in,
    size_t rotate_size = 1
  ) {
    static_assert( std::is_unsigned_v<T>, "Bit manipulation requires unsigned values." );
    constexpr size_t FIELD_BITS = sizeof(T) * 8;
    rotate_size %= FIELD_BITS;       // Make sure rotate is in range.
    return (in << rotate_size) |
           (in >> (FIELD_BITS - rotate_size));
  }

  // Rotate lowest "bit_count" bits to the left (looping around) in a provided field.
  template <typename T>
  [[nodiscard]] constexpr T RotateBitsLeft(
    T in,
    size_t rotate_size,
    size_t bit_count
  ) {
    static_assert( std::is_unsigned_v<T>, "Bit manipulation requires unsigned values." );
    constexpr size_t FIELD_BITS = sizeof(T) * 8;
    emp_assert(bit_count <= FIELD_BITS, "Cannot have more bits than can fit in field.");
    rotate_size %= bit_count; // Make sure rotate is in range.
    const T out = (in << rotate_size) | (in >> (bit_count - rotate_size));
    return out & MaskLow<T>(bit_count);  // Zero out excess bits.
  }

  // Rotate all bits to the left (looping around) in a provided field.
  template <typename T>
  [[nodiscard]] constexpr T RotateBitsRight(
    T in,
    size_t rotate_size = 1
  ) {
    static_assert( std::is_unsigned_v<T>, "Bit manipulation requires unsigned values." );
    constexpr size_t FIELD_BITS = sizeof(T) * 8;
    rotate_size %= FIELD_BITS;       // Make sure rotate is in range.
    return (in >> rotate_size) |
           (in << (FIELD_BITS - rotate_size));
  }

  // Rotate lowest "bit_count" bits to the left (looping around) in a provided field.
  template <typename T>
  [[nodiscard]] constexpr T RotateBitsRight(
    T in,
    size_t rotate_size,
    size_t bit_count
  ) {
    static_assert( std::is_unsigned_v<T>, "Bit manipulation requires unsigned values." );
    constexpr size_t FIELD_BITS = sizeof(T) * 8;
    emp_assert(bit_count <= FIELD_BITS, "Cannot have more bits than can fit in field.");
    rotate_size %= bit_count; // Make sure rotate is in range.
    const T out = (in >> rotate_size) | (in << (bit_count - rotate_size));
    return out & MaskLow<T>(bit_count);  // Zero out excess bits.
  }

  /// Count the number of bits ('0' or '1') found in a string.
  size_t CountBits(const std::string & bitstring) {
    return std::count_if(
      bitstring.begin(),
      bitstring.end(),
      [](char i) { return i == '0' || i == '1'; }
    );
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
