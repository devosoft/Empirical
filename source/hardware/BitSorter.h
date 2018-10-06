/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  BitSorter.h
 *  @brief A quick series of comparisons intended for sorting bits.
 */

#ifndef EMP_BIT_SORTER_H
#define EMP_BIT_SORTER_H

#include <string>

#include "../base/vector.h"

namespace emp {

  class BitSorter {
  private:
    using bits_t = size_t;            // Type used to represent pairs if posisions.
    emp::vector<bits_t> compare_set;  // All pairs of positions, in order.

  public:
    BitSorter() { ; }
    ~BitSorter() { ; }

    void AddCompare(size_t id1, size_t id2) {
      emp_assert(id1 < sizeof(bits_t));
      emp_assert(id2 < sizeof(bits_t));
      compare_set.push_back( (1 << id1) + (1 << id2) );
    }

    static bits_t RunCompare(bits_t values, bits_t comparator) {
      // If bits are the same, no change!
      if ( ((values & comparator) == 0)                     // Bits are both 0... no change!
         || ((~values & comparator) == 0)                   // Bits are both 1... no change!
         || ((values & comparator) < (~values & comparator))  // Bits are already in correct (0 - 1) order!
         ) {
        return values;
      }
      return values ^ comparator;  // Swap bits!
    }

    bits_t Sort(bits_t values) {
      for (bits_t c : compare_set) values = RunCompare(values, c);
      return values;
    }

    static std::string ToString(bits_t values, size_t num_bits=16) {
      std::string out_str;
      for (size_t id = num_bits; id > 0; id--) {
        if (values & 1 << (id-1)) out_str += "1";
        else out_str += "0";
      }
      return out_str;
    }
  };

}

#endif