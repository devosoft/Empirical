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
#include "../../evo/Stats.h"

// k controls # of hills in the fitness landscape
constexpr int K = 0;
constexpr int N = 100;
constexpr int WIDTH = 2;

constexpr int TOURNAMENT_SIZE = 10;
constexpr int POP_SIZE = 10;
constexpr int UD_COUNT = 2000;

using BitOrg = emp::BitVector;

template <typename ORG, typename... MANAGERS>
using MixedWorld = emp::evo::World<ORG, MANAGERS..., emp::evo::PopulationManager_Base<ORG>>;

int main()
{
  emp::Random random;
  emp::evo::NKLandscape landscape(N, K, random);

  MixedWorld<BitOrg> mixed_pop(random);

  std::function<double(BitOrg *)> fit_func =[&landscape](BitOrg * org) { return landscape.GetFitness(*org);};

  // Build a random initial population
  for (int i = 0; i < POP_SIZE; i++) {
    BitOrg next_org(N);
    for (int j = 0; j < N; j++) next_org[j] = random.P(0.5);
    
    // looking at the Insert() func it looks like it does a deep copy, so we should be safe in
    // doing this. Theoretically...
    mixed_pop.Insert(next_org);
  }

  mixed_pop.SetDefaultMutateFun( [](BitOrg* org, emp::Random& random) {
      (*org)[random.GetInt(N)] = random.P(0.5);
      //(*org)[random.GetInt(N)] = random.P(0.5);
      //(*org)[random.GetInt(N)] = random.P(0.5);
      return true;
    } );

  // Loop through updates
  std::cout << "Update,ShannonDiversity,MaxFitness" << std::endl;
  for (int ud = 0; ud < UD_COUNT; ud++) {

    // output allll of the population
    //std::cerr << "Update " << ud << std::endl;
    //for (int i = 0; i < POP_SIZE; i++) {std::cerr << "\t" << mixed_pop[i] << std::endl;}

    // handle tick for mixed_world
      
    // Print current state.
    // for (int i = 0; i < pop.GetSize(); i++) std::cout << pop[i] << std::endl;
    // std::cout << std::endl;
    std::cout << ud  << "," << emp::evo::ShannonDiversity(mixed_pop);
    std::cout << "," << emp::evo::MaxFitness(fit_func, mixed_pop.popM.pop) << std::endl;

    // Keep the best individual.
    mixed_pop.EliteSelect([&landscape](BitOrg * org){ return landscape.GetFitness(*org); }, 1);

    // Run a tournament for the rest...
    mixed_pop.TournamentSelect([&landscape](BitOrg * org){ return landscape.GetFitness(*org); }
			 , TOURNAMENT_SIZE, POP_SIZE-1);
    
    mixed_pop.Update();
    mixed_pop.MutatePop();

  }

}
