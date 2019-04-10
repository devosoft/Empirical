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
  public:
    using bits_t = uint32_t;          ///< Type used to represent pairs if posisions as bit masks. 
  protected:

    emp::vector<bits_t> compare_set;  ///< Comparators, in order (pairs of 1's in bitstring)

  public:
    BitSorter() { ; }
    BitSorter(const BitSorter &) = default;
    BitSorter(BitSorter &&) = default;
    ~BitSorter() { ; }

    BitSorter & operator=(const BitSorter &) = default;
    BitSorter & operator=(BitSorter &&) = default;

    bool operator!=(const BitSorter & other) const {
      return compare_set != other.compare_set;
    }

    bool operator<(const BitSorter & other) const {
      return compare_set < other.compare_set;
    }


    /// How many comparators are in this sorting network.
    size_t GetSize() const { return compare_set.size(); }
    size_t size() const { return GetSize(); }

    std::pair<size_t, size_t> GetComparator(size_t idx) {
      emp_assert(idx < compare_set.size(), idx, compare_set.size());
      bits_t curr = compare_set[idx];
      bits_t pos1 = pop_bit(curr);
      bits_t pos2 = find_bit(curr);
      return std::make_pair(pos1, pos2);
    }
    bits_t & operator[](size_t idx) {
      return compare_set[idx];
    }
    bits_t GetBits(size_t idx) {
      return compare_set[idx];
    }

    void Clear() {
      compare_set.clear();
    }

    /// If this network is compressed as far as possible, what will the max depth of each position be?
    void CalcDepth(size_t num_bits, emp::vector<size_t> & depth_vals) const {
      depth_vals.resize(0);
      depth_vals.resize(num_bits, 0);
      for (bits_t c : compare_set) {
        size_t pos1 = pop_bit(c);
        size_t pos2 = find_bit(c);
        emp_assert(pos1 < num_bits, pos1, num_bits);
        emp_assert(pos2 < num_bits, pos2, num_bits);
        emp_assert(pos1 != pos2, pos1, pos2);
        size_t max_depth = std::max(depth_vals[pos1], depth_vals[pos2]);
        depth_vals[pos1] = depth_vals[pos2] = max_depth+1;
      }
    }

    /// Return only the highest overall depth of the sorting network.
    size_t CalcDepth(size_t num_bits = 16) const {
      emp::vector<size_t> depth_vals;
      CalcDepth(num_bits, depth_vals);
      return depth_vals[ FindMaxIndex(depth_vals) ];
    }

    /// Push a new comparator onto the back of the list.
    bool AddCompare(size_t id1, size_t id2) {
      emp_assert(id1 < 8*sizeof(bits_t), id1, sizeof(bits_t));
      emp_assert(id2 < 8*sizeof(bits_t), id2, sizeof(bits_t));
      if (id1 == id2) return false;                            // If ids are the same, don't add comparator!
      compare_set.push_back( (1 << id1) + (1 << id2) );
      return true;
    }

    /// Change the positions compared in a specified comparator.
    bool EditCompare(size_t pos, size_t id1, size_t id2) {
      emp_assert(pos < compare_set.size());
      emp_assert(id1 < 8*sizeof(bits_t), id1, sizeof(bits_t));
      emp_assert(id2 < 8*sizeof(bits_t), id2, sizeof(bits_t));
      if (id1 == id2) return false;                            // If ids are the same, don't add comparator!
      compare_set[pos] = (1 << id1) + (1 << id2);
      return true;
    }

    /// Insert a new comparator at the specified position.
    bool InsertCompare(size_t pos, size_t id1, size_t id2) {
      emp_assert(pos <= compare_set.size());
      emp_assert(id1 < 8*sizeof(bits_t), id1, sizeof(bits_t));
      emp_assert(id2 < 8*sizeof(bits_t), id2, sizeof(bits_t));
      if (id1 == id2) return false;                            // If ids are the same, don't add comparator!
      compare_set.insert(compare_set.begin() + pos, (1 << id1) + (1 << id2));
      return true;
    }

    /// Remove the comparator at the specified position.
    bool RemoveCompare(size_t pos) {
      emp_assert(pos < compare_set.size());
      compare_set.erase(compare_set.begin() + pos);
      return true;
    }

    /// Run a specific set of bit values through a single comparator.
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

    /// Run a specific set of bit values through the series of comparators in this sorting network.
    bits_t Sort(bits_t values) const {
      for (bits_t c : compare_set) values = RunCompare(values, c);
      return values;
    }

    /// Determine if a particular input bit pattern will be sorted correctly by this network.
    bool TestSortable(bits_t values) const {
      const bits_t svals = Sort(values);
      return (svals & (svals+1)) == 0;
    }

    /// Try all possible bit sequences (with the specified number of bits) and count how many this sorting network
    /// fully sorts.
    size_t CountSortable(size_t num_bits = 16) const {
      const bits_t limit = 1 << num_bits;
      size_t count = 0;
      for (bits_t vals = 0; vals < limit; vals++) {
        bits_t svals = Sort(vals);
        if ( (svals & (svals+1)) == 0) count++;  // Sorted!  (a sorted network should have all one bits to the right)
      }
      return count;
    }

    /// Convert a specified set of values to an std::string of 0's and 1's.
    static std::string ToString(bits_t values, size_t num_bits=16) {
      std::string out_str;
      for (size_t id = num_bits; id > 0; id--) {
        if (values & 1 << (id-1)) out_str += "1";
        else out_str += "0";
      }
      return out_str;
    }

    /// Describe this sorting network in a string, listing all comparators in order.
    std::string AsString() const {
      std::stringstream out;
      for (bits_t c : compare_set) {
        size_t pos1 = pop_bit(c);
        size_t pos2 = find_bit(c);
        out << "[" << pos1 << "," << pos2 << "]";
      }
      return out.str();
    }
  };

}

namespace std {

  /// operator<< to work with ostream (must be in std to work)
  inline std::ostream & operator<<(std::ostream & out, const emp::BitSorter & bitsort) {
    out << bitsort.AsString();
    return out;
  }
}

#endif