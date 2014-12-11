#ifndef EMP_BIT_SET_H
#define EMP_BIT_SET_H

#include <assert.h>
#include <iostream>
#include <vector>

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class: template <int NUM_BITS> emp::BitSet
// Desc: This class handles a fixed-sized (but arbitrarily large) array of bits,
//       and optimizes operations on those bits to be as fast as possible.
//
//
// Constructors:
//  BitSet()                        -- Assume all zeroes in set
//  BitSet(const BitSet & in_set)   -- Copy Constructor
//
// Assignment and equality test:
//  BitSet & operator=(const BitSet & in_set)
//  bool operator==(const BitSet & in_set) const
//
// Sizing:
//  int GetSize() const
//
// Accessors:
//  void Set(int index, bool value)
//  bool Get(int index) const
//  bool operator[](int index) const
//  cBitProxy operator[](int index)
//  void Clear()
//  void SetAll()
//
// Printing:
//  void Print(ostream & out=cout) const
//  void PrintOneIDs(ostream & out=cout) const
//
// Bit play:
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


namespace emp {

  template <int NUM_BITS> class BitSet {
  private:
    unsigned int * bit_set;
    
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

    static const int NUM_FIELDS = 1 + ((NUM_BITS - 1) >> 5);
    static const int LAST_BIT = NUM_BITS & 31;

    inline static int FieldID(const int index) { return index >> 5; }
    inline static int FieldPos(const int index) { return index & 31; }

    inline static unsigned int * Duplicate(unsigned int * in_set) {
      unsigned int * out_set = new unsigned int[NUM_FIELDS];
      for (int i = 0; i < NUM_BITS; i++) out_set[i] = in_set[i];
      return out_set;
    }

    // Helper: call SHIFT with positive number instead
    void ShiftLeft(const int shift_size) {
      assert(shift_size > 0);
      int field_shift = shift_size / 32;
      int bit_shift = shift_size % 32;
    
      // account for field_shift
      if (field_shift) {
        for (int i = NUM_FIELDS - 1; i >= field_shift; i--) {
          bit_set[i] = bit_set[i - field_shift];
        }
        for (int i = field_shift - 1; i >= 0; i--) bit_set[i] = 0;
      }
    
      // account for bit_shift
      int temp = 0;
      for (int i = 0; i < NUM_FIELDS; i++) {
        temp = bit_set[i] >> (32 - bit_shift);
        bit_set[i] <<= bit_shift;
        if (i > 0) bit_set[i - 1] |= temp;  // lower bits of bit_set[i - 1] are all 0
      }
    
      // mask out any bits that have left-shifted away, allowing CountBits and CountBits2 to work
      // blw: if CountBits/CountBits2 are fixed, this code should be removed as it will be redundant
      unsigned int shift_mask = 0xFFFFFFFF >> ((32 - (NUM_BITS % 32)) & 0x1F);
      bit_set[NUM_FIELDS - 1] &= shift_mask;    
    }

  
    // Helper for calling SHIFT with negative number
    void ShiftRight(const int shift_size) {
      assert(shift_size > 0);
      int field_shift = shift_size / 32;
      int bit_shift = shift_size % 32;
  
      // account for field_shift
      if (field_shift) {
        for (int i = 0; i < NUM_FIELDS - field_shift; i++) {
          bit_set[i] = bit_set[i + field_shift];
        }
        for(int i = NUM_FIELDS - field_shift; i < NUM_FIELDS; i++) {
          bit_set[i] = 0;
        }
      }
  
      // account for bit_shift
      bit_set[NUM_FIELDS - 1] >>= bit_shift;  // drops off right end, may shift in ones if sign bit was set
      int temp = 0;
      for (int i = NUM_FIELDS - 2; i >= 0; i--) {
        temp = bit_set[i] << (32 - bit_shift);
        bit_set[i] >>= bit_shift;
        bit_set[i + 1] |= temp;
      }
    }

  public:
    BitSet() : bit_set(new unsigned int [NUM_FIELDS]) { Clear(); }
    BitSet(const BitSet & in_set) : bit_set(Duplicate(in_set.bit_set)) { ; }

    void TestPrint() {
      std::cout << *bit_set << std::endl;
    }
    
    BitSet & operator=(const BitSet & in_set) {
      if (bit_set) delete [] bit_set;
      bit_set = Duplicate(in_set.bit_set);
      return *this;
    }

    bool operator==(const BitSet & in_set) const {
      for (int i = 0; i < NUM_FIELDS; i++) {
        if (bit_set[i] != in_set.bit_set[i]) return false;
      }
      return true;
    }

    int GetSize() const { return NUM_BITS; }

    void Set(int index, bool value) {
      assert(index >= 0 && index < NUM_BITS);
      const int field_id = FieldID(index);
      const int pos_id = FieldPos(index);
      const int pos_mask = 1 << pos_id;

      if (value) bit_set[field_id] |= pos_mask;
      else       bit_set[field_id] &= ~pos_mask;
    }

    bool Get(int index) const {
      assert(index >= 0 && index < NUM_BITS);
      const int field_id = FieldID(index);
      const int pos_id = FieldPos(index);
      return (bit_set[field_id] & (1 << pos_id)) != 0;
    }

    bool operator[](int index) const { return Get(index); }
    cBitProxy operator[](int index) { return cBitProxy(*this, index); }

    void Clear() { for (int i = 0; i < NUM_FIELDS; i++) bit_set[i] = 0; }
    void SetAll() { 
      for (int i = 0; i < NUM_FIELDS; i++) { bit_set[i] = ~0; }    
      if (LAST_BIT > 0) { bit_set[NUM_FIELDS - 1] &= (1 << LAST_BIT) - 1; }
    }
  
    void Print(std::ostream & out=std::cout) const {
      for (int i = 0; i < NUM_BITS; i++) out << Get(i);
    }
    void PrintRightToLeft(std::ostream & out=std::cout) const {
      for (int i = NUM_BITS - 1; i >= 0; i--) out << Get(i);
    }
    void PrintOneIDs(std::ostream & out=std::cout, char spacer=' ') const {
      for (int i = 0; i < NUM_BITS; i++) { if (Get(i)) out << i << spacer; }
    }

    // Count 1's by looping through once for each bit equal to 1
    int CountOnes_Sparse() const { 
      int bit_count = 0;
      for (int i = 0; i < NUM_FIELDS; i++) {
        int temp = bit_set[i];
        while (temp) {
          temp = temp & (temp - 1);
          bit_count++;
        }
      }
      return bit_count;
    }

    // Count 1's in semi-parallel; fastest for even 0's & 1's
    int CountOnes_Mixed() const {
      int bit_count = 0;
      for (int i = 0; i < NUM_FIELDS; i++) {
        const int  v = bit_set[i];
        unsigned int const t1 = v - ((v >> 1) & 0x55555555);
        unsigned int const t2 = (t1 & 0x33333333) + ((t1 >> 2) & 0x33333333);
        bit_count += ((t2 + (t2 >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
      }
      return bit_count;
    }

    int CountOnes() const { return CountOnes_Mixed(); }

    int FindBit1(const int start_pos=0) const {
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
      if (LAST_BIT > 0) out_set.bit_set[NUM_FIELDS - 1] &= (1 << LAST_BIT) - 1;
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
      if (LAST_BIT > 0) out_set.bit_set[NUM_FIELDS - 1] &= (1 << LAST_BIT) - 1;
      return out_set;
    }

    BitSet NOR(const BitSet & set2) const {
      BitSet out_set(*this);
      for (int i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = ~(bit_set[i] | set2.bit_set[i]);
      if (LAST_BIT > 0) out_set.bit_set[NUM_FIELDS - 1] &= (1 << LAST_BIT) - 1;
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
      if (LAST_BIT > 0) out_set.bit_set[NUM_FIELDS - 1] &= (1 << LAST_BIT) - 1;
      return out_set;
    }
  

    // Boolean math functions...
    BitSet & NOT_SELF() const {
      for (int i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~bit_set[i];
      if (LAST_BIT > 0) bit_set[NUM_FIELDS - 1] &= (1 << LAST_BIT) - 1;
      return *this;
    }

    BitSet & AND_SELF(const BitSet & set2) const {
      for (int i = 0; i < NUM_FIELDS; i++) bit_set[i] = bit_set[i] & set2.bit_set[i];
      return *this;
    }

    BitSet & OR_SELF(const BitSet & set2) const {
      for (int i = 0; i < NUM_FIELDS; i++) bit_set[i] = bit_set[i] | set2.bit_set[i];
      return *this;
    }

    BitSet & NAND_SELF(const BitSet & set2) const {
      for (int i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~(bit_set[i] & set2.bit_set[i]);
      if (LAST_BIT > 0) bit_set[NUM_FIELDS - 1] &= (1 << LAST_BIT) - 1;
      return *this;
    }

    BitSet & NOR_SELF(const BitSet & set2) const {
      for (int i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~(bit_set[i] | set2.bit_set[i]);
      if (LAST_BIT > 0) bit_set[NUM_FIELDS - 1] &= (1 << LAST_BIT) - 1;
      return *this;
    }

    BitSet & XOR_SELF(const BitSet & set2) const {
      for (int i = 0; i < NUM_FIELDS; i++) bit_set[i] = bit_set[i] ^ set2.bit_set[i];
      return *this;
    }

    BitSet & EQU_SELF(const BitSet & set2) const {
      for (int i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~(bit_set[i] ^ set2.bit_set[i]);
      if (LAST_BIT > 0) bit_set[NUM_FIELDS - 1] &= (1 << LAST_BIT) - 1;
      return *this;
    }
  
    // Positive shifts go left and negative go right (0 does nothing)
    BitSet SHIFT(const int shift_size) const {
      BitSet out_set(*this);
      if (shift_size > 0) out_set.ShiftLeft(shift_size);
      else if (shift_size < 0) out_set.ShiftRight(-shift_size);
      return out_set;
    }

    BitSet & SHIFT_SELF(const int shift_size) {
      if (shift_size > 0) ShiftLeft(shift_size);
      else if (shift_size < 0) ShiftRight(-shift_size);
      return *this;
    }


    // Operator overloads...
    BitSet operator~() const { return NOT(); }
    BitSet operator&(const BitSet & ar2) const { return AND(ar2); }
    BitSet operator|(const BitSet & ar2) const { return OR(ar2); }
    BitSet operator^(const BitSet & ar2) const { return XOR(ar2); }
    BitSet operator<<(const int shift_size) const { return SHIFT(shift_size); }
    BitSet operator>>(const int shift_size) const { return SHIFT(-shift_size); }
    const BitSet & operator&=(const BitSet & ar2) { return AND_SELF(ar2); }
    const BitSet & operator|=(const BitSet & ar2) { return OR_SELF(ar2); }
    const BitSet & operator^=(const BitSet & ar2) { return XOR_SELF(ar2); }
    const BitSet & operator<<=(const int shift_size) { return SHIFT_SELF(shift_size); }
    const BitSet & operator>>=(const int shift_size) { return SHIFT_SELF(-shift_size); }
  };

  template <int NUM_BITS> std::ostream & operator<<(std::ostream & out, const BitSet<NUM_BITS> & _bit_set) {
    _bit_set.Print(out);
    return out;
  }


};

#endif
