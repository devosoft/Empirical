#include <iostream>

#include "base/vector.h"
#include "config/ArgManager.h"
#include "config/command_line.h"

#include "../AagosOrg.h"
#include "../AagosWorld.h"

int main(int argc, char* argv[])
{
  AagosConfig config;
  // Deal with loading config values via native interface (config file and command line args)
  config.Read("Aagos.cfg");
  auto args = emp::cl::ArgManager(argc, argv);
  if (args.ProcessConfigOptions(config, std::cout, "Aagos.cfg", "Aagos-macros.h") == false) exit(0);
  if (args.TestUnknown() == false) exit(0);  // If there are leftover args, throw an error.

  AagosWorld world(config);

  emp::Random & random = world.GetRandom();

  // Build a random initial population
  for (uint32_t i = 0; i < config.POP_SIZE(); i++) {
    AagosOrg next_org(config.NUM_BITS(), config.NUM_GENES(), config.GENE_SIZE());
    next_org.Randomize(random);
    std::cout << next_org.GetNumBits() << std::endl;
    world.Inject(next_org);
  }

  for (size_t gen = 0; gen < config.MAX_GENS(); gen++) {
    // Do mutations on the population.
    world.DoMutations(1);

    // Keep the best individual.
    if (ELITE_COUNT) emp::EliteSelect(world, config.ELITE_COUNT(), 1);

    // Run a tournament for the rest...
    emp::TournamentSelect(world, config.TOURNAMENT_SIZE(), config.POP_SIZE()-config.ELITE_COUNT());

    world.Update();

    if (gen % config.PRINT_INTERVAL() == 0) {
      std::cout << gen
                << " : fitness=" << world.CalcFitnessID(0)
                << " size=" << world[0].GetNumBits()
                << std::endl;
      world[0].Print();
    }
  }
}
