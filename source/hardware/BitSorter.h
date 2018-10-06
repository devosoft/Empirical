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
#include "../tools/bitset_utils.h"
#include "../tools/vector_utils.h"

namespace emp {

  class BitSorter {
  private:
    using bits_t = uint32_t;            // Type used to represent pairs if posisions.
    emp::vector<bits_t> compare_set;    // All pairs of positions, in order.

  public:
    BitSorter() { ; }
    ~BitSorter() { ; }

    size_t GetSize() const { return compare_set.size(); }

    size_t CalcDepth(size_t num_bits = 16) const {
      emp::vector<size_t> cur_depth(num_bits, 0);
      for (bits_t c : compare_set) {
        size_t pos1 = pop_bit(c);
        size_t pos2 = find_bit(c);
        emp_assert(pos1 < num_bits, pos1, num_bits);
        emp_assert(pos2 < num_bits, pos2, num_bits);
        emp_assert(pos1 != pos2, pos1, pos2);
        size_t max_depth = std::max(cur_depth[pos1], cur_depth[pos2]);
        cur_depth[pos1] = cur_depth[pos2] = max_depth+1;
      }
      return cur_depth[ FindMaxIndex(cur_depth) ];
    }

    bool AddCompare(size_t id1, size_t id2) {
      emp_assert(id1 < 8*sizeof(bits_t), id1, sizeof(bits_t));
      emp_assert(id2 < 8*sizeof(bits_t), id2, sizeof(bits_t));
      if (id1 == id2) return false;                            // If ids are the same, don't add comparator!
      compare_set.push_back( (1 << id1) + (1 << id2) );
      return true;
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

    bits_t Sort(bits_t values) const {
      for (bits_t c : compare_set) values = RunCompare(values, c);
      return values;
    }

    size_t CountSortable(size_t num_bits = 16) const {
      const bits_t limit = 1 << num_bits;
      size_t count = 0;
      for (bits_t vals = 0; vals < limit; vals++) {
        bits_t svals = Sort(vals);
        if ( (svals & (svals+1)) == 0) count++;  // Sorted!
      }
      return count;
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