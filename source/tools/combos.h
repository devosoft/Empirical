#ifndef COMBO_COUNTER_H
#define COMBO_COUNTER_H

#include <assert.h>
#include <iostream>

#include "tArray.h"

class cComboCounter {
private:
  int max_count;
  tArray<int> cur_combo;
  tArray<int> max_combo;
  int num_combos;

  static int CountCombos(int max_count, int combo_size);
public:
  cComboCounter(int in_max, int combo_size);
  ~cComboCounter() { ; }

  // Accessors
  const tArray<int> & GetCombo() const { return cur_combo; }
  const tArray<int> & GetMaxCombo() const { return max_combo; }
  int GetComboSize() const { return cur_combo.GetSize(); }
  int GetNumCombos() const { return num_combos; }

  int & operator[](const int index) { return cur_combo[index];  }
  const int & operator[](const int index) const { return cur_combo[index];  }

  // General Use manipulatros
  const tArray<int> & Reset();
  bool NextCombo();
  void ResizeCombos(int new_size);

  // Deal with inversions...
  tArray<int> GetInverseCombo();
};

cComboCounter::cComboCounter(int in_max, int combo_size)
  : max_count(in_max), cur_combo(combo_size), max_combo(combo_size),
    num_combos(CountCombos(in_max, combo_size))
{
  assert(combo_size <= in_max);
  const int diff = in_max - combo_size;
  for (int i = 0; i < cur_combo.GetSize(); i++) {
    cur_combo[i] = i;
    max_combo[i] = i+diff;
  }
}

const tArray<int> & cComboCounter::Reset()
{
  for (int i = 0; i < cur_combo.GetSize(); i++) {
    cur_combo[i] = i;
  }
  return cur_combo;
}

bool cComboCounter::NextCombo()
{
  int inc_pos = cur_combo.GetSize() - 1;
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
  for (int i = inc_pos + 1; i < cur_combo.GetSize(); i++) {
    cur_combo[i] = cur_combo[i-1] + 1;
  }

  return true;
}

void cComboCounter::ResizeCombos(int new_size)
{
  assert(new_size < max_count);

  // Reset internal state...
  cur_combo.Resize(new_size);
  max_combo.Resize(new_size);
  num_combos = CountCombos(max_count, new_size);

  const int diff = max_count - new_size;
  for (int i = 0; i < new_size; i++) {
    cur_combo[i] = i;
    max_combo[i] = i+diff;
  }
}


tArray<int> cComboCounter::GetInverseCombo()
{
  int inverse_size = max_count - cur_combo.GetSize();
  tArray<int> inverse_combo(inverse_size);

  int norm_pos = 0;
  int inv_pos = 0;
  for (int i = 0; i < max_count; i++) {
    if (norm_pos < cur_combo.GetSize() && cur_combo[norm_pos] == i) {
      norm_pos++;  // Found in cur combo...
   } else inverse_combo[inv_pos++] = i;         // Not in cur; put in inverse.
  }
  return inverse_combo;
}


int cComboCounter::CountCombos(int max_count, int combo_size)
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

#endif
