#ifndef EMP_BITSET_UTILS_H
#define EMP_BITSET_UTILS_H

//////////////////////////////////////////////////////////////////////////////
//
//  This file contains a set of simple functions to manipulate bitsets.
//

#include <bitset>

#include "functions.h"

namespace emp {

  // Returns the position of the first set (one) bit or a -1 if none exist.
  template <size_t BITS>
  int find_bit(const std::bitset<BITS> & in) noexcept {
    int offset = 0;
    unsigned long long tmp_bits = 0ULL;
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

  /*
  template <size_t BITS>
  constexpr std::bitset<BITS> mask_bit(unsigned int id) {
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
};

#endif
