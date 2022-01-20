/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2017
 *
 *  @file random_utils.hpp
 *  @brief Helper functions for emp::Random for common random tasks.
 *  @note Status: RELEASE
 */

#ifndef EMP_MATH_RANDOM_UTILS_HPP_INCLUDE
#define EMP_MATH_RANDOM_UTILS_HPP_INCLUDE

#include <functional>

#include "../base/vector.hpp"
#include "../bits/BitVector.hpp"

#include "Random.hpp"

namespace emp {

  /// Randomly reorder all of the elements in a vector.
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


  /// Generate a random BitVector of the specified size.
  inline BitVector RandomBitVector(Random & random, size_t size, double p=0.5)
  {
    emp_assert(p >= 0.0 && p <= 1.0);
    BitVector bits(size);
    for (size_t i = 0; i < size; i++) bits[i] = random.P(p);
    return bits;
  }

  /// Generate a random double vector in the specified range.
  inline emp::vector<double> RandomDoubleVector(Random & random, size_t size, double min, double max) {
    emp::vector<double> vals(size);
    for (double & v : vals) v = random.GetDouble(min, max);
    return vals;
  }

  /// Generate a random size_t vector in the specified range.
  inline emp::vector<size_t> RandomUIntVector(Random & random, size_t size, size_t min, size_t max) {
    emp::vector<size_t> vals(size);
    for (size_t & v : vals) v = random.GetUInt(min, max);
    return vals;
  }

  /// Generate a random vector in the specified type and range.
  template <typename T>
  inline emp::vector<T> RandomVector(Random & random, size_t size, T min, T max) {
    emp::vector<T> vals(size);
    for (T & v : vals) v = (T) random.GetDouble((double) min, (double) max);
    return vals;
  }

  /// Generate a random BitVector of the specified size.
  inline void RandomizeBitVector(BitVector & bits, Random & random, double p=0.5)
  {
    emp_assert(p >= 0.0 && p <= 1.0);
    for (size_t i = 0; i < bits.size(); i++) bits[i] = random.P(p);
  }

  /// Generate a random vector in the specified type and range.
  template <typename T>
  inline void RandomizeVector(emp::vector<T> & vals, Random & random, T min, T max) {
    for (T & v : vals) v = (T) random.GetDouble((double) min, (double) max);
  }

  inline size_t CountRngTouches(std::function< void( emp::Random& ) > routine) {
    emp::Random rand_baseline{ 1 };
    emp::Random rand_comparison{ 1 };

    routine( rand_baseline );
    const auto after_routine = rand_baseline.GetUInt();

    size_t count{};
    while( rand_comparison.GetUInt() != after_routine ) ++count;

    return count;

  }

}

#endif // #ifndef EMP_MATH_RANDOM_UTILS_HPP_INCLUDE
