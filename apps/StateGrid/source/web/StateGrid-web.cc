//  This file is part of StateGrid
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include "tools/Random.h"
#include "web/web.h"

#include "../SGWorld.h"

namespace UI = emp::web;

UI::Document doc("emp_base");

constexpr size_t POP_SIZE = 1000;
constexpr size_t GENOME_SIZE = 50;
constexpr size_t UPDATES = 10000;

int main()
{
  doc << "<h1>Hello, world!</h1>";

  emp::Random random;
  SGWorld world(random, "AvidaWorld");

  world.SetWellMixed(true);

  // Build a random initial popoulation.
  for (size_t i = 0; i < POP_SIZE; i++) {
    SGOrg cpu(&(world.inst_lib));
    cpu.PushRandom(random, GENOME_SIZE);
    world.Inject(cpu.GetGenome());
  }

  // Do the run...
  for (size_t ud = 0; ud < UPDATES; ud++) {
    // Keep the best individual.
    EliteSelect(world, 1, 1);

    // Run a tournament for the rest...
    TournamentSelect(world, 4, POP_SIZE-1);

    // Put new organisms is place.
    world.Update();

    // Mutate all but the first organism.
    world.DoMutations(1);
  }

  return 0;
}
