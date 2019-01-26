/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  Aagos.cc
 *  @brief An example of building a custom MABE world based on NK, but more specialized.
 */

#include "../core/World.h"
#include "../core/OrganismType.h"

#include "genomes/VectorGenome.h"
#include "environments/NKLandscape.h"
#include "schemas/TournamentSelect.h"

int main(int argc, char * argv[]) {
  // Build the world using these types.
  mabe::World world("AagosWorld");

  using org_type_t = mabe::OrganismType<mabe::VectorGenome<bool>, mabe::VectorGenome<int>>;
  using org_t = org_type_t::Organism;

  auto & org_type       = world.BuildModule<org_type_t>("Organisms");
  org_type.GetGenomeType<1>().SetName("GenePositions");

  auto & landscape      = world.BuildModule<mabe::NKLandscape>("NKLandscape");
  auto & tourny_schema  = world.BuildModule<mabe::TournamentSelect>("TournamentSelect");

  auto fit_fun = [&landscape](mabe::OrganismBase & base_org){
    org_t & org = (org_t &) base_org;
    (void) org;
    (void) landscape;
    return 1.0;
  };
  tourny_schema.SetFitFun(fit_fun);

  // Configure the world using the "Aagos.cfg" file and command-line overrides.
  world.Config(argc, argv, "Aagos.cfg");
  
  // Get details on how the population was configured.
  world.PrintStatus();

  // Running world will go for full configured duration; can also manually Update() it.
  return world.Run();
}

