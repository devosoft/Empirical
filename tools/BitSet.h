#ifndef EMP_BIT_SET_H
#define EMP_BIT_SET_H

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class: template <int NUM_BITS> emp::BitSet
// Desc: This class handles a fixed-sized (but arbitrarily large) array of bits,
//       and optimizes operations on those bits to be as fast as possible.
// Note: Unlike std::bitset, emp::BitSet gives access to bit fields for easy access to different
//       sized chucnk of bits and implementation new bit-magic tricks.
//
//
// Constructors:
//  BitSet()                        -- Assume all zeroes in set
//  BitSet(const BitSet & in_set)   -- Copy Constructor
//
// Assignment and equality test:
//  BitSet & operator=(const BitSet & in_set)     -- Copy over a BitSet of the same size
//  BitSet & Import(const BitSet & in_set)        -- Copy over a BitSet of a different size
//  BitSet Export<NEW_SIZE>()                     -- Convert this BitSet a different size
//  bool operator==(const BitSet & in_set) const  -- Test if all bits are the same size.
//
// Sizing:
//  int GetSize() const
//
// Accessors for bits:
//  void Set(int index, bool value)
//  bool Get(int index) const
//  bool operator[](int index) const
//  cBitProxy operator[](int index)
//
// Accessors for larger chunks:
//  void Clear()                                   -- Set all bits to zero
//  void SetAll()                                  -- Set all bits to one
//  unsigned char GetByte(int byte_id) const       -- Read a full byte of bits
//  void SetByte(int byte_id, unsigned char value) -- Set a full byte of bits
//  unsigned int GetUInt(int uint_id) const        -- Read 32 bits at once
//  void SetUInt(int uint_id, unsigned int value)  -- Set 32 bits at once
//
// Printing:
//  void Print(ostream & out=cout) const          -- Print BitSet (least significant on right)
//  void PrintArray(ostream & out=cout) const     -- Print as array (index zero on left)
//  void PrintOneIDs(ostream & out=cout) const    -- Just print the IDs of the set bits.
//
// Bit analysis:
//  int CountOnes()
//  int FindBit1(int start_bit)   -- Return pos of first 1 after start_bit 
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
//  BitSet operator>>(const int) const
//  BitSet operator<<(const int) const
//  const BitSet & operator&=(const BitSet & ar2)
//  const BitSet & operator|=(const BitSet & ar2)
//  const BitSet & operator^=(const BitSet & ar2)
//  const BitSet & operator>>=(const int)
//  const BitSet & operator<<=(const int)
//


#include <assert.h>
#include <iostream>
#include <vector>

#include "functions.h"

namespace emp {

  constexpr unsigned int UIntMaskLow(int num_bits) {
    return (num_bits == 32) ? -1 : ((1 << num_bits) - 1);
  }

  constexpr unsigned int UIntMaskHigh(int num_bits) {
    return UIntMaskLow(num_bits) << (32-num_bits);
  }

  template <int NUM_BITS> class BitSet {
  private:
    static const int NUM_FIELDS = 1 + ((NUM_BITS - 1) >> 5);
    static const int LAST_BIT = NUM_BITS & 31;
    static const int NUM_BYTES = 1 + ((NUM_BITS - 1) >> 3);

    unsigned int bit_set[NUM_FIELDS];
    
    // Setup a bit proxy so that we can use operator[] on bit sets as a lvalue.
    class cBitProxy {
    private:
      BitSet<NUM_BITS> & bit_set;
      int index;
    public:
      cBitProxy(BitSet<NUM_BITS> & _set, int _idx) : bit_set(_set), index(_idx) {;}
      
      cBitProxy & operator=(bool b) {    // lvalue handling...
        bit_set.Set(index, b);
        return *this;
      }
      operator bool() const {            // rvalue handling...
        return bit_set.Get(index);
      }
    };
    friend class cBitProxy;

    inline static int FieldID(const int index) {
      assert((index >> 5) >= 0 && (index >> 5) < NUM_FIELDS);
      return index >> 5;
    }
    inline static int FieldPos(const int index) { return index & 31; }

    inline static int Byte2Field(const int index) { return index/4; }
    inline static int Byte2FieldPos(const int index) { return (index & 3) << 3; }

    inline void Copy(const unsigned int in_set[NUM_FIELDS]) {
      for (int i = 0; i < NUM_FIELDS; i++) bit_set[i] = in_set[i];
    }

    // Helper: call SHIFT with positive number instead
    void ShiftLeft(const int shift_size) {
      assert(shift_size > 0);
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

      // mask out any bits that have left-shifted away, allowing CountBits and CountBits2 to work
      // blw: if CountBits/CountBits2 are fixed, this code should be removed as it will be redundant
      unsigned int shift_mask = 0xFFFFFFFF >> ((32 - (NUM_BITS % 32)) & 0x1F);
      bit_set[NUM_FIELDS - 1] &= shift_mask;    
    }

  
    // Helper for calling SHIFT with negative number
    void ShiftRight(const int shift_size) {
      assert(shift_size > 0);
      const int field_shift = shift_size / 32;
      const int bit_shift = shift_size % 32;
      const int bit_overflow = 32 - bit_shift;
  
      // account for field_shift
      if (field_shift) {
        for (int i = 0; i < (NUM_FIELDS - field_shift); ++i) {
          bit_set[i] = bit_set[i + field_shift];
        }
        for(int i = NUM_FIELDS - field_shift; i < NUM_FIELDS; i++) bit_set[i] = 0;
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
    BitSet() { Clear(); }
    BitSet(const BitSet & in_set) { Copy(in_set.bit_set); }
    ~BitSet() { ; }

    BitSet & operator=(const BitSet<NUM_BITS> & in_set) {
      Copy(in_set.bit_set);
      return *this;
    }

    // Assign from a BitSet of a different size.
    template <int NUM_BITS2>
    BitSet & Import(const BitSet<NUM_BITS2> & in_set) {
      static const int NUM_FIELDS2 = 1 + ((NUM_BITS2 - 1) >> 5);
      static const int MIN_FIELDS = (NUM_FIELDS < NUM_FIELDS2) ? NUM_FIELDS : NUM_FIELDS2;
      for (int i = 0; i < MIN_FIELDS; i++) bit_set[i] = in_set.GetUInt(i);  // Copy avail fields
      for (int i = MIN_FIELDS; i < NUM_FIELDS; i++) bit_set[i] = 0;         // Zero extra fields
      return *this;
    }

    // Convert to a Bitset of a different size.
    template <int NUM_BITS2>
    BitSet<NUM_BITS2> Export() const {
      static const int NUM_FIELDS2 = 1 + ((NUM_BITS2 - 1) >> 5);
      static const int MIN_FIELDS = (NUM_FIELDS < NUM_FIELDS2) ? NUM_FIELDS : NUM_FIELDS2;
      BitSet<NUM_BITS2> out_bits;
      for (int i = 0; i < MIN_FIELDS; i++) out_bits.SetUInt(i, bit_set[i]);  // Copy avail fields
      for (int i = MIN_FIELDS; i < NUM_FIELDS; i++) out_bits.SetUInt(i, 0);  // Zero extra fields
      return out_bits;
    }

    bool operator==(const BitSet & in_set) const {
      for (int i = 0; i < NUM_FIELDS; i++) {
        if (bit_set[i] != in_set.bit_set[i]) return false;
      }
      return true;
    }

    constexpr int GetSize() { return NUM_BITS; }
    constexpr size_t size() { return NUM_BITS; }  // For STL compatability.

    bool Get(int index) const {
      assert(index >= 0 && index < NUM_BITS);
      const int field_id = FieldID(index);
      const int pos_id = FieldPos(index);
      return (bit_set[field_id] & (1 << pos_id)) != 0;
    }

    void Set(int index, bool value) {
      assert(index >= 0 && index < NUM_BITS);
      const int field_id = FieldID(index);
      const int pos_id = FieldPos(index);
      const int pos_mask = 1 << pos_id;

      if (value) bit_set[field_id] |= pos_mask;
      else       bit_set[field_id] &= ~pos_mask;
    }

    unsigned char GetByte(int index) const {
      assert(index >= 0 && index < NUM_BYTES);
      const int field_id = Byte2Field(index);
      const int pos_id = Byte2FieldPos(index);
      return (bit_set[field_id] >> pos_id) & 255;
    }

    void SetByte(int index, unsigned char value) {
      assert(index >= 0 && index < NUM_BYTES);
      const int field_id = Byte2Field(index);
      const int pos_id = Byte2FieldPos(index);
      const unsigned int val_uint = value;
      bit_set[field_id] = (bit_set[field_id] & ~(255UL << pos_id)) | (val_uint << pos_id);
    }

    unsigned int GetUInt(int index) const {
      assert(index >= 0 && index < NUM_FIELDS);
      return bit_set[index];
    }

    void SetUInt(int index, unsigned int value) {
      assert(index >= 0 && index < NUM_FIELDS);
      bit_set[index] = value;
    }


    bool Any() const { for (auto i : bit_set) if (i) return true; return false; }
    bool None() const { return !Any(); }
    bool All() const { return (~(*this)).None(); }

    // For compatability with std::bitset.
    bool all() const { return All(); }
    bool any() const { return Any(); }
    bool none() const { return !Any(); }
    

    bool operator[](int index) const { return Get(index); }
    cBitProxy operator[](int index) { return cBitProxy(*this, index); }

    void Clear() { for (auto & i : bit_set) i = 0UL; }
    void SetAll() { 
      for (auto & i : bit_set) i = ~0UL;
      if (LAST_BIT > 0) { bit_set[NUM_FIELDS - 1] &= UIntMaskLow(LAST_BIT); }
    }

  
    void Print(std::ostream & out=std::cout) const {
      for (int i = NUM_BITS - 1; i >= 0; i--) {
        out << Get(i);
        // if (i % 32 == 0) out << ' ';
      }
    }
    void PrintArray(std::ostream & out=std::cout) const {
      for (int i = 0; i < NUM_BITS; i++) out << Get(i);
    }
    void PrintOneIDs(std::ostream & out=std::cout, char spacer=' ') const {
      for (int i = 0; i < NUM_BITS; i++) { if (Get(i)) out << i << spacer; }
    }


    // Count 1's by looping through once for each bit equal to 1
    int CountOnes_Sparse() const { 
      int bit_count = 0;
      for (auto i : bit_set) {
        while (i) {
          i &= (i-1);       // Peel off a single 1.
          bit_count++;      // And increment the counter
        }
      }
      return bit_count;
    }

    // Count 1's in semi-parallel; fastest for even 0's & 1's
    int CountOnes_Mixed() const {
      int bit_count = 0;
      for (const auto v : bit_set) {
        const unsigned int t1 = v - ((v >> 1) & 0x55555555);
        const unsigned int t2 = (t1 & 0x33333333) + ((t1 >> 2) & 0x33333333);
        bit_count += ((t2 + (t2 >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
      }
      return bit_count;
    }

    int CountOnes() const { return CountOnes_Mixed(); }
    int count() const { return CountOnes_Mixed(); } // For compatability with std::bitset

    int FindBit() const {
      int field_id;
      int offset = 0;
      for (field_id = 0; field_id < NUM_FIELDS; field_id++) {
        if (bit_set[field_id]) break;
        offset += 32;
      }
      return (field_id < NUM_FIELDS) ? find_bit(bit_set[field_id]) + offset : -1;
    }

    int FindBit(const int start_pos) const {
      // @CAO -- There are better ways to do this with bit tricks 
      //         (but start_pos is tricky...)
      for (int i = start_pos; i < NUM_BITS; i++) {
        if (Get(i)) return i;
      }
      return -1;
    }
    std::vector<int> GetOnes() const {
      // @CAO -- There are probably better ways to do this with bit tricks.
      std::vector<int> out_set(CountOnes());
      int cur_pos = 0;
      for (int i = 0; i < NUM_BITS; i++) {
        if (Get(i)) out_set[cur_pos++] = i;
      }
      return out_set;
    }
    
    // Boolean math functions...
    BitSet NOT() const {
      BitSet out_set(*this);
      for (int i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = ~bit_set[i];
      if (LAST_BIT > 0) out_set.bit_set[NUM_FIELDS - 1] &= UIntMaskLow(LAST_BIT);
      return out_set;
    }

    BitSet AND(const BitSet & set2) const {
      BitSet out_set(*this);
      for (int i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = bit_set[i] & set2.bit_set[i];
      return out_set;
    }

    BitSet OR(const BitSet & set2) const {
      BitSet out_set(*this);
      for (int i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = bit_set[i] | set2.bit_set[i];
      return out_set;
    }

    BitSet NAND(const BitSet & set2) const {
      BitSet out_set(*this);
      for (int i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = ~(bit_set[i] & set2.bit_set[i]);
      if (LAST_BIT > 0) out_set.bit_set[NUM_FIELDS - 1] &= UIntMaskLow(LAST_BIT);
      return out_set;
    }

    BitSet NOR(const BitSet & set2) const {
      BitSet out_set(*this);
      for (int i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = ~(bit_set[i] | set2.bit_set[i]);
      if (LAST_BIT > 0) out_set.bit_set[NUM_FIELDS - 1] &= UIntMaskLow(LAST_BIT);
      return out_set;
    }

    BitSet XOR(const BitSet & set2) const {
      BitSet out_set(*this);
      for (int i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = bit_set[i] ^ set2.bit_set[i];
      return out_set;
    }

    BitSet EQU(const BitSet & set2) const {
      BitSet out_set(*this);
      for (int i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = ~(bit_set[i] ^ set2.bit_set[i]);
      if (LAST_BIT > 0) out_set.bit_set[NUM_FIELDS - 1] &= UIntMaskLow(LAST_BIT);
      return out_set;
    }
  

    // Boolean math functions...
    BitSet & NOT_SELF() {
      for (int i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~bit_set[i];
      if (LAST_BIT > 0) bit_set[NUM_FIELDS - 1] &= UIntMaskLow(LAST_BIT);
      return *this;
    }

    BitSet & AND_SELF(const BitSet & set2) {
      for (int i = 0; i < NUM_FIELDS; i++) bit_set[i] = bit_set[i] & set2.bit_set[i];
      return *this;
    }

    BitSet & OR_SELF(const BitSet & set2) {
      for (int i = 0; i < NUM_FIELDS; i++) bit_set[i] = bit_set[i] | set2.bit_set[i];
      return *this;
    }

    BitSet & NAND_SELF(const BitSet & set2) {
      for (int i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~(bit_set[i] & set2.bit_set[i]);
      if (LAST_BIT > 0) bit_set[NUM_FIELDS - 1] &= UIntMaskLow(LAST_BIT);
      return *this;
    }

    BitSet & NOR_SELF(const BitSet & set2) {
      for (int i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~(bit_set[i] | set2.bit_set[i]);
      if (LAST_BIT > 0) bit_set[NUM_FIELDS - 1] &= UIntMaskLow(LAST_BIT);
      return *this;
    }

    BitSet & XOR_SELF(const BitSet & set2) {
      for (int i = 0; i < NUM_FIELDS; i++) bit_set[i] = bit_set[i] ^ set2.bit_set[i];
      return *this;
    }

    BitSet & EQU_SELF(const BitSet & set2) {
      for (int i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~(bit_set[i] ^ set2.bit_set[i]);
      if (LAST_BIT > 0) bit_set[NUM_FIELDS - 1] &= UIntMaskLow(LAST_BIT);
      return *this;
    }
  
    // Positive shifts go left and negative go right (0 does nothing)
    BitSet SHIFT(const int shift_size) const {
      BitSet out_set(*this);
      if (shift_size > 0) out_set.ShiftRight(shift_size);
      else if (shift_size < 0) out_set.ShiftLeft(-shift_size);
      return out_set;
    }

    BitSet & SHIFT_SELF(const int shift_size) {
      if (shift_size > 0) ShiftRight(shift_size);
      else if (shift_size < 0) ShiftLeft(-shift_size);
      return *this;
    }


    // Operator overloads...
    BitSet operator~() const { return NOT(); }
    BitSet operator&(const BitSet & ar2) const { return AND(ar2); }
    BitSet operator|(const BitSet & ar2) const { return OR(ar2); }
    BitSet operator^(const BitSet & ar2) const { return XOR(ar2); }
    BitSet operator<<(const int shift_size) const { return SHIFT(-shift_size); }
    BitSet operator>>(const int shift_size) const { return SHIFT(shift_size); }
    const BitSet & operator&=(const BitSet & ar2) { return AND_SELF(ar2); }
    const BitSet & operator|=(const BitSet & ar2) { return OR_SELF(ar2); }
    const BitSet & operator^=(const BitSet & ar2) { return XOR_SELF(ar2); }
    const BitSet & operator<<=(const int shift_size) { return SHIFT_SELF(-shift_size); }
    const BitSet & operator>>=(const int shift_size) { return SHIFT_SELF(shift_size); }
  };

  template <int NUM_BITS> std::ostream & operator<<(std::ostream & out, const BitSet<NUM_BITS> & _bit_set) {
    _bit_set.Print(out);
    return out;
  }

  template <int NUM_BITS1, int NUM_BITS2>
  BitSet<NUM_BITS1+NUM_BITS2> join(const BitSet<NUM_BITS1> & in1, const BitSet<NUM_BITS2> & in2) {
    BitSet<NUM_BITS1+NUM_BITS2> out_bits;
    out_bits.Import(in2);
    out_bits <<= NUM_BITS1;
    out_bits |= in2.template Export<NUM_BITS1+NUM_BITS2>();
  }

};

#endif
