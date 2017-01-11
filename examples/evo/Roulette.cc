//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the RouletteSelect() function in evo::World.h

#include <iostream>

#include "../../evo/World.h"
#include "../../tools/Random.h"
#include "../../tools/string_utils.h"

namespace evo = emp::evo;

int main()
{
  constexpr size_t POP_SIZE = 400;
  // constexpr size_t GENS = 10000;

  emp::Random random;

  std::function<std::string(int*)> print_fun = [](int * val) {
    char out_char = '+';
    *val %= 63;
    if (*val < 10) out_char = '0' + (char) *val;
    else if (*val < 36) out_char = 'a' + (char) (*val - 10);
    else if (*val < 62) out_char = 'A' + (char) (*val - 36);
    return emp::to_string(out_char);
  };

  evo::GridWorld<int> grid_world(random);
  const size_t side = (size_t) std::sqrt(POP_SIZE);
  grid_world.ConfigPop(side, side);
  emp_assert(grid_world.GetSize() == POP_SIZE); // POP_SIZE needs to be a perfect square.

  grid_world.InsertAt(1, side+1);
  grid_world.InsertAt(4, side*(side+1)/2);
  grid_world.Print(print_fun);

  auto fit_fun = [](int* org){ return (double) *org; };
  grid_world.RouletteSelect(fit_fun);

  std::cout << std::endl;
  grid_world.Print(print_fun);
  std::cout << "Final Org Counts:\n";
  grid_world.PrintOrgCounts(print_fun);
  std::cout << std::endl;
}
