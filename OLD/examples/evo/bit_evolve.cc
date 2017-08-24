//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the template defined in evo::Population.h

#include <iostream>

#include "../../evo/World.h"
#include "../../tools/BitSet.h"
#include "../../tools/Random.h"

using BitOrg = emp::BitSet<100>;

int main()
{
  emp::Random random;
  emp::evo::EAWorld<BitOrg> pop(random);

  // Build a random initial
  for (size_t i = 0; i < 100; i++) {
    BitOrg next_org;
    for (size_t j = 0; j < 100; j++) next_org[j] = random.P(0.3);
    pop.Insert(next_org);
  }

  std::cout << "Start Size = " << pop.GetSize() << std::endl;
  for (size_t i = 0; i < pop.GetSize(); i++) std::cout << pop[i] << std::endl;
  std::cout << std::endl;

  // Loop through 100 updates
  for (size_t ud = 0; ud < 100; ud++) {
    // Run a tournament...
    pop.TournamentSelect([](BitOrg * org){ return (double) org->CountOnes(); }, 5, 100);
    pop.Update();

    // Do mutations...
    for (size_t i = 0; i < pop.GetSize(); i++) {
      pop[i][random.GetUInt(100)] = random.P(0.3);
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
