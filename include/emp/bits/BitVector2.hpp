/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2020.
 *
 *  @file  BitVector.hpp
 *  @brief A drop-in replacement for std::vector<bool>, with additional bitwise logic features.
 *  @note Status: RELEASE
 *
 *  @note Compile with -O3 and -msse4.2 for fast bit counting.
 *
 *  @todo Most of the operators don't check to make sure that both BitVectors are the same size.
 *        We should create versions (Intersection() and Union()?) that adjust sizes if needed.
 *
 *  @todo Do small BitVector optimization.  Currently we have number of bits (8 bytes) and a
 *        pointer to the memory for the bitset (another 8 bytes), but we could use those 16 bytes
 *        as 1 byte of size info followed by 15 bytes of bitset (120 bits!)
 *  @todo For large BitVectors we can use a factory to preserve/adjust bit info.  That should be
 *        just as efficient than a reserve, but without the need to store extra in-class info.
 *  @todo Implement append(), resize(), push_bit(), insert(), remove()
 *  @todo Think about how itertors should work for BitVector.  It should probably go bit-by-bit,
 *        but there are very few circumstances where that would be useful.  Going through the
 *        positions of all ones would be more useful, but perhaps less intuitive.
 *  @todo Port Rotate() over from BitSet.
 *
 *  @note This class is 15-20% slower than emp::BitSet, but more flexible & run-time configurable.
 */


#ifndef EMP_BIT_VECTOR_H
#define EMP_BIT_VECTOR_H

#include <iostream>
#include <bitset>

#include "../base/assert.hpp"
#include "../base/Ptr.hpp"
#include "../base/vector.hpp"
#include "../math/math.hpp"
#include "../math/Random.hpp"
#include "../tools/functions.hpp"
#include "../polyfill/span.hpp"

#include "bitset_utils.hpp"
#include "_bitset_helpers.hpp"

namespace emp {

  /// @brief A drop-in replacement for std::vector<bool>, but with extra bitwise logic features.
  ///
  /// This class stores an arbirary number of bits in a set of "fields" (typically 32 bits or 64
  /// bits per field, depending on which should be faster.)  Individual bits can be extracted,
  /// -or- bitwise logic (including more complex bit magic) can be used on the groups of bits.

  class BitVector {
  private:
    // Use size_t for field_t since size_t is normally the native size for a processor (and, 
    // correctly, 32 bits for Emscripten), this should work in almost all cases.
    using field_t = size_t;

    // Compile-time constants
    static constexpr size_t FIELD_BITS = sizeof(field_t)*8; ///< Number of bits in a field

    static constexpr field_t FIELD_0 = (field_t) 0;         ///< All bits in a field set to 0
    static constexpr field_t FIELD_1 = (field_t) 1;         ///< Least significant bit set to 1
    static constexpr field_t FIELD_255 = (field_t) 255;     ///< Least significant 8 bits set to 1
    static constexpr field_t FIELD_ALL = ~FIELD_0;          ///< All bits in a field set to 1

    static constexpr size_t MAX_BITS = (size_t) -1;         ///< Value larger than any bit ID.

    size_t num_bits;        ///< Total number of bits are we using
    Ptr<field_t> bits;      ///< Pointer to array with the status of each bit

    /// Num bits used in partial field at the end; 0 if perfect fit.
    size_t NumEndBits() const { return num_bits & (FIELD_BITS - 1); }

    /// How many feilds do we need for the current set of bits?
    size_t NumFields() const { return num_bits ? (1 + ((num_bits - 1) / FIELD_BITS)) : 0; }

    /// How many bytes are used for the current set of bits? (rounded up!)
    size_t NumBytes() const { return num_bits ? (1 + ((num_bits - 1) >> 3)) : 0; }

    // Identify the field that a specified bit is in.
    static constexpr size_t FieldID(const size_t index)  { return index / FIELD_BITS; }

    // Identify the position within a field where a specified bit is.
    static constexpr size_t FieldPos(const size_t index) { return index & (FIELD_BITS-1); }

    // Identify which field a specified byte position would be in.
    static constexpr size_t Byte2Field(const size_t index) { return index / sizeof(field_t); }

    // Convert a byte position in BitVector to a byte position in the target field.
    static constexpr size_t Byte2FieldPos(const size_t index) { return FieldPos(index * 8); }

    // Assume that the size of the bits has already been adjusted to be the size of the one
    // being copied and only the fields need to be copied over.
    void RawCopy(const Ptr<field_t> in);

    // Convert the bits to bytes.
    emp::Ptr<unsigned char> BytePtr() { return bits.ReinterpretCast<unsigned char>(); }

    // Convert the bits to const bytes vector.
    emp::Ptr<const unsigned char> BytePtr() const {
      return bits.ReinterpretCast<const unsigned char>();
    }

    // Any bits past the last "real" bit in the last field should be kept as zeros.
    void ClearExcessBits() {
      if (NumEndBits() > 0) bits[NumFields() - 1] &= MaskLow<field_t>(NumEndBits());
    }

    // Helper: call SHIFT with positive number
    void ShiftLeft(const size_t shift_size);

    // Helper for calling SHIFT with negative number
    void ShiftRight(const size_t shift_size);

    // Scan this bitvector to make sure that there are no internal problems.
    bool OK() const;

  public:
    /// Build a new BitVector with specified bit count (default 0) and initialization (default 0)
    BitVector(size_t in_num_bits=0, bool init_val=false);

    /// Copy constructor of existing bit field.
    BitVector(const BitVector & in);

    /// Move constructor of existing bit field.
    BitVector(BitVector && in);

    /// Constructor to generate a random BitVector (with equal prob of 0 or 1).
    BitVector(size_t in_num_bits, Random & random);

    /// Constructor to generate a random BitVector with provided prob of 1's.
    BitVector(size_t in_num_bits, Random & random, const double p1);

    /// Constructor to generate a random BitVector with provided number of 1's.
    BitVector(size_t in_num_bits, Random & random, const size_t target_ones);

    /// Initializer list constructor.
    template <typename T> BitVector(const std::initializer_list<T> l);

    /// Copy, but with a resize.
    BitVector(const BitVector & in, size_t new_size);

    /// Destructor
    ~BitVector();

    /// Assignment operator.
    BitVector & operator=(const BitVector & in);

    /// Move operator.
    BitVector & operator=(BitVector && in);

    /// Assignment from another BitVector without changing size.
    BitVector & Import( const BitVector & from_bv, const size_t from_bit=0 );

    /// Convert to a BitVector of a different size.
    BitVector Export(size_t out_size, size_t start_bit=0) const;


    // >>>>>>>>>>  Accessors  <<<<<<<<<< //

    /// How many bits do we currently have?
    size_t GetSize() const { return num_bits; }

    /// How many bytes are in this BitVector?
    size_t GetNumBytes() const { return NumBytes(); }

    /// How many distinct values could be held in this BitVector?
    double GetNumStates() const { return emp::Pow2(num_bits); }

    /// Retrive the bit value from the specified index.
    bool Get(size_t index) const;

    /// A safe version of Get() for indexing out of range. Useful for representing collections.
    bool Has(size_t index) const { return (index < num_bits) ? Get(index) : false; }

    /// Update the bit value at the specified index.
    BitVector & Set(size_t index, bool value=true);

    /// Set all bits to 1.
    BitVector & SetAll();

    /// Set a range of bits to one: [start, stop)
    BitVector & SetRange(size_t start, size_t stop);

    /// Set all bits to 0.
    BitVector & Clear();

    /// Set specific bit to 0.
    BitVector & Clear(size_t index) { return Set(index, false); }

    /// Set bits to 0 in the range [start, stop)
    BitVector & Clear(const size_t start, const size_t stop);    

    /// Const index operator -- return the bit at the specified position.
    bool operator[](size_t index) const { return Get(index); }

    /// Index operator -- return a proxy to the bit at the specified position so it can be an lvalue.
    BitProxy<BitVector> operator[](size_t index) { return BitProxy<BitVector>(*this, index); }

    /// Change every bit in the sequence.
    BitVector & Toggle() { return NOT_SELF(); }

    /// Change a specified bit to the opposite value
    BitVector & Toggle(size_t index);

    /// Flips all the bits in a range [start, end)
    BitVector & Toggle(size_t start, size_t stop);

    /// Return true if ANY bits are set to 1, otherwise return false.
    bool Any() const;

    /// Return true if NO bits are set to 1, otherwise return false.
    bool None() const { return !Any(); }

    /// Return true if ALL bits are set to 1, otherwise return false.
    // @CAO: Can speed up by not duplicating the whole BitVector.
    bool All() const { return (~(*this)).None(); }

    /// Resize this BitVector to have the specified number of bits.
    BitVector & Resize(size_t new_bits);


    // >>>>>>>>>>  Randomization functions  <<<<<<<<<< //

    /// Set all bits randomly, with a 50% probability of being a 0 or 1.
    BitVector &  Randomize(Random & random);

    /// Set all bits randomly, with probability specified at compile time.
    template <Random::Prob P>
    BitVector & RandomizeP(Random & random, const size_t start_pos=0, size_t stop_pos=MAX_BITS);

    /// Set all bits randomly, with a given probability of being a one.
    BitVector & Randomize(Random & random, const double p,
                       const size_t start_pos=0, size_t stop_pos=MAX_BITS);

    /// Set all bits randomly, with a given probability of being a one.
    BitVector & Randomize(Random & random, const size_t target_ones,
                          const size_t start_pos=0, size_t stop_pos=MAX_BITS);
    
    /// Flip random bits with a given probability.
    BitVector & FlipRandom(Random & random, const double p,
                           const size_t start_pos=0, size_t stop_pos=MAX_BITS);

    /// Set random bits with a given probability (does not check if already set.)
    BitVector & SetRandom(Random & random, const double p,
                          const size_t start_pos=0, size_t stop_pos=MAX_BITS);

    /// Unset random bits with a given probability (does not check if already zero.)
    BitVector & ClearRandom(Random & random, const double p,
                            const size_t start_pos=0, size_t stop_pos=MAX_BITS);

    /// Flip a specified number of random bits.
    BitVector & FlipRandom(Random & random, const size_t target_bits);

    /// Set a specified number of random bits (does not check if already set.)
    BitVector & SetRandom(Random & random, const size_t target_bits);

    /// Unset  a specified number of random bits (does not check if already zero.)
    BitVector & ClearRandom(Random & random, const size_t target_bits);


    // >>>>>>>>>>  Comparison Operators  <<<<<<<<<< //

    bool operator==(const BitVector & in) const;
    bool operator!=(const BitVector & in) const { return !(*this == in); }
    bool operator< (const BitVector & in) const;
    bool operator> (const BitVector & in) const { return in < *this; }
    bool operator<=(const BitVector & in) const { return !(in < *this); }
    bool operator>=(const BitVector & in) const { return !(*this < in); }


    // >>>>>>>>>>  Conversion Operators  <<<<<<<<<< //

    /// Automatically convert BitVector to other vector types.
    template <typename T> operator emp::vector<T>();

    /// Casting a bit array to bool identifies if ANY bits are set to 1.
    explicit operator bool() const { return Any(); }


    // >>>>>>>>>>  Access Groups of bits  <<<<<<<<<< //

    /// Retrive the byte at the specified byte index.
    uint8_t GetByte(size_t index) const;

    /// Get a read-only view into the internal array used by BitVector.
    /// @return Read-only span of BitVector's bytes.
    std::span<const std::byte> GetBytes() const;

    /// Update the byte at the specified byte index.
    void SetByte(size_t index, uint8_t value);

    /// Get specified type at a given index (in steps of that type size)
    template <typename T> T GetValueAtIndex(const size_t index) const;

    // Retrieve the 8-bit uint from the specified uint index.
    uint8_t GetUInt8(size_t index) const { return GetValueAtIndex<uint8_t>(index); }

    // Retrieve the 16-bit uint from the specified uint index.
    uint16_t GetUInt16(size_t index) const { return GetValueAtIndex<uint16_t>(index); }

    // Retrieve the 32-bit uint from the specified uint index.
    uint32_t GetUInt32(size_t index) const { return GetValueAtIndex<uint32_t>(index); }
    
    // Retrieve the 64-bit uint from the specified uint index.
    uint64_t GetUInt64(size_t index) const { return GetValueAtIndex<uint64_t>(index); }

    // By default, retrieve the 32-bit uint from the specified uint index.
    uint32_t GetUInt(size_t index) const { return GetUInt32(index); }


    /// Set specified type at a given index (in steps of that type size)
    template <typename T> void SetValueAtIndex(const size_t index, T value);

    /// Update the 8-bit uint at the specified uint index.
    void SetUInt8(const size_t index, uint8_t value) { SetValueAtIndex(index, value); }

    /// Update the 16-bit uint at the specified uint index.
    void SetUInt16(const size_t index, uint16_t value) { SetValueAtIndex(index, value); }

    /// Update the 32-bit uint at the specified uint index.
    void SetUInt32(const size_t index, uint32_t value) { SetValueAtIndex(index, value); }

    /// Update the 64-bit uint at the specified uint index.
    void SetUInt64(const size_t index, uint64_t value) { SetValueAtIndex(index, value); }

    /// By default, update the 32-bit uint at the specified uint index.
    void SetUInt(const size_t index, uint32_t value) { SetUInt32(index, value); }


    /// Get specified type starting at a given BIT position.
    template <typename T> T GetValueAtBit(const size_t index) const;

    // Retrieve the 8-bit uint from the specified uint index.
    uint8_t GetUInt8AtBit(size_t index) const { return GetValueAtBit<uint8_t>(index); }

    // Retrieve the 16-bit uint from the specified uint index.
    uint16_t GetUInt16AtBit(size_t index) const { return GetValueAtBit<uint16_t>(index); }

    // Retrieve the 32-bit uint from the specified uint index.
    uint32_t GetUInt32AtBit(size_t index) const { return GetValueAtBit<uint32_t>(index); }
    
    // Retrieve the 64-bit uint from the specified uint index.
    uint64_t GetUInt64AtBit(size_t index) const { return GetValueAtBit<uint64_t>(index); }

    // By default, retrieve the 32-bit uint from the specified uint index.
    uint32_t GetUIntAtBit(size_t index) const { return GetUInt32AtBit(index); }


    template <typename T> void SetValueAtBit(const size_t index, T value);

    /// Update the 8-bit uint at the specified uint index.
    void SetUInt8AtBit(const size_t index, uint8_t value) { SetValueAtBit(index, value); }

    /// Update the 16-bit uint at the specified uint index.
    void SetUInt16AtBit(const size_t index, uint16_t value) { SetValueAtBit(index, value); }

    /// Update the 32-bit uint at the specified uint index.
    void SetUInt32AtBit(const size_t index, uint32_t value) { SetValueAtBit(index, value); }

    /// Update the 64-bit uint at the specified uint index.
    void SetUInt64AtBit(const size_t index, uint64_t value) { SetValueAtBit(index, value); }

    /// By default, update the 32-bit uint at the specified uint index.
    void SetUIntAtBit(const size_t index, uint32_t value) { SetUInt32AtBit(index, value); }


    // >>>>>>>>>>  Other Analyses  <<<<<<<<<< //

    /// A simple hash function for bit vectors.
    std::size_t Hash() const;

    /// Count the number of ones in the BitVector.
    size_t CountOnes() const;

    /// Faster counting of ones for very sparse bit vectors.
    size_t CountOnes_Sparse() const;

    /// Count the number of zeros in the BitVector.
    size_t CountZeros() const { return GetSize() - CountOnes(); }

    /// Return the position of the first one; return -1 if no ones in vector.
    int FindBit() const;

    /// Return the position of the first one after start_pos; return -1 if no ones in vector.
    /// You can loop through all 1-bit positions of a BitVector "bv" with:
    ///
    ///   for (int pos = bv.FindBit(); pos >= 0; pos = bv.FindBit(pos+1)) { ... }
    ///
    int FindBit(const size_t start_pos) const;

    /// Return the position of the first one and change it to a zero.  Return -1 if no ones.
    int PopBit();

    /// Return positions of all ones.
    emp::vector<size_t> GetOnes() const;


    // >>>>>>>>>>  Print/String Functions  <<<<<<<<<< //

    /// Convert a specified bit to a character.
    char GetAsChar(size_t id) const { return Get(id) ? '1' : '0'; }

    /// Convert this BitVector to a string.
    std::string ToString() const;

    /// Regular print function (from most significant bit to least)
    void Print(std::ostream & out=std::cout) const;

    /// Print a space between each field (or other provided spacer)
    void PrintFields(std::ostream & out=std::cout, const std::string & spacer=" ") const;

    /// Print from smallest bit position to largest.
    void PrintArray(std::ostream & out=std::cout) const;

    /// Print the positions of all one bits, spaces are the default separator.
    void PrintOneIDs(std::ostream & out=std::cout, const std::string & spacer=" ") const;

    /// Print the ones in a range format.  E.g., 2-5,7,10-15
    void PrintAsRange(std::ostream & out=std::cout,
                      const std::string & spacer=",",
                      const std::string & ranger="-") const;


    // >>>>>>>>>>  Boolean Logic and Shifting Operations  <<<<<<<<<< //

    /// Perform a Boolean NOT on this BitVector and return the result.
    BitVector NOT() const;

    /// Perform a Boolean AND on this BitVector and return the result.
    BitVector AND(const BitVector & bv2) const;

    /// Perform a Boolean OR on this BitVector and return the result.
    BitVector OR(const BitVector & bv2) const;

    /// Perform a Boolean NAND on this BitVector and return the result.
    BitVector NAND(const BitVector & bv2) const;

    /// Perform a Boolean NOR on this BitVector and return the result.
    BitVector NOR(const BitVector & bv2) const;

    /// Perform a Boolean XOR on this BitVector and return the result.
    BitVector XOR(const BitVector & bv2) const;

    /// Perform a Boolean EQU on this BitVector and return the result.
    BitVector EQU(const BitVector & bv2) const;

    /// Perform a Boolean NOT with this BitVector, store result here, and return this object.
    BitVector & NOT_SELF();

    /// Perform a Boolean AND with this BitVector, store result here, and return this object.
    BitVector & AND_SELF(const BitVector & bv2);

    /// Perform a Boolean OR with this BitVector, store result here, and return this object.
    BitVector & OR_SELF(const BitVector & bv2);

    /// Perform a Boolean NAND with this BitVector, store result here, and return this object.
    BitVector & NAND_SELF(const BitVector & bv2);

    /// Perform a Boolean NOR with this BitVector, store result here, and return this object.
    BitVector & NOR_SELF(const BitVector & bv2);

    /// Perform a Boolean XOR with this BitVector, store result here, and return this object.
    BitVector & XOR_SELF(const BitVector & bv2);

    /// Perform a Boolean EQU with this BitVector, store result here, and return this object.
    BitVector & EQU_SELF(const BitVector & bv2);

    /// Positive shifts go left and negative go right (0 does nothing); return result.
    BitVector SHIFT(const int shift_size) const;

    /// Positive shifts go left and negative go right; store result here, and return this object.
    BitVector & SHIFT_SELF(const int shift_size);

    /// Operator bitwise NOT...
    BitVector operator~() const { return NOT(); }

    /// Operator bitwise AND...
    BitVector operator&(const BitVector & ar2) const { return AND(ar2); }

    /// Operator bitwise OR...
    BitVector operator|(const BitVector & ar2) const { return OR(ar2); }

    /// Operator bitwise XOR...
    BitVector operator^(const BitVector & ar2) const { return XOR(ar2); }

    /// Operator shift left...
    inline BitVector operator<<(const size_t shift_size) const { return SHIFT(-(int)shift_size); }

    /// Operator shift right...
    inline BitVector operator>>(const size_t shift_size) const { return SHIFT((int)shift_size); }

    /// Compound operator bitwise AND...
    BitVector & operator&=(const BitVector & ar2) { return AND_SELF(ar2); }

    /// Compound operator bitwise OR...
    BitVector & operator|=(const BitVector & ar2) { return OR_SELF(ar2); }

    /// Compound operator bitwise XOR...
    BitVector & operator^=(const BitVector & ar2) { return XOR_SELF(ar2); }

    /// Compound operator for shift left...
    BitVector & operator<<=(const size_t shift_size) { return SHIFT_SELF(-(int)shift_size); }

    /// Compound operator for shift right...
    BitVector & operator>>=(const size_t shift_size) { return SHIFT_SELF((int)shift_size); }

    // >>>>>>>>>>  Standard Library Compatability  <<<<<<<<<< //
    // A set of functions to allow drop-in replacement with std::bitset.

    size_t size() const { return num_bits; }
    void resize(std::size_t new_size) { Resize(new_size); }
    bool all() const { return All(); }
    bool any() const { return Any(); }
    bool none() const { return !Any(); }
    size_t count() const { return CountOnes(); }
    BitVector & flip() { return Toggle(); }
    BitVector & flip(size_t pos) { return Toggle(pos); }
    BitVector & flip(size_t start, size_t end) { return Toggle(start, end); }
    void reset() { Clear(); }
    void reset(size_t id) { Set(id, false); }
    void set() { SetAll(); }
    void set(size_t id) { Set(id); }
    bool test(size_t index) const { return Get(index); }
  };



  // ------------------------ Implementations for Internal Functions ------------------------

  void BitVector::RawCopy(const Ptr<BitVector::field_t> in) {
    #ifdef EMP_TRACK_MEM
    emp_assert(in.IsNull() == false);
    emp_assert(bits.DebugIsArray() && in.DebugIsArray());
    emp_assert(bits.DebugGetArrayBytes() == in.DebugGetArrayBytes(),
                bits.DebugGetArrayBytes(), in.DebugGetArrayBytes());
    #endif

    const size_t NUM_FIELDS = NumFields();
    for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = in[i];
  }

  void BitVector::ShiftLeft(const size_t shift_size) {
    // If we are shifting out of range, clear the bits and stop.
    if (shift_size >= num_bits) { Clear(); return; }

    const size_t field_shift = shift_size / FIELD_BITS;
    const size_t bit_shift = shift_size % FIELD_BITS;
    const size_t bit_overflow = FIELD_BITS - bit_shift;
    const size_t LAST_FIELD = NumFields() - 1;

    // Loop through each field, from L to R, and update it.
    if (field_shift) {
      for (size_t i = LAST_FIELD; i >= field_shift; --i) {
        bits[i] = bits[i - field_shift];
      }
      for (size_t i = field_shift; i > 0; --i) bits[i-1] = 0;
    }

    // account for bit_shift
    if (bit_shift) {
      for (size_t i = LAST_FIELD; i > field_shift; --i) {
        bits[i] <<= bit_shift;
        bits[i] |= (bits[i-1] >> bit_overflow);
      }
      // Handle final field (field_shift position)
      bits[field_shift] <<= bit_shift;
    }

    // Mask out any bits that have left-shifted away
    ClearExcessBits();
  }

  void BitVector::ShiftRight(const size_t shift_size) {
    // If we are shifting out of range, clear the bits and stop.
    if (shift_size >= num_bits) { Clear(); return; }

    const size_t field_shift = shift_size / FIELD_BITS;
    const size_t bit_shift = shift_size % FIELD_BITS;
    const size_t bit_overflow = FIELD_BITS - bit_shift;
    const size_t NUM_FIELDS = NumFields();
    const size_t field_shift2 = NUM_FIELDS - field_shift;

    // account for field_shift
    if (field_shift) {
      for (size_t i = 0; i < field_shift2; ++i) {
        bits[i] = bits[i + field_shift];
      }
      for (size_t i = field_shift2; i < NUM_FIELDS; i++) bits[i] = FIELD_0;
    }

    // account for bit_shift
    if (bit_shift) {
      for (size_t i = 0; i < (field_shift2 - 1); ++i) {
        bits[i] >>= bit_shift;
        bits[i] |= (bits[i+1] << bit_overflow);
      }
      bits[field_shift2 - 1] >>= bit_shift;
    }
  }

  bool BitVector::OK() const {
    // Do some checking on the bits array ptr to make sure it's value.
    if (bits) {
#ifdef EMP_TRACK_MEM
      emp_assert(bits.DebugIsArray()); // Must be marked as an array.
      emp_assert(bits.OK());           // Pointer must be okay.
#endif
    }

    // Otherwise bits is null; num_bits should be zero.
    else emp_assert(num_bits == 0);

    // Make sure final bits are zeroed out.
    [[maybe_unused]] field_t excess_bits = bits[NumFields() - 1] & ~MaskLow<field_t>(NumEndBits());
    emp_assert(!excess_bits);

    return true;
  }


  // ------------------- Implementations of Constructors and Assignments --------------------

  /// Build a new BitVector with specified bit count (default 0) and initialization (default 0)
  BitVector::BitVector(size_t in_num_bits, bool init_val) : num_bits(in_num_bits), bits(nullptr) {
    if (num_bits) bits = NewArrayPtr<field_t>(NumFields());
    if (init_val) SetAll(); else Clear();
  }

  /// Copy constructor of existing bit field.
  BitVector::BitVector(const BitVector & in) : num_bits(in.num_bits), bits(nullptr) {
    emp_assert(in.OK());

    // There is only something to copy if there are a non-zero number of bits!
    if (num_bits) {
      #ifdef EMP_TRACK_MEM
      emp_assert(!in.bits.IsNull() && in.bits.DebugIsArray(), in.bits.IsNull(), in.bits.DebugIsArray());
      #endif
      bits = NewArrayPtr<field_t>(NumFields());
      RawCopy(in.bits);
    }
  }

  /// Move constructor of existing bit field.
  BitVector::BitVector(BitVector && in) : num_bits(in.num_bits), bits(in.bits) {
    emp_assert(in.OK());

    in.bits = nullptr;
    in.num_bits = 0;
  }

  /// Constructor to generate a random BitVector (with equal prob of 0 or 1).
  BitVector::BitVector(size_t in_num_bits, Random & random)
  : num_bits(in_num_bits), bits(nullptr)
  {
    if (num_bits) {
      bits = NewArrayPtr<field_t>(NumFields());
      Randomize(random);
    }
  }

  /// Constructor to generate a random BitVector with provided prob of 1's.
  BitVector::BitVector(size_t in_num_bits, Random & random, const double p1)
  : num_bits(in_num_bits), bits(nullptr)
  {
    if (num_bits) {
      bits = NewArrayPtr<field_t>(NumFields());
      Randomize(random, p1);
    }
  }

  /// Constructor to generate a random BitVector with provided number of 1's.
  BitVector::BitVector(size_t in_num_bits, Random & random, const size_t target_ones)
  : num_bits(in_num_bits), bits(nullptr)
  {
    if (num_bits) {
      bits = NewArrayPtr<field_t>(NumFields());
      Randomize(random, target_ones);
    }
  }

  /// Initializer list constructor.
  template <typename T>
  BitVector::BitVector(const std::initializer_list<T> l) : num_bits(l.size()), bits(nullptr) {
    if (num_bits) bits = NewArrayPtr<field_t>(NumFields());

    size_t idx = 0;
    for (auto i = std::rbegin(l); i != std::rend(l); ++i) Set(idx++, *i);
    ClearExcessBits();
  }

  /// Copy, but with a resize.
  BitVector::BitVector(const BitVector & in, size_t new_size) : BitVector(in) {
    if (num_bits != new_size) Resize(new_size);
  }

  /// Destructor
  BitVector::~BitVector() {
    if (bits) {        // A move constructor can make bits == nullptr
      bits.DeleteArray();
      bits = nullptr;
    }
  }

  /// Assignment operator.
  BitVector & BitVector::operator=(const BitVector & in) {
    emp_assert(in.OK());

    if (&in == this) return *this;
    const size_t in_num_fields = in.NumFields();
    const size_t prev_num_fields = NumFields();
    num_bits = in.num_bits;

    if (in_num_fields != prev_num_fields) {
      if (bits) bits.DeleteArray();
      if (num_bits) bits = NewArrayPtr<field_t>(in_num_fields);
      else bits = nullptr;
    }

    if (num_bits) RawCopy(in.bits);

    return *this;
  }

  /// Move operator.
  BitVector & BitVector::operator=(BitVector && in) {
    emp_assert(&in != this);        // in is an r-value, so this shouldn't be possible...
    if (bits) bits.DeleteArray();   // If we already had a bitset, get rid of it.
    num_bits = in.num_bits;         // Update the number of bits...
    bits = in.bits;                 // And steal the old memory for what those bits are.
    in.bits = nullptr;              // Prepare in for deletion without deallocating.
    in.num_bits = 0;

    return *this;
  }

    /// Assign from a BitSet of a different size.
  BitVector & BitVector::Import(const BitVector & from_bv, const size_t from_bit) {
    emp_assert(&from_bv != this);
    emp_assert(from_bit < from_bv.GetSize());

    size_t init_size = GetSize();
    *this = from_bv;
    *this << from_bit;
    Resize(init_size);

    return *this;
  }

  /// Convert to a Bitset of a different size.
  BitVector BitVector::Export(size_t out_size, size_t start_bit) const {

    BitVector out_bits(out_size);
    out_bits.Import(*this, start_bit);

    return out_bits;
  }


  // --------------------  Implementations of common accessors -------------------
  
  /// Retrive the bit value from the specified index.
  bool BitVector::Get(size_t index) const {
    emp_assert(index < num_bits, index, num_bits);
    const size_t field_id = FieldID(index);
    const size_t pos_id = FieldPos(index);
    return (bits[field_id] & (static_cast<field_t>(1) << pos_id)) != 0;
  }

  /// Update the bit value at the specified index.
  BitVector & BitVector::Set(size_t index, bool value) {
    emp_assert(index < num_bits, index, num_bits);
    const size_t field_id = FieldID(index);
    const size_t pos_id = FieldPos(index);
    const field_t pos_mask = FIELD_1 << pos_id;

    if (value) bits[field_id] |= pos_mask;
    else       bits[field_id] &= ~pos_mask;

    return *this;
  }

  /// Set all bits to 1.
  BitVector & BitVector::SetAll() {
    const size_t NUM_FIELDS = NumFields();
    for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = FIELD_ALL;
    ClearExcessBits();
    return *this;
  }

  /// Set a range of bits to one: [start, stop)
  BitVector & BitVector::SetRange(size_t start, size_t stop) {
    emp_assert(start <= stop, start, stop, num_bits);
    emp_assert(stop <= num_bits, stop, num_bits);
    const size_t start_pos = FieldPos(start);
    const size_t stop_pos = FieldPos(stop);
    size_t start_field = FieldID(start);
    const size_t stop_field = FieldID(stop);

    // If the start field and stop field are the same, just step through the bits.
    if (start_field == stop_field) {
      const size_t num_bits = stop - start;
      const field_t mask = MaskLow<field_t>(num_bits) << start_pos;
      bits[start_field] |= mask;
    }

    // Otherwise handle the ends and clear the chunks in between.
    else {
      // Set portions of start field
      if (start_pos != 0) {
        const size_t start_bits = FIELD_BITS - start_pos;
        const field_t start_mask = MaskLow<field_t>(start_bits) << start_pos;
        bits[start_field] |= start_mask;
        start_field++;
      }

      // Middle fields
      for (size_t cur_field = start_field; cur_field < stop_field; cur_field++) {
        bits[cur_field] = FIELD_ALL;
      }

      // Set portions of stop field
      const field_t stop_mask = MaskLow<field_t>(stop_pos);
      bits[stop_field] |= stop_mask;
    }

    return *this;
  }
  
  /// Set all bits to 0.
  BitVector & BitVector::Clear() {
    const size_t NUM_FIELDS = NumFields();
    for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = FIELD_0;
    return *this;
  }

  /// Set a range of bits to 0 in the range [start, stop)
  BitVector & BitVector::Clear(const size_t start, const size_t stop) {
    emp_assert(start <= stop, start, stop, num_bits);
    emp_assert(stop <= num_bits, stop, num_bits);
    const size_t start_pos = FieldPos(start);
    const size_t stop_pos = FieldPos(stop);
    size_t start_field = FieldID(start);
    const size_t stop_field = FieldID(stop);

    // If the start field and stop field are the same, just step through the bits.
    if (start_field == stop_field) {
      const size_t num_bits = stop - start;
      const field_t mask = ~(MaskLow<field_t>(num_bits) << start_pos);
      bits[start_field] &= mask;
    }

    // Otherwise handle the ends and clear the chunks in between.
    else {
      // Clear portions of start field
      if (start_pos != 0) {
        const size_t start_bits = FIELD_BITS - start_pos;
        const field_t start_mask = ~(MaskLow<field_t>(start_bits) << start_pos);
        bits[start_field] &= start_mask;
        start_field++;
      }

      // Middle fields
      for (size_t cur_field = start_field; cur_field < stop_field; cur_field++) {
        bits[cur_field] = 0;
      }

      // Clear portions of stop field
      const field_t stop_mask = ~MaskLow<field_t>(stop_pos);
      bits[stop_field] &= stop_mask;
    }

    return *this;
  }

  /// Change a specified bit to the opposite value
  BitVector & BitVector::Toggle(size_t index) {
    emp_assert(index < num_bits, index, num_bits);
    const size_t field_id = FieldID(index);
    const size_t pos_id = FieldPos(index);
    const field_t pos_mask = FIELD_1 << pos_id;

    bits[field_id] ^= pos_mask;

    return *this;
  }

  /// Flips all the bits in a range [start, end)
  BitVector & BitVector::Toggle(size_t start, size_t stop) {
    emp_assert(start <= stop, start, stop, num_bits);
    emp_assert(stop <= num_bits, stop, num_bits);
    const size_t start_pos = FieldPos(start);
    const size_t stop_pos = FieldPos(stop);
    size_t start_field = FieldID(start);
    const size_t stop_field = FieldID(stop);

    // If the start field and stop field are the same, just step through the bits.
    if (start_field == stop_field) {
      const size_t num_flips = stop - start;
      const field_t mask = MaskLow<field_t>(num_flips) << start_pos;
      bits[start_field] ^= mask;
    }

    // Otherwise handle the ends and clear the chunks in between.
    else {
      // Toggle correct portions of start field
      if (start_pos != 0) {
        const size_t start_bits = FIELD_BITS - start_pos;
        const field_t start_mask = MaskLow<field_t>(start_bits) << start_pos;
        bits[start_field] ^= start_mask;
        start_field++;
      }

      // Middle fields
      for (size_t cur_field = start_field; cur_field < stop_field; cur_field++) {
        bits[cur_field] = ~bits[cur_field];
      }

      // Set portions of stop field
      const field_t stop_mask = MaskLow<field_t>(stop_pos);
      bits[stop_field] ^= stop_mask;
    }

    return *this;
  }

  bool BitVector::Any() const {
    const size_t NUM_FIELDS = NumFields();
    for (size_t i = 0; i < NUM_FIELDS; i++) {
      if (bits[i]) return true;
    }
    return false;
  }

  /// Resize this BitVector to have the specified number of bits.
  BitVector & BitVector::Resize(size_t new_bits) {
    const size_t old_num_fields = NumFields();
    num_bits = new_bits;
    const size_t NUM_FIELDS = NumFields();

    if (NUM_FIELDS == old_num_fields) {   // We can use our existing bit field
      num_bits = new_bits;
      ClearExcessBits();                  // If there are extra bits, zero them out.
    }

    else {  // We must change the number of bitfields.  Resize & copy old info.
      Ptr<field_t> old_bits = bits;
      if (num_bits > 0) bits = NewArrayPtr<field_t>(NUM_FIELDS);
      else bits = nullptr;
      const size_t min_fields = std::min(old_num_fields, NUM_FIELDS);
      for (size_t i = 0; i < min_fields; i++) bits[i] = old_bits[i];
      for (size_t i = min_fields; i < NUM_FIELDS; i++) bits[i] = FIELD_0;
      if (old_bits) old_bits.DeleteArray();
    }

    return *this;
  }

  // -------------------------  Implementations Randomization functions -------------------------

  /// Set all bits randomly, with a 50% probability of being a 0 or 1.
  BitVector & BitVector::Randomize(Random & random) {
    random.RandFill(BytePtr(), NumBytes());
    ClearExcessBits();
    return *this;
  }

  /// Set all bits randomly, with probability specified at compile time.
  template <Random::Prob P>
  BitVector & BitVector::RandomizeP(Random & random,
                                    const size_t start_pos, size_t stop_pos) {
    if (stop_pos == MAX_BITS) stop_pos = num_bits;

    emp_assert(start_pos <= stop_pos);
    emp_assert(stop_pos <= num_bits);
    random.RandFillP<P>(BytePtr(), NumBytes(), start_pos, stop_pos);
    return *this;
  }


  /// Set all bits randomly, with a given probability of being on.
  BitVector & BitVector::Randomize(Random & random, const double p,
                                   const size_t start_pos, size_t stop_pos) {
    if (stop_pos == MAX_BITS) stop_pos = num_bits;

    emp_assert(start_pos <= stop_pos, start_pos, stop_pos);
    emp_assert(stop_pos <= num_bits, stop_pos, num_bits);
    emp_assert(p >= 0.0 && p <= 1.0, p);
    random.RandFill(BytePtr(), NumBytes(), p, start_pos, stop_pos);
    return *this;
  }

  /// Set all bits randomly, with a given number of them being on.
  BitVector & BitVector::Randomize(Random & random, const size_t target_ones,
                                   const size_t start_pos, size_t stop_pos) {
    if (stop_pos == MAX_BITS) stop_pos = num_bits;

    emp_assert(start_pos <= stop_pos);
    emp_assert(stop_pos <= num_bits);

    const size_t target_size = stop_pos - start_pos;
    emp_assert(target_ones <= target_size);

    // Approximate the probability of ones as a starting point.
    double p = ((double) target_ones) / (double) target_size;

    // If we are not randomizing the whole sequence, we need to track the number of ones
    // in the NON-randomized region to subtract off later.
    size_t kept_ones = 0;
    if (target_size != num_bits) {
      Clear(start_pos, stop_pos);
      kept_ones = CountOnes();
    }

    // Try to find a shortcut if p allows....
    // (These values are currently educated guesses)
    if (p < 0.12) { if (target_size == num_bits) Clear(start_pos, stop_pos); }
    else if (p < 0.2)  RandomizeP<Random::PROB_12_5>(random, start_pos, stop_pos);
    else if (p < 0.35) RandomizeP<Random::PROB_25>(random, start_pos, stop_pos);
    else if (p < 0.42) RandomizeP<Random::PROB_37_5>(random, start_pos, stop_pos);
    else if (p < 0.58) RandomizeP<Random::PROB_50>(random, start_pos, stop_pos);
    else if (p < 0.65) RandomizeP<Random::PROB_62_5>(random, start_pos, stop_pos);
    else if (p < 0.8)  RandomizeP<Random::PROB_75>(random, start_pos, stop_pos);
    else if (p < 0.88) RandomizeP<Random::PROB_87_5>(random, start_pos, stop_pos);
    else SetRange(start_pos, stop_pos);

    size_t cur_ones = CountOnes() - kept_ones;

    // Do we need to add more ones?
    while (cur_ones < target_ones) {
      size_t pos = random.GetUInt(start_pos, stop_pos);
      auto bit = operator[](pos);
      if (!bit) {
        bit.Set();
        cur_ones++;
      }
    }

    // See if we have too many ones.
    while (cur_ones > target_ones) {
      size_t pos = random.GetUInt(start_pos, stop_pos);
      auto bit = operator[](pos);
      if (bit) {
        bit.Clear();
        cur_ones--;
      }
    }

    return *this;
  }

  /// Flip random bits with a given probability.
  // @CAO: Possibly faster to generate a sequence of bits and XORing with them.
  BitVector & BitVector::FlipRandom(Random & random,
                                                  const double p,
                                                  const size_t start_pos,
                                                  size_t stop_pos)
  {
    if (stop_pos == MAX_BITS) stop_pos = num_bits;

    emp_assert(start_pos <= stop_pos);
    emp_assert(stop_pos <= num_bits);
    emp_assert(p >= 0.0 && p <= 1.0, p);

    for (size_t i=start_pos; i < stop_pos; ++i) if (random.P(p)) Toggle(i);

    return *this;
  }

  /// Set random bits with a given probability (does not check if already set.)
  BitVector & BitVector::SetRandom(Random & random,
                      const double p,
                      const size_t start_pos,
                      size_t stop_pos)
  {
    if (stop_pos == MAX_BITS) stop_pos = num_bits;

    emp_assert(start_pos <= stop_pos);
    emp_assert(stop_pos <= num_bits);
    emp_assert(p >= 0.0 && p <= 1.0, p);

    for (size_t i=start_pos; i < stop_pos; ++i) if (random.P(p)) Set(i);

    return *this;
  }

  /// Unset random bits with a given probability (does not check if already zero.)
  BitVector & BitVector::ClearRandom(Random & random,
                      const double p,
                      const size_t start_pos,
                      size_t stop_pos)
  {
    if (stop_pos == MAX_BITS) stop_pos = num_bits;

    emp_assert(start_pos <= stop_pos);
    emp_assert(stop_pos <= num_bits);
    emp_assert(p >= 0.0 && p <= 1.0, p);

    for (size_t i=start_pos; i < stop_pos; ++i) if (random.P(p)) Clear(i);

    return *this;
  }

  /// Flip a specified number of random bits.
  BitVector & BitVector::FlipRandom(Random & random, const size_t target_bits)
  {
    emp_assert(num_bits <= num_bits);
    BitVector choice(num_bits, random, target_bits);
    return XOR_SELF(choice);
  }

  /// Set a specified number of random bits (does not check if already set.)
  BitVector & BitVector::SetRandom(Random & random, const size_t target_bits)
  {
    emp_assert(num_bits <= num_bits);
    BitVector choice(num_bits, random, target_bits);
    return OR_SELF(choice);
  }

  /// Unset  a specified number of random bits (does not check if already zero.)
  BitVector & BitVector::ClearRandom(Random & random, const size_t target_bits)
  {
    emp_assert(num_bits <= num_bits);
    BitVector choice(num_bits, random, num_bits - target_bits);
    return AND_SELF(choice);
  }


  // -------------------------  Implementations of Comparison Operators -------------------------

  /// Test if two bit vectors are identical.
  bool BitVector::operator==(const BitVector & in) const {
    if (num_bits != in.num_bits) return false;

    const size_t NUM_FIELDS = NumFields();
    for (size_t i = 0; i < NUM_FIELDS; ++i) {
      if (bits[i] != in.bits[i]) return false;
    }
    return true;
  }

  /// Compare the would-be numerical values of two bit vectors.
  bool BitVector::operator<(const BitVector & in) const {
    if (num_bits != in.num_bits) return num_bits < in.num_bits;

    const size_t NUM_FIELDS = NumFields();
    for (size_t i = NUM_FIELDS; i > 0; --i) {   // Start loop at the largest field.
      const size_t pos = i-1;
      if (bits[pos] == in.bits[pos]) continue;  // If same, keep looking!
      return (bits[pos] < in.bits[pos]);        // Otherwise, do comparison
    }
    return false; // Bit vectors are identical.
  }

  /// Automatically convert BitVector to other vector types.
  template <typename T>
  BitVector::operator emp::vector<T>() {
    emp::vector<T> out(GetSize());
    for (size_t i = 0; i < GetSize(); i++) {
      out[i] = (T) Get(i);
    }
    return out;
  }


  // -------------------------  Access Groups of bits -------------------------

  /// Retrive the byte at the specified byte index.
  uint8_t BitVector::GetByte(size_t index) const {
    emp_assert(index < NumBytes(), index, NumBytes());
    const size_t field_id = Byte2Field(index);
    const size_t pos_id = Byte2FieldPos(index);
    return (bits[field_id] >> pos_id) & 255U;
  }

  /// Get a read-only view into the internal array used by BitVector.
  /// @return Read-only span of BitVector's bytes.
  std::span<const std::byte> BitVector::GetBytes() const {
    return std::span<const std::byte>(
      bits.ReinterpretCast<const std::byte>(),
      NumBytes()
    );
  }

  /// Update the byte at the specified byte index.
  void BitVector::SetByte(size_t index, uint8_t value) {
    emp_assert(index < NumBytes(), index, NumBytes());
    const size_t field_id = Byte2Field(index);
    const size_t pos_id = Byte2FieldPos(index);
    const field_t val_uint = value;
    bits[field_id] = (bits[field_id] & ~(FIELD_255 << pos_id)) | (val_uint << pos_id);
  }


  /// Get specified type at a given index (in steps of that type size)
  template <typename T>
  T BitVector::GetValueAtIndex(const size_t index) const {
    // For the moment, must fit inside bounds; eventually should pad with zeros.
    emp_assert((index + 1) * sizeof(T) <= NumBytes());

    T out_value;
    std::memcpy( &out_value, BytePtr() + index * sizeof(T), sizeof(T) );
    return out_value;
  }


  /// Set specified type at a given index (in steps of that type size)
  template <typename T>
  void BitVector::SetValueAtIndex(const size_t index, T in_value) {
    // For the moment, must fit inside bounds; eventually should pad with zeros.
    emp_assert((index + 1) * sizeof(T) <= NumBytes());

    std::memcpy( BytePtr() + index * sizeof(T), &in_value, sizeof(T) );

    ClearExcessBits();
  }


  /// Get the specified type starting from a given BIT position.
  template <typename T>
  T BitVector::GetValueAtBit(const size_t index) const {
    // For the moment, must fit inside bounds; eventually should pad with zeros.
    emp_assert((index+7)/8 + sizeof(T) < NumBytes());

    BitVector out_bits(sizeof(T));
    out_bits.Import(*this, index);

    return out_bits.template GetValueAtIndex<T>(0);
  }


  /// Set the specified type starting from a given BIT position.
  // @CAO: Can be optimized substantially, especially for long BitVectors.
  template <typename T>
  void BitVector::SetValueAtBit(const size_t index, T value) {
    // For the moment, must fit inside bounds; eventually should pad with zeros.
    emp_assert((index+7)/8 + sizeof(T) < NumBytes());
    constexpr size_t type_bits = sizeof(T) * 8;

    Clear(index, index+type_bits);       // Clear out the bits where new value will go.
    BitVector in_bits(GetSize());        // Setup a bitset to place the new bits in.
    in_bits.SetValueAtIndex(0, value);   // Insert the new bits.
    in_bits << index;                    // Shift new bits into place.
    OR_SELF(in_bits);                    // Place new bits into current BitVector.
  }


  // -------------------------  Other Analyses -------------------------

  /// A simple hash function for bit vectors.
  std::size_t BitVector::Hash() const {
    std::size_t hash_val = 0;
    const size_t NUM_FIELDS = NumFields();
    for (size_t i = 0; i < NUM_FIELDS; i++) {
      hash_val ^= bits[i] + i*1000000009;
    }
    return hash_val ^ ((97*num_bits) << 8);
  }

  // TODO: see https://arxiv.org/pdf/1611.07612.pdf for fast pop counts
  /// Count the number of ones in the BitVector.
  size_t BitVector::CountOnes() const { 
    const field_t NUM_FIELDS = (1 + ((num_bits - 1) / FIELD_BITS));
    size_t bit_count = 0;
    for (size_t i = 0; i < NUM_FIELDS; i++) {
        // when compiling with -O3 and -msse4.2, this is the fastest population count method.
        std::bitset<FIELD_BITS> std_bs(bits[i]);
        bit_count += std_bs.count();
      }

    return bit_count;
  }

  /// Faster counting of ones for very sparse bit vectors.
  size_t BitVector::CountOnes_Sparse() const {
    size_t bit_count = 0;
    const size_t NUM_FIELDS = NumFields();
    for (size_t i = 0; i < NUM_FIELDS; ++i) {
      field_t cur_field = bits[i];
      while (cur_field) {
        cur_field &= (cur_field-1);       // Peel off a single 1.
        bit_count++;                      // Increment the counter
      }
    }
    return bit_count;
  }

  /// Return the position of the first one; return -1 if no ones in vector.
  int BitVector::FindBit() const {
    const size_t NUM_FIELDS = NumFields();
    size_t field_id = 0;
    while (field_id < NUM_FIELDS && bits[field_id]==0) field_id++;
    return (field_id < NUM_FIELDS) ?
      (int) (find_bit(bits[field_id]) + (field_id * FIELD_BITS))  :  -1;
  }

  /// Return the position of the first one after start_pos; return -1 if no ones in vector.
  /// You can loop through all 1-bit positions of a BitVector "bv" with:
  ///
  ///   for (int pos = bv.FindBit(); pos >= 0; pos = bv.FindBit(pos+1)) { ... }

  int BitVector::FindBit(const size_t start_pos) const {
    if (start_pos >= num_bits) return -1;
    size_t field_id  = FieldID(start_pos);     // What field do we start in?
    const size_t field_pos = FieldPos(start_pos);    // What position in that field?
    if (field_pos && (bits[field_id] & ~(MaskLow<field_t>(field_pos)))) {  // First field hit!
      return (int) (find_bit(bits[field_id] & ~(MaskLow<field_t>(field_pos))) +
                    field_id * FIELD_BITS);
    }

    // Search other fields...
    const size_t NUM_FIELDS = NumFields();
    if (field_pos) field_id++;
    while (field_id < NUM_FIELDS && bits[field_id]==0) field_id++;
    return (field_id < NUM_FIELDS) ?
      (int) (find_bit(bits[field_id]) + (field_id * FIELD_BITS)) : -1;
  }

  /// Return the position of the first one and change it to a zero.  Return -1 if no ones.
  int BitVector::PopBit() {
    const size_t NUM_FIELDS = NumFields();
    size_t field_id = 0;
    while (field_id < NUM_FIELDS && bits[field_id]==0) field_id++;
    if (field_id == NUM_FIELDS) return -1;  // Failed to find bit!

    const size_t pos_found = find_bit(bits[field_id]);
    bits[field_id] &= ~(FIELD_1 << pos_found);
    return (int) (pos_found + (field_id * FIELD_BITS));
  }

  /// Return positions of all ones.
  emp::vector<size_t> BitVector::GetOnes() const {
    // @CAO -- There are better ways to do this with bit tricks.
    emp::vector<size_t> out_vals(CountOnes());
    size_t cur_pos = 0;
    for (size_t i = 0; i < num_bits; i++) {
      if (Get(i)) out_vals[cur_pos++] = i;
    }
    return out_vals;
  }

  /// Convert this BitVector to a string.
  std::string BitVector::ToString() const {
    std::string out_string;
    out_string.reserve(num_bits);
    for (size_t i = num_bits; i > 0; --i) out_string.push_back(GetAsChar(i-1));
    return out_string;
  }

  /// Regular print function (from most significant bit to least)
  void BitVector::Print(std::ostream & out) const {
    for (size_t i = num_bits; i > 0; --i) out << Get(i-1);
  }

  /// Print a space between each field (or other provided spacer)
  void BitVector::PrintFields(std::ostream & out, const std::string & spacer) const {
    for (size_t i = num_bits-1; i < num_bits; i--) {
      out << Get(i);
      if (i && (i % FIELD_BITS == 0)) out << spacer;
    }
  }

  /// Print from smallest bit position to largest.
  void BitVector::PrintArray(std::ostream & out) const {
    for (size_t i = 0; i < num_bits; i++) out << Get(i);
  }

  /// Print the positions of all one bits, spaces are the default separator.
  void BitVector::PrintOneIDs(std::ostream & out, const std::string & spacer) const {
    for (size_t i = 0; i < num_bits; i++) { if (Get(i)) out << i << spacer; }
  }

  /// Print the ones in a range format.  E.g., 2-5,7,10-15
  void BitVector::PrintAsRange(std::ostream & out,
                    const std::string & spacer,
                    const std::string & ranger) const
  {
    emp::vector<size_t> ones = GetOnes();

    for (size_t pos = 0; pos < ones.size(); pos++) {
      if (pos) out << spacer;

      size_t start = ones[pos];
      while (pos+1 < ones.size() && ones[pos+1] == ones[pos]+1) pos++;
      size_t end = ones[pos];

      out << start;
      if (start != end) out << ranger << end;
    }
  }

    /// Perform a Boolean NOT on this BitVector and return the result.
    BitVector BitVector::NOT() const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_bv(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_bv.bits[i] = ~bits[i];
      out_bv.ClearExcessBits();
      return out_bv;
    }

    /// Perform a Boolean AND on this BitVector and return the result.
    BitVector BitVector::AND(const BitVector & bv2) const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_bv(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_bv.bits[i] = bits[i] & bv2.bits[i];
      return out_bv;
    }

    /// Perform a Boolean OR on this BitVector and return the result.
    BitVector BitVector::OR(const BitVector & bv2) const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_bv(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_bv.bits[i] = bits[i] | bv2.bits[i];
      return out_bv;
    }

    /// Perform a Boolean NAND on this BitVector and return the result.
    BitVector BitVector::NAND(const BitVector & bv2) const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_bv(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_bv.bits[i] = ~(bits[i] & bv2.bits[i]);
      out_bv.ClearExcessBits();
      return out_bv;
    }

    /// Perform a Boolean NOR on this BitVector and return the result.
    BitVector BitVector::NOR(const BitVector & bv2) const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_bv(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_bv.bits[i] = ~(bits[i] | bv2.bits[i]);
      out_bv.ClearExcessBits();
      return out_bv;
    }

    /// Perform a Boolean XOR on this BitVector and return the result.
    BitVector BitVector::XOR(const BitVector & bv2) const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_bv(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_bv.bits[i] = bits[i] ^ bv2.bits[i];
      return out_bv;
    }

    /// Perform a Boolean EQU on this BitVector and return the result.
    BitVector BitVector::EQU(const BitVector & bv2) const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_bv(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_bv.bits[i] = ~(bits[i] ^ bv2.bits[i]);
      out_bv.ClearExcessBits();
      return out_bv;
    }


    /// Perform a Boolean NOT with this BitVector, store result here, and return this object.
    BitVector & BitVector::NOT_SELF() {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = ~bits[i];
      ClearExcessBits();
      return *this;
    }

    /// Perform a Boolean AND with this BitVector, store result here, and return this object.
    BitVector & BitVector::AND_SELF(const BitVector & bv2) {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = bits[i] & bv2.bits[i];
      return *this;
    }

    /// Perform a Boolean OR with this BitVector, store result here, and return this object.
    BitVector & BitVector::OR_SELF(const BitVector & bv2) {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = bits[i] | bv2.bits[i];
      return *this;
    }

    /// Perform a Boolean NAND with this BitVector, store result here, and return this object.
    BitVector & BitVector::NAND_SELF(const BitVector & bv2) {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = ~(bits[i] & bv2.bits[i]);
      ClearExcessBits();
      return *this;
    }

    /// Perform a Boolean NOR with this BitVector, store result here, and return this object.
    BitVector & BitVector::NOR_SELF(const BitVector & bv2) {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = ~(bits[i] | bv2.bits[i]);
      ClearExcessBits();
      return *this;
    }

    /// Perform a Boolean XOR with this BitVector, store result here, and return this object.
    BitVector & BitVector::XOR_SELF(const BitVector & bv2) {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = bits[i] ^ bv2.bits[i];
      return *this;
    }

    /// Perform a Boolean EQU with this BitVector, store result here, and return this object.
    BitVector & BitVector::EQU_SELF(const BitVector & bv2) {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = ~(bits[i] ^ bv2.bits[i]);
      ClearExcessBits();
      return *this;
    }

    /// Positive shifts go left and negative go right (0 does nothing); return result.
    BitVector BitVector::SHIFT(const int shift_size) const {
      BitVector out_bv(*this);
      if (shift_size > 0) out_bv.ShiftRight((size_t) shift_size);
      else if (shift_size < 0) out_bv.ShiftLeft((size_t) -shift_size);
      return out_bv;
    }

    /// Positive shifts go left and negative go right; store result here, and return this object.
    BitVector & BitVector::SHIFT_SELF(const int shift_size) {
      if (shift_size > 0) ShiftRight((size_t) shift_size);
      else if (shift_size < 0) ShiftLeft((size_t) -shift_size);
      return *this;
    }


}


// ---------------------- Implementations to work with standard library ----------------------

namespace std {
  /// Hash function to allow BitVector to be used with maps and sets (must be in std).
  template <>
  struct hash<emp::BitVector> {
    std::size_t operator()(const emp::BitVector & b) const {
      return b.Hash();
    }
  };

  /// operator<< to work with ostream (must be in std to work)
  inline std::ostream & operator<<(std::ostream & out, const emp::BitVector & bit_v) {
    bit_v.Print(out);
    return out;
  }
}

#endif
