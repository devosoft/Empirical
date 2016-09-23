//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the grid options for evo::World.h

#include <iostream>

#include "../../evo/World.h"
#include "../../tools/Random.h"
#include "../../tools/string_utils.h"

namespace evo = emp::evo;

int main()
{
  constexpr int POP_SIZE = 3600;
  constexpr int GENS = 10000;

  emp::Random random;

  std::function<std::string(int*)> print_fun = [](int * val){
    char out_char = '+';
    *val %= 63;
    if (*val < 10) out_char = '0' + *val;
    else if (*val < 36) out_char = 'a' + (*val - 10);
    else if (*val < 62) out_char = 'A' + (*val - 36);
    return emp::to_string(out_char);
  };

  evo::GridWorld<int> grid_world(random);
  grid_world.ConfigPop(std::sqrt(POP_SIZE), std::sqrt(POP_SIZE));
  // for (int i = 0; i < 63; i++) grid_world.Insert(i);
  for (int i = 0; i < POP_SIZE; i++) grid_world.InsertAt(i,i);
  grid_world.Print(print_fun);

  for (int g = 0; g < GENS; g++) {
    for (int i = 0; i < grid_world.GetSize(); ++i) {
      int id = random.GetInt(grid_world.GetSize());
      if (grid_world.IsOccupied(id)) grid_world.InsertBirth(grid_world[id], id);
    }
    if (g % 1000 == 0) std::cout << "Generation: " << g << std::endl;
  }

  std::cout << std::endl;
  grid_world.Print(print_fun);
  std::cout << std::endl;
}
