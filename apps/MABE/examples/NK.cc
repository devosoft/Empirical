/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  NK.cc
 *  @brief A simple NK landscape with no brains in the genomes.
 */

#include "../core/Evolver.h"
#include "../core/OrganismType.h"

#include "genomes/BitVectorGenome.h"
#include "environments/NKLandscape.h"
#include "schemas/TournamentSelect.h"

int main(int argc, char * argv[]) {
  mabe::Evolver<mabe::NKLandscape> evolver("NKWorld");

  using org_type_t = mabe::OrganismType<mabe::BitVectorGenome>;
  evolver.AddOrgType<org_type_t>("Organisms");
  evolver.AddSchema<mabe::TournamentSelect>("TournamentSelect");

  // Configure the evolver using the "NK.cfg" file and command-line overrides.
  evolver.Config(argc, argv, "NK.cfg");
  
  // Get details on how the population was configured.
  evolver.PrintStatus();

  // Running evolver will go for full configured duration; can also manually Update() it.
  return evolver.Run();
}

