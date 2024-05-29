/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2024.
*/
/**
 *  @file
 *  @brief Tools to step through combinations of items where size is known a compile time.
 *
 *  Step through all combinations of size K from a set of N values.
 */

#ifndef EMP_MATH_COMBO_SET_HPP_INCLUDE
#define EMP_MATH_COMBO_SET_HPP_INCLUDE

#include <iostream>
#include <stddef.h>

#include "../base/assert.hpp"
#include "../math/math.hpp"

namespace emp {

  template <size_t N, size_t K>
  class ComboSet {
  private:
    std::array<size_t,K> cur_combo = MakeSequenceArray<K>(); // Sequence from [0 to K)
    static constexpr std::array<size_t,K> max_combo = MakeSequenceArray<K>(N-K); // Sequence from [N-K to N)

  public:
    // Accessors
    const std::array<size_t,K> & GetCombo() const { return cur_combo; }
    const std::array<size_t,K> & GetMaxCombo() const { return max_combo; }

    size_t & operator[](const size_t index) { return cur_combo[index];  }
    const size_t & operator[](const size_t index) const { return cur_combo[index];  }

    // General Use manipulators
    const std::array<size_t,K> & Reset() { return cur_combo = MakeSequenceArray<K>(); }
    bool NextCombo();

    // Make sure obvious operators also work plus standard library compatibility.
    ComboSet & operator++() { NextCombo(); return *this; }
    ComboSet & operator++(int) { NextCombo(); return *this; }
    constexpr size_t size();
  };

  template <size_t N, size_t K>
  bool ComboSet<N,K>::NextCombo()
  {
    size_t inc_pos = K - 1;
    cur_combo[inc_pos]++;

    // Increase the first position that we can without it going over the max.
    while (inc_pos > 0 && cur_combo[inc_pos] > max_combo[inc_pos]) {
      inc_pos--;
      cur_combo[inc_pos]++;
    }

    // If we were already on the last combo, reset to the beginning.
    if (cur_combo[0] > max_combo[0]) {
      Reset();
      return false;
    }

    // Update all of the positions after the current one.
    for (size_t i = inc_pos + 1; i < K; i++) {
      cur_combo[i] = cur_combo[i-1] + 1;
    }

    return true;
  }

  template <size_t N, size_t K>
  constexpr size_t ComboSet<N,K>::size()
  {
    constexpr size_t combo_size = std::min(K, N - K);

    size_t result = 1;
    for (size_t i = 0; i < combo_size; i++) {
      result *= N - i;
      result /= i+1;
    }

    return result;
  }

}

#endif // #ifndef EMP_MATH_COMBO_SET_HPP_INCLUDE
