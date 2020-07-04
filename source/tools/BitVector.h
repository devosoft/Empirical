/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2020.
 *
 *  @file  BitVector.h
 *  @brief A drop-in replacement for std::vector<bool>, with additional bitwise logic features.
 *  @note Status: RELEASE
 *
 *  Compile with -O3 and -msse4.2 for fast bit counting.
 * 
 *  @todo Do small BitVector optimization.  Currently we have number of bits (8 bytes) and a
 *        pointer to the memory for the bitset (another 8 bytes), but we could use those 16 bytes
 *        as 1 byte of size info followed by 15 bytes of bitset (120 bits!)
 *  @todo For BitVectors larger than 120 bits, we can use a factory to preserve bit info.
 *  @todo Implement append(), resize()...
 *  @todo Implement techniques to push bits (we have pop)
 *  @todo Implement techniques to insert or remove bits from middle.
 *
 *  @note This class is 15-20% slower than emp::BitSet, but more flexible & run-time configurable.
 */


#ifndef EMP_BIT_VECTOR_H
#define EMP_BIT_VECTOR_H

#include <iostream>
#include <bitset>

#include "../base/assert.h"
#include "../base/Ptr.h"
#include "../base/vector.h"
#include "../base/array.h"

#include "bitset_utils.h"
#include "functions.h"
#include "math.h"



namespace emp {

  /// @brief A drop-in replacement for std::vector<bool>, but with extra bitwise logic features.
  ///
  /// This class stores an arbirary number of bits in a set of "fields" (either 32-bit or 64-bit,
  /// depending on which should be faster.)  Individual bits can be extracted, -or- bitwise logic
  /// (or bit magic) can be used on the groups of bits,

  class BitVector {
  private:
    // For the moment, field_t will always be equal to size_t.  Since size_t is normally the native
    // size for a processor (and, correctly, 32 bits for Emscripten), this should work in almost all
    // cases.
    using field_t = size_t;

    static constexpr size_t FIELD_BITS = sizeof(field_t)*8; ///< How many bits are in a field?
    size_t num_bits;                                        ///< How many total bits are we using?
    //Ptr<field_t> bit_set;                                   ///< What is the status of each bit?
    // emp::array<std::byte, 8> bit_set;                       ///< What is the status of each bit?
    emp::Ptr<std::byte> bit_set;

    static constexpr const size_t SHORT_THRESHOLD = 64;

    /// End position of the stored bits in the last field; 0 if perfect fit.
    size_t LastBitID() const { return num_bits & (FIELD_BITS - 1); }

    /// How many feilds do we need?
    size_t NumFields() const { return num_bits ? (1 + ((num_bits - 1) / FIELD_BITS)) : 0; }

    /// How many bytes are used in the current vector (round up to whole bytes.)
    size_t NumBytes()  const { return num_bits ? (1 + ((num_bits - 1) >> 3)) : 0; }

    /// BitProxy lets us use operator[] on with BitVector as an lvalue.
    struct BitProxy {
      BitVector & bit_vector;  ///< Which BitVector does this proxy belong to?
      size_t index;            ///< Which position in the bit vector does this proxy point at?

      /// Setup a new proxy with the associated vector and index.
      BitProxy(BitVector & _v, size_t _idx) : bit_vector(_v), index(_idx) {;}

      /// Assignment operator to the bit associated with this proxy (as an lvalue).
      BitProxy & operator=(bool b) {
        bit_vector.Set(index, b);
        return *this;
      }

      /// Conversion of this proxy to Boolean (as an rvalue)
      operator bool() const {
        return bit_vector.Get(index);
      }

      /// Compound assignement operator AND using BitProxy as lvalue.
      /// @note Implemented in BitProxy since it needs to work, but may not be efficient.
      BitProxy & operator &=(bool b) {
        const bool v = bit_vector.Get(index);
        bit_vector.Set(index, v & b);
        return *this;
      }

      /// Compound assignement operator OR using BitProxy as lvalue.
      /// @note Implemented in BitProxy since it needs to work, but may not be efficient.
      BitProxy & operator |=(bool b) {
        const bool v = bit_vector.Get(index);
        bit_vector.Set(index, v | b);
        return *this;
      }

      /// Compound assignement operator XOR using BitProxy as lvalue.
      /// @note Implemented in BitProxy since it needs to work, but may not be efficient.
      BitProxy & operator ^=(bool b) {
        const bool v = bit_vector.Get(index);
        bit_vector.Set(index, v ^ b);
        return *this;
      }

      /// Compound assignement operator PLUS using BitProxy as lvalue.
      /// @note Implemented in BitProxy since it needs to work, but may not be efficient.
      BitProxy & operator +=(bool b) {
        const bool v = bit_vector.Get(index);
        bit_vector.Set(index, v || b);
        return *this;
      }

      /// Compound assignement operator MINUS using BitProxy as lvalue.
      /// @note Implemented in BitProxy since it needs to work, but may not be efficient.
      BitProxy & operator -=(bool b) {
        const bool v = bit_vector.Get(index);
        bit_vector.Set(index, v - b);
        return *this;
      }

      /// Compound assignement operator TIMES using BitProxy as lvalue.
      /// @note Implemented in BitProxy since it needs to work, but may not be efficient.
      BitProxy & operator *=(bool b) {
        const bool v = bit_vector.Get(index);
        bit_vector.Set(index, v && b);
        return *this;
      }

      /// Compound assignement operator DIV using BitProxy as lvalue.
      /// @note Implemented in BitProxy since it needs to work, but may not be efficient.
      /// @note Never use this function except for consistency in a template since must divide by 1.
      BitProxy & operator /=(bool b) {
        emp_assert(b == true);
        return *this;
      }
    };

    /// Identify the field that a specified bit is in.
    static constexpr size_t FieldID(const size_t index)  { return index / FIELD_BITS; }

    /// Identify the position in a field where a specified bit is.
    static constexpr size_t FieldPos(const size_t index) { return index & (FIELD_BITS-1); }

    /// Identify which field a specified byte position would be in.
    static constexpr size_t Byte2Field(const size_t index) { return index/sizeof(field_t); }

    /// Convert a byte position in BitVector to a byte position in the target field.
    static constexpr size_t Byte2FieldPos(const size_t index) {
      return (index & (sizeof(field_t)-1)) << 3;
    }

    /// Assume that the size of the bit_set has already been adjusted to be the size of the one
    /// being copied and only the fields need to be copied over.
    void RawCopy(Ptr<const field_t> in_set) {
      #ifdef EMP_TRACK_MEM
      if (num_bits > SHORT_THRESHOLD) {
        emp_assert(in_set.IsNull() == false);
        emp_assert(BitSetPtr().DebugIsArray() && in_set.DebugIsArray());
        emp_assert(BitSetPtr().DebugGetArrayBytes() == in_set.DebugGetArrayBytes(),
                  BitSetPtr().DebugGetArrayBytes(), in_set.DebugGetArrayBytes());
      }
      #endif
      
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) BitSetPtr()[i] = in_set[i];
    }

    /// Helper: call SHIFT with positive number
    void ShiftLeft(const size_t shift_size) {
      const size_t field_shift = shift_size / FIELD_BITS;
      const size_t bit_shift = shift_size % FIELD_BITS;
      const size_t bit_overflow = FIELD_BITS - bit_shift;
      const size_t NUM_FIELDS = NumFields();
      if (num_bits > SHORT_THRESHOLD) {
        // Loop through each field, from L to R, and update it.
        if (field_shift) {
          for (size_t i = NUM_FIELDS; i > field_shift; --i) {
            BitSetPtr()[i-1] = BitSetPtr()[i - field_shift - 1];
          }
          for (size_t i = field_shift; i > 0; --i) BitSetPtr()[i-1] = 0;
        }

        // account for bit_shift
        if (bit_shift) {
          for (size_t i = NUM_FIELDS - 1; i > field_shift; --i) {
            BitSetPtr()[i] <<= bit_shift;
            BitSetPtr()[i] |= (BitSetPtr()[i-1] >> bit_overflow);
          }
          // Handle final field (field_shift position)
          BitSetPtr()[field_shift] <<= bit_shift;
        }

        // Mask out any bits that have left-shifted away
        const size_t last_bit_id = LastBitID();
        constexpr field_t val_one = 1;
        if (last_bit_id) { BitSetPtr()[NUM_FIELDS - 1] &= (val_one << last_bit_id) - val_one; }
      } else {
        *BitSetPtr().Raw() <<= bit_shift;
      }
    }

    /// Helper for calling SHIFT with negative number
    void ShiftRight(const size_t shift_size) {
      const size_t field_shift = shift_size / FIELD_BITS;
      const size_t bit_shift = shift_size % FIELD_BITS;
      const size_t bit_overflow = FIELD_BITS - bit_shift;
      const size_t NUM_FIELDS = NumFields();
      const size_t field_shift2 = NUM_FIELDS - field_shift;

      if (num_bits > SHORT_THRESHOLD) {
      // account for field_shift
        if (field_shift) {
          for (size_t i = 0; i < field_shift2; ++i) {
            BitSetPtr()[i] = BitSetPtr()[i + field_shift];
          }
          for (size_t i = field_shift2; i < NUM_FIELDS; i++) BitSetPtr()[i] = 0U;
        }

        // account for bit_shift
        if (bit_shift) {
          for (size_t i = 0; i < (field_shift2 - 1); ++i) {
            BitSetPtr()[i] >>= bit_shift;
            BitSetPtr()[i] |= (BitSetPtr()[i+1] << bit_overflow);
          }
          BitSetPtr()[field_shift2 - 1] >>= bit_shift;
        }
      } else {
        *BitSetPtr().Raw() >>= bit_shift;
      }
    }

  public:
    /// Build a new BitVector with specified bit count (default 0) and initialization (default 0)
    BitVector(size_t in_num_bits=0, bool init_val=false)
      : num_bits(in_num_bits), bit_set()
    {
      //if (num_bits) bit_set = emp::array<std::byte, 8>();
      
      if (num_bits > SHORT_THRESHOLD) {
        //bit_set = emp::NewArrayPtr<std::byte>(8);
        //BitSetPtr() = NewArrayPtr<field_t>(NumFields());
        bit_set = NewArrayPtr<field_t>(NumFields()).Cast<std::byte>();
      }
      if (init_val) SetAll(); else Clear();
    }

    /// Copy constructor of existing bit field.
    BitVector(const BitVector & in_set)
      : num_bits(in_set.num_bits), bit_set()
    {
      #ifdef EMP_TRACK_MEM
      if (num_bits > SHORT_THRESHOLD) {
        emp_assert(in_set.bit_set.IsNull() || in_set.bit_set.DebugIsArray());
        emp_assert(in_set.bit_set.OK());
      }
      #endif
      // There is only something to copy if there are a non-zero number of bits!
      if (num_bits) {
        #ifdef EMP_TRACK_MEM
        if (in_set.num_bits > SHORT_THRESHOLD)
          emp_assert(!in_set.bit_set.IsNull() && in_set.bit_set.DebugIsArray(), in_set.bit_set.IsNull(), in_set.bit_set.DebugIsArray());
        #endif
        if (num_bits > SHORT_THRESHOLD) 
        {
          bit_set = NewArrayPtr<field_t>(NumFields()).Cast<std::byte>();
        }
        if (in_set.num_bits > SHORT_THRESHOLD){
          RawCopy(in_set.BitSetPtr());
        } else {
          *BitSetPtr().Raw() =  *(in_set.BitSetPtr().Raw()); 
        }
      }
    }

    /// Move constructor of existing bit field.
    BitVector(BitVector && in_set) : num_bits(in_set.num_bits), bit_set(in_set.bit_set) {
      #ifdef EMP_TRACK_MEM
      if (in_set.num_bits > SHORT_THRESHOLD) {
        emp_assert(bit_set == nullptr || bit_set.DebugIsArray());
        emp_assert(bit_set.OK());
      }
      #endif
      in_set.bit_set = nullptr;
      in_set.num_bits = 0;
    }

    /// Copy, but with a resize.
    BitVector(const BitVector & in_set, size_t new_size) : BitVector(in_set) {
      if (num_bits != new_size) Resize(new_size);
    }

    /// Destructor
    ~BitVector() {
      if (bit_set) {        // A move constructor can make bit_set == nullptr
          if (num_bits > SHORT_THRESHOLD) {
            BitSetPtr().DeleteArray();
          }    
      }
    }

    /// Assignment operator.
    BitVector & operator=(const BitVector & in_set) {
      #ifdef EMP_TRACK_MEM
      if (in_set.num_bits > SHORT_THRESHOLD) {
        emp_assert(in_set.BitSetPtr() == nullptr || in_set.BitSetPtr().DebugIsArray());
        emp_assert(in_set.BitSetPtr() != nullptr || in_set.num_bits == 0);
        emp_assert(in_set.BitSetPtr().OK());
      }
      #endif
      
      if (&in_set == this) return *this;
      const size_t in_num_fields = in_set.NumFields();
      const size_t prev_num_fields = NumFields();

      if (in_num_fields != prev_num_fields) {
        if (bit_set) {
          if (num_bits > SHORT_THRESHOLD) {
            BitSetPtr().DeleteArray();
          } else {
            *BitSetPtr().Raw() = 0;
          }
        }
      }
      num_bits = in_set.num_bits;
      if (in_num_fields != prev_num_fields && in_set.num_bits > SHORT_THRESHOLD) {
          bit_set = NewArrayPtr<field_t>(NumFields()).Cast<std::byte>();
      }
      if (num_bits > SHORT_THRESHOLD){
        RawCopy(in_set.BitSetPtr());
      } else {
        *BitSetPtr().Raw() =  *(in_set.BitSetPtr().Raw()); 
      }

      return *this;
    }

    /// Move operator.
    BitVector & operator=(BitVector && in_set) {
      emp_assert(&in_set != this);        // in_set is an r-value, so this shouldn't be possible...
      if (num_bits > SHORT_THRESHOLD && BitSetPtr()) BitSetPtr().DeleteArray(); // If we already had a bitset, get rid of it.
      num_bits = in_set.num_bits;         // Update the number of bits...
      bit_set = in_set.bit_set;           // And steal the old memory for what those bits are.
      in_set.bit_set = nullptr;           // Prepare in_set for deletion without deallocating.
      in_set.num_bits = 0;

      return *this;
    }

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
      const size_t old_num_bits = num_bits;
      const size_t old_num_fields = NumFields();
      num_bits = new_bits;
      const size_t NUM_FIELDS = NumFields();

      if (NUM_FIELDS == old_num_fields) {   // We can use our existing bit field
        num_bits = new_bits;
        // If there are extra bits, zero them out.
        if (LastBitID() > 0) {
          if (num_bits > SHORT_THRESHOLD) {
            BitSetPtr()[NUM_FIELDS - 1] &= MaskLow<field_t>(LastBitID());
          } else {
            *BitSetPtr().Raw() &= MaskLow<field_t>(LastBitID());
          }
        }
      }
        
      else {  // We have to change the number of bitfields.  Resize & copy old info.
        auto old_bit_set = bit_set;
        
        if (num_bits > SHORT_THRESHOLD) {
          bit_set = NewArrayPtr<field_t>(NUM_FIELDS).Cast<std::byte>();
        } else {
          bit_set = Ptr<std::byte>();
          Clear();
        }
        const size_t min_fields = std::min(old_num_fields, NUM_FIELDS);
        if (num_bits > SHORT_THRESHOLD) {
          if (old_num_bits > SHORT_THRESHOLD) {
            for (size_t i = 0; i < min_fields; i++) BitSetPtr()[i] = BitSetPtr(old_bit_set, old_num_bits)[i];
          } else {
            BitSetPtr()[0] = *BitSetPtr(old_bit_set, old_num_bits).Raw();
          }
          for (size_t i = min_fields; i < NUM_FIELDS; i++) BitSetPtr()[i] = 0U;
          
        } else {
          if (old_num_bits > SHORT_THRESHOLD) {
            *BitSetPtr().Raw() =  BitSetPtr(old_bit_set, old_num_bits)[0];
          } else {
            bit_set = old_bit_set;
          }
          std::cout << *BitSetPtr().Raw() << std::endl;
          std::cout << (0xFFFFFFFFFFFFFFFF >> (SHORT_THRESHOLD - num_bits)) << std::endl;
          *BitSetPtr().Raw() = *BitSetPtr().Raw() & (0xFFFFFFFFFFFFFFFF >> (SHORT_THRESHOLD - num_bits));
          
        }
        if (old_bit_set && old_num_bits > SHORT_THRESHOLD) BitSetPtr(old_bit_set, old_num_bits).DeleteArray();

        
      }
      return *this;
    }

    /// Test if two bit vectors are identical.
    bool operator==(const BitVector & in_set) const {
      if (num_bits != in_set.num_bits) return false;

      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; ++i) {
        if (num_bits > SHORT_THRESHOLD) {
          if (BitSetPtr()[i] != in_set.BitSetPtr()[i]) return false;
        } else {
          if (*BitSetPtr().Raw() != *(in_set.BitSetPtr().Raw())) return false;
        }
      }
      return true;
    }

    /// Compare the would-be numerical values of two bit vectors.
    bool operator<(const BitVector & in_set) const {
      if (num_bits != in_set.num_bits) return num_bits < in_set.num_bits;
      if (num_bits <= SHORT_THRESHOLD) return (*BitSetPtr().Raw() < *(in_set.BitSetPtr().Raw()));

      const size_t NUM_FIELDS = NumFields();
      for (size_t i = NUM_FIELDS; i > 0; --i) {         // Start loop at the largest field.
        const size_t pos = i-1;
        if (BitSetPtr()[pos] == in_set.BitSetPtr()[pos]) continue;  // If same, keep looking!
        return (BitSetPtr()[pos] < in_set.BitSetPtr()[pos]);        // Otherwise, do comparison
      }
      return false;
    }

    /// Compare the would-be numerical values of two bit vectors.
    bool operator<=(const BitVector & in_set) const {
      if (num_bits != in_set.num_bits) return num_bits <= in_set.num_bits;
      if (num_bits <= SHORT_THRESHOLD) return (*BitSetPtr().Raw() <= *(in_set.BitSetPtr().Raw()));

      const size_t NUM_FIELDS = NumFields();
      for (size_t i = NUM_FIELDS; i > 0; --i) {         // Start loop at the largest field.
        const size_t pos = i-1;
        if (BitSetPtr()[pos] == in_set.BitSetPtr()[pos]) continue;  // If same, keep looking!
        return (BitSetPtr()[pos] < in_set.BitSetPtr()[pos]);        // Otherwise, do comparison
      }
      return true;
    }

    /// Determine if two bit vectors are different.
    bool operator!=(const BitVector & in_set) const { return !operator==(in_set); }

    /// Compare the would-be numerical values of two bit vectors.
    bool operator>(const BitVector & in_set) const { return !operator<=(in_set); }

    /// Compare the would-be numerical values of two bit vectors.
    bool operator>=(const BitVector & in_set) const { return !operator<(in_set); }

    /// How many bits do we currently have?
    size_t GetSize() const { return num_bits; }
      
    /// Return the proper casting of bit_set (but const)
    emp::Ptr<const field_t> BitSetPtr() const{
      // For large bit sets, the bit_set pointer tells you where it is.
      if (num_bits > SHORT_THRESHOLD) {
        return bit_set.Cast<const field_t>();
      }

      // For small bit_sets assume they all fit in the space of the bit_set pointer.
      return emp::Ptr<const field_t>((const field_t*) &bit_set);
    }

    /// Return the proper casting of bit_set
    emp::Ptr<field_t> BitSetPtr() {
      // For large bit sets, the bit_set pointer tells you where it is.
      if (num_bits > SHORT_THRESHOLD) {
        return bit_set.Cast<field_t>();
      }

      // For small bit_sets assume they all fit in the space of the bit_set pointer.
      return emp::Ptr<field_t>((field_t*) &bit_set);
    }

    /// Return the proper casting of a supplied bit_set and number of bits
    emp::Ptr<field_t> BitSetPtr(emp::Ptr<std::byte> p, size_t num) {
      // For large bit sets, the bit_set pointer tells you where it is.
      if (num > SHORT_THRESHOLD) {
        return p.Cast<field_t>();
      }

      // For small bit_sets assume they all fit in the space of the bit_set pointer.
      return emp::Ptr<field_t>((field_t*) &p);
    }


    /// Retrive the bit value from the specified index.
    bool Get(size_t index) const {
      emp_assert(index < num_bits, index, num_bits);
      const size_t field_id = FieldID(index);
      const size_t pos_id = FieldPos(index);
      if (num_bits > SHORT_THRESHOLD) {
        return (BitSetPtr()[field_id] & (static_cast<field_t>(1) << pos_id)) != 0;
      }
      return (*BitSetPtr().Raw() & (static_cast<field_t>(1) << pos_id)) != 0;
    }

    /// Update the bit value at the specified index.
    BitVector & Set(size_t index, bool value=true) {
      emp_assert(index < num_bits, index, num_bits);
      const size_t field_id = FieldID(index);
      const size_t pos_id = FieldPos(index);
      constexpr field_t val_one = 1;
      const field_t pos_mask = val_one << pos_id;

      if (num_bits > SHORT_THRESHOLD) {
        if (value) BitSetPtr()[field_id] |= pos_mask;
        else       BitSetPtr()[field_id] &= ~pos_mask;
      } else {
        if (value) *BitSetPtr().Raw() |=  pos_mask;
        else       *BitSetPtr().Raw() &= ~pos_mask;
      }
      
      return *this;
    }

    /// A simple hash function for bit vectors.
    std::size_t Hash() const {
      std::size_t hash_val = 0;
      if (num_bits > SHORT_THRESHOLD) {
        const size_t NUM_FIELDS = NumFields();
        for (size_t i = 0; i < NUM_FIELDS; i++) {
          hash_val ^= BitSetPtr()[i];
        }
      } else {
        hash_val ^= *BitSetPtr().Raw();
      }
      return hash_val ^ ((97*num_bits) << 8);
    }

    /// Retrive the byte at the specified byte index.
    uint8_t GetByte(size_t index) const {
      emp_assert(index < NumBytes(), index, NumBytes());
      const size_t pos_id = Byte2FieldPos(index);
      if (num_bits > SHORT_THRESHOLD) {
        const size_t field_id = Byte2Field(index);
        return (BitSetPtr()[field_id] >> pos_id) & 255U;
      } else {
        return (*BitSetPtr().Raw() >> pos_id) & 255U;
      }
    }

    /// Update the byte at the specified byte index.
    void SetByte(size_t index, uint8_t value) {
      emp_assert(index < NumBytes(), index, NumBytes());
      const size_t pos_id = Byte2FieldPos(index);
      const field_t val_uint = value;
      if (num_bits > SHORT_THRESHOLD) {
        const size_t field_id = Byte2Field(index);
        BitSetPtr()[field_id] = (BitSetPtr()[field_id] & ~(static_cast<field_t>(255) << pos_id)) | (val_uint << pos_id);
      } else {
        *BitSetPtr().Raw() = (*BitSetPtr().Raw() & ~(static_cast<field_t>(255) << pos_id)) | (val_uint << pos_id);
      }
    }

    /// Retrive the 32-bit uint from the specifeid uint index.
    uint32_t GetUInt(size_t index) const {
      // If the fields are already 32 bits, return.
      if constexpr (sizeof(field_t) == 4) return BitSetPtr()[index];

      emp_assert(sizeof(field_t) == 8);

      const size_t field_id = index/2;
      const size_t field_pos = 1 - (index & 1);

      emp_assert(field_id < NumFields());

      if (num_bits > SHORT_THRESHOLD) {
        return (uint32_t) (BitSetPtr()[field_id] >> (field_pos * 32));
      } else {
        return (uint32_t) (*BitSetPtr().Raw() >> (field_pos * 32));
      }

      
    }

    /// Update the 32-bit uint at the specified uint index.
    void SetUInt(size_t index, uint32_t value) {
      if constexpr (sizeof(field_t) == 4) BitSetPtr()[index] = value;
      emp_assert(sizeof(field_t) == 8);
      const size_t field_pos = 1 - (index & 1);
      const field_t mask = ((field_t) ((uint32_t) -1)) << (1-field_pos);

      if (num_bits > SHORT_THRESHOLD) {
        const size_t field_id = index/2;
        emp_assert(field_id < NumFields());
        BitSetPtr()[field_id] &= mask;   // Clear out bits that we are setting.
        BitSetPtr()[field_id] |= ((field_t) value) << (field_pos * 32);
      } else {
        *BitSetPtr().Raw() &= mask;   // Clear out bits that we are setting.
        *BitSetPtr().Raw() |= ((field_t) value) << (field_pos * 32);
      }
    }

    void SetUIntAtBit(size_t index, uint32_t value) {
      if (num_bits > SHORT_THRESHOLD) {
        if constexpr (sizeof(field_t) == 4) BitSetPtr()[index] = value;

        emp_assert(sizeof(field_t) == 8);

        const size_t field_id = FieldID(index);
        const size_t field_pos = FieldPos(index);
        const field_t mask = ((field_t) ((uint32_t) -1)) << (1-field_pos);

        emp_assert(field_id < NumFields());

        BitSetPtr()[field_id] &= mask;   // Clear out bits that we are setting.
        BitSetPtr()[field_id] |= ((field_t) value) << (field_pos * 32);
      } else {
        if (index == 0) {
          *BitSetPtr().Raw() &= 0xFFFFFFFF00000000;
          *BitSetPtr().Raw() += (field_t) value;
        } else {
          *BitSetPtr().Raw() &= 0x00000000FFFFFFFF;
          *BitSetPtr().Raw() += ( ((field_t) value) << 32);
        }
      }
    }

    /// Retrive the 32-bit uint at the specified BIT index.
    uint32_t GetUIntAtBit(size_t index) {
      // @CAO Need proper assert for non-32-size bit fields!
      // emp_assert(index < num_bits);
      if (num_bits > SHORT_THRESHOLD) {
        const size_t field_id = FieldID(index);
        const size_t pos_id = FieldPos(index);
        if (pos_id == 0) return (uint32_t) BitSetPtr()[field_id];
        const size_t NUM_FIELDS = NumFields();
        const uint32_t part1 = (uint32_t) (BitSetPtr()[field_id] >> pos_id);
        const uint32_t part2 =
          (uint32_t)((field_id+1 < NUM_FIELDS) ? BitSetPtr()[field_id+1] << (FIELD_BITS-pos_id) : 0);
        return part1 | part2;
      }
      if (index == 0) {
        return (uint32_t) *BitSetPtr().Raw() & 0xFFFFFFFF;
      }
      return (uint32_t) (*BitSetPtr().Raw() >> 32) & 0xFFFFFFFF;
    }

    /// Retrieve the specified number of bits (stored in the field type) at the target bit index.
    template <size_t OUT_BITS>
    field_t GetValueAtBit(size_t index) {
      // @CAO This function needs to be generalized to return more then sizeof(field_t)*8 bits.
      static_assert(OUT_BITS <= sizeof(field_t)*8, "Requesting too many bits to fit in a UInt");
      return GetUIntAtBit(index) & MaskLow<field_t>(OUT_BITS);
    }

    /// Return true if ANY bits are set to 1, otherwise return false.
    bool Any() const {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) {
        if (num_bits > SHORT_THRESHOLD) {
          if (BitSetPtr()[i]) return true;
        } else {
          if (*BitSetPtr().Raw()) return true;
        }
      }
      return false;
    }

    /// Return true if NO bits are set to 1, otherwise return false.
    bool None() const { return !Any(); }

    /// Return true if ALL bits are set to 1, otherwise return false.
    bool All() const {
      const size_t NUM_FIELDS = NumFields();
      for (size_t i = 0; i < NUM_FIELDS; i++) {
        if (num_bits > SHORT_THRESHOLD) {
          if (~(BitSetPtr()[i])) return false;
        } else {
          if (~(*BitSetPtr().Raw())) return false;
        }
      }
      return true;
    }

    /// Casting a bit array to bool identifies if ANY bits are set to 1.
    explicit operator bool() const { return Any(); }

    /// Const index operator -- return the bit at the specified position.
    bool operator[](size_t index) const { return Get(index); }

    /// Index operator -- return a proxy to the bit at the specified position so it can be an lvalue.
    BitProxy operator[](size_t index) { return BitProxy(*this, index); }

    /// Set all bits to 0.
    void Clear() {
      const size_t NUM_FIELDS = NumFields();
      if (num_bits > SHORT_THRESHOLD) {
        for (size_t i = 0; i < NUM_FIELDS; i++) BitSetPtr()[i] = 0U;
      } else {
        *BitSetPtr().Raw() = 0U;
      }
    }

    /// Set all bits to 1.
    void SetAll() {
      const size_t NUM_FIELDS = NumFields();
      constexpr field_t all0 = 0;
      if (num_bits > SHORT_THRESHOLD) {
        for (size_t i = 0; i < NUM_FIELDS; i++) BitSetPtr()[i] = ~all0;
      } else {
        *BitSetPtr().Raw() = ~all0;
      }
      if (LastBitID() > 0) { 
        if (num_bits > SHORT_THRESHOLD) {
          BitSetPtr()[NUM_FIELDS - 1] &= MaskLow<field_t>(LastBitID()); 
        } else {
          *BitSetPtr().Raw()  &= MaskLow<field_t>(LastBitID()); 
        }
        
      }
    }

    /// Regular print function (from most significant bit to least)
    void Print(std::ostream & out=std::cout) const {
      for (size_t i = num_bits; i > 0; --i) out << Get(i-1);
    }

    /// Print a space between each field (or other provided spacer)
    void PrintFields(std::ostream & out=std::cout, const std::string spacer=" ") const {
      for (size_t i = num_bits; i > 0; i--) {
        out << Get(i-1);
        if (i % FIELD_BITS == 0) out << spacer;
      }
    }

    /// Print from smallest bit position to largest.
    void PrintArray(std::ostream & out=std::cout) const {
      for (size_t i = 0; i < num_bits; i++) out << Get(i);
    }

    /// Print the positions of all one bits, spaces are the default separator.
    void PrintOneIDs(std::ostream & out=std::cout, std::string spacer=" ") const {
      for (size_t i = 0; i < num_bits; i++) { if (Get(i)) out << i << spacer; }
    }


    /// Count 1's by looping through once for each bit equal to 1
    size_t CountOnes_Sparse() const {
      const size_t NUM_FIELDS = NumFields();
      size_t bit_count = 0;
      if (num_bits > SHORT_THRESHOLD) {
        for (size_t i = 0; i < NUM_FIELDS; i++) {
          field_t cur_field = BitSetPtr()[i];
          while (cur_field) {
            cur_field &= (cur_field-1);       // Peel off a single 1.
            bit_count++;      // And increment the counter
          }
        }
      } else {
        field_t cur_field = *BitSetPtr().Raw();
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
      
       if (num_bits > SHORT_THRESHOLD) {
        for (size_t i = 0; i < NUM_FIELDS; i++) {
          // when compiling with -O3 and -msse4.2, this is the fastest population count method.
          std::bitset<FIELD_BITS> std_bs(BitSetPtr()[i]);
          bit_count += std_bs.count();
        }
      } else {
        std::bitset<FIELD_BITS> std_bs(*BitSetPtr().Raw());
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
      if (num_bits > SHORT_THRESHOLD) {
        while (field_id < NUM_FIELDS && BitSetPtr()[field_id]==0) field_id++;
        return (field_id < NUM_FIELDS) ?
          (int) (find_bit(BitSetPtr()[field_id]) + (field_id * FIELD_BITS))  :  -1;
      } else {
        return (field_id < NUM_FIELDS) ?
          (int) (find_bit(*BitSetPtr().Raw()) + (field_id * FIELD_BITS))  :  -1;
      }
      
    }

    /// Return the position of the first one and change it to a zero.  Return -1 if no ones.
    int PopBit() {
      const size_t NUM_FIELDS = NumFields();
      size_t field_id = 0;
      if (num_bits > SHORT_THRESHOLD) {
        while (field_id < NUM_FIELDS && BitSetPtr()[field_id]==0) field_id++;
        if (field_id == NUM_FIELDS) return -1;  // Failed to find bit!

        const size_t pos_found = find_bit(BitSetPtr()[field_id]);
        constexpr field_t val_one = 1;
        BitSetPtr()[field_id] &= ~(val_one << pos_found);
        return (int) (pos_found + (field_id * FIELD_BITS));
      } else {
        const size_t pos_found = find_bit(*BitSetPtr().Raw());
        constexpr field_t val_one = 1;
        *BitSetPtr().Raw() &= ~(val_one << pos_found);
        return (int) (pos_found + (field_id * FIELD_BITS));
      }
    }

    /// Return the position of the first one after start_pos; return -1 if no ones in vector.
    /// You can loop through all 1-bit positions of a BitVector "bv" with:
    ///
    ///   for (int pos = bv.FindBit(); pos >= 0; pos = bv.FindBit(pos+1)) { ... }

    int FindBit(const size_t start_pos) const {
      if (start_pos >= num_bits) return -1;
      size_t field_id  = FieldID(start_pos);     // What field do we start in?
      const size_t field_pos = FieldPos(start_pos);    // What position in that field?
      




      if (num_bits > SHORT_THRESHOLD) {
        if (field_pos && (BitSetPtr()[field_id] & ~(MaskLow<field_t>(field_pos)))) {  // First field hit!
          return (int) (find_bit(BitSetPtr()[field_id] & ~(MaskLow<field_t>(field_pos))) +
                      field_id * FIELD_BITS);
        }

        // Search other fields...
        const size_t NUM_FIELDS = NumFields();
        if (field_pos) field_id++;
        while (field_id < NUM_FIELDS && BitSetPtr()[field_id]==0) field_id++;
        return (field_id < NUM_FIELDS) ?
          (int) (find_bit(BitSetPtr()[field_id]) + (field_id * FIELD_BITS)) : -1;
      } else {
        if (field_pos && (*BitSetPtr().Raw() & ~(MaskLow<field_t>(field_pos)))) {  // First field hit!
          return (int) (find_bit(*BitSetPtr().Raw() & ~(MaskLow<field_t>(field_pos))) +
                      field_id * FIELD_BITS);
        }

        // Search other fields...
        const size_t NUM_FIELDS = NumFields();
        if (field_pos) field_id++;
        return (field_id < NUM_FIELDS) ?
          (int) (find_bit(*BitSetPtr().Raw()) + (field_id * FIELD_BITS)) : -1;
      }
    }

    /// Return positions of all ones.
    emp::vector<size_t> GetOnes() const {
      // @CAO -- There are probably better ways to do this with bit tricks.
      emp::vector<size_t> out_set(CountOnes());
      size_t cur_pos = 0;
      for (size_t i = 0; i < num_bits; i++) {
        if (Get(i)) out_set[cur_pos++] = i;
      }
      return out_set;
    }

    /// Perform a Boolean NOT on this BitVector and return the result.
    BitVector NOT() const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_set(*this);
      if (num_bits > SHORT_THRESHOLD) {
        for (size_t i = 0; i < NUM_FIELDS; i++) out_set.BitSetPtr()[i] = ~BitSetPtr()[i];
        if (LastBitID() > 0) out_set.BitSetPtr()[NUM_FIELDS - 1] &= MaskLow<field_t>(LastBitID());
      } else {
        *(out_set.BitSetPtr().Raw()) = ~(*(BitSetPtr().Raw()));
      }
      return out_set;
    }

    /// Perform a Boolean AND on this BitVector and return the result.
    BitVector AND(const BitVector & set2) const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_set(*this);
      if (num_bits > SHORT_THRESHOLD) {
        for (size_t i = 0; i < NUM_FIELDS; i++) out_set.BitSetPtr()[i] = BitSetPtr()[i] & set2.BitSetPtr()[i];
      } else {
        *(out_set.BitSetPtr().Raw()) = *BitSetPtr().Raw() & *(set2.BitSetPtr().Raw());
      }
      return out_set;
    }

    /// Perform a Boolean OR on this BitVector and return the result.
    BitVector OR(const BitVector & set2) const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_set(*this);
      if (num_bits > SHORT_THRESHOLD) {
        for (size_t i = 0; i < NUM_FIELDS; i++) out_set.BitSetPtr()[i] = BitSetPtr()[i] | set2.BitSetPtr()[i];
      } else {
        *(out_set.BitSetPtr().Raw()) = *BitSetPtr().Raw() | *set2.BitSetPtr().Raw();
      }
      return out_set;
    }

    /// Perform a Boolean NAND on this BitVector and return the result.
    BitVector NAND(const BitVector & set2) const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_set(*this);
      if (num_bits > SHORT_THRESHOLD) {
        for (size_t i = 0; i < NUM_FIELDS; i++) out_set.BitSetPtr()[i] = ~(BitSetPtr()[i] & set2.BitSetPtr()[i]);
        if (LastBitID() > 0) out_set.BitSetPtr()[NUM_FIELDS - 1] &= MaskLow<field_t>(LastBitID());
      } else {
        *(out_set.BitSetPtr().Raw()) = ~(*BitSetPtr().Raw() & *set2.BitSetPtr().Raw());
      }
      return out_set;
    }

    /// Perform a Boolean NOR on this BitVector and return the result.
    BitVector NOR(const BitVector & set2) const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_set(*this);
      if (num_bits > SHORT_THRESHOLD) {
        for (size_t i = 0; i < NUM_FIELDS; i++) out_set.BitSetPtr()[i] = ~(BitSetPtr()[i] | set2.BitSetPtr()[i]);
        if (LastBitID() > 0) out_set.BitSetPtr()[NUM_FIELDS - 1] &= MaskLow<field_t>(LastBitID());
      } else {
        *(out_set.BitSetPtr().Raw()) = ~(*BitSetPtr().Raw() | *set2.BitSetPtr().Raw());
      }
      return out_set;
    }

    /// Perform a Boolean XOR on this BitVector and return the result.
    BitVector XOR(const BitVector & set2) const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_set(*this);
      if (num_bits > SHORT_THRESHOLD) {
        for (size_t i = 0; i < NUM_FIELDS; i++) out_set.BitSetPtr()[i] = BitSetPtr()[i] ^ set2.BitSetPtr()[i];
      }else {
        *(out_set.BitSetPtr().Raw()) = *BitSetPtr().Raw() ^ *set2.BitSetPtr().Raw();
      }
      return out_set;
    }

    /// Perform a Boolean EQU on this BitVector and return the result.
    BitVector EQU(const BitVector & set2) const {
      const size_t NUM_FIELDS = NumFields();
      BitVector out_set(*this);
      if (num_bits > SHORT_THRESHOLD) {
        for (size_t i = 0; i < NUM_FIELDS; i++) out_set.BitSetPtr()[i] = ~(BitSetPtr()[i] ^ set2.BitSetPtr()[i]);
        if (LastBitID() > 0) out_set.BitSetPtr()[NUM_FIELDS - 1] &= MaskLow<field_t>(LastBitID());
      } else {
         *(out_set.BitSetPtr().Raw()) = ~(*BitSetPtr().Raw() ^ *set2.BitSetPtr().Raw());
      }
      return out_set;
    }


    /// Perform a Boolean NOT with this BitVector, store result here, and return this object.
    BitVector & NOT_SELF() {
      const size_t NUM_FIELDS = NumFields();
      if (num_bits > SHORT_THRESHOLD) {
        for (size_t i = 0; i < NUM_FIELDS; i++) BitSetPtr()[i] = ~BitSetPtr()[i];
        if (LastBitID() > 0) BitSetPtr()[NUM_FIELDS - 1] &= MaskLow<field_t>(LastBitID());
      } else {
        *BitSetPtr().Raw() = ~*BitSetPtr().Raw();
      }
      return *this;
    }

    /// Perform a Boolean AND with this BitVector, store result here, and return this object.
    BitVector & AND_SELF(const BitVector & set2) {
      const size_t NUM_FIELDS = NumFields();
      if (num_bits > SHORT_THRESHOLD) {
        for (size_t i = 0; i < NUM_FIELDS; i++) BitSetPtr()[i] = BitSetPtr()[i] & set2.BitSetPtr()[i];
      } else {
        *BitSetPtr().Raw() = *BitSetPtr().Raw() & *set2.BitSetPtr().Raw();
      }
      return *this;
    }

    /// Perform a Boolean OR with this BitVector, store result here, and return this object.
    BitVector & OR_SELF(const BitVector & set2) {
      const size_t NUM_FIELDS = NumFields();
      if (num_bits > SHORT_THRESHOLD) {
        for (size_t i = 0; i < NUM_FIELDS; i++) BitSetPtr()[i] = BitSetPtr()[i] | set2.BitSetPtr()[i];
      } else {
        *BitSetPtr().Raw() = *BitSetPtr().Raw() | *set2.BitSetPtr().Raw();
      }
      return *this;
    }

    /// Perform a Boolean NAND with this BitVector, store result here, and return this object.
    BitVector & NAND_SELF(const BitVector & set2) {
      const size_t NUM_FIELDS = NumFields();
      if (num_bits > SHORT_THRESHOLD) {
        for (size_t i = 0; i < NUM_FIELDS; i++) BitSetPtr()[i] = ~(BitSetPtr()[i] & set2.BitSetPtr()[i]);
        if (LastBitID() > 0) BitSetPtr()[NUM_FIELDS - 1] &= MaskLow<field_t>(LastBitID());
      } else {
        *BitSetPtr().Raw() = ~(*BitSetPtr().Raw() & *set2.BitSetPtr().Raw());
      }
      return *this;
    }

    /// Perform a Boolean NOR with this BitVector, store result here, and return this object.
    BitVector & NOR_SELF(const BitVector & set2) {
      const size_t NUM_FIELDS = NumFields();
      if (num_bits > SHORT_THRESHOLD) {
        for (size_t i = 0; i < NUM_FIELDS; i++) BitSetPtr()[i] = ~(BitSetPtr()[i] | set2.BitSetPtr()[i]);
        if (LastBitID() > 0) BitSetPtr()[NUM_FIELDS - 1] &= MaskLow<field_t>(LastBitID());
      } else {
        *BitSetPtr().Raw() = ~(*BitSetPtr().Raw() | *set2.BitSetPtr().Raw());
      }
      return *this;
    }

    /// Perform a Boolean XOR with this BitVector, store result here, and return this object.
    BitVector & XOR_SELF(const BitVector & set2) {
      const size_t NUM_FIELDS = NumFields();
      if (num_bits > SHORT_THRESHOLD) {
        for (size_t i = 0; i < NUM_FIELDS; i++) BitSetPtr()[i] = BitSetPtr()[i] ^ set2.BitSetPtr()[i];
      } else {
        *BitSetPtr().Raw() = *BitSetPtr().Raw() ^ *set2.BitSetPtr().Raw();
      }
      return *this;
    }

    /// Perform a Boolean EQU with this BitVector, store result here, and return this object.
    BitVector & EQU_SELF(const BitVector & set2) {
      const size_t NUM_FIELDS = NumFields();
      if (num_bits > SHORT_THRESHOLD) {
        for (size_t i = 0; i < NUM_FIELDS; i++) BitSetPtr()[i] = ~(BitSetPtr()[i] ^ set2.BitSetPtr()[i]);
        if (LastBitID() > 0) BitSetPtr()[NUM_FIELDS - 1] &= MaskLow<field_t>(LastBitID());
      } else {
        *BitSetPtr().Raw() = ~(*BitSetPtr().Raw() ^ *set2.BitSetPtr().Raw());
      }
      return *this;
    }

    /// Positive shifts go left and negative go right (0 does nothing); return result.
    BitVector SHIFT(const int shift_size) const {
      BitVector out_set(*this);
      if (shift_size > 0) out_set.ShiftRight((size_t) shift_size);
      else if (shift_size < 0) out_set.ShiftLeft((size_t) -shift_size);
      return out_set;
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

    /// Function to allow drop-in replacement with std::vector<bool>.
    size_t size() const { return num_bits; }

    /// Function to allow drop-in replacement with std::vector<bool>.
    void resize(std::size_t new_size) { Resize(new_size); }

    /// Function to allow drop-in replacement with std::vector<bool>.
    bool all() const { return All(); }

    /// Function to allow drop-in replacement with std::vector<bool>.
    bool any() const { return Any(); }

    /// Function to allow drop-in replacement with std::vector<bool>.
    bool none() const { return !Any(); }

    /// Function to allow drop-in replacement with std::vector<bool>.
    size_t count() const { return CountOnes_Mixed(); }
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
