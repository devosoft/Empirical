/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file  AvidaGP-Evo.cc
 *  @brief A test of AvidaGP with World; organisms must find squares of values.
 */

#include <iostream>

#include "emp/hardware/AvidaGP.hpp"
#include "emp/hardware/InstLib.hpp"
#include "emp/math/Random.hpp"
#include "emp/Evolve/World.hpp"

void Print(const emp::AvidaGP & cpu) {
  for (size_t i = 0; i < 16; i++) {
    std::cout << "[" << cpu.GetReg(i) << "] ";
  }
  std::cout << " IP=" << cpu.GetIP() << std::endl;
}

constexpr size_t POP_SIZE = 1000;
constexpr size_t GENOME_SIZE = 50;
constexpr size_t UPDATES = 500;

int main()
{
  emp::Random random;
  emp::World<emp::AvidaGP> world(random, "AvidaWorld");
  world.SetPopStruct_Mixed(true);

  // Build a random initial popoulation.
  for (size_t i = 0; i < POP_SIZE; i++) {
    emp::AvidaGP cpu;
    cpu.PushRandom(random, GENOME_SIZE);
    world.Inject(cpu.GetGenome());
  }

  // Setup the mutation function.
  world.SetMutFun( [](emp::AvidaGP & org, emp::Random & random) {
      uint32_t num_muts = random.GetUInt(4);  // 0 to 3 mutations.
      for (uint32_t m = 0; m < num_muts; m++) {
        const uint32_t pos = random.GetUInt(GENOME_SIZE);
        org.RandomizeInst(pos, random);
      }
      return num_muts;
    } );

  // Setup the fitness function.
  std::function<double(const emp::AvidaGP &)> fit_fun =
    [](const emp::AvidaGP & org) {
      int count = 0;
      for (int i = 0; i < 16; i++) {
        if (org.GetOutput(i) == (double) (i*i)) count++;
      }
      return (double) count;
    };
  world.SetFitFun(fit_fun);

  emp::vector< std::function<double(const emp::AvidaGP &)> > fit_set(16);
  for (size_t out_id = 0; out_id < 16; out_id++) {
    // Setup the fitness function.
    fit_set[out_id] = [out_id](const emp::AvidaGP & org) {
      return (double) -std::abs(org.GetOutput((int)out_id) - (double) (out_id * out_id));
    };
  }


  // Do the run...
  for (size_t ud = 0; ud < UPDATES; ud++) {
    // Update the status of all organisms.
    world.ResetHardware();
    world.Process(200);
    double fit0 = world.CalcFitnessID(0);
    std::cout << (ud+1) << " : " << 0 << " : " << fit0 << std::endl;

    // Keep the best individual.
    EliteSelect(world, 1, 1);

    // Run a tournament for the rest...
    TournamentSelect(world, 5, POP_SIZE-1);
    // LexicaseSelect(world, fit_set, POP_SIZE-1);
    // EcoSelect(world, fit_fun, fit_set, 100, 5, POP_SIZE-1);
    world.Update();

    // Mutate all but the first organism.
    world.DoMutations(1);
  }

  std::cout << std::endl;
  world[0].PrintGenome();
  std::cout << std::endl;
  for (int i = 0; i < 16; i++) {
    std::cout << i << ":" << world[0].GetOutput(i) << "  ";
  }
  std::cout << std::endl;

  return 0;
}
