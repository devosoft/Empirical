/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2023.
 *
 *  @file IndexSet.hpp
 *  @brief Collection of indices, ideally optimized for memory size.
 *  @note Status: ALPHA
 */

#ifndef EMP_DATASTRUCTS_INDEXMAP_HPP_INCLUDE
#define EMP_DATASTRUCTS_INDEXMAP_HPP_INCLUDE

#include "../base/Ptr.hpp"
#include "../bits/bitset_utils.hpp"

namespace emp {

  /// IndexSet maintains a collection of indices that can be easily manipulated.  Ideally it
  /// will be memory efficient, based on how many ids and how densely packed there are.

  // Assumptions that must be kept true internally:
  //   For value and array representations, indices must be kept sorted.
  //   For bits representation, the first field must always have at least one index.

  class IndexSet {
  private:
    // Various methods for holding values depending on how many values we have and how
    // they are organized.  Sparse is slow, so triggers only if we have less than 1:128
    // indices in set.  Revert back to dense if more than 1:64 indics in set.
    static constexpr size_t SPARSE_THRESHOLD = 128;
    static constexpr size_t DENSE_THRESHOLD = 64;

    struct _Index_Vals { size_t id1; size_t id2; size_t id3; };  // Few values
    struct _Index_Range { size_t start; size_t end; };           // Contiguous values
    struct _Index_Array { size_t num_ids; size_t capacity; emp::Ptr<size_t> ids; };  // Sparse
    struct _Index_Bits { size_t num_fields; size_t offset; emp::Ptr<size_t> bits; }; // Dense
    enum class index_t { NONE=0, VALS1, VALS2, VALS3, RANGE, ARRAY, BITS };

    union SetOptions {
      _Index_Vals vals;
      _Index_Range range;
      _Index_Array array;
      _Index_Bits bits;
      ~SetOptions() { }
    } ids;
    index_t type = index_t::NONE;

    // --- Helper functions ---

    /// Set an individual bit in a series of bit fields.
    static void _SetBit(emp::Ptr<size_t> bits, size_t id) {
      bits[id / NUM_FIELD_BITS] |= (1 << (id % NUM_FIELD_BITS));
    }

    /// Convert the internal representation to use bits.
    void _ToBits() {
      emp_assert(type != index_t::NONE, "Cannot start IndexSet as type BITS");
      if (type == index_t::BITS) return; // Already bits!

      const size_t offset = GetMin();
      const size_t num_bits = GetMax() - offset + 1;
      // Make twice as many fields as we need to support growth.
      const size_t num_fields = (num_bits / NUM_FIELD_BITS + 1) * 2;
      emp::Ptr<size_t> bits = NewArrayPtr<size_t>(num_fields);

      switch (type) {
        case index_t::VALS3:
          _SetBit(bits, ids.vals.id3 - offset);
          [[fallthrough]];
        case index_t::VALS2:
          _SetBit(bits, ids.vals.id2 - offset);
          [[fallthrough]];
        case index_t::VALS1:
          _SetBit(bits, ids.vals.id1 - offset);
          break;
        case index_t::RANGE:
          // @CAO For a large range being converted, this can be optimized.
          for (size_t id = ids.range.start; id < ids.range.end; ++id) {
            _SetBit(bits, id - offset);
          }
          break;
        case index_t::ARRAY:
          for (size_t i = 0; i < ids.array.num_ids; ++i) {
            _SetBit(bits, ids.array.ids[i] - offset);
          }
      }

      ids.bits = _Index_Bits{ num_fields, offset, bits };
    }

    /// Convert the internal representation to use an array.
    void _ToArray() {
      emp_assert(type != index_t::NONE, "Cannot start IndexSet as type ARRAY");
      if (type == index_t::ARRAY) return; // Already array format!

      const size_t num_vals = GetSize();
      const size_t capacity = num_vals * 2;
      emp::Ptr<size_t> array = emp::NewArrayPtr<size_t>(capacity);
      size_t pos = 0;

      switch (type) {
        case index_t::VALS1:
          array[pos++] = ids.vals.id1;
          break;
        case index_t::VALS2:
          array[pos++] = ids.vals.id1;
          array[pos++] = ids.vals.id2;
          break;
        case index_t::VALS3:
          array[pos++] = ids.vals.id1;
          array[pos++] = ids.vals.id2;
          array[pos++] = ids.vals.id3;
          break;
        case index_t::RANGE:
          for (size_t id = ids.range.start; id < ids.range.end; ++id) {
            array[pos++] = id;
          }
          break;
        case index_t::BITS:
          for (size_t field_id = 0; field_id < ids.bits.num_fields; ++field_id) {
            size_t field = ids.bits.bits[field_id];
            const size_t offset = ids.bits.offset + field_id * emp::NUM_FIELD_BITS;
            while (field) {
              array[pos++] = emp::pop_bit(field) + offset;
            }
          }
      }
      ids.array = _Index_Array{num_vals, capacity, array};
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

#endif // #ifndef EMP_DATASTRUCTS_INDEXMAP_HPP_INCLUDE
