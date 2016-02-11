//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the template defined in evo::Population.h

#include <iostream>

#include "../../evo/Population.h"
#include "../../tools/BitSet.h"
#include "../../tools/Random.h"

using BitOrg = emp::BitSet<100>;

int main()
{
  emp::Random random;

  emp::evo::Population<BitOrg> pop;

  // Build a random initial population
  for (int i = 0; i < 100; i++) {
    BitOrg next_org;
    for (int j = 0; j < 100; j++) next_org[j] = random.P(0.3);
    pop.Insert(next_org);
  }

  std::cout << "Start Size = " << pop.GetSize() << std::endl;
  for (int i = 0; i < pop.GetSize(); i++) std::cout << pop[i] << std::endl;
  std::cout << std::endl;

  // Loop through 100 updates
  for (int ud = 0; ud < 100; ud++) {
    // Run a tournament...
    pop.TournamentSelect([](BitOrg * org){ return (double) org->CountOnes(); }, 5, random, 100);
    pop.Update();

    // Do mutations...
    for (int i = 0; i < pop.GetSize(); i++) {
      pop[i][random.GetInt(100)] = random.P(0.3);
    }

    // Print results
    // for (int i = 0; i < pop.GetSize(); i++) std::cout << pop[i] << std::endl;
    // std::cout << std::endl;
    std::cout << pop[0] << std::endl;
  }

//   pop.EliteSelect([](int * i){ return (double) *i; }, 10, 10);
//   pop.Update();
//   std::cout << "Post-Elite Size = " << pop.GetSize() << std::endl;
//   for (int i = 0; i < pop.GetSize(); i++) std::cout << pop[i] << " ";
//   std::cout << std::endl;

}
