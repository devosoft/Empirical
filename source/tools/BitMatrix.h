//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  BIT_MATRIX maintains a COL x ROW matrix of bits and provides easy indexing and manipulation.
//
//  Status: BETA
//
//  Bits are translated to a bitset with 0 in the upper left and moving through bits from
//  left to right and top to bottom.  For example, the indecies in a 3x3 bit matrix would be
//  organized as such:
//
//    0 1 2
//    3 4 5
//    6 7 8
//

#ifndef EMP_BIT_MATRIX_H
#define EMP_BIT_MATRIX_H

#include <iostream>
#include <typeinfo>

#include "BitSet.h"

#ifndef functions
#include "functions.h"
#endif

#include "bitset_utils.h"

namespace emp {
  template <size_t COLS, size_t ROWS>
  class BitMatrix {
  private:
    BitSet<COLS*ROWS> bits;

  public:
    template <size_t START_POS, size_t STEP_POS, size_t END_POS>
    constexpr BitSet<COLS*ROWS> Mask() const {
      return BitSet<COLS*ROWS>();
    }

    template <size_t COL_ID>
    static const BitSet<COLS*ROWS> & MaskCol() {
      static bool init = false;
      static BitSet<COLS*ROWS> mask;
      if (!init) {
        for (size_t i = 0; i < ROWS; i++) mask[i*COLS + COL_ID] = 1;
        init = true;
      }
      return mask;
      // return mask_pattern<COLS*ROWS, COL_ID, COLS, COLS*ROWS-1>();
    }

    template <size_t ROW_ID>
    static const BitSet<COLS*ROWS> & MaskRow() {
      static bool init = false;
      static BitSet<COLS*ROWS> mask;
      if (!init) {
        for (size_t i = 0; i < COLS; i++) mask[ROW_ID * COLS + i] = 1;
        init = true;
      }
      return mask;
    }

//  public:
    BitMatrix() { ; }
    BitMatrix(const BitSet<COLS*ROWS> & in_bits) : bits(in_bits) { ; }
    BitMatrix(const BitMatrix & in_matrix) : bits(in_matrix.bits) { ; }
    ~BitMatrix() { ; }

    constexpr size_t NumRows() const { return ROWS; }
    constexpr size_t NumCols() const { return COLS; }
    constexpr size_t GetSize() const { return ROWS * COLS; }

    inline static size_t GetCol(size_t id) { return id % COLS; }
    inline static size_t GetRow(size_t id) { return id / COLS; }
    inline static size_t GetID(size_t col, size_t row) { return row * COLS + col; }

    bool Any() const { return bits.any(); }
    bool None() const { return bits.none(); }
    bool All() const { return bits.all(); }

    bool Get(size_t col, size_t row) const { return bits[GetID(col,row)]; }
    bool Get(size_t id) const { return bits[id]; }

    void Set(size_t col, size_t row, bool val=true) { bits[GetID(col, row)] = val; }
    void Set(size_t id) { bits[id] = true; }
    void Unset(size_t col, size_t row) { bits[GetID(col, row)] = false; }
    void Unset(size_t id) { bits[id] = false; }
    void Flip(size_t col, size_t row) { bits.flip(GetID(col, row)); }
    void Flip(size_t id) { bits.flip(id); }

    void SetAll() { bits.SetAll(); }
    void SetCol(size_t col) { bits |= MaskCol<0>() << col;}
    void SetRow(size_t row) { bits |= (MaskRow<0>() << (row * COLS)); }
    void Clear() { bits.Clear(); }
    void ClearCol(size_t col) { bits &= ~(MaskCol<0>() << col); }
    void ClearRow(size_t row) { bits &= ~(MaskRow<0>() << (row * COLS)); }

    // Count the number of set bits in the matrix.
    size_t CountOnes() const { return bits.count(); }

    // Find the position of the first non-zero bit.
    // size_t FindBit() const { return (~bits & (bits - 1)).count(); }

    int FindBit() const { return bits.FindBit(); }

    // Shift the whole matrix in the specified direction.
    BitMatrix LeftShift() const { return ((bits & ~MaskCol<0>()) >> 1); }
    BitMatrix RightShift() const { return ((bits << 1) & ~MaskCol<0>()); }
    BitMatrix UpShift() const { return bits >> COLS; }
    BitMatrix DownShift() const { return bits << COLS; }
    BitMatrix ULShift() const { return ((bits & ~MaskCol<0>()) >> (COLS+1)); }
    BitMatrix DLShift() const { return ((bits & ~MaskCol<0>()) << (COLS-1)); }
    BitMatrix URShift() const { return ((bits >> (COLS-1)) & ~MaskCol<0>()); }
    BitMatrix DRShift() const { return ((bits << (COLS+1)) & ~MaskCol<0>()); }

    // Find all points within one step of the ones on this bit matrix.
    BitMatrix GetReach() const { return *this | LeftShift() | RightShift() | UpShift() | DownShift(); }

    // Find all points reachable from the start position.
    BitMatrix GetRegion(size_t start_pos) const {
      // Make sure we have a legal region, or else return an empty matrix.
      if (start_pos < 0 || start_pos >= GetSize() || bits[start_pos] == 0) return BitMatrix();

      BitMatrix cur_region, last_region;
      cur_region.Set(start_pos);

      while (cur_region != last_region) {
        last_region = cur_region;
        cur_region = *this & cur_region.GetReach();
      }

      return cur_region;
    }
    BitMatrix GetRegion(size_t col, size_t row) const { return GetRegion(GetID(col,row)); }

    // Does this bit matrix represent a connected set of ones?
    bool IsConnected() const { return GetRegion((size_t)FindBit()) == *this; }

    // Does this bit matrix have any 2x2 square of ones in it?
    bool Has2x2() const { return (*this & UpShift() & LeftShift() & ULShift()).Any(); }

    void Print(std::ostream & os = std::cout) const {
      for (size_t y = 0; y < ROWS; y++) {
        for (size_t x = 0; x < COLS; x++) {
          os << bits[GetID(x,y)];
        }
        os << std::endl;
      }
    }

    // Assignments and compound assignments
    BitMatrix & operator=(const BitMatrix & in) { bits = in.bits; return *this; }
    BitMatrix & operator&=(const BitMatrix & in) { bits &= in.bits; return *this; }
    BitMatrix & operator|=(const BitMatrix & in) { bits |= in.bits; return *this; }
    BitMatrix & operator^=(const BitMatrix & in) { bits ^= in.bits; return *this; }

    // Comparisons
    bool operator==(const BitMatrix & in) const { return bits == in.bits; }
    bool operator!=(const BitMatrix & in) const { return bits != in.bits; }

    // Logic operators
    BitMatrix operator~() const { return ~bits; }
    BitMatrix operator&(const BitMatrix & in) const { return bits & in.bits; }
    BitMatrix operator|(const BitMatrix & in) const { return bits | in.bits; }
    BitMatrix operator^(const BitMatrix & in) const { return bits ^ in.bits; }

    // Conversions
    const BitSet<COLS*ROWS> & to_bitset() { return bits; }
  };
}

#endif
