/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2023
 *
 *  @file RangeSet.hpp
 *  @brief A collection of ranges that can be operated on collectively.
 *  @note Status: BETA
 */

#ifndef EMP_MATH_RANGE_HPP_INCLUDE
#define EMP_MATH_RANGE_HPP_INCLUDE

#include <algorithm>

#include "../base/vector.hpp"

#include "Range.hpp"

namespace emp {

  /// RangeSet maintains a collection of ranges.  The ranges are exclusive of the endpoint
  /// and kept sorted and non-adjacent (i.e., there is a gap between successive ranges).
  template <typename T>
  class RangeSet {
    using range_t = emp::Range<T, false>;

    emp::vector<range_t> range_set;

    // Helper function to find the id of an Range that a value belongs in or can extend;
    // returns next-higher index (where a new range would be place) if none fit perfectly.
    // size_t _FindRange(size_t val) const {
    //   for (size_t id = 0; id < range_set.size(); ++id) {
    //     if (id <= range_set[id].GetEnd()) return id;
    //   }
    //   return range_set.size();
    // }
    size_t _FindRange(T value) {
      auto it = std::lower_bound(
        range_set.begin(), 
        range_set.end(), 
        value,
        [](const range_t & range, T value) { return range.GetUpper() <= value; }
      return it - range_set.begin();
    };

    // Helper function to increase the side of a range, possibly merging it with the next range.
    void _CleanupMerge(size_t id) {
      while (id+1 < range_set.size() && range_set[id].Merge(range_set[id+1])) {
        range_set.erase(range_set.begin()+id+1); // Delete current range (merged in to previous)
      }
    }

  public:
    static constexpr bool is_integral = std::is_integral<T>();

    RangeSet() = default;
    RangeSet(const RangeSet &) = default;
    RangeSet(RangeSet &&) = default;

    RangeSet & operator=(const RangeSet &) = default;
    RangeSet & operator=(RangeSet &&) = default;

    bool Has(T val) const {
      const size_t id = _FindRange(val);
      return (id < range_set.size()) ? range_set[id].Has(val) : false;
    }

    /// @return Overall start of all ranges (or max value if no ranges exist.)
    T GetStart() const {
      return range_set.size() ? range_set[0].GetStart() : std::numeric_limits<T>::max();
    }

    /// @return Overall end of all ranges (or min value if no ranges exist.)
    T GetEnd() const {
      return range_set.size() ? range_set.back().GetEnd() : std::numeric_limits<T>::lowest();
    }
    
    size_t GetNumRanges() const { return range_set.size(); }

    /// @brief Calculate the total combined size of all ranges.
    T GetSize() const {
      T total = 0;
      for (const auto & x : range_set) total += x.GetSize();
      return total;
    }

    // Return all of the internal ranges.
    const emp::vector<range_t> & GetRanges() & { return range_set; }

    /// @brief Insert a value into this range set
    /// @param val Value to insert.
    /// @return Was there a change due to this insertion (or was it already there?)
    bool Insert(T val) {
      emp_assert(is_integral, "Only integral ranges can call Insert() with a single value.");

      // If empty or beyond the end, append a new range.
      if (range_set.size() == 0 || val > GetEnd()) {
        range_set.emplace_back(val);
        return true;
      }

      const size_t id = _FindRange(val);
      range_t & cur_range = range_set[id];

      // Do we already have the value?
      if (cur_range.Has(val)) return false;

      // Are we extending a range (and possibly merging)?
      else if (cur_range.Append(val)) {
        if (id+1 < range_set.size() && cur_range.Merge(range_set[id+1])) {
          range_set.erase(range_set.begin()+id+1); // Delete next range (merged in to current)
        }
      }

      // Are we extending the beginning of the next range?
      else if (cur_range.GetStart() == val+1) cur_range.Insert(val);

      // Otherwise we must insert an entirely new range.
      else range_set.emplace(id, val);

      return true;
    }

    /// @brief Insert a whole range into this set, merging other ranges if needed.
    /// @param in New range to include.
    /// @return Was there a change due to this insertion?
    bool Insert(range_t in) {
      // Are we adding a whole new range to the end?
      if (range_set.size() == 0 || in.GetStart() > GetEnd()) {
        range_set.emplace_back(in);
        return true;
      }

      const size_t start_id = _FindRange(in.GetStart());
      range_t & start_range = range_set[start_id];

      if (start_range.HasRange(in)) return false; // Already has range!

      // Do we have a new range to insert here?
      if (!start_range.IsConnected(in)) {
        range_set.insert(range_set.begin() + start_id, in);
        return true;        
      }

      // Otherwise merge with the existing range and cleanup if needed.
      start_range.Merge(in);
      _CleanupMerge(start_id);

      return true;
    }

    /// @brief  Remove a single value from this index range.
    /// @param val Value to remove
    /// @return Did the range change due to this removal?
    bool Remove(T val) {
      emp_assert(is_integral, "Only integral ranges can call Remove() with a single value.");

      if (!Has(val)) return false;
      size_t id = _FindRange(val);
      range_t & cur_range = range_set[id];
      if (cur_range.GetSize() == 1) range_set.erase(range_set.begin()+id);
      else if (cur_range.GetStart() == val) cur_range.SetStart(cur_range.GetStart()+1);
      else if (cur_range.GetEnd()-1 == val) cur_range.SetEnd(cur_range.GetEnd()-1);
      else {
        // Need to split the range.
        range_set.insert(range_set.begin()+id+1, range_t{val+1,cur_range.GetEnd()});
        cur_range.SetEnd(val);
      }
    }
  };

}

#endif
