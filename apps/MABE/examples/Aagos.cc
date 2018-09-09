/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  Aagos.cc
 *  @brief An example of building a custom MABE evolver based on NK, but more specialized.
 */

#include "../core/World.h"
#include "../core/OrganismType.h"

#include "genomes/VectorGenome.h"
#include "environments/NKLandscape.h"
#include "schemas/TournamentSelect.h"

int main(int argc, char * argv[]) {
  mabe::World<mabe::NKLandscape> evolver("AagosWorld");

  using org_type_t = mabe::OrganismType<mabe::VectorGenome<bool>, mabe::VectorGenome<int>>;
  using org_t = org_type_t::Organism;

  auto & org_type = evolver.AddOrgType<org_type_t>("Organisms");
  org_type.GetGenomeType<1>().SetName("GenePositions");

  auto & tourny_schema  = evolver.AddSchema<mabe::TournamentSelect>("TournamentSelect");

  auto fit_fun = [&evolver](mabe::OrganismBase & base_org){
    org_t & org = (org_t &) base_org;
    (void) org;
    // @CAO: evolver.GetEnvironment() ...
    return 1.0;
  };
  tourny_schema.SetFitFun(fit_fun);

  // Configure the evolver using the "Aagos.cfg" file and command-line overrides.
  evolver.Config(argc, argv, "Aagos.cfg");
  
  // Get details on how the population was configured.
  evolver.PrintStatus();

  // Running evolver will go for full configured duration; can also manually Update() it.
  return evolver.Run();
}

