//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
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
  VALUE(K, uint32_t, 10, "Level of epistasis in the NK model"),
  VALUE(N, uint32_t, 200, "Number of bits in each organisms (must be > K)"), ALIAS(GENOME_SIZE),
  VALUE(SEED, int, 0, "Random number seed (0 for based on time)"),
  VALUE(POP_SIZE, uint32_t, 1000, "Number of organisms in the popoulation."),
  VALUE(MAX_GENS, uint32_t, 2000, "How many generations should we process?"),
  VALUE(MUT_COUNT, uint32_t, 3, "How many bit positions should be randomized?"), ALIAS(NUM_MUTS),
  VALUE(NUM_LANDSCAPES, uint32_t, 5, "How many NK Landscapes should be run in parallel?")
)


using BitOrg = emp::BitVector;

uint32_t NUM_LANDSCAPES;
emp::vector<std::function<double(BitOrg*)>> fit_funs;

void PrintPop(emp::evo::EAWorld<BitOrg, emp::evo::FitCacheOff> & pop, std::string filename)
{
  std::ofstream of(filename);

  // Print out the whole population:
  for (size_t i = 0; i < pop.GetSize(); i++) {
    of << i;  // << " : " << pop[i] << " : ";
    for (size_t j = 0; j <= NUM_LANDSCAPES; j++) {
      of << ", " << fit_funs[j](&(pop[i]));
    }
    of << std::endl;
  }

  
}


int main(int argc, char* argv[])
{
  NKConfig config;
  config.Read("MultiNK.cfg");

  auto args = emp::cl::ArgManager(argc, argv);
  if (args.ProcessConfigOptions(config, std::cout, "MultiNK.cfg", "Lexicase-macros.h") == false) {
    exit(0);
  }
  if (args.TestUnknown() == false) exit(0);  // If there are leftover args, throw an error.

  const uint32_t N = config.N();
  const uint32_t K = config.K();
  const uint32_t POP_SIZE = config.POP_SIZE();
  const uint32_t MAX_GENS = config.MAX_GENS();
  const uint32_t MUT_COUNT = config.MUT_COUNT();
  NUM_LANDSCAPES = config.NUM_LANDSCAPES();

  emp::Random random(config.SEED());

  // Build all of the landscapes that we need.
  emp::vector<emp::evo::NKLandscape> landscapes;
  for (uint32_t i = 0; i < NUM_LANDSCAPES; i++) {
    landscapes.emplace_back(N, K, random);
  }

  emp::evo::EAWorld<BitOrg, emp::evo::FitCacheOff> pop_lex(random, "NKWorld");
  emp::evo::EAWorld<BitOrg, emp::evo::FitCacheOff> pop_eco(random, "NKWorld");
  emp::evo::EAWorld<BitOrg, emp::evo::FitCacheOff> pop_fshare(random, "NKWorld");
  emp::evo::EAWorld<BitOrg, emp::evo::FitCacheOff> pop_tourny(random, "NKWorld");

  // Build a random initial population
  for (uint32_t i = 0; i < POP_SIZE; i++) {
    BitOrg next_org(N);
    for (uint32_t j = 0; j < N; j++) next_org[j] = random.P(0.5);
    pop_lex.Insert(next_org);
    pop_eco.Insert(next_org);
    pop_fshare.Insert(next_org);
    pop_tourny.Insert(next_org);
  }

  auto mut_fun = [MUT_COUNT, N](BitOrg* org, emp::Random& random) {
    for (uint32_t m = 0; m < MUT_COUNT; m++) {
      const uint32_t pos = random.GetUInt(N);
      (*org)[pos] = random.P(0.5);
    }
    return true;
  };

  pop_lex.SetDefaultMutateFun( mut_fun );
  pop_eco.SetDefaultMutateFun( mut_fun );
  pop_fshare.SetDefaultMutateFun( mut_fun );
  pop_tourny.SetDefaultMutateFun( mut_fun );


  auto base_fit_fun = [&landscapes](BitOrg * org) {
    double result = 1.0;
    for (size_t i = 0; i < NUM_LANDSCAPES; i++) {
      result *= landscapes[i].GetFitness(*org);
    }
    return result;
  };


  // Build the component functions for EcoEA
  fit_funs.resize(NUM_LANDSCAPES);
  for (uint32_t i = 0; i < NUM_LANDSCAPES; i++) {
    fit_funs[i] = [&landscapes, i](BitOrg * org){ return landscapes[i].GetFitness(*org); };
  }


  // Start off with simple TOURNAMENT SELECTION
  for (uint32_t ud = 0; ud < MAX_GENS; ud++) {
    pop_tourny.TournamentSelect(base_fit_fun, 5, POP_SIZE);
    pop_tourny.Update();
    std::cout << "Gen " << (ud+1) << " : ";
    for (size_t i = 0; i < NUM_LANDSCAPES; i++) {
      std::cout << " fit(" << i << ")=" << fit_funs[i](&(pop_tourny[0]));
    }
    std::cout << " fit(" << NUM_LANDSCAPES << ")=" << base_fit_fun(&(pop_tourny[0]));
    std::cout << std::endl;
    pop_tourny.MutatePop();
  }


  // Continue with EcoEA
  for (uint32_t ud = 0; ud < MAX_GENS; ud++) {
    pop_eco.EcoSelect(base_fit_fun, fit_funs, 10000.0, 5, POP_SIZE);
    pop_eco.Update();
    std::cout << "Gen " << (ud+1) << " : ";
    for (size_t i = 0; i < NUM_LANDSCAPES; i++) {
      std::cout << " fit(" << i << ")=" << fit_funs[i](&(pop_eco[0]));
    }
    std::cout << " fit(" << NUM_LANDSCAPES << ")=" << base_fit_fun(&(pop_eco[0]));
    std::cout << std::endl;
    pop_eco.MutatePop();
  }


  // Add base function to set for lexicase
  fit_funs.push_back(base_fit_fun);


  // Loop through updates FOR LEXICASE
  for (uint32_t ud = 0; ud < MAX_GENS; ud++) {
    pop_lex.LexicaseSelect(fit_funs, POP_SIZE);
    pop_lex.Update();
    std::cout << "Gen " << (ud+1) << " : ";
    for (size_t i = 0; i <= NUM_LANDSCAPES; i++) {
      std::cout << " fit(" << i << ")=" << fit_funs[i](&(pop_lex[0]));
    }
    std::cout << std::endl;
    pop_lex.MutatePop();
  }

  PrintPop(pop_tourny, emp::to_string("tourny-", config.SEED(), ".dat"));
  PrintPop(pop_eco, emp::to_string("eco-", config.SEED(), ".dat"));
  PrintPop(pop_lex, emp::to_string("lex-", config.SEED(), ".dat"));
}
