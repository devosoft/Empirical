//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the Symbulation setup.

#include <iostream>

#include "../../evo/Symbulation.h"
#include "../../evo/Population.h"
#include "../../tools/Random.h"

constexpr int POP_SIZE = 10;
constexpr int ORG_SIZE = 100;
constexpr int UD_COUNT = 1000;


int main()
{
  emp::Random random;
  emp::evo::Population<emp::evo::SymbulationOrg> pop(random);

  for (int i = 0; i < POP_SIZE; i++) {
    pop.Insert(random, ORG_SIZE);  // Generate a random org.
  }
  
  std::cout << "Start!" << std::endl;
  for (int ud = 0; ud < UD_COUNT; ud++) {
    pop.Execute();
  }
  std::cout << "Done!" << std::endl;
  
}
