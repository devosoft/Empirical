/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file AvidaGP-Test.cpp
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

#include "emp/Evolve/World.hpp"
#include "emp/hardware/AvidaGP.hpp"
#include "emp/hardware/InstLib.hpp"
#include "emp/math/Random.hpp"

void Print(const emp::AvidaGP & cpu) {
  cpu.PrintGenome();
  for (size_t i = 0; i < 16; i++) {
    std::cout << "[" << cpu.GetReg(i) << "] ";
  }
  std::cout << " IP=" << cpu.GetIP() << std::endl;
}

constexpr size_t POP_SIZE = 1000;
constexpr size_t GENOME_SIZE = 50;
constexpr size_t UPDATES = 50;

int main()
{
  emp::Random random;
  emp::World<emp::AvidaGP> world(random, "AvidaWorld");
  world.SetPopStruct_Mixed(true);

  // Add a random organism.
  emp::AvidaGP cpu;
  cpu.PushRandom(random, 10);
  world.Inject(cpu.GetGenome());

  // Setup a mutation function that always performs a single mutation.
  world.SetMutFun( [](emp::AvidaGP & org, emp::Random & random) {
      const uint32_t pos = random.GetUInt(org.GetSize());
      org.RandomizeInst(pos, random);
      return 1;
    } );

  // Copy genome into cell 1
  world.Inject( world.GetGenomeAt(0) );

  std::cout << std::endl << "GENOME 0" << std::endl;
  Print(world[0]);

  std::cout << std::endl << "GENOME 1" << std::endl;
  Print(world[1]);

  // Mutate cell 1 and see what happens.
  world.DoMutations(1);

  std::cout << std::endl << "GENOME 1 (post mutations)" << std::endl;
  Print(world[1]);

  // Copy mutated genome 1 into cell 2
  world.Inject( world.GetGenomeAt(1) );

  std::cout << std::endl << "GENOME 2 (copy of mutant)" << std::endl;
  Print(world[2]);

  // Let's do some selection; setup a neutral fitness function.
  world.SetFitFun( [](const emp::AvidaGP &){ return 0.0; } );

  world.ResetHardware();
  world.Process(200);
  EliteSelect(world, 1, 3);
  TournamentSelect(world, 3, 1);

  std::cout << std::endl << "GENOME 0 (after selection!)" << std::endl;
  Print(world[0]);

  world.Update();
  double fit0 = world.CalcFitnessID(0);
  std::cout << "Fitness 0 = " << fit0 << std::endl;
  world.DoMutations(1);

  std::cout << std::endl << "GENOME 0 (and DoMutations, but not on this!)" << std::endl;
  Print(world[0]);

}
