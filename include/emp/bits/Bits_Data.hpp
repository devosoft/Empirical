/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022-23.
 *
 *  @file Bits_Data.hpp
 *  @brief Helper class to handle memory management for Bits objects.
 *  @note Status: BETA
 *
 *  Bits_Data handles the actual bits inside of the Bits class.  Bits itself provides many tools
 *  to operate on that data.
 */

#ifndef EMP_BITS_BITS_DATA_HPP_INCLUDE
#define EMP_BITS_BITS_DATA_HPP_INCLUDE


#include <span>
#include <string>

#include "../base/array.hpp"
#include "../base/assert.hpp"
#include "../base/Ptr.hpp"
#include "../math/math.hpp"

#include "bitset_utils.hpp"

namespace emp {

  // BitsMode specifies how a Bits object can change the number of bits in itself.
  //  FIXED is locked at the base size an cannot change and is stored in static memory.
  //  CAPPED must be the base size or lower, but requires size tracking.
  //  DYNAMIC defaults to base size, but can be changed; requires indirect memory and allocation.
  //  WATERMARK is like DYNAMIC, but never reallocates memory when shrinking active size.
  enum class BitsMode { FIXED, CAPPED, DYNAMIC, WATERMARK };

  namespace internal {

    // ------------------------------------------------------------------------------------
    //  SIZE TRACKING
    // ------------------------------------------------------------------------------------

    /// Dynamic size is stored here to work with, but not the actual bits.
    template <size_t DEFAULT_SIZE=0>
    struct Bits_Data_Size_Var {
      using field_t = bits_field_t;

      size_t num_bits;           ///< Total number of bits are we using

      constexpr void SetSize(size_t new_size) { num_bits = new_size; }

      [[nodiscard]] constexpr size_t NumBits() const noexcept { return num_bits; }

      // Number of bits locked in at compile time.
      [[nodiscard]] static constexpr size_t NumCTBits() noexcept { return 0; }

      /// Number of bits used in partial field at the end; 0 = perfect fit.
      [[nodiscard]] constexpr size_t NumEndBits() const noexcept {
        return num_bits & (NUM_FIELD_BITS - 1);
      }

      /// EXTRA bits leftover in the gap at the end
      [[nodiscard]] constexpr size_t EndGap() const noexcept {
        return NumEndBits() ? (NUM_FIELD_BITS - NumEndBits()) : 0;
      }

      /// Mask to cut off all of the final bits.
      [[nodiscard]] constexpr field_t EndMask() const noexcept {
        return MaskLow<field_t>(NumEndBits());
      }

      /// How many fields do we need for the current set of bits?
      [[nodiscard]] constexpr size_t NumFields() const noexcept {
        return num_bits ? (1 + ((num_bits - 1) / NUM_FIELD_BITS)) : 0;
      }

      /// ID of the last occupied field
      [[nodiscard]] constexpr size_t LastField() const noexcept {
        return NumFields() - 1;
      }

      /// Number of bytes needed for the current set of bits
      [[nodiscard]] constexpr size_t NumBytes() const noexcept {
        return num_bits ? (1 + ((num_bits - 1) >> 3)) : 0;
      }

      /// How many bytes are allocated?
      [[nodiscard]] constexpr size_t TotalBytes() const noexcept {
        return NumFields() * sizeof(field_t);
      }

      Bits_Data_Size_Var(size_t in_size=DEFAULT_SIZE) : num_bits(in_size) { }
      Bits_Data_Size_Var(const Bits_Data_Size_Var &) = default;

      template <class Archive>
      void serialize(Archive & ar) { ar(num_bits); }

      [[nodiscard]] constexpr bool OK() const { return true; } // Nothing to check yet.
    };

    /// If we have a fixed number of bits, we know size at compile time.
    template <size_t NUM_BITS>
    struct Bits_Data_Size_Fixed {
      using field_t = bits_field_t;
      static constexpr size_t DEFAULT_SIZE = NUM_BITS;

      constexpr void SetSize(size_t new_size) {
        emp_assert(new_size == NUM_BITS, "Cannot change to new_size");
      }

      [[nodiscard]] constexpr size_t NumBits() const noexcept { return NUM_BITS; }

      // Number of bits locked in at compile time.
      [[nodiscard]] static constexpr size_t NumCTBits() noexcept { return NUM_BITS; }

      /// Number of bits used in partial field at the end; 0 if perfect fit.
      [[nodiscard]] constexpr size_t NumEndBits() const noexcept {
        return NUM_BITS & (NUM_FIELD_BITS - 1);
      }

      /// How many EXTRA bits are leftover in the gap at the end?
      [[nodiscard]] constexpr size_t EndGap() const noexcept {
        return (NUM_FIELD_BITS - NumEndBits()) % NUM_FIELD_BITS;
      }

      /// A mask to cut off all of the final bits.
      [[nodiscard]] constexpr field_t EndMask() const noexcept {
        return MaskLow<field_t>(NumEndBits());
      }

      /// How many felids do we need for the current set of bits?
      [[nodiscard]] constexpr size_t NumFields() const noexcept {
        return NUM_BITS ? (1 + ((NUM_BITS - 1) / NUM_FIELD_BITS)) : 0;
      }

      /// What is the ID of the last occupied field?
      [[nodiscard]] constexpr size_t LastField() const noexcept { return NumFields() - 1; }

      /// How many bytes are used for the current set of bits? (rounded up!)
      [[nodiscard]] constexpr size_t NumBytes() const noexcept {
        return NUM_BITS ? (1 + ((NUM_BITS - 1) >> 3)) : 0;
      }

      /// How many bytes are allocated? (rounded up!)
      [[nodiscard]] constexpr size_t TotalBytes() const noexcept {
        return NumFields() * sizeof(field_t);
      }

      Bits_Data_Size_Fixed([[maybe_unused]] size_t in_size=NUM_BITS) {
        emp_assert(in_size <= NUM_BITS, in_size, NUM_BITS);
      }
      Bits_Data_Size_Fixed(const Bits_Data_Size_Fixed &) = default;

      template <class Archive>
      void serialize(Archive & /* ar */) { /* Nothing to do here. */ }

      [[nodiscard]] constexpr bool OK() const { return true; } // Nothing to check yet.
    };


    // ------------------------------------------------------------------------------------
    //  RAW MEMORY MANAGEMENT
    // ------------------------------------------------------------------------------------

    /// Data & functions for Bits types with fixed memory (size may be dynamic, capped by CAPACITY)
    template <typename BASE_T, size_t CAPACITY>
    struct Bits_Data_Mem_Static_Base : public BASE_T {
      using base_size_t = BASE_T;
      using field_t = bits_field_t;
      static constexpr size_t MAX_FIELDS = (1 + ((CAPACITY - 1) / NUM_FIELD_BITS));

      emp::array<field_t, MAX_FIELDS> bits;  ///< Fields to hold the actual bit values.

      Bits_Data_Mem_Static_Base() = default;
      Bits_Data_Mem_Static_Base(size_t num_bits) : BASE_T(num_bits)
      {
        emp_assert(num_bits <= CAPACITY, num_bits, CAPACITY);
      }
      Bits_Data_Mem_Static_Base(const Bits_Data_Mem_Static_Base &) = default;
      Bits_Data_Mem_Static_Base(Bits_Data_Mem_Static_Base &&) = default;

      Bits_Data_Mem_Static_Base & operator=(const Bits_Data_Mem_Static_Base &) = default;
      Bits_Data_Mem_Static_Base & operator=(Bits_Data_Mem_Static_Base &&) = default;

      // --- Helper functions --
      
      [[nodiscard]] Ptr<field_t> FieldPtr() { return bits.data(); }
      [[nodiscard]] Ptr<const field_t> FieldPtr() const { return bits.data(); }

      void RawResize(const size_t new_size, const bool preserve_data=false) {
        const size_t old_num_fields = BASE_T::NumFields();
        BASE_T::SetSize(new_size);
        if (preserve_data && BASE_T::NumEndBits()) {
          bits[BASE_T::LastField()] &= BASE_T::EndMask();          
          for (size_t id = BASE_T::NumFields(); id < old_num_fields; ++id) bits[id] = 0;
        }
      }

      [[nodiscard]] auto AsSpan() { return std::span<field_t,MAX_FIELDS>(bits.data(), MAX_FIELDS); }
      [[nodiscard]] auto AsSpan() const { return std::span<const field_t,MAX_FIELDS>(bits.data(), MAX_FIELDS); }

      [[nodiscard]] bool OK() const { return true; } // Nothing to check yet.

      template <class Archive>
      void serialize(Archive & ar) {
        BASE_T::serialize(ar); // Save size info.
        for (size_t i=0; i < BASE_T::NumFields(); ++i) { ar(bits[i]); }
      }

    };
    
    template <size_t CAPACITY, size_t DEFAULT_SIZE=0>
    using Bits_Data_Mem_Static =
      Bits_Data_Mem_Static_Base< Bits_Data_Size_Var<DEFAULT_SIZE>, CAPACITY >;

    template <size_t CAPACITY, size_t DEFAULT_SIZE=CAPACITY>
    using Bits_Data_Mem_Fixed =
      Bits_Data_Mem_Static_Base< Bits_Data_Size_Fixed<DEFAULT_SIZE>, CAPACITY >;

    /// Data & functions for Bits types with dynamic memory (size is tracked elsewhere)
    template <size_t DEFAULT_SIZE=0>
    struct Bits_Data_Mem_Dynamic : public Bits_Data_Size_Var<DEFAULT_SIZE>
    {
      using base_t = Bits_Data_Size_Var<DEFAULT_SIZE>;
      using base_size_t = base_t;
      using field_t = bits_field_t;

      Ptr<field_t> bits;      ///< Pointer to array with the status of each bit

      Bits_Data_Mem_Dynamic(size_t num_bits=DEFAULT_SIZE) : base_t(num_bits), bits(nullptr)
      {
        if (num_bits) bits = NewArrayPtr<field_t>(NumBitFields(num_bits));
      }
      Bits_Data_Mem_Dynamic(const Bits_Data_Mem_Dynamic & in) : base_t(), bits(nullptr) { Copy(in); }
      Bits_Data_Mem_Dynamic(Bits_Data_Mem_Dynamic && in) : bits(nullptr) { Move(std::move(in)); }
      ~Bits_Data_Mem_Dynamic() { if (bits) bits.DeleteArray(); }

      Bits_Data_Mem_Dynamic & operator=(const Bits_Data_Mem_Dynamic & in) { Copy(in); return *this; }
      Bits_Data_Mem_Dynamic & operator=(Bits_Data_Mem_Dynamic && in) { Move(std::move(in)); return *this; }

      // --- Helper functions --

      [[nodiscard]] Ptr<field_t> FieldPtr() { return bits; }
      [[nodiscard]] Ptr<const field_t> FieldPtr() const { return bits; }

      void MakeEmpty() {
        base_t::SetSize(0);
        if (bits) bits.DeleteArray();
        bits = nullptr;
      }

      void RawResize(const size_t new_size, const bool preserve_data=false) {
        if (new_size == 0) { return MakeEmpty(); }

        // See if number of bit fields needs to change.
        const size_t num_old_fields = base_t::NumFields();
        const size_t num_new_fields = NumBitFields(new_size);

        if (num_old_fields != num_new_fields) {
          auto new_bits = NewArrayPtr<field_t>(num_new_fields);
          if (num_old_fields) {
            if (preserve_data) {
              size_t copy_count = std::min(num_old_fields, num_new_fields);
              emp::CopyMemory(bits, new_bits, copy_count);
            }
            bits.DeleteArray();  // Delete old memory
          }
          bits = new_bits;     // Use new memory
          if (preserve_data) {
            // Zero out any newly added fields.
            for (size_t i = num_old_fields; i < num_new_fields; ++i) bits[i] = 0;
          }
        }

        base_t::SetSize(new_size);

        // Clear out any extra bits in the last field.
        if (preserve_data && base_t::NumEndBits()) {
          bits[base_t::LastField()] &= base_t::EndMask();
        }
      }

      // Assume size is already correct.
      void Copy(const Bits_Data_Mem_Dynamic & in) {
        RawResize(in.NumBits());
        for (size_t i = 0; i < base_t::NumFields(); ++i) bits[i] = in.bits[i];
      }

      void Move(Bits_Data_Mem_Dynamic && in) {
        base_t::SetSize(in.NumBits());
        if (bits) bits.DeleteArray();  // Clear out old bits.
        bits = in.bits;     // Move over the bits.
        in.bits = nullptr;  // Clear them out of the original.
      }

      [[nodiscard]] auto AsSpan() { return std::span<field_t>(bits.Raw(), base_t::NumFields()); }
      [[nodiscard]] auto AsSpan() const { return std::span<const field_t>(bits.Raw(), base_t::NumFields()); }

      template <class Archive>
      void save(Archive & ar) {
        base_t::serialize(ar); // Save size info.
        for (size_t i=0; i < base_t::NumFields(); ++i) {
          ar(bits[i]);
        }
      }

      template <class Archive>
      void load(Archive & ar) {
        base_t::serialize(ar);
        if (bits) bits.DeleteArray();  // Delete old memory if needed
        bits = NewArrayPtr<field_t>(base_t::NumFields());
        for (size_t i=0; i < base_t::NumFields(); ++i) {
          ar(bits[i]);
        }
      }

      bool OK() const {
        // Do some checking on the bits array ptr to make sure it's value.
        if (bits) {
          #ifdef EMP_TRACK_MEM
          emp_assert(bits.DebugIsArray()); // Must be marked as an array.
          emp_assert(bits.OK());           // Pointer must be okay.
          #endif
        }
        else { emp_assert(base_t::num_bits == 0); }  // If bits is null, num_bits should be zero.
        return true;
      }
    };

    /// Data & functions for Bits types with dynamic memory (size is tracked elsewhere)
    template <size_t DEFAULT_SIZE=0>
    struct Bits_Data_Mem_Watermark : public Bits_Data_Mem_Dynamic<DEFAULT_SIZE>
    {
      using this_t = Bits_Data_Mem_Watermark<DEFAULT_SIZE>;
      using base_t = Bits_Data_Mem_Dynamic<DEFAULT_SIZE>;
      using field_t = bits_field_t;
      using base_t::bits;           ///< Pointer to array with the status of each bit
      size_t field_capacity = 0;    ///< How many fields is the watermark up to?

      Bits_Data_Mem_Watermark(size_t num_bits=DEFAULT_SIZE) : base_t(num_bits)
      {
        field_capacity = base_t::NumFields();
      }
      Bits_Data_Mem_Watermark(const this_t & in) : base_t(0) { Copy(in); }
      Bits_Data_Mem_Watermark(this_t && in) : base_t(0) { Move(std::move(in)); }
      ~Bits_Data_Mem_Watermark() { /* cleanup in base class */ }

      Bits_Data_Mem_Watermark & operator=(const this_t & in) { Copy(in); return *this; }
      Bits_Data_Mem_Watermark & operator=(this_t && in) { Move(std::move(in)); return *this; }

      // --- Helper functions --

      /// Resize to have at least the specified number of fields.
      /// @param new_size The number of bits the new data needs to hold.
      /// @param preserve_data Should we keep existing bits and zero out new bits?
      void RawResize(const size_t new_size, const bool preserve_data=false) {
        // See if number of bit fields needs to change.
        const size_t num_old_fields = base_t::NumFields();
        const size_t num_new_fields = NumBitFields(new_size);

        // If we need more fields than are currently available, reallocate memory.
        if (num_new_fields > field_capacity) {
          auto new_bits = NewArrayPtr<field_t>(num_new_fields);
          if (field_capacity) {    // If we already had some allocated fields...
            // If needed, copy over previous memory.
            if (preserve_data) emp::CopyMemory(bits, new_bits, field_capacity);
            bits.DeleteArray();  // Delete old memory
          }
          field_capacity = num_new_fields;
          bits = new_bits;     // Use new memory
        }

        base_t::SetSize(new_size);

        if (preserve_data) {
          // Clear any new (or previously unused) fields.            
          for (size_t i = num_old_fields; i < num_new_fields; ++i) bits[i] = 0;

          // Clear out any extra end bits.
          if (base_t::NumEndBits()) bits[base_t::LastField()] &= base_t::EndMask();
        }
      }

      void Copy(const Bits_Data_Mem_Watermark & in) {  // Same as base class, but call THIS RawResize().
        RawResize(in.NumBits());
        for (size_t i = 0; i < base_t::NumFields(); ++i) bits[i] = in.bits[i];
      }

      void Move(Bits_Data_Mem_Watermark && in) {
        base_t::Move(std::move(in));
        field_capacity = in.field_capacity;
      }

      template <class Archive>
      void save(Archive & ar) { base_t::save(ar); } // Base class handles saving.

      template <class Archive>
      void load(Archive & ar) {
        base_t::load(ar);
        field_capacity = base_t::NumFields(); // Use loaded size as capacity.
      }

      bool OK() const {
        emp_assert(field_capacity >= base_t::NumFields());
        return base_t::OK();
      }
    };



    /// Internal data for the Bits class to separate static vs. dynamic.
    template <typename BASE_T>
    struct Bits_Data : public BASE_T
    {
      using field_t = bits_field_t;

      Bits_Data() = default;
      Bits_Data(size_t num_bits) : BASE_T(num_bits) { }
      Bits_Data(const Bits_Data & in) = default;
      Bits_Data(Bits_Data && in) = default;

      Bits_Data & operator=(const Bits_Data &) = default;
      Bits_Data & operator=(Bits_Data &&) = default;

      [[nodiscard]] emp::Ptr<unsigned char> BytePtr() {
        return BASE_T::FieldPtr().template ReinterpretCast<unsigned char>();
      }
      [[nodiscard]] emp::Ptr<const unsigned char> BytePtr() const {
        return BASE_T::FieldPtr().template ReinterpretCast<const unsigned char>();
      }

      [[nodiscard]] auto AsByteSpan() const { return std::as_bytes( BASE_T::AsSpan() ); }

      [[nodiscard]] bool OK() const {
        bool result = BASE_T::OK();

        // If there are end bits, make sure that everything past the last one is clear.
        if (BASE_T::NumEndBits()) {
          // Make sure final bits are zeroed out.
          const field_t excess_bits =
            BASE_T::bits[BASE_T::LastField()] & ~MaskLow<field_t>(BASE_T::NumEndBits());
          result &= !excess_bits;
        }

        return result;
      }

    };
  }

  using Bits_WatermarkData = internal::Bits_Data< internal::Bits_Data_Mem_Watermark<0> >;
  using Bits_DynamicData   = internal::Bits_Data< internal::Bits_Data_Mem_Dynamic<0> >;
  template <size_t NUM_BITS>
  using Bits_FixedData     = internal::Bits_Data< internal::Bits_Data_Mem_Fixed<NUM_BITS> >;
  template <size_t MAX_BITS>
  using Bits_StaticData    = internal::Bits_Data< internal::Bits_Data_Mem_Static<MAX_BITS> >;
}

#endif // #ifndef EMP_BITS_BITS_DATA_HPP_INCLUDE
