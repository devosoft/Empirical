//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Class: template <int NUM_BITS> emp::BitSet
//
//  Desc: This class handles a fixed-sized (but arbitrarily large) array of bits,
//        and optimizes operations on those bits to be as fast as possible.
//
//  Note: emp::BitSet is based on std::bitset, and can be used as a drop-in replacement.
//        Like std::bitset, bit zero is on the right side.  Unlike std::bitset, emp::BitSet
//        gives access to bit fields for easy access to different sized chucnk of bits and
//        implementation new bit-magic tricks.
//
//  Status: RELEASE
//
//
// Constructors:
//  BitSet()                                     -- Assume all zeroes in set
//  BitSet(const BitSet & in_set)                -- Copy Constructor
//  BitSet(emp::Random & random, double p1=0.5)  -- Build a random bitset.
//
// Assignment helpers:
//  BitSet & operator=(const BitSet & in_set)           -- Copy over a BitSet of the same size
//  void Randomize(emp::Random & random, double p1=0.5) -- Randomize all bits.
//  BitSet & Import(const BitSet & in_set)              -- Copy over a BitSet of a different size
//  BitSet Export<NEW_SIZE>()                           -- Convert this BitSet a different size
//
// Comparisons:
//  bool operator==(const BitSet & in_set) const  -- Test if all bits are identical
//  bool operator<(const BitSet & in_set) const   -- Ordering to facilitate sorting, etc.
//  bool operator<=(const BitSet & in_set) const  -- ...filling out remaining Boolean comparisons
//  bool operator>(const BitSet & in_set) const   --
//  bool operator>=(const BitSet & in_set) const  --
//  bool operator!=(const BitSet & in_set) const  --
//
// Sizing:
//  size_t GetSize() const
//
// Accessors for bits:
//  void Set(size_t index, bool value)
//  bool Get(size_t index) const
//  bool operator[](size_t index) const
//  BitProxy operator[](size_t index)
//
// Accessors for larger chunks:
//  void Clear()                                  -- Set all bits to zero
//  void SetAll()                                 -- Set all bits to one
//  uint8_t GetByte(size_t byte_id) const            -- Read a full byte of bits
//  void SetByte(size_t byte_id, uint8_t value)      -- Set a full byte of bits
//  uint32_t GetUInt(size_t uint_id) const           -- Read 32 bits at once
//  void SetUInt(size_t uint_id, uint32_t value)     -- Set 32 bits at once
//
// Printing:
//  void Print(ostream & out=cout) const          -- Print BitSet (least significant on right)
//  void PrintArray(ostream & out=cout) const     -- Print as array (index zero on left)
//  void PrintOneIDs(ostream & out=cout) const    -- Just print the IDs of the set bits.
//
// Bit analysis:
//  size_t CountOnes()
//  int FindBit(size_t start_bit)  -- Return pos of first 1 after start_bit
//  emp::vector<size_t> GetOnes()  -- Return pos of ALL ones.
//
// Boolean math functions:
//  BitSet NOT() const
//  BitSet AND(const BitSet & set2) const
//  BitSet OR(const BitSet & set2) const
//  BitSet NAND(const BitSet & set2) const
//  BitSet NOR(const BitSet & set2) const
//  BitSet XOR(const BitSet & set2) const
//  BitSet EQU(const BitSet & set2) const
//
//  const BitSet & NOT_SELF()
//  const BitSet & AND_SELF(const BitSet & set2)
//  const BitSet & OR_SELF(const BitSet & set2)
//  const BitSet & NAND_SELF(const BitSet & set2)
//  const BitSet & NOR_SELF(const BitSet & set2)
//  const BitSet & XOR_SELF(const BitSet & set2)
//  const BitSet & EQU_SELF(const BitSet & set2)
//
//  BitSet SHIFT(const int shift_size) const   -- positive for left shift, negative for right shift
//  const BitSet & SHIFT_SELF(const int shift_size) const
//
// Operator overloads:
//  BitSet operator~() const
//  BitSet operator&(const BitSet & ar2) const
//  BitSet operator|(const BitSet & ar2) const
//  BitSet operator^(const BitSet & ar2) const
//  BitSet operator>>(const size_t) const
//  BitSet operator<<(const size_t) const
//  const BitSet & operator&=(const BitSet & ar2)
//  const BitSet & operator|=(const BitSet & ar2)
//  const BitSet & operator^=(const BitSet & ar2)
//  const BitSet & operator>>=(const size_t)
//  const BitSet & operator<<=(const size_t)


#ifndef EMP_BIT_SET_H
#define EMP_BIT_SET_H

#include <iostream>

#include "../base/assert.h"
#include "../base/vector.h"

#include "bitset_utils.h"
#include "functions.h"
#include "math.h"
#include "Random.h"

namespace emp {

  template <size_t NUM_BITS> class BitSet {
  private:
    static const uint32_t NUM_FIELDS = 1 + ((NUM_BITS - 1) >> 5);
    static const uint32_t LAST_BIT = NUM_BITS & 31;
    static const uint32_t NUM_BYTES = 1 + ((NUM_BITS - 1) >> 3);

    uint32_t bit_set[NUM_FIELDS];

    // Setup a bit proxy so that we can use operator[] on bit sets as a lvalue.
    class BitProxy {
    private:
      BitSet<NUM_BITS> & bit_set;
      size_t index;
    public:
      BitProxy(BitSet<NUM_BITS> & _set, size_t _idx) : bit_set(_set), index(_idx) {
        emp_assert(_idx < bit_set.size());
      }

      BitProxy & operator=(bool b) {    // lvalue handling...
        bit_set.Set(index, b);
        return *this;
      }
      operator bool() const {            // rvalue handling...
        return bit_set.Get(index);
      }

      BitProxy & Toggle() { bit_set.Toggle(index); return *this; }
    };
    friend class BitProxy;

    inline static size_t FieldID(const size_t index) {
      emp_assert((index >> 5) < NUM_FIELDS);
      return index >> 5;
    }
    inline static size_t FieldPos(const size_t index) { return index & 31; }

    inline static size_t Byte2Field(const size_t index) { return index/4; }
    inline static size_t Byte2FieldPos(const size_t index) { return (index & 3) << 3; }

    inline void Copy(const uint32_t in_set[NUM_FIELDS]) {
      for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = in_set[i];
    }

    // Helper: call SHIFT with positive number instead
    void ShiftLeft(const uint32_t shift_size) {
      const int field_shift = shift_size / 32;
      const int bit_shift = shift_size % 32;
      const int bit_overflow = 32 - bit_shift;

      // Loop through each field, from L to R, and update it.
      if (field_shift) {
        for (int i = NUM_FIELDS - 1; i >= field_shift; --i) {
          bit_set[i] = bit_set[i - field_shift];
        }
        for (int i = field_shift - 1; i >= 0; i--) bit_set[i] = 0;
      }

      // account for bit_shift
      if (bit_shift) {
        for (int i = NUM_FIELDS - 1; i > field_shift; --i) {
          bit_set[i] <<= bit_shift;
          bit_set[i] |= (bit_set[i-1] >> bit_overflow);
        }
        // Handle final field (field_shift position)
        bit_set[field_shift] <<= bit_shift;
      }

      // Mask out any bits that have left-shifted away
      if (LAST_BIT) { bit_set[NUM_FIELDS - 1] &= (1U << LAST_BIT) - 1U; }
    }


    // Helper for calling SHIFT with negative number
    void ShiftRight(const uint32_t shift_size) {
      emp_assert(shift_size > 0);
      const uint32_t field_shift = shift_size / 32;
      const uint32_t bit_shift = shift_size % 32;
      const uint32_t bit_overflow = 32 - bit_shift;

      // account for field_shift
      if (field_shift) {
        for (size_t i = 0; i < (NUM_FIELDS - field_shift); ++i) {
          bit_set[i] = bit_set[i + field_shift];
        }
        for (size_t i = NUM_FIELDS - field_shift; i < NUM_FIELDS; i++) bit_set[i] = 0;
      }

      // account for bit_shift
      if (bit_shift) {
        for (size_t i = 0; i < (NUM_FIELDS - 1 - field_shift); ++i) {
          bit_set[i] >>= bit_shift;
          bit_set[i] |= (bit_set[i+1] << bit_overflow);
        }
        bit_set[NUM_FIELDS - 1 - field_shift] >>= bit_shift;
      }
    }

  public:
    /// Constructor: Assume all zeroes in set
    BitSet() { Clear(); }
    BitSet(const BitSet & in_set) { Copy(in_set.bit_set); }
    BitSet(Random & random, const double p1=0.5) { Randomize(random, p1); }
    ~BitSet() = default;

    BitSet & operator=(const BitSet<NUM_BITS> & in_set) {
      Copy(in_set.bit_set);
      return *this;
    }

    void Randomize(Random & random, const double p1=0.5) {
      for (size_t i = 0; i < NUM_BITS; i++) Set(i, random.P(p1));
    }

    // Assign from a BitSet of a different size.
    template <size_t NUM_BITS2>
    BitSet & Import(const BitSet<NUM_BITS2> & in_set) {
      static const size_t NUM_FIELDS2 = 1 + ((NUM_BITS2 - 1) >> 5);
      static const size_t MIN_FIELDS = (NUM_FIELDS < NUM_FIELDS2) ? NUM_FIELDS : NUM_FIELDS2;
      for (size_t i = 0; i < MIN_FIELDS; i++) bit_set[i] = in_set.GetUInt(i);  // Copy avail fields
      for (size_t i = MIN_FIELDS; i < NUM_FIELDS; i++) bit_set[i] = 0;         // Zero extra fields
      return *this;
    }

    // Convert to a Bitset of a different size.
    template <size_t NUM_BITS2>
    BitSet<NUM_BITS2> Export() const {
      static const size_t NUM_FIELDS2 = 1 + ((NUM_BITS2 - 1) >> 5);
      static const size_t MIN_FIELDS = (NUM_FIELDS < NUM_FIELDS2) ? NUM_FIELDS : NUM_FIELDS2;
      BitSet<NUM_BITS2> out_bits;
      for (size_t i = 0; i < MIN_FIELDS; i++) out_bits.SetUInt(i, bit_set[i]);  // Copy avail fields
      for (size_t i = MIN_FIELDS; i < NUM_FIELDS; i++) out_bits.SetUInt(i, 0);  // Zero extra fields
      return out_bits;
    }

    bool operator==(const BitSet & in_set) const {
      for (size_t i = 0; i < NUM_FIELDS; ++i) {
        if (bit_set[i] != in_set.bit_set[i]) return false;
      }
      return true;
    }
    bool operator<(const BitSet & in_set) const {
      for (int i = NUM_FIELDS-1; i >= 0; --i) {         // Start loop at the largest field.
        if (bit_set[i] == in_set.bit_set[i]) continue;  // If same, keep looking!
        return (bit_set[i] < in_set.bit_set[i]);        // Otherwise, do comparison
      }
      return false;
    }
    bool operator<=(const BitSet & in_set) const {
      for (int i = NUM_FIELDS-1; i >= 0; --i) {         // Start loop at the largest field.
        if (bit_set[i] == in_set.bit_set[i]) continue;  // If same, keep looking!
        return (bit_set[i] < in_set.bit_set[i]);        // Otherwise, do comparison
      }
      return true;
    }
    bool operator!=(const BitSet & in_set) const { return !operator==(in_set); }
    bool operator>(const BitSet & in_set) const { return !operator<=(in_set); }
    bool operator>=(const BitSet & in_set) const { return !operator<(in_set); }

    constexpr static size_t GetSize() { return NUM_BITS; }

    bool Get(size_t index) const {
      emp_assert(index >= 0 && index < NUM_BITS);
      const size_t field_id = FieldID(index);
      const size_t pos_id = FieldPos(index);
      return (bit_set[field_id] & (1 << pos_id)) != 0;
    }

    void Set(size_t index, bool value) {
      emp_assert(index < NUM_BITS);
      const size_t field_id = FieldID(index);
      const size_t pos_id = FieldPos(index);
      const uint32_t pos_mask = 1 << pos_id;

      if (value) bit_set[field_id] |= pos_mask;
      else       bit_set[field_id] &= ~pos_mask;
    }

    /// Flip all bits in this BitSet
    BitSet & Toggle() { return NOT_SELF(); }

    /// Flip a single bit
    BitSet & Toggle(size_t index) {
      emp_assert(index >= 0 && index < NUM_BITS);
      const size_t field_id = FieldID(index);
      const size_t pos_id = FieldPos(index);
      (bit_set[field_id] ^= (1 << pos_id));
      return *this;
    }

    /// Flips all the bits in a range [start, end)
    BitSet & Toggle(size_t start, size_t end) {
      emp_assert(start <= end && end <= NUM_BITS);
      for(size_t index = start; index < end; index++) {
        Toggle(index);
      }
      return *this;
    }

    uint8_t GetByte(size_t index) const {
      emp_assert(index < NUM_BYTES);
      const size_t field_id = Byte2Field(index);
      const size_t pos_id = Byte2FieldPos(index);
      return (bit_set[field_id] >> pos_id) & 255;
    }

    void SetByte(size_t index, uint8_t value) {
      emp_assert(index < NUM_BYTES);
      const size_t field_id = Byte2Field(index);
      const size_t pos_id = Byte2FieldPos(index);
      const uint32_t val_uint = value;
      bit_set[field_id] = (bit_set[field_id] & ~(255U << pos_id)) | (val_uint << pos_id);
    }

    uint32_t GetUInt(size_t index) const {
      emp_assert(index < NUM_FIELDS);
      return bit_set[index];
    }

    void SetUInt(size_t index, uint32_t value) {
      emp_assert(index < NUM_FIELDS);
      bit_set[index] = value;
    }

    uint32_t GetUIntAtBit(size_t index) {
      emp_assert(index < NUM_BITS);
      const size_t field_id = FieldID(index);
      const size_t pos_id = FieldPos(index);
      if (pos_id == 0) return bit_set[field_id];
      return (bit_set[field_id] >> pos_id) |
        ((field_id+1 < NUM_FIELDS) ? bit_set[field_id+1] << (32-pos_id) : 0);
    }

    template <size_t OUT_BITS>
    uint32_t GetValueAtBit(size_t index) {
      static_assert(OUT_BITS <= 32, "Requesting too many bits to fit in a UInt");
      return GetUIntAtBit(index) & MaskLow<uint32_t>(OUT_BITS);
    }


    bool Any() const { for (auto i : bit_set) if (i) return true; return false; }
    bool None() const { return !Any(); }
    bool All() const { return (~(*this)).None(); }


    bool operator[](size_t index) const { return Get(index); }
    BitProxy operator[](size_t index) { return BitProxy(*this, index); }

    void Clear() { for (auto & i : bit_set) i = 0U; }
    void SetAll() {
      for (auto & i : bit_set) i = ~0U;
      if (LAST_BIT > 0) { bit_set[NUM_FIELDS - 1] &= MaskLow<uint32_t>(LAST_BIT); }
    }


    void Print(std::ostream & out=std::cout) const {
      for (size_t i = NUM_BITS; i > 0; i--) { out << Get(i-1); }
    }
    void PrintArray(std::ostream & out=std::cout) const {
      for (size_t i = 0; i < NUM_BITS; i++) out << Get(i);
    }
    void PrintOneIDs(std::ostream & out=std::cout, char spacer=' ') const {
      for (size_t i = 0; i < NUM_BITS; i++) { if (Get(i)) out << i << spacer; }
    }


    // Count 1's by looping through once for each bit equal to 1
    size_t CountOnes_Sparse() const {
      size_t bit_count = 0;
      for (auto i : bit_set) {
        while (i) {
          i &= (i-1);       // Peel off a single 1.
          bit_count++;      // And increment the counter
        }
      }
      return bit_count;
    }

    // Count 1's in semi-parallel; fastest for even 0's & 1's
    size_t CountOnes_Mixed() const {
      size_t bit_count = 0;
      for (const auto v : bit_set) {
        const uint32_t t1 = v - ((v >> 1) & 0x55555555);
        const uint32_t t2 = (t1 & 0x33333333) + ((t1 >> 2) & 0x33333333);
        bit_count += (((t2 + (t2 >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24;
      }
      return bit_count;
    }

    size_t CountOnes() const { return CountOnes_Mixed(); }

    int FindBit() const {
      size_t field_id = 0;
      while (field_id < NUM_FIELDS && bit_set[field_id]==0) field_id++;
      return (field_id < NUM_FIELDS) ? (int) (find_bit(bit_set[field_id]) + (field_id << 5)) : -1;
    }

    int PopBit() {
      size_t field_id = 0;
      while (field_id < NUM_FIELDS && bit_set[field_id]==0) field_id++;
      if (field_id == NUM_FIELDS) return -1;  // Failed to find bit!

      const int pos_found = (int) find_bit(bit_set[field_id]);
      bit_set[field_id] &= ~(1U << pos_found);
      return pos_found + (int)(field_id << 5);
    }


    int FindBit(const size_t start_pos) const {
      // @CAO -- There are better ways to do this with bit tricks
      //         (but start_pos is tricky...)
      for (size_t i = start_pos; i < NUM_BITS; i++) {
        if (Get(i)) return (int) i;
      }
      return -1;
    }
    emp::vector<size_t> GetOnes() const {
      // @CAO -- There are better ways to do this with bit tricks.
      emp::vector<size_t> out_set(CountOnes());
      size_t cur_pos = 0;
      for (size_t i = 0; i < NUM_BITS; i++) {
        if (Get(i)) out_set[cur_pos++] = i;
      }
      return out_set;
    }

    // Boolean math functions...
    BitSet NOT() const {
      BitSet out_set(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = ~bit_set[i];
      if (LAST_BIT > 0) out_set.bit_set[NUM_FIELDS - 1] &= MaskLow<uint32_t>(LAST_BIT);
      return out_set;
    }

    BitSet AND(const BitSet & set2) const {
      BitSet out_set(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = bit_set[i] & set2.bit_set[i];
      return out_set;
    }

    BitSet OR(const BitSet & set2) const {
      BitSet out_set(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = bit_set[i] | set2.bit_set[i];
      return out_set;
    }

    BitSet NAND(const BitSet & set2) const {
      BitSet out_set(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = ~(bit_set[i] & set2.bit_set[i]);
      if (LAST_BIT > 0) out_set.bit_set[NUM_FIELDS - 1] &= MaskLow<uint32_t>(LAST_BIT);
      return out_set;
    }

    BitSet NOR(const BitSet & set2) const {
      BitSet out_set(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = ~(bit_set[i] | set2.bit_set[i]);
      if (LAST_BIT > 0) out_set.bit_set[NUM_FIELDS - 1] &= MaskLow<uint32_t>(LAST_BIT);
      return out_set;
    }

    BitSet XOR(const BitSet & set2) const {
      BitSet out_set(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = bit_set[i] ^ set2.bit_set[i];
      return out_set;
    }

    BitSet EQU(const BitSet & set2) const {
      BitSet out_set(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = ~(bit_set[i] ^ set2.bit_set[i]);
      if (LAST_BIT > 0) out_set.bit_set[NUM_FIELDS - 1] &= MaskLow<uint32_t>(LAST_BIT);
      return out_set;
    }


    // Boolean math functions...
    BitSet & NOT_SELF() {
      for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~bit_set[i];
      if (LAST_BIT > 0) bit_set[NUM_FIELDS - 1] &= MaskLow<uint32_t>(LAST_BIT);
      return *this;
    }

    BitSet & AND_SELF(const BitSet & set2) {
      for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = bit_set[i] & set2.bit_set[i];
      return *this;
    }

    BitSet & OR_SELF(const BitSet & set2) {
      for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = bit_set[i] | set2.bit_set[i];
      return *this;
    }

    BitSet & NAND_SELF(const BitSet & set2) {
      for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~(bit_set[i] & set2.bit_set[i]);
      if (LAST_BIT > 0) bit_set[NUM_FIELDS - 1] &= MaskLow<uint32_t>(LAST_BIT);
      return *this;
    }

    BitSet & NOR_SELF(const BitSet & set2) {
      for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~(bit_set[i] | set2.bit_set[i]);
      if (LAST_BIT > 0) bit_set[NUM_FIELDS - 1] &= MaskLow<uint32_t>(LAST_BIT);
      return *this;
    }

    BitSet & XOR_SELF(const BitSet & set2) {
      for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = bit_set[i] ^ set2.bit_set[i];
      return *this;
    }

    BitSet & EQU_SELF(const BitSet & set2) {
      for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~(bit_set[i] ^ set2.bit_set[i]);
      if (LAST_BIT > 0) bit_set[NUM_FIELDS - 1] &= MaskLow<uint32_t>(LAST_BIT);
      return *this;
    }

    // Positive shifts go left and negative go right (0 does nothing)
    BitSet SHIFT(const int shift_size) const {
      BitSet out_set(*this);
      if (shift_size > 0) out_set.ShiftRight((uint32_t) shift_size);
      else if (shift_size < 0) out_set.ShiftLeft((uint32_t) (-shift_size));
      return out_set;
    }

    BitSet & SHIFT_SELF(const int shift_size) {
      if (shift_size > 0) ShiftRight((uint32_t) shift_size);
      else if (shift_size < 0) ShiftLeft((uint32_t) -shift_size);
      return *this;
    }


    // Operator overloads...
    BitSet operator~() const { return NOT(); }
    BitSet operator&(const BitSet & ar2) const { return AND(ar2); }
    BitSet operator|(const BitSet & ar2) const { return OR(ar2); }
    BitSet operator^(const BitSet & ar2) const { return XOR(ar2); }
    BitSet operator<<(const size_t shift_size) const { return SHIFT(-(int)shift_size); }
    BitSet operator>>(const size_t shift_size) const { return SHIFT((int)shift_size); }
    const BitSet & operator&=(const BitSet & ar2) { return AND_SELF(ar2); }
    const BitSet & operator|=(const BitSet & ar2) { return OR_SELF(ar2); }
    const BitSet & operator^=(const BitSet & ar2) { return XOR_SELF(ar2); }
    const BitSet & operator<<=(const size_t shift_size) { return SHIFT_SELF(-(int)shift_size); }
    const BitSet & operator>>=(const size_t shift_size) { return SHIFT_SELF((int)shift_size); }

    // For compatability with std::bitset.
    constexpr static size_t size() { return NUM_BITS; }
    inline bool all() const { return All(); }
    inline bool any() const { return Any(); }
    inline bool none() const { return !Any(); }
    inline size_t count() const { return CountOnes_Mixed(); }
    inline BitSet & flip() { return Toggle(); }
    inline BitSet & flip(size_t pos) { return Toggle(pos); }
    inline BitSet & flip(size_t start, size_t end) { return Toggle(start, end); }
  };

  template <size_t NUM_BITS1, size_t NUM_BITS2>
  BitSet<NUM_BITS1+NUM_BITS2> join(const BitSet<NUM_BITS1> & in1, const BitSet<NUM_BITS2> & in2) {
    BitSet<NUM_BITS1+NUM_BITS2> out_bits;
    out_bits.Import(in2);
    out_bits <<= NUM_BITS1;
    out_bits |= in2.template Export<NUM_BITS1+NUM_BITS2>();
  }

}

template <size_t NUM_BITS> std::ostream & operator<<(std::ostream & out, const emp::BitSet<NUM_BITS> & _bit_set) {
  _bit_set.Print(out);
  return out;
}


#endif
