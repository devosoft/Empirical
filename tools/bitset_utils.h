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
  template <unsigned long BITS>
  int find_bit(const std::bitset<BITS> & in) {
    int offset = 0;
    unsigned long long tmp_bits = 0ULL;
    while (offset < BITS && ((tmp_bits = (in >> offset).to_ullong()) == 0ULL)) {
      offset += 64;
    }
    return tmp_bits ? find_bit(tmp_bits) + offset : -1;
  }

};

#endif
