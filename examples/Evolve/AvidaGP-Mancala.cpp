//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "emp/games/Mancala.hpp"
#include "emp/hardware/AvidaGP.hpp"
#include "emp/hardware/InstLib.hpp"
#include "emp/math/Random.hpp"
#include "emp/Evolve/World.hpp"

constexpr size_t POP_SIZE = 20;
constexpr size_t GENOME_SIZE = 100;
constexpr size_t EVAL_TIME = 500;
constexpr size_t UPDATES = 100;
constexpr size_t TOURNY_SIZE = 4;

// Determine the next move of a human player.
size_t EvalMove(emp::Mancala & game, std::ostream & os=std::cout, std::istream & is=std::cin) {
  // Present the current board.
  game.Print();

  // Request a move from the human.
  char move;
  os << "Move?" << std::endl;
  is >> move;

  while (move < 'A' || move > 'F' || game.GetCurSide()[(size_t)(move-'A')] == 0) {
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
  org.SetInputs(game.AsInput(game.GetCurPlayer()));

  // Run the code.
  org.Process(EVAL_TIME);

  // Determine the chosen move.
  int best_move = 0;
  for (int i = 1; i < 6; i++) {
    if (org.GetOutput(best_move) < org.GetOutput(i)) { best_move = i; }
  }

  return (size_t) best_move;
}

using mancala_ai_t = std::function< size_t(emp::Mancala & game) >;

// Setup the fitness function for a whole game.
double EvalGame(mancala_ai_t & player0, mancala_ai_t & player1,
                bool cur_player=0, bool verbose=false) {
  emp::Mancala game(cur_player==0);
  size_t round = 0, errors = 0;
  while (game.IsDone() == false) {
    // Determine the current player and their move.
    auto & play_fun = (cur_player == 0) ? player0 : player1;
    size_t best_move = play_fun(game);

    if (verbose) {
      std::cout << "round = " << round++ << "   errors = " << errors << std::endl;
      game.Print();
      char move_sym = (char) ('A' + best_move);
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

  if (verbose) {
    std::cout << "Final scores -- A: " << game.ScoreA()
              << "   B: " << game.ScoreB()
              << std::endl;
  }

  return ((double) game.ScoreA()) - ((double) game.ScoreB()) - ((double) errors * 10.0);
}

// Build wrappers for AvidaGP
double EvalGame(emp::AvidaGP & org0, emp::AvidaGP & org1, bool cur_player=0, bool verbose=false) {
  mancala_ai_t org_fun0 = [&org0](emp::Mancala & game){ return EvalMove(game, org0); };
  mancala_ai_t org_fun1 = [&org1](emp::Mancala & game){ return EvalMove(game, org1); };
  return EvalGame(org_fun0, org_fun1, cur_player, verbose);
}

// Otherwise assume a human opponent!
double EvalGame(emp::AvidaGP & org, bool cur_player=0) {
  mancala_ai_t fun0 = [&org](emp::Mancala & game){ return EvalMove(game, org); };
  mancala_ai_t fun1 = [](emp::Mancala & game){ return EvalMove(game, std::cout, std::cin); };
  return EvalGame(fun0, fun1, cur_player, true);
}


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
  world.SetMutFun( [](emp::AvidaGP & org, emp::Random& random) {
      uint32_t num_muts = random.GetUInt(4);  // 0 to 3 mutations.
      for (uint32_t m = 0; m < num_muts; m++) {
        const uint32_t pos = random.GetUInt(GENOME_SIZE);
        org.RandomizeInst(pos, random);
      }
      return (num_muts > 0);
    } );

  // Setup the fitness function.
  std::function<double(emp::AvidaGP &)> fit_fun =
    [&random, &world](emp::AvidaGP & org) {
      emp::AvidaGP & rand_org = world.GetRandomOrg();
      bool cur_player = random.P(0.5);
      return EvalGame(org, rand_org, cur_player);
    };
  world.SetFitFun(fit_fun);

  emp::vector< std::function<double(emp::AvidaGP &)> > fit_set(16);
  for (size_t out_id = 0; out_id < 16; out_id++) {
    // Setup the fitness function.
    fit_set[out_id] = [out_id](emp::AvidaGP & org) {
      return (double) -std::abs(org.GetOutput((int)out_id) - (double) (out_id * out_id));
    };
  }


  // Do the run...
  for (size_t ud = 0; ud < UPDATES; ud++) {
    // Keep the best individual.
    EliteSelect(world, 1, 1);

    // Run a tournament for each spot.
    TournamentSelect(world, TOURNY_SIZE, POP_SIZE-1);
    // LexicaseSelect(world, POP_SIZE-1);
    // EcoSelect(world, fit_set, 100, TOURNY_SIZE, POP_SIZE-1);
    world.Update();
    std::cout << (ud+1) << " : " << 0 << " : " << world.CalcFitnessID(0) << std::endl;

    // Mutate all but the first organism.
    world.DoMutations(1);
  }

  world.CalcFitnessID(0);

  std::cout << std::endl;
  emp::Mancala game(0);
  world[0].PrintGenome("mancala_save.org");

  game.DoMove(0);
  world.GetOrg(0).ResetHardware();
  world.GetOrg(0).SetInputs(game.AsInput(game.GetCurPlayer()));
  world.GetOrg(0).Trace(1);

  game.DoMove(5);
  world.GetOrg(0).ResetHardware();
  world.GetOrg(0).SetInputs(game.AsInput(game.GetCurPlayer()));
  world.GetOrg(0).Trace(1);


  // EvalGame(world[0], world[1], 0, true);
  //
  // // And try playing it!
  // while (true) {
  //   std::cout << "NEW GAME: Human vs. AI!\n";
  //   EvalGame(world[0]);
  // }

  return 0;
}
