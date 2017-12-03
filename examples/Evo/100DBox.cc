/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 */

#include <iostream>

#include "base/vector.h"
#include "tools/Random.h"
#include "Evo/World.h"
#include "Evo/Resource.h"
#include "config/ArgManager.h"

EMP_BUILD_CONFIG( BoxConfig,
  GROUP(DEFAULT, "Default settings for box experiment"),
  VALUE(SEED, int, 0, "Random number seed (0 for based on time)"),
  VALUE(POP_SIZE, uint32_t, 2000, "Number of organisms in the popoulation."),
  VALUE(UPDATES, uint32_t, 10000, "How many generations should we process?"),
  VALUE(SELECTION, std::string, "TOURNAMENT", "What selection scheme should we use?"),
  VALUE(N_NEUTRAL, int, 0, "Number of neutral fitness functions"),
  VALUE(N_GOOD, int, 0, "Number of good fitness functions"),
  VALUE(N_BAD, int, 0, "Number of bad fitness functions"),
  VALUE(DISTANCE_CUTOFF, double, 0, "How close to origin does fitness gradient start"),
  VALUE(RESOURCE_INFLOW, double, 100, "How much resource enters the world each update"),
  VALUE(PROBLEM_DIMENSIONS, int, 10, "How many axes does the box have?"),
  VALUE(RECOMBINATION, int, 0, "Does recombination happen?")
)

int main(int argc, char* argv[])
{
  BoxConfig config;
  config.Read("BoxConfig.cfg");

  auto args = emp::cl::ArgManager(argc, argv);
  if (args.ProcessConfigOptions(config, std::cout, "BoxConfig.cfg", "Box-macros.h") == false) exit(0);
  if (args.TestUnknown() == false) exit(0);  // If there are leftover args, throw an error.

  const uint32_t POP_SIZE = config.POP_SIZE();
  const uint32_t UPDATES = config.UPDATES();
  const int N_NEUTRAL = config.N_NEUTRAL();
  const int N_GOOD = config.N_GOOD();
  const int N_BAD = config.N_BAD();
  const int PROBLEM_DIMENSIONS = config.PROBLEM_DIMENSIONS();
  const double DISTANCE_CUTOFF = config.DISTANCE_CUTOFF();
  const double RESOURCE_INFLOW = config.RESOURCE_INFLOW();
  const std::string SELECTION = config.SELECTION();
  const bool RECOMBINATION = config.RECOMBINATION();

  const int GENOME_SIZE = PROBLEM_DIMENSIONS + N_NEUTRAL;

  using ORG_TYPE = emp::vector<double>;

  emp::Random random(config.SEED());
  emp::World<ORG_TYPE> world(random, "BoxWorld");
  world.SetWellMixed(true);
  world.SystematicsOff();

  emp::vector<emp::Resource> resources;
  for (int i=0; i<GENOME_SIZE; i++) {
      resources.push_back(emp::Resource(RESOURCE_INFLOW, RESOURCE_INFLOW, .01));
  }

  world.OnUpdate([&resources](int ud){
      for (emp::Resource& res : resources) {
          res.Update();
      }
  });

  // Build a random initial popoulation.
  for (size_t i = 0; i < POP_SIZE; i++) {
    ORG_TYPE org(GENOME_SIZE);
    for (int i = 0; i < GENOME_SIZE; i++) {
        org[i] = random.GetDouble(1);
    }
    world.Inject(org);
  }

  // Setup the mutation function.
  world.SetMutFun( [GENOME_SIZE, RECOMBINATION, &random, &world](ORG_TYPE & org, emp::Random & random) {
    //   uint32_t num_muts = random.GetUInt(4);  // 0 to 3 mutations.
      for (uint32_t pos = 0; pos < GENOME_SIZE; pos++) {
        org[pos] += random.GetRandNormal(0, .01);
        if (org[pos] < 0) {
            org[pos] = 0;
        } else if (org[pos] > 1) {
            org[pos] = 1;
        }
      }

      if (RECOMBINATION) {
          int crossover_point = random.GetUInt(GENOME_SIZE);
          const ORG_TYPE& parent2 = world.GetRandomOrg();

          if (random.P(.5) > .5) {
              for (int i = crossover_point; i < GENOME_SIZE; i++) {
                  org[i] = parent2[i];
              }
          } else {
              for (int i = 0; i < crossover_point; i++) {
                  org[i] = parent2[i];
              }
          }
      }

      return GENOME_SIZE;
    } );

  std::function<double(const ORG_TYPE&)> goal_function = [PROBLEM_DIMENSIONS, DISTANCE_CUTOFF](const ORG_TYPE & org){
    double dist = 0;
    for (int i = 0; i < PROBLEM_DIMENSIONS; i++) {
        dist += emp::Pow(org[i], 2.0);
    }

    dist = sqrt(dist);
    if (dist > DISTANCE_CUTOFF) {
        return 0.00;
    }

    return 1.0/dist;
};

  world.SetFitFun(goal_function);

  world.SetupFitnessFile();

  emp::vector< std::function<double(const ORG_TYPE &)> > fit_set;

  // Good hints
  for (int i = 0; i < N_GOOD; i++) {
      fit_set.push_back([i](const ORG_TYPE & org){return 1 - org[i];});
  }

  // Bad hints
  for (int i = N_GOOD; i < N_GOOD + N_BAD; i++) {
      fit_set.push_back([i](const ORG_TYPE & org){return org[i];});
  }

  // Neutral hints (these axes aren't evaluated)
  for (int i = PROBLEM_DIMENSIONS; i < GENOME_SIZE; i++) {
      fit_set.push_back([i](const ORG_TYPE & org){return org[i];});
  }

  if (SELECTION == "LEXICASE") {
      fit_set.push_back(goal_function);
  } else if (SELECTION == "TOURNAMENT") {
      world.SetCache(true);
  }
  // Do the run...
  for (size_t ud = 0; ud < UPDATES; ud++) {
    // Update the status of all organisms.
    // double fit0 = world.CalcFitnessID(0);
    // std::cout << (ud+1) << " : " << emp::to_string(world[0]) << " : " << fit0 << std::endl << std::endl;;

    // Keep the best individual.
    // EliteSelect(world, 1, 1);

    // Run a tournament for the rest...

    if (SELECTION == "TOURNAMENT") {
        TournamentSelect(world, 20, POP_SIZE);
    } else if (SELECTION == "LEXICASE") {
        LexicaseSelect(world, fit_set, POP_SIZE);
    } else if (SELECTION == "RESOURCE") {
        ResourceSelect(world, fit_set, resources, 20, POP_SIZE);
    } else {
        std::cout << "ERROR: INVALID SELECTION SCHEME: " << SELECTION << std::endl;
        exit(1);
    }
    world.Update();
    if (isinf(world.GetFitnessDataNode().GetMax())){
        return 0;
    }
    // Mutate all but the first organism.
    world.DoMutations(1);
    for (auto res : resources) {
        std::cout << res.GetAmount() << " ";
    }
    std::cout << std::endl;
  }

  // std::cout << std::endl;
  //
  // std::cout << emp::to_string(world[0]) << " " << goal_function(world[0]) << "  ";
  //
  // std::cout << std::endl;

  return 0;
}
