//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the evolving BitSorter sorting networks.

#include <iostream>

#include "config/ArgManager.h"
#include "Evolve/World.h"
#include "hardware/BitSorter.h"
#include "tools/Random.h"

EMP_BUILD_CONFIG( NKConfig,
  GROUP(DEFAULT, "Default settings for NK model"),
  VALUE(SEED, int, 0, "Random number seed (0 for based on time)"),
  VALUE(POP_SIZE, uint32_t, 1000, "Number of organisms in the popoulation."),
  VALUE(MAX_GENS, uint32_t, 2000, "How many generations should we process?"),
  VALUE(MUT_COUNT, uint32_t, 3, "How many bit positions should be randomized?"), ALIAS(NUM_MUTS),
  VALUE(TEST, std::string, "TestString", "This is a test string.")
)


using SorterOrg = emp::BitSorter;

int main(int argc, char* argv[])
{
  NKConfig config;
  config.Read("NK.cfg");

  auto args = emp::cl::ArgManager(argc, argv);
  if (args.ProcessConfigOptions(config, std::cout, "NK.cfg", "NK-macros.h") == false) exit(0);
  if (args.TestUnknown() == false) exit(0);  // If there are leftover args, throw an error.

  const uint32_t POP_SIZE = config.POP_SIZE();
  const uint32_t MAX_GENS = config.MAX_GENS();
  const uint32_t MUT_COUNT = config.MUT_COUNT();

  emp::Random random(config.SEED());

  // emp::EAWorld<SorterOrg, emp::FitCacheOff> pop(random, "NKWorld");
  emp::World<SorterOrg> pop(random, "SorterWorld");
  pop.SetupFitnessFile().SetTimingRepeat(10);
  pop.SetupSystematicsFile().SetTimingRepeat(10);
  pop.SetupPopulationFile().SetTimingRepeat(10);
  pop.SetPopStruct_Mixed(true);
  pop.SetCache();

  // Build a random initial population
  for (uint32_t i = 0; i < POP_SIZE; i++) {
    SorterOrg next_org;
    for (size_t i = 0; i < 60; i++) {
      next_org.AddCompare(random.GetUInt(16), random.GetUInt(16));
    }
    pop.Inject(next_org);
  }

  // Setup the mutation function.
  std::function<size_t(SorterOrg &, emp::Random &)> mut_fun =
    [MUT_COUNT](SorterOrg & org, emp::Random & random) {
      size_t num_muts = 0;
      for (uint32_t m = 0; m < MUT_COUNT; m++) {
        if (random.P(0.5)) {
          const uint32_t pos = random.GetUInt(org.GetSize());
          org.EditCompare(pos, random.GetUInt(16), random.GetUInt(16));
          num_muts++;
        }
      }
      return num_muts;
    };
  pop.SetMutFun( mut_fun );
  pop.SetAutoMutate();

  std::function<double(SorterOrg&)> fit_fun =
    [](SorterOrg & org){ return org.CountSortable(); };
  pop.SetFitFun( fit_fun );

  std::cout << 0 << " : " << pop[0].AsString() << " : " << fit_fun(pop[0]) << std::endl;

  // Loop through updates
  for (uint32_t ud = 0; ud < MAX_GENS; ud++) {
    // Print current state.
    // for (uint32_t i = 0; i < pop.GetSize(); i++) std::cout << pop[i] << std::endl;
    // std::cout << std::endl;

    // Keep the best individual.
    emp::EliteSelect(pop, 1, 1);

    // Run a tournament for the rest...
    TournamentSelect(pop, 5, POP_SIZE-1);
    pop.Update();
    std::cout << (ud+1) << " : " << pop[0].AsString() << " : " << fit_fun(pop[0]) << std::endl;
  }

  // pop.PrintLineage(0);

//  std::cout << MAX_GENS << " : " << pop[0].AsString() << " : " << fit_fun(pop[0]) << std::endl;

  // pop.GetSignalControl().PrintNames();
}
