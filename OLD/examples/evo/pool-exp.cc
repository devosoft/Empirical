//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the template defined in evo::Population.h

#include <iostream>
#include <sstream>
#include <string>

#include "../../config/ArgManager.h"
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
  VALUE(POP_SIZE, int, 100, "Number of organisms in the popoulation."),
  VALUE(MAX_GENS, int, 2000, "How many generations should we process?"),
  VALUE(MUT_COUNT, double, 0.005, "How many bit positions should be randomized?"), ALIAS(NUM_MUTS),
  VALUE(TOUR_SIZE, int, 20, "How many organisms should be picked in each Tournament?"),
  VALUE(NAME, std::string, "Result-", "Name of file printed to"),
  VALUE(POOLS, int, 5, "How many pools should the be in the population"),
  VALUE(MIG_COUNT, double, 0.05, "How often should an organsim migrate to another pool?"),
  VALUE(POOL_SIZES, std::string, "10,30,20,20,20", "What size should each pool be?"),
  VALUE(RU, int, 150, "What is the upper limit of random generated pool sizes?"),
  VALUE(RL, int, 10, "What is the lower limit of random generated pool sizes?"),
)


using BitOrg = emp::BitVector;

template <typename ORG>
using PoolWorld = emp::evo::World<ORG, emp::evo::PopulationManager_Pools<ORG>, emp::evo::LineagePruned >;

int main(int argc, char* argv[])
{
  NKConfig config;
  config.Read("Pool.cfg");

  auto args = emp::cl::ArgManager(argc, argv);
  if (args.ProcessConfigOptions(config, std::cout, "Pool.cfg", "NK-macros.h") == false) exit(0);
  if (args.TestUnknown() == false) exit(0);  // If there are leftover args, throw an error.

  // k controls # of hills in the fitness landscape
  const int K = config.K();
  const int N = config.N();
  const double MUTATION_RATE = config.MUT_COUNT();
  const int TOURNAMENT_SIZE = config.TOUR_SIZE();
  const int POP_SIZE = config.POP_SIZE();
  const int UD_COUNT = config.MAX_GENS();
  const int POOLS = config.POOLS();
  const double MIG_RATE = config.MIG_COUNT();
  const int RU = config.RU();
  const int RL = config.RL();
 
  //Manager settings configuration
  const std::string POOL_SIZES = config.POOL_SIZES();
  std::istringstream ss(POOL_SIZES);
  std::string token;
  emp::vector<int> ps;

  while(std::getline(ss, token, ',')){ ps.push_back(stoi(token)); }

  std::map<int, emp::vector<int> >connections;

  emp::Random random(config.SEED());
  emp::evo::NKLandscape landscape(N, K, random);

  std::string prefix;
  prefix = config.NAME();

  // Create World
  PoolWorld<BitOrg> pool_pop(random);

  pool_pop.ConfigPop(POOLS, ps, &connections, RU, RL, MIG_RATE, POP_SIZE);

  std::function<double(BitOrg *)> fit_func =[&landscape](BitOrg * org) { return landscape.GetFitness(*org);};

  pool_pop.SetDefaultFitnessFun(fit_func);

  // make a stats manager
  emp::evo::StatsManager_AdvancedStats<emp::evo::PopulationManager_Pools<BitOrg>> 
      pool_stats (&pool_pop, prefix + "pool.csv");

  pool_stats.SetDefaultFitnessFun(fit_func);
  
  // Insert default organisms into world
  for (int i = 0; i < POP_SIZE; i++) {
    BitOrg next_org(N);
    for (int j = 0; j < N; j++) next_org[j] = random.P(0.5);
    
    // looking at the Insert() func it looks like it does a deep copy, so we should be safe in
    // doing this. Theoretically...
    pool_pop.Insert(next_org);
  }


  // mutation function:
  // for every site in the gnome there is a MUTATION_RATE chance that the 
  // site will flip it's value.
  pool_pop.SetDefaultMutateFun( [MUTATION_RATE, N](BitOrg* org, emp::Random& random) {
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
    //pool_pop.EliteSelect([&landscape](BitOrg * org){ return landscape.GetFitness(*org); }, 1, 100);
    // Run a tournament for the rest... 
    
    pool_pop.TournamentSelect([&landscape](BitOrg * org){ return landscape.GetFitness(*org); }
			 , TOURNAMENT_SIZE, POP_SIZE);

    pool_pop.Update();
    pool_pop.MutatePop();

  }

}
