//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "../../games/Othello.h"
#include "../../hardware/AvidaGPOthello.h"
#include "../../hardware/InstLib.h"
#include "../../tools/Random.h"
#include "../../evo/World.h"
#include <stdlib.h>
#include <utility>

constexpr size_t POP_SIZE = 1000;
constexpr size_t GENOME_SIZE = 100;
constexpr size_t EVAL_TIME = 3000;
constexpr size_t UPDATES = 1000;
constexpr size_t TOURNY_SIZE = 4;

// Determine the next move of a human player.
size_t EvalMove(emp::Othello & game, std::ostream & os=std::cout, std::istream & is=std::cin) {
  // Present the current board.
  game.Print();
  std::exit(-1);
  // Request a move from the human.
  int moveX = 0;
  int moveY = 0;

  os << "Move?" << std::endl;
  is >> moveX;
  is >> moveY;

  std::pair<int, int> moveXY = std::make_pair(moveX, moveY);
  bool invalid = true;
  int boardSize = game.GetBoardSize();

  while (invalid) {

    if (moveX < 1 || moveX > boardSize) {
      std::cout << "Invalid move!! (choose an X value 1 to " << boardSize<<")" <<  std::endl;
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      continue;
    }
    if (moveY < 1 || moveY > boardSize) {
      std::cout << "Invalid move!! (choose an Y value 1 to " << boardSize<<")" <<  std::endl;
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      continue;
    }
    

    if (game.GetSquare(moveX, moveY) != 0) {
      std::cout << "Error: Cannot move to non-empty tile" << std::endl;
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      continue;
    }

    if (game.IsMoveValid(game.GetCurrPlayer(), moveXY ) == 0) {
        std::cout << "Invalid Move: Must flank at least one opponent disc" <<std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        continue;
    }

    invalid = false;
  }

  size_t move = game.GetIndex(moveX, moveY);

  return move;
}


// Determine the next move of an AvidaGP player.
size_t EvalMove(emp::Othello & game, emp::AvidaGP & org) {
  // Setup the hardware with proper inputs.
  org.ResetHardware();
  int boardSize = game.GetBoardSize();
  size_t player = game.GetCurrPlayer();
  for (size_t i = 0; i < boardSize* boardSize; i++) {
      size_t square = game.GetSquare(i);
      int conv_square;

      if (square == player) conv_square = 1;
      else if (square == 0) conv_square = 0;
      else conv_square = -1;
      org.SetInput(i, conv_square);
  }

  // Run the code.
  org.Process(EVAL_TIME);

  // Determine the chosen move.
  
  size_t best_move = 0;

  for (size_t i = 0; i < boardSize * boardSize; i++) {
      if (org.GetOutput(best_move) < org.GetOutput(i)) { best_move = i; }
          
  }

  return best_move;
}

using othello_ai_t = std::function< size_t(emp::Othello & game) >;

// Setup the fitness function for a whole game.
double EvalGame(othello_ai_t & player0, othello_ai_t & player1,
                bool cur_player=0, bool verbose=false) { 
  emp::Othello game(cur_player==0);
  int boardSize = game.GetBoardSize();
  size_t round = 0, errors = 0; 
  while (game.IsDone() == false) {
      // Determine the current player and their move.
    game.ClearValidMoves();
    game.ClearFlips();
    auto & play_fun = (cur_player == 0) ? player0 : player1;
    size_t best_move = play_fun(game);
    std::pair<int, int> best = game.GetCoord(best_move);

    if (verbose) {
      std::cout << "round = " << round++ << "   errors = " << errors << std::endl;
      game.Print();
      
      std::cout << "Move = " << best_move << std::endl;//best.first + " " + best.second<<std::endl;
      if (game.GetSquare(best_move) != 0) {
        std::cout << " (illegal!)";
      }
      std::cout << std::endl << std::endl;
    }
    // If the chosen move is illegal, shift through other options.
    size_t player = 1;
    if (cur_player) { player = 2; }
    while (game.IsMoveValid(player, best) == 0 || game.GetSquare(best.first, best.second) != 0) {  
      if (cur_player == 0) errors++;
      if (++best_move >= boardSize * boardSize) { best_move = 0; }
      best = game.GetCoord(best_move);
      game.ClearFlips();
      game.ClearValidMoves();
      
    }
    if (verbose){
        std::cout<<best.first<<" "<<best.second<<std::endl;
        std::cout<<player<<std::endl;
        //std::cout<<game.IsMoveValid(player, best)<<std::endl;
    }
    // Do the move and determine who goes next.
    bool go_again = game.DoMove(player, best, verbose);
    game.ClearFlips();
    game.ClearValidMoves();
    if (!go_again) {cur_player = !cur_player; game.ChangeTurn();}
    //else { if (verbose) std::cout<<"AGAIN!!!!!!!!!"<<std::endl; }
  }

  if (verbose) {
    game.Print();
    std::cout << "Final scores -- Black: " << game.GetScore(1)
              << "   White: " << game.GetScore(2)
              << std::endl;
  }

  return ((double) game.GetScore(1)) - ((double) game.GetScore(2)) - ((double) errors * 10.0);
};

// Build wrappers for AvidaGP
double EvalGame(emp::AvidaGP & org0, emp::AvidaGP & org1, bool cur_player=0, bool verbose=false) {
  othello_ai_t org_fun0 = [&org0](emp::Othello & game){ return EvalMove(game, org0); };
  othello_ai_t org_fun1 = [&org1](emp::Othello & game){ return EvalMove(game, org1); };
  return EvalGame(org_fun0, org_fun1, cur_player, verbose);
};

// Otherwise assume a human opponent!
double EvalGame(emp::AvidaGP & org, bool cur_player=0) { 
  othello_ai_t fun0 = [&org](emp::Othello & game){ return EvalMove(game, org); };
  othello_ai_t fun1 = [](emp::Othello & game){ return EvalMove(game, std::cout, std::cin); };
  std::cout<<"START"<<std::endl; //TODO
  std::exit(-1);
  return EvalGame(fun0, fun1, cur_player, true);
};


int main()
{
  emp::Random random;
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
  world[0].PrintGenome("othello_save.org");

  EvalGame(world[0], world[1], 0, true);

  // And try playing it!
  /*
  while (true) {
    std::cout << "NEW GAME: Human vs. AI!\n";
    EvalGame(world[0]);
  }
  */

  return 0;
}
