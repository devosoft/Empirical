#ifndef EMP_RANDOM_MATRIC_H
#define EMP_RANDOM_MATRIC_H

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  This class maintains a COL x ROW matrix of bits and provides easy indexing and manipulation.
//
//  Bits are translated to a bitset with 0 in the upper left and moving through bits from
//  left to right and top to bottom.  For example, the indecies in a 3x3 bit matrix would be
//  organized as such:
//
//    0 1 2
//    3 4 5
//    6 7 8
//

#include <bitset>
#include <iostream>

namespace emp {
  template <int COLS, int ROWS>
  class BitMatrix {
  private:
    std::bitset<COLS * ROWS> bits;

    inline static int id2col(int id) { return id % COLS; }
    inline static int id2row(int id) { return id / COLS; }
    inline static int get_id(int col, int row) { return row * COLS + col; }

    int get_bit(int col, int row) const { return bits[get_id(col,row)]; }
    void set_bit(int col, int row) const { bits[get_id(col,row)] = 1; }
    void unset_bit(int col, int row) const { bits[get_id(col,row)] = 0; }

    static bool static_init = false;
    static std::bitset<COLS*ROWS> col1_mask;
    static std::bitset<COLS*ROWS> row1_mask;
    static std::bitset<COLS*ROWS> outer_mask;  // Outer ring mask

    void StaticInit() {
      // Initialize static masks used in various functions.
      if (static_init) return; // Only run initialization once.

      // Setup the column one mask (to be used for any column with shifting.
      for (int i = 0; i < ROWS; i++) col1_mask[i*COLS] = 1;

      // Setup the row one mask
      for (int i = 0; i < COLS; i++) row1_mask[i] = 1;

      // Setup the mask for the outer ring
      outer_mask = col1_mask | row1_mask | (col1_mask << (COLS-1)) | (row1_mask << COLS * (ROWS-1));
    }
  public:
    BitMatrix() { ; }
    BitMatrix(BitMatrix & in_matrix) : bits(in_matrix.bits) { ; }
    ~BitMatrix() { ; }

    constexpr int NumRows() { return ROWS; }
    constexpr int NumCols() { return COLS; }
    constexpr int GetSize() { return ROWS * COLS; }

    bool Any() const { return bits.any(); }
    bool None() const { return bits.none(); }
    bool All() const { return bits.all(); }

    bool Get(int col, int row) { return bits[get_id(col,row)]; }
    bool Get(int id) { return bits[id]; }
    
    void Set(int col, int row, bool val=true) { bits[get_id(col, row)] = val; }
    void Set(int id) { bits[id] = true; }
    void Unset(int col, int row) { bits[get_id(col, row)] = false; }
    void Unset(int id) { bits[id] = false; }
    void Flip(int col, int row) { bits.flip(get_id(col, row)); }
    void Flip(int id) { bits.flip(id); }
    void SetAll() { bits.set(); }
    void Clear() { bits.clear(); }

    // Count the number of set bits in the matrix.
    int CountOnes() const { return bits.count(); }

    // Find the position of the first non-zero bit.
    int FindBit() const { return (~bits & (bits - 1)).count(); }

    // Shift the whole matrix in the specified direction.
    BitMatrix LeftShift() const { return ((bits & ~col1_mask) >> 1); }
    BitMatrix RightShift() const { return ((bits << 1) & ~col1_mask); }
    BitMatrix UpShift() const { return bits >> COLS; } 
    BitMatrix DownShift() const { return bits << COLS; }
    BitMatrix ULShift() const { return ((bits & ~col1_mask) >> (COLS+1)); }    
    BitMatrix DLShift() const { return ((bits & ~col1_mask) << (COLS-1)); }
    BitMatrix URShift() const { return ((bits >> (COLS-1)) & ~col1_mask); }
    BitMatrix DRShift() const { return ((bits << (COLS+1)) & ~col1_mask); }

    // Find all points within one step of the ones on this bit matrix.
    BitMatrix GetReach() const { return *this | LeftShift() | RightShift() | UpShift() | DownShift(); }

    // Find all points reachable from the start position.
    BitMatrix GetRegion(int start_pos) const {
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
    BitMatrix GetRegion(int col, int row) const { return GetRegion(get_id(col,row)); }

    // Does this bit matrix represent a connected set of ones?
    bool IsConnected() const { return GetRegion(FindBit()) == *this; }

    // Does this bit matrix have any 2x2 square of ones in it?
    bool Has2x2() const { return (*this & UpShift() & LeftShift() & ULShift()).Any(); }

    void Print(std::ostream & os = std::cout) const {
      for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
          os << bits[get_id(x,y)];
        }
        os << endl;
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
  };
};

#endif
