/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file combos.hpp
 *  @brief Tools to step through combinations of items.
 *
 *  Step through all combinations of size K from a set of N values.  For ComboIDs just return the
 *  indecies (the specific of the container don't matter).  Other versions will directly wrapper
 *  containers.
 *
 *  @todo Allow a minimum and maximum combo size, not just a fixed combo size.  If no sizes are
 *        specifed, all sizes should be explored (all possible combinations).
 *  @todo Create a ComboGenerator template to take a container and generate all combinations
 *  @todo Create a combos function that will build a ComboGenerator for the container type
 *        passed in.
 */

#ifndef EMP_MATH_COMBOS_HPP_INCLUDE
#define EMP_MATH_COMBOS_HPP_INCLUDE

#include <iostream>

#include "../base/assert.hpp"
#include "../base/vector.hpp"

namespace emp {

  class ComboIDs {
  private:
    size_t max_count;
    emp::vector<size_t> cur_combo;
    emp::vector<size_t> max_combo;
    size_t num_combos;

    static size_t CountCombos(size_t max_count, size_t combo_size);
  public:
    ComboIDs(size_t in_max, size_t combo_size);
    ~ComboIDs() { ; }

    // Accessors
    const emp::vector<size_t> & GetCombo() const { return cur_combo; }
    const emp::vector<size_t> & GetMaxCombo() const { return max_combo; }
    size_t GetComboSize() const { return cur_combo.size(); }
    size_t GetNumCombos() const { return num_combos; }

    size_t & operator[](const size_t index) { return cur_combo[index];  }
    const size_t & operator[](const size_t index) const { return cur_combo[index];  }

    // General Use manipulatros
    const emp::vector<size_t> & Reset();
    bool NextCombo();
    void ResizeCombos(size_t new_size);

    // Deal with inversions...
    emp::vector<size_t> GetInverseCombo();

    // Make sure obvious operators also work plus standatd library compatability.
    ComboIDs & operator++() { NextCombo(); return *this; }
    ComboIDs & operator++(int) { NextCombo(); return *this; }
    size_t size() { return num_combos; }
  };

  ComboIDs::ComboIDs(size_t in_max, size_t combo_size)
    : max_count(in_max), cur_combo(combo_size), max_combo(combo_size),
      num_combos(CountCombos(in_max, combo_size))
  {
    emp_assert(combo_size <= in_max);
    const size_t diff = in_max - combo_size;
    for (size_t i = 0; i < cur_combo.size(); i++) {
      cur_combo[i] = i;
      max_combo[i] = i+diff;
    }
  }

  const emp::vector<size_t> & ComboIDs::Reset()
  {
    for (size_t i = 0; i < cur_combo.size(); i++) {
      cur_combo[i] = i;
    }
    return cur_combo;
  }

  bool ComboIDs::NextCombo()
  {
    size_t inc_pos = cur_combo.size() - 1;
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
    for (size_t i = inc_pos + 1; i < cur_combo.size(); i++) {
      cur_combo[i] = cur_combo[i-1] + 1;
    }

    return true;
  }

  void ComboIDs::ResizeCombos(size_t new_size)
  {
    emp_assert(new_size < max_count);

    // Reset internal state...
    cur_combo.resize(new_size);
    max_combo.resize(new_size);
    num_combos = CountCombos(max_count, new_size);

    const size_t diff = max_count - new_size;
    for (size_t i = 0; i < new_size; i++) {
      cur_combo[i] = i;
      max_combo[i] = i+diff;
    }
  }


  emp::vector<size_t> ComboIDs::GetInverseCombo()
  {
    size_t inverse_size = max_count - cur_combo.size();
    emp::vector<size_t> inverse_combo(inverse_size);

    size_t norm_pos = 0;
    size_t inv_pos = 0;
    for (size_t i = 0; i < max_count; i++) {
      if (norm_pos < cur_combo.size() && cur_combo[norm_pos] == i) {
        norm_pos++;  // Found in cur combo...
     } else inverse_combo[inv_pos++] = i;         // Not in cur; put in inverse.
    }
    return inverse_combo;
  }


  size_t ComboIDs::CountCombos(size_t max_count, size_t combo_size)
  {
    if (combo_size * 2 > max_count) combo_size = max_count - combo_size;

    size_t choose_product = 1;
    size_t total_product = 1;
    for (size_t i = 0; i < combo_size; i++) {
      choose_product *= i+1;
      total_product *= max_count - i;
    }

    return total_product / choose_product;
  }

}

#endif // #ifndef EMP_MATH_COMBOS_HPP_INCLUDE
