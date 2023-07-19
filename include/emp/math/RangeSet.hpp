/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2023
 *
 *  @file RangeSet.hpp
 *  @brief A collection of ranges that can be operated on collectively.
 *  @note Status: BETA
 */

#ifndef EMP_MATH_RANGE_SET_HPP_INCLUDE
#define EMP_MATH_RANGE_SET_HPP_INCLUDE

#include <algorithm>

#include "../base/vector.hpp"
#include "../datastructs/vector_utils.hpp"

#include "Range.hpp"

namespace emp {

  /// RangeSet maintains a collection of ranges.  The ranges are exclusive of the endpoint
  /// and kept sorted and non-adjacent (i.e., there is a gap between successive ranges).
  template <typename T>
  class RangeSet {
  public:
    using range_t = emp::Range<T, false>;
    using this_t = RangeSet<T>;

  private:
    emp::vector<range_t> range_set;

    // Helper function to find the id of an Range that a value belongs in or can extend;
    // returns next-higher index (where a new range would be place) if none fit perfectly.
    // size_t _FindRange(size_t val) const {
    //   for (size_t id = 0; id < range_set.size(); ++id) {
    //     if (id <= range_set[id].GetEnd()) return id;
    //   }
    //   return range_set.size();
    // }
    size_t _FindRange(T value) const {
      auto it = std::lower_bound(
        range_set.begin(), 
        range_set.end(), 
        value,
        [](const range_t & range, T value) { return range.GetUpper() < value; }
      );
      return it - range_set.begin();
    };

    void _InsertRange(size_t id, range_t range) { emp::InsertAt(range_set, id, range); }
    void _RemoveRange(size_t id) { emp::RemoveAt(range_set, id); }
    void _RemoveRanges(size_t id, size_t count) {  emp::RemoveAt(range_set, id, count); }

    // Helper function to increase the side of a range, possibly merging it with the next range.
    void _CleanupMerge(size_t id) {
      while (id+1 < range_set.size() && range_set[id].Merge(range_set[id+1])) {
        _RemoveRange(id+1);  // Delete current range (merged in to previous)
      }
    }

  public:
    static constexpr bool is_integral = std::is_integral<T>();

    RangeSet() = default;
    RangeSet(range_t start_range) { Insert(start_range); }
    RangeSet(const RangeSet &) = default;
    RangeSet(RangeSet &&) = default;

    RangeSet & operator=(const RangeSet &) = default;
    RangeSet & operator=(RangeSet &&) = default;

    bool operator<=>(const RangeSet &) const = default;

    bool Has(T val) const {
      const size_t id = _FindRange(val);
      return (id < range_set.size()) ? range_set[id].Has(val) : false;
    }
    bool HasRange(range_t range) const {
      const size_t id = _FindRange(range.Lower());
      return (id < range_set.size()) ? range_set[id].HasRange(range) : false;
    }
    bool IsEmpty() const { return !range_set.size(); }
    static constexpr T MaxLimit() { return std::numeric_limits<T>::max(); }
    static constexpr T MinLimit() { return std::numeric_limits<T>::lowest(); }

    /// @return Overall start of all ranges (or max value if no ranges exist.)
    T GetStart() const { return IsEmpty() ? MaxLimit() : range_set[0].Lower(); }

    /// @return Overall end of all ranges (or min value if no ranges exist.)
    T GetEnd() const { return IsEmpty() ? MinLimit() : range_set.back().Upper(); }
    
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
      emp_assert(id < range_set.size(), id, range_set.size());
      range_t & cur_range = range_set[id];

      if (cur_range.Has(val)) return false;                        // Don't have the value!
      else if (cur_range.Append(val)) _CleanupMerge(id);           // Extending 'upper' on range
      else if (cur_range.GetLower() == val+1) cur_range.Lower()--; // Extending 'lower' on range
      else range_set.emplace(range_set.begin()+id, val);           // Inserting NEW range.

      return true;
    }

    /// @brief Insert a whole range into this set, merging other ranges if needed.
    /// @param in New range to include.
    /// @return Was there a change due to this insertion?
    bool Insert(range_t in) {
      const size_t start_id = _FindRange(in.GetLower());

      // Are we adding a whole new range to the end?
      if (start_id == range_set.size()) range_set.push_back(in);

      // Is it already included in the found range?
      else if (range_set[start_id].HasRange(in)) return false;

      // Should we merge in with an existing range?
      else if (range_set[start_id].IsConnected(in)) {
        range_set[start_id].Merge(in);
        _CleanupMerge(start_id);
      }

      // Otherwise insert as a new range.
      else _InsertRange(start_id, in);  

      return true;
    }

    /// @brief  Remove a single value from this RangeSet.
    /// @param val Value to remove
    /// @return Did the range change due to this removal?
    bool Remove(T val) {
      emp_assert(is_integral, "Only integral ranges can call Remove() with a single value.");

      if (!Has(val)) return false;  // Not included!

      const size_t id = _FindRange(val);
      range_t & cur_range = range_set[id];
      if (cur_range.GetSize() == 1) _RemoveRange(id);            // Remove whole range
      else if (cur_range.GetLower() == val) cur_range.Lower()++;             // Inc lower end
      else if (cur_range.GetUpper()-1 == val) cur_range.Upper()--;           // Dec upper end
      else {                                                                 // Split a range!
        _InsertRange(id+1, range_t{val+1,cur_range.GetUpper()});
        range_set[id].SetUpper(val);
      }
      return true;
    }

    /// @brief  Remove all ranges (or partial range) less than a target value.
    /// @param val New floor for ranges.
    /// @return Did the range change due to this removal?
    bool RemoveTo(T val) {
      if (val <= GetStart()) return false;  // Nothing to remove.
      size_t id = _FindRange(val);
      if (val == range_set[id].GetUpper()) ++id;
      _RemoveRanges(0, id);  // Remove everything before the new start.
      if (range_set.size() && range_set[0].Lower() < val) range_set[0].SetLower(val);
      return true;
    }

    /// @brief  Remove all ranges (or partial range) greater than a target value.
    /// @param val New cap for ranges.
    /// @return Did the range change due to this removal?
    bool RemoveFrom(T val) {
      if (val >= GetEnd()) return false;        // Nothing to remove.
      size_t id = _FindRange(val);
      if (val > range_set[id].GetLower()) ++id; // Include current range if needed.
      range_set.resize(id);                     // Remove everything past new end.
      if (GetEnd() > val) range_set.back.SetUpper(val);
      return true;
    }

    /// @brief  Remove a whole Range from this RangeSet.
    /// @param rm_range Range to remove
    /// @return Did the this RangeSet change due to this removal?
    bool Remove(range_t rm_range) {
      if (rm_range.Lower() <= GetStart()) return RemoveTo(rm_range.Upper());
      if (rm_range.Upper() >= GetEnd()) return RemoveFrom(rm_range.Lower());

      // Must be removing from the middle.
      size_t start_id = _FindRange(rm_range.Lower());
      range_t & start_range = range_set[start_id];

      // Inside of a single Range?
      if (start_range.Lower() < rm_range.Lower() && start_range.Upper() > rm_range.Upper()) {
        _InsertRange(start_id+1, range_t{rm_range.Upper(), start_range.Lower()});
        range_set[start_id].SetUpper(rm_range.Lower());
        return true;
      }
      
      // Deal with beginning of removal.
      if (rm_range.Lower() >= start_range.Lower()) {
        start_range.Upper() = rm_range.Lower();
        ++start_id;
      }

      // Deal with end of removal.
      size_t end_id = _FindRange(rm_range.Upper());
      if (rm_range.Upper() >= range_set[end_id].Upper()) end_id++;
      else range_set[end_id].Lower() = std::max(range_set[end_id].Lower(), rm_range.Upper());

      // Remove middle.
      _RemoveRanges(start_id, end_id - start_id);

      return true;
    }
  };

}

#endif
