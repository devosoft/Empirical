//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the template defined in EA::Population.h

#include <iostream>

#include "../../EA/NK.h"
#include "../../EA/Population.h"
#include "../../tools/BitSet.h"
#include "../../tools/Random.h"

constexpr int K = 2;
constexpr int N = 10;

constexpr int POP_SIZE = 100;
constexpr int UD_COUNT = 100;

using BitOrg = emp::BitSet<N>;

int main()
{
  emp::Random random;
  emp::EA::NKLandscape<N,K> landscape;
  emp::EA::Population<BitOrg> pop;

  // Build a random initial population
  for (int i = 0; i < POP_SIZE; i++) {
    BitOrg next_org;
    for (int j = 0; j < N; j++) next_org[j] = random.P(0.5);
    pop.Insert(next_org);
  }

  // Loop through updates
  for (int ud = 0; ud < UD_COUNT; ud++) {
    // Keep the best individual.
    pop.EliteSelect([landscape](BitOrg * org){ return landscape.GetFitness(*org); }, 1);

    // Run a tournament for the rest...
    pop.TournamentSelect([landscape](BitOrg * org){ return landscape.GetFitness(*org); }
			 , 5, random, POP_SIZE-1);
    pop.Update();

    // Do mutations...
    for (int i = 1; i < pop.GetSize(); i++) {
      pop[i][random.GetInt(N)] = random.P(0.5);
    }

    // Print results
    // for (int i = 0; i < pop.GetSize(); i++) std::cout << pop[i] << std::endl;
    // std::cout << std::endl;
    std::cout << pop[0] << " : " << landscape.GetFitness(pop[0]) << std::endl;
  }
  

}
