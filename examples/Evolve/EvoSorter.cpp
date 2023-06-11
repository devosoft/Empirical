/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file EvoSorter.cpp
 *  @brief This file explores the evolving BitSorter sorting networks.
 */

#include <iostream>

#include "emp/config/ArgManager.hpp"
#include "emp/Evolve/World.hpp"
#include "emp/hardware/BitSorter.hpp"
#include "emp/math/Random.hpp"

EMP_BUILD_CONFIG( EvoSortConfig,
  GROUP(DEFAULT, "Default settings for EvoSorter model"),
  VALUE(SEED, int, 0, "Random number seed (0 for based on time)"),
  VALUE(POP_SIZE, uint32_t, 200, "Number of organisms in the popoulation."),
  VALUE(MAX_GENS, uint32_t, 2000, "How many generations should we process?"),
  VALUE(ORG_SIZE, size_t, 100, "Number of comparisons in an organism."),
  VALUE(MUT_SUB_PROB, double, 0.5, "What is the probability for a comparison to be randomized?"),
  VALUE(MUT_INS_PROB, double, 0.5, "What is the probability for a comparison to have a new one inserted after?"),
  VALUE(MUT_DEL_PROB, double, 0.5, "What is the probability for a comparison to be deleted?"),
)


using SorterOrg = emp::BitSorter;

void PrintOrg(size_t update, const SorterOrg & org)
{
  std::cout << (update+1) << " : " << org.AsString()
            << " : SolveCount=" << org.CountSortable()
            << "  Size=" << org.GetSize()
            << std::endl;
}

int main(int argc, char* argv[])
{
  EvoSortConfig config;
  config.Read("EvoSorter.cfg");

  auto args = emp::cl::ArgManager(argc, argv);
  if (args.ProcessConfigOptions(config, std::cout, "EvoSorter.cfg", "EvoSorter-macros.h") == false) exit(0);
  if (args.TestUnknown() == false) exit(0);  // If there are leftover args, throw an error.

  const uint32_t POP_SIZE = config.POP_SIZE();
  const uint32_t MAX_GENS = config.MAX_GENS();
  const uint32_t ORG_SIZE = config.ORG_SIZE();

  const double MUT_SUB_PROB = config.MUT_SUB_PROB();
  const double MUT_INS_PROB = config.MUT_INS_PROB();
  const double MUT_DEL_PROB = config.MUT_DEL_PROB();

  emp::Random random(config.SEED());

  emp::World<SorterOrg> pop(random, "SorterWorld");
  pop.SetupFitnessFile().SetTimingRepeat(10);
  // pop.SetupSystematicsFile().SetTimingRepeat(10);
  pop.SetupPopulationFile().SetTimingRepeat(10);
  pop.SetPopStruct_Mixed(true);
  pop.SetCache();

  // Build a random initial population
  for (uint32_t i = 0; i < POP_SIZE; i++) {
    SorterOrg next_org;
    for (size_t i = 0; i < ORG_SIZE; i++) {
      next_org.AddCompare(random.GetUInt(16), random.GetUInt(16));
    }
    pop.Inject(next_org);
  }

  // Setup the mutation function.
  std::function<size_t(SorterOrg &, emp::Random &)> mut_fun =
    [MUT_SUB_PROB,MUT_INS_PROB,MUT_DEL_PROB](SorterOrg & org, emp::Random & random) {
      size_t num_muts = 0;
      // Delete first (so as to not delete something we just changed or added)
      if (random.P(MUT_DEL_PROB)) {
        const uint32_t pos = random.GetUInt(org.GetSize());
        org.RemoveCompare(pos);
        num_muts++;
      }
      // Substitute before insert (to not change something just added)
      if (random.P(MUT_SUB_PROB)) {
        const uint32_t pos = random.GetUInt(org.GetSize());
        org.EditCompare(pos, random.GetUInt(16), random.GetUInt(16));
        num_muts++;
      }
      // Finally, do any insertions.
      if (random.P(MUT_INS_PROB)) {
        const uint32_t pos = random.GetUInt(org.GetSize());
        org.InsertCompare(pos, random.GetUInt(16), random.GetUInt(16));
        num_muts++;
      }

      return num_muts;
    };
  pop.SetMutFun( mut_fun );
  pop.SetAutoMutate(1);

  // Build the main fitness function.
  std::function<double(const SorterOrg&)> fit_fun =
    [](const SorterOrg & org){ return org.CountSortable() * 10 - org.GetSize(); };
  pop.SetFitFun( fit_fun );

  // Setup a place to put the set of fitness functions for lexicase.
  constexpr size_t num_fit_funs = 100;
  emp::vector< std::function<double(const SorterOrg &)> > fit_set(num_fit_funs);

  PrintOrg(0, pop[0]);

  // Loop through updates
  for (uint32_t ud = 0; ud < MAX_GENS; ud++) {

    // Build the lexicase fitness functions (changing each update)
    for (size_t i = 0; i < num_fit_funs; i++) {
      // Setup the fitness function.
      const size_t target_id = random.GetUInt(1<<16);
      fit_set[i] = [target_id](const SorterOrg & org) {
        return (double) org.TestSortable(target_id);
      };
    }


    // Keep the best individual.
    emp::EliteSelect(pop, 1, 1);

    // Run a tournament for the rest...
    // TournamentSelect(pop, 5, POP_SIZE-1);
    emp::LexicaseSelect<emp::BitSorter>(pop, fit_set, POP_SIZE-1);

    pop.Update();
    // std::cout << (ud+1) << " : " << pop[0].AsString() << " : " << pop[0].CountSortable() << std::endl;
    PrintOrg(ud+1, pop[0]);
  }

  // pop.PrintLineage(0);

  std::cout << MAX_GENS << " : " << pop[0].AsString() << " : " << pop[0].CountSortable() << std::endl;

  // pop.GetSignalControl().PrintNames();
}
