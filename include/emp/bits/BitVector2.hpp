/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2020.
 *
 *  @file  BitVector.hpp
 *  @brief A drop-in replacement for std::vector<bool>, with additional bitwise logic features.
 *  @note Status: RELEASE
 *
 *  Compile with -O3 and -msse4.2 for fast bit counting.
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


#ifndef EMP_BIT_VECTOR2_H
#define EMP_BIT_VECTOR2_H

#include <iostream>
#include <bitset>

#include "../base/assert.hpp"
#include "../base/Ptr.hpp"
#include "../base/vector.hpp"
#include "../math/math.hpp"
#include "../tools/functions.hpp"
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
    static constexpr size_t FIELD_SIZE = sizeof(field_t); ///< Number of bytes in a field
    static constexpr size_t FIELD_BITS = FIELD_SIZE*8;    ///< Number of bits in a field

    static constexpr field_t FIELD_0 = (field_t) 0;       ///< All bits in a field set to 0
    static constexpr field_t FIELD_1 = (field_t) 1;       ///< Least significant bit set to 1
    static constexpr field_t FIELD_255 = (field_t) 255;   ///< Least significant 8 bits set to 1
    static constexpr field_t FIELD_ALL = ~FIELD_0;        ///< All bits in a field set to 1

    size_t num_bits;        ///< Total number of bits are we using
    Ptr<field_t> bits;      ///< Pointer to array with the status of each bit

    /// Num bits used in partial field at the end; 0 if perfect fit.
    size_t NumEndBits() const { return num_bits & (FIELD_BITS - 1); }

    /// How many feilds do we need?
    size_t NumFields() const { return num_bits ? (1 + ((num_bits - 1) / FIELD_BITS)) : 0; }

    /// How many bytes are used in the current vector (round up to whole bytes.)
    size_t NumBytes()  const { return num_bits ? (1 + ((num_bits - 1) >> 3)) : 0; }

    /// BitProxy lets us use operator[] on with BitVector as an lvalue.
    class BitProxy {
    private:
      BitVector & bit_vector;  ///< Which BitVector does this proxy belong to?
      size_t index;            ///< Which position in the bit vector does this proxy point at?

      // Helper functions.
      inline bool Get() const { return bit_vector.Get(index); }
      inline BitProxy & Set(bool b) { bit_vector.Set(index, b); return *this; }
      inline BitProxy & Toggle() { bit_vector.Toggle(index); return *this; }
      inline BitProxy & SetIf(bool test, bool b) { if (test) Set(b); return *this; }
      inline BitProxy & ToggleIf(bool test) { if (test) Toggle(); return *this; }

    public:
      /// Setup a new proxy with the associated vector and index.
      BitProxy(BitVector & _v, size_t _idx) : bit_vector(_v), index(_idx) {;}

      /// Assignment operator to the bit associated with this proxy (as an lvalue).
      BitProxy & operator=(bool b) { return Set(b); }

      /// Conversion of this proxy to Boolean (as an rvalue)
      operator bool() const { return Get(); }

      // Compound assignement operators with BitProxy as the lvalue.
      BitProxy & operator &=(bool b) { return SetIf(!b, 0); }
      BitProxy & operator *=(bool b) { return SetIf(!b, 0); }
      BitProxy & operator |=(bool b) { return SetIf(b, 1); }
      BitProxy & operator +=(bool b) { return SetIf(b, 1); }
      BitProxy & operator -=(bool b) { return SetIf(b, 0); }

      /// Compound assignement operator XOR using BitProxy as lvalue.
      BitProxy & operator ^=(bool b) { return ToggleIf(b); }

      /// Compound assignement operator DIV using BitProxy as lvalue.
      /// @note Never use this function except for consistency in a template since must divide by 1.
      BitProxy & operator /=(bool b) {
        emp_assert(b == true, "BitVector Division by Zero error.");
        return *this;
      }
    }; // --- End of BitProxy

    /// Identify the field that a specified bit is in.
    static constexpr size_t FieldID(const size_t index)  { return index / FIELD_BITS; }

    /// Identify the position within a field where a specified bit is.
    static constexpr size_t FieldPos(const size_t index) { return index & (FIELD_BITS-1); }

    /// Identify which field a specified byte position would be in.
    static constexpr size_t Byte2Field(const size_t index) { return index / FIELD_SIZE; }

    /// Convert a byte position in BitVector to a byte position in the target field.
    static constexpr size_t Byte2FieldPos(const size_t index) {
      return (index & (FIELD_SIZE-1)) << 3;
    }

    /// Assume that the size of the bits has already been adjusted to be the size of the one
    /// being copied and only the fields need to be copied over.
    void RawCopy(const Ptr<field_t> in) {
      #ifdef EMP_TRACK_MEM
      emp_assert(in.IsNull() == false);
      emp_assert(bits.DebugIsArray() && in.DebugIsArray());
      emp_assert(bits.DebugGetArrayBytes() == in.DebugGetArrayBytes(),
                 bits.DebugGetArrayBytes(), in.DebugGetArrayBytes());
      #endif

      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = in[i];
    }

    /// Any bits past the last "real" on in the last field should be kept as zeros.
    void ClearExcessBits() {
      if (NumEndBits() > 0) bits[NumFields() - 1] &= MaskLow<field_t>(NumEndBits());
    }

    /// Helper: call SHIFT with positive number
    void ShiftLeft(const size_t shift_size) {
      const size_t field_shift = shift_size / FIELD_BITS;
      const size_t bit_shift = shift_size % FIELD_BITS;
      const size_t bit_overflow = FIELD_BITS - bit_shift;
      const size_t NUM_FIELDS = NumFields();

      // Loop through each field, from L to R, and update it.
      if (field_shift) {
        for (size_t i = NUM_FIELDS; i > field_shift; --i) {
          bits[i-1] = bits[i - field_shift - 1];
        }
        for (size_t i = field_shift; i > 0; --i) bits[i-1] = 0;
      }

      // account for bit_shift
      if (bit_shift) {
        for (size_t i = NUM_FIELDS - 1; i > field_shift; --i) {
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
    void ShiftRight(const size_t shift_size) {
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
        for (size_t i = field_shift2; i < NUM_FIELDS; i++) bits[i] = 0U;
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

  public:
    /// Build a new BitVector with specified bit count (default 0) and initialization (default 0)
    BitVector(size_t in_num_bits=0, bool init_val=false) : num_bits(in_num_bits), bits(nullptr) {
      if (num_bits) bits = NewArrayPtr<field_t>(NumFields());
      if (init_val) SetAll(); else Clear();
    }

    /// Copy constructor of existing bit field.
    BitVector(const BitVector & in) : num_bits(in.num_bits), bits(nullptr) {
      #ifdef EMP_TRACK_MEM
      emp_assert(in.bits.IsNull() || in.bits.DebugIsArray());
      emp_assert(in.bits.OK());
      #endif

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
    BitVector(BitVector && in) : num_bits(in.num_bits), bits(in.bits) {
      #ifdef EMP_TRACK_MEM
      emp_assert(bits == nullptr || bits.DebugIsArray());
      emp_assert(bits.OK());
      #endif

      in.bits = nullptr;
      in.num_bits = 0;
    }

    /// Copy, but with a resize.
    BitVector(const BitVector & in, size_t new_size) : BitVector(in) {
      if (num_bits != new_size) Resize(new_size);
    }

    /// Destructor
    ~BitVector() {
      if (bits) {        // A move constructor can make bits == nullptr
        bits.DeleteArray();
        bits = nullptr;
      }
    }

    /// Assignment operator.
    BitVector & operator=(const BitVector & in) {
      #ifdef EMP_TRACK_MEM
      emp_assert(in.bits == nullptr || in.bits.DebugIsArray());
      emp_assert(in.bits != nullptr || in.num_bits == 0);
      emp_assert(in.bits.OK());
      #endif

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
    BitVector & operator=(BitVector && in) {
      emp_assert(&in != this);        // in is an r-value, so this shouldn't be possible...
      if (bits) bits.DeleteArray();   // If we already had a bitset, get rid of it.
      num_bits = in.num_bits;         // Update the number of bits...
      bits = in.bits;                 // And steal the old memory for what those bits are.
      in.bits = nullptr;              // Prepare in for deletion without deallocating.
      in.num_bits = 0;

      return *this;
    }

    /// Automatically convert BitVector to other vector types.
    template <typename T>
    operator emp::vector<T>() {
      emp::vector<T> out(GetSize());
      for (size_t i = 0; i < GetSize(); i++) {
        out[i] = (T) Get(i);
      }
      return out;
    }

    /// Resize this BitVector to have the specified number of bits.
    BitVector & Resize(size_t new_bits) {
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
        for (size_t i = min_fields; i < NUM_FIELDS; i++) bits[i] = 0U;
        if (old_bits) old_bits.DeleteArray();
      }

      return *this;
    }

    /// Test if two bit vectors are identical.
    bool operator==(const BitVector & in) const {
      if (num_bits != in.num_bits) return false;

      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; ++i) {
        if (bits[i] != in.bits[i]) return false;
      }
      return true;
    }

    /// Compare the would-be numerical values of two bit vectors.
    bool operator<(const BitVector & in) const {
      if (num_bits != in.num_bits) return num_bits < in.num_bits;

      const size_t NUM_FIELDS = NumFields();
      for (size_t i = NUM_FIELDS; i > 0; --i) {   // Start loop at the largest field.
        const size_t pos = i-1;
        if (bits[pos] == in.bits[pos]) continue;  // If same, keep looking!
        return (bits[pos] < in.bits[pos]);        // Otherwise, do comparison
      }
      return false; // Bit vectors are identical.
    }

    /// Compare the would-be numerical values of two bit vectors.
    bool operator<=(const BitVector & in) const {
      if (num_bits != in.num_bits) return num_bits <= in.num_bits;

      const size_t NUM_FIELDS = NumFields();
      for (size_t i = NUM_FIELDS; i > 0; --i) {   // Start loop at the largest field.
        const size_t pos = i-1;
        if (bits[pos] == in.bits[pos]) continue;  // If same, keep looking!
        return (bits[pos] < in.bits[pos]);        // Otherwise, do comparison
      }
      return true;
    }

    /// Determine if two bit vectors are different.
    bool operator!=(const BitVector & in) const { return !operator==(in); }

    /// Compare the would-be numerical values of two bit vectors.
    bool operator>(const BitVector & in) const { return !operator<=(in); }

    /// Compare the would-be numerical values of two bit vectors.
    bool operator>=(const BitVector & in) const { return !operator<(in); }

    /// How many bits do we currently have?
    size_t GetSize() const { return num_bits; }

    /// Retrive the bit value from the specified index.
    bool Get(size_t index) const {
      emp_assert(index < num_bits, index, num_bits);
      const size_t field_id = FieldID(index);
      const size_t pos_id = FieldPos(index);
      return (bits[field_id] & (static_cast<field_t>(1) << pos_id)) != 0;
    }

    /// A safe version of Get() for indexing out of range. Typically used when a BitVector
    /// represents a collection.
    bool Has(size_t index) const {
      return (index < num_bits) ? Get(index) : false;
    }

    /// Update the bit value at the specified index.
    BitVector & Set(size_t index, bool value=true) {
      emp_assert(index < num_bits, index, num_bits);
      const size_t field_id = FieldID(index);
      const size_t pos_id = FieldPos(index);
      const field_t pos_mask = FIELD_1 << pos_id;

      if (value) bits[field_id] |= pos_mask;
      else       bits[field_id] &= ~pos_mask;

      return *this;
    }

    /// Change every bit in the sequence.
    BitVector & Toggle() { return NOT_SELF(); }

    /// Change a specified bit to the opposite value
    BitVector & Toggle(size_t index) {
      emp_assert(index < num_bits, index, num_bits);
      const size_t field_id = FieldID(index);
      const size_t pos_id = FieldPos(index);
      const field_t pos_mask = FIELD_1 << pos_id;

      bits[field_id] ^= pos_mask;

      return *this;
    }

    /// Flips all the bits in a range [start, end)
    BitVector & Toggle(size_t start, size_t stop) {
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

    /// A simple hash function for bit vectors.
    std::size_t Hash() const {
      std::size_t hash_val = 0;
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) {
        hash_val ^= bits[i];
      }
      return hash_val ^ ((97*num_bits) << 8);
    }

    /// Retrive the byte at the specified byte index.
    uint8_t GetByte(size_t index) const {
      emp_assert(index < NumBytes(), index, NumBytes());
      const size_t field_id = Byte2Field(index);
      const size_t pos_id = Byte2FieldPos(index);
      return (bits[field_id] >> pos_id) & 255U;
    }

    /// Update the byte at the specified byte index.
    void SetByte(size_t index, uint8_t value) {
      emp_assert(index < NumBytes(), index, NumBytes());
      const size_t field_id = Byte2Field(index);
      const size_t pos_id = Byte2FieldPos(index);
      const field_t val_uint = value;
      bits[field_id] = (bits[field_id] & ~(FIELD_255 << pos_id)) | (val_uint << pos_id);
    }

   // Retrieve the 32-bit uint from the specified uint index (based on bitset.h GetUInt32)
    uint32_t GetUInt(size_t index) const {
      emp_assert(index * 32 < num_bits);

      uint32_t res;

      std::memcpy(
        &res,
        bits.Cast<unsigned char>().Raw() + index * (32/8),
        sizeof(res)
      );

      return res;
    }

    /// Update the 32-bit uint at the specified uint index.
    void SetUInt(const size_t index, uint32_t value) {
      emp_assert(index * 32 < num_bits);

      std::memcpy(
        bits.Cast<unsigned char>().Raw() + index * (32/8),
        &value,
        sizeof(value)
      );

      // Check to make sure that if there are any end bits, there are no excess ones.
      emp_assert(NumEndBits() == 0 ||
                 ( bits[NumFields() - 1] & ~MaskLow<field_t>(NumEndBits()) ) == 0 );

    }

    /// Set a 32-bit uint at the specified BIT index.
    void SetUIntAtBit(size_t index, uint32_t value) {
      const size_t field_id = FieldID(index);
      const size_t field_pos = FieldPos(index);
      const field_t mask1 = MaskLow<field_t>(field_pos);
      const size_t end_pos = field_pos + 32;
      const size_t overshoot = (end_pos > FIELD_BITS) ? end_pos - FIELD_BITS : 0;
      const field_t mask2 = ~MaskLow<field_t>(overshoot);

      emp_assert(index+32 <= num_bits);
      emp_assert(!overshoot || field_id+1 < NumFields());

      // Clear bits that we are setting 1's then OR in new value.
      bits[field_id] &= mask1;
      bits[field_id] |= ((field_t) value) >> field_pos;

      // Repeat for next field if needed.
      if (overshoot) {
        bits[field_id+1] &= mask2;
        bits[field_id+1] |= ((field_t) value) << (32-overshoot);
      }
    }

    /// Retrive the 32-bit uint at the specified BIT index.
    uint32_t GetUIntAtBit(size_t index) {
      // @CAO Need proper assert for non-32-size bit fields!
      // emp_assert(index < num_bits);
      const size_t field_id = FieldID(index);
      const size_t pos_id = FieldPos(index);
      if (pos_id == 0) return (uint32_t) bits[field_id];
      const size_t NUM_FIELDS = NumFields();
      const uint32_t part1 = (uint32_t) (bits[field_id] >> pos_id);
      const uint32_t part2 =
        (uint32_t)((field_id+1 < NUM_FIELDS) ? bits[field_id+1] << (FIELD_BITS-pos_id) : 0);
      return part1 | part2;
    }

    /// Retrieve the specified number of bits (stored in the field type) at the target bit index.
    template <size_t OUT_BITS>
    field_t GetValueAtBit(size_t index) {
      // @CAO This function needs to be generalized to return more then one field of bits.
      static_assert(OUT_BITS <= FIELD_BITS, "Requesting too many bits to fit in a field");
      return GetUIntAtBit(index) & MaskLow<field_t>(OUT_BITS);
    }

    /// Return true if ANY bits are set to 1, otherwise return false.
    bool Any() const {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) {
        if (bits[i]) return true;
      }
      return false;
    }

    /// Return true if NO bits are set to 1, otherwise return false.
    bool None() const { return !Any(); }

    /// Return true if ALL bits are set to 1, otherwise return false.
    // @CAO: Can speed up by not duplicating the whole BitVector.
    bool All() const { return (~(*this)).None(); }

    /// Casting a bit array to bool identifies if ANY bits are set to 1.
    explicit operator bool() const { return Any(); }

    /// Const index operator -- return the bit at the specified position.
    bool operator[](size_t index) const { return Get(index); }

    /// Index operator -- return a proxy to the bit at the specified position so it can be an lvalue.
    BitProxy operator[](size_t index) { return BitProxy(*this, index); }

    /// Set all bits to 0.
    BitVector & Clear() {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = 0U;
      return *this;
    }

    /// Set specific bit to 0.
    BitVector & Clear(size_t index) {
      return Set(index, false);
    }

    /// Set a range of bits to 0 in the range [start, stop)
    BitVector & Clear(const size_t start, const size_t stop) {
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

    /// Set all bits to 1.
    BitVector & SetAll() {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = FIELD_ALL;
      ClearExcessBits();
      return *this;
    }

    /// Set a range of bits to one: [start, stop)
    BitVector & SetRange(size_t start, size_t stop) {
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

    /// Convert this BitVector to a string.
    std::string ToString() const {
      std::string out_string;
      out_string.reserve(num_bits);
      for (size_t i = num_bits; i > 0; --i) out_string.push_back('0' + Get(i-1));
      return out_string;
    }

    /// Regular print function (from most significant bit to least)
    void Print(std::ostream & out=std::cout) const {
      for (size_t i = num_bits; i > 0; --i) out << Get(i-1);
    }

    /// Print a space between each field (or other provided spacer)
    void PrintFields(std::ostream & out=std::cout, const std::string & spacer=" ") const {
      for (size_t i = num_bits-1; i < num_bits; i--) {
        out << Get(i);
        if (i && (i % FIELD_BITS == 0)) out << spacer;
      }
    }

    /// Print from smallest bit position to largest.
    void PrintArray(std::ostream & out=std::cout) const {
      for (size_t i = 0; i < num_bits; i++) out << Get(i);
    }

    /// Print the positions of all one bits, spaces are the default separator.
    void PrintOneIDs(std::ostream & out=std::cout, const std::string & spacer=" ") const {
      for (size_t i = 0; i < num_bits; i++) { if (Get(i)) out << i << spacer; }
    }

    /// Print the ones in a range format.  E.g., 2-5,7,10-15
    void PrintAsRange(std::ostream & out=std::cout,
                      const std::string & spacer=",",
                      const std::string & ranger="-") const
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

    /// Count 1's by looping through once for each bit equal to 1
    size_t CountOnes_Sparse() const {
      const size_t NUM_FIELDS = NumFields();
      size_t bit_count = 0;
      for (size_t i = 0; i < NUM_FIELDS; i++) {
        field_t cur_field = bits[i];
        while (cur_field) {
          cur_field &= (cur_field-1);       // Peel off a single 1.
          bit_count++;      // And increment the counter
        }
      }
      return bit_count;
    }
    // TODO: see https://arxiv.org/pdf/1611.07612.pdf for faster pop counts
    size_t CountOnes_Mixed() const {
      const field_t NUM_FIELDS = (1 + ((num_bits - 1) / FIELD_BITS));
      size_t bit_count = 0;
      for (size_t i = 0; i < NUM_FIELDS; i++) {
          // when compiling with -O3 and -msse4.2, this is the fastest population count method.
          std::bitset<FIELD_BITS> std_bs(bits[i]);
          bit_count += std_bs.count();
       }

      return bit_count;
    }

    /// Count the number of ones in the BitVector.
    size_t CountOnes() const { return CountOnes_Mixed(); }

    /// Count the number of zeros in the BitVector.
    size_t CountZeros() const { return GetSize() - CountOnes(); }

    /// Return the position of the first one; return -1 if no ones in vector.
    int FindBit() const {
      const size_t NUM_FIELDS = NumFields();
      size_t field_id = 0;
      while (field_id < NUM_FIELDS && bits[field_id]==0) field_id++;
      return (field_id < NUM_FIELDS) ?
        (int) (find_bit(bits[field_id]) + (field_id * FIELD_BITS))  :  -1;
    }

    /// Return the position of the first one and change it to a zero.  Return -1 if no ones.
    int PopBit() {
      const size_t NUM_FIELDS = NumFields();
      size_t field_id = 0;
      while (field_id < NUM_FIELDS && bits[field_id]==0) field_id++;
      if (field_id == NUM_FIELDS) return -1;  // Failed to find bit!

      const size_t pos_found = find_bit(bits[field_id]);
      bits[field_id] &= ~(FIELD_1 << pos_found);
      return (int) (pos_found + (field_id * FIELD_BITS));
    }

    /// Return the position of the first one after start_pos; return -1 if no ones in vector.
    /// You can loop through all 1-bit positions of a BitVector "bv" with:
    ///
    ///   for (int pos = bv.FindBit(); pos >= 0; pos = bv.FindBit(pos+1)) { ... }

    int FindBit(const size_t start_pos) const {
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

    /// Return positions of all ones.
    emp::vector<size_t> GetOnes() const {
      // @CAO -- There are probably better ways to do this with bit tricks.
      emp::vector<size_t> out_vals(CountOnes());
      size_t cur_pos = 0;
      for (size_t i = 0; i < num_bits; i++) {
        if (Get(i)) out_vals[cur_pos++] = i;
      }
      return out_vals;
    }

    /// Perform a Boolean NOT on this BitVector and return the result.
    BitVector NOT() const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_bv(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_bv.bits[i] = ~bits[i];
      out_bv.ClearExcessBits();
      return out_bv;
    }

    /// Perform a Boolean AND on this BitVector and return the result.
    BitVector AND(const BitVector & bv2) const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_bv(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_bv.bits[i] = bits[i] & bv2.bits[i];
      return out_bv;
    }

    /// Perform a Boolean OR on this BitVector and return the result.
    BitVector OR(const BitVector & bv2) const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_bv(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_bv.bits[i] = bits[i] | bv2.bits[i];
      return out_bv;
    }

    /// Perform a Boolean NAND on this BitVector and return the result.
    BitVector NAND(const BitVector & bv2) const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_bv(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_bv.bits[i] = ~(bits[i] & bv2.bits[i]);
      out_bv.ClearExcessBits();
      return out_bv;
    }

    /// Perform a Boolean NOR on this BitVector and return the result.
    BitVector NOR(const BitVector & bv2) const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_bv(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_bv.bits[i] = ~(bits[i] | bv2.bits[i]);
      out_bv.ClearExcessBits();
      return out_bv;
    }

    /// Perform a Boolean XOR on this BitVector and return the result.
    BitVector XOR(const BitVector & bv2) const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_bv(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_bv.bits[i] = bits[i] ^ bv2.bits[i];
      return out_bv;
    }

    /// Perform a Boolean EQU on this BitVector and return the result.
    BitVector EQU(const BitVector & bv2) const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_bv(*this);
      for (size_t i = 0; i < NUM_FIELDS; i++) out_bv.bits[i] = ~(bits[i] ^ bv2.bits[i]);
      out_bv.ClearExcessBits();
      return out_bv;
    }


    /// Perform a Boolean NOT with this BitVector, store result here, and return this object.
    BitVector & NOT_SELF() {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = ~bits[i];
      ClearExcessBits();
      return *this;
    }

    /// Perform a Boolean AND with this BitVector, store result here, and return this object.
    BitVector & AND_SELF(const BitVector & bv2) {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = bits[i] & bv2.bits[i];
      return *this;
    }

    /// Perform a Boolean OR with this BitVector, store result here, and return this object.
    BitVector & OR_SELF(const BitVector & bv2) {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = bits[i] | bv2.bits[i];
      return *this;
    }

    /// Perform a Boolean NAND with this BitVector, store result here, and return this object.
    BitVector & NAND_SELF(const BitVector & bv2) {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = ~(bits[i] & bv2.bits[i]);
      ClearExcessBits();
      return *this;
    }

    /// Perform a Boolean NOR with this BitVector, store result here, and return this object.
    BitVector & NOR_SELF(const BitVector & bv2) {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = ~(bits[i] | bv2.bits[i]);
      ClearExcessBits();
      return *this;
    }

    /// Perform a Boolean XOR with this BitVector, store result here, and return this object.
    BitVector & XOR_SELF(const BitVector & bv2) {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = bits[i] ^ bv2.bits[i];
      return *this;
    }

    /// Perform a Boolean EQU with this BitVector, store result here, and return this object.
    BitVector & EQU_SELF(const BitVector & bv2) {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) bits[i] = ~(bits[i] ^ bv2.bits[i]);
      ClearExcessBits();
      return *this;
    }

    /// Positive shifts go left and negative go right (0 does nothing); return result.
    BitVector SHIFT(const int shift_size) const {
      BitVector out_bv(*this);
      if (shift_size > 0) out_bv.ShiftRight((size_t) shift_size);
      else if (shift_size < 0) out_bv.ShiftLeft((size_t) -shift_size);
      return out_bv;
    }

    /// Positive shifts go left and negative go right; store result here, and return this object.
    BitVector & SHIFT_SELF(const int shift_size) {
      if (shift_size > 0) ShiftRight((size_t) shift_size);
      else if (shift_size < 0) ShiftLeft((size_t) -shift_size);
      return *this;
    }


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
    const BitVector & operator&=(const BitVector & ar2) { return AND_SELF(ar2); }

    /// Compound operator bitwise OR...
    const BitVector & operator|=(const BitVector & ar2) { return OR_SELF(ar2); }

    /// Compound operator bitwise XOR...
    const BitVector & operator^=(const BitVector & ar2) { return XOR_SELF(ar2); }

    /// Compound operator for shift left...
    const BitVector & operator<<=(const size_t shift_size) { return SHIFT_SELF(-(int)shift_size); }

    /// Compound operator for shift right...
    const BitVector & operator>>=(const size_t shift_size) { return SHIFT_SELF((int)shift_size); }

    /// STL COMPATABILITY
    /// A set of functions to allow drop-in replacement with std::bitset.
    size_t size() const { return num_bits; }
    void resize(std::size_t new_size) { Resize(new_size); }
    bool all() const { return All(); }
    bool any() const { return Any(); }
    bool none() const { return !Any(); }
    size_t count() const { return CountOnes_Mixed(); }
    BitVector & flip() { return Toggle(); }
    BitVector & flip(size_t pos) { return Toggle(pos); }
    BitVector & flip(size_t start, size_t end) { return Toggle(start, end); }
    void reset() { Clear(); }
    void reset(size_t id) { Set(id, false); }
    void set() { SetAll(); }
    void set(size_t id) { Set(id); }
    bool test(size_t index) const { return Get(index); }
  };

}

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
