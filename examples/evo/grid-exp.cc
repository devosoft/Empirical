//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
// 
// Author: Steven Jorgensen
//
//  This file explores the template defined in evo::Population.h

#include <iostream>
#include <string>
#include <cmath>
#include <fstream>

#include "../../config/ArgManager.h"
#include "../../evo/NK.h"
#include "../../evo/World.h"
#include "../../tools/BitSet.h"
#include "../../tools/Random.h"
#include "../../evo/EvoStats.h"
#include "../../evo/StatsManager.h"
#include "../../evo/LineageTracker.h"

// Default Config Options for the Population
EMP_BUILD_CONFIG( NKConfig,
  GROUP(DEFAULT, "Default settings for NK model"),
  VALUE(K, int, 0, "Level of epistasis in the NK model"),
  VALUE(N, int, 100, "Number of bits in each organisms (must be > K)"), ALIAS(GENOME_SIZE),
  VALUE(SEED, int, 123, "Random number seed (0 for based on time)"),
  VALUE(POP_SIZE, int, 3025, "Number of organisms in the popoulation."),
  VALUE(MAX_GENS, int, 10000, "How many generations should we process?"),
  VALUE(MUT_COUNT, double, 0.0001, "How many bit positions should be randomized?"), ALIAS(NUM_MUTS),
  VALUE(TOUR_SIZE, int, 20, "How many organisms should be picked in each Tournament?"),
  VALUE(NAME, std::string, "Result-", "Name of file printed to"),
)

using BitOrg = emp::BitVector;

template <typename ORG>
using GridWorld = emp::evo::World<ORG, emp::evo::StatsManager_AdvancedStats<emp::evo::PopulationManager_Grid<BitOrg> >,
      emp::evo::PopulationManager_Grid<ORG>, emp::evo::LineagePruned >;




int main(int argc, char* argv[])
{
  NKConfig config;
  config.Read("Grid.cfg");

  auto args = emp::cl::ArgManager(argc, argv);
  if (args.ProcessConfigOptions(config, std::cout, "Grid.cfg", "NK-macros.h") == false) exit(0);
  if (args.TestUnknown() == false) exit(0);  // If there are leftover args, throw an error.

  config.Write("SetGrid.cfg");

  // K controls # of hills in the fitness landscape
  // N controls the length of the genomes
  const int K = config.K();
  const int N = config.N();
  const double MUTATION_RATE = config.MUT_COUNT();
  const int TOURNAMENT_SIZE = config.TOUR_SIZE();
  const int POP_SIZE = config.POP_SIZE();
  const int UD_COUNT = config.MAX_GENS();
  std::string prefix;
  prefix = config.NAME();
  emp::Random random(config.SEED());
  emp::evo::NKLandscape landscape(N, K, random);
  bool competitive = 1; // Sets population to use competitive selection


  // Create the world and set it up
  GridWorld<BitOrg> grid_pop(random);
  grid_pop.ConfigPop(std::sqrt(POP_SIZE), std::sqrt(POP_SIZE));
  std::function<double(BitOrg *)> fit_func =[&landscape](BitOrg * org) { return landscape.GetFitness(*org);};

  //Configure the world and the statsmanager output
  grid_pop.SetDefaultFitnessFun(fit_func);
  grid_pop.statsM.SetOutput(prefix + "grid.csv");

  // Insert default organisms into world
  for (int i = 0; i < POP_SIZE; i++)
  {
    BitOrg next_org(N);

    for (int j = 0; j < N; j++) next_org[j] = random.P(0.5);

    grid_pop.Insert(next_org);
  }


  // mutation function:
  // for every site in the gnome there is a MUTATION_RATE chance that the 
  // site will flip it's value.
  grid_pop.SetDefaultMutateFun( [MUTATION_RATE, N](BitOrg* org, emp::Random& random) 
    {
      bool mutated = false;    
        for (size_t site = 0; site < N; site++)
        {
          if (random.P(MUTATION_RATE))
          {
            (*org)[site] = !(*org)[site];
            mutated = true;
          }
        }
        return mutated;
    } );


  // Loop through updates
  for (int ud = 0; ud < UD_COUNT; ud++)
  {

    grid_pop.TournamentSelect([&landscape](BitOrg * org){ return landscape.GetFitness(*org); }
            , TOURNAMENT_SIZE, POP_SIZE, competitive);

    grid_pop.Update();
    grid_pop.MutatePop();
  }

  std::ofstream myfile;
  myfile.open("BEN_MUT_Grid.csv");

  for (auto org : grid_pop){ 
      auto test = *org;
      double fitness = fit_func(org);

      for (int i = 0; i < org->size(); i++) {
          test[i] = !test[i];
          double fit_num = fit_func(&test);
          if (fit_num > fitness) {
              myfile << fit_num - fitness<<",";
          }
          test[i] = !test[i];
      }
      myfile<<"\n";
  }
  myfile.close();

}
