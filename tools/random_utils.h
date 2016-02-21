//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE

#ifndef EMP_RANDOM_UTILS_H
#define EMP_RANDOM_UTILS_H

#include <vector>

#include "BitVector.h"
#include "Random.h"

namespace emp {

  template <typename TYPE>
  void shuffle(std::vector<TYPE> & v, Random & random)
  {
    for (int i = 1; i < (int) v.size(); i++) {
      int new_pos = random.GetInt(i+1);
      if (new_pos == i) continue;
      std::swap(v[i], v[new_pos]);
    }
  }


  BitVector RandomBitVector(Random & random, int size, double p=0.5)
  {
    BitVector bits(size);
    for (int i = 0; i < size; i++) bits[i] = random.P(p);
    return bits;
  }
}

#endif
