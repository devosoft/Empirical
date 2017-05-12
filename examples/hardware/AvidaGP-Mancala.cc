//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "../../games/Mancala.h"
#include "../../hardware/AvidaGP.h"
#include "../../hardware/InstLib.h"
#include "../../tools/Random.h"
#include "../../evo/World.h"

constexpr size_t POP_SIZE = 1000;
constexpr size_t GENOME_SIZE = 50;
constexpr size_t UPDATES = 500;

int main()
{
  emp::Random random;
  emp::Mancala game;
  emp::evo::EAWorld<emp::AvidaGP> world(random, "AvidaWorld");

  // Build a random initial popoulation.
  for (size_t i = 0; i < POP_SIZE; i++) {
    emp::AvidaGP cpu;
    cpu.PushRandom(random, GENOME_SIZE);
    world.Insert(cpu);
  }

  // Setup the mutation function.
  world.SetDefaultMutateFun( [](emp::AvidaGP* org, emp::Random& random) {
      uint32_t num_muts = random.GetUInt(4);  // 0 to 3 mutations.
      for (uint32_t m = 0; m < num_muts; m++) {
        const uint32_t pos = random.GetUInt(GENOME_SIZE);
        org->RandomizeInst(pos, random);
      }
      return (num_muts > 0);
    } );

  // Setup the fitness function.
  std::function<double(emp::AvidaGP*)> fit_fun =
    [&game, &random, &world](emp::AvidaGP * org) {
      emp::AvidaGP & org2 = world.GetRandomOrg();
      bool cur_player = random.P(0.5);
      game.Reset(cur_player);
      while (game.IsDone() == false) {
        if (cur_player == 0) {  // Tested org is cur player.
          // Setup the hardware.
          org->ResetHardware();


          org->Process(200);
        }
      }
      return 1.0;
    };

  emp::vector< std::function<double(emp::AvidaGP*)> > fit_set(16);
  for (size_t out_id = 0; out_id < 16; out_id++) {
    // Setup the fitness function.
    fit_set[out_id] = [out_id](emp::AvidaGP * org) {
      return (double) -std::abs(org->GetOutput(out_id) - out_id * out_id);
    };
  }


  // Do the run...
  for (size_t ud = 0; ud < UPDATES; ud++) {
    // Update the status of all organisms.
    for (size_t id = 0; id < POP_SIZE; id++) {
      world[id].ResetHardware();
      world[id].Process(200);
    }

    // Keep the best individual.
    world.EliteSelect(fit_fun, 1, 1);

    // Run a tournament for the rest...
    // world.TournamentSelect(fit_fun, 5, POP_SIZE-1);
    world.LexicaseSelect(fit_set, POP_SIZE-1);
    // world.EcoSelect(fit_fun, fit_set, 100, 5, POP_SIZE-1);
    world.Update();
    std::cout << (ud+1) << " : " << 0 << " : " << fit_fun(&(world[0])) << std::endl;

    // Mutate all but the first organism.
    world.MutatePop(1);
  }

  std::cout << std::endl;
  world[0].PrintGenome();
  std::cout << std::endl;
  for (size_t i = 0; i < 16; i++) {
    std::cout << i << ":" << world[0].GetOutput(i) << "  ";
  }
  std::cout << std::endl;

  return 0;
}
