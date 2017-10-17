//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Example code for emp::ComboIDs class, emp::ComboGenerator class, and emp::combos function.

#include <iostream>

#include "tools/combos.h"

int main()
{
  emp::cComboCounter combos(7,4);
  const size_t num_combos = combos.GetNumCombos();
  std::cout << "Num combos = " << num_combos << std::endl;

  for (int i = 0; i < num_combos; i++) {
    auto cur_combo = combos.GetCombo();
    for (int j = 0; j < cur_combo.size(); j++) std::cout << " " << cur_combo[j];
    std::cout << std::endl;
    combos.NextCombo();
  }

  std::cout << "Temp." << std::endl;
}

