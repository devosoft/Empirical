#ifndef EMP_BIT_ARRAY_H
#define EMP_BIT_ARRAY_H

#include <assert.h>
#include <iostream>
#include <vector>

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class: emp::BitArray
// Desc: This class handles an arbitrarily large array of bits,
//       and optimizes operations on those bits to be as fast as possible.
//
//
// Constructors:
//  BitArray()                            -- Assume a size zero array.
//  BitArray(int in_size)                 -- Create an uninitialized array.
//  BitArray(const BitArray & in_array)   -- Copy Constructor
//
// Assignment and equality test:
//  BitArray & operator=(const BitArray & in_array)
//  bool operator==(const BitArray & in_array) const
//
// Sizing:
//  int GetSize() const
//  void Resize(const int new_size)
//  void ResizeClear(const int new_size)
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
//  int CountBits()   -- Count 1s -- fast for sparse arrays.
//  int CountBits2()  -- Count 1s -- fast for arbitary arrays.
//  int FindBit1(int start_bit)   -- Return pos of first 1 after start_bit 
//
// Boolean math functions:
//  BitArray NOT() const
//  BitArray AND(const BitArray & array2) const
//  BitArray OR(const BitArray & array2) const
//  BitArray NAND(const BitArray & array2) const
//  BitArray NOR(const BitArray & array2) const
//  BitArray XOR(const BitArray & array2) const
//  BitArray EQU(const BitArray & array2) const
//  BitArray SHIFT(const int shift_size) const   -- positive for left shift, negative for right shift
//
//  const BitArray & NOTSELF()
//  const BitArray & ANDSELF(const BitArray & array2)
//  const BitArray & ORSELF(const BitArray & array2)
//  const BitArray & NANDSELF(const BitArray & array2)
//  const BitArray & NORSELF(const BitArray & array2)
//  const BitArray & XORSELF(const BitArray & array2)
//  const BitArray & EQUSELF(const BitArray & array2)
//  const BitArray & SHIFTSELF(const int shift_size) const
//
// Arithmetic:
//  BitArray INCREMENTSELF()
//
// Operator overloads:
//  BitArray operator~() const
//  BitArray operator&(const BitArray & ar2) const
//  BitArray operator|(const BitArray & ar2) const
//  BitArray operator^(const BitArray & ar2) const
//  BitArray operator>>(const int) const
//  BitArray operator<<(const int) const
//  const BitArray & operator&=(const BitArray & ar2)
//  const BitArray & operator|=(const BitArray & ar2)
//  const BitArray & operator^=(const BitArray & ar2)
//  const BitArray & operator>>=(const int)
//  const BitArray & operator<<=(const int)
//  BitArray & operator++()     // prefix ++
//  BitArray & operator++(int)  // postfix ++


namespace emp {

  // The bit_manip namespace contains a set of methods for manipulating raw bit arrays.
  // A pointer to the array and the number of bits in it must be passed in manually.
  // @CAO I am gradually moving functionality from cRawBitArray to here.

  namespace bit_manip {
    int CountFields(const int num_bits) { return 1 + ((num_bits - 1) >> 5); }
    int GetField(const int index) { return index >> 5; }
    int GetFieldPos(const int index) { return index & 31; }

    void SetZeros(unsigned int * bit_fields, const int num_bits) {
      const int num_fields = CountFields(num_bits);
      for (int i = 0; i < num_fields; i++) bit_fields[i] = 0;
    }

    void SetOnes(unsigned int * bit_fields, const int num_bits) {
      const int num_fields = CountFields(num_bits);
      const int last_bit   = GetFieldPos(num_bits);
      for (int i = 0; i < num_fields; i++) { bit_fields[i] = ~0; }    
      if (last_bit > 0) { bit_fields[num_fields - 1] &= (1 << last_bit) - 1; }
    }

    unsigned int * Duplicate(unsigned int * in_array, const int num_bits) {
      const int num_fields = bit_manip::CountFields(num_bits);
      unsigned int * bit_fields = new unsigned int[num_fields];
      for (int i = 0; i < num_fields; i++) bit_fields[i] = in_array[i];
      return bit_fields;
    }

  };

  class cRawBitArray {
  private:
    unsigned int * bit_fields;
  
    // Disallow default copy constructor and operator=
    // (we need to know the number of bits we're working with!)
    cRawBitArray(const cRawBitArray&);
    const cRawBitArray & operator=(const cRawBitArray&);

  public:
    cRawBitArray() : bit_fields(NULL) { ; }
    cRawBitArray(const int num_bits) {
      bit_fields = new unsigned int[ bit_manip::CountFields(num_bits) ];
      bit_manip::SetZeros(bit_fields, num_bits);
    }
    cRawBitArray(const cRawBitArray & in_array, const int num_bits) {
      bit_fields = bit_manip::Duplicate(in_array.bit_fields, num_bits);
    }
    ~cRawBitArray() {  if (bit_fields) delete [] bit_fields; }

    unsigned int * operator()() { return bit_fields; }

    // The Copy() method and the Copy Constructor must both be told how many
    // bits they are working with.
    void Copy(const cRawBitArray & in_array, const int num_bits) {
      if (bit_fields) delete [] bit_fields;
      bit_fields = bit_manip::Duplicate(in_array.bit_fields, num_bits);
    }

    // For fast bit operations, we're not going to setup operator[]; instead
    // we're going to have a GetBit and a SetBit commamd.  For this raw version
    // we're also going to assume that the index is within range w/o any special
    // checks.
    bool GetBit(const int index) const{
      const int field_id = bit_manip::GetField(index);
      const int pos_id = bit_manip::GetFieldPos(index);
      return (bit_fields[field_id] & (1 << pos_id)) != 0;
    }

    void SetBit(const int index, const bool value) {
      const int field_id = bit_manip::GetField(index);
      const int pos_id = bit_manip::GetFieldPos(index);
      const int pos_mask = 1 << pos_id;

      if (value) bit_fields[field_id] |= pos_mask;
      else       bit_fields[field_id] &= ~pos_mask;
    }

    bool IsEqual(const cRawBitArray & in_array, int num_bits) const {
      const int num_fields = bit_manip::CountFields(num_bits);
      for (int i = 0; i < num_fields; i++) {
        if (bit_fields[i] != in_array.bit_fields[i]) return false;
      }
      return true;
    }

    void Resize(const int old_bits, const int new_bits) {
      const int num_old_fields = bit_manip::CountFields(old_bits);
      const int num_new_fields = bit_manip::CountFields(new_bits);
      if (num_old_fields == num_new_fields) {
        // Clear all bits past the new end and stop (in case new_bits is slightly less than old_bits)
        unsigned int & last_field = bit_fields[num_new_fields - 1];
        for (int i = new_bits; i < old_bits; i++) {
          const unsigned int clear_bit = i & 31;
          last_field &= ~(1 << clear_bit);
        }
        return;
      }

      // If we made it here, we must change the number of fields.
      // Create the new bit array and copy the old one into it.
      unsigned int * new_bit_fields = new unsigned int[ num_new_fields ];
      const int min_fields = std::min(num_new_fields, num_old_fields);
      for (int i = 0; i < min_fields; i++) {
        new_bit_fields[i] = bit_fields[i];
      }
  
      // If the old bits are longer, we must clear the end of the last bit field.
      if (num_old_fields > num_new_fields) {
        unsigned int & last_field = new_bit_fields[num_new_fields - 1];
        // @CAO Speed this up!
        for (int clear_bit=bit_manip::GetFieldPos(new_bits); clear_bit < 32; clear_bit++) {
          last_field &= ~(1 << clear_bit);
        }
      }
  
      // If the new bits are longer, clear fields past the end of the old bits.
      for (int i = num_old_fields; i < num_new_fields; i++) {
        new_bit_fields[i] = 0;
      }

      if (bit_fields != NULL) {
        delete [] bit_fields;
      }
      bit_fields = new_bit_fields;
    }
  
    // Resize, but don't copy anything into the new space.
    void ResizeSloppy(const int new_bits) {
      const int new_fields = bit_manip::CountFields(new_bits);
      if (bit_fields) delete [] bit_fields;
      bit_fields = new unsigned int[ new_fields ];
    }

    // Resize and copy zeroes into the new space.
    void ResizeClear(const int new_bits) {
      ResizeSloppy(new_bits);
      bit_manip::SetZeros(bit_fields, new_bits);
    }


    // Count 1 bits by looping through once for each bit equal to 1; fast for sparse arrays.
    int CountBits(const int num_bits) const { 
      const int num_fields = bit_manip::CountFields(num_bits);
      int bit_count = 0;
    
      for (int i = 0; i < num_fields; i++) {
        int temp = bit_fields[i];
        while (temp) {
          temp = temp & (temp - 1);
          bit_count++;
        }
      }
      return bit_count;
    }

    // Count 1 bits in semi-parallel; fast for dense arrays.
    int CountBits2(const int num_bits) const {
      const int num_fields = bit_manip::CountFields(num_bits);
      int bit_count = 0;
    
      for (int i = 0; i < num_fields; i++) {
        const int  v = bit_fields[i];
        unsigned int const t1 = v - ((v >> 1) & 0x55555555);
        unsigned int const t2 = (t1 & 0x33333333) + ((t1 >> 2) & 0x33333333);
        bit_count += ((t2 + (t2 >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
      }
      return bit_count;
    }

    // Other bit-play
    int FindBit1(const int num_bits, const int start_pos=0) const {
      // @CAO -- There are better ways to do this with bit tricks.
      for (int i = start_pos; i < num_bits; i++) {
        if (GetBit(i)) return i;
      }
      return -1;
    }

    std::vector<int> GetOnes(const int num_bits) const {
      // @CAO -- There are probably better ways to do this with bit tricks.
      std::vector<int> out_array(CountBits2(num_bits));
      int cur_pos = 0;
      for (int i = 0; i < num_bits; i++) {
        if (GetBit(i)) out_array[cur_pos++] = i;
      }
      return out_array;
    }


    // Helper: call SHIFT with positive number instead
    // NOTE: This does NOT change the number of bits in the array.
    void ShiftLeft(const int num_bits, const int shift_size) {
      assert(shift_size > 0);
      int num_fields = bit_manip::CountFields(num_bits);
      int field_shift = shift_size / 32;
      int bit_shift = shift_size % 32;
    
      // account for field_shift
      if (field_shift) {
        for (int i = num_fields - 1; i >= field_shift; i--) {
          bit_fields[i] = bit_fields[i - field_shift];
        }
        for (int i = field_shift - 1; i >= 0; i--) {
          bit_fields[i] = 0;
        }
      }
    
      // account for bit_shift
      int temp = 0;
      for (int i = 0; i < num_fields; i++) {
        temp = bit_fields[i] >> (32 - bit_shift);
        bit_fields[i] <<= bit_shift;
        if (i > 0) bit_fields[i - 1] |= temp;  // lower bits of bit_fields[i - 1] are all 0
      }
    
      // mask out any bits that have left-shifted away, allowing CountBits and CountBits2 to work
      // blw: if CountBits/CountBits2 are fixed, this code should be removed as it will be redundant
      unsigned int shift_mask = 0xFFFFFFFF >> ((32 - (num_bits % 32)) & 0x1F);
      bit_fields[num_fields - 1] &= shift_mask;    
    }

  
    // Helper for calling SHIFT with negative number
    void ShiftRight(const int num_bits, const int shift_size) {
      assert(shift_size > 0);
      int num_fields = bit_manip::CountFields(num_bits);
      int field_shift = shift_size / 32;
      int bit_shift = shift_size % 32;
  
      // account for field_shift
      if (field_shift) {
        for (int i = 0; i < num_fields - field_shift; i++) {
          bit_fields[i] = bit_fields[i + field_shift];
        }
        for(int i = num_fields - field_shift; i < num_fields; i++) {
          bit_fields[i] = 0;
        }
      }
  
      // account for bit_shift
      bit_fields[num_fields - 1] >>= bit_shift;  // drops off right end, may shift in ones if sign bit was set
      int temp = 0;
      for (int i = num_fields - 2; i >= 0; i--) {
        temp = bit_fields[i] << (32 - bit_shift);
        bit_fields[i] >>= bit_shift;
        bit_fields[i + 1] |= temp;
      }
    }

    void Print(const int num_bits, std::ostream & out=std::cout) const {
      for (int i = 0; i < num_bits; i++) {
        out << GetBit(i);
      }
    }
  
    // prints in the accepted human readable low-to-hight = right-to-left format, taking bit 0 as low bit
    void PrintRightToLeft(const int num_bits, std::ostream & out=std::cout) const {
      for (int i = num_bits - 1; i >= 0; i--) {
        out << GetBit(i);
      }
    }

    void PrintOneIDs(const int num_bits, std::ostream & out=std::cout) const {
      for (int i = 0; i < num_bits; i++) {
        if (GetBit(i)) out << i << " ";
      }
    }

    // Fast bool operators: use this bit array as one input and for storing the result.
    void NOT(const int num_bits) {
      const int num_fields = bit_manip::CountFields(num_bits);
      for (int i = 0; i < num_fields; i++) {
        bit_fields[i] = ~bit_fields[i];
      }
    
      const int last_bit = bit_manip::GetFieldPos(num_bits);
      if (last_bit > 0) {
        bit_fields[num_fields - 1] &= (1 << last_bit) - 1;
      }
    }

    void AND(const cRawBitArray & array2, const int num_bits) {
      const int num_fields = bit_manip::CountFields(num_bits);
      for (int i = 0; i < num_fields; i++) {
        bit_fields[i] &= array2.bit_fields[i];
      }
    }

    void OR(const cRawBitArray & array2, const int num_bits) {
      const int num_fields = bit_manip::CountFields(num_bits);
      for (int i = 0; i < num_fields; i++) {
        bit_fields[i] |= array2.bit_fields[i];
      }
    }

    void NAND(const cRawBitArray & array2, const int num_bits) {
      const int num_fields = bit_manip::CountFields(num_bits);
      for (int i = 0; i < num_fields; i++) {
        bit_fields[i] = ~(bit_fields[i] & array2.bit_fields[i]);
      }
    
      const int last_bit = bit_manip::GetFieldPos(num_bits);
      if (last_bit > 0) {
        bit_fields[num_fields - 1] &= (1 << last_bit) - 1;
      }
    }

    void NOR(const cRawBitArray & array2, const int num_bits) {
      const int num_fields = bit_manip::CountFields(num_bits);
      for (int i = 0; i < num_fields; i++) {
        bit_fields[i] = ~(bit_fields[i] | array2.bit_fields[i]);
      }
    
      const int last_bit = bit_manip::GetFieldPos(num_bits);
      if (last_bit > 0) {
        bit_fields[num_fields - 1] &= (1 << last_bit) - 1;
      }
    }

    void XOR(const cRawBitArray & array2, const int num_bits) {
      const int num_fields = bit_manip::CountFields(num_bits);
      for (int i = 0; i < num_fields; i++) {
        bit_fields[i] ^= array2.bit_fields[i];
      }
    }

    void EQU(const cRawBitArray & array2, const int num_bits) {
      const int num_fields = bit_manip::CountFields(num_bits);
      for (int i = 0; i < num_fields; i++) {
        bit_fields[i] = ~(bit_fields[i] ^ array2.bit_fields[i]);
      }
    
      const int last_bit = bit_manip::GetFieldPos(num_bits);
      if (last_bit > 0) {
        bit_fields[num_fields - 1] &= (1 << last_bit) - 1;
      }
    }

    // Positive shifts go left and negative go right (0 does nothing)
    void SHIFT(const int num_bits, const int shift_size) {
      if (shift_size == 0) return;
      if (shift_size > 0) { ShiftLeft(num_bits, shift_size); return; }
      if (shift_size < 0) { ShiftRight(num_bits, -shift_size); return; }
      assert(false); // Should never get here.
    }

    void INCREMENT(const int num_bits){
      const int num_fields = bit_manip::CountFields(num_bits);
      int i = 0;
      for (i = 0; i < num_fields; i++) {
        bit_fields[i]++;
        if (bit_fields[i] != 0) break;  // no overflow, do not need to increment higher fields
      }
    
      // If highest bit field was incremented, mask out any unused portions of the field
      if (i == num_fields - 1) {
        unsigned int shift_mask = 0xffffffff >> (32 - num_bits % 32);
        bit_fields[num_fields - 1] &= shift_mask;
      }
    }

    // Fast bool operators: load inputs and store the results here.
    void NOT(const cRawBitArray & array1, const int num_bits) {
      ResizeSloppy(num_bits);
    
      const int num_fields = bit_manip::CountFields(num_bits);
      for (int i = 0; i < num_fields; i++) {
        bit_fields[i] = ~array1.bit_fields[i];
      }
    
      const int last_bit = bit_manip::GetFieldPos(num_bits);
      if (last_bit > 0) {
        bit_fields[num_fields - 1] &= (1 << last_bit) - 1;
      }
    }

    void AND(const cRawBitArray & array1, const cRawBitArray & array2, const int num_bits) {
      ResizeSloppy(num_bits);
    
      const int num_fields = bit_manip::CountFields(num_bits);
      for (int i = 0; i < num_fields; i++) {
        bit_fields[i] = array1.bit_fields[i] & array2.bit_fields[i];
      }
    }
  
    void OR(const cRawBitArray & array1, const cRawBitArray & array2, const int num_bits) {
      ResizeSloppy(num_bits);
    
      const int num_fields = bit_manip::CountFields(num_bits);
      for (int i = 0; i < num_fields; i++) {
        bit_fields[i] = array1.bit_fields[i] | array2.bit_fields[i];
      }
    }

    void NAND(const cRawBitArray & array1, const cRawBitArray & array2, const int num_bits) {
      ResizeSloppy(num_bits);
    
      const int num_fields = bit_manip::CountFields(num_bits);
      for (int i = 0; i < num_fields; i++) {
        bit_fields[i] = ~(array1.bit_fields[i] & array2.bit_fields[i]);
      }

      const int last_bit = bit_manip::GetFieldPos(num_bits);
      if (last_bit > 0) {
        bit_fields[num_fields - 1] &= (1 << last_bit) - 1;
      }
    }

    void NOR(const cRawBitArray & array1, const cRawBitArray & array2, const int num_bits) {
      ResizeSloppy(num_bits);
    
      const int num_fields = bit_manip::CountFields(num_bits);
      for (int i = 0; i < num_fields; i++) {
        bit_fields[i] = ~(array1.bit_fields[i] | array2.bit_fields[i]);
      }
    
      const int last_bit = bit_manip::GetFieldPos(num_bits);
      if (last_bit > 0) {
        bit_fields[num_fields - 1] &= (1 << last_bit) - 1;
      }
    }

    void XOR(const cRawBitArray & array1, const cRawBitArray & array2, const int num_bits) {
      ResizeSloppy(num_bits);
    
      const int num_fields = bit_manip::CountFields(num_bits);
      for (int i = 0; i < num_fields; i++) {
        bit_fields[i] = array1.bit_fields[i] ^ array2.bit_fields[i];
      }
    }

    void EQU(const cRawBitArray & array1, const cRawBitArray & array2, const int num_bits) {
      ResizeSloppy(num_bits);
    
      const int num_fields = bit_manip::CountFields(num_bits);
      for (int i = 0; i < num_fields; i++) {
        bit_fields[i] = ~(array1.bit_fields[i] ^ array2.bit_fields[i]);
      }
    
      const int last_bit = bit_manip::GetFieldPos(num_bits);
      if (last_bit > 0) {
        bit_fields[num_fields - 1] &= (1 << last_bit) - 1;
      }
    }

    void SHIFT(const cRawBitArray & array1, const int num_bits, const int shift_size) {
      if (shift_size == 0) return;
  
      if (bit_fields) delete [] bit_fields;
      bit_fields = bit_manip::Duplicate(array1.bit_fields, num_bits);

      SHIFT(num_bits, shift_size);
    }

    void INCREMENT(const cRawBitArray & array1, const int num_bits) {
      if (bit_fields) delete [] bit_fields;
      bit_fields = bit_manip::Duplicate(array1.bit_fields, num_bits);
      INCREMENT(num_bits);
    }
  };


  class BitArray {
  private:
    cRawBitArray m_raw_array;
    int m_array_size;
    
    // Setup a bit proxy so that we can use operator[] on bit arrays as a lvalue.
    class cBitProxy {
    private:
      BitArray & array;
      int index;
    public:
      cBitProxy(BitArray & _array, int _idx) : array(_array), index(_idx) {;}
      
      inline cBitProxy & operator=(bool b);    // lvalue handling...
      inline operator bool() const;            // rvalue handling...
    };
    friend class cBitProxy;
  public:
    BitArray() : m_array_size(0) { ; }
    BitArray(int in_size) : m_raw_array(in_size), m_array_size(in_size) { ; }
    BitArray(const BitArray & in_array)
      : m_raw_array(in_array.m_raw_array, in_array.m_array_size)
      , m_array_size(in_array.m_array_size) { ; }
    BitArray(const cRawBitArray & in_array, int in_size)
      : m_raw_array(in_array, in_size)
      , m_array_size(in_size) { ; }
    
    BitArray & operator=(const BitArray & in_array) {
      m_raw_array.Copy(in_array.m_raw_array, in_array.m_array_size);
      m_array_size = in_array.m_array_size;
      return *this;
    }

    bool operator==(const BitArray & in_array) const {
      if (m_array_size != in_array.m_array_size) return false;
      return m_raw_array.IsEqual(in_array.m_raw_array, m_array_size);
    }

    int GetSize() const { return m_array_size; }

    void Set(int index, bool value) {
      assert(index < m_array_size);
      m_raw_array.SetBit(index, value);
    }

    bool Get(int index) const {
      assert(index < m_array_size);
      return m_raw_array.GetBit(index);
    }

    bool operator[](int index) const { return Get(index); }
    cBitProxy operator[](int index) { return cBitProxy(*this, index); }

    void Clear() { bit_manip::SetZeros(m_raw_array(), m_array_size); }
    void SetAll() { bit_manip::SetOnes(m_raw_array(), m_array_size); }
  
    void Print(std::ostream & out=std::cout) const { m_raw_array.Print(m_array_size, out); }
    void PrintRightToLeft(std::ostream & out=std::cout) const { m_raw_array.PrintRightToLeft(m_array_size, out); }
    void PrintOneIDs(std::ostream & out=std::cout) const { m_raw_array.PrintOneIDs(m_array_size, out); }
    void Resize(const int new_size) {
      m_raw_array.Resize(m_array_size, new_size);
      m_array_size = new_size;
    }
    void ResizeClear(const int new_size) {
      m_raw_array.ResizeClear(new_size);
      m_array_size = new_size;
    }
    int CountBits() const { return m_raw_array.CountBits(m_array_size); }
    int CountBits2() const { return m_raw_array.CountBits2(m_array_size); }

    int FindBit1(int start_bit=0) const { return m_raw_array.FindBit1(m_array_size, start_bit); }
    std::vector<int> GetOnes() const { return m_raw_array.GetOnes(m_array_size); }
    
    // Boolean math functions...
    BitArray NOT() const {
      BitArray out_array;
      out_array.m_raw_array.NOT(m_raw_array, m_array_size);
      out_array.m_array_size = m_array_size;
      return out_array;
    }

    BitArray AND(const BitArray & array2) const {
      assert(m_array_size == array2.m_array_size);
      BitArray out_array;
      out_array.m_raw_array.AND(m_raw_array, array2.m_raw_array, m_array_size);
      out_array.m_array_size = m_array_size;
      return out_array;
    }

    BitArray OR(const BitArray & array2) const {
      assert(m_array_size == array2.m_array_size);
      BitArray out_array;
      out_array.m_raw_array.OR(m_raw_array, array2.m_raw_array, m_array_size);
      out_array.m_array_size = m_array_size;
      return out_array;
    }

    BitArray NAND(const BitArray & array2) const {
      assert(m_array_size == array2.m_array_size);
      BitArray out_array;
      out_array.m_raw_array.NAND(m_raw_array, array2.m_raw_array, m_array_size);
      out_array.m_array_size = m_array_size;
      return out_array;
    }

    BitArray NOR(const BitArray & array2) const {
      assert(m_array_size == array2.m_array_size);
      BitArray out_array;
      out_array.m_raw_array.NOR(m_raw_array, array2.m_raw_array, m_array_size);
      out_array.m_array_size = m_array_size;
      return out_array;
    }

    BitArray XOR(const BitArray & array2) const {
      assert(m_array_size == array2.m_array_size);
      BitArray out_array;
      out_array.m_raw_array.XOR(m_raw_array, array2.m_raw_array, m_array_size);
      out_array.m_array_size = m_array_size;
      return out_array;
    }

    BitArray EQU(const BitArray & array2) const {
      assert(m_array_size == array2.m_array_size);
      BitArray out_array;
      out_array.m_raw_array.EQU(m_raw_array, array2.m_raw_array, m_array_size);
      out_array.m_array_size = m_array_size;
      return out_array;
    }
  
    BitArray SHIFT(const int shift_size) const {
      BitArray out_array;
      out_array.m_raw_array.SHIFT(m_raw_array, m_array_size, shift_size);
      out_array.m_array_size = m_array_size;
      return out_array;
    }

    const BitArray & NOTSELF() {
      m_raw_array.NOT(m_array_size);
      return *this;
    }
    
    const BitArray & ANDSELF(const BitArray & array2) {
      assert(m_array_size == array2.m_array_size);
      m_raw_array.AND(array2.m_raw_array, m_array_size);
      return *this;
    }
    
    const BitArray & ORSELF(const BitArray & array2) {
      assert(m_array_size == array2.m_array_size);
      m_raw_array.OR(array2.m_raw_array, m_array_size);
      return *this;
    }
    
    const BitArray & NANDSELF(const BitArray & array2) {
      assert(m_array_size == array2.m_array_size);
      m_raw_array.NAND(array2.m_raw_array, m_array_size);
      return *this;
    }
    
    const BitArray & NORSELF(const BitArray & array2) {
      assert(m_array_size == array2.m_array_size);
      m_raw_array.NOR(array2.m_raw_array, m_array_size);
      return *this;
    }
    
    const BitArray & XORSELF(const BitArray & array2) {
      assert(m_array_size == array2.m_array_size);
      m_raw_array.XOR(array2.m_raw_array, m_array_size);
      return *this;
    }
    
    const BitArray & EQUSELF(const BitArray & array2) {
      assert(m_array_size == array2.m_array_size);
      m_raw_array.EQU(array2.m_raw_array, m_array_size);
      return *this;
    }
  
    const BitArray & SHIFTSELF(const int shift_size) {
      m_raw_array.SHIFT(m_array_size, shift_size);
      return *this;
    }
  
    BitArray & INCREMENTSELF() {
      m_raw_array.INCREMENT(m_array_size);
      return *this;
    }
  

    // Operator overloads...
    BitArray operator~() const { return NOT(); }
    BitArray operator&(const BitArray & ar2) const { return AND(ar2); }
    BitArray operator|(const BitArray & ar2) const { return OR(ar2); }
    BitArray operator^(const BitArray & ar2) const { return XOR(ar2); }
    BitArray operator<<(const int shift_size) const { return SHIFT(shift_size); }
    BitArray operator>>(const int shift_size) const { return SHIFT(-shift_size); }
    const BitArray & operator&=(const BitArray & ar2) { return ANDSELF(ar2); }
    const BitArray & operator|=(const BitArray & ar2) { return ORSELF(ar2); }
    const BitArray & operator^=(const BitArray & ar2) { return XORSELF(ar2); }
    const BitArray & operator<<=(const int shift_size) { return SHIFTSELF(shift_size); }
    const BitArray & operator>>=(const int shift_size) { return SHIFTSELF(-shift_size); }
    BitArray & operator++() { return INCREMENTSELF(); }  // prefix ++
    BitArray operator++(int) { BitArray ans = *this; operator++(); return ans;}  // postfix ++
  };

  std::ostream & operator << (std::ostream & out, const BitArray & in_bit_array) {
    in_bit_array.Print(out);
    return out;
  }

  BitArray::cBitProxy & BitArray::cBitProxy::operator=(bool b)
  {
    array.Set(index, b);
    return *this;
  }


  BitArray::cBitProxy::operator bool() const
  {
    return array.Get(index);
  }


};

#endif
