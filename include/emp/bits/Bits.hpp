/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022.
 *
 *  @file Bits.hpp
 *  @brief A generic bit-handler to replace vector<bool>, etc +additional bitwise logic features.
 *  @note Status: RELEASE
 *
 *  The Bits template allows the user to recreate the functionality of std::vector<bool>,
 *  array<bool>, std::bitset, and other such bit-handling classes.
 * 
 *  This class stores an arbitrary number of bits in a set of "fields" (typically 32 bits or 64
 *  bits per field, depending on which should be faster.)  Individual bits can be extracted,
 *  -or- bitwise logic (including more complex bit magic) can be used on the groups of bits.
 *
 *  The template parameters are:
 *    DATA_T : How is memory managed?
 *    ZERO_LEFT : Should the index of zero be the left-most bit? (right-most if false)
 *
 *  Specializations are:
 *    BitVector       : A replacement for std::vector<bool> (index 0 is on left)
 *    BitValue        : Like BitVector, but index 0 is on the right
 *    StaticBitVector : Like Bitvector, but max size and fixed memory.
 *    StaticBitValue  : Like BitValue, but max size and fixed memory.
 *    BitArray        : A replacement for std::array<bool> (index 0 is on left)
 *    BitSet          : A replacement for std::bitset (index 0 is on right)
 * 
 *  In the case of replacements, the aim was for identical functionality, but many additional
 *  features, especially associated with bitwise logic operations.
 * 
 *  @note Compile with -O3 and -msse4.2 for fast bit counting.
 *
 * 
 *  @todo Most of the operators don't check to make sure that both Bit groups are the same size.
 *        We should create versions (Intersection() and Union()?) that adjust sizes if needed.
 *  @todo Do small BitVector optimization.  Currently we have number of bits (8 bytes) and a
 *        pointer to the memory for the bitset (another 8 bytes), but we could use those 16 bytes
 *        as 1 byte of size info followed by 15 bytes of bitset (120 bits!)
 *  @todo For large BitVectors we can use a factory to preserve/adjust bit info.  That should be
 *        just as efficient than a reserve, but without the need to store extra in-class info.
 *  @todo Think about how iterators should work for Bit collections.  It should probably go
 *        bit-by-bit, but there are very few circumstances where that would be useful.  Going
 *        through the positions of all ones would be more useful, but perhaps less intuitive.
 */

#ifndef EMP_BITS_BITS_HPP_INCLUDE
#define EMP_BITS_BITS_HPP_INCLUDE


#include <bitset>
#include <cstring>
#include <initializer_list>
#include <iostream>
#include <span>
#include <sstream>

#include "../base/array.hpp"
#include "../base/assert.hpp"
#include "../base/error.hpp"
#include "../base/Ptr.hpp"
#include "../base/vector.hpp"
#include "../datastructs/hash_utils.hpp"
#include "../math/constants.hpp"
#include "../math/math.hpp"
#include "../math/Random.hpp"
#include "../meta/type_traits.hpp"

#include "_bitset_helpers.hpp"
#include "bitset_utils.hpp"
#include "Bits_Data.hpp"


namespace emp {

  /// @brief A flexible base template to handle BitVector, BitArray, BitSet, & other combinations.
  /// @tparam DATA_T How is this Bits object allowed to change size?
  /// @tparam ZERO_LEFT Should the index of zero be the left-most bit? (right-most if false) 
  template <typename DATA_T, bool ZERO_LEFT>
  class Bits {
    using this_t = Bits<DATA_T, ZERO_LEFT>;
    using field_t = bits_field_t;

    // All internal data (and base-level manipulators) for Bits.
    DATA_T _data;

    static constexpr size_t FIELD_BITS = NUM_FIELD_BITS;

    // Number of bits needed to specify position in a field + mask
    static constexpr size_t FIELD_LOG2 = static_cast<size_t>(emp::Log2(FIELD_BITS));
    static constexpr field_t FIELD_LOG2_MASK = MaskLow<field_t>(FIELD_LOG2);

    static constexpr field_t FIELD_0 = (field_t) 0;      ///< All bits in a field set to 0
    static constexpr field_t FIELD_1 = (field_t) 1;      ///< Least significant bit set to 1
    static constexpr field_t FIELD_255 = (field_t) 255;  ///< Least significant 8 bits set to 1
    static constexpr field_t FIELD_ALL = ~FIELD_0;       ///< All bits in a field set to 1

    // Identify the field that a specified bit is in.
    [[nodiscard]] static constexpr size_t FieldID(const size_t index)  { return index / FIELD_BITS; }

    // Identify the position within a field where a specified bit is.
    [[nodiscard]] static constexpr size_t FieldPos(const size_t index) { return index & (FIELD_BITS-1); }

    // Identify which field a specified byte position would be in.
    [[nodiscard]] static constexpr size_t Byte2Field(const size_t index) { return index / sizeof(field_t); }

    // Convert a byte position in Bits to a byte position in the target field.
    [[nodiscard]] static constexpr size_t Byte2FieldPos(const size_t index) { return FieldPos(index * 8); }

    [[nodiscard]] constexpr field_t MaskField(size_t mask_size) const {
      return MaskLow<field_t>(mask_size);
    }
    [[nodiscard]] constexpr field_t MaskField(size_t mask_size, size_t offset) const {
      return MaskLow<field_t>(mask_size) << offset;
    }

    // Assume that the size of the bits has already been adjusted to be the size of the one
    // being copied and only the fields need to be copied over.
    void RawCopy(const Ptr<const field_t> from, size_t copy_fields=emp::MAX_SIZE_T);

    // Shortcut for RawCopy if we are copying a whole other Bits object.
    template <typename DATA2_T, bool ZERO_LEFT2>
    void RawCopy(const Bits<DATA2_T, ZERO_LEFT2> & in_bits) {
      RawCopy(in_bits.FieldPtr(), in_bits.NumFields());
    }

    // Copy bits from one position in the genome to another; leave old positions unchanged.
    constexpr void RawMove(const size_t from_start, const size_t from_stop, const size_t to);

    // Convert the bits to bytes (note that bits are NOT in order at the byte level!)
    [[nodiscard]] emp::Ptr<unsigned char> BytePtr() { return _data.BytePtr(); }

    // Convert the bits to const bytes array (note that bits are NOT in order at the byte level!)
    [[nodiscard]] emp::Ptr<const unsigned char> BytePtr() const { return _data.BytePtr(); }

    // Any bits past the last "real" bit in the last field should be kept as zeros.
    constexpr this_t & ClearExcessBits() {
      if (_data.NumEndBits()) _data.bits[_data.LastField()] &= _data.EndMask();
      return *this;
    }

    // Apply a transformation to each bit field in a specified range.
    template <typename FUN_T>
    Bits<DATA_T,ZERO_LEFT> & ApplyRange(const FUN_T & fun, size_t start, size_t stop);

    // Helper: call SHIFT with positive number
    constexpr void ShiftLeft(const size_t shift_size);

    // Helper for calling SHIFT with negative number
    // Raw indicates if we should keep bits that are technically out of range; may be needed if
    // we are trying to shift bits back INTO range after another operation.
    constexpr void ShiftRight(const size_t shift_size, bool raw=false);

    /// Helper: call ROTATE with negative number instead
    constexpr void ROTL_SELF(const size_t shift_size_raw);

    /// Helper for calling ROTATE with positive number
    constexpr void ROTR_SELF(const size_t shift_size_raw);

  public:
    /// @brief Default constructor; will build the default number of bits (often 0, but not always)
    /// @param init_val Initial value of all default bits. 
    Bits(bool init_val=0) { if (init_val) SetAll(); else Clear(); }

    /// @brief Build a new Bits with specified bit count and initialization (default 0)
    Bits(size_t in_num_bits, bool init_val=false);

    // Prevent ambiguous conversions...
    /// @brief Anything not otherwise defined for first argument, convert to size_t.
    template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, int>::type = 0>
    Bits(T in_num_bits, bool init_val=false) : Bits(static_cast<size_t>(in_num_bits), init_val) {}

    /// @brief Copy constructor of existing bits object.
    Bits(const Bits & in) = default;

    /// @brief Constructor for other type of existing bits object.
    template <typename DATA2_T, bool ZERO_LEFT2>
    Bits(const Bits<DATA2_T, ZERO_LEFT2> & in);

    /// @brief Move constructor of existing bit field.
    Bits(this_t && in) = default;

    /// @brief Constructor to generate a Bits from a std::bitset.
    template <size_t NUM_BITS>
    explicit Bits(const std::bitset<NUM_BITS> & bitset);

    /// @brief Constructor to generate a Bits from a string of '0's and '1's.
    Bits(const std::string & bitstring);

    /// @brief Constructor to generate a Bits from a literal string of '0's and '1's.
    Bits(const char * bitstring) : Bits(std::string(bitstring)) {}

    /// @brief Constructor to generate a random set of bits in the default size.
    /// @param random Random number generator to use.
    Bits(Random & random);

    /// @brief Constructor to generate random Bits with provided prob of 1's, default size.
    /// @param random Random number generator to use.
    /// @param p1 Probability of a bit being a one.
    Bits(Random & random, const double p1);

    /// @brief Constructor to generate random Bits with specified # of ones, default size.
    /// @param random Random number generator to use.
    /// @param target_ones Number of ones to include in the Bits.
    Bits(Random & random, const size_t target_ones);

    /// @brief Constructor to generate random Bits with specified # of ones, default size.
    /// @param random Random number generator to use.
    /// @param target_ones Number of ones to include in the Bits.
    Bits(Random & random, const int target_ones) : Bits(random, (size_t) target_ones) { }

    /// @brief Constructor to generate a specified number of random Bits (with equal prob of 0 or 1).
    Bits(size_t in_num_bits, Random & random);

    /// @brief Constructor to generate a random Bits with provided prob of 1's.
    Bits(size_t in_num_bits, Random & random, const double p1);

    /// @brief Constructor to generate a random Bits with provided number of 1's.
    Bits(size_t in_num_bits, Random & random, const size_t target_ones);

    /// @brief Constructor to generate a random Bits with provided number of 1's.
    Bits(size_t in_num_bits, Random & random, const int target_ones)
      : Bits(in_num_bits, random, (size_t) target_ones) { }

    /// @brief Initializer list constructor.
    template <typename T> Bits(const std::initializer_list<T> l);

    /// @brief Copy, but with a resize.
    template <typename DATA2_T, bool ZERO_LEFT2>
    Bits(const Bits<DATA2_T, ZERO_LEFT2> & in, size_t new_size);

    /// @brief Destructor
    ~Bits() = default;

    /// @brief Copy assignment operator.
    Bits & operator=(const Bits<DATA_T, ZERO_LEFT> & in) &;

    /// @brief Assignment operator for other Bits object
    template <typename DATA2_T, bool ZERO_LEFT2>
    Bits & operator=(const Bits<DATA2_T, ZERO_LEFT2> & in) &;

    /// @brief Move operator.
    Bits & operator=(Bits && in) &;

    /// @brief Assignment operator from a std::bitset.
    template <size_t NUM_BITS>
    Bits & operator=(const std::bitset<NUM_BITS> & bitset) &;

    /// @brief Assignment operator from a string of '0's and '1's.
    Bits & operator=(const std::string & bitstring) &;

    /// @brief Assignment operator from a literal string of '0's and '1's.
    Bits & operator=(const char * bitstring) & { return operator=(std::string(bitstring)); }

    /// @brief Assignment from another Bits object without changing size.
    template <typename DATA2_T, bool ZERO_LEFT2>
    Bits & Import(
      const Bits<DATA2_T, ZERO_LEFT2> & from_bits,
      const size_t from_start_pos=0,
      size_t max_copy_bits=emp::MAX_SIZE_T
    );

    /// @brief Convert to a Bits of a different size.
    template <typename OUT_T = Bits<Bits_DynamicData,ZERO_LEFT> >
    [[nodiscard]] OUT_T Export(size_t out_size, size_t start_bit=0) const;

    /// @brief Convert to a BitArray of a different size.
    template <size_t NUM_BITS>
    [[nodiscard]] Bits<Bits_FixedData<NUM_BITS>,true>
    ExportArray(size_t start_bit=0) const {
      return Export< Bits<Bits_FixedData<NUM_BITS>,true> >(NUM_BITS, start_bit);
    }

    /// @brief concatenate another Bits object on to the end of this one.
    template<typename DATA2_T, bool ZERO_LEFT2>
    Bits & Append(const Bits<DATA2_T, ZERO_LEFT2> & in_bits);

    // @brief Scan this bitvector to make sure that there are no internal problems.
    [[nodiscard]] bool OK() const { return _data.OK(); }


    // =========  Accessors  ========= //

    /// @brief How many bits do we currently have?
    [[nodiscard]] constexpr auto GetSize() const { return _data.NumBits(); }

    /// @brief How many bytes are in this Bits? (includes empty field space)
    [[nodiscard]] constexpr auto GetNumBytes() const { return _data.NumBytes(); }

    /// @brief How many distinct values could be held in this Bits?
    [[nodiscard]] constexpr double GetNumStates() const { return emp::Pow2(_data.NumBits()); }

    /// @brief Retrieve the bit value from the specified index.
    [[nodiscard]] constexpr bool Get(size_t index) const;

    /// @brief A safe version of Get() for indexing out of range. Useful for representing collections.
    [[nodiscard]] constexpr bool Has(size_t index) const {
      return (index < _data.NumBits()) ? Get(index) : false;
    }

    /// @brief Update the bit value at the specified index.
    Bits & Set(size_t index, bool value=true);

    /// @brief Set all bits to 1.
    Bits & SetAll();

    /// @brief Set a range of bits to one: [start, stop)
    Bits & SetRange(size_t start, size_t stop)
      { return ApplyRange([](field_t){ return FIELD_ALL; }, start, stop); }

    /// @brief Set all bits to 0.
    Bits & Clear();

    /// @brief Set specific bit to 0.
    Bits & Clear(size_t index) { return Set(index, false); }

    /// @brief Set bits to 0 in the range [start, stop)
    Bits & Clear(const size_t start, const size_t stop) {
      return ApplyRange([](field_t) -> size_t { return 0; }, start, std::min(stop,GetSize()));
    }


    /// @brief Const index operator -- return the bit at the specified position.
    [[nodiscard]] bool operator[](size_t index) const { return Get(index); }

    /// @brief Index operator; return proxy to bit at specified position usable as an lvalue.
    BitProxy<Bits> operator[](size_t index) { return BitProxy<Bits>(*this, index); }

    /// @brief Change every bit in the sequence.
    Bits & Toggle() { return NOT_SELF(); }

    /// @brief Change a specified bit to the opposite value
    Bits & Toggle(size_t index);

    /// @brief Flips all the bits in a range [start, end)
    Bits & Toggle(size_t start, size_t stop)
      { return ApplyRange([](field_t x){ return ~x; }, start, stop); }

    /// @brief Return true if ANY bits are set to 1, otherwise return false.
    [[nodiscard]] bool Any() const;

    /// @brief Return true if NO bits are set to 1, otherwise return false.
    [[nodiscard]] bool None() const { return !Any(); }

    /// @brief Return true if ALL bits are set to 1, otherwise return false.
    // @CAO: Can speed up by not duplicating Bits; fields should be all 1, last should be mask.
    [[nodiscard]] bool All() const { return (~(*this)).None(); }

    /// @brief Resize this Bits object to have the specified number of bits (if allowed)
    Bits & Resize(size_t new_bits) { _data.RawResize(new_bits, true); return *this; }


    // =========  Randomization functions  ========= //

    /// @brief Set all bits randomly, with a 50% probability of being a 0 or 1.
    Bits & Randomize(Random & random);

    /// @brief Set all bits randomly, with probability specified at compile time.
    template <Random::Prob P>
    Bits & RandomizeP(Random & random, const size_t start_pos=0, size_t stop_pos=MAX_SIZE_T);

    /// @brief Set all bits randomly, with a given probability of being a one.
    Bits & Randomize(Random & random, const double p,
                     const size_t start_pos=0, size_t stop_pos=MAX_SIZE_T);

    /// @brief Set all bits randomly, with a given number of ones.
    Bits & ChooseRandom(Random & random, const size_t target_ones,
                        const size_t start_pos=0, size_t stop_pos=MAX_SIZE_T);

    /// @brief Flip random bits with a given probability.
    Bits & FlipRandom(Random & random, const double p,
                      const size_t start_pos=0, size_t stop_pos=MAX_SIZE_T);

    /// @brief Set random bits with a given probability (does not check if already set.)
    Bits & SetRandom(Random & random, const double p,
                     const size_t start_pos=0, size_t stop_pos=MAX_SIZE_T);

    /// @brief Unset random bits with a given probability (does not check if already zero.)
    Bits & ClearRandom(Random & random, const double p,
                       const size_t start_pos=0, size_t stop_pos=MAX_SIZE_T);

    /// @brief Flip a specified number of random bits.
    Bits & FlipRandomCount(Random & random, const size_t target_bits);

    /// @brief Set a specified number of random bits (does not check if already set.)
    Bits & SetRandomCount(Random & random, const size_t target_bits);

    /// @brief Unset  a specified number of random bits (does not check if already zero.)
    Bits & ClearRandomCount(Random & random, const size_t target_bits);


    // =========  Comparison Operators  ========= //

    /// @brief Compare two bits objects, even with different template arguments.
    template <typename DATA2_T, bool ZL2>
    [[nodiscard]] bool operator==(const Bits<DATA2_T,ZL2> & in) const;
    template <typename DATA2_T, bool ZL2>
    [[nodiscard]] bool operator!=(const Bits<DATA2_T,ZL2> & in) const { return !(*this == in); }
    template <typename DATA2_T, bool ZL2>
    [[nodiscard]] bool operator< (const Bits<DATA2_T,ZL2> & in) const;
    template <typename DATA2_T, bool ZL2>
    [[nodiscard]] bool operator> (const Bits<DATA2_T,ZL2> & in) const { return in < *this; }
    template <typename DATA2_T, bool ZL2>
    [[nodiscard]] bool operator<=(const Bits<DATA2_T,ZL2> & in) const { return !(in < *this); }
    template <typename DATA2_T, bool ZL2>
    [[nodiscard]] bool operator>=(const Bits<DATA2_T,ZL2> & in) const { return !(*this < in); }


    // =========  Conversion Operators  ========= //

    /// @brief Automatically convert Bits to other vector types.
    template <typename T> operator emp::vector<T>();

    /// @brief Casting a bit array to bool identifies if ANY bits are set to 1.
    explicit operator bool() const { return Any(); }


    // =========  Access Groups of bits  ========= //

    /// @brief Retrieve the byte at the specified byte index.
    [[nodiscard]] uint8_t GetByte(size_t index) const;

    /// @brief et a read-only view into the internal array used by Bits.
    /// @return Read-only span of Bits's bytes.
    [[nodiscard]] auto GetBytes() const { return _data.AsByteSpan(); }

    /// @brief Return a span with all fields in order.
    [[nodiscard]] std::span<field_t> FieldSpan() {
      return std::span<field_t>(_data.FieldPtr().Raw(), _data.NumFields());
    }

    /// @brief Return a const span with all fields in order.
    [[nodiscard]] std::span<const field_t> FieldSpan() const {
      return std::span<const field_t>(_data.FieldPtr().Raw(), _data.NumFields());
    }

    [[nodiscard]] size_t NumFields() const { return _data.NumFields(); }

    /// @brief Return a pointer to the set of fields.
    [[nodiscard]] auto FieldPtr() { return _data.FieldPtr(); }

    /// @brief Return a const pointer to the set of fields.
    [[nodiscard]] auto FieldPtr() const { return _data.FieldPtr(); }

    /// @brief Get a read-only pointer to the internal array used by Bits.
    /// (note that bits are NOT in order at the byte level!)
    /// @return Read-only pointer to Bits' bytes.
    [[nodiscard]] emp::Ptr<const unsigned char> RawBytes() const { return BytePtr(); }

    /// @brief Update the byte at the specified byte index.
    void SetByte(size_t index, uint8_t value);

    /// @brief Get overall base-2 value of this Bits, returning as a double.
    [[nodiscard]] double GetValue() const;

    /// @brief Get specified type at a given index (in steps of that type size)
    template <typename T>
    [[nodiscard]] T GetValueAtIndex(const size_t index) const;

    /// @brief Retrieve the 8-bit uint from the specified uint index.
    [[nodiscard]] uint8_t GetUInt8(size_t index) const { return GetValueAtIndex<uint8_t>(index); }

    /// @brief Retrieve the 16-bit uint from the specified uint index.
    [[nodiscard]] uint16_t GetUInt16(size_t index) const { return GetValueAtIndex<uint16_t>(index); }

    /// @brief Retrieve the 32-bit uint from the specified uint index.
    [[nodiscard]] uint32_t GetUInt32(size_t index) const { return GetValueAtIndex<uint32_t>(index); }

    /// @brief Retrieve the 64-bit uint from the specified uint index.
    [[nodiscard]] uint64_t GetUInt64(size_t index) const { return GetValueAtIndex<uint64_t>(index); }

    /// @brief By default, retrieve the 32-bit uint from the specified uint index.
    [[nodiscard]] uint32_t GetUInt(size_t index) const { return GetUInt32(index); }


    /// @brief Set specified type at a given index (in steps of that type size)
    template <typename T> Bits & SetValueAtIndex(const size_t index, T value);

    /// @brief Update the 8-bit uint at the specified uint index.
    void SetUInt8(const size_t index, uint8_t value) { SetValueAtIndex(index, value); }

    /// @brief Update the 16-bit uint at the specified uint index.
    void SetUInt16(const size_t index, uint16_t value) { SetValueAtIndex(index, value); }

    /// @brief Update the 32-bit uint at the specified uint index.
    void SetUInt32(const size_t index, uint32_t value) { SetValueAtIndex(index, value); }

    /// @brief Update the 64-bit uint at the specified uint index.
    void SetUInt64(const size_t index, uint64_t value) { SetValueAtIndex(index, value); }

    /// @brief By default, update the 32-bit uint at the specified uint index.
    void SetUInt(const size_t index, uint32_t value) { SetUInt32(index, value); }


    /// @briefGet specified type starting at a given BIT position.
    template <typename T>
    [[nodiscard]] T GetValueAtBit(const size_t index) const;

    /// @brief Retrieve the 8-bit uint from the specified uint index.
    [[nodiscard]] uint8_t GetUInt8AtBit(size_t index) const { return GetValueAtBit<uint8_t>(index); }

    /// @brief Retrieve the 16-bit uint from the specified uint index.
    [[nodiscard]] uint16_t GetUInt16AtBit(size_t index) const { return GetValueAtBit<uint16_t>(index); }

    /// @brief Retrieve the 32-bit uint from the specified uint index.
    [[nodiscard]] uint32_t GetUInt32AtBit(size_t index) const { return GetValueAtBit<uint32_t>(index); }

    /// @brief Retrieve the 64-bit uint from the specified uint index.
    [[nodiscard]] uint64_t GetUInt64AtBit(size_t index) const { return GetValueAtBit<uint64_t>(index); }

    /// @brief By default, retrieve the 32-bit uint from the specified uint index.
    [[nodiscard]] uint32_t GetUIntAtBit(size_t index) const { return GetUInt32AtBit(index); }


    template <typename T> Bits & SetValueAtBit(const size_t index, T value);

    /// @brief Update the 8-bit uint at the specified uint index.
    void SetUInt8AtBit(const size_t index, uint8_t value) { SetValueAtBit(index, value); }

    /// @brief Update the 16-bit uint at the specified uint index.
    void SetUInt16AtBit(const size_t index, uint16_t value) { SetValueAtBit(index, value); }

    /// @brief Update the 32-bit uint at the specified uint index.
    void SetUInt32AtBit(const size_t index, uint32_t value) { SetValueAtBit(index, value); }

    /// @brief Update the 64-bit uint at the specified uint index.
    void SetUInt64AtBit(const size_t index, uint64_t value) { SetValueAtBit(index, value); }

    /// @brief By default, update the 32-bit uint at the specified uint index.
    void SetUIntAtBit(const size_t index, uint32_t value) { SetUInt32AtBit(index, value); }


    // =========  Other Analyses  ========= //

    /// @brief A simple hash function for bit vectors.
    [[nodiscard]] std::size_t Hash(size_t start_field=0) const;

    /// @brief Count the number of ones in Bits.
    [[nodiscard]] constexpr size_t CountOnes() const;

    /// @brief Count the number of ones in a range within Bits.  [start, end)
    [[nodiscard]] constexpr size_t CountOnes(size_t start, size_t end) const;

    /// @brief Faster counting of ones for very sparse bit vectors.
    [[nodiscard]] constexpr size_t CountOnes_Sparse() const;

    /// @brief Count the number of zeros in Bits.
    [[nodiscard]] constexpr size_t CountZeros() const { return GetSize() - CountOnes(); }

    /// @brief Pop the last bit in the vector.
    /// @return value of the popped bit.
    bool PopBack();

    /// @brief Push given bit(s) onto the back of a vector.
    /// @param bit value of bit to be pushed.
    /// @param num number of bits to be pushed.
    void PushBack(const bool bit=true, const size_t num=1);

    /// @brief Insert bit(s) into any index of vector using bit magic.
    /// Blog post on implementation reasoning: https://devolab.org/?p=2249
    /// @param index location to insert bit(s).
    /// @param val value of bit(s) to insert.
    /// @param num number of bits to insert, default 1.
    void Insert(const size_t index, const bool val=true, const size_t num=1);

    /// @brief Delete bits from any index in a vector.
    // TODO: consider a bit magic approach here.
    /// @param index location to delete bit(s).
    /// @param num number of bits to delete, default 1.
    void Delete(const size_t index, const size_t num=1);

    /// @brief Return the position of the first one; return -1 if no ones in vector.
    [[nodiscard]] int FindOne() const;

    /// Deprecated: Return the position of the first one; return -1 if no ones in vector.
    [[deprecated("Renamed to more accurate FindOne()")]]
    [[nodiscard]] int FindBit() const { return FindOne(); }

    /// @brief Return the position of the first one after start_pos (or -1 if none)
    /// You can loop through all 1-bit positions of Bits object "bits" with:
    ///
    ///   for (int pos = bits.FindOne(); pos >= 0; pos = bits.FindOne(pos+1)) { ... }
    ///
    [[nodiscard]] int FindOne(const size_t start_pos) const;

    /// @brief Special version of FindOne takes int; most common way to call.
    [[nodiscard]] int FindOne(int start_pos) const {
      return FindOne(static_cast<size_t>(start_pos));
    }

    /// Deprecated version of FindOne().
    [[deprecated("Renamed to more accurate FindOne(start_pos)")]]
    [[nodiscard]] int FindBit(const size_t start_pos) const;

    /// @brief Find the most-significant set-bit.
    [[nodiscard]] int FindMaxOne() const;

    /// @brief Return the position of the first one and change it to a zero.  Return -1 if none.
    int PopOne();

    /// Deprecated version of PopOne().
    [[deprecated("Renamed to more accurate PopOne()")]]
    int PopBit() { return PopOne(); }

    /// @brief Return vector of positions of all ones.
    [[nodiscard]] emp::vector<size_t> GetOnes() const;

    /// @brief Collect positions of ones in the provided vector (allows id type choice)
    template <typename T>
    emp::vector<T> & GetOnes(emp::vector<T> & out_vals) const;

    /// @brief Find the length of the longest continuous series of ones.
    [[nodiscard]] size_t LongestSegmentOnes() const;

    /// @brief Return true if any ones are in common with another Bits.
    [[nodiscard]] bool HasOverlap(const Bits & in) const;


    // =========  Print/String Functions  ========= //

    /// @brief Convert a specified bit to a character.
    [[nodiscard]] char GetAsChar(size_t id) const { return Get(id) ? '1' : '0'; }

    /// @brief Convert this Bits to a vector string [index 0 based on ZERO_LEFT]
    [[nodiscard]] std::string ToString() const;

    /// @brief Convert this Bits to an array-based string [index 0 on left]
    [[nodiscard]] std::string ToArrayString() const;

    /// @brief Convert this Bits to a numerical string [index 0 on right]
    [[nodiscard]] std::string ToBinaryString() const;

    /// @brief Convert this Bits to a series of IDs
    [[nodiscard]] std::string ToIDString(const std::string & spacer=" ") const;

    /// @brief Convert this Bits to a series of IDs with ranges condensed.
    [[nodiscard]] std::string ToRangeString(const std::string & spacer=",",
                                            const std::string & ranger="-") const;

    /// @brief Regular print function (from least significant bit to most)
    void Print(std::ostream & out=std::cout) const { out << ToString(); }

    /// @brief Numerical print function (from most significant bit to least)
    void PrintBinary(std::ostream & out=std::cout) const { out << ToBinaryString(); }

    /// @brief Print from smallest bit position to largest.
    void PrintArray(std::ostream & out=std::cout) const { out << ToArrayString(); }

    /// @brief Print a space between each field (or other provided spacer)
    void PrintFields(std::ostream & out=std::cout, const std::string & spacer=" ") const;

    /// @brief Print out details about the internals of Bits.
    void PrintDebug(std::ostream & out=std::cout, const std::string & label="") const;

    /// @brief Print the positions of all one bits, spaces are the default separator.
    void PrintOneIDs(std::ostream & out=std::cout, const std::string & spacer=" ") const;

    /// @brief Print the ones in a range format.  E.g., 2-5,7,10-15
    void PrintAsRange(std::ostream & out=std::cout,
                      const std::string & spacer=",",
                      const std::string & ranger="-") const;

    /// @brief Overload ostream operator to return Print.
    friend std::ostream& operator<<(std::ostream &out, const Bits & bits) {
      bits.Print(out);
      return out;
    }


    // =========  Boolean Logic and Shifting Operations  ========= //

    /// @brief Perform a Boolean NOT with this Bits, store result here, and return this object.
    Bits & NOT_SELF();

    /// @brief Perform a Boolean AND with this Bits, store result here, and return this object.
    Bits & AND_SELF(const Bits & bits2);

    /// @brief Perform a Boolean OR with this Bits, store result here, and return this object.
    Bits & OR_SELF(const Bits & bits2);

    /// @brief Perform a Boolean NAND with this Bits, store result here, and return this object.
    Bits & NAND_SELF(const Bits & bits2);

    /// @brief Perform a Boolean NOR with this Bits, store result here, and return this object.
    Bits & NOR_SELF(const Bits & bits2);

    /// @brief Perform a Boolean XOR with this Bits, store result here, and return this object.
    Bits & XOR_SELF(const Bits & bits2);

    /// @brief Perform a Boolean EQU with this Bits, store result here, and return this object.
    Bits & EQU_SELF(const Bits & bits2);


    /// @brief Perform a Boolean NOT on this Bits and return the result.
    [[nodiscard]] Bits NOT() const { return Bits(*this).NOT_SELF(); }

    /// @brief Perform a Boolean AND on this Bits and return the result.
    [[nodiscard]] Bits AND(const Bits & bits2) const { return Bits(*this).AND_SELF(bits2); }

    /// @brief Perform a Boolean OR on this Bits and return the result.
    [[nodiscard]] Bits OR(const Bits & bits2) const { return Bits(*this).OR_SELF(bits2); }

    /// @brief Perform a Boolean NAND on this Bits and return the result.
    [[nodiscard]] Bits NAND(const Bits & bits2) const { return Bits(*this).NAND_SELF(bits2); }

    /// @brief Perform a Boolean NOR on this Bits and return the result.
    [[nodiscard]] Bits NOR(const Bits & bits2) const { return Bits(*this).NOR_SELF(bits2); }

    /// @brief Perform a Boolean XOR on this Bits and return the result.
    [[nodiscard]] Bits XOR(const Bits & bits2) const { return Bits(*this).XOR_SELF(bits2); }

    /// @brief Perform a Boolean EQU on this Bits and return the result.
    [[nodiscard]] Bits EQU(const Bits & bits2) const { return Bits(*this).EQU_SELF(bits2); }


    /// @brief Positive shifts left and negative right (0 does nothing); return result.
    [[nodiscard]] Bits SHIFT(const int shift_size) const;

    /// @brief Positive shifts left and negative right; store result here, and return *this.
    Bits & SHIFT_SELF(const int shift_size);

    /// @brief Reverse the order of bits in the bitset
    Bits & REVERSE_SELF();

    /// @brief Reverse order of bits in the bitset.
    [[nodiscard]] Bits REVERSE() const;

    /// @brief Positive rotates right and negative goes left; return result.
    [[nodiscard]] Bits ROTATE(const int rotate_size) const;

    /// @brief Positive rotates right and negative goes left; store here, and return *this.
    Bits & ROTATE_SELF(const int rotate_size);

    /// @brief Sums two Bits objects (following uint rules); returns result.
    [[nodiscard]] Bits ADD(const Bits & set2) const;

    /// @brief Sums another Bits object onto this one (following uint rules); returns *this.
    Bits & ADD_SELF(const Bits & set2);

    /// @brief Subtracts on Bits object from another (following uint rules); returns result.
    [[nodiscard]] Bits SUB(const Bits & set2) const;

    /// @brief Subtracts another Bits object from this one (following uint rules); returns *this.
    Bits & SUB_SELF(const Bits & set2);


    /// @brief Operator bitwise NOT...
    [[nodiscard]] inline Bits operator~() const { return NOT(); }

    /// @brief Operator bitwise AND...
    [[nodiscard]] inline Bits operator&(const Bits & ar2) const {
      emp_assert(size() == ar2.size(), size(), ar2.size());
      return AND(ar2);
    }

    /// @brief Operator bitwise OR...
    [[nodiscard]] inline Bits operator|(const Bits & ar2) const {
      emp_assert(size() == ar2.size(), size(), ar2.size());
      return OR(ar2);
    }

    /// @brief Operator bitwise XOR...
    [[nodiscard]] inline Bits operator^(const Bits & ar2) const {
      emp_assert(size() == ar2.size(), size(), ar2.size());
      return XOR(ar2);
    }

    /// @brief Operator shift left...
    [[nodiscard]] inline Bits operator<<(const size_t shift_size) const { return SHIFT(-(int)shift_size); }

    /// @brief Operator shift right...
    [[nodiscard]] inline Bits operator>>(const size_t shift_size) const { return SHIFT((int)shift_size); }

    /// @brief Compound operator bitwise AND...
    Bits & operator&=(const Bits & ar2) {
      emp_assert(size() == ar2.size()); return AND_SELF(ar2);
    }

    /// @brief Compound operator bitwise OR...
    Bits & operator|=(const Bits & ar2) {
      emp_assert(size() == ar2.size()); return OR_SELF(ar2);
    }

    /// @brief Compound operator bitwise XOR...
    Bits & operator^=(const Bits & ar2) {
      emp_assert(size() == ar2.size()); return XOR_SELF(ar2);
    }

    /// @brief Compound operator for shift left...
    Bits & operator<<=(const size_t shift_size) { return SHIFT_SELF(-(int)shift_size); }

    /// @brief Compound operator for shift right...
    Bits & operator>>=(const size_t shift_size) { return SHIFT_SELF((int)shift_size); }

    /// @brief Operator plus...
    [[nodiscard]] Bits operator+(const Bits & ar2) const { return ADD(ar2); }

    /// @brief Operator minus...
    [[nodiscard]] Bits operator-(const Bits & ar2) const { return SUB(ar2); }

    /// @brief Compound operator plus...
    const Bits & operator+=(const Bits & ar2) { return ADD_SELF(ar2); }

    /// @brief Compound operator minus...
    const Bits & operator-=(const Bits & ar2) { return SUB_SELF(ar2); }


    // =========  Cereal Compatability  ========= //

    /// @brief Setup this bits object so that it can be stored in an archive and re-loaded.
    template <class Archive>
    void serialize(Archive & ar) { ar(_data); }


    // =========  Standard Library Compatability  ========= //
    // A set of functions to allow drop-in replacement with std::bitset.

    [[nodiscard]] constexpr size_t size() const { return _data.NumBits(); }
    auto at(size_t pos) { return operator[](pos); }
    auto at(size_t pos) const { return operator[](pos); }
    auto front() { return at(0); }
    auto front() const { return at(0); }
    auto back() { return at(GetSize()-1); }
    auto back() const { return at(GetSize()-1); }
    void resize(std::size_t new_size) { Resize(new_size); }
    void push_back(const bool bit=true, const size_t num=1) { PushBack(bit, num); }
    void pop_back() { resize(GetSize() - 1); }
    [[nodiscard]] constexpr bool all() const { return All(); }
    [[nodiscard]] constexpr bool any() const { return Any(); }
    [[nodiscard]] constexpr bool none() const { return !Any(); }
    [[nodiscard]] constexpr size_t count() const { return CountOnes(); }
    Bits & flip() { return Toggle(); }
    Bits & flip(size_t pos) { return Toggle(pos); }
    Bits & flip(size_t start, size_t end) { return Toggle(start, end); }
    void reset() { Clear(); }
    void reset(size_t id) { Set(id, false); }
    void set() { SetAll(); }
    void set(size_t id) { Set(id); }
    [[nodiscard]] bool test(size_t index) const { return Get(index); }
    auto data() { return FieldSpan(); }
    auto data() const { return FieldSpan(); }
  };



  // ------------------------ Implementations for Internal Functions ------------------------

  template <typename DATA_T, bool ZERO_LEFT>
  void Bits<DATA_T, ZERO_LEFT>::
    RawCopy(const Ptr<const bits_field_t> from, size_t num_fields)
  {
    // If num_fields was not specified, set it to the max number of fields.
    if (num_fields == emp::MAX_SIZE_T) num_fields = _data.NumFields();

    emp_assert(num_fields <= _data.NumFields(), "Trying to RawCopy() more fields than can fit.");

    for (size_t i = 0; i < num_fields; i++) _data.bits[i] = from[i];
  }

  // Move bits from one position in the genome to another; leave old positions unchanged.
  // All positions are requires to exist and memory must be available for the move.
  // @CAO: Can speed up by focusing only on the moved fields (i.e., don't shift unused bits).
  template <typename DATA_T, bool ZERO_LEFT>
  constexpr void Bits<DATA_T, ZERO_LEFT>::
    RawMove(const size_t from_start, const size_t from_stop, const size_t to)
  {
    emp_assert(from_start <= from_stop);             // Must move legal region.
    emp_assert(from_stop <= _data.NumBits());         // Cannot move from past end.
    emp_assert(to <= _data.NumBits());                // Must move to somewhere legal.

    const size_t move_size = from_stop - from_start; // How big is the chunk to move?
    emp_assert(to + move_size <= _data.NumBits());    // Must fit in new position.

    // If nothing to copy OR already in place, stop right there.
    if (move_size == 0 || from_start == to) return;

    const size_t to_stop = to + move_size;           // Where is the end to move it to?
    const int shift = (int) from_start - (int) to;   // How far will the moved piece shift?
    this_t move_bits(*this);                         // Place to hold moved bits.
    move_bits.SHIFT_SELF(shift);                     // Put the moved bits in place.
    Clear(to, to_stop);                              // Make room for the moved bits.
    move_bits.Clear(0, to);                          // Clear everything BEFORE moved bits.
    move_bits.Clear(to_stop, _data.NumBits());        // Clear everything AFTER moved bits.
    OR_SELF(move_bits);                              // Merge bit strings together.
  }

  template <typename DATA_T, bool ZERO_LEFT>
  template <typename FUN_T>
  Bits<DATA_T,ZERO_LEFT> & Bits<DATA_T,ZERO_LEFT>::
    ApplyRange(const FUN_T & fun, size_t start, size_t stop)
  {
    emp_assert(start <= stop, start, stop, _data.NumBits());   // Start cannot be after stop.
    emp_assert(stop <= _data.NumBits(), stop, _data.NumBits()); // Stop must be in range.

    if (start == stop) return *this;  // Empty range.

    const size_t start_pos = FieldPos(start);  // Start position WITHIN a bit field.
    const size_t stop_pos = FieldPos(stop);    // Stop position WITHIN a bit field.
    size_t start_field = FieldID(start);       // ID of bit field we're starting in.
    const size_t stop_field = FieldID(stop);   // ID of last field to actively scan.

    // If all bits are in the same field, mask off the middle.
    if (start_field == FieldID(stop-1)) {
      const size_t apply_bits = stop - start;                  // How many bits to change?
      const field_t mask = MaskField(apply_bits, start_pos);   // Target change bits with a mask.
      field_t & target = _data.bits[start_field];               // Isolate the field to change.
      target = (target & ~mask) | (fun(target) & mask);        // Update targeted bits!
    }

    // Otherwise mask the ends and fully modify the chunks in between.
    else {
      // If we're only using a portions of start field, mask it and setup.
      if (start_pos != 0) {
        const size_t start_bits = FIELD_BITS - start_pos;       // How many bits in start field?
        const field_t mask = MaskField(start_bits, start_pos);  // Target start bits with a mask.
        field_t & target = _data.bits[start_field];              // Isolate the field to change.
        target = (target & ~mask) | (fun(target) & mask);       // Update targeted bits!
        start_field++;                                          // Move to the next field.
      }

      // Middle fields
      for (size_t cur_field = start_field; cur_field < stop_field; cur_field++) {
        _data.bits[cur_field] = fun(_data.bits[cur_field]);
      }

      // Set portions of stop field
      if (stop_pos != 0) {
        const field_t mask = MaskField(stop_pos);           // Target end bits with a mask.
        field_t & target = _data.bits[stop_field];           // Isolate the field to change.
        target = (target & ~mask) | (fun(target) & mask);   // Update targeted bits!
      }
    }

    return *this;
  }

  template <typename DATA_T, bool ZERO_LEFT>
  constexpr void Bits<DATA_T,ZERO_LEFT>::ShiftLeft(const size_t shift_size) {
    // If we are shifting out of range, clear the bits and stop.
    if (shift_size >= GetSize()) { Clear(); return; }

    // If we have only a single field, this operation can be quick.
    if (_data.NumFields() == 1) {
      (_data.bits[0] <<= shift_size) &= _data.EndMask();
      return;
    }

    const size_t field_shift = shift_size / FIELD_BITS;
    const size_t bit_shift = shift_size % FIELD_BITS;
    const size_t bit_overflow = FIELD_BITS - bit_shift;

    // Loop through each field, from L to R, and update it.
    if (field_shift) {
      for (size_t i = _data.LastField(); i >= field_shift; --i) {
        _data.bits[i] = _data.bits[i - field_shift];
      }
      for (size_t i = field_shift; i > 0; --i) _data.bits[i-1] = 0;
    }

    // account for bit_shift
    if (bit_shift) {
      for (size_t i = _data.LastField() ; i > field_shift; --i) {
        _data.bits[i] <<= bit_shift;
        _data.bits[i] |= (_data.bits[i-1] >> bit_overflow);
      }
      // Handle final field (field_shift position)
      _data.bits[field_shift] <<= bit_shift;
    }

    // Mask out any bits that have left-shifted away
    ClearExcessBits();
  }

  template <typename DATA_T, bool ZERO_LEFT>
  constexpr void Bits<DATA_T,ZERO_LEFT>::ShiftRight(const size_t shift_size, bool raw) {
    if (shift_size == 0) return;

    // If we are shifting out of range, clear the bits and stop.
    if (!raw && shift_size >= GetSize()) { Clear(); return; }

    // If we have only a single field, this operation can be quick.
    if (_data.NumFields() == 1) {
      _data.bits[0] >>= shift_size;
      return;
    }

    const size_t field_shift = shift_size / FIELD_BITS;
    const size_t bit_shift = shift_size % FIELD_BITS;
    const size_t bit_overflow = FIELD_BITS - bit_shift;
    const size_t NUM_FIELDS = _data.NumFields();
    const size_t field_shift2 = NUM_FIELDS - field_shift;

    // account for field_shift
    if (field_shift) {
      for (size_t i = 0; i < field_shift2; ++i) {
        _data.bits[i] = _data.bits[i + field_shift];
      }
      // Clear fields where bits were fully shifted out.
      for (size_t i = field_shift2; i < NUM_FIELDS; i++) _data.bits[i] = FIELD_0;
    }

    // account for bit_shift
    if (bit_shift) {
      for (size_t i = 0; i < (field_shift2 - 1); ++i) {
        _data.bits[i] >>= bit_shift;
        _data.bits[i] |= (_data.bits[i+1] << bit_overflow);
      }
      _data.bits[field_shift2 - 1] >>= bit_shift;
    }
  }

  /// Helper: call ROTATE with negative number
  template <typename DATA_T, bool ZERO_LEFT>
  constexpr void Bits<DATA_T,ZERO_LEFT>::ROTL_SELF(const size_t shift_size_raw) {
    if (GetSize() == 0) return;   // Nothing to rotate if there are not bits.
    const field_t shift_size = shift_size_raw % GetSize();

    // Use different approaches based on number of bits.
    if (_data.NumFields() == 1) {
      _data.bits[0] = emp::RotateBitsLeft(_data.bits[0], shift_size, GetSize());
    } else {  // For few bits, shifting L/R and OR-ing is faster.
      this_t dup(*this);
      dup.ShiftLeft(shift_size);
      ShiftRight(GetSize() - shift_size);
      OR_SELF(dup);
    }
  }


  /// Helper for calling ROTATE with positive number
  template <typename DATA_T, bool ZERO_LEFT>
  constexpr void Bits<DATA_T,ZERO_LEFT>::ROTR_SELF(const size_t shift_size_raw) {
    const size_t shift_size = shift_size_raw % GetSize();

    // use different approaches based on number of bits
    if (_data.NumFields() == 1) {
      _data.bits[0] = emp::RotateBitsRight(_data.bits[0], shift_size, GetSize());
    } else {
      this_t dup(*this);
      dup.ShiftRight(shift_size);
      ShiftLeft(GetSize() - shift_size);
      OR_SELF(dup);
    }
  }


  ///////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////
  // ----------------------------------------------------------------------------------------
  // --------------------- Implementations of Public Member Functions -----------------------
  // ----------------------------------------------------------------------------------------
  ///////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////


  // ------------------- Implementations of Constructors and Assignments --------------------

  /// Build a new Bits object with specified bit count and initialization (default 0)
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT>::Bits(size_t _num_bits, bool init_val) : _data(_num_bits) {
    if (init_val) SetAll(); else Clear();
  }

  /// Constructor from other type of Bits field.
  template <typename DATA_T, bool ZERO_LEFT>
  template <typename DATA2_T, bool ZERO_LEFT2>
  Bits<DATA_T,ZERO_LEFT>::Bits(const Bits<DATA2_T,ZERO_LEFT2> & in)
    : _data(in.GetSize())
  {
    emp_assert(in.OK());
    RawCopy(in);
  }

  // -- Move constructor in class; set to default --

  /// Constructor to generate a Bits from a std::bitset.
  template <typename DATA_T, bool ZERO_LEFT>
  template <size_t NUM_BITS>
  Bits<DATA_T,ZERO_LEFT>::Bits(const std::bitset<NUM_BITS> & bitset)
    : _data(NUM_BITS)
  {
    // Copy over the values.
    Clear();
    for (size_t i = 0; i < NUM_BITS; ++i) Set(i, bitset[i]);
  }

  /// Constructor to generate a Bits from a string of '0's and '1's.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT>::Bits(const std::string & bitstring)
    : _data(CountBits(bitstring))
  {
    Clear();

    size_t pos = 0;
    for (char c : bitstring) {
      if (c == '1') {
        if constexpr (ZERO_LEFT) Set(pos);
        else Set(GetSize() - pos - 1);
        pos++;
      }
      if (c == '0') ++pos; // Leave position as zero and move to next pos.
    }
  }

  /// Constructor to generate a random set of bits in the default size.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT>::Bits(Random & random)
  {
    emp_assert(GetSize() > 0, "Trying to construct a random series of bits, but with no bits!");
    Randomize(random);
    ClearExcessBits();
  }

  /// Constructor to generate random Bits with provided prob of 1's, default size.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT>::Bits(Random & random, const double p1)
  {
    emp_assert(GetSize() > 0, "Trying to construct a random series of bits, but with no bits!");
    emp_assert(p1 >= 0.0 && p1 <= 1.0, "Probability of ones out of range", p1);
    Randomize(random, p1);
    ClearExcessBits();
  }

  /// Constructor to generate random Bits with specified number of ones.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT>::Bits(Random & random, const size_t target_ones)
  {
    emp_assert(GetSize() > 0, "Trying to construct a random series of bits, but with no bits!");
    ChooseRandom(random, target_ones);
    ClearExcessBits();
  }

  /// Constructor to generate a random Bits (with equal prob of 0 or 1).
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT>::Bits(size_t in_num_bits, Random & random)
    : _data(in_num_bits)
  {
    Clear();
    Randomize(random);
  }

  /// Constructor to generate a random Bits with provided prob of 1's.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT>::Bits(size_t in_num_bits, Random & random, const double p1)
    : _data(in_num_bits)
  {
    emp_assert(p1 >= 0.0 && p1 <= 1.0, "Probability of ones out of range", p1);
    Clear();
    Randomize(random, p1);
  }

  /// Constructor to generate a random Bits with provided number of 1's.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT>::Bits(size_t in_num_bits, Random & random, const size_t target_ones)
    : _data(in_num_bits)
  {
    Clear();
    ChooseRandom(random, target_ones);
  }

  /// Initializer list constructor.
  template <typename DATA_T, bool ZERO_LEFT>
  template <typename T>
  Bits<DATA_T,ZERO_LEFT>::Bits(const std::initializer_list<T> l)
    : _data(l.size())
  {
    Clear();
    size_t idx = 0;
    if constexpr (ZERO_LEFT) {
      for (auto i = std::begin(l); i != std::end(l); ++i) Set(idx++, *i);
    } else {
      for (auto i = std::rbegin(l); i != std::rend(l); ++i) Set(idx++, *i);
    }
  }

  /// Copy, but with a resize.
  template <typename DATA_T, bool ZERO_LEFT>
  template <typename DATA2_T, bool ZERO_LEFT2>
  Bits<DATA_T,ZERO_LEFT>::
  Bits(const Bits<DATA2_T,ZERO_LEFT2> & in, size_t new_size)
    : Bits(new_size)
  {
    emp_assert(in.OK());

    // How many fields do we need to copy?
    size_t copy_fields = std::min(_data.NumFields(), in.NumFields());

    RawCopy(in.FieldPtr(), copy_fields);
  }

  /// Copy assignment operator.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> &
  Bits<DATA_T,ZERO_LEFT>::operator=(const Bits<DATA_T,ZERO_LEFT> & in) &
  {
    emp_assert(in.OK());
    if (&in != this) {
      _data.RawResize(in.GetSize());
      RawCopy(in);
    }

    return *this;
  }

  /// Other Bits assignment operator.
  template <typename DATA_T, bool ZERO_LEFT>
  template <typename DATA2_T, bool ZERO_LEFT2>
  Bits<DATA_T,ZERO_LEFT> &
  Bits<DATA_T,ZERO_LEFT>::operator=(const Bits<DATA2_T,ZERO_LEFT2> & in) &
  {
    emp_assert(in.OK());
    Resize(in.GetSize());
    RawCopy(in);

    return *this;
  }

  /// Move operator.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> &
  Bits<DATA_T,ZERO_LEFT>::operator=(Bits<DATA_T,ZERO_LEFT> && in) &
  {
    emp_assert(&in != this);        // Shouldn't be possible in an r-value
    _data = std::move(in._data);    // Shift move into _data objects.
    return *this;
  }

  /// Assignment operator from a std::bitset.
  template <typename DATA_T, bool ZERO_LEFT>
  template <size_t NUM_BITS>
  Bits<DATA_T,ZERO_LEFT> &
  Bits<DATA_T,ZERO_LEFT>::operator=(const std::bitset<NUM_BITS> & bitset) &
  {
    _data.RawResize(NUM_BITS);
    for (size_t i = 0; i < NUM_BITS; i++) Set(i, bitset[i]);  // Copy bits in.
    return ClearExcessBits();                                 // Set excess bits to zeros.
  }

  /// Assignment operator from a string of '0's and '1's.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> &
  Bits<DATA_T,ZERO_LEFT>::operator=(const std::string & bitstring) &
  {
    const size_t new_size = CountBits(bitstring);
    _data.RawResize(new_size);

    Clear();

    size_t pos = 0;
    for (char c : bitstring) {
      if (c == '1') {
        if constexpr (ZERO_LEFT) Set(pos);
        else Set(new_size - pos - 1);
        pos++;
      }
      if (c == '0') ++pos; // Leave position as zero and move to next pos.
    }

    return *this;
  }


  /// Assign from a Bits object of a different size, while keeping current size.
  /// If there are too many bits being imported, extras are cut off.
  /// If there are fewer bits, the remainder are zero'd out (up to max_copy_bits)
  // @CAO: Can copy fields for a speedup.
  template <typename DATA_T, bool ZERO_LEFT>
  template <typename DATA2_T, bool ZERO_LEFT2>
  Bits<DATA_T,ZERO_LEFT> &
  Bits<DATA_T,ZERO_LEFT>::Import(
    const Bits<DATA2_T,ZERO_LEFT2> & from_bits,
    const size_t from_start_pos,
    size_t max_copy_bits)
  {
    emp_assert(from_start_pos < from_bits.GetSize());
    size_t bits_available = from_bits.GetSize() - from_start_pos;

    // Actual copied bits is limited by bits available to copy and bits in this object.
    size_t copy_size = emp::Min(bits_available, GetSize(), max_copy_bits);

    for (size_t i = 0; i < copy_size; ++i) {
      Set(i, from_bits[i+from_start_pos]);
    }

    // Any bits AFTER the ones copied, but before the max copy, should be zeroed out.
    Clear(copy_size, max_copy_bits);

    return *this;
  }

  /// Convert to a Bitset of a different size.
  template <typename DATA_T, bool ZERO_LEFT>
  template <typename OUT_T>
  OUT_T Bits<DATA_T,ZERO_LEFT>::Export(size_t out_size, size_t start_bit) const {
    OUT_T out_bits(out_size);
    out_bits.Import(*this, start_bit);
    return out_bits;
  }

  /// Concatenate another Bits object on to the end of this one.
  template <typename DATA_T, bool ZERO_LEFT>
  template <typename DATA2_T, bool ZERO_LEFT2>
  Bits<DATA_T,ZERO_LEFT> & Bits<DATA_T,ZERO_LEFT>::Append(
    const Bits<DATA2_T, ZERO_LEFT2> & in_bits
  ) {
    this_t shift_copy(in_bits);
    const size_t old_size = GetSize();
    const size_t new_size = old_size + in_bits.GetSize();
    Resize(new_size);
    shift_copy.Resize(new_size);
    shift_copy <<= old_size;
    OR_SELF(shift_copy);
    return *this;
  }


  // --------------------  Implementations of common accessors -------------------

  /// Retrieve the bit value from the specified index.
  template <typename DATA_T, bool ZERO_LEFT>
  constexpr bool Bits<DATA_T,ZERO_LEFT>::Get(size_t index) const {
    emp_assert(index < GetSize(), index, GetSize());
    const size_t field_id = FieldID(index);
    const size_t pos_id = FieldPos(index);
    return _data.bits[field_id] & (FIELD_1 << pos_id);
  }

  /// Update the bit value at the specified index.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> & Bits<DATA_T,ZERO_LEFT>::Set(size_t index, bool value) {
    emp_assert(index < GetSize(), index, GetSize());
    const size_t field_id = FieldID(index);
    const size_t pos_id = FieldPos(index);
    const field_t pos_mask = FIELD_1 << pos_id;

    if (value) _data.bits[field_id] |= pos_mask;
    else       _data.bits[field_id] &= ~pos_mask;

    return *this;
  }

  /// Set all bits to 1.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> & Bits<DATA_T,ZERO_LEFT>::SetAll() {
    const size_t NUM_FIELDS = _data.NumFields();
    for (size_t i = 0; i < NUM_FIELDS; i++) _data.bits[i] = FIELD_ALL;
    return ClearExcessBits();
  }

  /// Set all bits to 0.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> & Bits<DATA_T,ZERO_LEFT>::Clear() {
    const size_t NUM_FIELDS = _data.NumFields();
    for (size_t i = 0; i < NUM_FIELDS; i++) _data.bits[i] = FIELD_0;
    return *this;
  }

  /// Change a specified bit to the opposite value
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> & Bits<DATA_T,ZERO_LEFT>::Toggle(size_t index) {
    emp_assert(index < GetSize(), index, GetSize());
    const size_t field_id = FieldID(index);
    const size_t pos_id = FieldPos(index);
    const field_t pos_mask = FIELD_1 << pos_id;

    _data.bits[field_id] ^= pos_mask;

    return *this;
  }


  // ------  @CAO CONTINUE HERE!!! ------


  template <typename DATA_T, bool ZERO_LEFT>
  bool Bits<DATA_T,ZERO_LEFT>::Any() const {
    const size_t NUM_FIELDS = _data.NumFields();
    for (size_t i = 0; i < NUM_FIELDS; i++) {
      if (_data.bits[i]) return true;
    }
    return false;
  }

  // -------------------------  Implementations Randomization functions -------------------------

  /// Set all bits randomly, with a 50% probability of being a 0 or 1.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> & Bits<DATA_T,ZERO_LEFT>::Randomize(Random & random) {
    random.RandFill(BytePtr(), _data.NumBytes());
    return ClearExcessBits();
  }

  /// Set all bits randomly, with probability specified at compile time.
  template <typename DATA_T, bool ZERO_LEFT>
  template <Random::Prob P>
  Bits<DATA_T,ZERO_LEFT> & Bits<DATA_T,ZERO_LEFT>::RandomizeP(Random & random,
                                    const size_t start_pos, size_t stop_pos) {
    if (stop_pos == MAX_SIZE_T) stop_pos = GetSize();

    emp_assert(start_pos <= stop_pos);
    emp_assert(stop_pos <= GetSize());
    random.RandFillP<P>(BytePtr(), _data.NumBytes(), start_pos, stop_pos);
    return *this;
  }


  /// Set all bits randomly, with a given probability of being on.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> &
  Bits<DATA_T,ZERO_LEFT>::Randomize(Random & random, const double p,
                                   const size_t start_pos, size_t stop_pos) {
    if (stop_pos == MAX_SIZE_T) stop_pos = GetSize();

    emp_assert(start_pos <= stop_pos, start_pos, stop_pos);
    emp_assert(stop_pos <= GetSize(), stop_pos, GetSize());
    emp_assert(p >= 0.0 && p <= 1.0, p);
    random.RandFill(BytePtr(), _data.NumBytes(), p, start_pos, stop_pos);
    return *this;
  }

  /// Set all bits randomly, with a given number of them being on.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> &
  Bits<DATA_T,ZERO_LEFT>::ChooseRandom(Random & random, const size_t target_ones,
                                      const size_t start_pos, size_t stop_pos) {
    if (stop_pos == MAX_SIZE_T) stop_pos = GetSize();

    emp_assert(start_pos <= stop_pos);
    emp_assert(stop_pos <= GetSize());

    const size_t target_size = stop_pos - start_pos;
    emp_assert(target_ones <= target_size);

    // Approximate the probability of ones as a starting point.
    double p = ((double) target_ones) / (double) target_size;

    // If we are not randomizing the whole sequence, we need to track the number of ones
    // in the NON-randomized region to subtract off later.
    size_t kept_ones = 0;
    if (target_size != GetSize()) {
      Clear(start_pos, stop_pos);
      kept_ones = CountOnes();
    }

    // Try to find a shortcut if p allows....
    // (These values are currently educated guesses)
    if (p < 0.12) { if (target_size == GetSize()) Clear(start_pos, stop_pos); }
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
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> &
  Bits<DATA_T,ZERO_LEFT>::FlipRandom(Random & random,
                                                  const double p,
                                                  const size_t start_pos,
                                                  size_t stop_pos)
  {
    if (stop_pos == MAX_SIZE_T) stop_pos = GetSize();

    emp_assert(start_pos <= stop_pos);
    emp_assert(stop_pos <= GetSize());
    emp_assert(p >= 0.0 && p <= 1.0, p);

    for (size_t i=start_pos; i < stop_pos; ++i) if (random.P(p)) Toggle(i);

    return *this;
  }

  /// Set random bits with a given probability (does not check if already set.)
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> & Bits<DATA_T,ZERO_LEFT>::SetRandom(Random & random,
                      const double p,
                      const size_t start_pos,
                      size_t stop_pos)
  {
    if (stop_pos == MAX_SIZE_T) stop_pos = GetSize();

    emp_assert(start_pos <= stop_pos);
    emp_assert(stop_pos <= GetSize());
    emp_assert(p >= 0.0 && p <= 1.0, p);

    for (size_t i=start_pos; i < stop_pos; ++i) if (random.P(p)) Set(i);

    return *this;
  }

  /// Unset random bits with a given probability (does not check if already zero.)
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> & Bits<DATA_T,ZERO_LEFT>::ClearRandom(Random & random,
                      const double p,
                      const size_t start_pos,
                      size_t stop_pos)
  {
    if (stop_pos == MAX_SIZE_T) stop_pos = GetSize();

    emp_assert(start_pos <= stop_pos);
    emp_assert(stop_pos <= GetSize());
    emp_assert(p >= 0.0 && p <= 1.0, p);

    for (size_t i=start_pos; i < stop_pos; ++i) if (random.P(p)) Clear(i);

    return *this;
  }

  /// Flip a specified number of random bits.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> & Bits<DATA_T,ZERO_LEFT>::FlipRandomCount(
    Random & random,
    const size_t target_bits
  ) {
    emp_assert(GetSize() <= GetSize());
    Bits<DATA_T,ZERO_LEFT> choice(GetSize(), random, target_bits);
    return XOR_SELF(choice);
  }

  /// Set a specified number of random bits (does not check if already set.)
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> & Bits<DATA_T,ZERO_LEFT>::SetRandomCount(
    Random & random,
    const size_t target_bits
  ) {
    emp_assert(GetSize() <= GetSize());
    Bits<DATA_T,ZERO_LEFT> choice(GetSize(), random, target_bits);
    return OR_SELF(choice);
  }

  /// Unset  a specified number of random bits (does not check if already zero.)
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> & Bits<DATA_T,ZERO_LEFT>::ClearRandomCount(
    Random & random,
    const size_t target_bits
  ) {
    emp_assert(GetSize() <= GetSize());
    Bits<DATA_T,ZERO_LEFT> choice(GetSize(), random, GetSize() - target_bits);
    return AND_SELF(choice);
  }


  // -------------------------  Implementations of Comparison Operators -------------------------

  /// Test if two bit vectors are identical.
  template <typename DATA_T, bool ZERO_LEFT>
  template <typename DATA2_T, bool ZL2>
  bool Bits<DATA_T,ZERO_LEFT>::operator==(const Bits<DATA2_T,ZL2> & in) const {
    if (GetSize() != in.GetSize()) return false;

    const size_t NUM_FIELDS = _data.NumFields();
    auto in_fields = in.FieldSpan();
    for (size_t i = 0; i < NUM_FIELDS; ++i) {
      if (_data.bits[i] != in_fields[i]) return false;
    }
    return true;
  }

  /// Compare the would-be numerical values of two bit vectors.
  template <typename DATA_T, bool ZERO_LEFT>
  template <typename DATA2_T, bool ZL2>
  bool Bits<DATA_T,ZERO_LEFT>::operator<(const Bits<DATA2_T,ZL2> & in) const {
    if (GetSize() != in.GetSize()) return GetSize() < in.GetSize();

    const size_t NUM_FIELDS = _data.NumFields();
    auto in_fields = in.FieldSpan();
    for (size_t i = NUM_FIELDS; i > 0; --i) {   // Start loop at the largest field.
      const size_t pos = i-1;
      if (_data.bits[pos] == in_fields[pos]) continue;  // If same, keep looking!
      return (_data.bits[pos] < in_fields[pos]);        // Otherwise, do comparison
    }
    return false; // Bit vectors are identical.
  }

  /// Automatically convert Bits object to other vector types.
  template <typename DATA_T, bool ZERO_LEFT>
  template <typename T>
  Bits<DATA_T,ZERO_LEFT>::operator emp::vector<T>() {
    emp::vector<T> out(GetSize());
    for (size_t i = 0; i < GetSize(); i++) {
      out[i] = (T) Get(i);
    }
    return out;
  }


  // -------------------------  Access Groups of bits -------------------------

  /// Retrieve the byte at the specified byte index.
  template <typename DATA_T, bool ZERO_LEFT>
  uint8_t Bits<DATA_T,ZERO_LEFT>::GetByte(size_t index) const {
    emp_assert(index < _data.NumBytes(), index, _data.NumBytes());
    const size_t field_id = Byte2Field(index);
    const size_t pos_id = Byte2FieldPos(index);
    return (_data.bits[field_id] >> pos_id) & 255U;
  }

  /// Update the byte at the specified byte index.
  template <typename DATA_T, bool ZERO_LEFT>
  void Bits<DATA_T,ZERO_LEFT>::SetByte(size_t index, uint8_t value) {
    emp_assert(index < _data.NumBytes(), index, _data.NumBytes());
    const size_t field_id = Byte2Field(index);
    const size_t pos_id = Byte2FieldPos(index);
    const field_t val_uint = value;
    _data.bits[field_id] = (_data.bits[field_id] & ~(FIELD_255 << pos_id)) | (val_uint << pos_id);
  }

  /// Get the overall value of this BitSet, using a uint encoding, but including all bits
  /// and returning the value as a double.
  template <typename DATA_T, bool ZERO_LEFT>
  double Bits<DATA_T,ZERO_LEFT>::GetValue() const {
    const int max_one = FindMaxOne();

    // If there are no ones, this value must be 0.
    if (max_one == -1) return 0.0;

    // If all ones are in the least-significant field, just return it.
    if (max_one < 64) return (double) GetUInt64(0);

    // To grab the most significant field, figure out how much to shift it by.
    const size_t shift_bits = static_cast<size_t>(max_one) - 63;
    double out_value = (double) (*this >> shift_bits).GetUInt64(0);

    out_value *= emp::Pow2(shift_bits);

    return out_value;
  }

  /// Get specified type at a given index (in steps of that type size)
  template <typename DATA_T, bool ZERO_LEFT>
  template <typename T>
  T Bits<DATA_T,ZERO_LEFT>::GetValueAtIndex(const size_t index) const {
    // For the moment, must fit inside bounds; eventually should pad with zeros.
    emp_assert((index + 1) * sizeof(T) <= _data.TotalBytes());

    T out_value;
    std::memcpy( &out_value, BytePtr().Raw() + index * sizeof(T), sizeof(T) );
    return out_value;
  }


  /// Set specified type at a given index (in steps of that type size)
  template <typename DATA_T, bool ZERO_LEFT>
  template <typename T>
  Bits<DATA_T,ZERO_LEFT> & Bits<DATA_T,ZERO_LEFT>::SetValueAtIndex(const size_t index, T in_value) {
    // For the moment, must fit inside bounds; eventually should pad with zeros.
    emp_assert((index + 1) * sizeof(T) <= _data.TotalBytes());
    std::memcpy( BytePtr().Raw() + index * sizeof(T), &in_value, sizeof(T) );
    return ClearExcessBits();
  }


  /// Get the specified type starting from a given BIT position.
  template <typename DATA_T, bool ZERO_LEFT>
  template <typename T>
  T Bits<DATA_T,ZERO_LEFT>::GetValueAtBit(const size_t index) const {
    // For the moment, must fit inside bounds; eventually should pad with zeros.
    emp_assert((index+7)/8 + sizeof(T) < _data.TotalBytes());

    Bits<DATA_T,ZERO_LEFT> out_bits(*this);
    out_bits >>= index;

    return out_bits.template GetValueAtIndex<T>(0);
  }


  /// Set the specified type starting from a given BIT position.
  // @CAO: Can be optimized substantially, especially for long Bits objects.
  template <typename DATA_T, bool ZERO_LEFT>
  template <typename T>
  Bits<DATA_T,ZERO_LEFT> & Bits<DATA_T,ZERO_LEFT>::SetValueAtBit(const size_t index, T value) {
    // For the moment, must fit inside bounds; eventually should (?) pad with zeros.
    emp_assert((index+7)/8 + sizeof(T) < _data.TotalBytes());
    constexpr size_t type_bits = sizeof(T) * 8;

    const size_t end_pos = Min(index+type_bits, GetSize());
    Clear(index, end_pos);               // Clear out the bits where new value will go.
    Bits<DATA_T,ZERO_LEFT> in_bits(GetSize());        // Setup a bitset for the new bits.
    in_bits.SetValueAtIndex(0, value);   // Insert the new bits.
    in_bits <<= index;                   // Shift new bits into place.
    OR_SELF(in_bits);                    // Place new bits into current Bits object.

    return ClearExcessBits();
  }


  // -------------------------  Other Analyses -------------------------

  /// A simple hash function for bit vectors.
  template <typename DATA_T, bool ZERO_LEFT>
  std::size_t Bits<DATA_T,ZERO_LEFT>::Hash(size_t start_field) const {
    static_assert(std::is_same_v<field_t, size_t>, "Hash() requires fields to be size_t");

    // If there are no fields left, hash on size one.
    if (start_field == _data.NumFields()) return GetSize();

    // If we have only one field left, combine it with size.
    if (start_field == _data.NumFields()-1) return hash_combine(_data.bits[start_field], GetSize());

    // Otherwise we have more than one field.  Combine and recurse.
    size_t partial_hash = hash_combine(_data.bits[start_field], _data.bits[start_field+1]);

    return hash_combine(partial_hash, Hash(start_field+2));
  }

  // TODO: see https://arxiv.org/pdf/1611.07612.pdf for fast pop counts
  /// Count the number of ones in Bits.
  template <typename DATA_T, bool ZERO_LEFT>
  constexpr size_t Bits<DATA_T,ZERO_LEFT>::CountOnes() const {
    if (GetSize() == 0) return 0;
    const field_t NUM_FIELDS = _data.NumFields();
    size_t bit_count = 0;
    for (size_t i = 0; i < NUM_FIELDS; i++) {
        // when compiling with -O3 and -msse4.2, this is the fastest population count method.
        std::bitset<FIELD_BITS> std_bs(_data.bits[i]);
        bit_count += std_bs.count();
      }

    emp_assert(bit_count <= GetSize());
    return bit_count;
  }

  // TODO: Speed this up so that we don't need to copy out all of the bits.
  /// Count the number of ones in a specified range of Bits.
  template <typename DATA_T, bool ZERO_LEFT>
  constexpr size_t Bits<DATA_T,ZERO_LEFT>::CountOnes(size_t start, size_t end) const {
    emp_assert(start <= end);
    emp_assert(end <= GetSize());
    if (start == end) return 0;
    const size_t range_size = end-start;
    return Export(range_size, start).CountOnes();
  }

  /// Faster counting of ones for very sparse bit vectors.
  template <typename DATA_T, bool ZERO_LEFT>
  constexpr size_t Bits<DATA_T,ZERO_LEFT>::CountOnes_Sparse() const {
    size_t bit_count = 0;
    const size_t NUM_FIELDS = _data.NumFields();
    for (size_t i = 0; i < NUM_FIELDS; ++i) {
      field_t cur_field = _data.bits[i];
      while (cur_field) {
        cur_field &= (cur_field-1);       // Peel off a single 1.
        bit_count++;                      // Increment the counter
      }
    }
    return bit_count;
  }

  /// Pop the last bit in the vector.
  /// @return value of the popped bit.
  template <typename DATA_T, bool ZERO_LEFT>
  bool Bits<DATA_T,ZERO_LEFT>::PopBack() {
    const bool val = Get(GetSize()-1);
    Resize(GetSize() - 1);
    return val;
  }

  /// Push given bit(s) onto the back of a vector.
  /// @param bit value of bit to be pushed.
  /// @param num number of bits to be pushed.
  template <typename DATA_T, bool ZERO_LEFT>
  void Bits<DATA_T,ZERO_LEFT>::PushBack(const bool bit, const size_t num) {
    Resize(GetSize() + num);
    if (bit) SetRange(GetSize()-num, GetSize());
  }

  /// Insert bit(s) into any index of vector using bit magic.
  /// Blog post on implementation reasoning: https://devolab.org/?p=2249
  /// @param index location to insert bit(s).
  /// @param val value of bit(s) to insert (default true)
  /// @param num number of bits to insert, default 1.
  template <typename DATA_T, bool ZERO_LEFT>
  void Bits<DATA_T,ZERO_LEFT>::Insert(const size_t index, const bool val, const size_t num) {
    Resize(GetSize() + num);                 // Adjust to new number of bits.
    Bits<DATA_T,ZERO_LEFT> low_bits(*this);              // Copy current bits
    SHIFT_SELF(-(int)num);                  // Shift the high bits into place.
    Clear(0, index+num);                    // Reduce current to just high bits.
    low_bits.Clear(index, GetSize());        // Reduce copy to just low bits.
    if (val) SetRange(index, index+num);    // If new bits should be ones, make it so.
    OR_SELF(low_bits);                      // Put the low bits back in place.
  }


  /// Delete bits from any index in a vector.
  /// @param index location to delete bit(s).
  /// @param num number of bits to delete, default 1.
  template <typename DATA_T, bool ZERO_LEFT>
  void Bits<DATA_T,ZERO_LEFT>::Delete(const size_t index, const size_t num) {
    emp_assert(index+num <= GetSize());    // Make sure bits to delete actually exist!
    RawMove(index+num, GetSize(), index);  // Shift positions AFTER delete into place.
    Resize(GetSize() - num);               // Crop off end bits.
  }

  /// Return the position of the first one; return -1 if no ones in vector.
  template <typename DATA_T, bool ZERO_LEFT>
  int Bits<DATA_T,ZERO_LEFT>::FindOne() const {
    const size_t NUM_FIELDS = _data.NumFields();
    size_t field_id = 0;
    while (field_id < NUM_FIELDS && _data.bits[field_id]==0) field_id++;
    return (field_id < NUM_FIELDS) ?
      (int) (find_bit(_data.bits[field_id]) + (field_id * FIELD_BITS))  :  -1;
  }

  /// Return the position of the first one after start_pos; return -1 if no ones in vector.
  /// You can loop through all 1-bit positions in "bits" with:
  ///
  ///   for (int pos = bits.FindOne(); pos >= 0; pos = bits.FindOne(pos+1)) { ... }

  template <typename DATA_T, bool ZERO_LEFT>
  int Bits<DATA_T,ZERO_LEFT>::FindOne(const size_t start_pos) const {
    if (start_pos >= GetSize()) return -1;            // If we're past the end, return fail.
    size_t field_id  = FieldID(start_pos);           // What field do we start in?
    const size_t field_pos = FieldPos(start_pos);    // What position in that field?

    // If there's a hit in a partial first field, return it.
    if (field_pos && (_data.bits[field_id] & ~(MaskField(field_pos)))) {
      return (int) (find_bit(_data.bits[field_id] & ~(MaskField(field_pos))) +
                    field_id * FIELD_BITS);
    }

    // Search other fields...
    const size_t NUM_FIELDS = _data.NumFields();
    if (field_pos) field_id++;
    while (field_id < NUM_FIELDS && _data.bits[field_id]==0) field_id++;
    return (field_id < NUM_FIELDS) ?
      (int) (find_bit(_data.bits[field_id]) + (field_id * FIELD_BITS)) : -1;
  }

  /// Find the most-significant set-bit.
  template <typename DATA_T, bool ZERO_LEFT>
  int Bits<DATA_T,ZERO_LEFT>::FindMaxOne() const {
    // Find the max field with a one.
    size_t max_field = _data.NumFields() - 1;
    while (max_field > 0 && _data.bits[max_field] == 0) max_field--;

    // If there are no ones, return -1.
    if (_data.bits[max_field] == 0) return -1;

    const field_t field = _data.bits[max_field]; // Save a local copy of this field.
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
  template <typename DATA_T, bool ZERO_LEFT>
  int Bits<DATA_T,ZERO_LEFT>::PopOne() {
    const int out_bit = FindOne();
    if (out_bit >= 0) Clear((size_t) out_bit);
    return out_bit;
  }

  /// Return positions of all ones.
  template <typename DATA_T, bool ZERO_LEFT>
  emp::vector<size_t> Bits<DATA_T,ZERO_LEFT>::GetOnes() const {
    emp::vector<size_t> out_vals;
    GetOnes(out_vals);
    return out_vals;
  }

  /// Return positions of all ones using a specified type.
  template <typename DATA_T, bool ZERO_LEFT>
  template <typename T>
  emp::vector<T> & Bits<DATA_T,ZERO_LEFT>::GetOnes(emp::vector<T> & out_vals) const {
    // @CAO -- There are better ways to do this with bit tricks.
    out_vals.resize(CountOnes());
    T cur_pos = 0;
    for (T i = 0; i < GetSize(); i++) {
      if (Get(i)) out_vals[cur_pos++] = i;
    }
    return out_vals;
  }

  /// Find the length of the longest continuous series of ones.
  template <typename DATA_T, bool ZERO_LEFT>
  size_t Bits<DATA_T,ZERO_LEFT>::LongestSegmentOnes() const {
    size_t length = 0;
    Bits<DATA_T,ZERO_LEFT> test_bits(*this);
    while(test_bits.Any()){
      ++length;
      test_bits.AND_SELF(test_bits<<1);
    }
    return length;
  }

  /// Return true if any ones are in common with another Bits object.
  template <typename DATA_T, bool ZERO_LEFT>
  bool Bits<DATA_T,ZERO_LEFT>::HasOverlap(const Bits<DATA_T,ZERO_LEFT> & in) const {
    const size_t num_fields = std::min(_data.NumFields(), in.NumFields());
    auto in_fields = in.FieldSpan();
    for (size_t i = 0; i < num_fields; ++i) {
      // Short-circuit if we find any overlap.
      if (_data.bits[i] & in_fields[i]) return true;
    }
    return false;
  }


  // -------------------------  Printing and string conversion -------------------------

  /// Convert this Bits object to a vector string [0 index on left]
  template <typename DATA_T, bool ZERO_LEFT>
  std::string Bits<DATA_T,ZERO_LEFT>::ToString() const {
    if constexpr (ZERO_LEFT) return ToArrayString();
    else return ToBinaryString();
  }

  /// Convert this Bits object to a vector string [0 index on left]
  template <typename DATA_T, bool ZERO_LEFT>
  std::string Bits<DATA_T,ZERO_LEFT>::ToArrayString() const {
    std::string out_string;
    out_string.reserve(GetSize());
    for (size_t i = 0; i < GetSize(); ++i) out_string.push_back(GetAsChar(i));
    return out_string;
  }

  /// Convert this Bits object to a numerical string [0 index on right]
  template <typename DATA_T, bool ZERO_LEFT>
  std::string Bits<DATA_T,ZERO_LEFT>::ToBinaryString() const {
    std::string out_string;
    out_string.reserve(GetSize());
    for (size_t i = GetSize(); i > 0; --i) out_string.push_back(GetAsChar(i-1));
    return out_string;
  }

  /// Convert this Bits object to a series of IDs
  template <typename DATA_T, bool ZERO_LEFT>
  std::string Bits<DATA_T,ZERO_LEFT>::ToIDString(const std::string & spacer) const {
    std::stringstream ss;
    PrintOneIDs(ss, spacer);
    return ss.str();
  }

  /// Convert this Bits object to a series of IDs with ranges condensed.
  template <typename DATA_T, bool ZERO_LEFT>
  std::string Bits<DATA_T,ZERO_LEFT>::ToRangeString(const std::string & spacer,
                                       const std::string & ranger) const
  {
    std::stringstream ss;
    PrintAsRange(ss, spacer, ranger);
    return ss.str();
  }

  /// Print a space between each field (or other provided spacer)
  template <typename DATA_T, bool ZERO_LEFT>
  void Bits<DATA_T,ZERO_LEFT>::PrintFields(std::ostream & out, const std::string & spacer) const {
    for (size_t i = GetSize()-1; i < GetSize(); i--) {
      out << Get(i);
      if (i && (i % FIELD_BITS == 0)) out << spacer;
    }
  }

  /// Print a space between each field (or other provided spacer)
  template <typename DATA_T, bool ZERO_LEFT>
  void Bits<DATA_T,ZERO_LEFT>::PrintDebug(
    std::ostream & out,
    const std::string & label
  ) const {
    if (label.size()) out << label << ":\n";
    for (size_t field = 0; field < _data.NumFields(); field++) {
      for (size_t bit_id = 0; bit_id < FIELD_BITS; bit_id++) {
        bool bit = (FIELD_1 << bit_id) & _data.bits[field];
        out << ( bit ? 1 : 0 );
      }
      out << " : " << field << std::endl;
    }
    size_t end_pos = _data.NumEndBits();
    if (end_pos == 0) end_pos = FIELD_BITS;
    for (size_t i = 0; i < end_pos; i++) out << " ";
    out << "^" << std::endl;
  }

  /// Print the positions of all one bits, spaces are the default separator.
  template <typename DATA_T, bool ZERO_LEFT>
  void Bits<DATA_T,ZERO_LEFT>::PrintOneIDs(std::ostream & out, const std::string & spacer) const {
    bool started = false;
    for (size_t i = 0; i < GetSize(); i++) {
      if (Get(i)) {
        if (started) out << spacer;
        out << i;
        started = true;
      }
    }
  }

  /// Print the ones in a range format.  E.g., 2-5,7,10-15
  template <typename DATA_T, bool ZERO_LEFT>
  void Bits<DATA_T,ZERO_LEFT>::PrintAsRange(std::ostream & out,
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

  /// Perform a Boolean NOT with this Bits object, store result here, and return this object.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> & Bits<DATA_T,ZERO_LEFT>::NOT_SELF() {
    const size_t NUM_FIELDS = _data.NumFields();
    for (size_t i = 0; i < NUM_FIELDS; i++) _data.bits[i] = ~_data.bits[i];
    return ClearExcessBits();
  }

  /// Perform a Boolean AND with this Bits object, store result here, and return this object.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> & Bits<DATA_T,ZERO_LEFT>::AND_SELF(const Bits<DATA_T,ZERO_LEFT> & bits2) {
    const size_t NUM_FIELDS = _data.NumFields();
    for (size_t i = 0; i < NUM_FIELDS; i++) _data.bits[i] = _data.bits[i] & bits2._data.bits[i];
    return *this;
  }

  /// Perform a Boolean OR with this Bits object, store result here, and return this object.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> & Bits<DATA_T,ZERO_LEFT>::OR_SELF(const Bits<DATA_T,ZERO_LEFT> & bits2) {
    const size_t NUM_FIELDS = _data.NumFields();
    for (size_t i = 0; i < NUM_FIELDS; i++) _data.bits[i] = _data.bits[i] | bits2._data.bits[i];
    return *this;
  }

  /// Perform a Boolean NAND with this Bits object, store result here, and return this object.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> & Bits<DATA_T,ZERO_LEFT>::NAND_SELF(const Bits<DATA_T,ZERO_LEFT> & bits2) {
    const size_t NUM_FIELDS = _data.NumFields();
    for (size_t i = 0; i < NUM_FIELDS; i++) _data.bits[i] = ~(_data.bits[i] & bits2._data.bits[i]);
    return ClearExcessBits();
  }

  /// Perform a Boolean NOR with this Bits object, store result here, and return this object.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> & Bits<DATA_T,ZERO_LEFT>::NOR_SELF(const Bits<DATA_T,ZERO_LEFT> & bits2) {
    const size_t NUM_FIELDS = _data.NumFields();
    for (size_t i = 0; i < NUM_FIELDS; i++) _data.bits[i] = ~(_data.bits[i] | bits2._data.bits[i]);
    return ClearExcessBits();
  }

  /// Perform a Boolean XOR with this Bits object, store result here, and return this object.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> & Bits<DATA_T,ZERO_LEFT>::XOR_SELF(const Bits<DATA_T,ZERO_LEFT> & bits2) {
    const size_t NUM_FIELDS = _data.NumFields();
    for (size_t i = 0; i < NUM_FIELDS; i++) _data.bits[i] = _data.bits[i] ^ bits2._data.bits[i];
    return *this;
  }

  /// Perform a Boolean EQU with this Bits object, store result here, and return this object.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> & Bits<DATA_T,ZERO_LEFT>::EQU_SELF(const Bits<DATA_T,ZERO_LEFT> & bits2) {
    const size_t NUM_FIELDS = _data.NumFields();
    for (size_t i = 0; i < NUM_FIELDS; i++) _data.bits[i] = ~(_data.bits[i] ^ bits2._data.bits[i]);
    return ClearExcessBits();
  }

  /// Positive shifts go left and negative go right (0 does nothing); return result.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> Bits<DATA_T,ZERO_LEFT>::SHIFT(const int shift_size) const {
    Bits<DATA_T,ZERO_LEFT> out_bits(*this);
    if (shift_size > 0) out_bits.ShiftRight((size_t) shift_size);
    else if (shift_size < 0) out_bits.ShiftLeft((size_t) -shift_size);
    return out_bits;
  }

  /// Positive shifts go left and negative go right; store result here, and return this object.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> & Bits<DATA_T,ZERO_LEFT>::SHIFT_SELF(const int shift_size) {
    if (shift_size > 0) ShiftRight((size_t) shift_size);
    else if (shift_size < 0) ShiftLeft((size_t) -shift_size);
    return *this;
  }

  /// Reverse the order of bits in the bitset
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> & Bits<DATA_T,ZERO_LEFT>::REVERSE_SELF() {
    auto bit_span = _data.AsSpan();

    // Reverse order of whole fields
    std::reverse( bit_span.begin(), bit_span.end() );

    // Reverse the bits in each field.
    for (auto & cur_field : bit_span) cur_field = emp::ReverseBits(cur_field);

    // Move the gap to the other side.
    if (_data.NumEndBits()) ShiftRight(_data.EndGap(), true);

    return *this;
  }

  /// Reverse order of bits in the bitset.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> Bits<DATA_T,ZERO_LEFT>::REVERSE() const {
    Bits<DATA_T,ZERO_LEFT> out_set(*this);
    return out_set.REVERSE_SELF();
  }


  /// Positive rotates go left and negative rotates go left (0 does nothing);
  /// return result.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> Bits<DATA_T,ZERO_LEFT>::ROTATE(const int rotate_size) const {
    Bits<DATA_T,ZERO_LEFT> out_set(*this);
    if (rotate_size > 0) out_set.ROTR_SELF((field_t) rotate_size);
    else if (rotate_size < 0) out_set.ROTL_SELF((field_t) (-rotate_size));
    return out_set;
  }

  /// Positive rotates go right and negative rotates go left (0 does nothing);
  /// store result here, and return this object.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> & Bits<DATA_T,ZERO_LEFT>::ROTATE_SELF(const int rotate_size) {
    if (rotate_size > 0) ROTR_SELF((field_t) rotate_size);
    else if (rotate_size < 0) ROTL_SELF((field_t) -rotate_size);
    return *this;
  }


  /// Addition of two Bitsets.
  /// Wraps if it overflows.
  /// Returns result.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> Bits<DATA_T,ZERO_LEFT>::ADD(const Bits<DATA_T,ZERO_LEFT> & set2) const{
    Bits<DATA_T,ZERO_LEFT> out_set(*this);
    return out_set.ADD_SELF(set2);
  }

  /// Addition of two Bitsets.
  /// Wraps if it overflows.
  /// Returns this object.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> & Bits<DATA_T,ZERO_LEFT>::ADD_SELF(const Bits<DATA_T,ZERO_LEFT> & set2) {
    bool carry = false;

    for (size_t i = 0; i < GetSize()/FIELD_BITS; ++i) {
      field_t addend = set2._data.bits[i] + static_cast<field_t>(carry);
      carry = set2._data.bits[i] > addend;

      field_t sum = _data.bits[i] + addend;
      carry |= _data.bits[i] > sum;

      _data.bits[i] = sum;
    }

    if (_data.NumEndBits()) {
      _data.bits[GetSize()/FIELD_BITS] = (
        _data.bits[GetSize()/FIELD_BITS]
        + set2._data.bits[GetSize()/FIELD_BITS]
        + static_cast<field_t>(carry)
      ) & _data.EndMask();
    }

    return *this;
  }

  /// Subtraction of two Bitsets.
  /// Wraps around if it underflows.
  /// Returns result.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> Bits<DATA_T,ZERO_LEFT>::SUB(const Bits<DATA_T,ZERO_LEFT> & set2) const{
    Bits<DATA_T,ZERO_LEFT> out_set(*this);
    return out_set.SUB_SELF(set2);
  }

  /// Subtraction of two Bitsets.
  /// Wraps if it underflows.
  /// Returns this object.
  template <typename DATA_T, bool ZERO_LEFT>
  Bits<DATA_T,ZERO_LEFT> & Bits<DATA_T,ZERO_LEFT>::SUB_SELF(const Bits<DATA_T,ZERO_LEFT> & set2){

    bool carry = false;

    for (size_t i = 0; i < GetSize()/FIELD_BITS; ++i) {
      field_t subtrahend = set2._data.bits[i] + static_cast<field_t>(carry);
      carry = set2._data.bits[i] > subtrahend;
      carry |= _data.bits[i] < subtrahend;
      _data.bits[i] -= subtrahend;
    }

    if (_data.NumEndBits()) {
      _data.bits[GetSize()/FIELD_BITS] = (
        _data.bits[GetSize()/FIELD_BITS]
        - set2._data.bits[GetSize()/FIELD_BITS]
        - static_cast<field_t>(carry)
      ) & _data.EndMask();
    }

    return *this;
  }

  // Set up some aliases from common types of Bit strings.
  // BitVector and BitArray function like vectors and arrays, which is to say that the zero
  // index is on the left-hand side.  BitSet and BitValue are treated like numerical
  // representations, with the zero-position on the right-hand side.

  // using BitVector = Bits<BitsMode::DYNAMIC, 0, true>;
  using BitVector = Bits<Bits_WatermarkData, true>;
  using BitValue  = Bits<Bits_DynamicData, false>;

  template <size_t NUM_BITS> using BitArray        = Bits<Bits_FixedData<NUM_BITS>, true>;
  template <size_t NUM_BITS> using BitSet          = Bits<Bits_FixedData<NUM_BITS>, false>;
  template <size_t MAX_BITS> using StaticBitVector = Bits<Bits_StaticData<MAX_BITS>, true>;
  template <size_t MAX_BITS> using StaticBitValue  = Bits<Bits_StaticData<MAX_BITS>, false>;
}


// ---------------------- Implementations to work with standard library ----------------------

namespace std {
  /// Hash function to allow BitVector to be used with maps and sets (must be in std).
  template <>
  struct hash<emp::BitVector> {
    std::size_t operator()(const emp::BitVector & bits) const {
      return bits.Hash();
    }
  };
}

#endif // #ifndef EMP_BITS_BITS_HPP_INCLUDE
