/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  Forage.cc
 *  @brief An example of building a MABE evovler where AvidaGP organisms evovle to forage.
 */

#include "../core/World.h"
#include "../core/OrganismType.h"

#include "brains/AvidaGP.h"
#include "genomes/LinearGenome.h"
#include "environments/ResourceGrid.h"
#include "schemas/TournamentSelect.h"
#include "schemas/TrackLineage.h"

int main(int argc, char * argv[]) {
  mabe::World<mabe::ResourceGrid> evolver("ForageWorld");

  using org_t = mabe::OrganismType<mabe::AvidaGP, mabe::LinearGenome>;
  auto & org_type        = evolver.AddOrgType<org_t>("Organisms");
  auto & tourny_schema   = evolver.AddSchema<mabe::TournamentSelect>("TournamentSelect");
  auto & lineage_tracker = evolver.AddSchema<mabe::TrackLineage>("LineageTracker");

  (void) org_type; (void) tourny_schema; (void) lineage_tracker;

  // Configure the evolver using the "forage.cfg" file and command-line overrides.
  evolver.Config(argc, argv, "forage.cfg");
  
  // Get details on how the population is configured.
  evolver.PrintStatus();

  // Running evolver will go for full configured duration; can also manually Update() it.
  return evolver.Run();
}

