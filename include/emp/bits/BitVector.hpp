/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2021.
 *
 *  @file BitVector.hpp
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
 *
 *  @note This class is 15-20% slower than emp::BitSet, but more flexible & run-time configurable.
 */

#ifndef EMP_BITS_BITVECTOR_HPP_INCLUDE
#define EMP_BITS_BITVECTOR_HPP_INCLUDE


#include <bitset>
#include <cstring>
#include <initializer_list>
#include <iostream>

#include "../base/assert.hpp"
#include "../base/Ptr.hpp"
#include "../base/vector.hpp"
#include "../datastructs/hash_utils.hpp"
#include "../math/math.hpp"
#include "../math/Random.hpp"
#include "../polyfill/span.hpp"

#include "_bitset_helpers.hpp"
#include "bitset_utils.hpp"

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

    // Number of bits needed to specify position in a field + mask
    static constexpr size_t FIELD_LOG2 = emp::Log2(FIELD_BITS);
    static constexpr field_t FIELD_LOG2_MASK = MaskLow<field_t>(FIELD_LOG2);

    size_t num_bits;        ///< Total number of bits are we using
    Ptr<field_t> bits;      ///< Pointer to array with the status of each bit

    /// Num bits used in partial field at the end; 0 if perfect fit.
    [[nodiscard]] size_t NumEndBits() const { return num_bits & (FIELD_BITS - 1); }

    /// How many EXTRA bits are leftover in the gap at the end?
    [[nodiscard]] size_t EndGap() const { return NumEndBits() ? (FIELD_BITS - NumEndBits()) : 0; }

    /// A mask to cut off all of the final bits.
    [[nodiscard]] field_t EndMask() const { return MaskLow<field_t>(NumEndBits()); }

    /// How many feilds do we need for the current set of bits?
    [[nodiscard]] size_t NumFields() const { return num_bits ? (1 + ((num_bits - 1) / FIELD_BITS)) : 0; }

    /// What is the ID of the last occupied field?
    [[nodiscard]] size_t LastField() const { return NumFields() - 1; }

    /// How many bytes are used for the current set of bits? (rounded up!)
    [[nodiscard]] size_t NumBytes() const { return num_bits ? (1 + ((num_bits - 1) >> 3)) : 0; }

    /// How many bytes are allocated? (rounded up!)
    [[nodiscard]] size_t TotalBytes() const { return NumFields() * sizeof(field_t); }

    // Identify the field that a specified bit is in.
    [[nodiscard]] static constexpr size_t FieldID(const size_t index)  { return index / FIELD_BITS; }

    // Identify the position within a field where a specified bit is.
    [[nodiscard]] static constexpr size_t FieldPos(const size_t index) { return index & (FIELD_BITS-1); }

    // Identify which field a specified byte position would be in.
    [[nodiscard]] static constexpr size_t Byte2Field(const size_t index) { return index / sizeof(field_t); }

    // Convert a byte position in BitVector to a byte position in the target field.
    [[nodiscard]] static constexpr size_t Byte2FieldPos(const size_t index) { return FieldPos(index * 8); }

    // Assume that the size of the bits has already been adjusted to be the size of the one
    // being copied and only the fields need to be copied over.
    void RawCopy(const Ptr<field_t> in);

    // Copy bits from one position in the genome to another; leave old positions unchanged.
    void RawCopy(const size_t from_start, const size_t from_stop, const size_t to);

    // Convert the bits to bytes (note that bits are NOT in order at the byte level!)
    [[nodiscard]] emp::Ptr<unsigned char> BytePtr() { return bits.ReinterpretCast<unsigned char>(); }

    // Convert the bits to const bytes array (note that bits are NOT in order at the byte level!)
    [[nodiscard]] emp::Ptr<const unsigned char> BytePtr() const {
      return bits.ReinterpretCast<const unsigned char>();
    }

    // Any bits past the last "real" bit in the last field should be kept as zeros.
    void ClearExcessBits() { if (NumEndBits()) bits[LastField()] &= EndMask(); }

    // Apply a transformation to each bit field in a specified range.
    template <typename FUN_T>
    inline BitVector & ApplyRange(const FUN_T & fun, size_t start, size_t stop);

    // Helper: call SHIFT with positive number
    void ShiftLeft(const size_t shift_size);

    // Helper for calling SHIFT with negative number
    void ShiftRight(const size_t shift_size);

    /// Helper: call ROTATE with negative number instead
    void RotateLeft(const size_t shift_size_raw);

    /// Helper for calling ROTATE with positive number
    void RotateRight(const size_t shift_size_raw);

  public:
    /// Build a new BitVector with specified bit count (default 0) and initialization (default 0)
    BitVector(size_t in_num_bits=0, bool init_val=false);

    // Prevent ambiguous conversions...
    /// Anything not otherwise defined for first argument, convert to size_t.
    template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, int>::type = 0>
    BitVector(T in_num_bits) : BitVector((size_t) in_num_bits, 0) {}

    /// Copy constructor of existing bit field.
    BitVector(const BitVector & in);

    /// Move constructor of existing bit field.
    BitVector(BitVector && in);

    /// Constructor to generate a BitVector from a std::bitset.
    template <size_t NUM_BITS>
    explicit BitVector(const std::bitset<NUM_BITS> & bitset);

    /// Constructor to generate a BitVector from a string of '0's and '1's.
    BitVector(const std::string & bitstring);

    /// Constructor to generate a BitVector from a literal string of '0's and '1's.
    BitVector(const char * bitstring) : BitVector(std::string(bitstring)) {}

    /// Constructor to generate a random BitVector (with equal prob of 0 or 1).
    BitVector(size_t in_num_bits, Random & random);

    /// Constructor to generate a random BitVector with provided prob of 1's.
    BitVector(size_t in_num_bits, Random & random, const double p1);

    /// Constructor to generate a random BitVector with provided number of 1's.
    BitVector(size_t in_num_bits, Random & random, const size_t target_ones);

    /// Constructor to generate a random BitVector with provided number of 1's.
    BitVector(size_t in_num_bits, Random & random, const int target_ones)
      : BitVector(in_num_bits, random, (size_t) target_ones) { }

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

    /// Assignment operator from a std::bitset.
    template <size_t NUM_BITS>
    BitVector & operator=(const std::bitset<NUM_BITS> & bitset);

    /// Assignment operator from a string of '0's and '1's.
    BitVector & operator=(const std::string & bitstring);

    /// Assignment operator from a literal string of '0's and '1's.
    BitVector & operator=(const char * bitstring) { return operator=(std::string(bitstring)); }

    /// Assignment from another BitVector without changing size.
    BitVector & Import( const BitVector & from_bv, const size_t from_bit=0 );

    /// Convert to a BitVector of a different size.
    BitVector Export(size_t out_size, size_t start_bit=0) const;

    // Scan this bitvector to make sure that there are no internal problems.
    bool OK() const;


    // =========  Accessors  ========= //

    /// How many bits do we currently have?
    [[nodiscard]] size_t GetSize() const { return num_bits; }

    /// How many bytes are in this BitVector? (includes empty field space)
    [[nodiscard]] size_t GetNumBytes() const { return NumBytes(); }

    /// How many distinct values could be held in this BitVector?
    [[nodiscard]] double GetNumStates() const { return emp::Pow2(num_bits); }

    /// Retrive the bit value from the specified index.
    [[nodiscard]] bool Get(size_t index) const;

    /// A safe version of Get() for indexing out of range. Useful for representing collections.
    [[nodiscard]] bool Has(size_t index) const { return (index < num_bits) ? Get(index) : false; }

    /// Update the bit value at the specified index.
    BitVector & Set(size_t index, bool value=true);

    /// Set all bits to 1.
    BitVector & SetAll();

    /// Set a range of bits to one: [start, stop)
    BitVector & SetRange(size_t start, size_t stop)
      { return ApplyRange([](field_t){ return FIELD_ALL; }, start, stop); }

    /// Set all bits to 0.
    BitVector & Clear();

    /// Set specific bit to 0.
    BitVector & Clear(size_t index) { return Set(index, false); }

    /// Set bits to 0 in the range [start, stop)
    BitVector & Clear(const size_t start, const size_t stop)
      { return ApplyRange([](field_t){ return 0; }, start, stop); }


    /// Const index operator -- return the bit at the specified position.
    [[nodiscard]] bool operator[](size_t index) const { return Get(index); }

    /// Index operator -- return a proxy to the bit at the specified position so it can be an lvalue.
    BitProxy<BitVector> operator[](size_t index) { return BitProxy<BitVector>(*this, index); }

    /// Change every bit in the sequence.
    BitVector & Toggle() { return NOT_SELF(); }

    /// Change a specified bit to the opposite value
    BitVector & Toggle(size_t index);

    /// Flips all the bits in a range [start, end)
    BitVector & Toggle(size_t start, size_t stop)
      { return ApplyRange([](field_t x){ return ~x; }, start, stop); }

    /// Return true if ANY bits are set to 1, otherwise return false.
    [[nodiscard]] bool Any() const;

    /// Return true if NO bits are set to 1, otherwise return false.
    [[nodiscard]] bool None() const { return !Any(); }

    /// Return true if ALL bits are set to 1, otherwise return false.
    // @CAO: Can speed up by not duplicating the whole BitVector.
    [[nodiscard]] bool All() const { return (~(*this)).None(); }

    /// Resize this BitVector to have the specified number of bits.
    BitVector & Resize(size_t new_bits);


    // =========  Randomization functions  ========= //

    /// Set all bits randomly, with a 50% probability of being a 0 or 1.
    BitVector &  Randomize(Random & random);

    /// Set all bits randomly, with probability specified at compile time.
    template <Random::Prob P>
    BitVector & RandomizeP(Random & random, const size_t start_pos=0, size_t stop_pos=MAX_BITS);

    /// Set all bits randomly, with a given probability of being a one.
    BitVector & Randomize(Random & random, const double p,
                       const size_t start_pos=0, size_t stop_pos=MAX_BITS);

    /// Set all bits randomly, with a given number of ones.
    BitVector & ChooseRandom(Random & random, const int target_ones,
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
    BitVector & FlipRandomCount(Random & random, const size_t target_bits);

    /// Set a specified number of random bits (does not check if already set.)
    BitVector & SetRandomCount(Random & random, const size_t target_bits);

    /// Unset  a specified number of random bits (does not check if already zero.)
    BitVector & ClearRandomCount(Random & random, const size_t target_bits);


    // =========  Comparison Operators  ========= //

    [[nodiscard]] bool operator==(const BitVector & in) const;
    [[nodiscard]] bool operator!=(const BitVector & in) const { return !(*this == in); }
    [[nodiscard]] bool operator< (const BitVector & in) const;
    [[nodiscard]] bool operator> (const BitVector & in) const { return in < *this; }
    [[nodiscard]] bool operator<=(const BitVector & in) const { return !(in < *this); }
    [[nodiscard]] bool operator>=(const BitVector & in) const { return !(*this < in); }


    // =========  Conversion Operators  ========= //

    /// Automatically convert BitVector to other vector types.
    template <typename T> operator emp::vector<T>();

    /// Casting a bit array to bool identifies if ANY bits are set to 1.
    explicit operator bool() const { return Any(); }


    // =========  Access Groups of bits  ========= //

    /// Retrive the byte at the specified byte index.
    [[nodiscard]] uint8_t GetByte(size_t index) const;

    /// Get a read-only view into the internal array used by BitVector.
    /// @return Read-only span of BitVector's bytes.
    [[nodiscard]] std::span<const std::byte> GetBytes() const;

    /// Get a read-only pointer to the internal array used by BitVector.
    /// (note that bits are NOT in order at the byte level!)
    /// @return Read-only pointer to BitVector's bytes.
    emp::Ptr<const unsigned char> RawBytes() const { return BytePtr(); }

    /// Update the byte at the specified byte index.
    void SetByte(size_t index, uint8_t value);

    /// Get the overall value of this BitVector, using a uint encoding, but including all bits
    /// and returning the value as a double.
    [[nodiscard]] double GetValue() const;

    /// Return a span with all fields in order.
    std::span<field_t> FieldSpan() { return std::span<field_t>(bits.Raw(), NumFields()); }

    /// Get specified type at a given index (in steps of that type size)
    template <typename T>
    [[nodiscard]] T GetValueAtIndex(const size_t index) const;

    // Retrieve the 8-bit uint from the specified uint index.
    [[nodiscard]] uint8_t GetUInt8(size_t index) const { return GetValueAtIndex<uint8_t>(index); }

    // Retrieve the 16-bit uint from the specified uint index.
    [[nodiscard]] uint16_t GetUInt16(size_t index) const { return GetValueAtIndex<uint16_t>(index); }

    // Retrieve the 32-bit uint from the specified uint index.
    [[nodiscard]] uint32_t GetUInt32(size_t index) const { return GetValueAtIndex<uint32_t>(index); }

    // Retrieve the 64-bit uint from the specified uint index.
    [[nodiscard]] uint64_t GetUInt64(size_t index) const { return GetValueAtIndex<uint64_t>(index); }

    // By default, retrieve the 32-bit uint from the specified uint index.
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
    template <typename T>
    [[nodiscard]] T GetValueAtBit(const size_t index) const;

    // Retrieve the 8-bit uint from the specified uint index.
    [[nodiscard]] uint8_t GetUInt8AtBit(size_t index) const { return GetValueAtBit<uint8_t>(index); }

    // Retrieve the 16-bit uint from the specified uint index.
    [[nodiscard]] uint16_t GetUInt16AtBit(size_t index) const { return GetValueAtBit<uint16_t>(index); }

    // Retrieve the 32-bit uint from the specified uint index.
    [[nodiscard]] uint32_t GetUInt32AtBit(size_t index) const { return GetValueAtBit<uint32_t>(index); }

    // Retrieve the 64-bit uint from the specified uint index.
    [[nodiscard]] uint64_t GetUInt64AtBit(size_t index) const { return GetValueAtBit<uint64_t>(index); }

    // By default, retrieve the 32-bit uint from the specified uint index.
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


    // =========  Other Analyses  ========= //

    /// A simple hash function for bit vectors.
    [[nodiscard]] std::size_t Hash(size_t start_field=0) const;

    /// Count the number of ones in the BitVector.
    [[nodiscard]] size_t CountOnes() const;

    /// Faster counting of ones for very sparse bit vectors.
    [[nodiscard]] size_t CountOnes_Sparse() const;

    /// Count the number of zeros in the BitVector.
    [[nodiscard]] size_t CountZeros() const { return GetSize() - CountOnes(); }

    /// Pop the last bit in the vector.
    /// @return value of the popped bit.
    bool PopBack();

    /// Push given bit(s) onto the back of a vector.
    /// @param bit value of bit to be pushed.
    /// @param num number of bits to be pushed.
    void PushBack(const bool bit=true, const size_t num=1);

    /// Insert bit(s) into any index of vector using bit magic.
    /// Blog post on implementation reasoning: https://devolab.org/?p=2249
    /// @param index location to insert bit(s).
    /// @param val value of bit(s) to insert.
    /// @param num number of bits to insert, default 1.
    void Insert(const size_t index, const bool val=true, const size_t num=1);

    /// Delete bits from any index in a vector.
    /// TODO: consider a bit magic approach here.
    /// @param index location to delete bit(s).
    /// @param num number of bits to delete, default 1.
    void Delete(const size_t index, const size_t num=1);

    /// Return the position of the first one; return -1 if no ones in vector.
    [[nodiscard]] int FindOne() const;

    /// Deprecated: Return the position of the first one; return -1 if no ones in vector.
    [[deprecated("Renamed to more acurate FindOne()")]]
    [[nodiscard]] int FindBit() const { return FindOne(); }

    /// Return the position of the first one after start_pos; return -1 if no ones in vector.
    /// You can loop through all 1-bit positions of a BitVector "bv" with:
    ///
    ///   for (int pos = bv.FindOne(); pos >= 0; pos = bv.FindOne(pos+1)) { ... }
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

    /// Return true if any ones are in common with another BitVector.
    [[nodiscard]] bool HasOverlap(const BitVector & in) const;


    // =========  Print/String Functions  ========= //

    /// Convert a specified bit to a character.
    [[nodiscard]] char GetAsChar(size_t id) const { return Get(id) ? '1' : '0'; }

    /// Convert this BitVector to a vector string [index 0 on left]
    [[nodiscard]] std::string ToString() const;

    /// Convert this BitVector to a numerical string [index 0 on right]
    [[nodiscard]] std::string ToBinaryString() const;

    /// Convert this BitVector to a series of IDs
    [[nodiscard]] std::string ToIDString(const std::string & spacer=" ") const;

    /// Convert this BitVector to a series of IDs with ranges condensed.
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

    /// Print out details about the internals of the BitVector.
    void PrintDebug(std::ostream & out=std::cout) const;

    /// Print the positions of all one bits, spaces are the default separator.
    void PrintOneIDs(std::ostream & out=std::cout, const std::string & spacer=" ") const;

    /// Print the ones in a range format.  E.g., 2-5,7,10-15
    void PrintAsRange(std::ostream & out=std::cout,
                      const std::string & spacer=",",
                      const std::string & ranger="-") const;

    /// Overload ostream operator to return Print.
    friend std::ostream& operator<<(std::ostream &out, const BitVector & bv) {
      bv.Print(out);
      return out;
    }


    // =========  Boolean Logic and Shifting Operations  ========= //

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


    /// Perform a Boolean NOT on this BitVector and return the result.
    [[nodiscard]] BitVector NOT() const { return BitVector(*this).NOT_SELF(); }

    /// Perform a Boolean AND on this BitVector and return the result.
    [[nodiscard]] BitVector AND(const BitVector & bv2) const { return BitVector(*this).AND_SELF(bv2); }

    /// Perform a Boolean OR on this BitVector and return the result.
    [[nodiscard]] BitVector OR(const BitVector & bv2) const { return BitVector(*this).OR_SELF(bv2); }

    /// Perform a Boolean NAND on this BitVector and return the result.
    [[nodiscard]] BitVector NAND(const BitVector & bv2) const { return BitVector(*this).NAND_SELF(bv2); }

    /// Perform a Boolean NOR on this BitVector and return the result.
    [[nodiscard]] BitVector NOR(const BitVector & bv2) const { return BitVector(*this).NOR_SELF(bv2); }

    /// Perform a Boolean XOR on this BitVector and return the result.
    [[nodiscard]] BitVector XOR(const BitVector & bv2) const { return BitVector(*this).XOR_SELF(bv2); }

    /// Perform a Boolean EQU on this BitVector and return the result.
    [[nodiscard]] BitVector EQU(const BitVector & bv2) const { return BitVector(*this).EQU_SELF(bv2); }


    /// Positive shifts go left and negative go right (0 does nothing); return result.
    [[nodiscard]] BitVector SHIFT(const int shift_size) const;

    /// Positive shifts go left and negative go right; store result here, and return this object.
    BitVector & SHIFT_SELF(const int shift_size);

    /// Reverse the order of bits in the bitset
    BitVector & REVERSE_SELF();

    /// Reverse order of bits in the bitset.
    [[nodiscard]] BitVector REVERSE() const;

    /// Positive rotates go left and negative rotates go left (0 does nothing);
    /// return result.
    [[nodiscard]] BitVector ROTATE(const int rotate_size) const;

    /// Positive rotates go right and negative rotates go left (0 does nothing);
    /// store result here, and return this object.
    BitVector & ROTATE_SELF(const int rotate_size);

    /// Helper: call ROTATE with negative number instead
    template<size_t shift_size_raw>
    BitVector & ROTL_SELF();

    /// Helper for calling ROTATE with positive number
    template<size_t shift_size_raw>
    BitVector & ROTR_SELF();

    /// Addition of two BitVectors.
    /// Wraps if it overflows.
    /// Returns result.
    [[nodiscard]] BitVector ADD(const BitVector & set2) const;

    /// Addition of two BitVectors.
    /// Wraps if it overflows.
    /// Returns this object.
    BitVector & ADD_SELF(const BitVector & set2);

    /// Subtraction of two BitVectors.
    /// Wraps around if it underflows.
    /// Returns result.
    [[nodiscard]] BitVector SUB(const BitVector & set2) const;

    /// Subtraction of two BitVectors.
    /// Wraps if it underflows.
    /// Returns this object.
    BitVector & SUB_SELF(const BitVector & set2);


    /// Operator bitwise NOT...
    [[nodiscard]] inline BitVector operator~() const { return NOT(); }

    /// Operator bitwise AND...
    [[nodiscard]] inline BitVector operator&(const BitVector & ar2) const { return AND(ar2); }

    /// Operator bitwise OR...
    [[nodiscard]] inline BitVector operator|(const BitVector & ar2) const { return OR(ar2); }

    /// Operator bitwise XOR...
    [[nodiscard]] inline BitVector operator^(const BitVector & ar2) const { return XOR(ar2); }

    /// Operator shift left...
    [[nodiscard]] inline BitVector operator<<(const size_t shift_size) const { return SHIFT(-(int)shift_size); }

    /// Operator shift right...
    [[nodiscard]] inline BitVector operator>>(const size_t shift_size) const { return SHIFT((int)shift_size); }

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

    // =========  Standard Library Compatability  ========= //
    // A set of functions to allow drop-in replacement with std::bitset.

    [[nodiscard]] size_t size() const { return num_bits; }
    void resize(std::size_t new_size) { Resize(new_size); }
    [[nodiscard]] bool all() const { return All(); }
    [[nodiscard]] bool any() const { return Any(); }
    [[nodiscard]] bool none() const { return !Any(); }
    size_t count() const { return CountOnes(); }
    BitVector & flip() { return Toggle(); }
    BitVector & flip(size_t pos) { return Toggle(pos); }
    BitVector & flip(size_t start, size_t end) { return Toggle(start, end); }
    void reset() { Clear(); }
    void reset(size_t id) { Set(id, false); }
    void set() { SetAll(); }
    void set(size_t id) { Set(id); }
    [[nodiscard]] bool test(size_t index) const { return Get(index); }
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

  // Move bits from one position in the genome to another; leave old positions unchanged.
  // @CAO: Can speed up by focusing only on the moved fields (i.e., don't shift unused bits).
  void BitVector::RawCopy(const size_t from_start, const size_t from_stop, const size_t to) {
    emp_assert(from_start <= from_stop);  // Must move legal region.
    emp_assert(from_stop <= num_bits);    // Cannot move from past end.
    emp_assert(to <= num_bits);           // Must move to somewhere legal.

    // If nothing to copy OR already in place, stop right there.
    if (from_start == from_stop || from_start == to) return;

    const size_t move_size = from_stop - from_start;    // How bit is the chunk to move?
    const size_t to_stop = Min(to+move_size, num_bits); // Where is the end to move it to?
    const int shift = (int) from_start - (int) to;      // How far will the moved piece shift?
    BitVector move_bits(*this);                         // Vector to hold moved bits.
    move_bits.SHIFT_SELF(shift);                        // Put the moved bits in place.
    Clear(to, to_stop);                                 // Make room for the moved bits.
    move_bits.Clear(0, to);                             // Clear everything BEFORE moved bits.
    move_bits.Clear(to_stop, num_bits);                 // Clear everything AFTER moved bits.
    OR_SELF(move_bits);                                 // Merge bitstrings together.
  }

  template <typename FUN_T>
  BitVector & BitVector::ApplyRange(const FUN_T & fun, size_t start, size_t stop) {
    if (start == stop) return *this;  // Empty range.

    emp_assert(start <= stop, start, stop, num_bits);  // Start cannot be after stop.
    emp_assert(stop <= num_bits, stop, num_bits);      // Stop cannot be past the end of the bits
    const size_t start_pos = FieldPos(start);          // Identify the start position WITHIN a bit field.
    const size_t stop_pos = FieldPos(stop);            // Identify the stop position WITHIN a bit field.
    size_t start_field = FieldID(start);               // Ideftify WHICH bit field we're starting in.
    const size_t stop_field = FieldID(stop-1);         // Identify the last field where we actually make a change.

    // If the start field and stop field are the same, mask off the middle.
    if (start_field == stop_field) {
      const size_t apply_bits = stop - start;                          // How many bits to change?
      const field_t mask = MaskLow<field_t>(apply_bits) << start_pos;  // Target change bits with a mask.
      field_t & target = bits[start_field];                            // Isolate the field to change.
      target = (target & ~mask) | (fun(target) & mask);                // Update targeted bits!
    }

    // Otherwise mask the ends and fully modify the chunks in between.
    else {
      // If we're only using a portions of start field, mask it and setup.
      if (start_pos != 0) {
        const size_t start_bits = FIELD_BITS - start_pos;                // How many bits in start field?
        const field_t mask = MaskLow<field_t>(start_bits) << start_pos;  // Target start bits with a mask.
        field_t & target = bits[start_field];                            // Isolate the field to change.
        target = (target & ~mask) | (fun(target) & mask);                // Update targeted bits!
        start_field++;                                                   // Done with this field; move to the next.
      }

      // Middle fields
      for (size_t cur_field = start_field; cur_field < stop_field; cur_field++) {
        bits[cur_field] = fun(bits[cur_field]);
      }

      // Set portions of stop field
      const field_t mask = MaskLow<field_t>(stop_pos);
      field_t & target = bits[stop_field];                             // Isolate the field to change.
      target = (target & ~mask) | (fun(target) & mask);                // Update targeted bits!
    }

    return *this;
  }

  void BitVector::ShiftLeft(const size_t shift_size) {
    // If we are shifting out of range, clear the bits and stop.
    if (shift_size >= num_bits) { Clear(); return; }

    // If we have only a single field, this operation can be quick.
    if (NumFields() == 1) {
      (bits[0] <<= shift_size) &= EndMask();
      return;
    }

    const size_t field_shift = shift_size / FIELD_BITS;
    const size_t bit_shift = shift_size % FIELD_BITS;
    const size_t bit_overflow = FIELD_BITS - bit_shift;

    // Loop through each field, from L to R, and update it.
    if (field_shift) {
      for (size_t i = LastField(); i >= field_shift; --i) {
        bits[i] = bits[i - field_shift];
      }
      for (size_t i = field_shift; i > 0; --i) bits[i-1] = 0;
    }

    // account for bit_shift
    if (bit_shift) {
      for (size_t i = LastField() ; i > field_shift; --i) {
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

    // If we have only a single field, this operation can be quick.
    if (NumFields() == 1) {
      bits[0] >>= shift_size;
      return;
    }

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

  /// Helper: call ROTATE with negative number
  void BitVector::RotateLeft(const size_t shift_size_raw) {
    if (num_bits == 0) return;   // Nothing to rotate in an empty BitVector.

    const field_t shift_size = shift_size_raw % num_bits;
    const size_t NUM_FIELDS = NumFields();

    // Use different approaches based on BitVector size
    if (NUM_FIELDS == 1) {
      // Special case: for exactly one field_T, try to go low level.
      // Adapted from https://stackoverflow.com/questions/776508/best-practices-for-circular-shift-rotate-operations-in-c
      field_t & n = bits[0];
      size_t c = shift_size;

      // Mask necessary to suprress shift count overflow warnings.
      c &= FIELD_LOG2_MASK;
      n = (n<<c) | (n>>( (-(c+FIELD_BITS-num_bits)) & FIELD_LOG2_MASK ));
    }
    else if (NUM_FIELDS < 32) {  // For small BitVectors, shifting L/R and ORing is faster.
      emp::BitVector dup(*this);
      dup.ShiftLeft(shift_size);
      ShiftRight(num_bits - shift_size);
      OR_SELF(dup);
    }
    else {  // For big BitVectors, manual rotating is fater
      // Note: we already modded shift_size by num_bits, so no need to mod by FIELD_SIZE
      const int field_shift = ( shift_size + EndGap() ) / FIELD_BITS;

      // If we field shift, we need to shift bits by (FIELD_BITS - NumEndBits())
      // to account for the filler that gets pulled out of the middle
      const int bit_shift = NumEndBits() && (shift_size + field_shift ? EndGap() : 0) % FIELD_BITS;
      const int bit_overflow = FIELD_BITS - bit_shift;

      // if rotating more than field capacity, we need to rotate fields
      auto field_span = FieldSpan();
      std::rotate(
        field_span.rbegin(),
        field_span.rbegin()+field_shift,
        field_span.rend()
      );

      // if necessary, shift filler bits out of the middle
      if (NumEndBits()) {
        const int filler_idx = (LastField() + field_shift) % NUM_FIELDS;
        for (int i = filler_idx + 1; i < (int)NUM_FIELDS; ++i) {
          bits[i-1] |= bits[i] << NumEndBits();
          bits[i] >>= (FIELD_BITS - NumEndBits());
        }
      }

      // account for bit_shift
      if (bit_shift) {

        const field_t keystone = NumEndBits() ? (
          (bits[LastField()] << (FIELD_BITS - NumEndBits()))
          | (bits[NUM_FIELDS - 2] >> NumEndBits())
        ) : (
          bits[LastField()]
        );

        for (int i = LastField(); i > 0; --i) {
          bits[i] <<= bit_shift;
          bits[i] |= (bits[i-1] >> bit_overflow);
        }
        // Handle final field
        bits[0] <<= bit_shift;
        bits[0] |= keystone >> bit_overflow;

      }

    }

    // Mask out filler bits
    ClearExcessBits();
  }


  /// Helper for calling ROTATE with positive number
  void BitVector::RotateRight(const size_t shift_size_raw) {
    const size_t shift_size = shift_size_raw % num_bits;
    const size_t NUM_FIELDS = NumFields();

    // use different approaches based on BitVector size
    if (NUM_FIELDS == 1) {
      // special case: for exactly one field_t, try to go low level
      // adapted from https://stackoverflow.com/questions/776508/best-practices-for-circular-shift-rotate-operations-in-c

      field_t & n = bits[0];
      size_t c = shift_size;

      // mask necessary to suprress shift count overflow warnings
      c &= FIELD_LOG2_MASK;
      n = (n>>c) | (n<<( (num_bits-c) & FIELD_LOG2_MASK ));

    } else if (NUM_FIELDS < 32) {
      // for small BitVectors, shifting L/R and ORing is faster
      emp::BitVector dup(*this);
      dup.ShiftRight(shift_size);
      ShiftLeft(num_bits - shift_size);
      OR_SELF(dup);
    } else {
      // for big BitVectors, manual rotating is fater

      const field_t field_shift = (shift_size / FIELD_BITS) % NUM_FIELDS;
      const int bit_shift = shift_size % FIELD_BITS;
      const field_t bit_overflow = FIELD_BITS - bit_shift;

      // if rotating more than field capacity, we need to rotate fields
      auto field_span = FieldSpan();
      std::rotate(
        field_span.begin(),
        field_span.begin()+field_shift,
        field_span.end()
      );

      // if necessary, shift filler bits out of the middle
      if (NumEndBits()) {
        const int filler_idx = LastField() - field_shift;
        for (int i = filler_idx + 1; i < (int)NUM_FIELDS; ++i) {
          bits[i-1] |= bits[i] << NumEndBits();
          bits[i] >>= (FIELD_BITS - NumEndBits());
        }
      }

      // account for bit_shift
      if (bit_shift) {

        const field_t keystone = NumEndBits() ? (
          bits[0] >> (FIELD_BITS - NumEndBits())
        ) : (
          bits[0]
        );

        if (NumEndBits()) {
          bits[NUM_FIELDS-1] |= bits[0] << NumEndBits();
        }

        for (size_t i = 0; i < LastField(); ++i) {
          bits[i] >>= bit_shift;
          bits[i] |= (bits[i+1] << bit_overflow);
        }
        bits[LastField()] >>= bit_shift;
        bits[LastField()] |= keystone << bit_overflow;
      }
    }

    // Mask out filler bits
    ClearExcessBits();
  }


  ///////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////
  // ----------------------------------------------------------------------------------------
  // --------------------- Implementations of Public Member Functions -----------------------
  // ----------------------------------------------------------------------------------------
  ///////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////


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

  /// Constructor to generate a BitVector from a std::bitset.
  template <size_t NUM_BITS>
  BitVector::BitVector(const std::bitset<NUM_BITS> & bitset) : num_bits(NUM_BITS), bits(nullptr) {
    if (num_bits) {
      bits = NewArrayPtr<field_t>(NumFields());
      Clear();
      for (size_t i = 0; i < NUM_BITS; i++) if (bitset[i]) Set(i);
    }
  }

  /// Constructor to generate a BitVector from a string of '0's and '1's.
  BitVector::BitVector(const std::string & bitstring) : num_bits(bitstring.size()), bits(nullptr) {
    if (num_bits) {
      bits = NewArrayPtr<field_t>(NumFields());
      Clear();
      for (size_t i = 0; i < num_bits; i++) {
        if (bitstring[i] != '0') Set(i);
      }
    }
  }

  /// Constructor to generate a random BitVector (with equal prob of 0 or 1).
  BitVector::BitVector(size_t in_num_bits, Random & random)
  : num_bits(in_num_bits), bits(nullptr)
  {
    if (num_bits) {
      bits = NewArrayPtr<field_t>(NumFields());
      Clear();
      Randomize(random);
    }
  }

  /// Constructor to generate a random BitVector with provided prob of 1's.
  BitVector::BitVector(size_t in_num_bits, Random & random, const double p1)
  : num_bits(in_num_bits), bits(nullptr)
  {
    if (num_bits) {
      bits = NewArrayPtr<field_t>(NumFields());
      Clear();
      Randomize(random, p1);
    }
  }

  /// Constructor to generate a random BitVector with provided number of 1's.
  BitVector::BitVector(size_t in_num_bits, Random & random, const size_t target_ones)
  : num_bits(in_num_bits), bits(nullptr)
  {
    if (num_bits) {
      bits = NewArrayPtr<field_t>(NumFields());
      Clear();
      ChooseRandom(random, target_ones);
    }
  }

  /// Initializer list constructor.
  template <typename T>
  BitVector::BitVector(const std::initializer_list<T> l) : num_bits(l.size()), bits(nullptr) {
    if (num_bits) bits = NewArrayPtr<field_t>(NumFields());

    size_t idx = 0;
    for (auto i = std::begin(l); i != std::end(l); ++i) Set(idx++, *i);
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
    if (bits) bits.DeleteArray();   // If we already have bits, get rid of them.
    num_bits = in.num_bits;         // Update the number of bits...
    bits = in.bits;                 // And steal the old memory for what those bits are.
    in.bits = nullptr;              // Prepare in for deletion without deallocating.
    in.num_bits = 0;

    return *this;
  }

  /// Assignment operator from a std::bitset.
  template <size_t NUM_BITS>
  BitVector & BitVector::operator=(const std::bitset<NUM_BITS> & bitset) {
    const size_t start_fields = NumFields();
    num_bits = NUM_BITS;
    const size_t new_fields = NumFields();

    // Update the size of internal fields if needed.
    if (start_fields != new_fields) {
      if (bits) bits.DeleteArray();   // If we already had a bitset, get rid of it.
      if constexpr (NUM_BITS > 0) bits = NewArrayPtr<field_t>(new_fields);
      else bits = nullptr;
    }

    for (size_t i = 0; i < NUM_BITS; i++) Set(i, bitset[i]);  // Copy bits in.
    ClearExcessBits();                                        // Set excess bits to zeros.

    return *this;
  }

  /// Assignment operator from a string of '0's and '1's.
  BitVector & BitVector::operator=(const std::string & bitstring) {
    const size_t start_fields = NumFields();
    num_bits = bitstring.size();
    const size_t new_fields = NumFields();

    // Update the size of internal fields if needed.
    if (start_fields != new_fields) {
      if (bits) bits.DeleteArray();   // If we already had a bitset, get rid of it.
      if (num_bits) bits = NewArrayPtr<field_t>(new_fields);
      else bits = nullptr;
      Clear();
    }

    // If we have bits, copy them in.
    if (num_bits) {
      for (size_t i = 0; i < num_bits; i++) {
        if (bitstring[i] != '0') Set(i);
      }
    }

    return *this;
  }


  /// Assign from a BitVector of a different size.
  // @CAO: Can manually copy to skip unused fields for a speedup.
  BitVector & BitVector::Import(const BitVector & from_bv, const size_t from_bit) {
    emp_assert(&from_bv != this);
    emp_assert(from_bit < from_bv.GetSize());

    const size_t init_size = GetSize();
    *this = from_bv;
    *this >>= from_bit;
    Resize(init_size);

    return *this;
  }

  /// Convert to a Bitset of a different size.
  BitVector BitVector::Export(size_t out_size, size_t start_bit) const {

    BitVector out_bits(out_size);
    out_bits.Import(*this, start_bit);

    return out_bits;
  }

  bool BitVector::OK() const {
    // Do some checking on the bits array ptr to make sure it's value.
    if (bits) {
#ifdef EMP_TRACK_MEM
      emp_assert(bits.DebugIsArray()); // Must be marked as an array.
      emp_assert(bits.OK());           // Pointer must be okay.
#endif

      // If there are end bits, make sure that everything past the last one is clear.
      if (NumEndBits()) {
        // Make sure final bits are zeroed out.
        [[maybe_unused]] field_t excess_bits = bits[LastField()] & ~MaskLow<field_t>(NumEndBits());
        emp_assert(!excess_bits);
      }
    }

    // Otherwise bits is null; num_bits should be zero.
    else emp_assert(num_bits == 0);

    return true;
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

  /// Set all bits to 0.
  BitVector & BitVector::Clear() {
    const size_t NUM_FIELDS = NumFields();
    for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = FIELD_0;
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
    }

    else {  // We must change the number of bitfields.  Resize & copy old info.
      Ptr<field_t> old_bits = bits;                                       // Backup old ptr.
      if (num_bits > 0) bits = NewArrayPtr<field_t>(NUM_FIELDS);          // Allocate new mem.
      else bits = nullptr;                                                // (or null if no bits)
      const size_t min_fields = std::min(old_num_fields, NUM_FIELDS);     // Calc num fields to copy
      for (size_t i = 0; i < min_fields; i++) bits[i] = old_bits[i];      // Copy fields
      for (size_t i = min_fields; i < NUM_FIELDS; i++) bits[i] = FIELD_0; // Zero any excess fields
      if (old_bits) old_bits.DeleteArray();                               // Cleanup old memory
    }

    ClearExcessBits();     // If there are ones past the end, zero them out.

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
  BitVector & BitVector::ChooseRandom(Random & random, const int target_ones,
                                      const size_t start_pos, size_t stop_pos) {
    if (stop_pos == MAX_BITS) stop_pos = num_bits;

    emp_assert(start_pos <= stop_pos);
    emp_assert(stop_pos <= num_bits);

    const size_t target_size = stop_pos - start_pos;
    emp_assert(target_ones >= 0);
    emp_assert(target_ones <= (int) target_size);

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
    while (cur_ones < (size_t) target_ones) {
      size_t pos = random.GetUInt(start_pos, stop_pos);
      auto bit = operator[](pos);
      if (!bit) {
        bit.Set();
        cur_ones++;
      }
    }

    // See if we have too many ones.
    while (cur_ones > (size_t) target_ones) {
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
  BitVector & BitVector::FlipRandomCount(Random & random, const size_t target_bits)
  {
    emp_assert(num_bits <= num_bits);
    BitVector choice(num_bits, random, target_bits);
    return XOR_SELF(choice);
  }

  /// Set a specified number of random bits (does not check if already set.)
  BitVector & BitVector::SetRandomCount(Random & random, const size_t target_bits)
  {
    emp_assert(num_bits <= num_bits);
    BitVector choice(num_bits, random, target_bits);
    return OR_SELF(choice);
  }

  /// Unset  a specified number of random bits (does not check if already zero.)
  BitVector & BitVector::ClearRandomCount(Random & random, const size_t target_bits)
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

  /// Get the overall value of this BitSet, using a uint encoding, but including all bits
  /// and returning the value as a double.
  double BitVector::GetValue() const {
    const int max_one = FindMaxOne();

    // If there are no ones, this value must be 0.
    if (max_one == -1) return 0.0;

    // If all ones are in the least-significant field, just return it.
    if (max_one < 64) return (double) GetUInt64(0);

    // To grab the most significant field, figure out how much to shift it by.
    const int shift_bits = max_one - 63;
    double out_value = (double) (*this >> shift_bits).GetUInt64(0);

    out_value *= emp::Pow2(shift_bits);

    return out_value;
  }

  /// Get specified type at a given index (in steps of that type size)
  template <typename T>
  T BitVector::GetValueAtIndex(const size_t index) const {
    // For the moment, must fit inside bounds; eventually should pad with zeros.
    emp_assert((index + 1) * sizeof(T) <= TotalBytes());

    T out_value;
    std::memcpy( &out_value, BytePtr().Raw() + index * sizeof(T), sizeof(T) );
    return out_value;
  }


  /// Set specified type at a given index (in steps of that type size)
  template <typename T>
  void BitVector::SetValueAtIndex(const size_t index, T in_value) {
    // For the moment, must fit inside bounds; eventually should pad with zeros.
    emp_assert((index + 1) * sizeof(T) <= TotalBytes());

    std::memcpy( BytePtr().Raw() + index * sizeof(T), &in_value, sizeof(T) );

    ClearExcessBits();
  }


  /// Get the specified type starting from a given BIT position.
  template <typename T>
  T BitVector::GetValueAtBit(const size_t index) const {
    // For the moment, must fit inside bounds; eventually should pad with zeros.
    emp_assert((index+7)/8 + sizeof(T) < TotalBytes());

    BitVector out_bits(*this);
    out_bits >>= index;

    return out_bits.template GetValueAtIndex<T>(0);
  }


  /// Set the specified type starting from a given BIT position.
  // @CAO: Can be optimized substantially, especially for long BitVectors.
  template <typename T>
  void BitVector::SetValueAtBit(const size_t index, T value) {
    // For the moment, must fit inside bounds; eventually should (?) pad with zeros.
    emp_assert((index+7)/8 + sizeof(T) < TotalBytes());
    constexpr size_t type_bits = sizeof(T) * 8;

    const size_t end_pos = Min(index+type_bits, num_bits);
    Clear(index, end_pos);               // Clear out the bits where new value will go.
    BitVector in_bits(GetSize());        // Setup a bitset for the new bits.
    in_bits.SetValueAtIndex(0, value);   // Insert the new bits.
    in_bits <<= index;                   // Shift new bits into place.
    OR_SELF(in_bits);                    // Place new bits into current BitVector.

    ClearExcessBits();
  }


  // -------------------------  Other Analyses -------------------------

  /// A simple hash function for bit vectors.
  std::size_t BitVector::Hash(size_t start_field) const {
    static_assert(std::is_same_v<field_t, size_t>, "Hash() requires fields to be size_t");

    // If there are no fields left, hash on size one.
    if (start_field == NumFields()) return num_bits;

    // If we have only one field left, combine it with size.
    if (start_field == NumFields()-1) return hash_combine(bits[start_field], num_bits);

    // Otherwise we have more than one field.  Combine and recurse.
    size_t partial_hash = hash_combine(bits[start_field], bits[start_field+1]);

    return hash_combine(partial_hash, Hash(start_field+2));
  }

  // TODO: see https://arxiv.org/pdf/1611.07612.pdf for fast pop counts
  /// Count the number of ones in the BitVector.
  size_t BitVector::CountOnes() const {
    if (num_bits == 0) return 0;
    const field_t NUM_FIELDS = NumFields();
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

  /// Pop the last bit in the vector.
  /// @return value of the popped bit.
  bool BitVector::PopBack() {
    const bool val = Get(num_bits-1);
    Resize(num_bits - 1);
    return val;
  }

  /// Push given bit(s) onto the back of a vector.
  /// @param bit value of bit to be pushed.
  /// @param num number of bits to be pushed.
  void BitVector::PushBack(const bool bit, const size_t num) {
    Resize(num_bits + num);
    if (bit) SetRange(num_bits-num, num_bits);
  }

  /// Insert bit(s) into any index of vector using bit magic.
  /// Blog post on implementation reasoning: https://devolab.org/?p=2249
  /// @param index location to insert bit(s).
  /// @param val value of bit(s) to insert (default true)
  /// @param num number of bits to insert, default 1.
  void BitVector::Insert(const size_t index, const bool val, const size_t num) {
    Resize(num_bits + num);                 // Adjust to new number of bits.
    BitVector low_bits(*this);              // Copy current bits
    SHIFT_SELF(-(int)num);                  // Shift the high bits into place.
    Clear(0, index+num);                    // Reduce current to just high bits.
    low_bits.Clear(index, num_bits);        // Reduce copy to just low bits.
    if (val) SetRange(index, index+num);    // If new bits should be ones, make it so.
    OR_SELF(low_bits);                      // Put the low bits back in place.
  }


  /// Delete bits from any index in a vector.
  /// @param index location to delete bit(s).
  /// @param num number of bits to delete, default 1.
  void BitVector::Delete(const size_t index, const size_t num) {
    emp_assert(index+num <= GetSize());   // Make sure bits to delete actually exist!
    RawCopy(index+num, num_bits, index);  // Shift positions AFTER delete into place.
    Resize(num_bits - num);               // Crop off end bits.
  }

  /// Return the position of the first one; return -1 if no ones in vector.
  int BitVector::FindOne() const {
    const size_t NUM_FIELDS = NumFields();
    size_t field_id = 0;
    while (field_id < NUM_FIELDS && bits[field_id]==0) field_id++;
    return (field_id < NUM_FIELDS) ?
      (int) (find_bit(bits[field_id]) + (field_id * FIELD_BITS))  :  -1;
  }

  /// Return the position of the first one after start_pos; return -1 if no ones in vector.
  /// You can loop through all 1-bit positions of a BitVector "bv" with:
  ///
  ///   for (int pos = bv.FindOne(); pos >= 0; pos = bv.FindOne(pos+1)) { ... }

  int BitVector::FindOne(const size_t start_pos) const {
    if (start_pos >= num_bits) return -1;            // If we're past the end, return fail.
    size_t field_id  = FieldID(start_pos);           // What field do we start in?
    const size_t field_pos = FieldPos(start_pos);    // What position in that field?

    // If there's a hit in a partial first field, return it.
    if (field_pos && (bits[field_id] & ~(MaskLow<field_t>(field_pos)))) {
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

  /// Find the most-significant set-bit.
  int BitVector::FindMaxOne() const {
    // Find the max field with a one.
    int max_field = NumFields() - 1;
    while (max_field >= 0 && bits[max_field] == 0) max_field--;

    // If there are no ones, return -1.
    if (max_field == -1) return -1;

    const field_t field = bits[max_field]; // Save a local copy of this field.
    field_t mask = (field_t) -1;           // Mask off the bits still under consideration.
    size_t offset = 0;                     // Indicate where the mask should be applied.
    size_t range = FIELD_BITS;             // Indicate how many bits are in the mask.

    while (range > 1) {
      // Cut the range in half and see if we need to adjust the offset.
      range /= 2;      // Cut range size in half
      mask >>= range;  // Cut the mask down.

      // Check the upper half of original range; if has a one shift new offset to there.
      if (field & (mask << (offset + range))) offset += range;
    }

    return (int) (max_field * FIELD_BITS + offset);
  }

  /// Return the position of the first one and change it to a zero.  Return -1 if no ones.
  int BitVector::PopOne() {
    const int out_bit = FindOne();
    if (out_bit >= 0) Clear((size_t) out_bit);
    return out_bit;
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

  /// Find the length of the longest continuous series of ones.
  size_t BitVector::LongestSegmentOnes() const {
    size_t length = 0;
    BitVector test_bits(*this);
    while(test_bits.Any()){
      ++length;
      test_bits.AND_SELF(test_bits<<1);
    }
    return length;
  }

  /// Return true if any ones are in common with another BitVector.
  bool BitVector::HasOverlap(const BitVector & in) const {
    const size_t num_fields = std::min(NumFields(), in.NumFields());
    for (size_t i = 0; i < num_fields; ++i) {
      // Short-circuit if we find any overlap.
      if (bits[i] & in.bits[i]) return true;
    }
    return false;
  }


  // -------------------------  Printing and string conversion -------------------------

  /// Convert this BitVector to a vector string [0 index on left]
  std::string BitVector::ToString() const {
    std::string out_string;
    out_string.reserve(num_bits);
    for (size_t i = 0; i < num_bits; ++i) out_string.push_back(GetAsChar(i));
    return out_string;
  }

  /// Convert this BitVector to a numerical string [0 index on right]
  std::string BitVector::ToBinaryString() const {
    std::string out_string;
    out_string.reserve(num_bits);
    for (size_t i = num_bits; i > 0; --i) out_string.push_back(GetAsChar(i-1));
    return out_string;
  }

  /// Convert this BitVector to a series of IDs
  std::string BitVector::ToIDString(const std::string & spacer) const {
    std::stringstream ss;
    PrintOneIDs(ss, spacer);
    return ss.str();
  }

  /// Convert this BitVector to a series of IDs with ranges condensed.
  std::string BitVector::ToRangeString(const std::string & spacer,
                                       const std::string & ranger) const
  {
    std::stringstream ss;
    PrintAsRange(ss, spacer, ranger);
    return ss.str();
  }

  /// Print a space between each field (or other provided spacer)
  void BitVector::PrintFields(std::ostream & out, const std::string & spacer) const {
    for (size_t i = num_bits-1; i < num_bits; i--) {
      out << Get(i);
      if (i && (i % FIELD_BITS == 0)) out << spacer;
    }
  }

  /// Print a space between each field (or other provided spacer)
  void BitVector::PrintDebug(std::ostream & out) const {
    for (size_t field = 0; field < NumFields(); field++) {
      for (size_t bit_id = 0; bit_id < FIELD_BITS; bit_id++) {
        bool bit = (FIELD_1 << bit_id) & bits[field];
        out << ( bit ? 1 : 0 );
      }
      out << " : " << field << std::endl;
    }
    size_t end_pos = NumEndBits();
    if (end_pos == 0) end_pos = FIELD_BITS;
    for (size_t i = 0; i < end_pos; i++) out << " ";
    out << "^" << std::endl;
  }

  /// Print the positions of all one bits, spaces are the default separator.
  void BitVector::PrintOneIDs(std::ostream & out, const std::string & spacer) const {
    bool started = false;
    for (size_t i = 0; i < num_bits; i++) {
      if (Get(i)) {
        if (started) out << spacer;
        out << i;
        started = true;
      }
    }
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


  // -------------------------  Base Boolean-logic operations -------------------------

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

  /// Reverse the order of bits in the bitset
  BitVector & BitVector::REVERSE_SELF() {
    // reverse bytes
    std::reverse( BytePtr().Raw(), BytePtr().Raw() + NumBytes() );

    // reverse each byte
    // adapted from https://stackoverflow.com/questions/2602823/in-c-c-whats-the-simplest-way-to-reverse-the-order-of-bits-in-a-byte
    for (size_t i = 0; i < NumBytes(); ++i) {
      unsigned char & b = BytePtr()[i];
      b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
      b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
      b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    }

    // shift out filler bits
    size_t filler_bits = num_bits % 8;
    if (filler_bits) {
      this->ShiftRight(8-filler_bits);
    }

    return *this;

  }

  /// Reverse order of bits in the bitset.
  BitVector BitVector::REVERSE() const {
    BitVector out_set(*this);
    return out_set.REVERSE_SELF();
  }


  /// Positive rotates go left and negative rotates go left (0 does nothing);
  /// return result.
  BitVector BitVector::ROTATE(const int rotate_size) const {
    BitVector out_set(*this);
    if (rotate_size > 0) out_set.RotateRight((field_t) rotate_size);
    else if (rotate_size < 0) out_set.RotateLeft((field_t) (-rotate_size));
    return out_set;
  }

  /// Positive rotates go right and negative rotates go left (0 does nothing);
  /// store result here, and return this object.
  BitVector & BitVector::ROTATE_SELF(const int rotate_size) {
    if (rotate_size > 0) RotateRight((field_t) rotate_size);
    else if (rotate_size < 0) RotateLeft((field_t) -rotate_size);
    return *this;
  }

  /// Helper: call ROTATE with negative number instead
  template<size_t shift_size_raw>
  BitVector & BitVector::ROTL_SELF() {
    constexpr size_t shift_size = shift_size_raw % num_bits;
    const size_t NUM_FIELDS = NumFields();
    const size_t LAST_FIELD = LastField();

    // special case: for exactly one field_t, try to go low level
    // adapted from https://stackoverflow.com/questions/776508/best-practices-for-circular-shift-rotate-operations-in-c
    if (NUM_FIELDS == 1) {
      field_t & n = bits[0];
      size_t c = shift_size;

      // mask necessary to suprress shift count overflow warnings
      c &= FIELD_LOG2_MASK;
      n = (n<<c) | (n>>( (-(c+FIELD_BITS-num_bits)) & FIELD_LOG2_MASK ));

    } else {

      // note that we already modded shift_size by num_bits
      // so there's no need to mod by FIELD_SIZE here
      int field_shift = NumEndBits() ? (
        (shift_size + FIELD_BITS - NumEndBits()) / FIELD_BITS
      ) : (
        shift_size / FIELD_BITS
      );
      // if we field shift, we need to shift bits by (FIELD_BITS - NumEndBits())
      // more to account for the filler that gets pulled out of the middle
      int bit_shift = NumEndBits() && field_shift ? (
        (shift_size + FIELD_BITS - NumEndBits()) % FIELD_BITS
      ) : (
        shift_size % FIELD_BITS
      );
      int bit_overflow = FIELD_BITS - bit_shift;

      // if rotating more than field capacity, we need to rotate fields
      if (field_shift) {
        auto field_span = FieldSpan();
        std::rotate(
          field_span.rbegin(),
          field_span.rbegin()+field_shift,
          field_span.rend()
        );
      }

      // if necessary, shift filler bits out of the middle
      if (NumEndBits()) {
        const int filler_idx = (LAST_FIELD + field_shift) % NUM_FIELDS;
        for (int i = filler_idx + 1; i < (int)NUM_FIELDS; ++i) {
          bits[i-1] |= bits[i] << NumEndBits();
          bits[i] >>= (FIELD_BITS - NumEndBits());
        }
      }

      // account for bit_shift
      if (bit_shift) {

        const field_t keystone = NumEndBits() ? (
          (bits[LAST_FIELD] << (FIELD_BITS - NumEndBits()))
          | (bits[NUM_FIELDS - 2] >> NumEndBits())
        ) : (
          bits[LAST_FIELD]
        );

        for (int i = LAST_FIELD; i > 0; --i) {
          bits[i] <<= bit_shift;
          bits[i] |= (bits[i-1] >> bit_overflow);
        }
        // Handle final field
        bits[0] <<= bit_shift;
        bits[0] |= keystone >> bit_overflow;

      }

    }

    ClearExcessBits();

    return *this;

  }


  /// Helper for calling ROTATE with positive number
  template<size_t shift_size_raw>
  BitVector & BitVector::ROTR_SELF() {
    const size_t shift_size = shift_size_raw % num_bits;
    const size_t NUM_FIELDS = NumFields();
    const size_t LAST_FIELD = LastField();

    // special case: for exactly one field_t, try to go low level
    // adapted from https://stackoverflow.com/questions/776508/best-practices-for-circular-shift-rotate-operations-in-c
    if (NUM_FIELDS == 1) {
      field_t & n = bits[0];
      size_t c = shift_size;

      // mask necessary to suprress shift count overflow warnings
      c &= FIELD_LOG2_MASK;
      n = (n>>c) | (n<<( (num_bits-c) & FIELD_LOG2_MASK ));

    } else {

      field_t field_shift = (shift_size / FIELD_BITS) % NUM_FIELDS;
      int bit_shift = shift_size % FIELD_BITS;
      field_t bit_overflow = FIELD_BITS - bit_shift;

      // if rotating more than field capacity, we need to rotate fields
      if (field_shift) {
        auto field_span = FieldSpan();
        std::rotate(
          field_span.begin(),
          field_span.begin()+field_shift,
          field_span.end()
        );
      }

      // if necessary, shift filler bits out of the middle
      if (NumEndBits()) {
        int filler_idx = LAST_FIELD - field_shift;
        for (int i = filler_idx + 1; i < (int)NUM_FIELDS; ++i) {
          bits[i-1] |= bits[i] << NumEndBits();
          bits[i] >>= (FIELD_BITS - NumEndBits());
        }
      }

      // account for bit_shift
      if (bit_shift) {

        const field_t keystone = NumEndBits() ? (
          bits[0] >> (FIELD_BITS - NumEndBits())
        ) : (
          bits[0]
        );

        if (NumEndBits()) {
          bits[LastField()] |= bits[0] << NumEndBits();
        }

        for (size_t i = 0; i < LAST_FIELD; ++i) {
          bits[i] >>= bit_shift;
          bits[i] |= (bits[i+1] << bit_overflow);
        }
        bits[LAST_FIELD] >>= bit_shift;
        bits[LAST_FIELD] |= keystone << bit_overflow;
      }
    }

    ClearExcessBits();

    return *this;

  }

  /// Addition of two Bitsets.
  /// Wraps if it overflows.
  /// Returns result.
  BitVector BitVector::ADD(const BitVector & set2) const{
    BitVector out_set(*this);
    return out_set.ADD_SELF(set2);
  }

  /// Addition of two Bitsets.
  /// Wraps if it overflows.
  /// Returns this object.
  BitVector & BitVector::ADD_SELF(const BitVector & set2) {
    bool carry = false;

    for (size_t i = 0; i < num_bits/FIELD_BITS; ++i) {
      field_t addend = set2.bits[i] + static_cast<field_t>(carry);
      carry = set2.bits[i] > addend;

      field_t sum = bits[i] + addend;
      carry |= bits[i] > sum;

      bits[i] = sum;
    }

    if (NumEndBits()) {
      bits[num_bits/FIELD_BITS] = (
        bits[num_bits/FIELD_BITS]
        + set2.bits[num_bits/FIELD_BITS]
        + static_cast<field_t>(carry)
      ) & EndMask();
    }

    return *this;
  }

  /// Subtraction of two Bitsets.
  /// Wraps around if it underflows.
  /// Returns result.
  BitVector BitVector::SUB(const BitVector & set2) const{
    BitVector out_set(*this);
    return out_set.SUB_SELF(set2);
  }

  /// Subtraction of two Bitsets.
  /// Wraps if it underflows.
  /// Returns this object.
  BitVector & BitVector::SUB_SELF(const BitVector & set2){

    bool carry = false;

    for (size_t i = 0; i < num_bits/FIELD_BITS; ++i) {
      field_t subtrahend = set2.bits[i] + static_cast<field_t>(carry);
      carry = set2.bits[i] > subtrahend;
      carry |= bits[i] < subtrahend;
      bits[i] -= subtrahend;
    }

    if (NumEndBits()) {
      bits[num_bits/FIELD_BITS] = (
        bits[num_bits/FIELD_BITS]
        - set2.bits[num_bits/FIELD_BITS]
        - static_cast<field_t>(carry)
      ) & EndMask();
    }

    return *this;
  }

}


// ---------------------- Implementations to work with standard library ----------------------

namespace std {
  /// Hash function to allow BitVector to be used with maps and sets (must be in std).
  template <>
  struct hash<emp::BitVector> {
    std::size_t operator()(const emp::BitVector & bv) const {
      return bv.Hash();
    }
  };
}

#endif // #ifndef EMP_BITS_BITVECTOR_HPP_INCLUDE
