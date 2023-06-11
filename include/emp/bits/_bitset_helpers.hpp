/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file _bitset_helpers.hpp
 *  @brief An internal Empirical class with tools to build collections of bits.
 */

#ifndef EMP_BITS__BITSET_HELPERS_HPP_INCLUDE
#define EMP_BITS__BITSET_HELPERS_HPP_INCLUDE


namespace emp {

  /// BitProxy lets us use operator[] on with BitVector or BitSet as an lvalue.
  template <typename T>
  class BitProxy {
  private:
      T & bit_container; ///< Which container does this proxy belong to?
      size_t index;      ///< Which position in the bit vector does this proxy point at?

  public:
      // Helper functions.
      inline bool Get() const { return bit_container.Get(index); }
      inline BitProxy & Set(bool b=true) { bit_container.Set(index, b); return *this; }
      inline BitProxy & Clear() { bit_container.Clear(index); return *this; }
      inline BitProxy & Toggle() { bit_container.Toggle(index); return *this; }
      inline BitProxy & SetIf(bool test, bool b) { if (test) Set(b); return *this; }
      inline BitProxy & ToggleIf(bool test) { if (test) Toggle(); return *this; }

      /// Setup a new proxy with the associated vector and index.
      BitProxy(T & _v, size_t _idx) : bit_container(_v), index(_idx) {;}

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
      BitProxy & operator /=([[maybe_unused]] bool b) {
      emp_assert(b == true, "BitProxy Division by Zero error.");
      return *this;
      }
  }; // --- End of BitProxy
}

#endif // #ifndef EMP_BITS__BITSET_HELPERS_HPP_INCLUDE
