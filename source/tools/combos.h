/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file  combos.h
 *  @brief Tools to step through combinations of items.
 *
 *  Step through all combinations of size K from a set of N values.
 *
 *  @todo Allow a minimum and maximum combo size, not just a fixed combo size.  If no sizes are
 *        specifed, all sizes should be explored (all possible combinations).
 *  @todo Create a ComboGenerator template to take a container and generate all combinations
 *  @todo Create a combos function that will build a ComboGenerator for the container type
 *        passed in.
 */

#ifndef EMP_COMBOS_H
#define EMP_COMBOS_H

#include <assert.h>
#include <iostream>

#include "../base/vector.h"

namespace emp {

  class ComboIDs {
  private:
    int max_count;
    emp::vector<int> cur_combo;
    emp::vector<int> max_combo;
    int num_combos;

    static int CountCombos(int max_count, int combo_size);
  public:
    ComboIDs(int in_max, int combo_size);
    ~ComboIDs() { ; }

    // Accessors
    const emp::vector<int> & GetCombo() const { return cur_combo; }
    const emp::vector<int> & GetMaxCombo() const { return max_combo; }
    int GetComboSize() const { return cur_combo.size(); }
    int GetNumCombos() const { return num_combos; }

    int & operator[](const int index) { return cur_combo[index];  }
    const int & operator[](const int index) const { return cur_combo[index];  }

    // General Use manipulatros
    const emp::vector<int> & Reset();
    bool NextCombo();
    void ResizeCombos(int new_size);

    // Deal with inversions...
    emp::vector<int> GetInverseCombo();
  };

  ComboIDs::ComboIDs(int in_max, int combo_size)
    : max_count(in_max), cur_combo(combo_size), max_combo(combo_size),
      num_combos(CountCombos(in_max, combo_size))
  {
    assert(combo_size <= in_max);
    const int diff = in_max - combo_size;
    for (int i = 0; i < cur_combo.size(); i++) {
      cur_combo[i] = i;
      max_combo[i] = i+diff;
    }
  }

  const emp::vector<int> & ComboIDs::Reset()
  {
    for (int i = 0; i < cur_combo.size(); i++) {
      cur_combo[i] = i;
    }
    return cur_combo;
  }

  bool ComboIDs::NextCombo()
  {
    int inc_pos = cur_combo.size() - 1;
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
    for (int i = inc_pos + 1; i < cur_combo.size(); i++) {
      cur_combo[i] = cur_combo[i-1] + 1;
    }

    return true;
  }

  void ComboIDs::ResizeCombos(int new_size)
  {
    assert(new_size < max_count);

    // Reset internal state...
    cur_combo.resize(new_size);
    max_combo.resize(new_size);
    num_combos = CountCombos(max_count, new_size);

    const int diff = max_count - new_size;
    for (int i = 0; i < new_size; i++) {
      cur_combo[i] = i;
      max_combo[i] = i+diff;
    }
  }


  emp::vector<int> ComboIDs::GetInverseCombo()
  {
    int inverse_size = max_count - cur_combo.size();
    emp::vector<int> inverse_combo(inverse_size);

    int norm_pos = 0;
    int inv_pos = 0;
    for (int i = 0; i < max_count; i++) {
      if (norm_pos < cur_combo.size() && cur_combo[norm_pos] == i) {
        norm_pos++;  // Found in cur combo...
     } else inverse_combo[inv_pos++] = i;         // Not in cur; put in inverse.
    }
    return inverse_combo;
  }


  int ComboIDs::CountCombos(int max_count, int combo_size)
  {
    if (combo_size * 2 > max_count) combo_size = max_count - combo_size;

    int choose_product = 1;
    int total_product = 1;
    for (int i = 0; i < combo_size; i++) {
      choose_product *= i+1;
      total_product *= max_count - i;
    }

    return total_product / choose_product;
  }

}

#endif
