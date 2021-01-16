/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2020.
 *
 *  @file  BitSet.hpp
 *  @brief A drop-in replacement for std::bitset, with additional bit magic features.
 *  @note Status: RELEASE
 *
 *  @note Like std::bitset, bit zero is on the right side.  Unlike std::bitset, emp::BitSet
 *        gives access to bit fields for easy access to different sized chunk of bits and
 *        implementation new bit-magic tricks.
 * 
 *  @todo Some of the functions allow a start bit and end bit; each of these should be checked
 *        to make sure that they will work if the start and end are part of the same byte.  One
 *        option is to do this well ONCE with a macro that properly fills in the details.
 */


#ifndef EMP_BIT_SET_H
#define EMP_BIT_SET_H

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
#include "../tools/functions.hpp"
#include "../polyfill/span.hpp"

#include "bitset_utils.hpp"
#include "_bitset_helpers.hpp"

namespace emp {

  ///  A fixed-sized (but arbitrarily large) array of bits, and optimizes operations on those bits
  ///  to be as fast as possible.
  template <size_t NUM_BITS>
  class BitSet {

    // make all templated instantiations friends with each other
    template <size_t FRIEND_BITS> friend class BitSet;

  private:
    using this_t = BitSet<NUM_BITS>;

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

    // Mask to use to clear out any end bits that should be zeroes.
    static constexpr field_t END_MASK = MaskLow<field_t>(NUM_END_BITS);

    static constexpr field_t FIELD_0 = (field_t) 0;      ///< All bits in a field set to 0
    static constexpr field_t FIELD_1 = (field_t) 1;      ///< Least significant bit set to 1
    static constexpr field_t FIELD_255 = (field_t) 255;  ///< Least significant 8 bits set to 1
    static constexpr field_t FIELD_ALL = ~FIELD_0;       ///< All bits in a field set to 1

    field_t bit_set[NUM_FIELDS];  ///< Fields to hold the actual bits for this BitSet.

    // Identify the field that a specified bit is in.
    static size_t FieldID(const size_t index) { return index >> FIELD_LOG2; }

    // Identify the byte that a specified bit is in.
    static size_t ByteID(const size_t index) { return index >> 3; }

    // Identify the position within a field where a specified bit is.
    static size_t FieldPos(const size_t index) { return index & (FIELD_BITS - 1); }

    // Identify the position within a byte where a specified bit is.
    static size_t BytePos(const size_t index) { return index & 7; }

    // Identify which field a specified byte position would be in.
    static size_t Byte2Field(const size_t index) { return index / sizeof(field_t); }

    // Convert a byte position in BitVector to a byte position in the target field.
    static size_t Byte2FieldPos(const size_t index) { return FieldPos(index * 8); }

    // Copy an array of bits into this BitSet (internal use only!)
    template <size_t IN_FIELDS, size_t COPY_FIELDS=NUM_FIELDS>
    void Copy(const field_t in_set[IN_FIELDS]) {
      static_assert(COPY_FIELDS <= IN_FIELDS, "Cannot copy more fields than we are given.");
      static_assert(COPY_FIELDS <= NUM_FIELDS, "Cannot copy into more fields than are available.");
      constexpr size_t COPY_BYTES = COPY_FIELDS * sizeof(field_t);
      std::memcpy(bit_set, in_set, COPY_BYTES);
    }

    // Any bits past the last "real" bit in the last field should be kept as zeros.
    void ClearExcessBits() { if constexpr (NUM_END_BITS > 0) bit_set[LAST_FIELD] &= END_MASK; }

    // Convert the bit_set to bytes.
    emp::Ptr<unsigned char> BytePtr() { return reinterpret_cast<unsigned char*>(bit_set); }

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
    BitSet() { Clear(); }

    /// Copy constructor from another BitSet
    BitSet(const BitSet & in_set) { Copy(in_set.bit_set); }

    /// Constructor to generate a random BitSet (with equal prob of 0 or 1).
    BitSet(Random & random) { Randomize(random); ClearExcessBits(); }

    /// Constructor to generate a random BitSet with provided PROBABILITY of 1's.
    BitSet(Random & random, double p1) { Randomize(random, p1); ClearExcessBits(); }

    /// Constructor to generate a random BitSet with provided NUMBER of 1's.
    BitSet(Random & random, size_t num_ones) { Randomize(random, num_ones); ClearExcessBits(); }

    /// Constructor to fill in a bit set from a vector.
    template <typename T> BitSet(const std::initializer_list<T> l);

    /// Destructor.
    ~BitSet() = default;

    /// Assignment operator (no separate move opperator since no resources to move...)
    BitSet & operator=(const BitSet<NUM_BITS> & in_set) { Copy(in_set.bit_set); return *this; }

    /// How many bits are in this BitSet?
    constexpr static size_t GetSize() { return NUM_BITS; }

    /// Retrieve the bit as a specified index.
    bool Get(size_t index) const;

    /// A safe version of Get() for indexing out of range. Useful for representing collections.
    bool Has(size_t index) const { return (index < NUM_BITS) ? Get(index) : false; }

    /// Set the bit at a specified index.
    BitSet & Set(size_t index, bool value=true);

    /// Set all bits to one.
    BitSet & SetAll();

    /// Set a range of bits to one: [start, stop)
    BitSet & SetRange(size_t start, size_t stop);

    /// Set all bits to zero.
    BitSet & Clear() { for (field_t & x : bit_set) x = FIELD_0; return *this; }

    /// Set specific bit to 0.
    BitSet & Clear(size_t index) { return Set(index, false); }

    /// Set bits to 0 in the range [start, stop)
    BitSet & Clear(const size_t start, const size_t stop);

    /// Index into a const BitSet (i.e., cannot be set this way.)
    bool operator[](size_t index) const { return Get(index); }

    /// Index into a BitSet, returning a proxy that will allow bit assignment to work.
    BitProxy<this_t> operator[](size_t index) { return BitProxy<this_t>(*this, index); }

    /// Flip all bits in this BitSet
    BitSet & Toggle() { return NOT_SELF(); }

    /// Flip a single bit
    BitSet & Toggle(size_t index);

    /// Flips all the bits in a range [start, stop)
    BitSet & Toggle(size_t start, size_t stop);

    /// Return true if ANY bits in the BitSet are one, else return false.
    bool Any() const { for (auto i : bit_set) if (i) return true; return false; }

    /// Return true if NO bits in the BitSet are one, else return false.
    bool None() const { return !Any(); }

    /// Return true if ALL bits in the BitSet are one, else return false.
    bool All() const { return (~(*this)).None(); }

    /// Set all bits randomly, with a 50% probability of being a 0 or 1.
    BitSet &  Randomize(Random & random);

    /// Set all bits randomly, with probability specified at compile time.
    template <Random::Prob P>
    BitSet & RandomizeP(Random & random,
                        const size_t start_pos=0, const size_t stop_pos=NUM_BITS);

    /// Set all bits randomly, with a given probability of being a one.
    BitSet & Randomize(Random & random, const double p,
                       const size_t start_pos=0, const size_t stop_pos=NUM_BITS);

    /// Set all bits randomly, with a given probability of being a one.
    BitSet & Randomize(Random & random, const size_t target_ones,
                       const size_t start_pos=0, const size_t stop_pos=NUM_BITS);
    
    /// Flip random bits with a given probability.
    BitSet & FlipRandom(Random & random, const double p,
                        const size_t start_pos=0, const size_t stop_pos=NUM_BITS);

    /// Set random bits with a given probability (does not check if already set.)
    BitSet & SetRandom(Random & random, const double p,
                        const size_t start_pos=0, const size_t stop_pos=NUM_BITS);

    /// Unset random bits with a given probability (does not check if already zero.)
    BitSet & ClearRandom(Random & random, const double p,
                        const size_t start_pos=0, const size_t stop_pos=NUM_BITS);

    /// Flip a specified number of random bits.
    /// @note: This was previously called Mutate.
    BitSet & FlipRandom(Random & random, const size_t num_bits);

    /// Set a specified number of random bits (does not check if already set.)
    BitSet & SetRandom(Random & random, const size_t num_bits);

    /// Unset  a specified number of random bits (does not check if already zero.)
    BitSet & ClearRandom(Random & random, const size_t num_bits);

    // >>>>>>>>>>  Comparison Operators  <<<<<<<<<< //

    template <size_t T2> bool operator==(const BitSet<T2> & in) const;
    template <size_t T2> bool operator!=(const BitSet<T2> & in) const { return !(*this == in); }
    template <size_t T2> bool operator< (const BitSet<T2> & in) const;
    template <size_t T2> bool operator> (const BitSet<T2> & in) const { return in < *this; }
    template <size_t T2> bool operator<=(const BitSet<T2> & in) const { return !(in < *this); }
    template <size_t T2> bool operator>=(const BitSet<T2> & in) const { return !(*this < in); }

    ////////////////////////////////////////////
    ////////////////////////////////////////////
    ////////////////////////////////////////////
    ///////////// CONTINUE HERE!! //////////////
    ////////////////////////////////////////////
    ////////////////////////////////////////////
    ////////////////////////////////////////////

    /// Assign from a BitSet of a different size.
    template <size_t FROM_BITS>
    BitSet & Import(
      const BitSet<FROM_BITS> & from_set,
      const size_t from_bit=0
    ) {

      if constexpr (FROM_BITS == NUM_BITS) emp_assert(&from_set != this);

      emp_assert(from_bit < FROM_BITS);

      if (FROM_BITS - from_bit < NUM_BITS) Clear();

      const size_t DEST_BYTES = (NUM_BITS + 7)/8;
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
    template <size_t FROM_BITS>
    BitSet<FROM_BITS> Export(size_t start_bit=0) const {

      BitSet<FROM_BITS> out_bits;
      out_bits.Import(*this, start_bit);

      return out_bits;
    }


    /// How many bytes are in this BitSet?
    constexpr static size_t GetNumBytes() { return TOTAL_BYTES; }

    /// Get the full byte starting from the bit at a specified index.
    uint8_t GetByte(size_t index) const {
      emp_assert(index < TOTAL_BYTES);
      const size_t field_id = Byte2Field(index);
      const size_t pos_id = Byte2FieldPos(index);
      return (bit_set[field_id] >> pos_id) & 255;
    }

    /// Get a read-only view into the internal array used by BitSet.
    /// @return Read-only span of BitSet's bytes.
    std::span<const std::byte> GetBytes() const {
      return std::span<const std::byte>(
        reinterpret_cast<const std::byte*>(bit_set),
        TOTAL_BYTES
      );
    }

    /// Set the full byte starting at the bit at the specified index.
    void SetByte(size_t index, uint8_t value) {
      emp_assert(index < TOTAL_BYTES);
      const size_t field_id = Byte2Field(index);
      const size_t pos_id = Byte2FieldPos(index);
      const field_t val_uint = value;
      bit_set[field_id] = (bit_set[field_id] & ~(((field_t)255U) << pos_id)) | (val_uint << pos_id);
    }

    /// Get the unsigned int; index in in 32-bit jumps
    /// (i.e., this is a field ID not bit id)
    uint32_t GetUInt(const size_t index) const { return GetUInt32(index); }

    /// Set the unsigned int; index in in 32-bit jumps
    /// (i.e., this is a field ID not bit id)
    void SetUInt(const size_t index, const uint32_t value) {
      SetUInt32(index, value);
    }

    /// Get the field_t unsigned int; index in in 32-bit jumps
    /// (i.e., this is a field ID not bit id)
    uint32_t GetUInt32(const size_t index) const {
      emp_assert(index * 32 < NUM_BITS);

      uint32_t res;

      std::memcpy(
        &res,
        reinterpret_cast<const unsigned char*>(bit_set) + index * (32/8),
        sizeof(res)
      );

      return res;
    }

    /// Set the field_t unsigned int; index in in 32-bit jumps
    /// (i.e., this is a field ID not bit id)
    void SetUInt32(const size_t index, const uint32_t value) {
      emp_assert(index * 32 < NUM_BITS);

      std::memcpy( BytePtr() + index * (32/8), &value, sizeof(value) );

      ClearExcessBits();
    }

    /// Get the field_t unsigned int; index in in 64-bit jumps
    /// (i.e., this is a field ID not bit id)
    uint64_t GetUInt64(const size_t index) const {
      emp_assert(index * 64 < NUM_BITS);

      uint64_t res = 0;

      if constexpr (FIELD_BITS == 64) {
        res = bit_set[index];
      } else if constexpr (FIELD_BITS == 32 && (NUM_FIELDS % 2 == 0)) {
        std::memcpy(
          &res,
          reinterpret_cast<const unsigned char*>(bit_set) + index * (64/8),
          sizeof(res)
        );
      } else if constexpr (FIELD_BITS == 32 && NUM_FIELDS == 1) {
        std::memcpy(
          &res,
          reinterpret_cast<const unsigned char*>(bit_set),
          32/8
        );
      } else {
        std::memcpy(
          &res,
          reinterpret_cast<const unsigned char*>(bit_set) + index * (64/8),
          std::min<size_t>(64, NUM_FIELDS * FIELD_BITS - 64 * index)/8
        );
      }

      return res;

    }

    /// Set the field_t unsigned int; index in in 64-bit jumps
    /// (i.e., this is a field ID not bit id)
    void SetUInt64(const size_t index, const uint64_t value) {
      emp_assert(index * 64 < NUM_BITS);

      if constexpr (FIELD_BITS == 64) {
        bit_set[index] = value;
      } else if constexpr (FIELD_BITS == 32 && (NUM_FIELDS % 2 == 0)) {
        std::memcpy( BytePtr() + index * (64/8), &value, sizeof(value)
        );
      } else if constexpr (FIELD_BITS == 32 && NUM_FIELDS == 1) {
        std::memcpy( BytePtr(), &value, 32/8 );
      } else {
        std::memcpy( BytePtr() + index * (64/8), &value,
                     std::min<size_t>(64, NUM_FIELDS * FIELD_BITS - 64 * index)/8 );
      }

      ClearExcessBits();
    }

    /// Get the full uint32_t unsigned int starting from the bit at a specified index.
    uint32_t GetUIntAtBit(const size_t index) { return GetUInt32AtBit(index); }

    /// Get the full uint32_t unsigned int starting from the bit at a specified index.
    uint32_t GetUInt32AtBit(const size_t index) {
      emp_assert(index < NUM_BITS);

      BitSet<32> res;
      res.Import(*this, index);

      return res.GetUInt32(0);

    }

    /// Get OUT_BITS bits starting from the bit at a specified index (max 32)
    template <size_t OUT_BITS>
    uint32_t GetValueAtBit(const size_t index) {
      static_assert(OUT_BITS <= 32, "Requesting too many bits to fit in a UInt");
      return GetUIntAtBit(index) & MaskLow<uint32_t>(OUT_BITS);
    }

    /// Get the unsigned numeric value represented by the BitSet as a double
    double GetDouble() const {

      if constexpr (NUM_BITS <= 64) {
        uint64_t res{};
        std::memcpy(&res, bit_set, TOTAL_BYTES);
        return res;
      } else {
        double res = 0.0;
        for (size_t i = 0; i < (NUM_BITS + 63) / 64; ++i) {
          res += GetUInt64(i) * emp::Pow2(i * 64);
        }
        return res;
      }

    }

    /// What is the maximum value this BitSet could contain, as a double?
    static constexpr double MaxDouble() { return emp::Pow2(NUM_BITS) - 1.0; }

    /// Overload ostream operator to return Print.
    friend std::ostream& operator<<(std::ostream &out, const BitSet& bs){
      bs.Print(out);
      return out;
    }

    /// Print all bits to the provided output stream.
    void Print(std::ostream & out=std::cout) const {
      for (size_t i = NUM_BITS; i > 0; i--) { out << Get(i-1); }

      // out << "/";
      // for (size_t i = NUM_BITS; i < NUM_FIELDS * FIELD_BITS; i++) {
      //   const size_t field_id = FieldID(i);
      //   const size_t pos_id = FieldPos(i);
      //   out << ((bit_set[field_id] & (((field_t)1U) << pos_id)) != 0);
      // }
    }

    /// Print all bits from smallest to largest, as if this were an array, not a bit representation.
    void PrintArray(std::ostream & out=std::cout) const {
      for (size_t i = 0; i < NUM_BITS; i++) out << Get(i);
    }

    /// Print the locations of all one bits, using the provided spacer (default is a single space)
    void PrintOneIDs(std::ostream & out=std::cout, char spacer=' ') const {
      for (size_t i = 0; i < NUM_BITS; i++) { if (Get(i)) out << i << spacer; }
    }

    /// Count 1's by looping through once for each bit equal to 1
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

    /// Count 1's in semi-parallel; fastest for even 0's & 1's
    size_t CountOnes_Mixed() const {

      size_t bit_count = 0;
      for (size_t f = 0; f < NUM_FIELDS; ++f) {
          // when compiling with -O3 and -msse4.2, this is the fastest population count method.
          // this is due to using a dedicated instuction that runs in 1 clock cycle.
          std::bitset<FIELD_BITS> std_bs(bit_set[f]);
          bit_count += std_bs.count();
       }

      return bit_count;
    }

    /// Count the number of ones in the BitSet using bit tricks for a speedup.
    size_t CountOnes() const { return CountOnes_Mixed(); }

    /// Return the index of the first one in the sequence; return -1 if no ones are available.
    int FindBit() const {
      size_t field_id = 0;
      while (field_id < NUM_FIELDS && bit_set[field_id]==0) field_id++;
      return (field_id < NUM_FIELDS) ? (int) (find_bit(bit_set[field_id]) + (field_id << FIELD_LOG2)) : -1;
    }

    /// Return index of first one in sequence (or -1 if no ones); change this position to zero.
    int PopBit() {
      size_t field_id = 0;
      while (field_id < NUM_FIELDS && bit_set[field_id]==0) field_id++;
      if (field_id == NUM_FIELDS) return -1;  // Failed to find bit!

      const int pos_found = (int) find_bit(bit_set[field_id]);
      bit_set[field_id] &= ~(1U << pos_found);
      return pos_found + (int)(field_id << FIELD_LOG2);
    }

    /// Return index of first one in sequence AFTER start_pos (or -1 if no ones)
    int FindBit(const size_t start_pos) const {
      // @CAO -- There are better ways to do this with bit tricks
      //         (but start_pos is tricky...)
      for (size_t i = start_pos; i < NUM_BITS; i++) {
        if (Get(i)) return (int) i;
      }
      return -1;
    }

    /// Return a vector indicating the posistions of all ones in the BitSet.
    emp::vector<size_t> GetOnes() const {
      // @CAO -- There are better ways to do this with bit tricks.
      emp::vector<size_t> out_set(CountOnes());
      size_t cur_pos = 0;
      for (size_t i = 0; i < NUM_BITS; i++) {
        if (Get(i)) out_set[cur_pos++] = i;
      }
      return out_set;
    }

    /// Finds the length of the longest segment of ones.
    size_t LongestSegmentOnes() const {
      size_t length = 0;
      BitSet out_set(*this);
      while(out_set.Any()){
        out_set.AND_SELF(out_set<<1);
        ++length;
      }
      return length;
    }


    /// Perform a Boolean NOT on this BitSet and return the result.
    BitSet NOT() const {
      BitSet out_set(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = ~bit_set[i];
      out_set.ClearExcessBits();
      return out_set;
    }

    /// Perform a Boolean AND with a second BitSet and return the result.
    BitSet AND(const BitSet & set2) const {
      BitSet out_set(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = bit_set[i] & set2.bit_set[i];
      return out_set;
    }

    /// Perform a Boolean OR with a second BitSet and return the result.
    BitSet OR(const BitSet & set2) const {
      BitSet out_set(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = bit_set[i] | set2.bit_set[i];
      return out_set;
    }

    /// Perform a Boolean NAND with a second BitSet and return the result.
    BitSet NAND(const BitSet & set2) const {
      BitSet out_set(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = ~(bit_set[i] & set2.bit_set[i]);
      out_set.ClearExcessBits();
      return out_set;
    }

    /// Perform a Boolean NOR with a second BitSet and return the result.
    BitSet NOR(const BitSet & set2) const {
      BitSet out_set(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = ~(bit_set[i] | set2.bit_set[i]);
      out_set.ClearExcessBits();
      return out_set;
    }

    /// Perform a Boolean XOR with a second BitSet and return the result.
    BitSet XOR(const BitSet & set2) const {
      BitSet out_set(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = bit_set[i] ^ set2.bit_set[i];
      return out_set;
    }

    /// Perform a Boolean EQU with a second BitSet and return the result.
    BitSet EQU(const BitSet & set2) const {
      BitSet out_set(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_set.bit_set[i] = ~(bit_set[i] ^ set2.bit_set[i]);
      out_set.ClearExcessBits();
      return out_set;
    }


    /// Perform a Boolean NOT on this BitSet, store result here, and return this object.
    BitSet & NOT_SELF() {
      for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~bit_set[i];
      ClearExcessBits();
      return *this;
    }

    /// Perform a Boolean AND with a second BitSet, store result here, and return this object.
    BitSet & AND_SELF(const BitSet & set2) {
      for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = bit_set[i] & set2.bit_set[i];
      return *this;
    }

    /// Perform a Boolean OR with a second BitSet, store result here, and return this object.
    BitSet & OR_SELF(const BitSet & set2) {
      for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = bit_set[i] | set2.bit_set[i];
      return *this;
    }

    /// Perform a Boolean NAND with a second BitSet, store result here, and return this object.
    BitSet & NAND_SELF(const BitSet & set2) {
      for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~(bit_set[i] & set2.bit_set[i]);
      ClearExcessBits();
      return *this;
    }

    /// Perform a Boolean NOR with a second BitSet, store result here, and return this object.
    BitSet & NOR_SELF(const BitSet & set2) {
      for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~(bit_set[i] | set2.bit_set[i]);
      ClearExcessBits();
      return *this;
    }

    /// Perform a Boolean XOR with a second BitSet, store result here, and return this object.
    BitSet & XOR_SELF(const BitSet & set2) {
      for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = bit_set[i] ^ set2.bit_set[i];
      return *this;
    }

    /// Perform a Boolean EQU with a second BitSet, store result here, and return this object.
    BitSet & EQU_SELF(const BitSet & set2) {
      for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~(bit_set[i] ^ set2.bit_set[i]);
      ClearExcessBits();
      return *this;
    }

    /// Positive shifts go right and negative shifts go left (0 does nothing);
    /// return result.
    BitSet SHIFT(const int shift_size) const {
      BitSet out_set(*this);
      if (shift_size > 0) out_set.ShiftRight((field_t) shift_size);
      else if (shift_size < 0) out_set.ShiftLeft((field_t) (-shift_size));
      return out_set;
    }

    /// Positive shifts go right and negative shifts go left (0 does nothing);
    /// store result here, and return this object.
    BitSet & SHIFT_SELF(const int shift_size) {
      if (shift_size > 0) ShiftRight((field_t) shift_size);
      else if (shift_size < 0) ShiftLeft((field_t) -shift_size);
      return *this;
    }

    /// Reverse the order of bits in the bitset
    BitSet & REVERSE_SELF() {

      // reverse bytes
      std::reverse(
        reinterpret_cast<unsigned char *>(bit_set),
        reinterpret_cast<unsigned char *>(bit_set) + TOTAL_BYTES
      );

      // reverse each byte
      // adapted from https://stackoverflow.com/questions/2602823/in-c-c-whats-the-simplest-way-to-reverse-the-order-of-bits-in-a-byte
      for (size_t i = 0; i < TOTAL_BYTES; ++i) {
        unsigned char & b = reinterpret_cast<unsigned char *>(bit_set)[i];
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
    BitSet REVERSE() const {
      BitSet out_set(*this);
      return out_set.REVERSE_SELF();
    }


    /// Positive rotates go left and negative rotates go left (0 does nothing);
    /// return result.
    BitSet ROTATE(const int rotate_size) const {
      BitSet out_set(*this);
      if (rotate_size > 0) out_set.RotateRight((field_t) rotate_size);
      else if (rotate_size < 0) out_set.RotateLeft((field_t) (-rotate_size));
      return out_set;
    }

    /// Positive rotates go right and negative rotates go left (0 does nothing);
    /// store result here, and return this object.
    BitSet & ROTATE_SELF(const int rotate_size) {
      if (rotate_size > 0) RotateRight((field_t) rotate_size);
      else if (rotate_size < 0) RotateLeft((field_t) -rotate_size);
      return *this;
    }

    /// Helper: call ROTATE with negative number instead
    template<size_t shift_size_raw>
    BitSet & ROTL_SELF() {
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
    template<size_t shift_size_raw>
    BitSet & ROTR_SELF() {

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
    BitSet ADD(const BitSet & set2) const{
      BitSet out_set(*this);
      return out_set.ADD_SELF(set2);
    }

    /// Addition of two Bitsets.
    /// Wraps if it overflows.
    /// Returns this object.
    BitSet & ADD_SELF(const BitSet & set2) {
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
    BitSet SUB(const BitSet & set2) const{
      BitSet out_set(*this);
      return out_set.SUB_SELF(set2);
    }

    /// Subtraction of two Bitsets.
    /// Wraps if it underflows.
    /// Returns this object.
    BitSet & SUB_SELF(const BitSet & set2){

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

    /// Operator bitwise NOT...
    BitSet operator~() const { return NOT(); }

    /// Operator bitwise AND...
    BitSet operator&(const BitSet & ar2) const { return AND(ar2); }

    /// Operator bitwise OR...
    BitSet operator|(const BitSet & ar2) const { return OR(ar2); }

    /// Operator bitwise XOR...
    BitSet operator^(const BitSet & ar2) const { return XOR(ar2); }

    /// Operator shift left...
    BitSet operator<<(const size_t shift_size) const { return SHIFT(-(int)shift_size); }

    /// Operator shift right...
    BitSet operator>>(const size_t shift_size) const { return SHIFT((int)shift_size); }

    /// Compound operator bitwise AND...
    const BitSet & operator&=(const BitSet & ar2) { return AND_SELF(ar2); }

    /// Compound operator bitwise OR...
    const BitSet & operator|=(const BitSet & ar2) { return OR_SELF(ar2); }

    /// Compound operator bitwise XOR...
    const BitSet & operator^=(const BitSet & ar2) { return XOR_SELF(ar2); }

    /// Compound operator shift left...
    const BitSet & operator<<=(const size_t shift_size) { return SHIFT_SELF(-(int)shift_size); }

    /// Compound operator shift right...
    const BitSet & operator>>=(const size_t shift_size) { return SHIFT_SELF((int)shift_size); }

    /// Operator plus...
    BitSet operator+(const BitSet & ar2) const { return ADD(ar2); }

    /// Operator minus...
    BitSet operator-(const BitSet & ar2) const { return SUB(ar2); }

    /// Compound operator plus...
    const BitSet & operator+=(const BitSet & ar2) { return ADD_SELF(ar2); }

    /// Compoount operator minus...
    const BitSet & operator-=(const BitSet & ar2) { return SUB_SELF(ar2); }

    /// STL COMPATABILITY
    /// A set of functions to allow drop-in replacement with std::bitset.
    constexpr static size_t size() { return NUM_BITS; }
    inline bool all() const { return All(); }
    inline bool any() const { return Any(); }
    inline bool none() const { return !Any(); }
    inline size_t count() const { return CountOnes_Mixed(); }
    inline BitSet & flip() { return Toggle(); }
    inline BitSet & flip(size_t pos) { return Toggle(pos); }
    inline BitSet & flip(size_t start, size_t stop) { return Toggle(start, stop); }
    inline void reset() { Clear(); }
    inline void reset(size_t id) { Set(id, false); }
    inline void set() { SetAll(); }
    inline void set(size_t id) { Set(id); }
    inline bool test(size_t index) const { return Get(index); }

    template <class Archive>
    void serialize( Archive & ar )
    {
      ar( bit_set );
    }

  };

  // ------------------------ Implementations for Internal Functions ------------------------

  template <size_t NUM_BITS>
  void BitSet<NUM_BITS>::ShiftLeft(const size_t shift_size) {
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
  void BitSet<NUM_BITS>::ShiftRight(const size_t shift_size) {
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
  void BitSet<NUM_BITS>::RotateLeft(const size_t shift_size_raw) {
    const field_t shift_size = shift_size_raw % NUM_BITS;

    // use different approaches based on BitSet size
    if constexpr (NUM_FIELDS == 1) {
      // special case: for exactly one field_T, try to go low level
      // adapted from https://stackoverflow.com/questions/776508/best-practices-for-circular-shift-rotate-operations-in-c
      field_t & n = bit_set[0];
      size_t c = shift_size;

      // mask necessary to suprress shift count overflow warnings
      c &= FIELD_LOG2_MASK;
      n = (n<<c) | (n>>( (-(c+FIELD_BITS-NUM_BITS)) & FIELD_LOG2_MASK ));

    } else if constexpr (NUM_FIELDS < 32) {
      // for small BitSets, shifting L/R and ORing is faster
      emp::BitSet<NUM_BITS> dup(*this);
      dup.ShiftLeft(shift_size);
      ShiftRight(NUM_BITS - shift_size);
      OR_SELF(dup);
    } else {
      // for big BitSets, manual rotating is fater

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
  void BitSet<NUM_BITS>::RotateRight(const size_t shift_size_raw) {

    const size_t shift_size = shift_size_raw % NUM_BITS;

    // use different approaches based on BitSet size
    if constexpr (NUM_FIELDS == 1) {
      // special case: for exactly one field_t, try to go low level
      // adapted from https://stackoverflow.com/questions/776508/best-practices-for-circular-shift-rotate-operations-in-c

      field_t & n = bit_set[0];
      size_t c = shift_size;

      // mask necessary to suprress shift count overflow warnings
      c &= FIELD_LOG2_MASK;
      n = (n>>c) | (n<<( (NUM_BITS-c) & FIELD_LOG2_MASK ));

    } else if constexpr (NUM_FIELDS < 32) {
      // for small BitSets, shifting L/R and ORing is faster
      emp::BitSet<NUM_BITS> dup(*this);
      dup.ShiftRight(shift_size);
      ShiftLeft(NUM_BITS - shift_size);
      OR_SELF(dup);
    } else {
      // for big BitSets, manual rotating is fater

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

  // ------------------------- Longer Constructors --------------------------
  template <size_t NUM_BITS>
  template <typename T>
  BitSet<NUM_BITS>::BitSet(const std::initializer_list<T> l) {
    emp_assert(l.size() <= NUM_BITS, "Initializer longer than BitSet", l.size(), NUM_BITS);
    Clear();
    auto it = std::rbegin(l); // Right-most bit is position 0.
    for (size_t idx = 0; idx < NUM_BITS; ++idx) Set(idx, (idx < l.size()) && *it++);
  }

  // --------------------  Implementations of common accessors -------------------

  template <size_t NUM_BITS>
  bool BitSet<NUM_BITS>::Get(size_t index) const {
    emp_assert(index >= 0 && index < NUM_BITS);
    const size_t field_id = FieldID(index);
    const size_t pos_id = FieldPos(index);
    return (bit_set[field_id] & (((field_t)1U) << pos_id)) != 0;
  }

  /// Set the bit at a specified index.
  template <size_t NUM_BITS>
  BitSet<NUM_BITS> & BitSet<NUM_BITS>::Set(size_t index, bool value) {
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
  BitSet<NUM_BITS> & BitSet<NUM_BITS>::SetAll() {
    for (field_t & x : bit_set) x = FIELD_ALL;
    ClearExcessBits();
    return *this;
  }

  /// Set a range of bits to one: [start, stop)
  template <size_t NUM_BITS>
  BitSet<NUM_BITS> & BitSet<NUM_BITS>::SetRange(size_t start, size_t stop) {
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
  BitSet<NUM_BITS> & BitSet<NUM_BITS>::Clear(const size_t start, const size_t stop) {
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
  BitSet<NUM_BITS> & BitSet<NUM_BITS>::Toggle(size_t index) {
    emp_assert(index >= 0 && index < NUM_BITS);
    const size_t field_id = FieldID(index);
    const size_t pos_id = FieldPos(index);
    const field_t pos_mask = FIELD_1 << pos_id;

    bit_set[field_id] ^= pos_mask;

    return *this;
  }

  /// Flips all the bits in a range [start, stop)
  template <size_t NUM_BITS>
  BitSet<NUM_BITS> & BitSet<NUM_BITS>::Toggle(size_t start, size_t stop) {
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
  BitSet<NUM_BITS> & BitSet<NUM_BITS>::Randomize(Random & random) {
    random.RandFill(BytePtr(), TOTAL_BYTES);
    ClearExcessBits();
    return *this;
  }

  /// Set all bits randomly, with probability specified at compile time.
  template <size_t NUM_BITS>
  template <Random::Prob P>
  BitSet<NUM_BITS> & BitSet<NUM_BITS>::RandomizeP(Random & random,
                                                  const size_t start_pos, const size_t stop_pos) {
    emp_assert(start_pos <= stop_pos);
    emp_assert(stop_pos <= NUM_BITS);
    random.RandFillP<P>(BytePtr(), TOTAL_BYTES, start_pos, stop_pos);
    return *this;
  }


  /// Set all bits randomly, with a given probability of being on.
  template <size_t NUM_BITS>
  BitSet<NUM_BITS> & BitSet<NUM_BITS>::Randomize(Random & random, const double p,
                                                 const size_t start_pos, const size_t stop_pos) {
    emp_assert(start_pos <= stop_pos);
    emp_assert(stop_pos <= NUM_BITS);
    emp_assert(p >= 0.0 && p <= 1.0, p);
    random.RandFill(BytePtr(), TOTAL_BYTES, p, start_pos, stop_pos);
    return *this;
  }

  /// Set all bits randomly, with a given number of them being on.
  template <size_t NUM_BITS>
  BitSet<NUM_BITS> & BitSet<NUM_BITS>::Randomize(Random & random, const size_t target_ones,
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
  BitSet<NUM_BITS> & BitSet<NUM_BITS>::FlipRandom(Random & random,
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
  BitSet<NUM_BITS> & BitSet<NUM_BITS>::SetRandom(Random & random,
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
  BitSet<NUM_BITS> & BitSet<NUM_BITS>::ClearRandom(Random & random,
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
  BitSet<NUM_BITS> & BitSet<NUM_BITS>::FlipRandom(Random & random,
                                                  const size_t num_bits)
  {
    emp_assert(num_bits <= NUM_BITS);
    this_t target_bits(random, num_bits);
    return *this ^= target_bits;
  }

  /// Set a specified number of random bits (does not check if already set.)
  template <size_t NUM_BITS>
  BitSet<NUM_BITS> & BitSet<NUM_BITS>::SetRandom(Random & random,
                                                 const size_t num_bits)
  {
    emp_assert(num_bits <= NUM_BITS);
    this_t target_bits(random, num_bits);
    return *this |= target_bits;
  }

  /// Unset  a specified number of random bits (does not check if already zero.)
  template <size_t NUM_BITS>
  BitSet<NUM_BITS> & BitSet<NUM_BITS>::ClearRandom(Random & random,
                                                   const size_t num_bits)
  {
    emp_assert(num_bits <= NUM_BITS);
    this_t target_bits(random, NUM_BITS - num_bits);
    return *this &= target_bits;
  }


  // -------------------------  Implementations of Comparison Operators -------------------------

  /// Test if two BitSet objects are identical.
  template <size_t NUM_BITS>
  template <size_t SIZE2>
  bool BitSet<NUM_BITS>::operator==(const BitSet<SIZE2> & in_set) const {
    if constexpr (NUM_BITS != SIZE2) return false;

    for (size_t i = 0; i < NUM_FIELDS; ++i) {
      if (bit_set[i] != in_set.bit_set[i]) return false;
    }
    return true;
  }

  /// Compare two BitSet objects, based on the associated binary value.
  template <size_t NUM_BITS>
  template <size_t SIZE2>
  bool BitSet<NUM_BITS>::operator<(const BitSet<SIZE2> & in_set) const {
    if constexpr (NUM_BITS != SIZE2) return NUM_BITS < SIZE2;

    for (int i = NUM_FIELDS-1; i >= 0; --i) {         // Start loop at the largest field.
      if (bit_set[i] == in_set.bit_set[i]) continue;  // If same, keep looking!
      return (bit_set[i] < in_set.bit_set[i]);        // Otherwise, do comparison
    }
    return false;
  }

  // -------------------------  Extra Functions  -------------------------

  template <size_t NUM_BITS1, size_t NUM_BITS2>
  BitSet<NUM_BITS1+NUM_BITS2> join(const BitSet<NUM_BITS1> & in1, const BitSet<NUM_BITS2> & in2) {
    BitSet<NUM_BITS1+NUM_BITS2> out_bits;
    out_bits.Import(in2);
    out_bits <<= NUM_BITS1;
    out_bits |= in2.template Export<NUM_BITS1+NUM_BITS2>();
  }

  /// Computes simple matching coefficient (https://en.wikipedia.org/wiki/Simple_matching_coefficient).
  template <size_t NUM_BITS>
  double SimpleMatchCoeff(const BitSet<NUM_BITS> & in1, const BitSet<NUM_BITS> & in2) {
    emp_assert(NUM_BITS > 0); // TODO: can be done with XOR
    return (double)(~(in1 ^ in2)).CountOnes() / (double) NUM_BITS;
  }

}

/// For hashing BitSets
namespace std
{
    template <size_t N>
    struct hash<emp::BitSet<N>>
    {
        size_t operator()( const emp::BitSet<N>& bs ) const
        {
          return emp::murmur_hash(bs.GetBytes());
        }
    };
}


#endif
