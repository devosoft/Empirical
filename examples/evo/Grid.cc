//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the grid options for evo::World.h

#include <iostream>

#include "../../evo/World.h"
#include "../../tools/Random.h"

namespace evo = emp::evo;

int main()
{
  emp::Random random;

  evo::GridWorld<int> grid_world(random);
  for (int i = 0; i < 10; i++) grid_world.Insert(i);
  grid_world.Print();

  for (int i = 0; i < 10*grid_world.GetSize(); ++i) {
    int id = random.GetInt(grid_world.GetSize());
    if (grid_world.IsOccupied(id)) grid_world.InsertBirth(grid_world[id], id);
  }


  std::cout << std::endl;
  grid_world.Print();
  std::cout << std::endl;
}

