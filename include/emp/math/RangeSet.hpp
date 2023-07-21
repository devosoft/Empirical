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
    RangeSet(T start, T end) { InsertRange(start, end); }
    RangeSet(const RangeSet &) = default;
    RangeSet(RangeSet &&) = default;

    RangeSet & operator=(const RangeSet &) = default;
    RangeSet & operator=(RangeSet &&) = default;

    [[nodiscard]] bool operator<=>(const RangeSet &) const = default;

    [[nodiscard]] bool Has(T val) const {
      const size_t id = _FindRange(val);
      return (id < range_set.size()) ? range_set[id].Has(val) : false;
    }
    [[nodiscard]] bool HasRange(range_t range) const {
      const size_t id = _FindRange(range.Lower());
      return (id < range_set.size()) ? range_set[id].HasRange(range) : false;
    }
    [[nodiscard]] bool IsEmpty() const { return !range_set.size(); }
    [[nodiscard]] static constexpr T MaxLimit() { return std::numeric_limits<T>::max(); }
    [[nodiscard]] static constexpr T MinLimit() { return std::numeric_limits<T>::lowest(); }

    /// @return Overall start of all ranges (or max value if no ranges exist.)
    [[nodiscard]] T GetStart() const { return IsEmpty() ? MaxLimit() : range_set[0].Lower(); }

    /// @return Overall end of all ranges (or min value if no ranges exist.)
    [[nodiscard]] T GetEnd() const { return IsEmpty() ? MinLimit() : range_set.back().Upper(); }
    
    [[nodiscard]] size_t GetNumRanges() const { return range_set.size(); }

    /// @brief Calculate the total combined size of all ranges.
    [[nodiscard]] T GetSize() const {
      T total = 0;
      for (const auto & x : range_set) total += x.GetSize();
      return total;
    }

    /// Present this set of ranges as a string.
    [[nodiscard]] emp::String ToString() const {
      emp::String out;
      for (size_t i = 0; i < range_set.size(); ++i) {
        if (i) out += ',';
        out += range_set[i].ToString();
      }
      return out;
    }

    // Return all of the internal ranges (can only be called on l-values)
    [[nodiscard]] const emp::vector<range_t> & GetRanges() const & { return range_set; }

    // Calculate the size of the overlap with a provided range.
    [[nodiscard]] bool HasOverlap(range_t range) const {
      size_t low_id = _FindRange(range.GetLower());
      if (low_id >= range_set.size()) return false;          // Entirely after ranges.
      if (range_set[low_id].HasOverlap(range)) return true;  // Overlaps at beginning.
      return low_id+1 < range_set.size() && range_set[low_id+1].HasOverlap(range);
    }

    // Calculate the size of the overlap with a provided range.
    [[nodiscard]] T CalcOverlap(range_t range) const {
      size_t low_id = _FindRange(range.GetLower());
      size_t up_id = _FindRange(range.GetUpper());
      T result = range_set[low_id].CalcOverlap(range);
      if (low_id < up_id) {
        for (size_t id=low_id+1; id < up_id; id++) result += range_set[id].GetSize();
        result += range_set[up_id].CalcOverlap(range);
      }
      return result;
    }

    /// @brief Shift all ranges by a fixed amount.
    /// @param shift 
    void Shift(T shift) {
      for (auto & range : range_set) range.Shift(shift);
    }

    [[nodiscard]] this_t CalcShift(T shift) {
      this_t out(*this);
      out.Shift(shift);
      return out;
    }

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

    /// @brief Merge an entire range set into this one.
    /// @param in_set Range set to add in.
    /// @return Did this RangeSet change?
    /// @note Can be optimized to handle big sets more efficiently!
    bool Insert(const this_t & in_set) {
      bool result = false;
      for (const range_t & range : in_set.GetRanges()) {
        result |= Insert(range);
      }
      return result;
    }

    /// @brief Insert a range into this set, specifying the start and end points.
    /// @param start Beginning of new range to include.
    /// @param stop Ending of new range to include (range is not inclusive of stop)
    /// @return Was there a change due to this insertion?
    bool InsertRange(T start, T stop) { return Insert(range_t{start, stop}); }

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
      if (GetEnd() > val) range_set.back().SetUpper(val);
      return true;
    }

    /// @brief  Remove a whole Range from this RangeSet.
    /// @param rm_range Range to remove
    /// @return Did the this RangeSet change due to this removal?
    bool Remove(range_t rm_range) {
      if (!HasOverlap(rm_range)) return false;
      if (rm_range.Lower() <= GetStart()) return RemoveTo(rm_range.Upper());
      if (rm_range.Upper() >= GetEnd()) return RemoveFrom(rm_range.Lower());

      // Must be removing from the middle.
      size_t start_id = _FindRange(rm_range.Lower());
      range_t & start_range = range_set[start_id];

      // Fully internal to a single Range?  Split it!
      if (start_range.Lower() < rm_range.Lower() && start_range.Upper() > rm_range.Upper()) {
        _InsertRange(start_id+1, range_t{rm_range.Upper(), start_range.Upper()});
        range_set[start_id].SetUpper(rm_range.Lower());
        return true;
      }
      
      // Deal with beginning of removal - cut it down if needed, and move on to next range.
      if (rm_range.Lower() > start_range.Lower()) {
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

    /// @brief Remove all ranges in an entire range set from this one.
    /// @param in_set Range set to remove.
    /// @return Did this RangeSet change?
    /// @note Can be optimized to handle big sets more efficiently!
    bool Remove(const this_t & in_set) {
      bool result = false;
      for (const range_t & range : in_set.GetRanges()) {
        result |= Remove(range);
      }
      return result;
    }


    bool RemoveRange(T start, T stop) { return Remove(range_t{start, stop}); }


    // Some more advanced functions.

    /// @brief  Calculate the inverted range set, swapping included and excluded values.
    /// @return The inverted RangeSet.
    [[nodiscard]] this_t CalcInverse() const {
      const bool add_begin = (GetStart() != MinLimit());
      const bool add_end = (GetEnd() != MaxLimit());
      this_t out_ranges;
      out_ranges.range_set.reserve(range_set.size() + add_begin + add_end - 1);
      if (add_begin) out_ranges.range_set.emplace_back(MinLimit(),GetStart());
      for (size_t i = 1; i < range_set.size(); ++i) {
        out_ranges.range_set.emplace_back(range_set[i-1].Upper(), range_set[i].Lower());
      }
      if (add_end) out_ranges.range_set.emplace_back(GetEnd(), MaxLimit());
      return out_ranges;
    }

    this_t & Invert() { *this = CalcInverse(); return *this; }

    // Simple operators:
    [[nodiscard]] this_t operator~() const { return CalcInverse(); }
    [[nodiscard]] this_t operator|(const this_t & in) const {
      this_t out(*this);
      out.Insert(in);
      return out;
    }
    [[nodiscard]] this_t operator&(const this_t & in) const {
      this_t out(*this);
      out.Remove(~in);
      return out;
    }
    [[nodiscard]] this_t operator^(const this_t & in) {
      return (*this | in) & ~(this & in);
    }
    [[nodiscard]] this_t operator<<(const T shift) const { return CalcShift(shift); }
    [[nodiscard]] this_t operator>>(const T shift) const { return CalcShift(-shift); }
    [[nodiscard]] bool operator[](T val) const { return Has(val); }

    this_t & operator|=(const this_t & in) const { Insert(in); return *this; }
    this_t & operator&=(const this_t & in) const { Remove(~in); return *this; }
    this_t & operator<<=(const T shift) const { Shift(shift); return *this; }
    this_t & operator>>=(const T shift) const { Shift(-shift); return *this; }

    explicit operator bool() const { return range_set.size(); }


    /// @brief Overload ostream operator to return Print.
    friend std::ostream& operator<<(std::ostream &out, const this_t & range) {
      out << range.ToString();
      return out;
    }
  };

}

#endif
