//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "../../games/Mancala.h"
#include "../../hardware/AvidaGP.h"
#include "../../hardware/InstLib.h"
#include "../../tools/Random.h"
#include "../../evo/World.h"

constexpr size_t POP_SIZE = 100;
constexpr size_t GENOME_SIZE = 50;
constexpr size_t EVAL_TIME = 200;
constexpr size_t UPDATES = 200;

bool verbose = false;

// Setup the fitness function.
double EvalGame(emp::AvidaGP & org0, emp::AvidaGP & org1, bool cur_player=0) {
  emp::Mancala game(cur_player);
  size_t round = 0, errors = 0;
  while (game.IsDone() == false) {
    if (verbose) {
      std::cout << "round = " << round++
                << "   errors = " << errors
                << std::endl;
      game.Print();
    }
    if (cur_player == 0) {  // Tested org is cur player.
      org0.ResetHardware();
      for (size_t i = 0; i < 14; i++) { org0.SetInput(i, game[i]); }
      org0.Process(EVAL_TIME);
      size_t best_move = 1;
      for (size_t i = 2; i <= 6; i++) {
        if (org0.GetOutput(best_move) < org0.GetOutput(i)) { best_move = i; }
      }
      while (game[best_move] == 0) {  // Cannot make a move into an empty pit!
        errors++;
        if (++best_move > 6) best_move = 1;
      }
      bool go_again = game.DoMove(0, best_move);
      if (!go_again) cur_player = 1;
    }
    else {
      org1.ResetHardware();
      for (size_t i = 0; i < 14; i++) { org1.SetInput((i+7)%14, game[i]); }
      org1.Process(EVAL_TIME);
      size_t best_move = 1;
      for (size_t i = 2; i <= 6; i++) {
        if (org1.GetOutput(best_move) < org1.GetOutput(i)) { best_move = i; }
      }
      while (game[best_move+7] == 0) {  // Cannot make a move in an empty pit!
        if (++best_move > 6) best_move = 1;
      }
      bool go_again = game.DoMove(1, best_move);
      if (!go_again) cur_player = 0;
    }
  }
  return ((double) game.ScoreA()) - ((double) game.ScoreB());
};


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
    [&random, &world](emp::AvidaGP * org) {
      emp::AvidaGP & rand_org = world.GetRandomOrg();
      bool cur_player = random.P(0.5);
      return EvalGame(*org, rand_org, cur_player);
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
    // Keep the best individual.
    world.EliteSelect(fit_fun, 1, 1);

    // Run a tournament for each spot.
    world.TournamentSelect(fit_fun, 5, POP_SIZE-1);
    // world.LexicaseSelect(fit_set, POP_SIZE-1);
    // world.EcoSelect(fit_fun, fit_set, 100, 5, POP_SIZE-1);
    world.Update();
    std::cout << (ud+1) << " : " << 0 << " : " << fit_fun(&(world[0])) << std::endl;

    // Mutate all but the first organism.
    world.MutatePop(1);
  }

  verbose = true;
  fit_fun(&(world[0]));

  std::cout << std::endl;
  world[0].PrintGenome();
  std::cout << std::endl;
  for (size_t i = 0; i < 16; i++) {
    std::cout << i << ":" << world[0].GetOutput(i) << "  ";
  }
  std::cout << std::endl;

  return 0;
}
