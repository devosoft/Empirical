/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2023
*/
/**
 *  @file
 *  @brief Collection of indices, ideally optimized for memory size.
 *  @note Status: ALPHA
 */

#ifndef EMP_DATASTRUCTS_INDEXSET_HPP_INCLUDE
#define EMP_DATASTRUCTS_INDEXSET_HPP_INCLUDE

#include "../base/Ptr.hpp"
#include "../bits/BitVector.hpp"
#include "../math/constants.hpp"

namespace emp {

  /// Index range is a simple pair of values indicating the start and end of a series of indices.
  class IndexRange {
    size_t start = 0; // First value in this range.
    size_t end = 0;   // First value after start NOT in this range; zero for empty range.

  public:
    IndexRange() = default;
    IndexRange(size_t val) : start(val), end(val+1) { }
    IndexRange(size_t _start, size_t _end) : start(_start), end(_end) { }
    IndexRange(const IndexRange &) = default;

    IndexRange & operator=(const IndexRange &) = default;

    auto operator<=>(const IndexRange &) const = default;

    size_t GetStart() const { return start; }
    size_t GetEnd() const { return end; }
    size_t GetSize() const { return end-start; }

    void SetStart(size_t in) { start = in; }
    void SetEnd(size_t in) { end = in; }

    bool Has(size_t val) const { return val >= start && val < end; }
    bool Has(IndexRange in) const { return in.start >= start && in.end < end; }

    /// Will identify if two ranges are next to each other or overlapping.
    bool IsConnected(IndexRange in) const {
      return (in.start >= start && in.start <= end ||
          start >= in.start && start <= in.end);
    }

    /// Grow this range (default, by one)
    void Grow(size_t count=1) { end += count; }

    /// Insert a value into a range if valid; return false if not.
    bool Insert(size_t val) {
      if (val == end) { end++; return true; }
      if (val == start - 1) { start--; return true; }
      return Has(val);
    }

    /// Extend the current range with a new one.  Must be perfectly adjacent!
    bool Append(IndexRange in) {
      if (end == in.start) { end = in.end; return true; }
      return false;
    }

    /// @brief  Expand this range to encompass a provided value.
    /// @param val Value to expand through.
    /// @return Whether the range has changed due to this expansion.
    bool Expand(size_t val) {
      if (val < start) start = val;
      else if (val > end) end = val;
      else return false;
      return true;
    }

    /// @brief Expand this range to encompass all provided values.
    /// @param vals Values to expand through
    /// @return Whether the range has changed due to this expansion.
    template <typename... Ts>
    bool Expand(size_t val1, size_t val2, Ts... args) {
      return Expand(val1) + Expand(val2, args...);
    }


    /// Merge this range with another.  Must be adjacent or overlap!
    bool Merge(IndexRange in) {
      if (!IsConnected(in)) return false;
      return Expand(in.start) + Expand(in.end);  // Use + to avoid short-circuiting.
    }
  };

  /// IndexRanges is a class to maintain a series of ranges of indexes.  The ranges will
  /// always be kept sorted and non-adjacent (i.e., there will always be at least one index
  /// missing between two ranges).
  class IndexRangeSet {
    emp::vector<IndexRange> range_set;

    // Helper function to find the id of an IndexRange that a value belongs in or can extend;
    // returns next-higher index if none fit perfectly.
    // @CAO - consider doing a binary search.
    size_t _FindRange(size_t val) const {
      for (size_t id = 0; id < range_set.size(); ++id) {
        if (id <= range_set[id].GetEnd()) return id;
      }
      return range_set.size();
    }

    // Helper function to grow a range by one, possibly merging it with the next range.
    void _GrowRange(size_t id) {
      emp_assert(id < range_set.size());
      range_set[id].Grow();

      // Test if we need to merge with the next range.
      if (id+1 < range_set.size() && range_set[id].GetEnd() == range_set[id+1].GetStart()) {
        range_set[id].SetEnd(range_set[id+1].GetEnd());
        range_set.erase(range_set.begin()+id+1); // Delete next range (now merged in)
      }
    }

  public:
    IndexRangeSet() = default;
    IndexRangeSet(const IndexRangeSet &) = default;
    IndexRangeSet(IndexRangeSet &&) = default;

    IndexRangeSet & operator=(const IndexRangeSet &) = default;
    IndexRangeSet & operator=(IndexRangeSet &&) = default;

    bool Has(size_t val) const {
      size_t id = _FindRange(val);
      if (id >= range_set.size()) return false;
      return range_set[id].Has(val);
    }

    size_t GetStart() const {
      return range_set.size() ? range_set[0].GetStart() : emp::MAX_SIZE_T;
    }
    size_t GetEnd() const {
      return range_set.size() ? range_set.back().GetEnd() : 0;
    }

    size_t GetNumRanges() const { return range_set.size(); }

    /// @brief Calculate the total combined size of all ranges.
    size_t GetSize() const {
      size_t total = 0;
      for (const auto & x : range_set) total += x.GetSize();
      return total;
    }

    // Return all of the internal ranges.
    const emp::vector<IndexRange> & GetRanges() & { return range_set; }

    /// @brief  Add a new value that belongs at the end of the sets.
    /// @param val Value to add
    /// @return Did the append work?  If it's not at the end, returns false.
    bool Append(size_t val) {
      if (range_set.size() == 0 || val > GetEnd()) range_set.emplace_back(val); // New Range
      else if (val == GetEnd()) range_set.back().SetEnd(val+1);                 // Extend range
      else return false;                                                        // Not at end

      return true;
    }

    /// @brief  Add an entire range that belongs at the end of the sets.
    /// @param val Range to add
    /// @return Did the append work?  If it's not at the end, returns false.
    bool Append(IndexRange in) {
      // Are we adding on a new range?
      if (range_set.size() == 0 || in.GetStart() > GetEnd()) {
        range_set.emplace_back(in);
      }

      // Are we extending an existing range?
      else if (in.GetEnd() > GetEnd()) {
        // Are we encompassing ALL existing ranges?
        if (in.GetStart() <= range_set[0].GetStart()) {
          range_set.resize(1);
          range_set[0] = in;
        }

        // Otherwise find the start and convert from there.
        else {
          const size_t start_id = _FindRange(in.GetStart());
          range_set[start_id].SetEnd(in.GetEnd());
        }
      }

      else return false; // Not at end

      return true;
    }

    /// @brief Insert a value into this range set
    /// @param val Value to insert.
    /// @return Was there a change due to this insertion (or was it already there?)
    bool Insert(size_t val) {
      // Are we inserting a new range onto the end?
      if (Append(val)) return true;

      // Do we already have the value?
      size_t id = _FindRange(val);
      if (range_set[id].Has(val)) return false;

      // Are we extending the range (and possibly merging)?
      else if (range_set[id].GetEnd() == val) _GrowRange(id);

      // Are we extending the beginning of the next range?
      else if (range_set[id].GetStart() == val+1) range_set[id].Insert(val);

      // Otherwise we must insert an entirely new range.
      else range_set.emplace(id, val);

      return true;
    }

    /// @brief Insert a whole range into this set, merging other ranges as needed.
    /// @param in New range to include.
    /// @return Was there a change due to this insertion (or were they already there?)
    bool Insert(IndexRange in) {
      // If the new range goes past the end, Append will take care of it.
      if (Append(in)) return true;

      size_t start_id = _FindRange(in.GetStart());
      size_t end_id = _FindRange(in.GetEnd());
      emp_assert(start_id <= end_id);

      // If both are in the same range id, either insert a new range or modify an existing one.
      if (start_id == end_id) {
        // If the end of the new range is before the start of the found range, insert the new one!
        if (in.GetEnd() < range_set[start_id].GetStart() - 1) {
          range_set.insert(range_set.begin() + start_id, in);
        }

        // Otherwise try to merge it into the existing range (will return false if already there)
        else return range_set[start_id].Merge(in);
      }

      // We are across multiple ranges.  Collapse into first!
      else {
        if (in.GetEnd()+1 < range_set[end_id].GetStart()) --end_id; // Don't include end id.
        range_set[start_id].Expand(in.GetStart(), in.GetEnd(), range_set[end_id].GetEnd());
        range_set.erase(range_set.begin()+start_id+1, range_set.begin()+end_id+1);
      }

      return true;
    }

    /// @brief  Remove a single value from this index range.
    /// @param val Value to remove
    /// @return Did the range change due to this removal?
    bool Remove(size_t val) {
      if (!Has(val)) return false;
      size_t id = _FindRange(val);
      IndexRange & cur_range = range_set[id];
      if (cur_range.GetSize() == 1) range_set.erase(range_set.begin()+id);
      else if (cur_range.GetStart() == val) cur_range.SetStart(cur_range.GetStart()+1);
      else if (cur_range.GetEnd()-1 == val) cur_range.SetEnd(cur_range.GetEnd()-1);
      else {
        // Need to split the range.
        range_set.insert(range_set.begin()+id+1, IndexRange{val+1,cur_range.GetEnd()});
        cur_range.SetEnd(val);
      }
    }
  };

  /// @brief  A class to maintain a set of indices with a bit vector to represent them.
  class IndexBits {
    emp::BitVector bits;
    size_t offset = 0;   // Always a multiple of 64.

    // Figure out the best offset for a given value.
    size_t _CalcOffset(size_t val) const {
      return (val >> 6) << 6;
    }

    /// @brief Increase the range of valid values
    /// @param val Value to make sure can be set.
    void _ExpandRange(size_t val) {
      if (bits.GetSize() == 0) {                       // Must setup bits
        offset = _CalcOffset(val);
        bits.Resize(64);
      }
      else if (val < offset) {                         // Value is before offset...
        const size_t new_offset = _CalcOffset(val);
        bits.PushFront(offset - new_offset);
        offset = new_offset;
      }
      else if (bits.GetSize() <= val-offset) {         // Value is out of range...
        bits.Resize(_CalcOffset(val) + 64 - offset);
      }
    }
  public:
    IndexBits() = default;
    IndexBits(const IndexBits &) = default;
    IndexBits(IndexBits &&) = default;
    IndexBits(size_t min_val, size_t max_val) : offset(_CalcOffset(min_val)) {
      bits.Resize(_CalcOffset(max_val) + 64 - offset);
    }

    IndexBits & operator=(const IndexBits &) = default;
    IndexBits & operator=(IndexBits &&) = default;

    bool Has(size_t val) const { return (val < offset) ? false : bits[val-offset]; }
    size_t GetStart() const { return static_cast<size_t>(bits.FindOne()) + offset; }
    size_t GetEnd() const { return static_cast<size_t>(bits.FindMaxOne()) + offset; }
    size_t GetNumRanges() const {
      return (bits & ~(bits >> 1)).CountOnes();
    }
    size_t GetSize() const { return bits.CountOnes(); }
    bool Insert(size_t val) {
      _ExpandRange(val); // Make sure there is room for the new value.
      bits.Set(val-offset);
    }
    bool Insert(IndexRange in) {
      _ExpandRange(in.GetStart());
      _ExpandRange(in.GetEnd());
      bits.SetRange(in.GetStart()-offset, in.GetEnd()-offset);
    }
    bool Remove(size_t val) {
      bits.Clear(val - offset);
    }
  };

  /// @brief IndexSet maintains a collection of indices that can be easily manipulated.
  /// It will try to adjust representation to maintain speed and memory efficiency
  class IndexSet {
  private:
    // For zero to three entries, it will maintain values directly.
    // For more than four entries it will use either bits or ranges based on how
    // packed the values are into ranges.
    enum class index_t { NONE=0, VALS1, VALS2, VALS3, RANGES, BITS };
    struct _Index_Vals { size_t id1; size_t id2; size_t id3; };  // Few values

    union {
      _Index_Vals vals;
      IndexRangeSet ranges;
      IndexBits bits;
    };
    index_t type = index_t::NONE;

    // --- Helper functions ---

    /// Free whatever type we currently have.
    void _ReleaseUnion() {
      if (type == index_t::BITS) bits.~IndexBits();
      else if (type == index_t::RANGES) ranges.~IndexRangeSet();
    }

    /// Convert the internal representation to use bits.
    void _ToBits() {
      emp_assert(type != index_t::NONE, "Cannot start IndexSet as type BITS");
      if (type == index_t::BITS) return; // Already bits!

      IndexBits new_bits(GetMin(), GetMax());

      switch (type) {
        case index_t::VALS3:  new_bits.Insert(vals.id3);  [[fallthrough]];
        case index_t::VALS2:  new_bits.Insert(vals.id2);  [[fallthrough]];
        case index_t::VALS1:  new_bits.Insert(vals.id1);
          break;
        case index_t::RANGES:
          for (const auto & range : ranges.GetRanges()) {
            new_bits.Insert(range);
          }
          break;
      }

      _ReleaseUnion();
      new (&bits) IndexBits(std::move(new_bits));
    }

    /// Convert the internal representation to use a ranges.
    void _ToRanges() {
      emp_assert(type != index_t::NONE, "Cannot start IndexSet as type RANGES");
      if (type == index_t::RANGES) return; // Already ranges format!

      IndexRangeSet new_ranges;

      switch (type) {
        case index_t::VALS3:  new_ranges.Insert(vals.id3);  [[fallthrough]];
        case index_t::VALS2:  new_ranges.Insert(vals.id2);  [[fallthrough]];
        case index_t::VALS1:  new_ranges.Insert(vals.id1);
          break;
        case index_t::BITS:
          for (const auto & range : ranges.GetRanges()) {
            new_ranges.Insert(range);
          }
          break;
      }

      _ReleaseUnion();
      new (&bits) IndexBits(std::move(new_bits));

    }

  public:
    static constexpr const size_t npos = static_cast<size_t>(-1);

    IndexSet() = default;
    ~IndexSet() {
      switch (type) {
        case index_t::ARRAY:
          ids.array.ids.DeleteArray();
          break;
        case index_t::BITS:
          ids.bits.bits.DeleteArray();
          break;
      }
    }

    size_t GetSize() const {
      switch (type) {
        case index_t::NONE: return 0;
        case index_t::VALS1: return 1;
        case index_t::VALS2: return 2;
        case index_t::VALS3: return 3;
        case index_t::RANGE: return ids.range.end - ids.range.start;
        case index_t::ARRAY: return ids.array.num_ids;
        case index_t::BITS:
          size_t count = 0;
          for (size_t i = 0; i < ids.bits.num_fields; ++i) {
            count += emp::count_bits(ids.bits.bits[i]);
          }
          return count;
      }
    }

    bool Has(size_t id) const {
      switch (type) {
        case index_t::NONE: return false;
        case index_t::VALS1: return ids.vals.id1 == id;
        case index_t::VALS2: return ids.vals.id1 == id || ids.vals.id2 == id;
        case index_t::VALS3: return ids.vals.id1 == id || ids.vals.id2 == id || ids.vals.id3 == id;
        case index_t::RANGE: return id >= ids.range.start && id < ids.range.end;
        case index_t::ARRAY: return ids.array.num_ids;
        case index_t::BITS: {
          if (id < ids.bits.offset) return false;
          id = id - ids.bits.offset;
          const size_t field = id / NUM_FIELD_BITS;
          if (field >= ids.bits.num_fields) return false;
          const size_t shift = id % NUM_FIELD_BITS;
          return (ids.bits.bits[field] >> shift) & 1;
        }
      }
    }

    size_t GetMin() const {
      switch (type) {
        case index_t::NONE: return npos;
        case index_t::VALS1: return ids.vals.id1;
        case index_t::VALS2: return ids.vals.id1;
        case index_t::VALS3: return ids.vals.id1;
        case index_t::RANGE: return ids.range.start;
        case index_t::ARRAY: return ids.array.ids[0];
        case index_t::BITS:
          return emp::find_bit(ids.bits.bits[0]) + ids.bits.offset;
      }
    }

    size_t GetMax() const {
      switch (type) {
        case index_t::NONE: return npos;
        case index_t::VALS1: return ids.vals.id1;
        case index_t::VALS2: return ids.vals.id2;
        case index_t::VALS3: return ids.vals.id3;
        case index_t::RANGE: return ids.range.end - 1;
        case index_t::ARRAY: return ids.array.ids[ids.array.num_ids-1];
        case index_t::BITS: {
          const size_t field_id = ids.bits.num_fields - 1;
          const size_t offset = field_id * NUM_FIELD_BITS + ids.bits.offset;
          return emp::find_last_bit(ids.bits.bits[field_id]) + offset;
        }
      }
    }

    // Are all of the indicies one after the next?
    bool IsConsecutive() const {
      switch (type) {
        case index_t::NONE: return true;
        case index_t::VALS1: return true;
        case index_t::VALS2: return ids.vals.id2 == ids.vals.id1+1;
        case index_t::VALS3: return ids.vals.id3 == ids.vals.id2+1 && ids.vals.id2 == ids.vals.id1+1;
        case index_t::RANGE: return true;
        case index_t::ARRAY:
        case index_t::BITS:
          return GetSize() == GetMax() - GetMin() + 1;
      }
    }

    void Set(size_t id) {
      switch (type) {
        case index_t::NONE: ids.vals.id1 = id; type=index_t::VALS1; break;
        case index_t::VALS1: if (!Has(id)) { ids.vals.id2 = id; type=index_t::VALS2; } break;
        case index_t::VALS2: if (!Has(id)) { ids.vals.id3 = id; type=index_t::VALS3; } break;
        case index_t::VALS3:
          if (!Has(id)) {
            // If current values are consecutive, try to make a range.
            if (IsConsecutive()) {
              if (id == ids.vals.id1 - 1) {
                size_t max_val = ids.vals.id3;
                ids.range.start = id;
                ids.range.end = max_val+1;
                break;
              } else if (id == ids.vals.id3 + 1) {
                size_t min_val = ids.vals.id1;
                ids.range.start = min_val;
                ids.range.end = id+1;
                break;
              }
            }

            // If we made it here, values are not consecutive.
            // For now: ASSUME we shift to BITS.
            const size_t min_val = std::min(id, ids.vals.id1);
            const size_t max_val = std::max(id, ids.vals.id3);
            const size_t num_bits = max_val - min_val + 1;
            const size_t num_fields = (num_bits / NUM_FIELD_BITS + 1) * 2;
          }
        case index_t::RANGE: return ids.range.end - ids.range.start;
        case index_t::ARRAY: return ids.array.num_ids;
        case index_t::BITS: return ids.bits.num_ids;
      }
    }
  };
}

#endif // #ifndef EMP_DATASTRUCTS_INDEXSET_HPP_INCLUDE
