//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE

#ifndef EMP_RANDOM_UTILS_H
#define EMP_RANDOM_UTILS_H

#include <vector>

#include "BitVector.h"
#include "Random.h"
#include "vector.h"

namespace emp {

  /// Shuffle all of the elements in a vector.
  /// If max_count is provided, just make sure that the first max_count entries are randomly
  /// drawn from entire vector.
  template <typename T>
  void Shuffle(Random & random, emp::vector<T> & v, int max_count=-1)
  {
    if (max_count < 0) max_count = (int) v.size();
    for (uint32_t i = 0; i < (uint32_t) max_count; i++) {
      const uint32_t pos = random.GetUInt(i, v.size());
      if (pos == i) continue;
      std::swap(v[i], v[pos]);
    }
  }


  /// Return an emp::vector<int> numbered 0 through size-1 in a random order.

  emp::vector<int> GetPermutation(Random & random, int size) {
    emp::vector<int> seq(size);
    seq[0] = 0;
    for (int i = 1; i < size; i++) {
      uint32_t val_pos = random.GetUInt(i+1);
      seq[i] = seq[val_pos];
      seq[val_pos] = i;
    }
    return seq;
  }

  /// Choose K positions from N possibilities.

  void Choose(Random & random, int N, int K, std::vector<int> & choices) {
    if (N < K || K < 0) return;  // @CAO Should be an assert!

    choices.resize(K);
    while (K) {
      if (N==K || random.P(((double) K)/((double) N))) { choices[--K] = --N; }
      else --N;
    }
  }

  std::vector<int> Choose(Random & random, int N, int K) {
    std::vector<int> choices;
    Choose(random,N,K,choices);
    return choices;
  }


  /// Generate a random bit vector of the specified size.

  BitVector RandomBitVector(Random & random, int size, double p=0.5)
  {
    BitVector bits(size);
    for (int i = 0; i < size; i++) bits[i] = random.P(p);
    return bits;
  }
}

#endif
