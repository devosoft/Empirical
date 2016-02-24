//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the Symbulation setup.

#include <iostream>

#include "../../evo/Symbulation.h"
#include "../../evo/Population.h"
#include "../../tools/Random.h"

constexpr int POP_SIZE = 1000;
constexpr int UD_COUNT = 1000;


int main()
{
  emp::Random random;
  emp::evo::Population<emp::evo::SymbulationOrg> pop;

  for (int i = 0; i < POP_SIZE; i++) {
    pop.Insert(random, 100);  // Generate a random org.
  }
  
  std::cout << "Start!" << std::endl;
  pop.Execute();
  pop.Execute();
  pop.Execute();
  std::cout << "Done!" << std::endl;
  
}
