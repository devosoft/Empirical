//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE

#ifndef EMP_RANDOM_UTILS_H
#define EMP_RANDOM_UTILS_H

#include <vector>

#include "BitVector.h"
#include "Random.h"
#include "vector.h"

namespace emp {

  // Shuffle all of the elements in a vector.
  // If max_count is provided, just make sure that the first max_count entries are randomly
  // drawn from entire vector.
  template <typename T>
  void Shuffle(Random & random, emp::vector<T> & v, int max_count=-1)
  {
    if (max_count < 0) max_count = (int) v.size();
    for (uint32_t i = 0; i < max_count; i++) {
      const uint32_t pos = random.GetUInt(i, v.size());
      if (pos == i) continue;
      std::swap(v[i], v[pos]);
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
