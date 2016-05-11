//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the Symbulation setup.

#include <iostream>

#include "../../evo/Symbulation.h"
#include "../../evo/World.h"
#include "../../tools/Random.h"

constexpr int POP_SIZE  = 10; // 14000;
constexpr int ORG_SIZE  = 100;
constexpr int SYMB_SIZE = 50;
constexpr int UD_COUNT  = 10; //20000;


int main()
{
  emp::Random random;
  emp::evo::World<emp::evo::SymbulationOrg> world(random);

  for (int i = 0; i < POP_SIZE; i++) {
    world.InsertRandomOrg(ORG_SIZE);  // Fill population with random organisms
  }

  // Start with a single symbiont.
  world[0].SetSymbiont(RandomBitVector(random, SYMB_SIZE, 0.5));

  world.Print();

  std::cout << "Start!" << std::endl;
  for (int ud = 0; ud < UD_COUNT; ud++) {
    world.Execute();
    std::cout << "UPDATE " << ud << std::endl;
  }
  std::cout << "Done!" << std::endl;

}
