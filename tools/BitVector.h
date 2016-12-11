//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
// Class: emp::BitVector
// Desc: A customized version of std::vector<bool> with additional bit magic operations
//
// To implement: append(), resize()...
//
// Note, this class is about 15-20% slower than emp::BitSet, but is not fixed size and does
// not require knowledge of the size at compile time.
//

#ifndef EMP_BIT_VECTOR_H
#define EMP_BIT_VECTOR_H

#include <iostream>
#include <vector>

#include "assert.h"
#include "bitset_utils.h"
#include "functions.h"
#include "math.h"

namespace emp {

  class BitVector {
  private:
#ifdef EMSCRIPTEN
    using field_t = uint32_t;
#else
    using field_t = uint64_t;
#endif

    static constexpr int FIELD_BITS = sizeof(field_t)*8;
    size_t num_bits;
    field_t * bit_set;

    size_t LastBitID() const { return num_bits & (FIELD_BITS - 1); }
    size_t NumFields() const { return num_bits ? (1 + ((num_bits - 1) / FIELD_BITS)) : 0; }
    size_t NumBytes()  const { return num_bits ? (1 + ((num_bits - 1) >> 3)) : 0; }
    size_t NumSizeFields() const { return NumFields() * sizeof(field_t) / sizeof(std::size_t); }

    // Setup a bit proxy so that we can use operator[] on bit sets as an lvalue.
    class BitProxy {
    private:
      BitVector & bit_vector;
      size_t index;
    public:
      BitProxy(BitVector & _v, size_t _idx) : bit_vector(_v), index(_idx) {;}

      BitProxy & operator=(bool b) {    // lvalue handling...
        bit_vector.Set(index, b);
        return *this;
      }
      operator bool() const {            // rvalue handling...
        return bit_vector.Get(index);
      }
    };
    friend class BitProxy;

    emp_constexpr static size_t FieldID(const size_t index)  { return index / FIELD_BITS; }
    emp_constexpr static size_t FieldPos(const size_t index) { return index & (FIELD_BITS-1); }

    static constexpr size_t Byte2Field(const size_t index) { return index/sizeof(field_t); }
    static constexpr size_t Byte2FieldPos(const size_t index) {
      return (index & (sizeof(field_t)-1)) << 3;
    }

    // The following function assumes that the size of the bit_set has already been adjusted
    // to be the same as the size of the one being copied and only the fields need to be
    // copied over.
    void RawCopy(const field_t * in_set) {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = in_set[i];
    }

    // Helper: call SHIFT with positive number
    void ShiftLeft(const size_t shift_size) {
      const size_t field_shift = shift_size / FIELD_BITS;
      const size_t bit_shift = shift_size % FIELD_BITS;
      const size_t bit_overflow = FIELD_BITS - bit_shift;
      const size_t NUM_FIELDS = NumFields();

      // Loop through each field, from L to R, and update it.
      if (field_shift) {
        for (size_t i = NUM_FIELDS; i > field_shift; --i) {
          bit_set[i] = bit_set[i - field_shift - 1];
        }
        for (size_t i = field_shift; i > 0; --i) bit_set[i-1] = 0;
      }

      // account for bit_shift
      if (bit_shift) {
        for (size_t i = NUM_FIELDS - 1; i > field_shift; --i) {
          bit_set[i] <<= bit_shift;
          bit_set[i] |= (bit_set[i-1] >> bit_overflow);
        }
        // Handle final field (field_shift position)
        bit_set[field_shift] <<= bit_shift;
      }

      // Mask out any bits that have left-shifted away
      const size_t last_bit_id = LastBitID();
      if (last_bit_id) { bit_set[NUM_FIELDS - 1] &= (1U << last_bit_id) - 1U; }
    }


    // Helper for calling SHIFT with negative number
    void ShiftRight(const size_t shift_size) {
      const size_t field_shift = shift_size / FIELD_BITS;
      const size_t bit_shift = shift_size % FIELD_BITS;
      const size_t bit_overflow = FIELD_BITS - bit_shift;
      const size_t NUM_FIELDS = NumFields();
      const size_t field_shift2 = NUM_FIELDS - field_shift;

      // account for field_shift
      if (field_shift) {
        for (size_t i = 0; i < field_shift2; ++i) {
          bit_set[i] = bit_set[i + field_shift];
        }
        for (size_t i = field_shift2; i < NUM_FIELDS; i++) bit_set[i] = 0U;
      }

      // account for bit_shift
      if (bit_shift) {
        for (size_t i = 0; i < (field_shift2 - 1); ++i) {
          bit_set[i] >>= bit_shift;
          bit_set[i] |= (bit_set[i+1] << bit_overflow);
        }
        bit_set[field_shift2 - 1] >>= bit_shift;
      }
    }

  public:
    BitVector(size_t in_num_bits=0, bool init_val=false) : num_bits(in_num_bits) {
      if (num_bits) bit_set = new field_t[NumFields()];
      else bit_set = nullptr;
      if (init_val) SetAll(); else Clear();
    }
    BitVector(const BitVector & in_set) : num_bits(in_set.num_bits) {
      if (num_bits) bit_set = new field_t[NumFields()];
      else bit_set = nullptr;
      RawCopy(in_set.bit_set);
    }
    BitVector(BitVector && in_set) : num_bits(in_set.num_bits) {
      bit_set = in_set.bit_set;
      in_set.bit_set = nullptr;
    }
    ~BitVector() {
      if (bit_set) delete [] bit_set;  // A move constructor cane make bit_set == nullptr
    }

    BitVector & operator=(const BitVector & in_set) {
      const size_t in_num_fields = in_set.NumFields();
      const size_t prev_num_fields = NumFields();
      num_bits = in_set.num_bits;

      if (in_num_fields != prev_num_fields) {
        delete [] bit_set;
	      if (num_bits) bit_set = new field_t[NumFields()];
        else bit_set = nullptr;
      }

      if (num_bits) RawCopy(in_set.bit_set);

      return *this;
    }

    BitVector & operator=(BitVector && in_set) {
      if (&in_set == this) return *this;
      delete [] bit_set;
      num_bits = in_set.num_bits;
      bit_set = in_set.bit_set;
      in_set.bit_set = nullptr;

      return *this;
    }

    BitVector & Resize(size_t new_bits) {
      const size_t old_num_fields = NumFields();
      num_bits = new_bits;
      const size_t NUM_FIELDS = NumFields();

      if (NUM_FIELDS == old_num_fields) {   // We can use our existing bit field
        num_bits = new_bits;
        // If there are extra bits, zero them out.
        if (LastBitID() > 0) bit_set[NUM_FIELDS - 1] &= MaskLow<field_t>(LastBitID());
      }

      else {  // We have to change the number of bitfields.  Resize & copy old info.
        field_t * old_bit_set = bit_set;
        if (num_bits > 0) bit_set = new field_t[NUM_FIELDS];
        else bit_set = nullptr;
        const size_t min_fields = std::min(old_num_fields, NUM_FIELDS);
        for (size_t i = 0; i < min_fields; i++) bit_set[i] = old_bit_set[i];
        for (size_t i = min_fields; i < NUM_FIELDS; i++) bit_set[i] = 0U;
        delete [] old_bit_set;
      }

      return *this;
    }

    bool operator==(const BitVector & in_set) const {
      if (num_bits != in_set.num_bits) return false;

      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; ++i) {
        if (bit_set[i] != in_set.bit_set[i]) return false;
      }
      return true;
    }
    bool operator<(const BitVector & in_set) const {
      if (num_bits != in_set.num_bits) return num_bits < in_set.num_bits;

      const size_t NUM_FIELDS = NumFields();
      for (size_t i = NUM_FIELDS; i > 0; --i) {         // Start loop at the largest field.
        const size_t pos = i-1;
        if (bit_set[pos] == in_set.bit_set[pos]) continue;  // If same, keep looking!
        return (bit_set[pos] < in_set.bit_set[pos]);        // Otherwise, do comparison
      }
      return false;
    }
    bool operator<=(const BitVector & in_set) const {
      if (num_bits != in_set.num_bits) return num_bits <= in_set.num_bits;

      const size_t NUM_FIELDS = NumFields();
      for (size_t i = NUM_FIELDS; i > 0; --i) {         // Start loop at the largest field.
        const size_t pos = i-1;
        if (bit_set[pos] == in_set.bit_set[pos]) continue;  // If same, keep looking!
        return (bit_set[pos] < in_set.bit_set[pos]);        // Otherwise, do comparison
      }
      return true;
    }
    bool operator!=(const BitVector & in_set) const { return !operator==(in_set); }
    bool operator>(const BitVector & in_set) const { return !operator<=(in_set); }
    bool operator>=(const BitVector & in_set) const { return !operator<(in_set); }

    size_t GetSize() const { return num_bits; }

    bool Get(size_t index) const {
      emp_assert(index >= 0 && index < num_bits);
      const size_t field_id = FieldID(index);
      const size_t pos_id = FieldPos(index);
      return (bit_set[field_id] & (static_cast<field_t>(1) << pos_id)) != 0;
    }

    void Set(size_t index, bool value) {
      emp_assert(index >= 0 && index < num_bits);
      const size_t field_id = FieldID(index);
      const size_t pos_id = FieldPos(index);
      const field_t pos_mask = static_cast<field_t>(1) << pos_id;

      if (value) bit_set[field_id] |= pos_mask;
      else       bit_set[field_id] &= ~pos_mask;
    }

    std::size_t Hash() const {
      const size_t num_sfields = NumSizeFields();
      std::size_t hash_val = 0;
      for (size_t i = 0; i < num_sfields; i++) {
        hash_val ^= ((std::size_t*) bit_set)[i];
      }
      return hash_val ^ ((97*num_bits) << 8);
    }

    uint8_t GetByte(size_t index) const {
      emp_assert(index >= 0 && index < NumBytes());
      const size_t field_id = Byte2Field(index);
      const size_t pos_id = Byte2FieldPos(index);
      return (bit_set[field_id] >> pos_id) & 255U;
    }

    void SetByte(size_t index, uint8_t value) {
      emp_assert(index >= 0 && index < NumBytes());
      const size_t field_id = Byte2Field(index);
      const size_t pos_id = Byte2FieldPos(index);
      const field_t val_uint = value;
      bit_set[field_id] = (bit_set[field_id] & ~(static_cast<field_t>(255) << pos_id)) | (val_uint << pos_id);
    }

    uint32_t GetUInt(size_t index) const {
      // @CAO Need proper assert for variable bit fields!
      // emp_assert(index >= 0 && index < NumFields());
      return ((uint32_t *) bit_set)[index];
    }

    void SetUInt(size_t index, uint32_t value) {
      // @CAO Need proper assert for variable bit fields!
      // emp_assert(index >= 0 && index < NumFields());
      ((uint32_t *) bit_set)[index] = value;
    }

    uint32_t GetUIntAtBit(size_t index) {
      // @CAO Need proper assert for non-32-size bit fields!
      // emp_assert(index >= 0 && index < num_bits);
      const size_t field_id = FieldID(index);
      const size_t pos_id = FieldPos(index);
      if (pos_id == 0) return (uint32_t) bit_set[field_id];
      const size_t NUM_FIELDS = NumFields();
      const uint32_t part1 = (uint32_t) (bit_set[field_id] >> pos_id);
      const uint32_t part2 =
        (uint32_t)((field_id+1 < NUM_FIELDS) ? bit_set[field_id+1] << (FIELD_BITS-pos_id) : 0);
      return part1 | part2;
    }

    template <size_t OUT_BITS>
    field_t GetValueAtBit(size_t index) {
      // @CAO This function needs to be generalized to return more then sizeof(field_t)*8 bits.
      static_assert(OUT_BITS <= sizeof(field_t)*8, "Requesting too many bits to fit in a UInt");
      return GetUIntAtBit(index) & MaskLow<field_t>(OUT_BITS);
    }


    bool Any() const {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) if (bit_set[i]) return true; return false;
    }
    bool None() const { return !Any(); }
    bool All() const { return (~(*this)).None(); }


    bool operator[](size_t index) const { return Get(index); }
    BitProxy operator[](size_t index) { return BitProxy(*this, index); }

    void Clear() {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = 0U;
    }
    void SetAll() {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~(0U);
      if (LastBitID() > 0) { bit_set[NUM_FIELDS - 1] &= MaskLow<field_t>(LastBitID()); }
    }


    // Regular print function (from most significant bit to least)
    void Print(std::ostream & out=std::cout) const {
      for (size_t i = num_bits; i > 0; --i) out << Get(i-1);
    }

    // Print a space between each field
    void PrintFields(std::ostream & out=std::cout) const {
      for (size_t i = num_bits; i > 0; i--) {
        out << Get(i-1);
        if (i % FIELD_BITS == 0) out << ' ';
      }
    }

    // Print from smallest bit position to largest.
    void PrintArray(std::ostream & out=std::cout) const {
      for (size_t i = 0; i < num_bits; i++) out << Get(i);
    }

    // Print the positions of all one bits.
    void PrintOneIDs(std::ostream & out=std::cout, char spacer=' ') const {
      for (size_t i = 0; i < num_bits; i++) { if (Get(i)) out << i << spacer; }
    }


    // Count 1's by looping through once for each bit equal to 1
    int CountOnes_Sparse() const {
      const size_t NUM_FIELDS = NumFields();
      int bit_count = 0;
      for (size_t i = 0; i < NUM_FIELDS; i++) {
        field_t cur_field = bit_set[i];
        while (cur_field) {
          cur_field &= (cur_field-1);       // Peel off a single 1.
          bit_count++;      // And increment the counter
        }
      }
      return bit_count;
    }

    // Count 1's in semi-parallel; fastest for even 0's & 1's
    int CountOnes_Mixed() const {
      const size_t NUM_FIELDS = NumFields() * sizeof(field_t)/4;
      uint32_t* uint_bit_set = (uint32_t *) bit_set;
      int bit_count = 0;
      for (size_t i = 0; i < NUM_FIELDS; i++) {
        const uint32_t v = uint_bit_set[i];
        const uint32_t t1 = v - ((v >> 1) & 0x55555555);
        const uint32_t t2 = (t1 & 0x33333333) + ((t1 >> 2) & 0x33333333);
        bit_count += (((t2 + (t2 >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24;
      }
      return bit_count;
    }

    int CountOnes() const { return CountOnes_Mixed(); }

    int FindBit() const {
      const size_t NUM_FIELDS = NumFields();
      size_t field_id = 0;
      while (field_id < NUM_FIELDS && bit_set[field_id]==0) field_id++;
      return (field_id < NUM_FIELDS) ?
        (int) (find_bit(bit_set[field_id]) + (field_id * FIELD_BITS))  :  -1;
    }

    int PopBit() {
      const size_t NUM_FIELDS = NumFields();
      size_t field_id = 0;
      while (field_id < NUM_FIELDS && bit_set[field_id]==0) field_id++;
      if (field_id == NUM_FIELDS) return -1;  // Failed to find bit!

      const size_t pos_found = find_bit(bit_set[field_id]);
      bit_set[field_id] &= ~(1U << pos_found);
      return (int) (pos_found + (field_id * FIELD_BITS));
    }

    int FindBit(const size_t start_pos) const {
      if (start_pos >= num_bits) return -1;
      size_t field_id  = FieldID(start_pos);     // What field do we start in?
      const size_t field_pos = FieldPos(start_pos);    // What position in that field?
      if (field_pos && (bit_set[field_id] & ~(MaskLow<field_t>(field_pos)))) {  // First field hit!
        return (int) (find_bit(bit_set[field_id] & ~(MaskLow<field_t>(field_pos))) +
                      field_id * FIELD_BITS);
      }

      // Search other fields...
      const size_t NUM_FIELDS = NumFields();
      if (field_pos) field_id++;
      while (field_id < NUM_FIELDS && bit_set[field_id]==0) field_id++;
      return (field_id < NUM_FIELDS) ?
        (int) (find_bit(bit_set[field_id]) + (field_id * FIELD_BITS)) : -1;
    }
    std::vector<size_t> GetOnes() const {
      // @CAO -- There are probably better ways to do this with bit tricks.
      std::vector<size_t> out_set((size_t) CountOnes());
      size_t cur_pos = 0;
      for (size_t i = 0; i < num_bits; i++) {
        if (Get(i)) out_set[cur_pos++] = i;
      }
      return out_set;
    }

    // Boolean math functions...
    BitVector NOT() const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_set(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = ~bit_set[i];
      if (LastBitID() > 0) out_set.bit_set[NUM_FIELDS - 1] &= MaskLow<field_t>(LastBitID());
      return out_set;
    }

    BitVector AND(const BitVector & set2) const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_set(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = bit_set[i] & set2.bit_set[i];
      return out_set;
    }

    BitVector OR(const BitVector & set2) const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_set(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = bit_set[i] | set2.bit_set[i];
      return out_set;
    }

    BitVector NAND(const BitVector & set2) const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_set(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = ~(bit_set[i] & set2.bit_set[i]);
      if (LastBitID() > 0) out_set.bit_set[NUM_FIELDS - 1] &= MaskLow<field_t>(LastBitID());
      return out_set;
    }

    BitVector NOR(const BitVector & set2) const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_set(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = ~(bit_set[i] | set2.bit_set[i]);
      if (LastBitID() > 0) out_set.bit_set[NUM_FIELDS - 1] &= MaskLow<field_t>(LastBitID());
      return out_set;
    }

    BitVector XOR(const BitVector & set2) const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_set(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = bit_set[i] ^ set2.bit_set[i];
      return out_set;
    }

    BitVector EQU(const BitVector & set2) const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_set(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = ~(bit_set[i] ^ set2.bit_set[i]);
      if (LastBitID() > 0) out_set.bit_set[NUM_FIELDS - 1] &= MaskLow<field_t>(LastBitID());
      return out_set;
    }


    // Boolean math functions...
    BitVector & NOT_SELF() {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~bit_set[i];
      if (LastBitID() > 0) bit_set[NUM_FIELDS - 1] &= MaskLow<field_t>(LastBitID());
      return *this;
    }

    BitVector & AND_SELF(const BitVector & set2) {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = bit_set[i] & set2.bit_set[i];
      return *this;
    }

    BitVector & OR_SELF(const BitVector & set2) {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = bit_set[i] | set2.bit_set[i];
      return *this;
    }

    BitVector & NAND_SELF(const BitVector & set2) {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~(bit_set[i] & set2.bit_set[i]);
      if (LastBitID() > 0) bit_set[NUM_FIELDS - 1] &= MaskLow<field_t>(LastBitID());
      return *this;
    }

    BitVector & NOR_SELF(const BitVector & set2) {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~(bit_set[i] | set2.bit_set[i]);
      if (LastBitID() > 0) bit_set[NUM_FIELDS - 1] &= MaskLow<field_t>(LastBitID());
      return *this;
    }

    BitVector & XOR_SELF(const BitVector & set2) {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = bit_set[i] ^ set2.bit_set[i];
      return *this;
    }

    BitVector & EQU_SELF(const BitVector & set2) {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~(bit_set[i] ^ set2.bit_set[i]);
      if (LastBitID() > 0) bit_set[NUM_FIELDS - 1] &= MaskLow<field_t>(LastBitID());
      return *this;
    }

    // Positive shifts go left and negative go right (0 does nothing)
    BitVector SHIFT(const int shift_size) const {
      BitVector out_set(*this);
      if (shift_size > 0) out_set.ShiftRight((size_t) shift_size);
      else if (shift_size < 0) out_set.ShiftLeft((size_t) -shift_size);
      return out_set;
    }

    BitVector & SHIFT_SELF(const int shift_size) {
      if (shift_size > 0) ShiftRight((size_t) shift_size);
      else if (shift_size < 0) ShiftLeft((size_t) -shift_size);
      return *this;
    }


    // Operator overloads...
    BitVector operator~() const { return NOT(); }
    BitVector operator&(const BitVector & ar2) const { return AND(ar2); }
    BitVector operator|(const BitVector & ar2) const { return OR(ar2); }
    BitVector operator^(const BitVector & ar2) const { return XOR(ar2); }
    inline BitVector operator<<(const int shift_size) const { return SHIFT(-shift_size); }
    inline BitVector operator>>(const int shift_size) const { return SHIFT(shift_size); }
    const BitVector & operator&=(const BitVector & ar2) { return AND_SELF(ar2); }
    const BitVector & operator|=(const BitVector & ar2) { return OR_SELF(ar2); }
    const BitVector & operator^=(const BitVector & ar2) { return XOR_SELF(ar2); }
    const BitVector & operator<<=(const int shift_size) { return SHIFT_SELF(-shift_size); }
    const BitVector & operator>>=(const int shift_size) { return SHIFT_SELF(shift_size); }

    // For compatability with std::vector<bool>.
    size_t size() const { return num_bits; }
    void resize(std::size_t new_size) { Resize(new_size); }
    bool all() const { return All(); }
    bool any() const { return Any(); }
    bool none() const { return !Any(); }
    size_t count() const { return (size_t) CountOnes_Mixed(); }
  };

}

namespace std {
  template <>
  struct hash<emp::BitVector> {
    std::size_t operator()(const emp::BitVector & b) const {
      return b.Hash();
    }
  };
}

inline std::ostream & operator<<(std::ostream & out, const emp::BitVector & _bit_set) {
  _bit_set.Print(out);
  return out;
}

#endif
