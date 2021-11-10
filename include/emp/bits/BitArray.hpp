/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file BitArray.hpp
 *  @brief An Array of a fixed number of bits; similar to std::bitset, but with extra bit magic.
 *  @note Status: RELEASE
 *
 *  @todo Some of the functions allow a start bit and end bit; each of these should be checked
 *        to make sure that they will work if the start and end are part of the same byte.  One
 *        option is to do this well ONCE with a macro that properly fills in the details.
 */

#ifndef EMP_BITS_BITARRAY_HPP_INCLUDE
#define EMP_BITS_BITARRAY_HPP_INCLUDE


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
#include "../meta/type_traits.hpp"
#include "../polyfill/span.hpp"

#include "_bitset_helpers.hpp"
#include "bitset_utils.hpp"

namespace emp {

  ///  A fixed-sized (but arbitrarily large) array of bits, and optimizes operations on those bits
  ///  to be as fast as possible.
  ///  @param NUM_BITS is the fixed number of bits in this BitArray.
  ///  @param ZERO_LEFT indicates the side that bit zero will be located.
  template <size_t NUM_BITS, bool ZERO_LEFT=true>
  class BitArray {

    // make all templated instantiations friends with each other
    template <size_t FRIEND_BITS, bool FRIEND_LEFT> friend class BitArray;

  private:
    using this_t = BitArray<NUM_BITS, ZERO_LEFT>;

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

    field_t bits[NUM_FIELDS];  ///< Fields to hold the actual bits for this BitArray.

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
    BitArray & Copy(const field_t in_bits[IN_FIELDS]) noexcept {
      static_assert(COPY_FIELDS <= IN_FIELDS, "Cannot copy more fields than we are given.");
      static_assert(COPY_FIELDS <= NUM_FIELDS, "Cannot copy into more fields than are available.");
      constexpr size_t COPY_BYTES = COPY_FIELDS * sizeof(field_t);
      std::memcpy(bits, in_bits, COPY_BYTES);
      return *this;
    }

    // Any bits past the last "real" bit in the last field should be kept as zeros.
    void ClearExcessBits() noexcept { if constexpr (NUM_END_BITS > 0) bits[LAST_FIELD] &= END_MASK; }

    // Apply a transformation to each bit field in a specified range.
    template <typename FUN_T>
    inline BitArray & ApplyRange(const FUN_T & fun, size_t start, size_t stop);

    // Convert the bits to const bytes.
    [[nodiscard]] emp::Ptr<const unsigned char> BytePtr() const
    { return reinterpret_cast<const unsigned char*>(bits); }

    // Convert the bits to bytes.
    [[nodiscard]] emp::Ptr<unsigned char> BytePtr()
    { return reinterpret_cast<unsigned char*>(bits); }

    /// Helper: call SHIFT with positive number instead
    void ShiftLeft(const size_t shift_size);

    /// Helper for calling SHIFT with negative number
    void ShiftRight(const size_t shift_size);

    /// Helper: call ROTATE with negative number instead
    void RotateLeft(const size_t shift_size_raw);

    /// Helper for calling ROTATE with positive number
    void RotateRight(const size_t shift_size_raw);

  public:
    /// Constructor: Assume all bits set to zero.
    explicit BitArray(bool init_val=false) noexcept { if (init_val) SetAll(); else Clear(); }

    /// Copy constructor from another BitArray
    BitArray(const this_t & _in) noexcept { Copy<NUM_FIELDS>(_in.bits); }

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

    /// Constructor to fill in a bit array from a vector.
    template <typename T> BitArray(const std::initializer_list<T> l);

    /// Destructor.
    ~BitArray() = default;

    /// Assignment operator (no separate move opperator since no resources to move...)
    BitArray & operator=(const this_t & in_bits) noexcept { return Copy<NUM_FIELDS>(in_bits.bits); }

    /// Assignment operator from a std::bitset.
    BitArray & operator=(const std::bitset<NUM_BITS> & bitset);

    /// Assignment operator from a string of '0's and '1's.
    BitArray & operator=(const std::string & bitstring);

    /// Assignment operator from a literal string of '0's and '1's.
    BitArray & operator=(const char * bitstring) { return operator=(std::string(bitstring)); }

    /// Assignment from another BitArray of a different size.
    template <size_t FROM_BITS, bool FROM_LEFT>
    BitArray & Import( const BitArray<FROM_BITS,FROM_LEFT> & from_bits, const size_t from_bit=0 );

    /// Convert to a BitArray of a different size.
    template <size_t TO_BITS, bool TO_LEFT=ZERO_LEFT>
    BitArray<TO_BITS,TO_LEFT> Export(size_t start_bit=0) const;

    /// For debugging: make sure that there are no obvous problems with a BitArray object.
    bool OK() const;

    /// How many bits are in this BitArray?
    [[nodiscard]] constexpr static size_t GetSize() { return NUM_BITS; }

    /// How many bytes are in this BitArray?
    [[nodiscard]] constexpr static size_t GetNumBytes() { return TOTAL_BYTES; }

    /// How many distinct values could be held in this BitArray?
    [[nodiscard]] static constexpr double GetNumStates() { return emp::Pow2(NUM_BITS); }

    /// Retrieve the bit as a specified index.
    [[nodiscard]] bool Get(size_t index) const;

    /// A safe version of Get() for indexing out of range. Useful for representing collections.
    [[nodiscard]] bool Has(size_t index) const { return (index < NUM_BITS) ? Get(index) : false; }

    /// Set the bit at a specified index.
    BitArray & Set(size_t index, bool value=true);

    /// Set all bits to one.
    BitArray & SetAll() noexcept;

    /// Set a range of bits to one: [start, stop)
    BitArray & SetRange(size_t start, size_t stop)
      { return ApplyRange([](field_t){ return FIELD_ALL; }, start, stop); }

    /// Set all bits to zero.
    BitArray & Clear() noexcept { for (field_t & x : bits) x = FIELD_0; return *this; }

    /// Set specific bit to 0.
    BitArray & Clear(size_t index) { return Set(index, false); }

    /// Set bits to 0 in the range [start, stop)
    BitArray & Clear(const size_t start, const size_t stop)
      { return ApplyRange([](field_t){ return 0; }, start, stop); }

    /// Index into a const BitArray (i.e., cannot be set this way.)
    bool operator[](size_t index) const { return Get(index); }

    /// Index into a BitArray, returning a proxy that will allow bit assignment to work.
    BitProxy<this_t> operator[](size_t index) { return BitProxy<this_t>(*this, index); }

    /// Flip all bits in this BitArray
    BitArray & Toggle() { return NOT_SELF(); }

    /// Flip a single bit
    BitArray & Toggle(size_t index);

    /// Flips all the bits in a range [start, stop)
    BitArray & Toggle(size_t start, size_t stop)
      { return ApplyRange([](field_t x){ return ~x; }, start, stop); }

    /// Return true if ANY bits in the BitArray are one, else return false.
    [[nodiscard]] bool Any() const { for (auto i : bits) if (i) return true; return false; }

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

    // =========  Comparison Operators  ========== //

    template <size_t T2, bool L2>
    [[nodiscard]] bool operator==(const BitArray<T2,L2> & in) const;

    template <size_t T2, bool L2>
    [[nodiscard]] bool operator!=(const BitArray<T2,L2> & in) const { return !(*this == in); }

    template <size_t T2, bool L2>
    [[nodiscard]] bool operator< (const BitArray<T2,L2> & in) const;

    template <size_t T2, bool L2>
    [[nodiscard]] bool operator> (const BitArray<T2,L2> & in) const { return in < *this; }

    template <size_t T2, bool L2>
    [[nodiscard]] bool operator<=(const BitArray<T2,L2> & in) const { return !(in < *this); }

    template <size_t T2, bool L2>
    [[nodiscard]] bool operator>=(const BitArray<T2,L2> & in) const { return !(*this < in); }

    /// Casting a BitArray to bool identifies if ANY bits are set to 1.
    explicit operator bool() const { return Any(); }


    // =========  Access Groups of bits  ========= //

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


    // =========  Other Analyses  ========= //

    /// A simple hash function for bit vectors.
    [[nodiscard]] std::size_t Hash() const noexcept;

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


    // =========  Print/String Functions  ========= //

    /// Convert a specified bit to a character.
    [[nodiscard]] char GetAsChar(size_t id) const { return Get(id) ? '1' : '0'; }

    /// Convert this BitArray to a string.
    [[nodiscard]] std::string ToString() const;

    /// Convert this BitArray to an array-based string [index 0 on left]
    [[nodiscard]] std::string ToArrayString() const;

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
    BitArray & AND_SELF(const BitArray & array2);

    /// Perform a Boolean OR with a second BitArray, store result here, and return this object.
    BitArray & OR_SELF(const BitArray & array2);

    /// Perform a Boolean NAND with a second BitArray, store result here, and return this object.
    BitArray & NAND_SELF(const BitArray & array2);

    /// Perform a Boolean NOR with a second BitArray, store result here, and return this object.
    BitArray & NOR_SELF(const BitArray & array2);

    /// Perform a Boolean XOR with a second BitArray, store result here, and return this object.
    BitArray & XOR_SELF(const BitArray & array2);

    /// Perform a Boolean EQU with a second BitArray, store result here, and return this object.
    BitArray & EQU_SELF(const BitArray & array2);

    /// Perform a Boolean NOT on this BitArray and return the result.
    [[nodiscard]] BitArray NOT() const { return this_t(*this).NOT_SELF(); }

    /// Perform a Boolean AND with a second BitArray and return the result.
    [[nodiscard]] BitArray AND(const BitArray & in) const { return this_t(*this).AND_SELF(in); }

    /// Perform a Boolean OR with a second BitArray and return the result.
    [[nodiscard]] BitArray OR(const BitArray & in) const { return this_t(*this).OR_SELF(in); }

    /// Perform a Boolean NAND with a second BitArray and return the result.
    [[nodiscard]] BitArray NAND(const BitArray & in) const { return this_t(*this).NAND_SELF(in); }

    /// Perform a Boolean NOR with a second BitArray and return the result.
    [[nodiscard]] BitArray NOR(const BitArray & in) const { return this_t(*this).NOR_SELF(in); }

    /// Perform a Boolean XOR with a second BitArray and return the result.
    [[nodiscard]] BitArray XOR(const BitArray & in) const { return this_t(*this).XOR_SELF(in); }

    /// Perform a Boolean EQU with a second BitArray and return the result.
    BitArray EQU(const BitArray & in) const { return this_t(*this).EQU_SELF(in); }

    /// Positive shifts go right and negative shifts go left (0 does nothing);
    /// return result.
    [[nodiscard]] BitArray SHIFT(const int shift_size) const;

    /// Positive shifts go right and negative shifts go left (0 does nothing);
    /// store result here, and return this object.
    BitArray & SHIFT_SELF(const int shift_size);

    /// Reverse the order of bits in the BitArray
    BitArray & REVERSE_SELF();

    /// Reverse order of bits in the BitArray.
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

    /// Addition of two BitArrays.
    /// Wraps if it overflows.
    /// Returns result.
    [[nodiscard]] BitArray ADD(const BitArray & array2) const;

    /// Addition of two BitArrays.
    /// Wraps if it overflows.
    /// Returns this object.
    BitArray & ADD_SELF(const BitArray & array2);

    /// Subtraction of two BitArrays.
    /// Wraps around if it underflows.
    /// Returns result.
    [[nodiscard]] BitArray SUB(const BitArray & array2) const;

    /// Subtraction of two BitArrays.
    /// Wraps if it underflows.
    /// Returns this object.
    BitArray & SUB_SELF(const BitArray & array2);

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
      ar( bits );
    }

  };

  // ------------------------ Implementations for Internal Functions ------------------------

  template <size_t NUM_BITS, bool ZERO_LEFT>
  template <typename FUN_T>
  BitArray<NUM_BITS,ZERO_LEFT> &
  BitArray<NUM_BITS,ZERO_LEFT>::ApplyRange(const FUN_T & fun, size_t start, size_t stop) {
    if (start == stop) return *this;  // Empty range.

    emp_assert(start <= stop, start, stop, NUM_BITS);  // Start cannot be after stop.
    emp_assert(stop <= NUM_BITS, stop, NUM_BITS);      // Stop must be in range.

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


  template <size_t NUM_BITS, bool ZERO_LEFT>
  void BitArray<NUM_BITS,ZERO_LEFT>::ShiftLeft(const size_t shift_size) {
    // If we have only a single field, this operation can be quick.
    if constexpr (NUM_FIELDS == 1) {
      bits[0] <<= shift_size;
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
        bits[i] = bits[i - field_shift];
      }
      for (size_t i = field_shift; i > 0; i--) bits[i-1] = 0;
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


  /// Helper for calling SHIFT with negative number
  template <size_t NUM_BITS, bool ZERO_LEFT>
  void BitArray<NUM_BITS,ZERO_LEFT>::ShiftRight(const size_t shift_size) {
    // If we have only a single field, this operation can be quick.
    if constexpr (NUM_FIELDS == 1) {
      bits[0] >>= shift_size;
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
        bits[i] = bits[i + field_shift];
      }
      for (size_t i = NUM_FIELDS - field_shift; i < NUM_FIELDS; i++) bits[i] = 0;
    }

    // account for bit_shift
    if (bit_shift) {
      for (size_t i = 0; i < (LAST_FIELD - field_shift); ++i) {
        bits[i] >>= bit_shift;
        bits[i] |= (bits[i+1] << bit_overflow);
      }
      bits[LAST_FIELD - field_shift] >>= bit_shift;
    }
  }

  /// Helper: call ROTATE with negative number
  template <size_t NUM_BITS, bool ZERO_LEFT>
  void BitArray<NUM_BITS,ZERO_LEFT>::RotateLeft(const size_t shift_size_raw) {
    const field_t shift_size = shift_size_raw % NUM_BITS;

    // use different approaches based on BitArray size
    if constexpr (NUM_FIELDS == 1) {
      // special case: for exactly one field_T, try to go low level
      // adapted from https://stackoverflow.com/questions/776508/best-practices-for-circular-shift-rotate-operations-in-c
      field_t & n = bits[0];
      size_t c = shift_size;

      // mask necessary to suprress shift count overflow warnings
      c &= FIELD_LOG2_MASK;
      n = (n<<c) | (n>>( (-(c+FIELD_BITS-NUM_BITS)) & FIELD_LOG2_MASK ));

    } else if constexpr (NUM_FIELDS < 32) {
      // for small BitArrays, shifting L/R and ORing is faster
      this_t dup(*this);
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
        std::rbegin(bits),
        std::rbegin(bits)+field_shift,
        std::rend(bits)
      );

      // if necessary, shift filler bits out of the middle
      if constexpr ((bool)NUM_END_BITS) {
        const int filler_idx = (LAST_FIELD + field_shift) % NUM_FIELDS;
        for (int i = filler_idx + 1; i < (int)NUM_FIELDS; ++i) {
          bits[i-1] |= bits[i] << NUM_END_BITS;
          bits[i] >>= (FIELD_BITS - NUM_END_BITS);
        }
      }

      // account for bit_shift
      if (bit_shift) {

        const field_t keystone = NUM_END_BITS ? (
          (bits[LAST_FIELD] << (FIELD_BITS - NUM_END_BITS))
          | (bits[NUM_FIELDS - 2] >> NUM_END_BITS)
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

    // Mask out filler bits
    ClearExcessBits();
  }


  /// Helper for calling ROTATE with positive number
  template <size_t NUM_BITS, bool ZERO_LEFT>
  void BitArray<NUM_BITS,ZERO_LEFT>::RotateRight(const size_t shift_size_raw) {

    const size_t shift_size = shift_size_raw % NUM_BITS;

    // use different approaches based on BitArray size
    if constexpr (NUM_FIELDS == 1) {
      // special case: for exactly one field_t, try to go low level
      // adapted from https://stackoverflow.com/questions/776508/best-practices-for-circular-shift-rotate-operations-in-c

      field_t & n = bits[0];
      size_t c = shift_size;

      // mask necessary to suprress shift count overflow warnings
      c &= FIELD_LOG2_MASK;
      n = (n>>c) | (n<<( (NUM_BITS-c) & FIELD_LOG2_MASK ));

    } else if constexpr (NUM_FIELDS < 32) {
      // for small BitArrays, shifting L/R and ORing is faster
      this_t dup(*this);
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
        std::begin(bits),
        std::begin(bits)+field_shift,
        std::end(bits)
      );

      // if necessary, shift filler bits out of the middle
      if constexpr (NUM_END_BITS > 0) {
        const int filler_idx = LAST_FIELD - field_shift;
        for (int i = filler_idx + 1; i < (int)NUM_FIELDS; ++i) {
          bits[i-1] |= bits[i] << NUM_END_BITS;
          bits[i] >>= (FIELD_BITS - NUM_END_BITS);
        }
      }

      // account for bit_shift
      if (bit_shift) {

        const field_t keystone = NUM_END_BITS ? (
          bits[0] >> (FIELD_BITS - NUM_END_BITS)
        ) : (
          bits[0]
        );

        if constexpr (NUM_END_BITS > 0) {
          bits[NUM_FIELDS-1] |= bits[0] << NUM_END_BITS;
        }

        for (size_t i = 0; i < LAST_FIELD; ++i) {
          bits[i] >>= bit_shift;
          bits[i] |= (bits[i+1] << bit_overflow);
        }
        bits[LAST_FIELD] >>= bit_shift;
        bits[LAST_FIELD] |= keystone << bit_overflow;
      }
    }

    // Mask out filler bits
    ClearExcessBits();
  }

  // -------------------- Longer Constructors and bit copying ---------------------

  /// Constructor to generate a BitArray from a std::bitset.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT>::BitArray(const std::bitset<NUM_BITS> & bitset) {
    for (size_t bit{}; bit < NUM_BITS; ++bit) Set( bit, bitset[bit] );
    ClearExcessBits();
  }

  /// Constructor to generate a BitArray from a string of '0's and '1's.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT>::BitArray(const std::string & bitstring)
  {
    emp_assert(bitstring.size() <= NUM_BITS);
    Clear();
    if constexpr (ZERO_LEFT) {
      for (size_t i = 0; i < bitstring.size(); i++) Set(i, bitstring[i] != '0');
    } else {
      const size_t in_size = bitstring.size();
      for (size_t i = 0; i < in_size; i++) Set(in_size - i - 1, bitstring[i] != '0');
    }
  }

  template <size_t NUM_BITS, bool ZERO_LEFT>
  template <typename T>
  BitArray<NUM_BITS,ZERO_LEFT>::BitArray(const std::initializer_list<T> l) {
    emp_assert(l.size() <= NUM_BITS, "Initializer longer than BitArray", l.size(), NUM_BITS);
    Clear();
    if constexpr (ZERO_LEFT) {
      auto it = std::begin(l); // Left-most bit is position 0.
      for (size_t idx = 0; idx < NUM_BITS; ++idx) Set(idx, (idx < l.size()) && *it++);
    } else {
      auto it = std::rbegin(l); // Right-most bit is position 0.
      for (size_t idx = 0; idx < NUM_BITS; ++idx) Set(idx, (idx < l.size()) && *it++);
    }
  }

    /// Assignment operator from a std::bitset.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> &
  BitArray<NUM_BITS,ZERO_LEFT>::operator=(const std::bitset<NUM_BITS> & bitset) {
    for (size_t i = 0; i < NUM_BITS; i++) Set(i, bitset[i]);
    return *this;
  }

  /// Assignment operator from a string of '0's and '1's.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> &
  BitArray<NUM_BITS,ZERO_LEFT>::operator=(const std::string & bitstring) {
    emp_assert(bitstring.size() <= NUM_BITS);
    Clear();
    if constexpr (ZERO_LEFT) {
      for (size_t i = 0; i < bitstring.size(); i++) Set(i, bitstring[i] != '0');
    } else {
      const size_t in_size = bitstring.size();
      for (size_t i = 0; i < in_size; i++) Set(in_size - i - 1, bitstring[i] != '0');
    }
    return *this;
  }


  /// Assign from a BitArray of a different size.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  template <size_t FROM_BITS, bool FROM_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> & BitArray<NUM_BITS,ZERO_LEFT>::Import(
    const BitArray<FROM_BITS, FROM_LEFT> & from_array,
    const size_t from_bit
  ) {
    // Only check for same-ness if the two types are the same.
    if constexpr (FROM_BITS == NUM_BITS) emp_assert(&from_array != this);

    emp_assert(from_bit < FROM_BITS);

    if (FROM_BITS - from_bit < NUM_BITS) Clear();

    constexpr size_t DEST_BYTES = (NUM_BITS + 7)/8;
    const size_t FROM_BYTES = (FROM_BITS + 7)/8 - from_bit/8;

    const size_t COPY_BYTES = std::min(DEST_BYTES, FROM_BYTES);

    std::memcpy(
      bits,
      reinterpret_cast<const unsigned char*>(from_array.bits) + from_bit/8,
      COPY_BYTES
    );

    if (from_bit%8) {

      this->ShiftRight(from_bit%8);

      if (FROM_BYTES > COPY_BYTES) {
        reinterpret_cast<unsigned char*>(bits)[COPY_BYTES-1] |= (
          reinterpret_cast<const unsigned char*>(
            from_array.bits
          )[from_bit/8 + COPY_BYTES]
          << (8 - from_bit%8)
        );
      }

    }

    ClearExcessBits();

    return *this;

  }

  /// Convert to a BitArray of a different size.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  template <size_t TO_BITS, bool TO_LEFT>
  BitArray<TO_BITS,TO_LEFT> BitArray<NUM_BITS,ZERO_LEFT>::Export(size_t start_bit) const {
    BitArray<TO_BITS,TO_LEFT> out_bits;
    out_bits.Import(*this, start_bit);

    return out_bits;
  }

    /// For debugging: make sure that there are no obvous problems with a BitArray object.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  bool BitArray<NUM_BITS,ZERO_LEFT>::OK() const {
    // Make sure final bits are zeroed out.
    emp_assert((bits[LAST_FIELD] & ~END_MASK) == 0);

    return true;
  }



  // --------------------  Implementations of common accessors -------------------

  template <size_t NUM_BITS, bool ZERO_LEFT>
  bool BitArray<NUM_BITS,ZERO_LEFT>::Get(size_t index) const {
    emp_assert(index >= 0 && index < NUM_BITS);
    const size_t field_id = FieldID(index);
    const size_t pos_id = FieldPos(index);
    return (bits[field_id] & (((field_t)1U) << pos_id)) != 0;
  }

  /// Set the bit at a specified index.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> & BitArray<NUM_BITS,ZERO_LEFT>::Set(size_t index, bool value) {
    emp_assert(index < NUM_BITS);
    const size_t field_id = FieldID(index);
    const size_t pos_id = FieldPos(index);
    const field_t pos_mask = FIELD_1 << pos_id;

    if (value) bits[field_id] |= pos_mask;
    else       bits[field_id] &= ~pos_mask;

    return *this;
  }

  /// Set all bits to one.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> & BitArray<NUM_BITS,ZERO_LEFT>::SetAll() noexcept {
    for (field_t & x : bits) x = FIELD_ALL;
    ClearExcessBits();
    return *this;
  }


  /// Flip a single bit
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> & BitArray<NUM_BITS,ZERO_LEFT>::Toggle(size_t index) {
    emp_assert(index >= 0 && index < NUM_BITS);
    const size_t field_id = FieldID(index);
    const size_t pos_id = FieldPos(index);
    const field_t pos_mask = FIELD_1 << pos_id;

    bits[field_id] ^= pos_mask;

    return *this;
  }



  // -------------------------  Implementations Randomization functions -------------------------

  /// Set all bits randomly, with a 50% probability of being a 0 or 1.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> & BitArray<NUM_BITS,ZERO_LEFT>::Randomize(Random & random) {
    random.RandFill(BytePtr(), TOTAL_BYTES);
    ClearExcessBits();
    return *this;
  }

  /// Set all bits randomly, with probability specified at compile time.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  template <Random::Prob P>
  BitArray<NUM_BITS,ZERO_LEFT> & BitArray<NUM_BITS,ZERO_LEFT>::RandomizeP(Random & random,
                                                  const size_t start_pos, const size_t stop_pos) {
    emp_assert(start_pos <= stop_pos);
    emp_assert(stop_pos <= NUM_BITS);
    random.RandFillP<P>(BytePtr(), TOTAL_BYTES, start_pos, stop_pos);
    ClearExcessBits();
    return *this;
  }


  /// Set all bits randomly, with a given probability of being on.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> & BitArray<NUM_BITS,ZERO_LEFT>::Randomize(Random & random, const double p,
                                                 const size_t start_pos, const size_t stop_pos) {
    emp_assert(start_pos <= stop_pos);
    emp_assert(stop_pos <= NUM_BITS);
    emp_assert(p >= 0.0 && p <= 1.0, p);
    random.RandFill(BytePtr(), TOTAL_BYTES, p, start_pos, stop_pos);
    ClearExcessBits();
     return *this;
  }

  /// Set all bits randomly, with a given number of them being on.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> &
  BitArray<NUM_BITS,ZERO_LEFT>::ChooseRandom(
    Random & random,
    const size_t target_ones,
    const size_t start_pos,
    const size_t stop_pos
  ) {
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
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> & BitArray<NUM_BITS,ZERO_LEFT>::FlipRandom(Random & random,
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
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> & BitArray<NUM_BITS,ZERO_LEFT>::SetRandom(Random & random,
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
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> & BitArray<NUM_BITS,ZERO_LEFT>::ClearRandom(Random & random,
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
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> & BitArray<NUM_BITS,ZERO_LEFT>::FlipRandomCount(Random & random,
                                                       const size_t num_bits)
  {
    emp_assert(num_bits <= NUM_BITS);
    this_t target_bits(random, num_bits);
    return *this ^= target_bits;
  }

  /// Set a specified number of random bits (does not check if already set.)
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> & BitArray<NUM_BITS,ZERO_LEFT>::SetRandomCount(Random & random,
                                                      const size_t num_bits)
  {
    emp_assert(num_bits <= NUM_BITS);
    this_t target_bits(random, num_bits);
    return *this |= target_bits;
  }

  /// Unset  a specified number of random bits (does not check if already zero.)
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> & BitArray<NUM_BITS,ZERO_LEFT>::ClearRandomCount(Random & random,
                                                        const size_t num_bits)
  {
    emp_assert(num_bits <= NUM_BITS);
    this_t target_bits(random, NUM_BITS - num_bits);
    return *this &= target_bits;
  }


  // -------------------------  Implementations of Comparison Operators -------------------------

  /// Test if two BitArray objects are identical.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  template <size_t SIZE2, bool LEFT2>
  bool BitArray<NUM_BITS,ZERO_LEFT>::operator==(const BitArray<SIZE2,LEFT2> & in_bits) const {
    if constexpr (NUM_BITS != SIZE2) return false;

    for (size_t i = 0; i < NUM_FIELDS; ++i) {
      if (bits[i] != in_bits.bits[i]) return false;
    }
    return true;
  }

  /// Compare two BitArray objects, based on the associated binary value.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  template <size_t SIZE2, bool LEFT2>
  bool BitArray<NUM_BITS,ZERO_LEFT>::operator<(const BitArray<SIZE2,LEFT2> & in_bits) const {
    if constexpr (NUM_BITS != SIZE2) return NUM_BITS < SIZE2;

    for (int i = NUM_FIELDS-1; i >= 0; --i) {         // Start loop at the largest field.
      if (bits[i] == in_bits.bits[i]) continue;  // If same, keep looking!
      return (bits[i] < in_bits.bits[i]);        // Otherwise, do comparison
    }
    return false;
  }


  // -------------------------  Access Groups of bits -------------------------

  /// Get the full byte starting from the bit at a specified index.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  uint8_t BitArray<NUM_BITS,ZERO_LEFT>::GetByte(size_t index) const {
    emp_assert(index < TOTAL_BYTES);
    const size_t field_id = Byte2Field(index);
    const size_t pos_id = Byte2FieldPos(index);
    return (bits[field_id] >> pos_id) & 255;
  }


  /// Get a read-only view into the internal array used by BitArray.
  /// @return Read-only span of BitArray's bytes.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  std::span<const std::byte> BitArray<NUM_BITS,ZERO_LEFT>::GetBytes() const {
    return std::span<const std::byte>(
      reinterpret_cast<const std::byte*>(bits),
      TOTAL_BYTES
    );
  }


  /// Set the full byte starting at the bit at the specified index.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  void BitArray<NUM_BITS,ZERO_LEFT>::SetByte(size_t index, uint8_t value) {
    emp_assert(index < TOTAL_BYTES);
    const size_t field_id = Byte2Field(index);
    const size_t pos_id = Byte2FieldPos(index);
    const field_t val_uint = value;
    bits[field_id] = (bits[field_id] & ~(((field_t)255U) << pos_id)) | (val_uint << pos_id);
  }

  /// Get the overall value of this BitArray, using a uint encoding, but including all bits
  /// and returning the value as a double.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  double BitArray<NUM_BITS,ZERO_LEFT>::GetValue() const {
    // If we have 64 bits or fewer, we can load the full value and return it.
    if constexpr (NUM_FIELDS == 1) return (double) bits[0];

    // Otherwise grab the most significant one and figure out how much to shift it by.
    const int max_one = FindMaxOne();

    // If there are no ones, this value must be 0.
    if (max_one == -1) return 0.0;

    // If all ones are in the least-significant field, just return it.
    // NOTE: If we have more than one field, FIELD_SIZE is usually 64 already.
    if (max_one < 64) return (double) GetUInt64(0);

    // To grab the most significant field, figure out how much to shift it by.
    const size_t shift_bits = (size_t) max_one - 63;
    double out_value = (double) (*this >> shift_bits).GetUInt64(0);

    out_value *= emp::Pow2(shift_bits);

    return out_value;
  }

  /// Get specified type at a given index (in steps of that type size)
  template <size_t NUM_BITS, bool ZERO_LEFT>
  template <typename T>
  T BitArray<NUM_BITS,ZERO_LEFT>::GetValueAtIndex(const size_t index) const {
    // For the moment, must fit inside bounds; eventually should pad with zeros.
    emp_assert((index + 1) * sizeof(T) <= NUM_FIELDS * sizeof(field_t),
              index, sizeof(T), NUM_BITS, NUM_FIELDS);

    // If we are using the native field type, just grab it from bits.
    if constexpr( std::is_same<T, field_t>() ) return bits[index];

    T out_value;
    std::memcpy( &out_value, BytePtr().Raw() + index * sizeof(T), sizeof(T) );
    return out_value;
  }


  /// Set specified type at a given index (in steps of that type size)
  template <size_t NUM_BITS, bool ZERO_LEFT>
  template <typename T>
  void BitArray<NUM_BITS,ZERO_LEFT>::SetValueAtIndex(const size_t index, T in_value) {
    // For the moment, must fit inside bounds; eventually should pad with zeros.
    emp_assert((index + 1) * sizeof(T) <= NUM_FIELDS * sizeof(field_t),
              index, sizeof(T), NUM_BITS, NUM_FIELDS);

    std::memcpy( BytePtr().Raw() + index * sizeof(T), &in_value, sizeof(T) );

    ClearExcessBits();
  }


  /// Get the specified type starting from a given BIT position.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  template <typename T>
  T BitArray<NUM_BITS,ZERO_LEFT>::GetValueAtBit(const size_t index) const {
    // For the moment, must fit inside bounds; eventually should pad with zeros.
    emp_assert((index+7)/8 + sizeof(T) < NUM_FIELDS * sizeof(field_t));

    BitArray<sizeof(T)*8> out_bits;
    out_bits.Import(*this, index);

    return out_bits.template GetValueAtIndex<T>(0);
  }


  /// Set the specified type starting from a given BIT position.
  // @CAO: Can be optimized substantially, especially for long BitArrays.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  template <typename T>
  void BitArray<NUM_BITS,ZERO_LEFT>::SetValueAtBit(const size_t index, T value) {
    // For the moment, must fit inside bounds; eventually should pad with zeros.
    emp_assert((index+7)/8 + sizeof(T) < NUM_FIELDS * sizeof(field_t));
    constexpr size_t type_bits = sizeof(T) * 8;

    Clear(index, index+type_bits);       // Clear out the bits where new value will go.
    this_t in_bits;          // Setup a BitArray to place the new bits in.
    in_bits.SetValueAtIndex(0, value);   // Insert the new bits.
    in_bits <<= index;                   // Shift new bits into place.
    OR_SELF(in_bits);                    // Place new bits into current BitArray.

    ClearExcessBits();
  }


  // -------------------------  Other Analyses -------------------------

  /// A simple hash function for bit vectors.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  std::size_t BitArray<NUM_BITS,ZERO_LEFT>::Hash() const noexcept {
    /// If we have a vector of size_t, treat it as a vector of hash values to combine.
    if constexpr (std::is_same_v<field_t, size_t>) {
      return hash_combine(bits, NUM_FIELDS);
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
  template <size_t NUM_BITS, bool ZERO_LEFT>
  size_t BitArray<NUM_BITS,ZERO_LEFT>::CountOnes() const {
    size_t bit_count = 0;
    for (size_t i = 0; i < NUM_FIELDS; ++i) {
        // when compiling with -O3 and -msse4.2, this is the fastest population count method.
        std::bitset<FIELD_BITS> std_bs(bits[i]);
        bit_count += std_bs.count();
      }

    return bit_count;
  }

  /// Faster counting of ones for very sparse bit vectors.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  size_t BitArray<NUM_BITS,ZERO_LEFT>::CountOnes_Sparse() const {
    size_t bit_count = 0;
    for (field_t cur_field : bits) {
      while (cur_field) {
        cur_field &= (cur_field-1);       // Peel off a single 1.
        bit_count++;                      // Increment the counter
      }
    }
    return bit_count;
  }

    /// Return the index of the first one in the sequence; return -1 if no ones are available.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  int BitArray<NUM_BITS,ZERO_LEFT>::FindOne() const {
    size_t field_id = 0;
    while (field_id < NUM_FIELDS && bits[field_id]==0) field_id++;
    return (field_id < NUM_FIELDS) ?
      (int) (find_bit(bits[field_id]) + (field_id << FIELD_LOG2))  :  -1;
  }

  /// Return index of first one in sequence AFTER start_pos (or -1 if no ones)
  template <size_t NUM_BITS, bool ZERO_LEFT>
  int BitArray<NUM_BITS,ZERO_LEFT>::FindOne(const size_t start_pos) const {
    if (start_pos >= NUM_BITS) return -1;            // If we're past the end, return fail.
    size_t field_id  = FieldID(start_pos);           // What field do we start in?
    const size_t field_pos = FieldPos(start_pos);    // What position in that field?

    // If there's a hit in a partial first field, return it.
    if (field_pos && (bits[field_id] & ~(MaskLow<field_t>(field_pos)))) {
      return (int) (find_bit(bits[field_id] & ~(MaskLow<field_t>(field_pos))) +
                    field_id * FIELD_BITS);
    }

    // Search other fields...
    if (field_pos) field_id++;
    while (field_id < NUM_FIELDS && bits[field_id]==0) field_id++;
    return (field_id < NUM_FIELDS) ?
      (int) (find_bit(bits[field_id]) + (field_id * FIELD_BITS)) : -1;
  }

  /// Find the most-significant set-bit.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  int BitArray<NUM_BITS,ZERO_LEFT>::FindMaxOne() const {
    // Find the max field with a one.
    int max_field = ((int) NUM_FIELDS) - 1;
    while (max_field >= 0 && bits[max_field] == 0) max_field--;

    // If there are no ones, return -1.
    if (max_field == -1) return -1;

    const field_t field = bits[max_field]; // Save a local copy of this field.
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
  template <size_t NUM_BITS, bool ZERO_LEFT>
  int BitArray<NUM_BITS,ZERO_LEFT>::PopOne() {
    const int out_bit = FindOne();
    if (out_bit >= 0) Clear(out_bit);
    return out_bit;
  }

  /// Return a vector indicating the posistions of all ones in the BitArray.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  emp::vector<size_t> BitArray<NUM_BITS,ZERO_LEFT>::GetOnes() const {
    // @CAO -- There are better ways to do this with bit tricks.
    emp::vector<size_t> ones(CountOnes());
    size_t cur_pos = 0;
    for (size_t i = 0; i < NUM_BITS; i++) {
      if (Get(i)) ones[cur_pos++] = i;
    }
    return ones;
  }

  /// Find the length of the longest continuous series of ones.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  size_t BitArray<NUM_BITS,ZERO_LEFT>::LongestSegmentOnes() const {
    size_t length = 0;
    BitArray test_bits(*this);
    while(test_bits.Any()){
      ++length;
      test_bits.AND_SELF(test_bits<<1);
    }
    return length;
  }


  // -------------------------  Print/String Functions  ------------------------- //

  /// Convert this BitArray to a string (using default direction)
  template <size_t NUM_BITS, bool ZERO_LEFT>
  std::string BitArray<NUM_BITS,ZERO_LEFT>::ToString() const {
    if constexpr (ZERO_LEFT) return ToArrayString();
    else return ToBinaryString();
  }

  /// Convert this BitArray to an array string [0 index on left]
  template <size_t NUM_BITS, bool ZERO_LEFT>
  std::string BitArray<NUM_BITS,ZERO_LEFT>::ToArrayString() const {
    std::string out_string;
    out_string.reserve(NUM_BITS);
    for (size_t i = 0; i < NUM_BITS; ++i) out_string.push_back(GetAsChar(i));
    return out_string;
  }

  /// Convert this BitArray to a numerical string [0 index on right]
  template <size_t NUM_BITS, bool ZERO_LEFT>
  std::string BitArray<NUM_BITS,ZERO_LEFT>::ToBinaryString() const {
    std::string out_string;
    out_string.reserve(NUM_BITS);
    for (size_t i = NUM_BITS; i > 0; --i) out_string.push_back(GetAsChar(i-1));
    return out_string;
  }

  /// Convert this BitArray to a series of IDs
  template <size_t NUM_BITS, bool ZERO_LEFT>
  std::string BitArray<NUM_BITS,ZERO_LEFT>::ToIDString(const std::string & spacer) const {
    std::stringstream ss;
    PrintOneIDs(ss, spacer);
    return ss.str();
  }

  /// Convert this BitArray to a series of IDs with ranges condensed.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  std::string BitArray<NUM_BITS,ZERO_LEFT>::ToRangeString(const std::string & spacer,
                                       const std::string & ranger) const
  {
    std::stringstream ss;
    PrintAsRange(ss, spacer, ranger);
    return ss.str();
  }

  /// Print a space between each field (or other provided spacer)
  template <size_t NUM_BITS, bool ZERO_LEFT>
  void BitArray<NUM_BITS,ZERO_LEFT>::PrintFields(std::ostream & out, const std::string & spacer) const {
    for (size_t i = NUM_BITS-1; i < NUM_BITS; i--) {
      out << Get(i);
      if (i && (i % FIELD_BITS == 0)) out << spacer;
    }
  }

  /// Print a space between each field (or other provided spacer)
  template <size_t NUM_BITS, bool ZERO_LEFT>
  void BitArray<NUM_BITS,ZERO_LEFT>::PrintDebug(std::ostream & out) const {
    for (size_t field = 0; field < NUM_FIELDS; field++) {
      for (size_t bit_id = 0; bit_id < FIELD_BITS; bit_id++) {
        bool bit = (FIELD_1 << bit_id) & bits[field];
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
  template <size_t NUM_BITS, bool ZERO_LEFT>
  void BitArray<NUM_BITS,ZERO_LEFT>::PrintOneIDs(std::ostream & out, const std::string & spacer) const {
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
  template <size_t NUM_BITS, bool ZERO_LEFT>
  void BitArray<NUM_BITS,ZERO_LEFT>::PrintAsRange(std::ostream & out,
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
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> & BitArray<NUM_BITS,ZERO_LEFT>::NOT_SELF() {
    for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = ~bits[i];
    ClearExcessBits();
    return *this;
  }

  /// Perform a Boolean AND with a second BitArray, store result here, and return this object.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> & BitArray<NUM_BITS,ZERO_LEFT>::AND_SELF(const this_t & array2) {
    for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = bits[i] & array2.bits[i];
    return *this;
  }

  /// Perform a Boolean OR with a second BitArray, store result here, and return this object.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> & BitArray<NUM_BITS,ZERO_LEFT>::OR_SELF(const this_t & array2) {
    for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = bits[i] | array2.bits[i];
    return *this;
  }

  /// Perform a Boolean NAND with a second BitArray, store result here, and return this object.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> & BitArray<NUM_BITS,ZERO_LEFT>::NAND_SELF(const this_t & array2) {
    for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = ~(bits[i] & array2.bits[i]);
    ClearExcessBits();
    return *this;
  }

  /// Perform a Boolean NOR with a second BitArray, store result here, and return this object.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> & BitArray<NUM_BITS,ZERO_LEFT>::NOR_SELF(const this_t & array2) {
    for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = ~(bits[i] | array2.bits[i]);
    ClearExcessBits();
    return *this;
  }

  /// Perform a Boolean XOR with a second BitArray, store result here, and return this object.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> & BitArray<NUM_BITS,ZERO_LEFT>::XOR_SELF(const this_t & array2) {
    for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = bits[i] ^ array2.bits[i];
    return *this;
  }

  /// Perform a Boolean EQU with a second BitArray, store result here, and return this object.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> & BitArray<NUM_BITS,ZERO_LEFT>::EQU_SELF(const this_t & array2) {
    for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = ~(bits[i] ^ array2.bits[i]);
    ClearExcessBits();
    return *this;
  }

  /// Positive shifts go right and negative shifts go left (0 does nothing);
  /// return result.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> BitArray<NUM_BITS,ZERO_LEFT>::SHIFT(const int shift_size) const {
    this_t out_array(*this);
    if (shift_size > 0) out_array.ShiftRight((field_t) shift_size);
    else if (shift_size < 0) out_array.ShiftLeft((field_t) (-shift_size));
    return out_array;
  }

  /// Positive shifts go right and negative shifts go left (0 does nothing);
  /// store result here, and return this object.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> & BitArray<NUM_BITS,ZERO_LEFT>::SHIFT_SELF(const int shift_size) {
    if (shift_size > 0) ShiftRight((field_t) shift_size);
    else if (shift_size < 0) ShiftLeft((field_t) -shift_size);
    return *this;
  }

  /// Reverse the order of bits in the BitArray
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> & BitArray<NUM_BITS,ZERO_LEFT>::REVERSE_SELF() {

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

  /// Reverse order of bits in the BitArray.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> BitArray<NUM_BITS,ZERO_LEFT>::REVERSE() const {
    this_t out_array(*this);
    return out_array.REVERSE_SELF();
  }


  /// Positive rotates go left and negative rotates go left (0 does nothing);
  /// return result.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> BitArray<NUM_BITS,ZERO_LEFT>::ROTATE(const int rotate_size) const {
    this_t out_array(*this);
    if (rotate_size > 0) out_array.RotateRight((field_t) rotate_size);
    else if (rotate_size < 0) out_array.RotateLeft((field_t) (-rotate_size));
    return out_array;
  }

  /// Positive rotates go right and negative rotates go left (0 does nothing);
  /// store result here, and return this object.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> & BitArray<NUM_BITS,ZERO_LEFT>::ROTATE_SELF(const int rotate_size) {
    if (rotate_size > 0) RotateRight((field_t) rotate_size);
    else if (rotate_size < 0) RotateLeft((field_t) -rotate_size);
    return *this;
  }

  /// Helper: call ROTATE with negative number instead
  template <size_t NUM_BITS, bool ZERO_LEFT>
  template<size_t shift_size_raw>
  BitArray<NUM_BITS,ZERO_LEFT> & BitArray<NUM_BITS,ZERO_LEFT>::ROTL_SELF() {
    constexpr size_t shift_size = shift_size_raw % NUM_BITS;

    // special case: for exactly one field_t, try to go low level
    // adapted from https://stackoverflow.com/questions/776508/best-practices-for-circular-shift-rotate-operations-in-c
    if constexpr (NUM_FIELDS == 1) {
      field_t & n = bits[0];
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
          std::rbegin(bits),
          std::rbegin(bits)+field_shift,
          std::rend(bits)
        );
      }

      // if necessary, shift filler bits out of the middle
      if constexpr ((bool)NUM_END_BITS) {
        const int filler_idx = (LAST_FIELD + field_shift) % NUM_FIELDS;
        for (int i = filler_idx + 1; i < (int)NUM_FIELDS; ++i) {
          bits[i-1] |= bits[i] << NUM_END_BITS;
          bits[i] >>= (FIELD_BITS - NUM_END_BITS);
        }
      }

      // account for bit_shift
      if (bit_shift) {

        const field_t keystone = NUM_END_BITS ? (
          (bits[LAST_FIELD] << (FIELD_BITS - NUM_END_BITS))
          | (bits[NUM_FIELDS - 2] >> NUM_END_BITS)
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
  template <size_t NUM_BITS, bool ZERO_LEFT>
  template<size_t shift_size_raw>
  BitArray<NUM_BITS,ZERO_LEFT> & BitArray<NUM_BITS,ZERO_LEFT>::ROTR_SELF() {

    constexpr size_t shift_size = shift_size_raw % NUM_BITS;

    // special case: for exactly one field_t, try to go low level
    // adapted from https://stackoverflow.com/questions/776508/best-practices-for-circular-shift-rotate-operations-in-c
    if constexpr (NUM_FIELDS == 1) {
      field_t & n = bits[0];
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
          std::begin(bits),
          std::begin(bits)+field_shift,
          std::end(bits)
        );
      }

      // if necessary, shift filler bits out of the middle
      if constexpr ((bool)NUM_END_BITS) {
        constexpr int filler_idx = LAST_FIELD - field_shift;
        for (int i = filler_idx + 1; i < (int)NUM_FIELDS; ++i) {
          bits[i-1] |= bits[i] << NUM_END_BITS;
          bits[i] >>= (FIELD_BITS - NUM_END_BITS);
        }
      }

      // account for bit_shift
      if (bit_shift) {

        const field_t keystone = NUM_END_BITS ? (
          bits[0] >> (FIELD_BITS - NUM_END_BITS)
        ) : (
          bits[0]
        );

        if constexpr ((bool)NUM_END_BITS) {
          bits[NUM_FIELDS-1] |= bits[0] << NUM_END_BITS;
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

  /// Addition of two BitArrays.
  /// Wraps if it overflows.
  /// Returns result.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> BitArray<NUM_BITS,ZERO_LEFT>::ADD(const BitArray & array2) const{
    this_t out_array(*this);
    return out_array.ADD_SELF(array2);
  }

  /// Addition of two BitArrays.
  /// Wraps if it overflows.
  /// Returns this object.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> & BitArray<NUM_BITS,ZERO_LEFT>::ADD_SELF(const this_t & array2) {
    bool carry = false;

    for (size_t i = 0; i < NUM_BITS/FIELD_BITS; ++i) {
      field_t addend = array2.bits[i] + static_cast<field_t>(carry);
      carry = array2.bits[i] > addend;

      field_t sum = bits[i] + addend;
      carry |= bits[i] > sum;

      bits[i] = sum;
    }

    if constexpr (static_cast<bool>(NUM_END_BITS)) {
      bits[NUM_BITS/FIELD_BITS] = (
        bits[NUM_BITS/FIELD_BITS]
        + array2.bits[NUM_BITS/FIELD_BITS]
        + static_cast<field_t>(carry)
      ) & END_MASK;
    }

    return *this;
  }

  /// Subtraction of two BitArrays.
  /// Wraps around if it underflows.
  /// Returns result.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> BitArray<NUM_BITS,ZERO_LEFT>::SUB(const this_t & array2) const{
    this_t out_array(*this);
    return out_array.SUB_SELF(array2);
  }

  /// Subtraction of two BitArrays.
  /// Wraps if it underflows.
  /// Returns this object.
  template <size_t NUM_BITS, bool ZERO_LEFT>
  BitArray<NUM_BITS,ZERO_LEFT> & BitArray<NUM_BITS,ZERO_LEFT>::SUB_SELF(const this_t & array2){

    bool carry = false;

    for (size_t i = 0; i < NUM_BITS/FIELD_BITS; ++i) {
      const field_t subtrahend = array2.bits[i] + static_cast<field_t>(carry);
      carry = array2.bits[i] > subtrahend;
      carry |= bits[i] < subtrahend;
      bits[i] -= subtrahend;
    }

    if constexpr (static_cast<bool>(NUM_END_BITS)) {
      bits[NUM_BITS/FIELD_BITS] = (
        bits[NUM_BITS/FIELD_BITS]
          - array2.bits[NUM_BITS/FIELD_BITS]
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
    out_bits |= in1.template Export<NUM_BITS1+NUM_BITS2>();
  }

  /// Computes simple matching coefficient (https://en.wikipedia.org/wiki/Simple_matching_coefficient).
  template <size_t NUM_BITS, bool ZERO_LEFT>
  double SimpleMatchCoeff(const BitArray<NUM_BITS,ZERO_LEFT> & in1,
                          const BitArray<NUM_BITS,ZERO_LEFT> & in2) {
    emp_assert(NUM_BITS > 0); // TODO: can be done with XOR
    return (double)(~(in1 ^ in2)).CountOnes() / (double) NUM_BITS;
  }

}

/// For hashing BitArrays
namespace std
{
    template <size_t N, bool ZERO_LEFT>
    struct hash<emp::BitArray<N, ZERO_LEFT>>
    {
        size_t operator()( const emp::BitArray<N,ZERO_LEFT> & bs ) const noexcept
        {
          return bs.Hash();
        }
    };
}

#endif // #ifndef EMP_BITS_BITARRAY_HPP_INCLUDE
