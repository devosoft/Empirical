//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the template defined in evo::Population.h

#include <iostream>
#include <string>

#include "../../evo/NK.h"
#include "../../evo/World.h"
#include "../../tools/BitSet.h"
#include "../../tools/Random.h"
#include "../../evo/EvoStats.h"
#include "../../evo/StatsManager.h"

// k controls # of hills in the fitness landscape
constexpr int K = 0;
constexpr int N = 10;
constexpr double MUTATION_RATE = 0.005;

constexpr int TOURNAMENT_SIZE = 20;
constexpr int POP_SIZE = 300;
constexpr int UD_COUNT = 1000;

using BitOrg = emp::BitVector;

int main(int argc, char* argv[])
{
  emp::Random random;
  emp::evo::NKLandscape landscape(N, K, random);

  std::string prefix;


  if (argc == 1) { // program name, no prefix
    prefix = "";
  }
  else if (argc == 2) { // has prefix
    prefix = std::string(argv[1]) + "-";
  }
  else {
    std::cerr << "** Usage: ./grid-exp output-prefix";
  }


  emp::evo::GridWorld<BitOrg> grid_pop(random);

  std::function<double(BitOrg *)> fit_func =[&landscape](BitOrg * org) { return landscape.GetFitness(*org);};

  grid_pop.SetDefaultFitnessFun(fit_func);

  // make a stats manager
  emp::evo::StatsManager_DefaultStats<emp::evo::PopulationManager_Grid<BitOrg>> 
      grid_stats (&grid_pop, prefix + "grid.csv");

  grid_stats.SetDefaultFitnessFun(fit_func);

  // Build a random initial population
  
  for (int i = 0; i < POP_SIZE; i++) {
    BitOrg next_org(N);
    for (int j = 0; j < N; j++) next_org[j] = random.P(0.5);
    

    // looking at the Insert() func it looks like it does a deep copy, so we should be safe in
    // doing this. Theoretically...
    grid_pop.Insert(next_org);
  }

  // mutation function:
  // for every site in the gnome there is a MUTATION_RATE chance that the 
  // site will flip it's value.
  grid_pop.SetDefaultMutateFun( [](BitOrg* org, emp::Random& random) {
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
    //    grid_pop.EliteSelect([&landscape](BitOrg * org){ return landscape.GetFitness(*org); }, 5, 10);

    // Run a tournament for the rest...   
    grid_pop.TournamentSelect([&landscape](BitOrg * org){ return landscape.GetFitness(*org); }
			 , TOURNAMENT_SIZE, POP_SIZE);

    grid_pop.Update();
    grid_pop.MutatePop();

  }

}
