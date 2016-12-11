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
  inline void Shuffle(Random & random, emp::vector<T> & v, size_t max_count)
  {
    for (size_t i = 0; i < max_count; i++) {
      const size_t pos = random.GetUInt(i, v.size());
      if (pos == i) continue;
      std::swap(v[i], v[pos]);
    }
  }
  template <typename T>
  inline void Shuffle(Random & random, emp::vector<T> & v) { Shuffle(random, v, v.size()); }

  /// Return an emp::vector<int> numbered 0 through size-1 in a random order.

  inline emp::vector<size_t> GetPermutation(Random & random, size_t size) {
    emp::vector<size_t> seq(size);
    seq[0] = 0;
    for (size_t i = 1; i < size; i++) {
      size_t val_pos = random.GetUInt(i+1);
      seq[i] = seq[val_pos];
      seq[val_pos] = i;
    }
    return seq;
  }

  /// Choose K positions from N possibilities.

  inline void Choose(Random & random, size_t N, size_t K, std::vector<size_t> & choices) {
    emp_assert (N >= K);

    choices.resize(K);
    while (K) {
      if (N==K || random.P(((double) K)/((double) N))) { choices[--K] = --N; }
      else --N;
    }
  }

  inline std::vector<size_t> Choose(Random & random, size_t N, size_t K) {
    std::vector<size_t> choices;
    Choose(random,N,K,choices);
    return choices;
  }


  /// Generate a random bit vector of the specified size.

  inline BitVector RandomBitVector(Random & random, size_t size, double p=0.5)
  {
    emp_assert(p >= 0.0 && p <= 1.0);
    BitVector bits(size);
    for (size_t i = 0; i < size; i++) bits[i] = random.P(p);
    return bits;
  }
}

#endif
