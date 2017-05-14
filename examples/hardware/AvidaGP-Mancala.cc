//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "../../games/Mancala.h"
#include "../../hardware/AvidaGP.h"
#include "../../hardware/InstLib.h"
#include "../../tools/Random.h"
#include "../../evo/World.h"

constexpr size_t POP_SIZE = 200;
constexpr size_t GENOME_SIZE = 100;
constexpr size_t EVAL_TIME = 500;
constexpr size_t UPDATES = 5000;
constexpr size_t TOURNY_SIZE = 5;

// Determine the next move of a human player.
size_t EvalMove(emp::Mancala & game, std::ostream & os=std::cout, std::istream & is=std::cin) {
  // Request a move from the human.
  char move;
  os << "Move?" << std::endl;
  is >> move;

  while (move < 'A' || move > 'F' || game.GetCurSide()[move-'A'] == 0) {
    os << "Invalid move! (choose a value 'A' to 'F')" <<  std::endl;
    is.clear();
    is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    is >> move;
  }

  return (size_t) (move - 'A');
}


// Determine the next move of an AvidaGP player.
size_t EvalMove(emp::Mancala & game, emp::AvidaGP & org) {
  // Setup the hardware with proper inputs.
  org.ResetHardware();
  const auto & cur_side = game.GetCurSide();
  const auto & other_side = game.GetOtherSide();
  for (size_t i = 0; i < 7; i++) {
    org.SetInput(i, cur_side[i]);
    org.SetInput(i+7, other_side[i]);
  }

  // Run the code.
  org.Process(EVAL_TIME);

  // Determine the chosen move.
  size_t best_move = 0;
  for (size_t i = 1; i < 6; i++) {
    if (org.GetOutput(best_move) < org.GetOutput(i)) { best_move = i; }
  }

  return best_move;
}

// Setup the fitness function for a whole game.
double EvalGame(emp::AvidaGP & org0, emp::AvidaGP & org1, bool cur_player=0, bool verbose=false) {
  emp::Mancala game(cur_player==0);
  size_t round = 0, errors = 0;
  while (game.IsDone() == false) {
    // Determine the current player and their move.
    emp::AvidaGP & cur_org = (cur_player == 0) ? org0 : org1;
    size_t best_move = EvalMove(game, cur_org);

    if (verbose) {
      std::cout << "round = " << round++ << "   errors = " << errors << std::endl;
      game.Print();
      char move_sym = 'A' + (char) best_move;
      std::cout << "Move = " << move_sym;
      if (game.GetCurSide()[best_move] == 0) {
        std::cout << " (illegal!)";
      }
      std::cout << std::endl << std::endl;
    }

    // If the chosen move is illegal, shift through other options.
    while (game.GetCurSide()[best_move] == 0) {  // Cannot make a move into an empty pit!
      if (cur_player == 0) errors++;
      if (++best_move > 5) best_move = 0;
    }

    // Do the move and determine who goes next.
    bool go_again = game.DoMove(cur_player, best_move);
    if (!go_again) cur_player = !cur_player;
  }

  return ((double) game.ScoreA()) - ((double) game.ScoreB()) - ((double) errors * 10.0);
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
    world.TournamentSelect(fit_fun, TOURNY_SIZE, POP_SIZE-1);
    // world.LexicaseSelect(fit_set, POP_SIZE-1);
    // world.EcoSelect(fit_fun, fit_set, 100, TOURNY_SIZE, POP_SIZE-1);
    world.Update();
    std::cout << (ud+1) << " : " << 0 << " : " << fit_fun(&(world[0])) << std::endl;

    // Mutate all but the first organism.
    world.MutatePop(1);
  }

  fit_fun(&(world[0]));

  std::cout << std::endl;
  world[0].PrintGenome();
  std::cout << std::endl;

  EvalGame(world[0], world[1], 0, true);

  return 0;
}
