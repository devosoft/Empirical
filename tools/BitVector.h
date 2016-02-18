//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
// Class: emp::BitVector
// Desc: A customized version of std::vector<bool> with additional bit magic operations
//
// To implement: append(), resize()...
//
// Note, this class is about 15-20% slower than BitSet, but is not fixed size and does
// not require knowledge of the size at compile time.
//

#ifndef EMP_BIT_VECTOR_H
#define EMP_BIT_VECTOR_H

#include <assert.h>
#include <iostream>
#include <vector>

#include "bitset_utils.h"
#include "const_utils.h"
#include "functions.h"

namespace emp {

  class BitVector {
  private:
#ifdef EMSCRIPTEN
    typedef uint32_t field_type;
#else
    // typedef uint32_t field_type;
    typedef uint64_t field_type;
#endif

    static const int FIELD_BITS = sizeof(field_type)*8;
    int num_bits;
    field_type * bit_set;

    // NOTE: due to math edge cases, when num_bits=0 a minimum of 1 field/byte is returned.
    int LastBitID() const { return num_bits & (FIELD_BITS - 1); }
    int NumFields() const { return 1 + ((num_bits - 1) / FIELD_BITS); }
    int NumBytes()  const { return 1 + ((num_bits - 1) >> 3); }

    // Setup a bit proxy so that we can use operator[] on bit sets as an lvalue.
    class BitProxy {
    private:
      BitVector & bit_vector;
      int index;
    public:
      BitProxy(BitVector & _v, int _idx) : bit_vector(_v), index(_idx) {;}
      
      BitProxy & operator=(bool b) {    // lvalue handling...
        bit_vector.Set(index, b);
        return *this;
      }
      operator bool() const {            // rvalue handling...
        return bit_vector.Get(index);
      }
    };
    friend class BitProxy;

    inline static int FieldID(const int index)  {
      assert(index >= 0);
      return index / FIELD_BITS;
    }
    inline static int FieldPos(const int index) {
      assert(index >= 0);
      return index & (FIELD_BITS-1);
    }

    inline static int Byte2Field(const int index) { return index/4; }
    inline static int Byte2FieldPos(const int index) { return (index & 3) << 3; }

    // The following function assumes that the size of the bit_set has already been adjusted
    // to be the same as the size of the one being copied and only the fields need to be
    // copied over.
    inline void RawCopy(const field_type * in_set) {
      const int NUM_FIELDS = NumFields();
      for (int i = 0; i < NUM_FIELDS; i++) bit_set[i] = in_set[i];
    }

    // Helper: call SHIFT with positive number
    void ShiftLeft(const int shift_size) {
      assert(shift_size > 0);
      const int field_shift = shift_size / FIELD_BITS;
      const int bit_shift = shift_size % FIELD_BITS;
      const int bit_overflow = FIELD_BITS - bit_shift;
      const int NUM_FIELDS = NumFields();

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
      const int last_bit_id = LastBitID();
      if (last_bit_id) { bit_set[NUM_FIELDS - 1] &= (1U << last_bit_id) - 1U; }
    }

  
    // Helper for calling SHIFT with negative number
    void ShiftRight(const int shift_size) {
      assert(shift_size > 0);
      const int field_shift = shift_size / FIELD_BITS;
      const int bit_shift = shift_size % FIELD_BITS;
      const int bit_overflow = FIELD_BITS - bit_shift;
      const int NUM_FIELDS = NumFields();
      
      // account for field_shift
      if (field_shift) {
        for (int i = 0; i < (NUM_FIELDS - field_shift); ++i) {
          bit_set[i] = bit_set[i + field_shift];
        }
        for (int i = NUM_FIELDS - field_shift; i < NUM_FIELDS; i++) bit_set[i] = 0U;
      }
  
      // account for bit_shift
      if (bit_shift) {
        for (int i = 0; i < (NUM_FIELDS - 1 - field_shift); ++i) {
          bit_set[i] >>= bit_shift;
          bit_set[i] |= (bit_set[i+1] << bit_overflow);
        }
        bit_set[NUM_FIELDS - 1 - field_shift] >>= bit_shift;
      }
    }

  public:
    BitVector(int in_num_bits=0, bool init_val=false) : num_bits(in_num_bits) {
      assert(num_bits >= 0);
      bit_set = new field_type[NumFields()];
      if (init_val) SetAll(); else Clear();
    }
    BitVector(const BitVector & in_set) : num_bits(in_set.num_bits) {
      bit_set = new field_type[NumFields()];
      RawCopy(in_set.bit_set);
    }
    BitVector(BitVector && in_set) : num_bits(in_set.num_bits) {
      bit_set = in_set.bit_set;
      in_set.bit_set = nullptr;
    }
    ~BitVector() {
      // NOTE: A move constructor may have left bit_set == nullptr
      if (bit_set) delete [] bit_set;
    }

    BitVector & operator=(const BitVector & in_set) {
      const int in_num_fields = in_set.NumFields();
      const int prev_num_fields = NumFields();
      num_bits = in_set.num_bits;

      if (in_num_fields != prev_num_fields) {
        delete [] bit_set;
	bit_set = new field_type[NumFields()];
      }

      if (num_bits > 0) RawCopy(in_set.bit_set);

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

    BitVector & Resize(int new_bits) {
      const int old_num_fields = NumFields();
      num_bits = new_bits;
      const int NUM_FIELDS = NumFields();
      
      if (NUM_FIELDS == old_num_fields) {   // We can use our existing bit field
        num_bits = new_bits;
        // If there are extra bits, zero them out.
        if (LastBitID() > 0) bit_set[NUM_FIELDS - 1] &= constant::MaskLow<field_type>(LastBitID()); 
      }

      else {  // We have to change the number of bitfields.  Resize & copy old info.
        field_type * old_bit_set = bit_set;
        bit_set = new field_type[NUM_FIELDS];
        const int min_fields = std::min(old_num_fields, NUM_FIELDS);
        for (int i = 0; i < min_fields; i++) bit_set[i] = old_bit_set[i];
        for (int i = min_fields; i < NUM_FIELDS; i++) bit_set[i] = 0U;
        delete [] old_bit_set;
      }

      return *this;
    }

    bool operator==(const BitVector & in_set) const {
      if (num_bits != in_set.num_bits) return false;

      const int NUM_FIELDS = NumFields();
      for (int i = 0; i < NUM_FIELDS; ++i) {
        if (bit_set[i] != in_set.bit_set[i]) return false;
      }
      return true;
    }
    bool operator<(const BitVector & in_set) const {
      if (num_bits != in_set.num_bits) return num_bits < in_set.num_bits;

      const int NUM_FIELDS = NumFields();
      for (int i = NUM_FIELDS-1; i >= 0; --i) {         // Start loop at the largest field.
        if (bit_set[i] == in_set.bit_set[i]) continue;  // If same, keep looking!
        return (bit_set[i] < in_set.bit_set[i]);        // Otherwise, do comparison
      }
      return false;
    }
    bool operator<=(const BitVector & in_set) const {
      if (num_bits != in_set.num_bits) return num_bits <= in_set.num_bits;

      const int NUM_FIELDS = NumFields();
      for (int i = NUM_FIELDS-1; i >= 0; --i) {         // Start loop at the largest field.
        if (bit_set[i] == in_set.bit_set[i]) continue;  // If same, keep looking!
        return (bit_set[i] < in_set.bit_set[i]);        // Otherwise, do comparison
      }
      return true;
    }
    bool operator!=(const BitVector & in_set) const { return !operator==(in_set); }
    bool operator>(const BitVector & in_set) const { return !operator<=(in_set); }
    bool operator>=(const BitVector & in_set) const { return !operator<(in_set); }


    int GetSize() const { return num_bits; }

    bool Get(int index) const {
      assert(index >= 0 && index < num_bits);
      const int field_id = FieldID(index);
      const int pos_id = FieldPos(index);
      return (bit_set[field_id] & (1U << pos_id)) != 0;
    }

    void Set(int index, bool value) {
      assert(index >= 0 && index < num_bits);
      const int field_id = FieldID(index);
      const int pos_id = FieldPos(index);
      const int pos_mask = 1U << pos_id;

      if (value) bit_set[field_id] |= pos_mask;
      else       bit_set[field_id] &= ~pos_mask;
    }

    uint8_t GetByte(int index) const {
      assert(index >= 0 && index < NumBytes());
      const int field_id = Byte2Field(index);
      const int pos_id = Byte2FieldPos(index);
      return (bit_set[field_id] >> pos_id) & 255U;
    }

    void SetByte(int index, uint8_t value) {
      assert(index >= 0 && index < NumBytes());
      const int field_id = Byte2Field(index);
      const int pos_id = Byte2FieldPos(index);
      const field_type val_uint = value;
      bit_set[field_id] = (bit_set[field_id] & ~(255U << pos_id)) | (val_uint << pos_id);
    }

    uint32_t GetUInt(int index) const {
      // @CAO Need proper assert for variable bit fields!
      // assert(index >= 0 && index < NumFields());
      return ((uint32_t *) bit_set)[index];
    }

    void SetUInt(int index, field_type value) {
      // @CAO Need proper assert for variable bit fields!
      // assert(index >= 0 && index < NumFields());
      ((uint32_t *) bit_set)[index] = value;
    }

    uint32_t GetUIntAtBit(int index) {
      // @CAO Need proper assert for non-32-size bit fields!
      // assert(index >= 0 && index < num_bits);
      const int field_id = FieldID(index);
      const int pos_id = FieldPos(index);
      if (pos_id == 0) return (uint32_t) bit_set[field_id];
      const int NUM_FIELDS = NumFields();
      return (bit_set[field_id] >> pos_id) |
        ((field_id+1 < NUM_FIELDS) ? bit_set[field_id+1] << (FIELD_BITS-pos_id) : 0);
    }

    template <int OUT_BITS>
    field_type GetValueAtBit(int index) {
      // @CAO This function needs to be generalized to return more then 32 bits.
      static_assert(OUT_BITS <= 32, "Requesting too many bits to fit in a UInt");
      return GetUIntAtBit(index) & constant::MaskLow<uint32_t>(OUT_BITS);
    }




    bool Any() const {
      const int NUM_FIELDS = NumFields();
      for (int i = 0; i < NUM_FIELDS; i++) if (bit_set[i]) return true; return false;
    }
    bool None() const { return !Any(); }
    bool All() const { return (~(*this)).None(); }


    bool operator[](int index) const { return Get(index); }
    BitProxy operator[](int index) { return BitProxy(*this, index); }

    void Clear() {
      const int NUM_FIELDS = NumFields();
      for (int i = 0; i < NUM_FIELDS; i++) bit_set[i] = 0U;
    }
    void SetAll() { 
      const int NUM_FIELDS = NumFields();
      for (int i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~(0U);
      if (LastBitID() > 0) { bit_set[NUM_FIELDS - 1] &= constant::MaskLow<field_type>(LastBitID()); }
    }

  
    void Print(std::ostream & out=std::cout) const {
      for (int i = num_bits - 1; i >= 0; i--) {
        out << Get(i);
        if (i % FIELD_BITS == 0) out << ' ';
      }
    }
    void PrintArray(std::ostream & out=std::cout) const {
      for (int i = 0; i < num_bits; i++) out << Get(i);
    }
    void PrintOneIDs(std::ostream & out=std::cout, char spacer=' ') const {
      for (int i = 0; i < num_bits; i++) { if (Get(i)) out << i << spacer; }
    }


    // Count 1's by looping through once for each bit equal to 1
    int CountOnes_Sparse() const { 
      const int NUM_FIELDS = NumFields();
      int bit_count = 0;
      for (int i = 0; i < NUM_FIELDS; i++) {
        field_type cur_field = bit_set[i];
        while (cur_field) {
          cur_field &= (cur_field-1);       // Peel off a single 1.
          bit_count++;      // And increment the counter
        }
      }
      return bit_count;
    }

    // Count 1's in semi-parallel; fastest for even 0's & 1's
    int CountOnes_Mixed() const {
      // const int NUM_FIELDS = NumFields(); 
      const int NUM_FIELDS = NumFields() * sizeof(field_type)/4; 
      uint32_t * uint_bit_set = (uint32_t *) bit_set;
      int bit_count = 0;
      for (int i = 0; i < NUM_FIELDS; i++) {
        const uint32_t v = uint_bit_set[i];
        const uint32_t t1 = v - ((v >> 1) & 0x55555555);
        const uint32_t t2 = (t1 & 0x33333333) + ((t1 >> 2) & 0x33333333);
        bit_count += (((t2 + (t2 >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24;
      }
      return bit_count;
    }

    int CountOnes() const { return CountOnes_Mixed(); }

    int FindBit() const {
      const int NUM_FIELDS = NumFields();
      int field_id = 0;
      while (field_id < NUM_FIELDS && bit_set[field_id]==0) field_id++;
      return (field_id < NUM_FIELDS) ? find_bit(bit_set[field_id]) + (field_id * FIELD_BITS) : -1;
    }

    int PopBit() {
      const int NUM_FIELDS = NumFields();
      int field_id = 0;
      while (field_id < NUM_FIELDS && bit_set[field_id]==0) field_id++;
      if (field_id == NUM_FIELDS) return -1;  // Failed to find bit!

      const int pos_found = find_bit(bit_set[field_id]);
      bit_set[field_id] &= ~(1U << pos_found);
      return pos_found + (field_id * FIELD_BITS);
    }

    int FindBit(const int start_pos) const {
      assert(start_pos >= 0);
      if (start_pos >= num_bits) return -1;
      int field_id  = FieldID(start_pos);     // What field do we start in?
      const int field_pos = FieldPos(start_pos);    // What position in that field?
      if (field_pos && (bit_set[field_id] & ~(constant::MaskLow<field_type>(field_pos)))) {  // First field hit!
        return find_bit(bit_set[field_id] & ~(constant::MaskLow<field_type>(field_pos))) + (field_id * FIELD_BITS);
      }

      // Search other fields...
      const int NUM_FIELDS = NumFields();
      if (field_pos) field_id++;
      while (field_id < NUM_FIELDS && bit_set[field_id]==0) field_id++;
      return (field_id < NUM_FIELDS) ? find_bit(bit_set[field_id]) + (field_id * FIELD_BITS) : -1;
    }
    std::vector<int> GetOnes() const {
      // @CAO -- There are probably better ways to do this with bit tricks.
      std::vector<int> out_set(CountOnes());
      int cur_pos = 0;
      for (int i = 0; i < num_bits; i++) {
        if (Get(i)) out_set[cur_pos++] = i;
      }
      return out_set;
    }
    
    // Boolean math functions...
    BitVector NOT() const {
      const int NUM_FIELDS = NumFields();
      BitVector out_set(*this);
      for (int i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = ~bit_set[i];
      if (LastBitID() > 0) out_set.bit_set[NUM_FIELDS - 1] &= constant::MaskLow<field_type>(LastBitID());
      return out_set;
    }

    BitVector AND(const BitVector & set2) const {
      const int NUM_FIELDS = NumFields();
      BitVector out_set(*this);
      for (int i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = bit_set[i] & set2.bit_set[i];
      return out_set;
    }

    BitVector OR(const BitVector & set2) const {
      const int NUM_FIELDS = NumFields();
      BitVector out_set(*this);
      for (int i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = bit_set[i] | set2.bit_set[i];
      return out_set;
    }

    BitVector NAND(const BitVector & set2) const {
      const int NUM_FIELDS = NumFields();
      BitVector out_set(*this);
      for (int i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = ~(bit_set[i] & set2.bit_set[i]);
      if (LastBitID() > 0) out_set.bit_set[NUM_FIELDS - 1] &= constant::MaskLow<field_type>(LastBitID());
      return out_set;
    }

    BitVector NOR(const BitVector & set2) const {
      const int NUM_FIELDS = NumFields();
      BitVector out_set(*this);
      for (int i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = ~(bit_set[i] | set2.bit_set[i]);
      if (LastBitID() > 0) out_set.bit_set[NUM_FIELDS - 1] &= constant::MaskLow<field_type>(LastBitID());
      return out_set;
    }

    BitVector XOR(const BitVector & set2) const {
      const int NUM_FIELDS = NumFields();
      BitVector out_set(*this);
      for (int i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = bit_set[i] ^ set2.bit_set[i];
      return out_set;
    }

    BitVector EQU(const BitVector & set2) const {
      const int NUM_FIELDS = NumFields();
      BitVector out_set(*this);
      for (int i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = ~(bit_set[i] ^ set2.bit_set[i]);
      if (LastBitID() > 0) out_set.bit_set[NUM_FIELDS - 1] &= constant::MaskLow<field_type>(LastBitID());
      return out_set;
    }
  

    // Boolean math functions...
    BitVector & NOT_SELF() {
      const int NUM_FIELDS = NumFields();
      for (int i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~bit_set[i];
      if (LastBitID() > 0) bit_set[NUM_FIELDS - 1] &= constant::MaskLow<field_type>(LastBitID());
      return *this;
    }

    BitVector & AND_SELF(const BitVector & set2) {
      const int NUM_FIELDS = NumFields();
      for (int i = 0; i < NUM_FIELDS; i++) bit_set[i] = bit_set[i] & set2.bit_set[i];
      return *this;
    }

    BitVector & OR_SELF(const BitVector & set2) {
      const int NUM_FIELDS = NumFields();
      for (int i = 0; i < NUM_FIELDS; i++) bit_set[i] = bit_set[i] | set2.bit_set[i];
      return *this;
    }

    BitVector & NAND_SELF(const BitVector & set2) {
      const int NUM_FIELDS = NumFields();
      for (int i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~(bit_set[i] & set2.bit_set[i]);
      if (LastBitID() > 0) bit_set[NUM_FIELDS - 1] &= constant::MaskLow<field_type>(LastBitID());
      return *this;
    }

    BitVector & NOR_SELF(const BitVector & set2) {
      const int NUM_FIELDS = NumFields();
      for (int i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~(bit_set[i] | set2.bit_set[i]);
      if (LastBitID() > 0) bit_set[NUM_FIELDS - 1] &= constant::MaskLow<field_type>(LastBitID());
      return *this;
    }

    BitVector & XOR_SELF(const BitVector & set2) {
      const int NUM_FIELDS = NumFields();
      for (int i = 0; i < NUM_FIELDS; i++) bit_set[i] = bit_set[i] ^ set2.bit_set[i];
      return *this;
    }

    BitVector & EQU_SELF(const BitVector & set2) {
      const int NUM_FIELDS = NumFields();
      for (int i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~(bit_set[i] ^ set2.bit_set[i]);
      if (LastBitID() > 0) bit_set[NUM_FIELDS - 1] &= constant::MaskLow<field_type>(LastBitID());
      return *this;
    }
  
    // Positive shifts go left and negative go right (0 does nothing)
    BitVector SHIFT(const int shift_size) const {
      BitVector out_set(*this);
      if (shift_size > 0) out_set.ShiftRight(shift_size);
      else if (shift_size < 0) out_set.ShiftLeft(-shift_size);
      return out_set;
    }

    BitVector & SHIFT_SELF(const int shift_size) {
      if (shift_size > 0) ShiftRight(shift_size);
      else if (shift_size < 0) ShiftLeft(-shift_size);
      return *this;
    }


    // Operator overloads...
    BitVector operator~() const { return NOT(); }
    BitVector operator&(const BitVector & ar2) const { return AND(ar2); }
    BitVector operator|(const BitVector & ar2) const { return OR(ar2); }
    BitVector operator^(const BitVector & ar2) const { return XOR(ar2); }
    BitVector operator<<(const int shift_size) const { return SHIFT(-shift_size); }
    BitVector operator>>(const int shift_size) const { return SHIFT(shift_size); }
    const BitVector & operator&=(const BitVector & ar2) { return AND_SELF(ar2); }
    const BitVector & operator|=(const BitVector & ar2) { return OR_SELF(ar2); }
    const BitVector & operator^=(const BitVector & ar2) { return XOR_SELF(ar2); }
    const BitVector & operator<<=(const int shift_size) { return SHIFT_SELF(-shift_size); }
    const BitVector & operator>>=(const int shift_size) { return SHIFT_SELF(shift_size); }

    // For compatability with std::vector<bool>.
    inline size_t size() const { return num_bits; }
    inline bool all() const { return All(); }
    inline bool any() const { return Any(); }
    inline bool none() const { return !Any(); }
    inline size_t count() const { return CountOnes_Mixed(); }
  };

}

std::ostream & operator<<(std::ostream & out, const emp::BitVector & _bit_set) {
  _bit_set.Print(out);
  return out;
}

#endif
