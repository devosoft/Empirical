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

constexpr uint32_t POP_SIZE  = 14000;
constexpr uint32_t ORG_SIZE  = 100;
constexpr uint32_t SYMB_SIZE = 50;
constexpr uint32_t UD_COUNT  = 20000;


int main()
{
  emp::Random random;
  emp::evo::World<emp::evo::SymbulationOrg> world(random);

  for (uint32_t i = 0; i < POP_SIZE; i++) {
    world.InsertRandomOrg(ORG_SIZE, 0.5, ORG_SIZE, SYMB_SIZE);// Fill population with random organisms
    world[i].SetSymbiont(RandomBitVector(random, SYMB_SIZE, 0.5));
  }

  world.Print(std::cout, "X", "\n");

  std::cout << "Start!" << std::endl;
  for (uint32_t ud = 0; ud < UD_COUNT; ud++) {
    world.Execute();
    std::cout << "UPDATE " << ud << std::endl;
  }
  std::cout << "Done!" << std::endl;

}
