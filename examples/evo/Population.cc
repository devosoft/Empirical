//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the template defined in evo::Population.h

#include <iostream>

#include "../../evo/Population.h"
#include "../../tools/Random.h"

int main()
{
  emp::evo::Population<int> pop;

  for (int i = 0; i < 100; i++) pop.Insert(i+100);

  std::cout << "Start Size = " << pop.GetSize() << std::endl;
  for (int i = 0; i < pop.GetSize(); i++) std::cout << pop[i] << " ";
  std::cout << std::endl;

  emp::Random random;
  pop.TournamentSelect([](int * i){ return (double) *i; }, 5, random, 100);
  pop.Update();
  std::cout << "Post-Tourney Size = " << pop.GetSize() << std::endl;
  for (int i = 0; i < pop.GetSize(); i++) std::cout << pop[i] << " ";
  std::cout << std::endl;

  pop.EliteSelect([](int * i){ return (double) *i; }, 10, 10);
  pop.Update();
  std::cout << "Post-Elite Size = " << pop.GetSize() << std::endl;
  for (int i = 0; i < pop.GetSize(); i++) std::cout << pop[i] << " ";
  std::cout << std::endl;

}
