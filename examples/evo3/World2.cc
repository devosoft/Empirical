//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file is an example for using the re-vamped World template.

#include <iostream>

#include "../../evo3/World.h"
#include "../../tools/Random.h"

int main() {
  emp::World<int> world;
  world.SetWellMixed(true);

  // Inject from 100 to 199.
  for (int i = 0; i < 100; i++) { world.Inject(i+200); }

  std::cout << "Start Size = " << world.GetSize() << std::endl << std::endl;
  for (size_t i = 0; i < world.GetSize(); i++) std::cout << world[i] << " ";
  std::cout << std::endl;

  // world.EliteSelect(10, 10);
  //
  // std::cout << "\nElite Select(10,10)\n" << std::endl;
  // for (size_t i = 0; i < world.GetSize(); i++) std::cout << world[i] << " ";
  // std::cout << std::endl;
  //
  // world.TournamentSelect(5, 100);
  // world.Update();
  // std::cout << "\nPost-Tourney Size = " << world.GetSize() << std::endl << std::endl;
  // for (size_t i = 0; i < world.GetSize(); i++) std::cout << world[i] << " ";
  // std::cout << std::endl;
  //
  // world.EliteSelect(10, 10);
  // world.Update();
  // std::cout << "\nPost-Elite Size = " << world.GetSize() << std::endl << std::endl;
  // for (size_t i = 0; i < world.GetSize(); i++) std::cout << world[i] << " ";
  // std::cout << std::endl << std::endl;

}
