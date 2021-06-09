/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2018
 *
 *  @file  BitSet.hpp
 *  @brief A drop-in replacement for std::bitset, with additional bit magic features.
 *  @note Status: RELEASE
 *
 *  @note Like std::bitset, bit zero is on the right side.  Unlike std::bitset, emp::BitSet
 *       gives access to bit fields for easy access to different sized chucnk of bits and
 *       implementation new bit-magic tricks.
 */


#ifndef EMP_BIT_SET_HPP
#define EMP_BIT_SET_HPP

#include <iostream>
#include <initializer_list>
#include <cstring>
#include <bitset>

#include "../base/assert.hpp"
#include "../base/vector.hpp"
#include "../base/Ptr.hpp"
#include "../datastructs/hash_utils.hpp"
#include "../math/math.hpp"
#include "../math/Random.hpp"
#include "../math/random_utils.hpp"
#include "../polyfill/span.hpp"
#include "bitset_utils.hpp"

namespace emp {

  /// SFINAE helper to determine field_t for BitSet
  template <size_t NUM_BITS> struct FieldHelper {
#ifdef __EMSCRIPTEN__
    ///< Field sizes are 32 bits in Emscripten (max directly handled)
    using field_t = uint32_t;
#else
    ///< Field sizes are 64 bits in native, unless NUM_BITS == 32
    using field_t = uint64_t;
#endif
  };

  template <> struct FieldHelper<32> {
    // if NUM_BITS == 32, use uint32_t
    using field_t = uint32_t;
  };

  ///  A fixed-sized (but arbitrarily large) array of bits, and optimizes operations on those bits
  ///  to be as fast as possible.
  template <size_t NUM_BITS> class BitSet {

  // make all templated instantiations friends with each other
  template<size_t FRIEND_BITS> friend class BitSet;

  private:

    ///< field size is 64 for native (except NUM_BITS == 32), 32 for emscripten
    using field_t = typename FieldHelper<NUM_BITS>::field_t;

    ///< How many bytes are in a field?
    static constexpr field_t FIELD_BYTES = sizeof(field_t);

    ///< How many bits are in a field?
    static constexpr field_t FIELD_BITS = 8 * FIELD_BYTES;

    static constexpr field_t FIELD_LOG2 = emp::Log2(FIELD_BITS);

    /// Fields hold bits in groups of 32 or 64 (as uint32_t or uint64_t);
    /// how many fields do we need?
    static constexpr field_t NUM_FIELDS = (1 + ((NUM_BITS - 1) / FIELD_BITS));

    /// End position of the stored bits in the last field; 0 if perfect fit.
    static constexpr field_t LAST_BIT = NUM_BITS & (FIELD_BITS - 1);

    /// How many total bytes are needed to represent these bits? (rounded up to full bytes)
    static const field_t NUM_BYTES = 1 + ((NUM_BITS - 1) >> 3);

    field_t bit_set[NUM_FIELDS];  ///< Fields to hold the actual bits for this BitSet.

    /// BitProxy lets us use operator[] on with BitSet as an lvalue.
    class BitProxy {
    private:
      BitSet<NUM_BITS> & bit_set;  ///< BitSet object that this proxy refers to.
      size_t index;                ///< Position in BitSet the this proxy refers to.
    public:
      BitProxy(BitSet<NUM_BITS> & _set, size_t _idx) : bit_set(_set), index(_idx) {
        emp_assert(_idx < bit_set.size());
      }

      /// Set the bit value that this proxy refers to.
      BitProxy & operator=(bool b) {    // lvalue handling...
        bit_set.Set(index, b);
        return *this;
      }

      /// Convert BitProxy to a regular boolean value.
      operator bool() const {            // rvalue handling...
        return bit_set.Get(index);
      }

      /// Flip this bit.
      BitProxy & Toggle() { bit_set.Toggle(index); return *this; }
    };
    friend class BitProxy;

    inline static size_t FieldID(const size_t index) {
      emp_assert((index >> FIELD_LOG2) < NUM_FIELDS);
      return index >> FIELD_LOG2;
    }

    inline static size_t FieldPos(const size_t index) {
      return index & (FIELD_BITS - 1);
    }

    inline static size_t Byte2Field(const size_t index) {
      return index / FIELD_BYTES;
    }

    inline static size_t Byte2FieldPos(const size_t index) {
      return FieldPos(index * 8);
    }

    inline void Copy(const field_t in_set[NUM_FIELDS]) {
      std::memcpy(bit_set, in_set, sizeof(bit_set));
    }

    template<size_t shift>
    void ShiftLeft() {
      // profiled this templated, special case variant
      // and did see a difference in runtime MAM

      // TODO currently only implemented for NUM_FIELDS == 1
      //static_assert( NUM_FIELDS == 1 );
      //static_assert( LAST_BIT == 0 );
      if constexpr (NUM_FIELDS != 1) {
        ShiftLeft(shift);

      }
      else {
        if constexpr (LAST_BIT != 0)
            bit_set[NUM_FIELDS - 1] &= MaskLow<field_t>(LAST_BIT);
        bit_set[0] <<= shift;
      }
    }

    /// Helper: call SHIFT with positive number instead
    void ShiftLeft(const size_t shift_size) {

      if (shift_size > NUM_BITS) {
        Clear();
        return;
      }

      const int field_shift = shift_size / FIELD_BITS;
      const int bit_shift = shift_size % FIELD_BITS;
      const int bit_overflow = FIELD_BITS - bit_shift;

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
      if (LAST_BIT) { bit_set[NUM_FIELDS - 1] &= MaskLow<field_t>(LAST_BIT); }
    }


    /// Helper for calling SHIFT with negative number
    void ShiftRight(const size_t shift_size) {
      if (!shift_size) return;

      const field_t field_shift = shift_size / FIELD_BITS;

      // only clear and return if we are field_shift-ing
      // we want to be able to always shift by up to a byte
      // so that Import and Export work
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
        for (size_t i = 0; i < (NUM_FIELDS - 1 - field_shift); ++i) {
          bit_set[i] >>= bit_shift;
          bit_set[i] |= (bit_set[i+1] << bit_overflow);
        }
        bit_set[NUM_FIELDS - 1 - field_shift] >>= bit_shift;
      }
    }

    /// Helper: call ROTATE with negative number instead
    void RotateLeft(const size_t shift_size_raw) {
      const field_t shift_size = shift_size_raw % NUM_BITS;

      // use different approaches based on BitSet size
      if constexpr (NUM_FIELDS == 1) {
        // special case: for exactly one field_T, try to go low level
        // adapted from https://stackoverflow.com/questions/776508/best-practices-for-circular-shift-rotate-operations-in-c
        field_t & n = bit_set[0];
        field_t c = shift_size;

        // mask necessary to suprress shift count overflow warnings
        constexpr field_t mask = MaskLow<field_t>(FIELD_LOG2);

        c &= mask;
        n = (n<<c) | (n>>( (-(c+FIELD_BITS-NUM_BITS))&mask ));

      } else if (NUM_FIELDS < 32) {
        // for small BitSets, shifting L/R and ORing is faster
        emp::BitSet<NUM_BITS> dup(*this);
        dup.ShiftLeft(shift_size);
        ShiftRight(NUM_BITS - shift_size);
        OR_SELF(dup);
      } else {
        // for big BitSets, manual rotating is fater

        // note that we already modded shift_size by NUM_BITS
        // so there's no need to mod by FIELD_SIZE here
        const int field_shift = LAST_BIT ? (
          (shift_size + FIELD_BITS - LAST_BIT) / FIELD_BITS
        ) : (
          shift_size / FIELD_BITS
        );
        // if we field shift, we need to shift bits by (FIELD_BITS - LAST_BIT)
        // more to account for the filler that gets pulled out of the middle
        const int bit_shift = LAST_BIT && field_shift ? (
          (shift_size + FIELD_BITS - LAST_BIT) % FIELD_BITS
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
        if constexpr ((bool)LAST_BIT) {
          const int filler_idx = (NUM_FIELDS - 1 + field_shift) % NUM_FIELDS;
          for (int i = filler_idx + 1; i < (int)NUM_FIELDS; ++i) {
            bit_set[i-1] |= bit_set[i] << LAST_BIT;
            bit_set[i] >>= (FIELD_BITS - LAST_BIT);
          }
        }

        // account for bit_shift
        if (bit_shift) {

          const field_t keystone = LAST_BIT ? (
            (bit_set[NUM_FIELDS - 1] << (FIELD_BITS - LAST_BIT))
            | (bit_set[NUM_FIELDS - 2] >> LAST_BIT)
          ) : (
            bit_set[NUM_FIELDS - 1]
          );

          for (int i = NUM_FIELDS - 1; i > 0; --i) {
            bit_set[i] <<= bit_shift;
            bit_set[i] |= (bit_set[i-1] >> bit_overflow);
          }
          // Handle final field
          bit_set[0] <<= bit_shift;
          bit_set[0] |= keystone >> bit_overflow;

        }

      }

      // Mask out filler bits
      if constexpr ((bool)LAST_BIT) {
        bit_set[NUM_FIELDS - 1] &= MaskLow<field_t>(LAST_BIT);
      }

    }


    /// Helper for calling ROTATE with positive number
    void RotateRight(const size_t shift_size_raw) {

      const field_t shift_size = shift_size_raw % NUM_BITS;

      // use different approaches based on BitSet size
      if constexpr (NUM_FIELDS == 1) {
        // special case: for exactly one field_t, try to go low level
        // adapted from https://stackoverflow.com/questions/776508/best-practices-for-circular-shift-rotate-operations-in-c

        field_t & n = bit_set[0];
        field_t c = shift_size;

        // mask necessary to suprress shift count overflow warnings
        constexpr field_t mask = MaskLow<field_t>(FIELD_LOG2);

        c &= mask;
        n = (n>>c) | (n<<( (NUM_BITS-c)&mask ));

      } else if (NUM_FIELDS < 32) {
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
        if constexpr ((bool)LAST_BIT) {
          const int filler_idx = NUM_FIELDS - 1 - field_shift;
          for (int i = filler_idx + 1; i < (int)NUM_FIELDS; ++i) {
            bit_set[i-1] |= bit_set[i] << LAST_BIT;
            bit_set[i] >>= (FIELD_BITS - LAST_BIT);
          }
        }

        // account for bit_shift
        if (bit_shift) {

          const field_t keystone = LAST_BIT ? (
            bit_set[0] >> (FIELD_BITS - LAST_BIT)
          ) : (
            bit_set[0]
          );

          if constexpr ((bool)LAST_BIT) {
            bit_set[NUM_FIELDS-1] |= bit_set[0] << LAST_BIT;
          }

          for (size_t i = 0; i < NUM_FIELDS - 1; ++i) {
            bit_set[i] >>= bit_shift;
            bit_set[i] |= (bit_set[i+1] << bit_overflow);
          }
          bit_set[NUM_FIELDS - 1] >>= bit_shift;
          bit_set[NUM_FIELDS - 1] |= keystone << bit_overflow;
        }
      }

      // Mask out filler bits
      if constexpr ((bool)LAST_BIT) {
        bit_set[NUM_FIELDS - 1] &= MaskLow<field_t>(LAST_BIT);
      }

    }

  public:
    /// Constructor: Assume all zeroes in set
    BitSet() { Clear(); }

    /// Copy constructor from another BitSet
    BitSet(const BitSet & in_set) { Copy(in_set.bit_set); }

    /// Constructor to generate a random BitSet (with equal prob of 0 or 1).
    BitSet(Random & random) { Randomize(random); }

    /// Constructor to generate a random BitSet with provided prob of 1's.
    BitSet(Random & random, const double p1) { Clear(); Randomize(random, p1); }

    /// Constructor to generate a BitSet from a std::bitset.
    explicit BitSet(const std::bitset<NUM_BITS>& bitset) {
      Clear(); // have to clear out field bits beyond NUM_BITS
      for (size_t bit{}; bit < NUM_BITS; ++bit) Set( bit, bitset[bit] );
    }

    /// Constructor to generate a BitSet from a string.
    explicit BitSet(const std::string& bitstring)
    : BitSet( std::bitset<NUM_BITS>( bitstring ) )
    { emp_assert( bitstring.size() == NUM_BITS ); }

    /// Constructor to fill in a bit set from a vector.
    template <typename T>
    BitSet(const std::initializer_list<T> l) {
      // TODO: should we enforce the initializer list to be the same length as the bitset?
      // emp_assert(l.size() == NUM_BITS);

      // check that initializer list isn't longer than bitset
      emp_assert(l.size() <= NUM_BITS);

      Clear();

      size_t idx = 0;
      for (auto i = std::rbegin(l); i != std::rend(l); ++i) {
        Set(idx, *i);
        ++idx;
      }

    }

    /// Destructor.
    ~BitSet() = default;

    /// Assignment operator.
    BitSet & operator=(const BitSet<NUM_BITS> & in_set) {
      Copy(in_set.bit_set);
      return *this;
    }

    /// Set all bits randomly, with a 50% probability of being a 0 or 1.
    void Randomize(Random & random) {
      // Randomize all fields, then mask off bits in the last field if not complete.

      random.RandFill(
        reinterpret_cast<unsigned char*>(bit_set),
        (NUM_BITS+7)/8
      );

      if constexpr (static_cast<bool>(LAST_BIT)) {
        bit_set[NUM_FIELDS-1] &= MaskLow<field_t>(NUM_BITS%32);
      }

    }

    /// Set all bits randomly, with a given probability of being a 1.
    void Randomize(Random & random, const double p1) {
      if (p1 == 0.5) return Randomize(random); // If 0.5 probability, generate by field!
      for (size_t i = 0; i < NUM_BITS; i++) Set(i, random.P(p1));
    }

  /// Mutate bits, return how many mutations were performed
  size_t Mutate(
    Random & random,
    const size_t num_muts, // @CAO: use tools/Binomial in Distribution.h with this part?
    const size_t min_idx=0 // draw this from a distribution to make some
                           // bits more volatile than others
  ) {
    emp_assert(min_idx <= NUM_BITS);
    emp_assert(num_muts <= NUM_BITS - min_idx);

    std::vector<size_t> res;
    Choose(random, NUM_BITS - min_idx, num_muts, res);

    for (size_t idx : res) Toggle(idx + min_idx);

    return num_muts;

  }

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

      // mask out filler bits
      if constexpr (static_cast<bool>(LAST_BIT)) {
        bit_set[NUM_FIELDS - 1] &= MaskLow<field_t>(LAST_BIT);
      }

      return *this;

    }

    /// Convert to a Bitset of a different size.
    template <size_t FROM_BITS>
    BitSet<FROM_BITS> Export(size_t start_bit=0) const {

      BitSet<FROM_BITS> out_bits;
      out_bits.Import(*this, start_bit);

      return out_bits;
    }

    /// Test if two BitSet objects are identical.
    bool operator==(const BitSet & in_set) const {
      for (size_t i = 0; i < NUM_FIELDS; ++i) {
        if (bit_set[i] != in_set.bit_set[i]) return false;
      }
      return true;
    }

    /// Compare two BitSet objects, based on the associated binary value.
    bool operator<(const BitSet & in_set) const {
      for (int i = NUM_FIELDS-1; i >= 0; --i) {         // Start loop at the largest field.
        if (bit_set[i] == in_set.bit_set[i]) continue;  // If same, keep looking!
        return (bit_set[i] < in_set.bit_set[i]);        // Otherwise, do comparison
      }
      return false;
    }

    /// Compare two BitSet objects, based on the associated binary value.
    bool operator<=(const BitSet & in_set) const {
      for (int i = NUM_FIELDS-1; i >= 0; --i) {         // Start loop at the largest field.
        if (bit_set[i] == in_set.bit_set[i]) continue;  // If same, keep looking!
        return (bit_set[i] < in_set.bit_set[i]);        // Otherwise, do comparison
      }
      return true;
    }

    /// Test if two BitSet objects are different.
    bool operator!=(const BitSet & in_set) const { return !operator==(in_set); }

    /// Compare two BitSet objects, based on the associated binary value.
    bool operator>(const BitSet & in_set) const { return !operator<=(in_set); }

    /// Compare two BitSet objects, based on the associated binary value.
    bool operator>=(const BitSet & in_set) const { return !operator<(in_set); }

    /// How many bits are in this BitSet?
    constexpr static size_t GetSize() { return NUM_BITS; }

    /// How many bytes are in this BitSet?
    constexpr static size_t GetNumBytes() { return NUM_BYTES; }

    /// Retrieve the bit as a specified index.
    bool Get(size_t index) const {
      emp_assert(index >= 0 && index < NUM_BITS);
      const size_t field_id = FieldID(index);
      const size_t pos_id = FieldPos(index);
      return (bit_set[field_id] & (((field_t)1U) << pos_id)) != 0;
    }

    /// Set the bit at a specified index.
    void Set(size_t index, bool value=true) {
      emp_assert(index < NUM_BITS);
      const size_t field_id = FieldID(index);
      const size_t pos_id = FieldPos(index);
      const field_t pos_mask = ((field_t)1U) << pos_id;

      if (value) bit_set[field_id] |= pos_mask;
      else       bit_set[field_id] &= ~pos_mask;
    }

    /// Flip all bits in this BitSet
    BitSet & Toggle() { return NOT_SELF(); }

    /// Flip a single bit
    BitSet & Toggle(size_t index) {
      emp_assert(index >= 0 && index < NUM_BITS);
      const size_t field_id = FieldID(index);
      const size_t pos_id = FieldPos(index);
      (bit_set[field_id] ^= (((field_t)1U) << pos_id));
      return *this;
    }

    /// Flips all the bits in a range [start, end)
    BitSet & Toggle(size_t start, size_t end) {
      emp_assert(start <= end && end <= NUM_BITS);
      for(size_t index = start; index < end; index++) {
        Toggle(index);
      }
      return *this;
    }

    /// Get the full byte starting from the bit at a specified index.
    uint8_t GetByte(size_t index) const {
      emp_assert(index < NUM_BYTES);
      const size_t field_id = Byte2Field(index);
      const size_t pos_id = Byte2FieldPos(index);
      return (bit_set[field_id] >> pos_id) & 255;
    }

    /// Get a read-only view into the internal array used by BitSet.
    /// @return Read-only span of BitSet's bytes.
    std::span<const std::byte> GetBytes() const {
      return std::span<const std::byte>(
        reinterpret_cast<const std::byte*>(bit_set),
        NUM_BYTES
      );
    }

    /// Set the full byte starting at the bit at the specified index.
    void SetByte(size_t index, uint8_t value) {
      emp_assert(index < NUM_BYTES);
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

      std::memcpy(
        reinterpret_cast<unsigned char*>(bit_set) + index * (32/8),
        &value,
        sizeof(value)
      );

      // Mask out filler bits if necessary
      if constexpr (static_cast<bool>(LAST_BIT)) {
        // we only need to do this
        // if (index * 32 == (NUM_FIELDS - 1) * FIELD_BITS)
        // but just doing it always is probably faster
        // check to make sure there are no leading ones in the unused bits
        emp_assert((bit_set[NUM_FIELDS - 1] & ~MaskLow<field_t>(LAST_BIT)) == 0);
      }

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
          std::min(64, NUM_FIELDS * FIELD_BITS - 64 * index)/8
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
        std::memcpy(
          reinterpret_cast<unsigned char*>(bit_set) + index * (64/8),
          &value,
          sizeof(value)
        );
      } else if constexpr (FIELD_BITS == 32 && NUM_FIELDS == 1) {
        std::memcpy(
          reinterpret_cast<unsigned char*>(bit_set),
          &value,
          32/8
        );
      } else {
        std::memcpy(
          reinterpret_cast<unsigned char*>(bit_set) + index * (64/8),
          &value,
          std::min(64, NUM_FIELDS * FIELD_BITS - 64 * index)/8
        );
      }

      // Mask out filler bits if necessary
      if constexpr (static_cast<bool>(LAST_BIT)) {
        // we only need to do this
        // if (index * 64 == (NUM_FIELDS - 1) * FIELD_BITS)
        // but just doing it always is probably faster
        // check to make sure there are no leading ones in the unused bits
        emp_assert((bit_set[NUM_FIELDS - 1] & ~MaskLow<field_t>(LAST_BIT)) == 0);
      }


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
        std::memcpy(&res, bit_set, NUM_BYTES);
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

    /// Return true if ANY bits in the BitSet are one, else return false.
    bool Any() const {
      // profiled the if constexpr else
      // and did see a difference on perf reports and in runtime MAM
      if constexpr (NUM_FIELDS == 1) return bit_set[0];
      else {
        for (auto i : bit_set) if (i) return true;
        return false;
      }
    }

    /// Return true if NO bits in the BitSet are one, else return false.
    bool None() const { return !Any(); }

    /// Return true if ALL bits in the BitSet are one, else return false.
    bool All() const { return (~(*this)).None(); }

    /// Index into a const BitSet (i.e., cannot be set this way.)
    bool operator[](size_t index) const { return Get(index); }

    /// Index into a BitSet, returning a proxy that will allow bit assignment to work.
    BitProxy operator[](size_t index) { return BitProxy(*this, index); }

    /// Set all bits to zero.
    void Clear() { std::memset(bit_set, 0, sizeof(bit_set)); }

    /// Set all bits to one.
    void SetAll() {
      std::memset(bit_set, 255, sizeof(bit_set));;
      if constexpr (static_cast<bool>(LAST_BIT)) {
        bit_set[NUM_FIELDS - 1] &= MaskLow<field_t>(LAST_BIT);
      }
    }

    /// Overload ostream operator to return Print.
    friend std::ostream& operator<<(std::ostream &out, const BitSet& bs){
      bs.Print(out);
      return out;
    }

    /// Print all bits to the provided output stream.
    void Print(std::ostream & out=std::cout) const {
      for (size_t i = NUM_BITS; i > 0; i--) { out << Get(i-1); }
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
        BitSet temp( out_set );
        // optimization currently only implemented for NUM_FIELDS == 1
        if constexpr (NUM_FIELDS == 1) temp.template ShiftLeft<1>();
        else temp <<= 1;

        out_set.AND_SELF(temp);
        ++length;

      }
      return length;
    }


    /// Perform a Boolean NOT on this BitSet and return the result.
    BitSet NOT() const {
      BitSet out_set(*this);
      out_set.NOT_SELF();
      return out_set;
    }

    /// Perform a Boolean AND with a second BitSet and return the result.
    BitSet AND(const BitSet & set2) const {
      BitSet out_set(*this);
      out_set.AND_SELF( set2 );
      return out_set;
    }

    /// Perform a Boolean OR with a second BitSet and return the result.
    BitSet OR(const BitSet & set2) const {
      BitSet out_set(*this);
      out_set.OR_SELF( set2 );
      return out_set;
    }

    /// Perform a Boolean NAND with a second BitSet and return the result.
    BitSet NAND(const BitSet & set2) const {
      BitSet out_set(*this);
      out_set.NAND_SELF( set2 );
      return out_set;
    }

    /// Perform a Boolean NOR with a second BitSet and return the result.
    BitSet NOR(const BitSet & set2) const {
      BitSet out_set(*this);
      out_set.NOR_SELF( set2 );
      return out_set;
    }

    /// Perform a Boolean XOR with a second BitSet and return the result.
    BitSet XOR(const BitSet & set2) const {
      BitSet out_set(*this);
      out_set.XOR_SELF( set2 );
      return out_set;
    }

    /// Perform a Boolean EQU with a second BitSet and return the result.
    BitSet EQU(const BitSet & set2) const {
      BitSet out_set(*this);
      out_set.EQU_SELF( set2 );
      return out_set;
    }


    /// Perform a Boolean NOT on this BitSet, store result here, and return this object.
    BitSet & NOT_SELF() {
      if constexpr (NUM_FIELDS == 1) bit_set[0] = ~bit_set[0];
      else for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~bit_set[i];

      if constexpr (LAST_BIT > 0) {
        bit_set[NUM_FIELDS - 1] &= MaskLow<field_t>(LAST_BIT);
      }

      return *this;
    }

    /// Perform a Boolean AND with a second BitSet, store result here, and return this object.
    __attribute__ ((hot))
    BitSet & AND_SELF(const BitSet & set2) {
      if constexpr ( NUM_FIELDS == 1 ) {
        bit_set[0] = bit_set[0] & set2.bit_set[0];
      } else for (size_t i = 0; i < NUM_FIELDS; i++) {
        bit_set[i] = bit_set[i] & set2.bit_set[i];
      }
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
      if constexpr (LAST_BIT > 0) {
        bit_set[NUM_FIELDS - 1] &= MaskLow<field_t>(LAST_BIT);
      }
      return *this;
    }

    /// Perform a Boolean NOR with a second BitSet, store result here, and return this object.
    BitSet & NOR_SELF(const BitSet & set2) {
      for (size_t i = 0; i < NUM_FIELDS; i++) bit_set[i] = ~(bit_set[i] | set2.bit_set[i]);
      if constexpr (LAST_BIT > 0) {
        bit_set[NUM_FIELDS - 1] &= MaskLow<field_t>(LAST_BIT);
      }
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
      if constexpr (LAST_BIT > 0) {
        bit_set[NUM_FIELDS - 1] &= MaskLow<field_t>(LAST_BIT);
      }
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
        reinterpret_cast<unsigned char *>(bit_set) + NUM_BYTES
      );

      // reverse each byte
      // adapted from https://stackoverflow.com/questions/2602823/in-c-c-whats-the-simplest-way-to-reverse-the-order-of-bits-in-a-byte
      for (size_t i = 0; i < NUM_BYTES; ++i) {
        unsigned char & b = reinterpret_cast<unsigned char *>(bit_set)[i];
        b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
        b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
        b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
      }

      // shift out filler bits
      if constexpr (static_cast<bool>((8-NUM_BITS%8)%8)) {
        this->ShiftRight((8-NUM_BITS%8)%8);
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
      constexpr field_t shift_size = shift_size_raw % NUM_BITS;

      // special case: for exactly one field_t, try to go low level
      // adapted from https://stackoverflow.com/questions/776508/best-practices-for-circular-shift-rotate-operations-in-c
      if constexpr (NUM_FIELDS == 1) {
        field_t & n = bit_set[0];
        field_t c = shift_size;

        // mask necessary to suprress shift count overflow warnings
        constexpr field_t mask = MaskLow<field_t>(FIELD_LOG2);

        c &= mask;
        n = (n<<c) | (n>>( (-(c+FIELD_BITS-NUM_BITS))&mask ));

      } else {

        // note that we already modded shift_size by NUM_BITS
        // so there's no need to mod by FIELD_SIZE here
        constexpr int field_shift = LAST_BIT ? (
          (shift_size + FIELD_BITS - LAST_BIT) / FIELD_BITS
        ) : (
          shift_size / FIELD_BITS
        );
        // if we field shift, we need to shift bits by (FIELD_BITS - LAST_BIT)
        // more to account for the filler that gets pulled out of the middle
        constexpr int bit_shift = LAST_BIT && field_shift ? (
          (shift_size + FIELD_BITS - LAST_BIT) % FIELD_BITS
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
        if constexpr ((bool)LAST_BIT) {
          const int filler_idx = (NUM_FIELDS - 1 + field_shift) % NUM_FIELDS;
          for (int i = filler_idx + 1; i < (int)NUM_FIELDS; ++i) {
            bit_set[i-1] |= bit_set[i] << LAST_BIT;
            bit_set[i] >>= (FIELD_BITS - LAST_BIT);
          }
        }

        // account for bit_shift
        if (bit_shift) {

          const field_t keystone = LAST_BIT ? (
            (bit_set[NUM_FIELDS - 1] << (FIELD_BITS - LAST_BIT))
            | (bit_set[NUM_FIELDS - 2] >> LAST_BIT)
          ) : (
            bit_set[NUM_FIELDS - 1]
          );

          for (int i = NUM_FIELDS - 1; i > 0; --i) {
            bit_set[i] <<= bit_shift;
            bit_set[i] |= (bit_set[i-1] >> bit_overflow);
          }
          // Handle final field
          bit_set[0] <<= bit_shift;
          bit_set[0] |= keystone >> bit_overflow;

        }

      }

      // mask out filler bits
      if constexpr ((bool)LAST_BIT) {
        bit_set[NUM_FIELDS - 1] &= MaskLow<field_t>(LAST_BIT);
      }

      return *this;

    }


    /// Helper for calling ROTATE with positive number
    template<size_t shift_size_raw>
    BitSet & ROTR_SELF() {

      constexpr field_t shift_size = shift_size_raw % NUM_BITS;

      // special case: for exactly one field_t, try to go low level
      // adapted from https://stackoverflow.com/questions/776508/best-practices-for-circular-shift-rotate-operations-in-c
      if constexpr (NUM_FIELDS == 1) {
        field_t & n = bit_set[0];
        field_t c = shift_size;

        // mask necessary to suprress shift count overflow warnings
        constexpr field_t mask = MaskLow<field_t>(FIELD_LOG2);

        c &= mask;
        n = (n>>c) | (n<<( (NUM_BITS-c)&mask ));

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
        if constexpr ((bool)LAST_BIT) {
          constexpr int filler_idx = NUM_FIELDS - 1 - field_shift;
          for (int i = filler_idx + 1; i < (int)NUM_FIELDS; ++i) {
            bit_set[i-1] |= bit_set[i] << LAST_BIT;
            bit_set[i] >>= (FIELD_BITS - LAST_BIT);
          }
        }

        // account for bit_shift
        if (bit_shift) {

          const field_t keystone = LAST_BIT ? (
            bit_set[0] >> (FIELD_BITS - LAST_BIT)
          ) : (
            bit_set[0]
          );

          if constexpr ((bool)LAST_BIT) {
            bit_set[NUM_FIELDS-1] |= bit_set[0] << LAST_BIT;
          }

          for (size_t i = 0; i < NUM_FIELDS - 1; ++i) {
            bit_set[i] >>= bit_shift;
            bit_set[i] |= (bit_set[i+1] << bit_overflow);
          }
          bit_set[NUM_FIELDS - 1] >>= bit_shift;
          bit_set[NUM_FIELDS - 1] |= keystone << bit_overflow;
        }
      }

      // mask out filler bits
      if constexpr ((bool)LAST_BIT) {
        bit_set[NUM_FIELDS - 1] &= MaskLow<field_t>(LAST_BIT);
      }

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

      if constexpr (static_cast<bool>(LAST_BIT)) {
        bit_set[NUM_BITS/FIELD_BITS] = (
          bit_set[NUM_BITS/FIELD_BITS]
          + set2.bit_set[NUM_BITS/FIELD_BITS]
          + static_cast<field_t>(carry)
        ) & emp::MaskLow<field_t>(LAST_BIT);
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

      if constexpr (static_cast<bool>(LAST_BIT)) {
        bit_set[NUM_BITS/FIELD_BITS] = (
          bit_set[NUM_BITS/FIELD_BITS]
          - set2.bit_set[NUM_BITS/FIELD_BITS]
          - static_cast<field_t>(carry)
        ) & emp::MaskLow<field_t>(LAST_BIT);
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

    /// Function to allow drop-in replacement with std::bitset.
    constexpr static size_t size() { return NUM_BITS; }

    /// Function to allow drop-in replacement with std::bitset.
    inline bool all() const { return All(); }

    /// Function to allow drop-in replacement with std::bitset.
    inline bool any() const { return Any(); }

    /// Function to allow drop-in replacement with std::bitset.
    inline bool none() const { return !Any(); }

    /// Function to allow drop-in replacement with std::bitset.
    inline size_t count() const { return CountOnes_Mixed(); }

    /// Function to allow drop-in replacement with std::bitset.
    inline BitSet & flip() { return Toggle(); }

    /// Function to allow drop-in replacement with std::bitset.
    inline BitSet & flip(size_t pos) { return Toggle(pos); }

    /// Function to allow drop-in replacement with std::bitset.
    inline BitSet & flip(size_t start, size_t end) { return Toggle(start, end); }

    template <class Archive>
    void serialize( Archive & ar )
    {
      if constexpr ( sizeof(field_t) != sizeof(uint64_t) ) {
        // in order for JSON serialization interoperability of native (64-bit)
        // and emscripten (32-bit) artifacts,
        // in 32-bit mode we have serialize as an array of 64-bit values
        uint64_t buf[ (sizeof(bit_set) + 7) / 8 ]{};
        std::memcpy( buf, bit_set, sizeof(bit_set) ); // copy to buf
        ar( buf );
        std::memcpy( bit_set, buf, sizeof(bit_set) ); // copy from buf
      } else {
        ar( bit_set );
      }
    }

  };

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
    return (double)((in1 & in2).CountOnes() + (~in1 & ~in2).CountOnes()) / (double)NUM_BITS;
  }

}

/// For hashing BitSets
namespace std {

    template <size_t N>
    struct hash< emp::BitSet<N> > {
        size_t operator()( const emp::BitSet<N>& bs ) const {
          if constexpr ( 8 * sizeof( size_t ) == 32 ) {
            if constexpr ( N <= 32 ) {
              return bs.GetUInt32( 0 );
            } else if constexpr ( N <= 32 * 2 ) {
              return emp::hash_combine(  bs.GetUInt32( 0 ), bs.GetUInt32( 1 ) );
            } else if constexpr ( N <= 32 * 3 ) {
              return emp::hash_combine(
                emp::hash_combine( bs.GetUInt32( 0 ), bs.GetUInt32( 1 ) ),
                bs.GetUInt32( 2 )
              );
            } else if constexpr ( N <= 32 * 4 ) {
              return emp::hash_combine(
                emp::hash_combine( bs.GetUInt32( 0 ), bs.GetUInt32( 1 ) ),
                emp::hash_combine( bs.GetUInt32( 2 ), bs.GetUInt32( 3 ) )
              );
            } else return emp::murmur_hash( bs.GetBytes() );
          } else if constexpr ( 8 * sizeof( size_t ) == 64 ) {
            if constexpr ( N <= 64 ) {
              return bs.GetUInt64( 0 );
            } else if constexpr ( N <= 64 * 2 ) {
              return emp::hash_combine(  bs.GetUInt64( 0 ), bs.GetUInt64( 1 ) );
            } else return emp::murmur_hash( bs.GetBytes() );
          } else { emp_assert( false ); return 0; }
        }
    };
}


#endif // #ifndef EMP_BIT_SET_HPP
