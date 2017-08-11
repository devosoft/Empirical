//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file uses the Fitness Sharing functionality defined in evo::World.h

#include <iostream>

#include "../../evo/NK-const.h"
#include "../../evo3/World.h"
#include "../../tools/BitSet.h"
#include "../../tools/Random.h"

constexpr size_t K = 3;
constexpr size_t N = 50;

constexpr size_t POP_SIZE = 100;
constexpr size_t UD_COUNT = 1000;

using BitOrg = emp::BitSet<N>;

int main()
{
  emp::Random random;
  emp::evo::NKLandscapeConst<N,K> landscape(random);
  emp::World<BitOrg> pop(random);
  pop.SetWellMixed(true);
  pop.SetCache();

  // Build a random initial population
  for (size_t i = 0; i < POP_SIZE; i++) {
    BitOrg next_org;
    for (size_t j = 0; j < N; j++) next_org[j] = random.P(0.5);
    pop.Inject(next_org);
  }

  // Loop through updates
  for (size_t ud = 0; ud < UD_COUNT; ud++) {
    // Run a tournament...
    pop.SetSharedFitFun( [&landscape](BitOrg &org){ return landscape.GetFitness(org); },
                         [](BitOrg& org1, BitOrg& org2){ return (double)(org1->XOR(*org2)).CountOnes();},
                         10, 1 );

    emp::TournamentSelect(pop, 5, POP_SIZE-1);
    pop.Update();

    // Do mutations...
    for (size_t i = 1; i < pop.GetSize(); i++) {
      pop[i][random.GetUInt(N)] = random.P(0.5);
      pop[i][random.GetUInt(N)] = random.P(0.5);
      pop[i][random.GetUInt(N)] = random.P(0.5);
    }

  }


  std::cout << pop[0] << " : " << landscape.GetFitness(pop[0]) << std::endl;
}
