//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the template defined in evo::Population.h


//#include "../../emtools/init.h"
//#include "../../emtools/JSWrap.h"
//#include "../../../d3-emscripten/selection.h"

#include <iostream>
#include <functional>

#include "../../evo/NK-const.h"
#include "../../evo/OEE.h"
#include "../../evo/World.h"
#include "../../tools/BitSet.h"
#include "../../tools/Random.h"
#include "../../evo/StatsManager.h"


constexpr int K = 3;
constexpr int N = 20;

constexpr int POP_SIZE = 200;
constexpr int UD_COUNT = 500;
constexpr int TOURNAMENT_SIZE = 15;

using BitOrg = emp::BitSet<N>;

int main()
{
  std::cout << "N: " << N << ", K: " << K << ", POP_SIZE: " << POP_SIZE << ", Selection: " << "Standard_tournament" << ", TournamentSize: " << TOURNAMENT_SIZE << std::endl;
  emp::Random random;
  emp::evo::NKLandscapeConst<N,K> landscape(random);
  emp::evo::World<BitOrg, emp::evo::OEEStats > pop(random);

  //#ifdef EMSCRIPTEN
  //std::cout << n_objects() << std::endl;
  //D3::Selection svg = D3::Selection("body").Append("svg");
  //#endif

  // Build a random initial population
  for (int i = 0; i < POP_SIZE; i++) {
    BitOrg next_org;
    for (int j = 0; j < N; j++) next_org[j] = random.P(0.5);
    pop.Insert(next_org);
  }

  pop.SetDefaultMutateFun( [](BitOrg* org, emp::Random& random) {
      (*org)[random.GetInt(N)] = random.P(0.5);
      (*org)[random.GetInt(N)] = random.P(0.5);
      (*org)[random.GetInt(N)] = random.P(0.5);
      return true;
    } );


pop.SetDefaultFitnessFun([&landscape](BitOrg * org){ return landscape.GetFitness(*org); });

  // Loop through updates
  for (int ud = 0; ud < UD_COUNT+1; ud++) {
    // Print current state.
    // for (int i = 0; i < pop.GetSize(); i++) std::cout << pop[i] << std::endl;
    // std::cout << std::endl;
    //std::cout << ud << " : " << pop[0] << " : " << landscape.GetFitness(pop[0]) << std::endl;

    // Run a tournament for the rest...
    pop.TournamentSelect([&landscape](BitOrg * org){ return landscape.GetFitness(*org); }
    , TOURNAMENT_SIZE, POP_SIZE);
    //pop.FitnessSharingTournamentSelect([landscape](BitOrg * org){ return landscape.GetFitness(*org); }, [](BitOrg* org1, BitOrg* org2){ return (double)(org1->XOR(*org2)).CountOnes();}, 10, 1, TOURNAMENT_SIZE, POP_SIZE);
    pop.Update();
    pop.MutatePop();
  }


  //std::cout << UD_COUNT << " : " << pop[0] << " : " << landscape.GetFitness(pop[0]) << std::endl;
}
