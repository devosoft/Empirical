//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the template defined in evo::Population.h

#include <iostream>
#include <string>

//#include "../../config/ArgManager.h"
#include "../../evo/NK.h"
#include "../../evo/World.h"
#include "../../tools/BitSet.h"
#include "../../tools/Random.h"
#include "../../evo/EvoStats.h"
#include "../../evo/StatsManager.h"

EMP_BUILD_CONFIG( NKConfig,
  GROUP(DEFAULT, "Default settings for NK model"),
  VALUE(K, int, 10, "Level of epistasis in the NK model"),
  VALUE(N, int, 50, "Number of bits in each organisms (must be > K)"), ALIAS(GENOME_SIZE),
  VALUE(SEED, int, 0, "Random number seed (0 for based on time)"),
  VALUE(POP_SIZE, int, 1000, "Number of organisms in the popoulation."),
  VALUE(MAX_GENS, int, 2000, "How many generations should we process?"),
  VALUE(MUT_COUNT, double, 0.005, "How many bit positions should be randomized?"), ALIAS(NUM_MUTS),
  VALUE(TOUR_SIZE, int, 20, "How many organisms should be picked in each Tournament?"),
  VALUE(NAME, std::string, "Result-", "Name of file printed to"),
)


using BitOrg = emp::BitVector;

template <typename ORG>
using MixedWorld = emp::evo::World<ORG, emp::evo::PopulationManager_Base<ORG>, emp::evo::LineagePruned >;

int main(int argc, char* argv[])
{
  NKConfig config;
  config.Read("Mixed.cfg");

  //auto args = emp::cl::ArgManager(argc, argv);
  //if (args.ProcessConfigOptions(config, std::cout, "Mixed.cfg", "NK-macros.h") == false) exit(0);
  //if (args.TestUnknown() == false) exit(0);  // If there are leftover args, throw an error.

  // k controls # of hills in the fitness landscape
  const int K = config.K();
  const int N = config.N();
  const double MUTATION_RATE = config.MUT_COUNT();

  const int TOURNAMENT_SIZE = config.TOUR_SIZE();
  const int POP_SIZE = config.POP_SIZE();
  const int UD_COUNT = config.MAX_GENS();

  emp::Random random(1234);
  emp::evo::NKLandscape landscape(N, K, random);

  std::string prefix;
  prefix = config.NAME();

  // Create World
  MixedWorld<BitOrg> mixed_pop(random);

  std::function<double(BitOrg *)> fit_func =[&landscape](BitOrg * org) { return landscape.GetFitness(*org);};

  mixed_pop.SetDefaultFitnessFun(fit_func);

  // make a stats manager
  emp::evo::StatsManager_AdvancedStats<emp::evo::PopulationManager_Base<BitOrg>> 
      mixed_stats (&mixed_pop, prefix + "mixed.csv");

  mixed_stats.SetDefaultFitnessFun(fit_func);
  
  // Insert default organisms into world
  for (int i = 0; i < POP_SIZE; i++) {
    BitOrg next_org(N);
    for (int j = 0; j < N; j++) next_org[j] = random.P(0.5);
    
    // looking at the Insert() func it looks like it does a deep copy, so we should be safe in
    // doing this. Theoretically...
    mixed_pop.Insert(next_org);
  }


  // mutation function:
  // for every site in the gnome there is a MUTATION_RATE chance that the 
  // site will flip it's value.
  mixed_pop.SetDefaultMutateFun( [MUTATION_RATE, N](BitOrg* org, emp::Random& random) {
    bool mutated = false;    
      for (size_t site = 0; site < N; site++) {
        if (random.P(MUTATION_RATE)) {
          (*org)[site] = !(*org)[site];
          mutated = true;
        }
      }
      return mutated;
    } );


  // Loop through updates
  for (int ud = 0; ud < UD_COUNT; ud++) {

    // Keep the best individual.
    //mixed_pop.EliteSelect([&landscape](BitOrg * org){ return landscape.GetFitness(*org); }, 1, 100);
    // Run a tournament for the rest... 
    
    mixed_pop.TournamentSelect([&landscape](BitOrg * org){ return landscape.GetFitness(*org); }
			 , TOURNAMENT_SIZE, POP_SIZE);

    mixed_pop.Update();
    mixed_pop.MutatePop();

  }

}
