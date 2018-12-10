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
  // Build a simple evolver, specfying an NK Landscape for its environment.
  mabe::Evolver<mabe::NKLandscape> evolver("NKWorld");

  // Use trounament selection to update this world.
  evolver.AddSchema<mabe::TournamentSelect>("TournamentSelect");

  // Generate a new organism based on BitVectorGenome and put it in the "Organism" category.
  evolver.AddOrganism<mabe::BitVectorGenome>("Organism");

  // Configure the evolver using the "NK.cfg" file and command-line overrides.
  evolver.Config(argc, argv, "NK.cfg");
  
  // Get details on how the population was configured.
  evolver.PrintStatus();

  // Running evolver will go for full configured duration; can also manually Update() it.
  return evolver.Run();
}

