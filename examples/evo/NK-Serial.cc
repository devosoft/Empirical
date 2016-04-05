//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the template defined in evo::Population.h

#include <iostream>

#include "../../evo/NK.h"
#include "../../evo/World.h"
#include "../../tools/BitSet.h"
#include "../../tools/Random.h"

constexpr int K = 3;
constexpr int N = 50;

constexpr int MIN_POP_SIZE = 100;
constexpr int MAX_POP_SIZE = 10000;
constexpr int UD_COUNT = 1000;

using BitOrg = emp::BitSet<N>;
using STWorld = emp::evo::World<BitOrg, emp::evo::PopulationManager_SerialTransfer<BitOrg>>;

int main()
{
  emp::Random random;                            // Build a random number generator.
  emp::evo::NKLandscape<N,K> landscape(random);  // Build the landscape...
  STWorld world(random);                         // Build the world...
  world.pop.Config(MAX_POP_SIZE,MIN_POP_SIZE);   // Setup default population extremes.

  BitOrg ancestor(random);                       // Build a random ancestor
  world.Insert(ancestor, MIN_POP_SIZE);          // Insert several copies of ancestor

  // Setup default method for mutations...
  world.SetDefaultMutateFun( [](BitOrg* org, emp::Random& random) {
      (*org)[random.GetInt(N)] = random.P(0.5);
      (*org)[random.GetInt(N)] = random.P(0.5);
      (*org)[random.GetInt(N)] = random.P(0.5);
      return true;
    } );


  // Loop through updates
  for (int ud = 0; ud < UD_COUNT; ud++) {
    // Print current state.
    std::cout << ud << " : " << world[0]
              << " : " << landscape.GetFitness(world[0])
              << " : " << world.GetSize()
              << std::endl;

    // Run a tournament for the rest...
    world.TournamentSelect([landscape](BitOrg * org){ return landscape.GetFitness(*org); }
			 , 5, 1000, false);
    world.Update();
    world.MutatePop();
  }


  std::cout << UD_COUNT << " : " << world[0] << " : "
            << landscape.GetFitness(world[0]) << std::endl;
}
