/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file  AvidaGP-Test.cc
 *  @brief A simple test of AvidaGP with World for copies and mutations.
 *
 *  A few basic steps to examine AvidaGP organisms in a world.
 *  1. Inject a lenght 10 organism
 *  2. Copy that first oranism
 *  3. Mutate the copy
 *  4. Make a copy of the mutant.
 *
 *  Each step of the way is printed.
 */

#include <iostream>

#include "hardware/AvidaGP.h"
#include "hardware/InstLib.h"
#include "tools/Random.h"
#include "Evo/World.h"

int main()
{
  emp::Random random;
  emp::World<emp::AvidaGP> world(random, "AvidaWorld");
  world.SetWellMixed(true);

  // Setup a mutation function that always performs a single mutation.
  world.SetMutFun( [](emp::AvidaGP & org, emp::Random & random) {
      const uint32_t pos = random.GetUInt(org.GetSize());
      org.RandomizeInst(pos, random);
      return 1;
    } );

  // Add a random organism.
  emp::AvidaGP cpu;
  cpu.PushRandom(random, 10);
  world.Inject(cpu.GetGenome());

  // Copy genome into cell 1
  world.Inject( world.GetGenomeAt(0) );

  std::cout << "\nGENOME 0\n";
  world[0].PrintGenome();

  std::cout << "\nGENOME 1\n";
  world[1].PrintGenome();

  // Mutate cell 1 and see what happens.
  world.DoMutations(1);

  std::cout << "\nGENOME 1 (post mutations)\n";
  world[1].PrintGenome();

  // Copy mutated genome 1 into cell 2
  world.Inject( world.GetGenomeAt(1) );

  std::cout << "\nGENOME 2 (copy of mutant)\n";
  world[2].PrintGenome();

}
