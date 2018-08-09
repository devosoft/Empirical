/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  Forage.cc
 *  @brief An example of building a MABE world where AvidaGP organisms evovle to forage.
 */

#include "../core/World.h"
#include "../core/OrganismType.h"

#include "genomes/VectorGenome.h"
#include "environments/NKLandscape.h"
#include "schemas/TournamentSelect.h"

int main(int argc, char * argv[]) {
  // Build the world using these types.
  mabe::World world("AagosWorld");

  using org_t = mabe::OrganismType<mabe::VectorGenome<bool>, mabe::VectorGenome<int>>;
  auto & org_type       = world.BuildModule<org_t>("Organisms");
  org_type.GetGenome<1>().SetName("GenePositions");

  auto & landscape      = world.BuildModule<mabe::NKLandscape>("NKLandscape");
  auto & tourny_schema  = world.BuildModule<mabe::TournamentSelect>("TournamentSelect");

  (void) org_type; (void) landscape; (void) tourny_schema;

  // Configure the world using the "Aagos.cfg" file and command-line overrides.
  world.Config(argc, argv, "Aagos.cfg");
  
  // Get details on how the population was configured.
  world.PrintStatus();

  // Running world will go for full configured duration; can also manually Update() it.
  return world.Run();
}

