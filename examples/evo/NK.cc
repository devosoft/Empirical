//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the template defined in evo::Population.h

#include <iostream>

#include "../../config/ArgManager.h"
#include "../../evo/NK.h"
#include "../../evo/World.h"
#include "../../tools/BitSet.h"
#include "../../tools/Random.h"

EMP_BUILD_CONFIG( NKConfig,
  GROUP(DEFAULT, "Default settings for NK model"),
  VALUE(K, int, 10, "Level of epistasis in the NK model"),
  VALUE(N, int, 50, "Number of bits in each organisms (must be > K)"), ALIAS(GENOME_SIZE),
  VALUE(SEED, int, 0, "Random number seed (0 for based on time)"),
  CONST(POP_SIZE, int, 1000, "Number of organisms in the popoulation."),
  VALUE(MAX_GENS, int, 2000, "How many generations should we process?"),
  VALUE(MUT_COUNT, int, 3, "How many bit positions should be randomized?"), ALIAS(NUM_MUTS),
)


using BitOrg = emp::BitVector;

int main(int argc, char* argv[])
{
  NKConfig config;
  config.Read("NK.cfg");

  auto args = emp::cl::ArgManager(argc, argv);
  args.UseArg("-N", config, "N", "Set the number of bits in each sequence.");
  args.UseArg("-K", config, "K", "Set the number of extra bits affecting each fitness contribution.");
  args.UseArg("-seed", config, "SEED", "Set the number of sites randomized with each birth");
  args.UseArg("-pop", config, "POP_SIZE", "Set the population size.");
  args.UseArg("-gens", config, "MAX_GENS", "Set the number of generations to run.");
  args.UseArg("-mut", config, "MUT_COUNT", "Set the number of sites randomized with each birth");
  bool print_help = args.UseFlag("-h", "Print help information.");
  bool create_config = args.UseFlag("-gen", "Generate configuration file.");
  bool const_macros = args.UseFlag("-const", "Generate const version of macros file.");

  // If there are leftover args, throw an error.
  if (args.size() > 1) {
    std::cerr << "Unknown args:";
    for (int i = 1; i < args.size(); i++) std::cout << " " << args[i];
    std::cerr << std::endl;
    print_help = true;
  }

  if (print_help) { args.PrintHelp(std::cout); exit(0); }
  if (create_config) { config.Write("NK.cfg"); exit(0); }
  if (const_macros) { config.WriteMacros("NK-macros.h", true); exit(0); }

  const int N = config.N();
  const int K = config.K();
  const int POP_SIZE = config.POP_SIZE();
  const int MAX_GENS = config.MAX_GENS();
  const int MUT_COUNT = config.MUT_COUNT();

  emp::Random random(config.SEED());
  emp::evo::NKLandscape landscape(N, K, random);
  emp::evo::EAWorld<BitOrg> pop(random);

  // Build a random initial population
  for (int i = 0; i < config.POP_SIZE(); i++) {
    BitOrg next_org(N);
    for (int j = 0; j < N; j++) next_org[j] = random.P(0.5);
    pop.Insert(next_org);
  }

  pop.SetDefaultMutateFun( [MUT_COUNT, N](BitOrg* org, emp::Random& random) {
      for (int m = 0; m < MUT_COUNT; m++) {
        const int pos = random.GetInt(N);
        (*org)[pos] = random.P(0.5);
      }
      return true;
    } );


  // Loop through updates
  for (int ud = 0; ud < MAX_GENS; ud++) {
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


  std::cout << MAX_GENS << " : " << pop[0] << " : " << landscape.GetFitness(pop[0]) << std::endl;
}
