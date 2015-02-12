#ifndef EMP_RANDOM_MATRIC_H
#define EMP_RANDOM_MATRIC_H

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
  public:
    BitMatrix() { ; }
    ~BitMatrix() { ; }

    void Print(std::ostream & os = std::cout) {
      for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
          os << bits[get_id(x,y)];
        }
        os << endl;
      }
    }

  };
};

#endif
