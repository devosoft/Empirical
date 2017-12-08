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
constexpr size_t EVAL_TIME = 2000;
constexpr size_t UPDATES = 1000;
constexpr size_t TOURNY_SIZE = 4;

void Print(const emp::AvidaGP & cpu) {
  for (size_t i = 0; i < 16; i++) {
    std::cout << "[" << cpu.GetReg(i) << "] ";
  }
  std::cout << " IP=" << cpu.GetIP() << std::endl;
}

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
  std::cout<<"PLL: "<<player<<std::endl;
  for (size_t i = 0; i < boardSize* boardSize; i++) {
      size_t square = game.GetSquare(i);
      int conv_square;

      if (square == player) conv_square = 1;
      else if (square == 0) conv_square = 0;
      else conv_square = -1;
      std::cout<<conv_square<<" ";
      if (i % 8 == 7) {std::cout<<std::endl;}
      org.SetInput(i, conv_square);
  }

  // Run the code.
  org.Process(EVAL_TIME);

  // Determine the chosen move.
  
  size_t best_move = 0;

  for (size_t i = 0; i < boardSize * boardSize; i++) {
    //std::cout << "BEST: "<< org.GetOutput(best_move)<< " "<< org.GetOutput(i)<<std::endl;
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
    std::cout<<"Player: "<<cur_player<<std::endl;
    if (!go_again) {cur_player = !cur_player; game.ChangeTurn();}
    //else { if (verbose) std::cout<<"AGAIN!!!!!!!!!"<<std::endl; }
    std::cout<<"#################################"<<std::endl;
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
  emp::AvidaGP cpu;

  cpu.PushInst("SetBoard", 0);
  cpu.PushInst("SetReg", 0, 64);
  cpu.PushInst("SetReg", 2, 0);
  cpu.PushInst("SetReg", 1, 1);

  cpu.PushInst("While", 1, 1 );
    cpu.PushInst("GetValidAbove", 2, 3);
    cpu.PushInst("If", 3, 2);
      cpu.PushInst("SetReg", 1, 0);
      cpu.PushInst("Break", 0);
    cpu.PushInst("Scope", 2);
    cpu.PushInst("GetValidBelow", 2, 3);
    cpu.PushInst("If", 3, 2);
      cpu.PushInst("SetReg", 1, 0);
      cpu.PushInst("Break", 0);
    cpu.PushInst("Scope", 2);
    cpu.PushInst("GetValidLeft", 2, 3);
    cpu.PushInst("If", 3, 2);
      cpu.PushInst("SetReg", 1, 0);
      cpu.PushInst("Break", 0);
    cpu.PushInst("Scope", 2);
    cpu.PushInst("GetValidRight", 2, 3);
    cpu.PushInst("If", 3, 2);
      cpu.PushInst("SetReg", 1, 0);
      cpu.PushInst("Break", 0);
    cpu.PushInst("Scope", 2);
    cpu.PushInst("GetValidUL", 2, 3);
    cpu.PushInst("If", 3, 2);
      cpu.PushInst("SetReg", 1, 0);
      cpu.PushInst("Break", 0);
    cpu.PushInst("Scope", 2);
    cpu.PushInst("GetValidUR", 2, 3);
    cpu.PushInst("If", 3, 2);
      cpu.PushInst("SetReg", 1, 0);
      cpu.PushInst("Break", 0);
    cpu.PushInst("Scope", 2);
    cpu.PushInst("GetValidLL", 2, 3);
    cpu.PushInst("If", 3, 2);
      cpu.PushInst("SetReg", 1, 0);
      cpu.PushInst("Break", 0);
    cpu.PushInst("Scope", 2);
    cpu.PushInst("GetValidLR", 2, 3);
    cpu.PushInst("If", 3, 2);
      cpu.PushInst("SetReg", 1, 0);
      cpu.PushInst("Break", 0);
    cpu.PushInst("Scope", 2);
    cpu.PushInst("Inc", 2);
    cpu.PushInst("TestNEqu", 2, 0, 1);
  cpu.PushInst("Scope", 0);
  cpu.PushInst("Output", 2, 2);

  EvalGame(cpu, cpu, 0, true);
  //EvalGame(cpu);

  // And try playing it!
  /*
  while (true) {
    std::cout << "NEW GAME: Human vs. AI!\n";
    EvalGame(world[0]);
  }
  */

  return 0;
}
