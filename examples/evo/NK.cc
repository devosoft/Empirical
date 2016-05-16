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
#include "../../tools/command_line.h"
#include "../../tools/Random.h"

int K = 13;
int N = 50;

int POP_SIZE = 1000;
int UD_COUNT = 2000;

using BitOrg = emp::BitVector;

int main(int argc, char* argv[])
{
  auto args = emp::cl::args_to_strings(argc, argv);
  emp::cl::use_arg_value(args, "-N", N);
  emp::cl::use_arg_value(args, "-K", K);
  emp::cl::use_arg_value(args, "-pop", POP_SIZE);
  emp::cl::use_arg_value(args, "-ud", UD_COUNT);

  emp::Random random;
  emp::evo::NKLandscape landscape(N, K, random);
  emp::evo::EAWorld<BitOrg> pop(random);

  // Build a random initial population
  for (int i = 0; i < POP_SIZE; i++) {
    BitOrg next_org(N);
    for (int j = 0; j < N; j++) next_org[j] = random.P(0.5);
    pop.Insert(next_org);
  }

  pop.SetDefaultMutateFun( [](BitOrg* org, emp::Random& random) {
      (*org)[random.GetInt(N)] = random.P(0.5);
      (*org)[random.GetInt(N)] = random.P(0.5);
      (*org)[random.GetInt(N)] = random.P(0.5);
      return true;
    } );


  // Loop through updates
  for (int ud = 0; ud < UD_COUNT; ud++) {
    // Print current state.
    // for (int i = 0; i < pop.GetSize(); i++) std::cout << pop[i] << std::endl;
    // std::cout << std::endl;
    std::cout << ud << " : " << pop[0] << " : " << landscape.GetFitness(pop[0]) << std::endl;

    // Keep the best individual.
    pop.EliteSelect([&landscape](BitOrg * org){ return landscape.GetFitness(*org); }, 1);

    // Run a tournament for the rest...
    pop.TournamentSelect([&landscape](BitOrg * org){ return landscape.GetFitness(*org); }
			 , 5, POP_SIZE-1);
    pop.Update();
    pop.MutatePop();
  }


  std::cout << UD_COUNT << " : " << pop[0] << " : " << landscape.GetFitness(pop[0]) << std::endl;
}
