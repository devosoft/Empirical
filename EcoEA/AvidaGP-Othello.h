

#ifndef EMP_ECOEA_OTHELLO_H
#define EMP_ECOEA_OTHELLO_H

#include <iostream>
#include <stdlib.h>

#include "../source/games/Othello.h"
#include "../source/hardware/OthelloGP.h"
#include "../source/hardware/InstLib.h"

using othello_ai_t = std::function< size_t(emp::Othello & game) >;

size_t POP_SIZE = 1000;
size_t GENOME_SIZE = 100;
size_t EVAL_TIME = 3500;
size_t UPDATES = 20000;
size_t TIME = 1;

constexpr size_t TOURNY_SIZE = 4;
constexpr size_t BOARD_SIZE = 8;


bool FileExists(const std::string &filename)
{
  std::ifstream ifile(filename.c_str());
  return (bool)ifile;
}

bool LoadConfig(std::string path = "run.cfg")
{
  std::ifstream cfg_file;
  std::string param;
  size_t setting;
  emp::vector<size_t> setting_list;

  if (FileExists(path))
  {
    cfg_file.open(path);
    while (cfg_file >> param){
      cfg_file >> setting;
      setting_list.push_back(setting);
    }

    if (setting_list.size() != 5)
    {
      std::cout<<"Invalid Config file - Invalid arg count"<<std::endl;
      exit(-1);
    }

    POP_SIZE = setting_list[0];
    EVAL_TIME = setting_list[1];
    GENOME_SIZE = setting_list[2];
    UPDATES = setting_list[3];
    TIME = setting_list[4];

    cfg_file.close();
    return 1;
  }
  return 0;
}

std::vector<size_t> GetValidMoves(emp::Othello game)
{
  std::vector<size_t> moves;
  for (size_t i = 0; i < BOARD_SIZE * BOARD_SIZE; i++)
  {
    if (game.IsMoveValid(2, i))
      moves.push_back(i);
  }
  return moves;
}

// Determine the next move of an AvidaGP player.
size_t EvalMove(emp::Othello &game, emp::AvidaGP &org)
{

  // Setup the hardware with proper inputs.
  org.ResetHardware();
  size_t player = game.GetCurrPlayer();

  for (size_t i = 0; i < game.GetBoardSize() * game.GetBoardSize(); i++)
  {
    size_t tile = game.GetTile(i);
    int conv_tile;

    if (tile == player)
      conv_tile = 1;
    else if (tile == 0)
      conv_tile = 0;
    else
      conv_tile = -1;

    org.SetInput(i, conv_tile);
  }

  // Run the code.
  org.Process(EVAL_TIME);

  // Determine the chosen move.
  size_t best_move = 0; // TODO: Shoul the have to make a move?

  for (size_t i = 0; i < game.GetBoardSize() * game.GetBoardSize(); i++)
  {
    if (org.GetOutput(best_move) < org.GetOutput(i))
    {
      best_move = i;
    }
  }

  return best_move;
}

// Setup the fitness function for a whole game.
double EvalGame(emp::Random& random, othello_ai_t &player1, othello_ai_t &player2, 
                size_t first_player=1, bool verbose=false, bool rand_player=0)
{

  emp::Othello game(BOARD_SIZE, first_player); // Check to see if Black first

  size_t round = 0;
  double score;

  while (game.IsDone() == false)
  {
    // Determine the current player and their move.
    size_t player = game.GetCurrPlayer();

    auto &play_fun = (player == 1) ? player1 : player2;
    size_t best_move;

    if (rand_player && player == 2)
    {
      std::vector<size_t> validMoves = GetValidMoves(game);
      int move_idx = random.GetInt(0, validMoves.size());
      best_move = validMoves[move_idx];
    }
    else
    {
      best_move = play_fun(game);
    }

    if (verbose)
    {
      std::cout << "player: " << player << std::endl;
      std::cout << "round = " << round << std::endl;
      game.Print();
      std::cout << "Move = " << best_move << std::endl;
      if (game.GetTile(best_move) != 0)
      {
        std::cout << " (illegal!)";
      }
      std::cout << std::endl << player << std::endl;
    }

    // If the chosen move is illegal, end the game
    if (player == 1)
    {
      //std::cout<<"best move: "<<best_move<<std::endl;
      if (game.IsMoveValid(player, best_move) == 0 || game.GetTile(best_move) != 0)
      {
        if (verbose)
        {
          std::cout << "break" << std::endl;
        }
        break;
      }
    }
    else
    {
      while (game.IsMoveValid(player, best_move) == 0 || game.GetTile(best_move) != 0)
      {
        best_move++;
        //if (verbose) {std::cout<<"Player: "<<best_move<<"  "<<player<<std::endl;}
        if (best_move >= game.GetBoardSize() * game.GetBoardSize())
        {
          best_move = 0;
        }
      }
    }

    // Do the move and determine who goes next.
    bool go_again = game.DoMove(player, best_move);
    if (!go_again)
    {
      game.SetTurn(game.GetOpponent(player));
    }
    round++;
    
  }

  score = round; // Score based on total rounds without mistake

  if (game.IsDone())
  {
      score += 100; // Bonus for completion of a game
      score += ((double)game.GetScore(1)) - ((double)game.GetScore(2));
  }

  if (verbose)
  {
      game.Print();
      std::cout << "Final scores -- Black: " << game.GetScore(1)
              << "   White: " << game.GetScore(2)
              << std::endl;
  }
  return score;
};

// Build wrappers for AvidaGP
double EvalGame(emp::Random &random, emp::AvidaGP &org0, emp::AvidaGP &org1,
                size_t first_player=1, bool verbose=false, bool rand_player=0)
{
    othello_ai_t org_fun0 = [&org0](emp::Othello &game) { return EvalMove(game, org0); };
    othello_ai_t org_fun1 = [&org1](emp::Othello &game) { return EvalMove(game, org1); };
    return EvalGame(random, org_fun0, org_fun1, first_player, verbose, rand_player);
};


#endif