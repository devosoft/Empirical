/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file combos.cpp
 *  @brief Example code for emp::ComboIDs class, emp::ComboGenerator class, and emp::combos function.
 */

#include <iostream>

#include "emp/math/combos.hpp"

int main()
{
  emp::ComboIDs combos(8,4);
  const size_t num_combos = combos.GetNumCombos();
  std::cout << "Num combos = " << num_combos << std::endl;

  for (size_t i = 0; i < num_combos; i++) {
    for (size_t j = 0; j < combos.GetComboSize(); j++) std::cout << " " << combos[j];
    std::cout << std::endl;
    ++combos;
  }

  std::cout << "Temp." << std::endl;
}
