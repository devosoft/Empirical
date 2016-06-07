//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the template defined in evo::Population.h

#include <iostream>
#include <string>

#include "../../evo/NK.h"
#include "../../tools/BitSet.h"
#include "../../tools/Random.h"
#include "../../evo/EvoStats.h"
#include "../../evo/StatsManager.h"
#include "../../evo/World.h"

// k controls # of hills in the fitness landscape
constexpr int K = 0;
constexpr int N = 10;
constexpr double MUTATION_RATE = 0.005;

constexpr int TOURNAMENT_SIZE = 2;
constexpr int POP_SIZE = 10;
constexpr int UD_COUNT = 1000;

using BitOrg = emp::BitVector;

template <typename ORG>
using MixedWorld = emp::evo::World<ORG, emp::evo::PopulationManager_Base<ORG>, emp::evo::LineagePruned >;

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
    std::cerr << "** Usage: ./replica output-prefix";
  }

//  MixedWorld<BitOrg> mixed_pop(random);
//  emp::evo::EAWorld<BitOrg> mixed_pop(random);
  MixedWorld<BitOrg> mixed_pop(random);
  emp::evo::GridWorld<BitOrg, emp::evo::LineagePruned > grid_pop(random);

  std::function<double(BitOrg *)> fit_func =[&landscape](BitOrg * org) { return landscape.GetFitness(*org);};

  mixed_pop.SetDefaultFitnessFun(fit_func);
  grid_pop.SetDefaultFitnessFun(fit_func);

  grid_pop.ConfigPop(10,1);
  //std::cout<<"HELLP"<<std::endl;

  // make a couple stats managers
  emp::evo::StatsManager_AdvancedStats<emp::evo::PopulationManager_Base<BitOrg>>
      mixed_stats (&mixed_pop, prefix + "mixed.csv");
  emp::evo::StatsManager_AdvancedStats<emp::evo::PopulationManager_Grid<BitOrg>>
      grid_stats (&grid_pop, prefix + "grid.csv");

  mixed_stats.SetDefaultFitnessFun(fit_func);
  grid_stats.SetDefaultFitnessFun(fit_func);


  //mixed_stats.fit_fun = fit_func;
  //grid_stats.fit_fun = fit_func;

  // Build a random initial population
  /*
  BitOrg next_orgA(N);
  for(int j = 0; j < N; j++) next_orgA[j] = 0;
  BitOrg next_orgB(N);
  for(int j = 0; j < N; j++) next_orgB[j] = 1;
  BitOrg next_orgC(N);
  for(int j = 0; j < N; j++){
      if(j % 2 == 0){next_orgC[j] = 0;}
      else{next_orgC[j] = 1;}
  }
  mixed_pop.Insert(next_orgA);
  mixed_pop.Insert(next_orgB);
  mixed_pop.Insert(next_orgC);

  grid_pop.Insert(next_orgA);
  grid_pop.Insert(next_orgB);
  grid_pop.Insert(next_orgC);

  */
  std::cout<<"WHYME"<<std::endl;
  for (int i = 0; i < POP_SIZE; i++) {
    BitOrg next_org(N);
    for (int j = 0; j < N; j++) next_org[j] = random.P(0.5);


    // looking at the Insert() func it looks like it does a deep copy, so we should be safe in
    // doing this. Theoretically...
    mixed_pop.Insert(next_org);
    grid_pop.Insert(next_org);
  }

  // mutation function:
  // for every site in the gnome there is a MUTATION_RATE chance that the
  // site will flip it's value.
  mixed_pop.SetDefaultMutateFun( [](BitOrg* org, emp::Random& random) {
    bool mutated = false;
      for (size_t site = 0; site < N; site++) {
        if (random.P(MUTATION_RATE)) {
          (*org)[site] = !(*org)[site];
          mutated = true;
//          std::cerr << "ZergRush" << std::endl;
        }
      }
      return mutated;
    } );

  // make them have the same mutation func
  grid_pop.SetDefaultMutateFun( [](BitOrg* org, emp::Random& random) {
    bool mutated = false;
      for (size_t site = 0; site < N; site++) {
        if (random.P(MUTATION_RATE)) {
          (*org)[site] = !(*org)[site];
          mutated = true;
//          std::cerr << "ZergRush" << std::endl;
        }
      }
      return mutated;
    } );

  std::cout<<"LLLLLLLLLLLLLLLLL"<<std::endl;
  // Loop through updates
  //std::cout << "Update,ShannonDiversity,MaxFitness,AvgFitness" << std::endl;
  for (int ud = 0; ud < UD_COUNT; ud++) {

//    std::cout << ud  << "," << emp::evo::ShannonDiversity(mixed_pop);
//    std::cout << "," << emp::evo::MaxFitness(fit_func, mixed_pop);
//    std::cout << "," << emp::evo::AverageFitness(fit_func, mixed_pop) << std::endl;

    // Keep the best individual.
    //    mixed_pop.EliteSelect([&landscape](BitOrg * org){ return landscape.GetFitness(*org); }, 5, 10);

    // Run a tournament for the rest...

    mixed_pop.TournamentSelect([&landscape](BitOrg * org){ return landscape.GetFitness(*org); }
			 , TOURNAMENT_SIZE, POP_SIZE);
    grid_pop.TournamentSelect([&landscape](BitOrg * org){ return landscape.GetFitness(*org); }
			 , TOURNAMENT_SIZE, POP_SIZE);

    std::cout<<"UPDATE________"<<std::endl;

    grid_pop.Update();
    std::cout<<"BETWEEN"<<std::endl;
    grid_pop.MutatePop();

    std::cout<<"GRID DONE"<<std::endl;
    mixed_pop.Update();
    mixed_pop.MutatePop();
    //MLandscape data = MutLandscape(fit_func, grid_pop);
    //std::cout<<"Max Benefitial:  "<<data.max_ben<<std::endl;
    //std::cout<<"Max Detrimental:  "<<data.max_det<<std::endl;

    //mixed_pop.Update();
    //grid_pop.Update();
  }

}
