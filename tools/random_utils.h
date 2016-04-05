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
  void Shuffle(Random & random, std::vector<TYPE> & v)
  {
    for (int i = 1; i < (int) v.size(); i++) {
      int pos = random.GetInt(i+1);
      if (pos == i) continue;
      std::swap(v[i], v[pos]);
    }
  }

  // Shrink N-element vector to a random K elements:
  template <typename T>
  void Shrink(Random & random, std::vector<T> & v, int K) {
    const int N = v.size();
    if (N <= K) return;                     // Nothing to shrink!
    for (int i = 0; i < K; i++) {           // Choose the elements to shrink to.
      const int pos = random.GetInt(i, N);
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
