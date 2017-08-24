//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the template defined in evo::Population.h with an NK landscape.

#include <iostream>

#include "../../config/ArgManager.h"
#include "../../evo/NK.h"
#include "../../evo/World.h"
#include "../../tools/BitSet.h"
#include "../../tools/Random.h"

EMP_BUILD_CONFIG( NKConfig,
  GROUP(DEFAULT, "Default settings for NK model"),
  CONST(K, uint32_t, 10, "Level of epistasis in the NK model"),
  CONST(N, uint32_t, 200, "Number of bits in each organisms (must be > K)"), ALIAS(GENOME_SIZE),
  CONST(SEED, int, 1, "Random number seed (0 for based on time)"),
  CONST(POP_SIZE, uint32_t, 1000, "Number of organisms in the popoulation."),
  CONST(MAX_GENS, uint32_t, 500, "How many generations should we process?"),
  CONST(MUT_COUNT, uint32_t, 3, "How many bit positions should be randomized?"), ALIAS(NUM_MUTS),
  VALUE(TEST, std::string, "TestString", "This is a test string.")
)


using BitOrg = emp::BitVector;

int main(int argc, char* argv[])
{
  NKConfig config;
  config.Read("NK.cfg");

  auto args = emp::cl::ArgManager(argc, argv);
  if (args.ProcessConfigOptions(config, std::cout, "NK.cfg", "NK-macros.h") == false) exit(0);
  if (args.TestUnknown() == false) exit(0);  // If there are leftover args, throw an error.

  constexpr uint32_t N = config.N();
  constexpr uint32_t K = config.K();
  constexpr uint32_t POP_SIZE = config.POP_SIZE();
  constexpr uint32_t MAX_GENS = config.MAX_GENS();
  constexpr uint32_t MUT_COUNT = config.MUT_COUNT();

  emp::Random random(config.SEED());
  emp::evo::NKLandscape landscape(N, K, random);
  emp::evo::EAWorld<BitOrg> pop(random, "NKWorld");

  // Build a random initial population
  for (uint32_t i = 0; i < config.POP_SIZE(); i++) {
    BitOrg next_org(N);
    for (uint32_t j = 0; j < N; j++) next_org[j] = random.P(0.5);
    pop.Insert(next_org);
  }

  pop.SetDefaultMutateFun( [MUT_COUNT, N](BitOrg* org, emp::Random& random) {
      for (uint32_t m = 0; m < MUT_COUNT; m++) {
        const uint32_t pos = random.GetUInt(N);
        (*org)[pos] = random.P(0.5);
      }
      return true;
    } );


  // emp::LinkSignal("NKWorld::org-placement", std::function<void()>([](){ std::cout << "Placed." << std::endl; }) );

  // Loop through updates
  for (uint32_t ud = 0; ud < MAX_GENS; ud++) {
    // Print current state.
    // for (uint32_t i = 0; i < pop.GetSize(); i++) std::cout << pop[i] << std::endl;
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


  std::cout << MAX_GENS << " : " << pop[0] << " : " << landscape.GetFitness(pop[0]) << std::endl;

  pop.GetSignalControl().PrintNames();
}
