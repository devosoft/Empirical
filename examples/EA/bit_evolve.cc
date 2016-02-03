//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the template defined in EA::Population.h

#include <iostream>

#include "../../EA/Population.h"
#include "../../tools/Random.h"
#include "../../tools/BitSet.h"

using BitOrg = emp::BitSet<100>;

int main()
{
  emp::Random random;

  emp::EA::Population<BitOrg> pop;

  for (int i = 0; i < 100; i++) {
    BitOrg next_org;
    for (int j = 0; j < 100; j++) next_org[j] = random.P(0.3);
    pop.Insert(next_org);
  }

  std::cout << "Start Size = " << pop.GetSize() << std::endl;
//   for (int i = 0; i < pop.GetSize(); i++) std::cout << pop[i] << " ";
//   std::cout << std::endl;

//   pop.TournamentSelect([](int * i){ return (double) *i; }, 5, random, 100);
//   pop.Update();
//   std::cout << "Post-Tourney Size = " << pop.GetSize() << std::endl;
//   for (int i = 0; i < pop.GetSize(); i++) std::cout << pop[i] << " ";
//   std::cout << std::endl;

//   pop.EliteSelect([](int * i){ return (double) *i; }, 10, 10);
//   pop.Update();
//   std::cout << "Post-Elite Size = " << pop.GetSize() << std::endl;
//   for (int i = 0; i < pop.GetSize(); i++) std::cout << pop[i] << " ";
//   std::cout << std::endl;

}
