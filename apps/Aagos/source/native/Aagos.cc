#include <iostream>

#include "base/vector.h"
#include "config/command_line.h"

#include "../AagosOrg.h"
#include "../AagosWorld.h"

int main(int argc, char* argv[])
{
  emp::vector<std::string> args = emp::cl::args_to_strings(argc, argv);

  constexpr size_t NUM_BITS = 64;
  constexpr size_t NUM_GENES = 64;
  constexpr size_t GENE_SIZE = 8;
  constexpr size_t CHANGE_RATE = 0;

  constexpr size_t POP_SIZE = 400;
  constexpr size_t MAX_GENS = 1000;
  constexpr size_t MUT_COUNT = 3;

  AagosWorld world(NUM_BITS, NUM_GENES, GENE_SIZE, CHANGE_RATE);

  emp::Random & random = world.GetRandom();

  // Build a random initial population
  for (uint32_t i = 0; i < POP_SIZE; i++) {
    AagosOrg next_org(NUM_BITS, NUM_GENES, GENE_SIZE);
    next_org.Randomize(random);
    std::cout << next_org.GetNumBits() << std::endl;
    world.Inject(next_org);
  }

  for (size_t gen = 0; gen < MAX_GENS; gen++) {
    // Do mutations on the population.
    world.DoMutations(1);

    // Keep the best individual.
    emp::EliteSelect(world, 1, 1);

    // Run a tournament for the rest...
    emp::TournamentSelect(world, 5, POP_SIZE-1);

    world.Update();

    std::cout << gen << std::endl;
  }

  std::cout << "Hello World!" << std::endl;
}
