/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  BitArray.hpp
 *  @brief An Array of a fixed number of bits, with the leftmost bit being position zero.
 *  @note Status: RELEASE
 *
 *  @todo Some of the functions allow a start bit and end bit; each of these should be checked
 *        to make sure that they will work if the start and end are part of the same byte.  One
 *        option is to do this well ONCE with a macro that properly fills in the details.
 */


#ifndef EMP_BIT_ARRAY_HPP
#define EMP_BIT_ARRAY_HPP

#include <iostream>
#include <initializer_list>
#include <cstring>
#include <bitset>

#include "../base/assert.hpp"
#include "../base/Ptr.hpp"
#include "../base/vector.hpp"
#include "../datastructs/hash_utils.hpp"
#include "../math/math.hpp"
#include "../math/Random.hpp"
#include "../math/random_utils.hpp"
#include "../meta/type_traits.hpp"
#include "../polyfill/span.hpp"

#include "bitset_utils.hpp"
#include "_bitset_helpers.hpp"

namespace emp {

  ///  A fixed-sized (but arbitrarily large) array of bits, and optimizes operations on those bits
  ///  to be as fast as possible.
  template <size_t NUM_BITS>
  class BitArray {

    // make all templated instantiations friends with each other
    template <size_t FRIEND_BITS> friend class BitArray;

  private:
    using this_t = BitArray<NUM_BITS>;

    // Determine the size of the fields to use.  By default, size_t will be the natural size for
    // the machine; exact fits in other sizes may also allow for skipping zeroing out extra bits.
    using field_t = typename emp::uint_bit_count_t<NUM_BITS, size_t>;

    // Compile-time constants
    static constexpr size_t FIELD_BITS = 8 * sizeof(field_t);
    static constexpr size_t NUM_FIELDS = (1 + ((NUM_BITS - 1) / FIELD_BITS));
    static constexpr size_t TOTAL_BYTES = 1 + ((NUM_BITS - 1) >> 3);
    static constexpr size_t LAST_FIELD = NUM_FIELDS - 1;

    // Number of bits needed to specify position in a field + mask
    static constexpr size_t FIELD_LOG2 = emp::Log2(FIELD_BITS);
    static constexpr field_t FIELD_LOG2_MASK = MaskLow<field_t>(FIELD_LOG2);

    // Track number of bits in the final field; use 0 if a perfect fit.
    static constexpr size_t NUM_END_BITS = NUM_BITS & (FIELD_BITS - 1);

    /// How many EXTRA bits are leftover in the gap at the end?
    static constexpr size_t END_GAP = NUM_END_BITS ? (FIELD_BITS - NUM_END_BITS) : 0;

    // Mask to use to clear out any end bits that should be zeroes.
    static constexpr field_t END_MASK = MaskLow<field_t>(NUM_END_BITS);

    static constexpr field_t FIELD_0 = (field_t) 0;      ///< All bits in a field set to 0
    static constexpr field_t FIELD_1 = (field_t) 1;      ///< Least significant bit set to 1
    static constexpr field_t FIELD_255 = (field_t) 255;  ///< Least significant 8 bits set to 1
    static constexpr field_t FIELD_ALL = ~FIELD_0;       ///< All bits in a field set to 1

    field_t bit_set[NUM_FIELDS];  ///< Fields to hold the actual bits for this BitArray.

    // Identify the field that a specified bit is in.
    [[nodiscard]] static size_t FieldID(const size_t index) { return index >> FIELD_LOG2; }

    // Identify the byte that a specified bit is in.
    [[nodiscard]] static size_t ByteID(const size_t index) { return index >> 3; }

    // Identify the position within a field where a specified bit is.
    [[nodiscard]] static size_t FieldPos(const size_t index) { return index & (FIELD_BITS - 1); }

    // Identify the position within a byte where a specified bit is.
    [[nodiscard]] static size_t BytePos(const size_t index) { return index & 7; }

    // Identify which field a specified byte position would be in.
    [[nodiscard]] static size_t Byte2Field(const size_t index) { return index / sizeof(field_t); }

    // Convert a byte position in BitArray to a byte position in the target field.
    [[nodiscard]] static size_t Byte2FieldPos(const size_t index) { return FieldPos(index * 8); }

    // Copy an array of bits into this BitArray (internal use only!)
    template <size_t IN_FIELDS, size_t COPY_FIELDS=NUM_FIELDS>
    BitArray & Copy(const field_t in_set[IN_FIELDS]) {
      static_assert(COPY_FIELDS <= IN_FIELDS, "Cannot copy more fields than we are given.");
      static_assert(COPY_FIELDS <= NUM_FIELDS, "Cannot copy into more fields than are available.");
      constexpr size_t COPY_BYTES = COPY_FIELDS * sizeof(field_t);
      std::memcpy(bit_set, in_set, COPY_BYTES);
      return *this;
    }

    // Any bits past the last "real" bit in the last field should be kept as zeros.
    void ClearExcessBits() { if constexpr (NUM_END_BITS > 0) bit_set[LAST_FIELD] &= END_MASK; }

    // Convert the bit_set to const bytes.
    [[nodiscard]] emp::Ptr<const unsigned char> BytePtr() const
    { return reinterpret_cast<const unsigned char*>(bit_set); }

    // Convert the bit_set to bytes.
    [[nodiscard]] emp::Ptr<unsigned char> BytePtr()
    { return reinterpret_cast<unsigned char*>(bit_set); }

    /// Helper: call SHIFT with positive number instead
    void ShiftLeft(const size_t shift_size);

    /// Helper for calling SHIFT with negative number
    void ShiftRight(const size_t shift_size);

    /// Helper: call ROTATE with negative number instead
    void RotateLeft(const size_t shift_size_raw);

    /// Helper for calling ROTATE with positive number
    void RotateRight(const size_t shift_size_raw);

  public:
    /// Constructor: Assume all zeroes in set
    explicit BitArray(bool init_val=false) { if (init_val) SetAll(); else Clear(); }

    /// Copy constructor from another BitArray
    BitArray(const BitArray<NUM_BITS> & in_set) { Copy<NUM_FIELDS>(in_set.bit_set); }

    /// Constructor to generate a BitArray from a std::bitset.
    explicit BitArray(const std::bitset<NUM_BITS> & bitset);

    /// Constructor to generate a BitArray from a string of '0's and '1's.
    BitArray(const std::string & bitstring);

    /// Constructor to generate a BitArray from a literal string of '0's and '1's.
    BitArray(const char * bitstring) : BitArray(std::string(bitstring)) { }

    /// Constructor to generate a random BitArray (with equal prob of 0 or 1).
    BitArray(Random & random) { Clear(); Randomize(random); }

    /// Constructor to generate a random BitArray with provided PROBABILITY of 1's.
    BitArray(Random & random, double p1) { Clear(); Randomize(random, p1); }

    /// Constructor to generate a random BitArray with provided NUMBER of 1's.
    BitArray(Random & random, size_t num_ones) { Clear(); ChooseRandom(random, num_ones); }

    /// Constructor to generate a random BitArray with provided NUMBER of 1's.
    BitArray(Random & random, int num_ones) { Clear(); ChooseRandom(random, num_ones); }

    /// Constructor to fill in a bit set from a vector.
    template <typename T> BitArray(const std::initializer_list<T> l);

    /// Destructor.
    ~BitArray() = default;

    /// Assignment operator (no separate move opperator since no resources to move...)
    BitArray & operator=(const BitArray<NUM_BITS> & in_set) { return Copy<NUM_FIELDS>(in_set.bit_set); }

    /// Assignment operator from a std::bitset.
    BitArray & operator=(const std::bitset<NUM_BITS> & bitset);

    /// Assignment operator from a string of '0's and '1's.
    BitArray & operator=(const std::string & bitstring);

    /// Assignment operator from a literal string of '0's and '1's.
    BitArray & operator=(const char * bitstring) { return operator=(std::string(bitstring)); }

    /// Assignment from another BitArray of a different size.
    template <size_t FROM_BITS>
    BitArray & Import( const BitArray<FROM_BITS> & from_set, const size_t from_bit=0 );

    /// Convert to a Bitset of a different size.
    template <size_t TO_BITS>
    BitArray<TO_BITS> Export(size_t start_bit=0) const;

    /// For debugging: make sure that there are no obvous problems with a BitArray object.
    bool OK() const;

    /// How many bits are in this BitArray?
    [[nodiscard]] constexpr static size_t GetSize() { return NUM_BITS; }

    /// How many bytes are in this BitArray?
    [[nodiscard]] constexpr static size_t GetNumBytes() { return TOTAL_BYTES; }

    /// How many distinct values could be held in this bitset?
    [[nodiscard]] static constexpr double GetNumStates() { return emp::Pow2(NUM_BITS); }

    /// Retrieve the bit as a specified index.
    [[nodiscard]] bool Get(size_t index) const;

    /// A safe version of Get() for indexing out of range. Useful for representing collections.
    [[nodiscard]] bool Has(size_t index) const { return (index < NUM_BITS) ? Get(index) : false; }

    /// Set the bit at a specified index.
    BitArray & Set(size_t index, bool value=true);

    /// Set all bits to one.
    BitArray & SetAll();

    /// Set a range of bits to one: [start, stop)
    BitArray & SetRange(size_t start, size_t stop);

    /// Set all bits to zero.
    BitArray & Clear() { for (field_t & x : bit_set) x = FIELD_0; return *this; }

    /// Set specific bit to 0.
    BitArray & Clear(size_t index) { return Set(index, false); }

    /// Set bits to 0 in the range [start, stop)
    BitArray & Clear(const size_t start, const size_t stop);

    /// Index into a const BitArray (i.e., cannot be set this way.)
    bool operator[](size_t index) const { return Get(index); }

    /// Index into a BitArray, returning a proxy that will allow bit assignment to work.
    BitProxy<this_t> operator[](size_t index) { return BitProxy<this_t>(*this, index); }

    /// Flip all bits in this BitArray
    BitArray & Toggle() { return NOT_SELF(); }

    /// Flip a single bit
    BitArray & Toggle(size_t index);

    /// Flips all the bits in a range [start, stop)
    BitArray & Toggle(size_t start, size_t stop);

    /// Return true if ANY bits in the BitArray are one, else return false.
    [[nodiscard]] bool Any() const { for (auto i : bit_set) if (i) return true; return false; }

    /// Return true if NO bits in the BitArray are one, else return false.
    [[nodiscard]] bool None() const { return !Any(); }

    /// Return true if ALL bits in the BitArray are one, else return false.
    [[nodiscard]] bool All() const { return (~(*this)).None(); }

    /// Set all bits randomly, with a 50% probability of being a 0 or 1.
    BitArray &  Randomize(Random & random);

    /// Set all bits randomly, with probability specified at compile time.
    template <Random::Prob P>
    BitArray & RandomizeP(Random & random,
                        const size_t start_pos=0, const size_t stop_pos=NUM_BITS);

    /// Set all bits randomly, with a given probability of being a one.
    BitArray & Randomize(Random & random, const double p,
                       const size_t start_pos=0, const size_t stop_pos=NUM_BITS);

    /// Set all bits randomly, with a fixed number of them being ones.
    BitArray & ChooseRandom(Random & random, const size_t target_ones,
                       const size_t start_pos=0, const size_t stop_pos=NUM_BITS);
    
    /// Flip random bits with a given probability.
    BitArray & FlipRandom(Random & random, const double p,
                        const size_t start_pos=0, const size_t stop_pos=NUM_BITS);

    /// Set random bits with a given probability (does not check if already set.)
    BitArray & SetRandom(Random & random, const double p,
                        const size_t start_pos=0, const size_t stop_pos=NUM_BITS);

    /// Unset random bits with a given probability (does not check if already zero.)
    BitArray & ClearRandom(Random & random, const double p,
                        const size_t start_pos=0, const size_t stop_pos=NUM_BITS);

    /// Flip a specified number of random bits.
    /// @note: This was previously called Mutate.
    BitArray & FlipRandomCount(Random & random, const size_t num_bits);

    /// Set a specified number of random bits (does not check if already set.)
    BitArray & SetRandomCount(Random & random, const size_t num_bits);

    /// Unset  a specified number of random bits (does not check if already zero.)
    BitArray & ClearRandomCount(Random & random, const size_t num_bits);

    // >>>>>>>>>>  Comparison Operators  <<<<<<<<<< //

    template <size_t T2> [[nodiscard]] bool operator==(const BitArray<T2> & in) const;
    template <size_t T2> [[nodiscard]] bool operator!=(const BitArray<T2> & in) const { return !(*this == in); }
    template <size_t T2> [[nodiscard]] bool operator< (const BitArray<T2> & in) const;
    template <size_t T2> [[nodiscard]] bool operator> (const BitArray<T2> & in) const { return in < *this; }
    template <size_t T2> [[nodiscard]] bool operator<=(const BitArray<T2> & in) const { return !(in < *this); }
    template <size_t T2> [[nodiscard]] bool operator>=(const BitArray<T2> & in) const { return !(*this < in); }

    /// Casting a BitArray to bool identifies if ANY bits are set to 1.
    explicit operator bool() const { return Any(); }


    // >>>>>>>>>>  Access Groups of bits  <<<<<<<<<< //

    /// Retrive the byte at the specified byte index.
    [[nodiscard]] uint8_t GetByte(size_t index) const;

    /// Get a read-only view into the internal array used by BitArray.
    /// @return Read-only span of BitArray's bytes.
    [[nodiscard]] std::span<const std::byte> GetBytes() const;

    /// Get a read-only pointer to the internal array used by BitArray.
    /// @return Read-only pointer to BitArray's bytes.
    [[nodiscard]] emp::Ptr<const unsigned char> RawBytes() const { return BytePtr(); }

    /// Update the byte at the specified byte index.
    void SetByte(size_t index, uint8_t value);

    /// Get the overall value of this BitArray, using a uint encoding, but including all bits
    /// and returning the value as a double.
    [[nodiscard]] double GetValue() const;

    /// Get specified type at a given index (in steps of that type size)
    template <typename T> [[nodiscard]] T GetValueAtIndex(const size_t index) const;

    /// Retrieve a 'size_t' chunk from the current bits at the specified index.
    [[nodiscard]] std::size_t GetSizeT(size_t index) const { return GetValueAtIndex<size_t>(index); }

    /// Retrieve the 8-bit uint from the specified uint index.
    [[nodiscard]] uint8_t GetUInt8(size_t index) const { return GetValueAtIndex<uint8_t>(index); }

    /// Retrieve the 16-bit uint from the specified uint index.
    [[nodiscard]] uint16_t GetUInt16(size_t index) const { return GetValueAtIndex<uint16_t>(index); }

    /// Retrieve the 32-bit uint from the specified uint index.
    [[nodiscard]] uint32_t GetUInt32(size_t index) const { return GetValueAtIndex<uint32_t>(index); }
    
    /// Retrieve the 64-bit uint from the specified uint index.
    [[nodiscard]] uint64_t GetUInt64(size_t index) const { return GetValueAtIndex<uint64_t>(index); }

    /// By default, retrieve the 32-bit uint from the specified uint index.
    [[nodiscard]] uint32_t GetUInt(size_t index) const { return GetUInt32(index); }


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
    template <typename T> [[nodiscard]] T GetValueAtBit(const size_t index) const;

    /// Retrieve the 8-bit uint from the specified uint index.
    [[nodiscard]] uint8_t GetUInt8AtBit(size_t index) const { return GetValueAtBit<uint8_t>(index); }

    /// Retrieve the 16-bit uint from the specified uint index.
    [[nodiscard]] uint16_t GetUInt16AtBit(size_t index) const { return GetValueAtBit<uint16_t>(index); }

    /// Retrieve the 32-bit uint from the specified uint index.
    [[nodiscard]] uint32_t GetUInt32AtBit(size_t index) const { return GetValueAtBit<uint32_t>(index); }
    
    /// Retrieve the 64-bit uint from the specified uint index.
    [[nodiscard]] uint64_t GetUInt64AtBit(size_t index) const { return GetValueAtBit<uint64_t>(index); }

    /// By default, retrieve the 32-bit uint from the specified uint index.
    [[nodiscard]] uint32_t GetUIntAtBit(size_t index) const { return GetUInt32AtBit(index); }


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
    [[nodiscard]] std::size_t Hash() const;

    /// Count the number of ones in the BitArray.
    [[nodiscard]] size_t CountOnes() const;

    /// Faster counting of ones for very sparse bit vectors.
    [[nodiscard]] size_t CountOnes_Sparse() const;

    /// Count the number of zeros in the BitArray.
    [[nodiscard]] size_t CountZeros() const { return GetSize() - CountOnes(); }

    /// Return the position of the first one; return -1 if no ones in vector.
    [[nodiscard]] int FindOne() const;

    /// Deprecated: Return the position of the first one; return -1 if no ones in vector.
    [[deprecated("Renamed to more acurate FindOne()")]]
    [[nodiscard]] int FindBit() const { return FindOne(); }

    /// Return the position of the first one after start_pos; return -1 if no ones in vector.
    /// You can loop through all 1-bit positions of a BitArray "bits" with:
    ///
    ///   for (int pos = bits.FindOne(); pos >= 0; pos = bits.FindOne(pos+1)) { ... }
    ///
    [[nodiscard]] int FindOne(const size_t start_pos) const;

    /// Deprecated version of FindOne().
    [[deprecated("Renamed to more acurate FindOne(start_pos)")]]
    [[nodiscard]] int FindBit(const size_t start_pos) const;

    /// Find the most-significant set-bit.
    [[nodiscard]] int FindMaxOne() const;

    /// Return the position of the first one and change it to a zero.  Return -1 if no ones.
    int PopOne();

    /// Deprecated version of PopOne().
    [[deprecated("Renamed to more acurate PopOne()")]]
    int PopBit() { return PopOne(); }

    /// Return positions of all ones.
    [[nodiscard]] emp::vector<size_t> GetOnes() const;

    /// Find the length of the longest continuous series of ones.
    [[nodiscard]] size_t LongestSegmentOnes() const;


    // >>>>>>>>>>  Print/String Functions  <<<<<<<<<< //

    /// Convert a specified bit to a character.
    [[nodiscard]] char GetAsChar(size_t id) const { return Get(id) ? '1' : '0'; }

    /// Convert this BitArray to a string.
    [[nodiscard]] std::string ToString() const;

    /// Convert this BitArray to a numerical string [index 0 on right]
    [[nodiscard]] std::string ToBinaryString() const;

    /// Convert this BitArray to a series of IDs
    [[nodiscard]] std::string ToIDString(const std::string & spacer=" ") const;

    /// Convert this BitArray to a series of IDs with ranges condensed.
    [[nodiscard]] std::string ToRangeString(const std::string & spacer=",",
                                            const std::string & ranger="-") const;

    /// Regular print function (from least significant bit to most)
    void Print(std::ostream & out=std::cout) const { out << ToString(); }

    /// Numerical print function (from most significant bit to least)
    void PrintBinary(std::ostream & out=std::cout) const { out << ToBinaryString(); }

    /// Print from smallest bit position to largest.
    void PrintArray(std::ostream & out=std::cout) const { out << ToString(); }

    /// Print a space between each field (or other provided spacer)
    void PrintFields(std::ostream & out=std::cout, const std::string & spacer=" ") const;

    /// Print out details about the internals of the BitArray.
    void PrintDebug(std::ostream & out=std::cout) const;

    /// Print the locations of all one bits, using the provided spacer (default is a single space)
    void PrintOneIDs(std::ostream & out=std::cout, const std::string & spacer=" ") const;

    /// Print the ones in a range format.  E.g., 2-5,7,10-15
    void PrintAsRange(std::ostream & out=std::cout,
                      const std::string & spacer=",",
                      const std::string & ranger="-") const;


    /// Overload ostream operator to return Print.
    friend std::ostream& operator<<(std::ostream &out, const BitArray& bs) {
      bs.Print(out);
      return out;
    }

    /// Perform a Boolean NOT on this BitArray, store result here, and return this object.
    BitArray & NOT_SELF();

    /// Perform a Boolean AND with a second BitArray, store result here, and return this object.
    BitArray & AND_SELF(const BitArray & set2);

    /// Perform a Boolean OR with a second BitArray, store result here, and return this object.
    BitArray & OR_SELF(const BitArray & set2);

    /// Perform a Boolean NAND with a second BitArray, store result here, and return this object.
    BitArray & NAND_SELF(const BitArray & set2);

    /// Perform a Boolean NOR with a second BitArray, store result here, and return this object.
    BitArray & NOR_SELF(const BitArray & set2);

    /// Perform a Boolean XOR with a second BitArray, store result here, and return this object.
    BitArray & XOR_SELF(const BitArray & set2);

    /// Perform a Boolean EQU with a second BitArray, store result here, and return this object.
    BitArray & EQU_SELF(const BitArray & set2);

    /// Perform a Boolean NOT on this BitArray and return the result.
    [[nodiscard]] BitArray NOT() const { return BitArray<NUM_BITS>(*this).NOT_SELF(); }

    /// Perform a Boolean AND with a second BitArray and return the result.
    [[nodiscard]] BitArray AND(const BitArray & in) const { return BitArray<NUM_BITS>(*this).AND_SELF(in); }

    /// Perform a Boolean OR with a second BitArray and return the result.
    [[nodiscard]] BitArray OR(const BitArray & in) const { return BitArray<NUM_BITS>(*this).OR_SELF(in); }

    /// Perform a Boolean NAND with a second BitArray and return the result.
    [[nodiscard]] BitArray NAND(const BitArray & in) const { return BitArray<NUM_BITS>(*this).NAND_SELF(in); }

    /// Perform a Boolean NOR with a second BitArray and return the result.
    [[nodiscard]] BitArray NOR(const BitArray & in) const { return BitArray<NUM_BITS>(*this).NOR_SELF(in); }

    /// Perform a Boolean XOR with a second BitArray and return the result.
    [[nodiscard]] BitArray XOR(const BitArray & in) const { return BitArray<NUM_BITS>(*this).XOR_SELF(in); }

    /// Perform a Boolean EQU with a second BitArray and return the result.
    BitArray EQU(const BitArray & in) const { return BitArray<NUM_BITS>(*this).EQU_SELF(in); }

    /// Positive shifts go right and negative shifts go left (0 does nothing);
    /// return result.
    [[nodiscard]] BitArray SHIFT(const int shift_size) const;

    /// Positive shifts go right and negative shifts go left (0 does nothing);
    /// store result here, and return this object.
    BitArray & SHIFT_SELF(const int shift_size);

    /// Reverse the order of bits in the bitset
    BitArray & REVERSE_SELF();

    /// Reverse order of bits in the bitset.
    [[nodiscard]] BitArray REVERSE() const;

    /// Positive rotates go left and negative rotates go left (0 does nothing);
    /// return result.
    [[nodiscard]] BitArray ROTATE(const int rotate_size) const;

    /// Positive rotates go right and negative rotates go left (0 does nothing);
    /// store result here, and return this object.
    BitArray & ROTATE_SELF(const int rotate_size);

    /// Helper: call ROTATE with negative number instead
    template<size_t shift_size_raw>
    BitArray & ROTL_SELF();

    /// Helper for calling ROTATE with positive number
    template<size_t shift_size_raw>
    BitArray & ROTR_SELF();

    /// Addition of two Bitsets.
    /// Wraps if it overflows.
    /// Returns result.
    [[nodiscard]] BitArray ADD(const BitArray & set2) const;

    /// Addition of two Bitsets.
    /// Wraps if it overflows.
    /// Returns this object.
    BitArray & ADD_SELF(const BitArray & set2);

    /// Subtraction of two Bitsets.
    /// Wraps around if it underflows.
    /// Returns result.
    [[nodiscard]] BitArray SUB(const BitArray & set2) const;

    /// Subtraction of two Bitsets.
    /// Wraps if it underflows.
    /// Returns this object.
    BitArray & SUB_SELF(const BitArray & set2);
    
    /// Operator bitwise NOT...
    [[nodiscard]] BitArray operator~() const { return NOT(); }

    /// Operator bitwise AND...
    [[nodiscard]] BitArray operator&(const BitArray & ar2) const { return AND(ar2); }

    /// Operator bitwise OR...
    [[nodiscard]] BitArray operator|(const BitArray & ar2) const { return OR(ar2); }

    /// Operator bitwise XOR...
    [[nodiscard]] BitArray operator^(const BitArray & ar2) const { return XOR(ar2); }

    /// Operator shift left...
    [[nodiscard]] BitArray operator<<(const size_t shift_size) const { return SHIFT(-(int)shift_size); }

    /// Operator shift right...
    [[nodiscard]] BitArray operator>>(const size_t shift_size) const { return SHIFT((int)shift_size); }

    /// Compound operator bitwise AND...
    BitArray & operator&=(const BitArray & ar2) { return AND_SELF(ar2); }

    /// Compound operator bitwise OR...
    BitArray & operator|=(const BitArray & ar2) { return OR_SELF(ar2); }

    /// Compound operator bitwise XOR...
    BitArray & operator^=(const BitArray & ar2) { return XOR_SELF(ar2); }

    /// Compound operator shift left...
    BitArray & operator<<=(const size_t shift_size) { return SHIFT_SELF(-(int)shift_size); }

    /// Compound operator shift right...
    BitArray & operator>>=(const size_t shift_size) { return SHIFT_SELF((int)shift_size); }

    /// Operator plus...
    [[nodiscard]] BitArray operator+(const BitArray & ar2) const { return ADD(ar2); }

    /// Operator minus...
    [[nodiscard]] BitArray operator-(const BitArray & ar2) const { return SUB(ar2); }

    /// Compound operator plus...
    const BitArray & operator+=(const BitArray & ar2) { return ADD_SELF(ar2); }

    /// Compoount operator minus...
    const BitArray & operator-=(const BitArray & ar2) { return SUB_SELF(ar2); }

    /// STL COMPATABILITY
    /// A set of functions to allow drop-in replacement with std::bitset.
    [[nodiscard]] constexpr static size_t size() { return NUM_BITS; }
    [[nodiscard]] inline bool all() const { return All(); }
    [[nodiscard]] inline bool any() const { return Any(); }
    [[nodiscard]] inline bool none() const { return !Any(); }
    [[nodiscard]] inline size_t count() const { return CountOnes(); }
    inline BitArray & flip() { return Toggle(); }
    inline BitArray & flip(size_t pos) { return Toggle(pos); }
    inline BitArray & flip(size_t start, size_t stop) { return Toggle(start, stop); }
    inline void reset() { Clear(); }
    inline void reset(size_t id) { Set(id, false); }
    inline void set() { SetAll(); }
    inline void set(size_t id) { Set(id); }
    [[nodiscard]] inline bool test(size_t index) const { return Get(index); }

    template <class Archive>
    void serialize( Archive & ar )
    {
      ar( bit_set );
    }

  };

  // ------------------------ Implementations for Internal Functions ------------------------

  template <size_t NUM_BITS>
  void BitArray<NUM_BITS>::ShiftLeft(const size_t shift_size) {
    // If we have only a single field, this operation can be quick.
    if constexpr (NUM_FIELDS == 1) {
      bit_set[0] <<= shift_size;
      ClearExcessBits();
      return;
    }

    // If we are shifting out of range, clear the bits and stop.
    if (shift_size >= NUM_BITS) { Clear(); return; }

    const size_t field_shift = shift_size / FIELD_BITS;
    const size_t bit_shift = shift_size % FIELD_BITS;
    const size_t bit_overflow = FIELD_BITS - bit_shift;

    // Loop through each field, from L to R, and update it.
    if (field_shift) {
      for (size_t i = LAST_FIELD; i >= field_shift; --i) {
        bit_set[i] = bit_set[i - field_shift];
      }
      for (size_t i = field_shift; i > 0; i--) bit_set[i-1] = 0;
    }

    // account for bit_shift
    if (bit_shift) {
      for (size_t i = LAST_FIELD; i > field_shift; --i) {
        bit_set[i] <<= bit_shift;
        bit_set[i] |= (bit_set[i-1] >> bit_overflow);
      }
      // Handle final field (field_shift position)
      bit_set[field_shift] <<= bit_shift;
    }

    // Mask out any bits that have left-shifted away
    ClearExcessBits();
  }


  /// Helper for calling SHIFT with negative number
  template <size_t NUM_BITS>
  void BitArray<NUM_BITS>::ShiftRight(const size_t shift_size) {
    // If we have only a single field, this operation can be quick.
    if constexpr (NUM_FIELDS == 1) {
      bit_set[0] >>= shift_size;
      return;
    }

    if (!shift_size) return;

    const field_t field_shift = shift_size / FIELD_BITS;

    // Only clear and return if we are field_shift-ing
    // We want to be able to always shift by up to a byte so that Import and Export work
    if (field_shift && shift_size > NUM_BITS) {
      Clear();
      return;
    }
    const field_t bit_shift = shift_size % FIELD_BITS;
    const field_t bit_overflow = FIELD_BITS - bit_shift;

    // account for field_shift
    if (field_shift) {
      for (size_t i = 0; i < (NUM_FIELDS - field_shift); ++i) {
        bit_set[i] = bit_set[i + field_shift];
      }
      for (size_t i = NUM_FIELDS - field_shift; i < NUM_FIELDS; i++) bit_set[i] = 0;
    }

    // account for bit_shift
    if (bit_shift) {
      for (size_t i = 0; i < (LAST_FIELD - field_shift); ++i) {
        bit_set[i] >>= bit_shift;
        bit_set[i] |= (bit_set[i+1] << bit_overflow);
      }
      bit_set[LAST_FIELD - field_shift] >>= bit_shift;
    }
  }

  /// Helper: call ROTATE with negative number
  template <size_t NUM_BITS>
  void BitArray<NUM_BITS>::RotateLeft(const size_t shift_size_raw) {
    const field_t shift_size = shift_size_raw % NUM_BITS;

    // use different approaches based on BitArray size
    if constexpr (NUM_FIELDS == 1) {
      // special case: for exactly one field_T, try to go low level
      // adapted from https://stackoverflow.com/questions/776508/best-practices-for-circular-shift-rotate-operations-in-c
      field_t & n = bit_set[0];
      size_t c = shift_size;

      // mask necessary to suprress shift count overflow warnings
      c &= FIELD_LOG2_MASK;
      n = (n<<c) | (n>>( (-(c+FIELD_BITS-NUM_BITS)) & FIELD_LOG2_MASK ));

    } else if constexpr (NUM_FIELDS < 32) {
      // for small BitArrays, shifting L/R and ORing is faster
      emp::BitArray<NUM_BITS> dup(*this);
      dup.ShiftLeft(shift_size);
      ShiftRight(NUM_BITS - shift_size);
      OR_SELF(dup);
    } else {
      // for big BitArrays, manual rotating is fater

      // note that we already modded shift_size by NUM_BITS
      // so there's no need to mod by FIELD_SIZE here
      const int field_shift = NUM_END_BITS ? (
        (shift_size + FIELD_BITS - NUM_END_BITS) / FIELD_BITS
      ) : (
        shift_size / FIELD_BITS
      );
      // if we field shift, we need to shift bits by (FIELD_BITS - NUM_END_BITS)
      // more to account for the filler that gets pulled out of the middle
      const int bit_shift = NUM_END_BITS && field_shift ? (
        (shift_size + FIELD_BITS - NUM_END_BITS) % FIELD_BITS
      ) : (
        shift_size % FIELD_BITS
      );
      const int bit_overflow = FIELD_BITS - bit_shift;

      // if rotating more than field capacity, we need to rotate fields
      std::rotate(
        std::rbegin(bit_set),
        std::rbegin(bit_set)+field_shift,
        std::rend(bit_set)
      );

      // if necessary, shift filler bits out of the middle
      if constexpr ((bool)NUM_END_BITS) {
        const int filler_idx = (LAST_FIELD + field_shift) % NUM_FIELDS;
        for (int i = filler_idx + 1; i < (int)NUM_FIELDS; ++i) {
          bit_set[i-1] |= bit_set[i] << NUM_END_BITS;
          bit_set[i] >>= (FIELD_BITS - NUM_END_BITS);
        }
      }

      // account for bit_shift
      if (bit_shift) {

        const field_t keystone = NUM_END_BITS ? (
          (bit_set[LAST_FIELD] << (FIELD_BITS - NUM_END_BITS))
          | (bit_set[NUM_FIELDS - 2] >> NUM_END_BITS)
        ) : (
          bit_set[LAST_FIELD]
        );

        for (int i = LAST_FIELD; i > 0; --i) {
          bit_set[i] <<= bit_shift;
          bit_set[i] |= (bit_set[i-1] >> bit_overflow);
        }
        // Handle final field
        bit_set[0] <<= bit_shift;
        bit_set[0] |= keystone >> bit_overflow;

      }

    }

    // Mask out filler bits
    ClearExcessBits();
  }


  /// Helper for calling ROTATE with positive number
  template <size_t NUM_BITS>
  void BitArray<NUM_BITS>::RotateRight(const size_t shift_size_raw) {

    const size_t shift_size = shift_size_raw % NUM_BITS;

    // use different approaches based on BitArray size
    if constexpr (NUM_FIELDS == 1) {
      // special case: for exactly one field_t, try to go low level
      // adapted from https://stackoverflow.com/questions/776508/best-practices-for-circular-shift-rotate-operations-in-c

      field_t & n = bit_set[0];
      size_t c = shift_size;

      // mask necessary to suprress shift count overflow warnings
      c &= FIELD_LOG2_MASK;
      n = (n>>c) | (n<<( (NUM_BITS-c) & FIELD_LOG2_MASK ));

    } else if constexpr (NUM_FIELDS < 32) {
      // for small BitArrays, shifting L/R and ORing is faster
      emp::BitArray<NUM_BITS> dup(*this);
      dup.ShiftRight(shift_size);
      ShiftLeft(NUM_BITS - shift_size);
      OR_SELF(dup);
    } else {
      // for big BitArrays, manual rotating is fater

      const field_t field_shift = (shift_size / FIELD_BITS) % NUM_FIELDS;
      const int bit_shift = shift_size % FIELD_BITS;
      const field_t bit_overflow = FIELD_BITS - bit_shift;

      // if rotating more than field capacity, we need to rotate fields
      std::rotate(
        std::begin(bit_set),
        std::begin(bit_set)+field_shift,
        std::end(bit_set)
      );

      // if necessary, shift filler bits out of the middle
      if constexpr (NUM_END_BITS > 0) {
        const int filler_idx = LAST_FIELD - field_shift;
        for (int i = filler_idx + 1; i < (int)NUM_FIELDS; ++i) {
          bit_set[i-1] |= bit_set[i] << NUM_END_BITS;
          bit_set[i] >>= (FIELD_BITS - NUM_END_BITS);
        }
      }

      // account for bit_shift
      if (bit_shift) {

        const field_t keystone = NUM_END_BITS ? (
          bit_set[0] >> (FIELD_BITS - NUM_END_BITS)
        ) : (
          bit_set[0]
        );

        if constexpr (NUM_END_BITS > 0) {
          bit_set[NUM_FIELDS-1] |= bit_set[0] << NUM_END_BITS;
        }

        for (size_t i = 0; i < LAST_FIELD; ++i) {
          bit_set[i] >>= bit_shift;
          bit_set[i] |= (bit_set[i+1] << bit_overflow);
        }
        bit_set[LAST_FIELD] >>= bit_shift;
        bit_set[LAST_FIELD] |= keystone << bit_overflow;
      }
    }

    // Mask out filler bits
    ClearExcessBits();
  }

  // -------------------- Longer Constructors and bit copying ---------------------

  /// Constructor to generate a BitArray from a std::bitset.
  template <size_t NUM_BITS>
  BitArray<NUM_BITS>::BitArray(const std::bitset<NUM_BITS> & bitset) {
    for (size_t bit{}; bit < NUM_BITS; ++bit) Set( bit, bitset[bit] );
    ClearExcessBits();
  }

  /// Constructor to generate a BitArray from a string of '0's and '1's.
  template <size_t NUM_BITS>
  BitArray<NUM_BITS>::BitArray(const std::string & bitstring)
  {
    emp_assert(bitstring.size() <= NUM_BITS);
    Clear();
    for (size_t i = 0; i < bitstring.size(); i++) Set(i, bitstring[i] != '0');
  }

  template <size_t NUM_BITS>
  template <typename T>
  BitArray<NUM_BITS>::BitArray(const std::initializer_list<T> l) {
    emp_assert(l.size() <= NUM_BITS, "Initializer longer than BitArray", l.size(), NUM_BITS);
    Clear();
    auto it = std::begin(l); // Right-most bit is position 0.
    for (size_t idx = 0; idx < NUM_BITS; ++idx) Set(idx, (idx < l.size()) && *it++);
  }

    /// Assignment operator from a std::bitset.
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::operator=(const std::bitset<NUM_BITS> & bitset) {
    for (size_t i = 0; i < NUM_BITS; i++) Set(i, bitset[i]);
    return *this;
  }

  /// Assignment operator from a string of '0's and '1's.
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::operator=(const std::string & bitstring) {
    emp_assert(bitstring.size() <= NUM_BITS);
    Clear();
    for (size_t i = 0; i < bitstring.size(); i++) Set(i, bitstring[i] != '0');
    return *this;
  }


  /// Assign from a BitArray of a different size.
  template <size_t NUM_BITS>
  template <size_t FROM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::Import(
    const BitArray<FROM_BITS> & from_set,
    const size_t from_bit
  ) {
    // Only check for same-ness if the two types are the same.
    if constexpr (FROM_BITS == NUM_BITS) emp_assert(&from_set != this);

    emp_assert(from_bit < FROM_BITS);

    if (FROM_BITS - from_bit < NUM_BITS) Clear();

    constexpr size_t DEST_BYTES = (NUM_BITS + 7)/8;
    const size_t FROM_BYTES = (FROM_BITS + 7)/8 - from_bit/8;

    const size_t COPY_BYTES = std::min(DEST_BYTES, FROM_BYTES);

    std::memcpy(
      bit_set,
      reinterpret_cast<const unsigned char*>(from_set.bit_set) + from_bit/8,
      COPY_BYTES
    );

    if (from_bit%8) {

      this->ShiftRight(from_bit%8);

      if (FROM_BYTES > COPY_BYTES) {
        reinterpret_cast<unsigned char*>(bit_set)[COPY_BYTES-1] |= (
          reinterpret_cast<const unsigned char*>(
            from_set.bit_set
          )[from_bit/8 + COPY_BYTES]
          << (8 - from_bit%8)
        );
      }

    }

    ClearExcessBits();

    return *this;

  }

  /// Convert to a Bitset of a different size.
  template <size_t NUM_BITS>
  template <size_t TO_BITS>
  BitArray<TO_BITS> BitArray<NUM_BITS>::Export(size_t start_bit) const {

    BitArray<TO_BITS> out_bits;
    out_bits.Import(*this, start_bit);

    return out_bits;
  }

    /// For debugging: make sure that there are no obvous problems with a BitArray object.
  template <size_t NUM_BITS>
  bool BitArray<NUM_BITS>::OK() const {
    // Make sure final bits are zeroed out.
    emp_assert((bit_set[LAST_FIELD] & ~END_MASK) == 0);

    return true;
  }



  // --------------------  Implementations of common accessors -------------------

  template <size_t NUM_BITS>
  bool BitArray<NUM_BITS>::Get(size_t index) const {
    emp_assert(index >= 0 && index < NUM_BITS);
    const size_t field_id = FieldID(index);
    const size_t pos_id = FieldPos(index);
    return (bit_set[field_id] & (((field_t)1U) << pos_id)) != 0;
  }

  /// Set the bit at a specified index.
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::Set(size_t index, bool value) {
    emp_assert(index < NUM_BITS);
    const size_t field_id = FieldID(index);
    const size_t pos_id = FieldPos(index);
    const field_t pos_mask = FIELD_1 << pos_id;

    if (value) bit_set[field_id] |= pos_mask;
    else       bit_set[field_id] &= ~pos_mask;

    return *this;
  }

  /// Set all bits to one.
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::SetAll() {
    for (field_t & x : bit_set) x = FIELD_ALL;
    ClearExcessBits();
    return *this;
  }

  /// Set a range of bits to one: [start, stop)
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::SetRange(size_t start, size_t stop) {
    emp_assert(start <= stop, start, stop);
    emp_assert(stop <= NUM_BITS, stop, NUM_BITS);
    const size_t start_pos = FieldPos(start);
    const size_t stop_pos = FieldPos(stop);
    size_t start_field = FieldID(start);
    const size_t stop_field = FieldID(stop);

    // If the start field and stop field are the same, just set those bits.
    if (start_field == stop_field) {
      const size_t bit_count = stop - start;
      const field_t mask = MaskLow<field_t>(bit_count) << start_pos;
      bit_set[start_field] |= mask;
    }

    // Otherwise handle the ends and clear the chunks in between.
    else {
      // Set portions of start field
      if (start_pos != 0) {
        const size_t start_bits = FIELD_BITS - start_pos;
        const field_t start_mask = MaskLow<field_t>(start_bits) << start_pos;
        bit_set[start_field] |= start_mask;
        start_field++;
      }

      // Middle fields
      for (size_t cur_field = start_field; cur_field < stop_field; cur_field++) {
        bit_set[cur_field] = FIELD_ALL;
      }

      // Set portions of stop field
      const field_t stop_mask = MaskLow<field_t>(stop_pos);
      bit_set[stop_field] |= stop_mask;
    }

    return *this;
  }

  /// Set a range of bits to 0 in the range [start, stop)
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::Clear(const size_t start, const size_t stop) {
    emp_assert(start <= stop, start, stop);
    emp_assert(stop <= NUM_BITS, stop, NUM_BITS);
    const size_t start_pos = FieldPos(start);
    const size_t stop_pos = FieldPos(stop);
    size_t start_field = FieldID(start);
    const size_t stop_field = FieldID(stop);

    // If the start field and stop field are the same, just step through the bits.
    if (start_field == stop_field) {
      const size_t num_bits = stop - start;
      const field_t mask = ~(MaskLow<field_t>(num_bits) << start_pos);
      bit_set[start_field] &= mask;
    }

    // Otherwise handle the ends and clear the chunks in between.
    else {
      // Clear portions of start field
      if (start_pos != 0) {
        const size_t start_bits = FIELD_BITS - start_pos;
        const field_t start_mask = ~(MaskLow<field_t>(start_bits) << start_pos);
        bit_set[start_field] &= start_mask;
        start_field++;
      }

      // Middle fields
      for (size_t cur_field = start_field; cur_field < stop_field; cur_field++) {
        bit_set[cur_field] = 0;
      }

      // Clear portions of stop field
      const field_t stop_mask = ~MaskLow<field_t>(stop_pos);
      bit_set[stop_field] &= stop_mask;
    }

    return *this;
  }

  /// Flip a single bit
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::Toggle(size_t index) {
    emp_assert(index >= 0 && index < NUM_BITS);
    const size_t field_id = FieldID(index);
    const size_t pos_id = FieldPos(index);
    const field_t pos_mask = FIELD_1 << pos_id;

    bit_set[field_id] ^= pos_mask;

    return *this;
  }

  /// Flips all the bits in a range [start, stop)
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::Toggle(size_t start, size_t stop) {
    emp_assert(start <= stop, start, stop);
    emp_assert(stop <= NUM_BITS, stop, NUM_BITS);
    const size_t start_pos = FieldPos(start);
    const size_t stop_pos = FieldPos(stop);
    size_t start_field = FieldID(start);
    const size_t stop_field = FieldID(stop);

    // If the start field and stop field are the same, just step through the bits.
    if (start_field == stop_field) {
      const size_t num_flips = stop - start;
      const field_t mask = MaskLow<field_t>(num_flips) << start_pos;
      bit_set[start_field] ^= mask;
    }

    // Otherwise handle the ends and clear the chunks in between.
    else {
      // Toggle correct portions of start field
      if (start_pos != 0) {
        const size_t start_bits = FIELD_BITS - start_pos;
        const field_t start_mask = MaskLow<field_t>(start_bits) << start_pos;
        bit_set[start_field] ^= start_mask;
        start_field++;
      }

      // Middle fields
      for (size_t cur_field = start_field; cur_field < stop_field; cur_field++) {
        bit_set[cur_field] = ~bit_set[cur_field];
      }

      // Set portions of stop field
      const field_t stop_mask = MaskLow<field_t>(stop_pos);
      bit_set[stop_field] ^= stop_mask;
    }

    return *this;
  }



  // -------------------------  Implementations Randomization functions -------------------------

  /// Set all bits randomly, with a 50% probability of being a 0 or 1.
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::Randomize(Random & random) {
    random.RandFill(BytePtr(), TOTAL_BYTES);
    ClearExcessBits();
    return *this;
  }

  /// Set all bits randomly, with probability specified at compile time.
  template <size_t NUM_BITS>
  template <Random::Prob P>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::RandomizeP(Random & random,
                                                  const size_t start_pos, const size_t stop_pos) {
    emp_assert(start_pos <= stop_pos);
    emp_assert(stop_pos <= NUM_BITS);
    random.RandFillP<P>(BytePtr(), TOTAL_BYTES, start_pos, stop_pos);
    ClearExcessBits();
    return *this;
  }


  /// Set all bits randomly, with a given probability of being on.
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::Randomize(Random & random, const double p,
                                                 const size_t start_pos, const size_t stop_pos) {
    emp_assert(start_pos <= stop_pos);
    emp_assert(stop_pos <= NUM_BITS);
    emp_assert(p >= 0.0 && p <= 1.0, p);
    random.RandFill(BytePtr(), TOTAL_BYTES, p, start_pos, stop_pos);
    ClearExcessBits();
     return *this;
  }

  /// Set all bits randomly, with a given number of them being on.
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> &
  BitArray<NUM_BITS>::ChooseRandom(Random & random, const size_t target_ones,
                                   const size_t start_pos, const size_t stop_pos) {
    emp_assert(start_pos <= stop_pos);
    emp_assert(stop_pos <= NUM_BITS);

    const size_t target_size = stop_pos - start_pos;
    emp_assert(target_ones <= target_size);

    // Approximate the probability of ones as a starting point.
    double p = ((double) target_ones) / (double) target_size;

    // If we are not randomizing the whole sequence, we need to track the number of ones
    // in the NON-randomized region to subtract off later.
    size_t kept_ones = 0;
    if (target_size != NUM_BITS) {
      Clear(start_pos, stop_pos);
      kept_ones = CountOnes();
    }

    // Try to find a shortcut if p allows....
    // (These values are currently educated guesses)
    if (p < 0.12) { if (target_size == NUM_BITS) Clear(start_pos, stop_pos); }
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
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::FlipRandom(Random & random,
                                                  const double p,
                                                  const size_t start_pos,
                                                  const size_t stop_pos)
  {
    emp_assert(start_pos <= stop_pos);
    emp_assert(stop_pos <= NUM_BITS);
    emp_assert(p >= 0.0 && p <= 1.0, p);

    for (size_t i=start_pos; i < stop_pos; ++i) if (random.P(p)) Toggle(i);

    return *this;
  }

  /// Set random bits with a given probability (does not check if already set.)
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::SetRandom(Random & random,
                      const double p,
                      const size_t start_pos,
                      const size_t stop_pos)
  {
    emp_assert(start_pos <= stop_pos);
    emp_assert(stop_pos <= NUM_BITS);
    emp_assert(p >= 0.0 && p <= 1.0, p);

    for (size_t i=start_pos; i < stop_pos; ++i) if (random.P(p)) Set(i);

    return *this;
  }

  /// Unset random bits with a given probability (does not check if already zero.)
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::ClearRandom(Random & random,
                      const double p,
                      const size_t start_pos,
                      const size_t stop_pos)
  {
    emp_assert(start_pos <= stop_pos);
    emp_assert(stop_pos <= NUM_BITS);
    emp_assert(p >= 0.0 && p <= 1.0, p);

    for (size_t i=start_pos; i < stop_pos; ++i) if (random.P(p)) Clear(i);

    return *this;
  }

  /// Flip a specified number of random bits.
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::FlipRandomCount(Random & random,
                                                       const size_t num_bits)
  {
    emp_assert(num_bits <= NUM_BITS);
    this_t target_bits(random, num_bits);
    return *this ^= target_bits;
  }

  /// Set a specified number of random bits (does not check if already set.)
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::SetRandomCount(Random & random,
                                                      const size_t num_bits)
  {
    emp_assert(num_bits <= NUM_BITS);
    this_t target_bits(random, num_bits);
    return *this |= target_bits;
  }

  /// Unset  a specified number of random bits (does not check if already zero.)
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::ClearRandomCount(Random & random,
                                                        const size_t num_bits)
  {
    emp_assert(num_bits <= NUM_BITS);
    this_t target_bits(random, NUM_BITS - num_bits);
    return *this &= target_bits;
  }


  // -------------------------  Implementations of Comparison Operators -------------------------

  /// Test if two BitArray objects are identical.
  template <size_t NUM_BITS>
  template <size_t SIZE2>
  bool BitArray<NUM_BITS>::operator==(const BitArray<SIZE2> & in_set) const {
    if constexpr (NUM_BITS != SIZE2) return false;

    for (size_t i = 0; i < NUM_FIELDS; ++i) {
      if (bit_set[i] != in_set.bit_set[i]) return false;
    }
    return true;
  }

  /// Compare two BitArray objects, based on the associated binary value.
  template <size_t NUM_BITS>
  template <size_t SIZE2>
  bool BitArray<NUM_BITS>::operator<(const BitArray<SIZE2> & in_set) const {
    if constexpr (NUM_BITS != SIZE2) return NUM_BITS < SIZE2;

    for (int i = NUM_FIELDS-1; i >= 0; --i) {         // Start loop at the largest field.
      if (bit_set[i] == in_set.bit_set[i]) continue;  // If same, keep looking!
      return (bit_set[i] < in_set.bit_set[i]);        // Otherwise, do comparison
    }
    return false;
  }


  // -------------------------  Access Groups of bits -------------------------

  /// Get the full byte starting from the bit at a specified index.
  template <size_t NUM_BITS>
  uint8_t BitArray<NUM_BITS>::GetByte(size_t index) const {
    emp_assert(index < TOTAL_BYTES);
    const size_t field_id = Byte2Field(index);
    const size_t pos_id = Byte2FieldPos(index);
    return (bit_set[field_id] >> pos_id) & 255;
  }


  /// Get a read-only view into the internal array used by BitArray.
  /// @return Read-only span of BitArray's bytes.
  template <size_t NUM_BITS>
  std::span<const std::byte> BitArray<NUM_BITS>::GetBytes() const {
    return std::span<const std::byte>(
      reinterpret_cast<const std::byte*>(bit_set),
      TOTAL_BYTES
    );
  }


  /// Set the full byte starting at the bit at the specified index.
  template <size_t NUM_BITS>
  void BitArray<NUM_BITS>::SetByte(size_t index, uint8_t value) {
    emp_assert(index < TOTAL_BYTES);
    const size_t field_id = Byte2Field(index);
    const size_t pos_id = Byte2FieldPos(index);
    const field_t val_uint = value;
    bit_set[field_id] = (bit_set[field_id] & ~(((field_t)255U) << pos_id)) | (val_uint << pos_id);
  }

  /// Get the overall value of this BitArray, using a uint encoding, but including all bits
  /// and returning the value as a double.
  template <size_t NUM_BITS>
  double BitArray<NUM_BITS>::GetValue() const {
    // If we have 64 bits or fewer, we can load the full value and return it.
    if constexpr (NUM_FIELDS == 1) return (double) bit_set[0];

    // Otherwise grab the most significant one and figure out how much to shift it by.
    const size_t max_one = FindMaxOne();

    // If there are no ones, this value must be 0.
    if (max_one == -1) return 0.0;

    // If all ones are in the least-significant field, just return it.
    // NOTE: If we have more than one field, FIELD_SIZE is usually 64 already.
    if (max_one < 64) return (double) GetUInt64(0);

    // To grab the most significant field, figure out how much to shift it by.
    const size_t shift_bits = max_one - 63;
    double out_value = (double) (*this >> shift_bits).GetUInt64(0);

    out_value *= emp::Pow2(shift_bits);

    return out_value;
  }

  /// Get specified type at a given index (in steps of that type size)
  template <size_t NUM_BITS>
  template <typename T>
  T BitArray<NUM_BITS>::GetValueAtIndex(const size_t index) const {
    // For the moment, must fit inside bounds; eventually should pad with zeros.
    emp_assert((index + 1) * sizeof(T) <= NUM_FIELDS * sizeof(field_t),
              index, sizeof(T), NUM_BITS, NUM_FIELDS);

    // If we are using the native field type, just grab it from bit_set.
    if constexpr( std::is_same<T, field_t>() ) return bit_set[index];

    T out_value;
    std::memcpy( &out_value, BytePtr() + index * sizeof(T), sizeof(T) );
    return out_value;
  }


  /// Set specified type at a given index (in steps of that type size)
  template <size_t NUM_BITS>
  template <typename T>
  void BitArray<NUM_BITS>::SetValueAtIndex(const size_t index, T in_value) {
    // For the moment, must fit inside bounds; eventually should pad with zeros.
    emp_assert((index + 1) * sizeof(T) <= NUM_FIELDS * sizeof(field_t),
              index, sizeof(T), NUM_BITS, NUM_FIELDS);

    std::memcpy( BytePtr() + index * sizeof(T), &in_value, sizeof(T) );

    ClearExcessBits();
  }


  /// Get the specified type starting from a given BIT position.
  template <size_t NUM_BITS>
  template <typename T>
  T BitArray<NUM_BITS>::GetValueAtBit(const size_t index) const {
    // For the moment, must fit inside bounds; eventually should pad with zeros.
    emp_assert((index+7)/8 + sizeof(T) < NUM_FIELDS * sizeof(field_t));

    BitArray<sizeof(T)*8> out_bits;
    out_bits.Import(*this, index);

    return out_bits.template GetValueAtIndex<T>(0);
  }


  /// Set the specified type starting from a given BIT position.
  // @CAO: Can be optimized substantially, especially for long BitArrays.
  template <size_t NUM_BITS>
  template <typename T>
  void BitArray<NUM_BITS>::SetValueAtBit(const size_t index, T value) {
    // For the moment, must fit inside bounds; eventually should pad with zeros.
    emp_assert((index+7)/8 + sizeof(T) < NUM_FIELDS * sizeof(field_t));
    constexpr size_t type_bits = sizeof(T) * 8;

    Clear(index, index+type_bits);       // Clear out the bits where new value will go.
    BitArray<NUM_BITS> in_bits;            // Setup a bitset to place the new bits in.
    in_bits.SetValueAtIndex(0, value);   // Insert the new bits.
    in_bits <<= index;                    // Shift new bits into place.
    OR_SELF(in_bits);                    // Place new bits into current BitArray.

    ClearExcessBits();
  }


  // -------------------------  Other Analyses -------------------------

  /// A simple hash function for bit vectors.
  template <size_t NUM_BITS>
  std::size_t BitArray<NUM_BITS>::Hash() const {
    /// If we have a vector of size_t, treat it as a vector of hash values to combine.
    if constexpr (std::is_same_v<field_t, size_t>) {
      return hash_combine(bit_set, NUM_FIELDS);
    }

    constexpr size_t SIZE_T_BITS = sizeof(std::size_t)*8;

    // If all of the bits will fit into a single size_t, return it.
    if constexpr (NUM_BITS <= SIZE_T_BITS) return GetSizeT(0);

    // If the bits fit into TWO size_t units, merge them.
    if constexpr (NUM_BITS <= 2 * SIZE_T_BITS) {
      return emp::hash_combine(GetSizeT(0), GetSizeT(1));
    }

    // Otherwise just use murmur hash (should never happen and slightly slower, but generalizes).
    return emp::murmur_hash( GetBytes() );
  }

  // TODO: see https://arxiv.org/pdf/1611.07612.pdf for fast pop counts
  /// Count the number of ones in the BitArray.
  template <size_t NUM_BITS>
  size_t BitArray<NUM_BITS>::CountOnes() const { 
    size_t bit_count = 0;
    for (size_t i = 0; i < NUM_FIELDS; ++i) {
        // when compiling with -O3 and -msse4.2, this is the fastest population count method.
        std::bitset<FIELD_BITS> std_bs(bit_set[i]);
        bit_count += std_bs.count();
      }

    return bit_count;
  }

  /// Faster counting of ones for very sparse bit vectors.
  template <size_t NUM_BITS>
  size_t BitArray<NUM_BITS>::CountOnes_Sparse() const {
    size_t bit_count = 0;
    for (field_t cur_field : bit_set) {
      while (cur_field) {
        cur_field &= (cur_field-1);       // Peel off a single 1.
        bit_count++;                      // Increment the counter
      }
    }
    return bit_count;
  }

    /// Return the index of the first one in the sequence; return -1 if no ones are available.
  template <size_t NUM_BITS>
  int BitArray<NUM_BITS>::FindOne() const {
    size_t field_id = 0;
    while (field_id < NUM_FIELDS && bit_set[field_id]==0) field_id++;
    return (field_id < NUM_FIELDS) ?
      (int) (find_bit(bit_set[field_id]) + (field_id << FIELD_LOG2))  :  -1;
  }

  /// Return index of first one in sequence AFTER start_pos (or -1 if no ones)
  template <size_t NUM_BITS>
  int BitArray<NUM_BITS>::FindOne(const size_t start_pos) const {
    if (start_pos >= NUM_BITS) return -1;            // If we're past the end, return fail.
    size_t field_id  = FieldID(start_pos);           // What field do we start in?
    const size_t field_pos = FieldPos(start_pos);    // What position in that field?

    // If there's a hit in a partial first field, return it.
    if (field_pos && (bit_set[field_id] & ~(MaskLow<field_t>(field_pos)))) {
      return (int) (find_bit(bit_set[field_id] & ~(MaskLow<field_t>(field_pos))) +
                    field_id * FIELD_BITS);
    }

    // Search other fields...
    if (field_pos) field_id++;
    while (field_id < NUM_FIELDS && bit_set[field_id]==0) field_id++;
    return (field_id < NUM_FIELDS) ?
      (int) (find_bit(bit_set[field_id]) + (field_id * FIELD_BITS)) : -1;
  }

  /// Find the most-significant set-bit.
  template <size_t NUM_BITS>
  int BitArray<NUM_BITS>::FindMaxOne() const {
    // Find the max field with a one.
    int max_field = ((int) NUM_FIELDS) - 1;
    while (max_field >= 0 && bit_set[max_field] == 0) max_field--;

    // If there are no ones, return -1.
    if (max_field == -1) return -1;

    const field_t field = bit_set[max_field]; // Save a local copy of this field.
    field_t mask = (field_t) -1;              // Mask off the bits still under consideration.
    size_t offset = 0;                        // Indicate where the mask should be applied.
    size_t range = FIELD_BITS;                // Indicate how many bits are in the mask.

    while (range > 1) {
      // Cut the range in half and see if we need to adjust the offset.
      range /= 2;      // Cut range size in half
      mask >>= range;  // Cut the mask down.

      // Check the upper half of original range; if has a one shift new offset to there.
      if (field & (mask << (offset + range))) offset += range;
    }

    return (int) (max_field * FIELD_BITS + offset);
  }

  /// Return index of first one in sequence (or -1 if no ones); change this position to zero.
  template <size_t NUM_BITS>
  int BitArray<NUM_BITS>::PopOne() {
    const int out_bit = FindOne();
    if (out_bit >= 0) Clear(out_bit);
    return out_bit;
  }

  /// Return a vector indicating the posistions of all ones in the BitArray.
  template <size_t NUM_BITS>
  emp::vector<size_t> BitArray<NUM_BITS>::GetOnes() const {
    // @CAO -- There are better ways to do this with bit tricks.
    emp::vector<size_t> out_set(CountOnes());
    size_t cur_pos = 0;
    for (size_t i = 0; i < NUM_BITS; i++) {
      if (Get(i)) out_set[cur_pos++] = i;
    }
    return out_set;
  }

  /// Find the length of the longest continuous series of ones.
  template <size_t NUM_BITS>
  size_t BitArray<NUM_BITS>::LongestSegmentOnes() const {
    size_t length = 0;
    BitArray test_bits(*this);
    while(test_bits.Any()){
      ++length;
      test_bits.AND_SELF(test_bits<<1);
    }
    return length;
  }


  // -------------------------  Print/String Functions  ------------------------- //

  /// Convert this BitArray to a vector string [0 index on left]
  template <size_t NUM_BITS>
  std::string BitArray<NUM_BITS>::ToString() const {
    std::string out_string;
    out_string.reserve(NUM_BITS);
    for (size_t i = 0; i < NUM_BITS; ++i) out_string.push_back(GetAsChar(i));
    return out_string;
  }

  /// Convert this BitArray to a numerical string [0 index on right]
  template <size_t NUM_BITS>
  std::string BitArray<NUM_BITS>::ToBinaryString() const {
    std::string out_string;
    out_string.reserve(NUM_BITS);
    for (size_t i = NUM_BITS; i > 0; --i) out_string.push_back(GetAsChar(i-1));
    return out_string;
  }

  /// Convert this BitArray to a series of IDs
  template <size_t NUM_BITS>
  std::string BitArray<NUM_BITS>::ToIDString(const std::string & spacer) const {
    std::stringstream ss;
    PrintOneIDs(ss, spacer);
    return ss.str();
  }

  /// Convert this BitArray to a series of IDs with ranges condensed.
  template <size_t NUM_BITS>
  std::string BitArray<NUM_BITS>::ToRangeString(const std::string & spacer,
                                       const std::string & ranger) const
  {
    std::stringstream ss;
    PrintAsRange(ss, spacer, ranger);
    return ss.str();
  }

  /// Print a space between each field (or other provided spacer)
  template <size_t NUM_BITS>
  void BitArray<NUM_BITS>::PrintFields(std::ostream & out, const std::string & spacer) const {
    for (size_t i = NUM_BITS-1; i < NUM_BITS; i--) {
      out << Get(i);
      if (i && (i % FIELD_BITS == 0)) out << spacer;
    }
  }

  /// Print a space between each field (or other provided spacer)
  template <size_t NUM_BITS>
  void BitArray<NUM_BITS>::PrintDebug(std::ostream & out) const {
    for (size_t field = 0; field < NUM_FIELDS; field++) {
      for (size_t bit_id = 0; bit_id < FIELD_BITS; bit_id++) {
        bool bit = (FIELD_1 << bit_id) & bit_set[field];
        out << ( bit ? 1 : 0 );
      }
      out << " : " << field << std::endl;
    }
    size_t end_pos = NUM_END_BITS;
    if (end_pos == 0) end_pos = FIELD_BITS;
    for (size_t i = 0; i < end_pos; i++) out << " ";
    out << "^" << std::endl;
  }

  /// Print the locations of all one bits, using the provided spacer (default is a single space)
  template <size_t NUM_BITS>
  void BitArray<NUM_BITS>::PrintOneIDs(std::ostream & out, const std::string & spacer) const {
    bool started = false;
    for (size_t i = 0; i < NUM_BITS; i++) {
      if (Get(i)) {
        if (started) out << spacer;
        out << i;
        started = true;
      }
    }
  }

  /// Print the ones in a range format.  E.g., 2-5,7,10-15
  template <size_t NUM_BITS>
  void BitArray<NUM_BITS>::PrintAsRange(std::ostream & out,
                                      const std::string & spacer,
                                      const std::string & ranger) const
  {
    emp::vector<size_t> ones = GetOnes();        // Identify the one to represent in output.

    for (size_t pos = 0; pos < ones.size(); pos++) {
      if (pos) out << spacer;                    // If not first range, put a space before it.
      size_t start = ones[pos];                  // The current range starts here.
      while (pos+1 < ones.size() &&              // If there is another one...
             ones[pos+1] == ones[pos]+1) pos++;  // ...and it is sequential to this one, grab it.
      size_t end = ones[pos];                    // The last one we got to is the end position.

      out << start;                              // Output the range start.
      if (start != end) out << ranger << end;    // If there's more than one in range, show range.
    }
  }


  // -------------------------  Whole BitArray manipulation functions  -------------------------


  /// Perform a Boolean NOT on this BitArray, store result here, and return this object.
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::NOT_SELF() {
    for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~bit_set[i];
    ClearExcessBits();
    return *this;
  }

  /// Perform a Boolean AND with a second BitArray, store result here, and return this object.
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::AND_SELF(const BitArray<NUM_BITS> & set2) {
    for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = bit_set[i] & set2.bit_set[i];
    return *this;
  }

  /// Perform a Boolean OR with a second BitArray, store result here, and return this object.
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::OR_SELF(const BitArray<NUM_BITS> & set2) {
    for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = bit_set[i] | set2.bit_set[i];
    return *this;
  }

  /// Perform a Boolean NAND with a second BitArray, store result here, and return this object.
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::NAND_SELF(const BitArray<NUM_BITS> & set2) {
    for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~(bit_set[i] & set2.bit_set[i]);
    ClearExcessBits();
    return *this;
  }

  /// Perform a Boolean NOR with a second BitArray, store result here, and return this object.
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::NOR_SELF(const BitArray<NUM_BITS> & set2) {
    for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~(bit_set[i] | set2.bit_set[i]);
    ClearExcessBits();
    return *this;
  }

  /// Perform a Boolean XOR with a second BitArray, store result here, and return this object.
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::XOR_SELF(const BitArray<NUM_BITS> & set2) {
    for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = bit_set[i] ^ set2.bit_set[i];
    return *this;
  }

  /// Perform a Boolean EQU with a second BitArray, store result here, and return this object.
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::EQU_SELF(const BitArray<NUM_BITS> & set2) {
    for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~(bit_set[i] ^ set2.bit_set[i]);
    ClearExcessBits();
    return *this;
  }

  /// Positive shifts go right and negative shifts go left (0 does nothing);
  /// return result.
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> BitArray<NUM_BITS>::SHIFT(const int shift_size) const {
    BitArray<NUM_BITS> out_set(*this);
    if (shift_size > 0) out_set.ShiftRight((field_t) shift_size);
    else if (shift_size < 0) out_set.ShiftLeft((field_t) (-shift_size));
    return out_set;
  }

  /// Positive shifts go right and negative shifts go left (0 does nothing);
  /// store result here, and return this object.
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::SHIFT_SELF(const int shift_size) {
    if (shift_size > 0) ShiftRight((field_t) shift_size);
    else if (shift_size < 0) ShiftLeft((field_t) -shift_size);
    return *this;
  }

  /// Reverse the order of bits in the bitset
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::REVERSE_SELF() {

    // reverse bytes
    std::reverse( BytePtr().Raw(), BytePtr().Raw() + TOTAL_BYTES );

    // reverse each byte
    // adapted from https://stackoverflow.com/questions/2602823/in-c-c-whats-the-simplest-way-to-reverse-the-order-of-bits-in-a-byte
    for (size_t i = 0; i < TOTAL_BYTES; ++i) {
      unsigned char & b = BytePtr()[i];
      b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
      b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
      b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    }

    // shift out filler bits
    constexpr size_t filler_bits = NUM_BITS % 8;
    if constexpr (filler_bits != 0) {
      this->ShiftRight(8-filler_bits);
    }

    return *this;

  }

  /// Reverse order of bits in the bitset.
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> BitArray<NUM_BITS>::REVERSE() const {
    BitArray<NUM_BITS> out_set(*this);
    return out_set.REVERSE_SELF();
  }


  /// Positive rotates go left and negative rotates go left (0 does nothing);
  /// return result.
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> BitArray<NUM_BITS>::ROTATE(const int rotate_size) const {
    BitArray<NUM_BITS> out_set(*this);
    if (rotate_size > 0) out_set.RotateRight((field_t) rotate_size);
    else if (rotate_size < 0) out_set.RotateLeft((field_t) (-rotate_size));
    return out_set;
  }

  /// Positive rotates go right and negative rotates go left (0 does nothing);
  /// store result here, and return this object.
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::ROTATE_SELF(const int rotate_size) {
    if (rotate_size > 0) RotateRight((field_t) rotate_size);
    else if (rotate_size < 0) RotateLeft((field_t) -rotate_size);
    return *this;
  }

  /// Helper: call ROTATE with negative number instead
  template <size_t NUM_BITS>
  template<size_t shift_size_raw>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::ROTL_SELF() {
    constexpr size_t shift_size = shift_size_raw % NUM_BITS;

    // special case: for exactly one field_t, try to go low level
    // adapted from https://stackoverflow.com/questions/776508/best-practices-for-circular-shift-rotate-operations-in-c
    if constexpr (NUM_FIELDS == 1) {
      field_t & n = bit_set[0];
      size_t c = shift_size;

      // mask necessary to suprress shift count overflow warnings
      c &= FIELD_LOG2_MASK;
      n = (n<<c) | (n>>( (-(c+FIELD_BITS-NUM_BITS)) & FIELD_LOG2_MASK ));

    } else {

      // note that we already modded shift_size by NUM_BITS
      // so there's no need to mod by FIELD_SIZE here
      constexpr int field_shift = NUM_END_BITS ? (
        (shift_size + FIELD_BITS - NUM_END_BITS) / FIELD_BITS
      ) : (
        shift_size / FIELD_BITS
      );
      // if we field shift, we need to shift bits by (FIELD_BITS - NUM_END_BITS)
      // more to account for the filler that gets pulled out of the middle
      constexpr int bit_shift = NUM_END_BITS && field_shift ? (
        (shift_size + FIELD_BITS - NUM_END_BITS) % FIELD_BITS
      ) : (
        shift_size % FIELD_BITS
      );
      constexpr int bit_overflow = FIELD_BITS - bit_shift;

      // if rotating more than field capacity, we need to rotate fields
      if constexpr ((bool)field_shift) {
        std::rotate(
          std::rbegin(bit_set),
          std::rbegin(bit_set)+field_shift,
          std::rend(bit_set)
        );
      }

      // if necessary, shift filler bits out of the middle
      if constexpr ((bool)NUM_END_BITS) {
        const int filler_idx = (LAST_FIELD + field_shift) % NUM_FIELDS;
        for (int i = filler_idx + 1; i < (int)NUM_FIELDS; ++i) {
          bit_set[i-1] |= bit_set[i] << NUM_END_BITS;
          bit_set[i] >>= (FIELD_BITS - NUM_END_BITS);
        }
      }

      // account for bit_shift
      if (bit_shift) {

        const field_t keystone = NUM_END_BITS ? (
          (bit_set[LAST_FIELD] << (FIELD_BITS - NUM_END_BITS))
          | (bit_set[NUM_FIELDS - 2] >> NUM_END_BITS)
        ) : (
          bit_set[LAST_FIELD]
        );

        for (int i = LAST_FIELD; i > 0; --i) {
          bit_set[i] <<= bit_shift;
          bit_set[i] |= (bit_set[i-1] >> bit_overflow);
        }
        // Handle final field
        bit_set[0] <<= bit_shift;
        bit_set[0] |= keystone >> bit_overflow;

      }

    }

    ClearExcessBits();

    return *this;

  }


  /// Helper for calling ROTATE with positive number
  template <size_t NUM_BITS>
  template<size_t shift_size_raw>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::ROTR_SELF() {

    constexpr size_t shift_size = shift_size_raw % NUM_BITS;

    // special case: for exactly one field_t, try to go low level
    // adapted from https://stackoverflow.com/questions/776508/best-practices-for-circular-shift-rotate-operations-in-c
    if constexpr (NUM_FIELDS == 1) {
      field_t & n = bit_set[0];
      size_t c = shift_size;

      // mask necessary to suprress shift count overflow warnings
      c &= FIELD_LOG2_MASK;
      n = (n>>c) | (n<<( (NUM_BITS-c) & FIELD_LOG2_MASK ));

    } else {

      constexpr field_t field_shift = (shift_size / FIELD_BITS) % NUM_FIELDS;
      constexpr int bit_shift = shift_size % FIELD_BITS;
      constexpr field_t bit_overflow = FIELD_BITS - bit_shift;

      // if rotating more than field capacity, we need to rotate fields
      if constexpr ((bool)field_shift) {
        std::rotate(
          std::begin(bit_set),
          std::begin(bit_set)+field_shift,
          std::end(bit_set)
        );
      }

      // if necessary, shift filler bits out of the middle
      if constexpr ((bool)NUM_END_BITS) {
        constexpr int filler_idx = LAST_FIELD - field_shift;
        for (int i = filler_idx + 1; i < (int)NUM_FIELDS; ++i) {
          bit_set[i-1] |= bit_set[i] << NUM_END_BITS;
          bit_set[i] >>= (FIELD_BITS - NUM_END_BITS);
        }
      }

      // account for bit_shift
      if (bit_shift) {

        const field_t keystone = NUM_END_BITS ? (
          bit_set[0] >> (FIELD_BITS - NUM_END_BITS)
        ) : (
          bit_set[0]
        );

        if constexpr ((bool)NUM_END_BITS) {
          bit_set[NUM_FIELDS-1] |= bit_set[0] << NUM_END_BITS;
        }

        for (size_t i = 0; i < LAST_FIELD; ++i) {
          bit_set[i] >>= bit_shift;
          bit_set[i] |= (bit_set[i+1] << bit_overflow);
        }
        bit_set[LAST_FIELD] >>= bit_shift;
        bit_set[LAST_FIELD] |= keystone << bit_overflow;
      }
    }

    ClearExcessBits();

    return *this;

  }

  /// Addition of two Bitsets.
  /// Wraps if it overflows.
  /// Returns result.
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> BitArray<NUM_BITS>::ADD(const BitArray & set2) const{
    BitArray<NUM_BITS> out_set(*this);
    return out_set.ADD_SELF(set2);
  }

  /// Addition of two Bitsets.
  /// Wraps if it overflows.
  /// Returns this object.
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::ADD_SELF(const BitArray<NUM_BITS> & set2) {
    bool carry = false;

    for (size_t i = 0; i < NUM_BITS/FIELD_BITS; ++i) {
      field_t addend = set2.bit_set[i] + static_cast<field_t>(carry);
      carry = set2.bit_set[i] > addend;

      field_t sum = bit_set[i] + addend;
      carry |= bit_set[i] > sum;

      bit_set[i] = sum;
    }

    if constexpr (static_cast<bool>(NUM_END_BITS)) {
      bit_set[NUM_BITS/FIELD_BITS] = (
        bit_set[NUM_BITS/FIELD_BITS]
        + set2.bit_set[NUM_BITS/FIELD_BITS]
        + static_cast<field_t>(carry)
      ) & END_MASK;
    }

    return *this;
  }

  /// Subtraction of two Bitsets.
  /// Wraps around if it underflows.
  /// Returns result.
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> BitArray<NUM_BITS>::SUB(const BitArray<NUM_BITS> & set2) const{
    BitArray<NUM_BITS> out_set(*this);
    return out_set.SUB_SELF(set2);
  }

  /// Subtraction of two Bitsets.
  /// Wraps if it underflows.
  /// Returns this object.
  template <size_t NUM_BITS>
  BitArray<NUM_BITS> & BitArray<NUM_BITS>::SUB_SELF(const BitArray<NUM_BITS> & set2){

    bool carry = false;

    for (size_t i = 0; i < NUM_BITS/FIELD_BITS; ++i) {
      field_t subtrahend = set2.bit_set[i] + static_cast<field_t>(carry);
      carry = set2.bit_set[i] > subtrahend;
      carry |= bit_set[i] < subtrahend;
      bit_set[i] -= subtrahend;
    }

    if constexpr (static_cast<bool>(NUM_END_BITS)) {
      bit_set[NUM_BITS/FIELD_BITS] = (
        bit_set[NUM_BITS/FIELD_BITS]
        - set2.bit_set[NUM_BITS/FIELD_BITS]
        - static_cast<field_t>(carry)
      ) & END_MASK;
    }

    return *this;
  }


  // -------------------------  Extra Functions  -------------------------

  template <size_t NUM_BITS1, size_t NUM_BITS2>
  BitArray<NUM_BITS1+NUM_BITS2> join(const BitArray<NUM_BITS1> & in1, const BitArray<NUM_BITS2> & in2) {
    BitArray<NUM_BITS1+NUM_BITS2> out_bits;
    out_bits.Import(in2);
    out_bits <<= NUM_BITS1;
    out_bits |= in2.template Export<NUM_BITS1+NUM_BITS2>();
  }

  /// Computes simple matching coefficient (https://en.wikipedia.org/wiki/Simple_matching_coefficient).
  template <size_t NUM_BITS>
  double SimpleMatchCoeff(const BitArray<NUM_BITS> & in1, const BitArray<NUM_BITS> & in2) {
    emp_assert(NUM_BITS > 0); // TODO: can be done with XOR
    return (double)(~(in1 ^ in2)).CountOnes() / (double) NUM_BITS;
  }

}

/// For hashing BitArrays
namespace std
{
    template <size_t N>
    struct hash<emp::BitArray<N>>
    {
        size_t operator()( const emp::BitArray<N>& bs ) const
        {
          return bs.Hash();
        }
    };
}

#endif
