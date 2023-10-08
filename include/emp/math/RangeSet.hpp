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

    // Helper function to remove empty ranges at the beginning.
    void _PruneEmptyFront() {
      size_t count = 0;
      while (count < range_set.size() && range_set[count].GetSize() == 0) ++count;
      if (count) _RemoveRanges(0, count);
    }

    // Helper function to remove empty ranges at the end.
    void _PruneEmptyBack() {
      size_t count = 0;
      while (count < range_set.size() &&
             range_set[range_set.size()-count-1].GetSize() == 0) ++count;
      if (count) range_set.resize(range_set.size() - count);
    }

    // Helper function to increase the side of a range, possibly merging it with the next range.
    void _CleanupMerge(size_t id) {
      while (id+1 < range_set.size() && range_set[id].Merge(range_set[id+1])) {
        _RemoveRange(id+1);  // Delete next range (merged in to current)
      }
      emp_assert(OK());
    }

    // Helper function to convert a string into a RangeSet.
    // Two formats are available
    //   bitstring:  010001110101111
    //   RangeSet:   [1,2),[5,8),[9,10),[11,15)
    // In the bitstring format, if the final character is '+', all additional positions are
    // assumed to be 1.  In the RangeSet format, if the first or last element is a '*', it is
    // assumed to be the limit for the type; also commas are optional.
    // A star by itself is a full RangeSet.
    void _FromString(emp::String in) {
      if (in.size() == 0) { Clear(); }
      else if (in[0] == '*') {
        emp::notify::TestError(in.size() > 1, "Star indicates a full range, but must be by itself.");
        SetAll();
      }
      else if (in[0] == '0' || in[0] == '1') {
        Clear();
        for (size_t i=0; i < in.size(); ++i) {
          if (in[i] != '0') Insert((T)i);
        }
      }
      else if (in[0] == '[') {
        while (in.size()) {
          emp::String segment = in.Pop(')');
          segment.PopIf(',');
          emp::notify::TestError(!segment.PopIf('['), "Each segment of a RangeSet must begin with '['");
          T start = segment.PopIf('*') ? MinLimit() : segment.PopLiteral<T>();
          emp::notify::TestError(!segment.PopIf(',') && !segment.PopIf('-'),
            "Each segment of a RangeSet must be separated by ',' or '-'");
          T end = segment.PopIf('*') ? MaxLimit() : segment.PopLiteral<T>();
          InsertRange(start, end);
        }
      }
    }

  public:
    static constexpr bool is_integral = std::is_integral<T>();

    RangeSet() = default;
    explicit RangeSet( range_t start_range) { Insert(start_range); }
    RangeSet(T start, T end) { InsertRange(start, end); }
    RangeSet(const RangeSet &) = default;
    RangeSet(RangeSet &&) = default;
    RangeSet(const std::string & bitstring) {
      emp_assert(is_integral, "RangeSets can be represented as strings only if they are integral.");
      for (size_t i=0; i < bitstring.size(); ++i) {
        if (bitstring[i] == '1') Insert((T)i);
      }
    }

    RangeSet & operator=(const RangeSet &) = default;
    RangeSet & operator=(RangeSet &&) = default;
    RangeSet & operator=(const std::string & bitstring) {
      emp_assert(is_integral, "RangeSets can be represented as strings only if they are integral.");
      Clear();
      for (size_t i=0; i < bitstring.size(); ++i) {
        if (bitstring[i] == '1') Insert((T)i);
      }
      return *this;
    }

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

    /// @brief Remove all ranges in the set.
    RangeSet & Clear() { range_set.resize(0); return *this; }

    /// @brief Set a single range that includes all value.
    RangeSet & SetAll() { InsertRange(MinLimit(), MaxLimit()); return *this; }

    /// @brief Shift all ranges by a fixed amount.
    /// @param shift How much should the range be shifted by?
    RangeSet & Shift(T shift) {
      if (shift > 0) ShiftUp(shift);
      else if (shift < 0) ShiftDown(shift);
      return *this;
    }

    RangeSet & ShiftUp(T shift) {
      for (auto & range : range_set) range.ShiftUp(shift);
      _PruneEmptyBack();
      return *this;
    }

    RangeSet & ShiftDown(T shift) {
      for (auto & range : range_set) range.ShiftDown(shift);
      _PruneEmptyFront();
      return *this;
    }


    [[nodiscard]] this_t CalcShift(T shift) const {
      this_t out(*this);
      return out.Shift(shift);
    }

    [[nodiscard]] this_t CalcShiftDown(T shift) const {
      this_t out(*this);
      return out.ShiftDown(shift);
    }

    [[nodiscard]] this_t CalcShiftUp(T shift) const {
      this_t out(*this);
      return out.ShiftUp(shift);
    }

    /// @brief Insert a value into this range set
    /// @param val Value to insert.
    /// @return This RangeSet after insertion.
    RangeSet & Insert(T val) {
      emp_assert(is_integral, "Only integral ranges can call Insert() with a single value.");

      // If empty or beyond the end, append a new range.
      if (range_set.size() == 0 || val > GetEnd()) {
        range_set.emplace_back(val);
      }

      else {
        const size_t id = _FindRange(val);
        emp_assert(id < range_set.size(), id, range_set.size());
        range_t & range = range_set[id];

        if (range.Has(val)) return *this;                     // Already has the value!
        else if (range.Append(val)) _CleanupMerge(id);        // Extending 'upper' on range
        else if (range.GetLower() == val+1) range.Lower()--;  // Extending 'lower' on range
        else range_set.emplace(range_set.begin()+id, val);    // Inserting NEW range.
      }

      return *this;
    }

    /// @brief Insert a whole range into this set, merging other ranges if needed.
    /// @param in New range to include.
    /// @return This RangeSet after insertion.
    RangeSet & Insert(range_t in) {
      const size_t start_id = _FindRange(in.GetLower());

      // Are we adding a whole new range to the end?
      if (start_id == range_set.size()) range_set.push_back(in);

      // Is it already included in the found range?  No change!
      else if (range_set[start_id].HasRange(in)) return *this;

      // Should we merge in with an existing range?
      else if (range_set[start_id].IsConnected(in)) {
        range_set[start_id].Merge(in);
        _CleanupMerge(start_id);
      }

      // Otherwise insert as a new range.
      else _InsertRange(start_id, in);  

      return *this;
    }

    /// @brief Merge an entire range set into this one.
    /// @param in_set Range set to add in.
    /// @return This RangeSet after insertion.
    /// @note Can be optimized to handle big set mergers more efficiently!
    RangeSet & Insert(const this_t & in_set) {
      for (const range_t & range : in_set.GetRanges()) Insert(range);
      return *this;
    }

    /// @brief Insert a range into this set, specifying the start and end points.
    /// @param start Beginning of new range to include.
    /// @param stop Ending of new range to include (range is not inclusive of stop)
    /// @return This RangeSet after insertion.
    RangeSet & InsertRange(T start, T stop) { return Insert(range_t{start, stop}); }

    /// @brief  Remove a single value from this RangeSet.
    /// @param val Value to remove
    /// @return This RangeSet after removal.
    RangeSet & Remove(T val) {
      emp_assert(is_integral, "Only integral ranges can call Remove() with a single value.");

      if (!Has(val)) return *this;  // Nothing to remove.

      const size_t id = _FindRange(val);
      range_t & range = range_set[id];
      if (range.GetSize() == 1) _RemoveRange(id);             // Remove whole range
      else if (range.GetLower() == val) range.Lower()++;      // Inc lower end
      else if (range.GetUpper()-1 == val) range.Upper()--;    // Dec upper end
      else {                                                  // Split a range!
        _InsertRange(id+1, range_t{val+1,range.GetUpper()});
        range_set[id].SetUpper(val);
      }
      return *this;
    }

    /// @brief  Remove all ranges (or partial range) less than a target value.
    /// @param val New floor for ranges.
    /// @return This RangeSet after removal.
    RangeSet & RemoveTo(T val) {
      if (val <= GetStart()) return *this;  // Nothing to remove.
      size_t id = _FindRange(val);
      if (val == range_set[id].GetUpper()) ++id;
      _RemoveRanges(0, id);  // Remove everything before the new start.
      if (range_set.size() && range_set[0].Lower() < val) range_set[0].SetLower(val);
      return *this;
    }

    /// @brief  Remove all ranges (or partial range) greater than a target value.
    /// @param val New cap for ranges.
    /// @return This RangeSet after removal.
    RangeSet & RemoveFrom(T val) {
      if (val >= GetEnd()) return *this;        // Nothing to remove.
      size_t id = _FindRange(val);
      if (val > range_set[id].GetLower()) ++id; // Include current range if needed.
      range_set.resize(id);                     // Remove everything past new end.
      if (GetEnd() > val) range_set.back().SetUpper(val);
      return *this;
    }

    /// @brief  Remove a whole Range from this RangeSet.
    /// @param rm_range Range to remove
    /// @return This RangeSet after removal.
    RangeSet & Remove(range_t rm_range) {
      if (!HasOverlap(rm_range)) return *this;
      if (rm_range.Lower() <= GetStart()) return RemoveTo(rm_range.Upper());
      if (rm_range.Upper() >= GetEnd()) return RemoveFrom(rm_range.Lower());

      // Must be removing from the middle.
      size_t start_id = _FindRange(rm_range.Lower());
      range_t & start_range = range_set[start_id];

      // Fully internal to a single Range?  Split it!
      if (start_range.Lower() < rm_range.Lower() && start_range.Upper() > rm_range.Upper()) {
        _InsertRange(start_id+1, range_t{rm_range.Upper(), start_range.Upper()});
        range_set[start_id].SetUpper(rm_range.Lower());
        return *this;
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

      return *this;
    }

    /// @brief Remove all ranges in an entire range set from this one.
    /// @param in_set Range set to remove.
    /// @return This RangeSet after removal.
    /// @note Can be optimized to handle big sets more efficiently!
    RangeSet & Remove(const this_t & in_set) {
      for (const range_t & range : in_set.GetRanges()) Remove(range);
      return *this;
    }


    RangeSet & RemoveRange(T start, T stop) { return Remove(range_t{start, stop}); }

    /// @brief Remove everything outside of the provided range.
    RangeSet & KeepOnly(T start, T stop) {
      emp_assert(start < stop);
      RemoveTo(start);
      return RemoveFrom(stop);
    }

    /// @brief Remove everything outside of the provided range.
    RangeSet & KeepOnly(range_t keep_range) {
      return KeepOnly(keep_range.GetLower(), keep_range.GetUpper());
    }

    /// @brief Remove everything outside of the provided set of ranges.
    RangeSet & KeepOnly(const this_t & in_set) { return Remove(~in_set); }


    // Some more advanced functions.

    /// @brief  Calculate the inverted range set, swapping included and excluded values.
    /// @return The inverted RangeSet.
    [[nodiscard]] this_t CalcInverse() const {
      emp_assert(OK());
      // If this is an empty set, return a full set.
      if (range_set.size() == 0) return this_t(MinLimit(), MaxLimit());

      // Determine if we need to extend the the limits on each side.
      const bool add_begin = (GetStart() != MinLimit());
      const bool add_end = (GetEnd() != MaxLimit());
      this_t out;
      out.range_set.reserve(range_set.size() + add_begin + add_end - 1);
      if (add_begin) out.range_set.emplace_back(MinLimit(),GetStart());
      for (size_t i = 1; i < range_set.size(); ++i) {
        out.range_set.emplace_back(range_set[i-1].Upper(), range_set[i].Lower());
      }
      if (add_end) out.range_set.emplace_back(GetEnd(), MaxLimit());
      emp_assert(out.OK());
      return out;
    }

    this_t & Invert() { *this = CalcInverse(); return *this; }

    // Simple operators:
    [[nodiscard]] this_t operator~() const { return CalcInverse(); }
    [[nodiscard]] this_t operator|(const this_t & in) const {
      emp_assert(in.OK());
      this_t out(*this);
      return out.Insert(in);
    }
    [[nodiscard]] this_t operator&(const this_t & in) const {
      emp_assert(in.OK());
      this_t out(*this);
      return out.Remove(~in);
    }
    [[nodiscard]] this_t operator^(const this_t & in) {
      emp_assert(in.OK());
      return (*this | in) & ~(*this & in);
    }
    [[nodiscard]] this_t operator<<(const T shift) const { return CalcShiftUp(shift); }
    [[nodiscard]] this_t operator>>(const T shift) const { return CalcShiftDown(shift); }
    [[nodiscard]] bool operator[](T val) const { return Has(val); }

    this_t & operator|=(const this_t & in) { Insert(in); return *this; }
    this_t & operator&=(const this_t & in) { Remove(~in); return *this; }
    this_t & operator^=(const this_t & in) { emp_assert(in.OK()); *this = *this^in; return *this; }
    this_t & operator<<=(const T shift) { ShiftUp(shift); return *this; }
    this_t & operator>>=(const T shift) { ShiftDown(shift); return *this; }

    explicit operator bool() const { return range_set.size(); }


    /// @brief Overload ostream operator to return Print.
    friend std::ostream& operator<<(std::ostream &out, const this_t & range) {
      out << range.ToString();
      return out;
    }

    /// @brief Check for internal errors in this RangeSet. 
    bool OK() const {
      // Check each range individually.
      for (const auto & range : range_set) {
        if (range.GetLower() > range.GetUpper()) {
          emp::notify::Message("RangeSet::OK() Failed due to invalid range: ", range.ToString());
          return false;
        }
      }

      // Make sure ranges are in order and have gaps between them.
      for (size_t i = 1; i < range_set.size(); ++i) {
        if (range_set[i-1].GetUpper() >= range_set[i].GetLower()) {
          emp::notify::Message("RangeSet::OK() Failed at range ", i, " of ", range_set.size(),
                               ".  Ranges are: ", ToString());
          return false;
        }
      }
      return true;
    }
  };

}

#endif
