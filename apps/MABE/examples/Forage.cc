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

#include "brains/AvidaGP.h"
#include "genomes/LinearGenome.h"
#include "environments/ResourceGrid.h"
#include "schemas/TournamentSelect.h"
#include "schemas/TrackLineage.h"

int main(int argc, char * argv[]) {
  // Build the world using these types.
  mabe::World<mabe::ResourceGrid> world("ForageWorld");

  using org_t = mabe::OrganismType<mabe::AvidaGP, mabe::LinearGenome>;
  auto & org_type        = world.AddOrgType<org_t>("Organisms");
  auto & tourny_schema   = world.AddSchema<mabe::TournamentSelect>("TournamentSelect");
  auto & lineage_tracker = world.AddSchema<mabe::TrackLineage>("LineageTracker");

  (void) org_type; (void) tourny_schema; (void) lineage_tracker;

  // Configure the world using the "forage.cfg" file and command-line overrides.
  world.Config(argc, argv, "forage.cfg");
  
  // Get details on how the population is configured.
  world.PrintStatus();

  // Running world will go for full configured duration; can also manually Update() it.
  return world.Run();
}

