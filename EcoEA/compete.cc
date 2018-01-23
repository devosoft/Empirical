#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <utility>
#include <string>
#include <cctype>
#include <sstream>
#include <vector>

#include "../source/games/Othello.h"
#include "../source/hardware/OthelloGP.h"
#include "../source/hardware/InstLib.h"
#include "../source/tools/Random.h"

constexpr size_t EVAL_TIME = 3500;
constexpr size_t BOARD_SIZE = 8;

// Determine the next move of an AvidaGP player.
size_t EvalMove(emp::Othello & game, emp::AvidaGP & org) {
  // Setup the hardware with proper inputs.
  org.ResetHardware();

  size_t player = game.GetCurrPlayer();
  //std::cout<<std::endl;

  for (size_t i = 0; i < BOARD_SIZE * BOARD_SIZE; i++) {
    size_t tile = game.GetTile(i);
    int conv_tile;

    if (tile == player) conv_tile = 1;
    else if (tile == 0) conv_tile = 0;
    else conv_tile = -1;
    //if (i % BOARD_SIZE == 0){std::cout<<std::endl;}
    //std::cout<<conv_tile<<" ";
    org.SetInput(i, conv_tile);
  }

  // Run the code.
  org.Process(EVAL_TIME);

  // Determine the chosen move.
  size_t best_move = 0;
  //std::cout<<"MOVE begins "<<best_move<<std::endl;

  for (size_t i = 0; i < BOARD_SIZE * BOARD_SIZE; i++) {
    //std::cout<<i<<" : "<<org.GetOutput(best_move)<<std::endl;
    if (org.GetOutput(best_move) < org.GetOutput(i)) { best_move = i; }
  }

  return best_move;
}

using othello_ai_t = std::function< size_t(emp::Othello & game) >;

struct stats {
  size_t winner = -1;
  bool invalid = 0;
};

std::vector<size_t> GetValidMoves(emp::Othello game){
  std::vector<size_t> moves;
  for (size_t i = 0; i < BOARD_SIZE * BOARD_SIZE; i++){
    if(game.IsMoveValid(2, i)) moves.push_back(i);
  }
  return moves;
}

// Setup the fitness function for a whole game.
stats EvalGame(emp::Random& random, othello_ai_t & player0, othello_ai_t & player1,
                size_t first_player=1, bool verbose=false, bool rand_player=1) {

  emp::Othello game(BOARD_SIZE, first_player); // Check to see if Black goes first
  size_t round = 0;
  stats statsM;

  while (game.IsDone() == false) {
    // Determine the current player and their move.
    size_t player = game.GetCurrPlayer();

    auto & play_fun = (player == 1) ? player0 : player1;
    size_t best_move = play_fun(game);
    if (rand_player && player == 2){
      std::vector<size_t> validMoves = GetValidMoves(game);
      int move_idx = random.GetInt(0, validMoves.size());
      best_move = validMoves[move_idx];
    }

    if (verbose) {
      std::cout<<"player: "<<player<<std::endl;
      std::cout << "round = " << round << std::endl;
      game.Print();
      std::cout << "Move = " << best_move << std::endl;//best.first + " " + best.second<<std::endl;
      if (game.GetTile(best_move) != 0) {
        std::cout << " (illegal!)";
      }
      std::cout << std::endl <<player<< std::endl;
    }

    // If the chosen move is illegal, end the game
    // if (player == 1){
    //   //std::cout<<"best move: "<<best_move<<std::endl;
    //   if (game.IsMoveValid(player, best_move) == 0 || game.GetTile(best_move) != 0){
    //     if (verbose){std::cout<<"break"<<std::endl;}
    //     statsM.invalid = 1;
    //     statsM.winner = game.GetOpponent(player);
    //     break;
    //   }
    // }
    // else{
    //   while (game.IsMoveValid(player, best_move) == 0 || game.GetTile(best_move) != 0){
    //     best_move++;
    //     //if (verbose) {std::cout<<"Player: "<<best_move<<"  "<<player<<std::endl;}
    //     if (best_move >= BOARD_SIZE*BOARD_SIZE) { best_move = 0; }
    //   }
    // }

    //std::cout<<"best move: "<<best_move<<std::endl;
    if (game.IsMoveValid(player, best_move) == 0 || game.GetTile(best_move) != 0){
      if (verbose){std::cout<<"break"<<std::endl;}
      statsM.invalid = 1;
      std::cout<<"round: "<<round<<" player: "<<player<<std::endl;
      statsM.winner = game.GetOpponent(player);
      break;
    }

    // Do the move and determine who goes next.
    bool go_again = game.DoMove(player, best_move);
    if (!go_again) {game.SetTurn(game.GetOpponent(player));}
    round++;
  }

  if (verbose) {
    game.Print();
    std::cout << "Final scores -- Black: " << game.GetScore(1)
              << "   White: " << game.GetScore(2)
              << std::endl;
  }

  if (statsM.invalid) return statsM;
  if (game.GetScore(1) > game.GetScore(2)) statsM.winner = 1;
  else if (game.GetScore(2) > game.GetScore(1)) statsM.winner = 2;
  else statsM.winner = 0;

  return statsM;
};

// Build wrappers for AvidaGP
stats EvalGame(emp::Random& random, emp::AvidaGP & org0, emp::AvidaGP & org1, size_t first_player=1, bool verbose=false) {
  othello_ai_t org_fun0 = [&org0](emp::Othello & game){ return EvalMove(game, org0); };
  othello_ai_t org_fun1 = [&org1](emp::Othello & game){ return EvalMove(game, org1); };
  return EvalGame(random, org_fun0, org_fun1, first_player, verbose);
};

emp::AvidaGP load_org(std::string org_filename){
  std::string line;
  std::string instruction;
  std::ifstream file_org;
  std::vector<int> args;
  std::vector<std::string> id;
  emp::AvidaGP org;

  file_org.open(org_filename);
  if (!file_org) {std::cout<<"Invalid file "<<org_filename<<std::endl; std::exit(-1);}

  while(std::getline(file_org, line)){
    std::istringstream iss(line);
    bool bad_line = 0;
    while(iss >> instruction){
      if (std::isdigit(instruction[0])) args.push_back(std::stoi(instruction));
      else if (std::isalpha(instruction[0])) id.push_back(instruction);
      else if (instruction == "-->") continue;
      else { bad_line = 1; break; }
    }

    if (bad_line){
      //std::cout<<line<<" is bad"<<std::endl;
      continue;
    }

    int len = args.size();

    if (len == 1) org.PushInst(id[0], args[0]);
    else if (len == 2) org.PushInst(id[0], args[0], args[1]);
    else if (len == 3) org.PushInst(id[0], args[0], args[1], args[2]);
    else{
      std::cout<<"Invalid number of arguments: "<<len<<" "<<id[0]<<std::endl;
      std::exit(-1);
    }

    id.clear();
    args.clear();
  }
  return org;
}

int main(int argc, char* argv[]){
  std::string org1_type = argv[1];
  int num_org1 = std::atoi(argv[2]);
  std::string org2_type = argv[3];
  int num_org2 = std::atoi(argv[4]);
  int competitions = std::atoi(argv[5]);
  int seed = std::atoi(argv[6]);

  std::string path = "./compete_orgs/";
  std::string filename = "othello_save";
  std::string extension = ".org";

  emp::Random random;
  random.ResetSeed(seed);

  int total_wins_1, total_wins_2, invalid_games, ties = 0;
  for (int comp_num = 0; comp_num < competitions; comp_num++){
    int org1_id = random.GetInt(0, num_org1);
    int org2_id = random.GetInt(0, num_org2);
    std::string org1_filename = path + org1_type + "/" + filename + std::to_string(org1_id) + extension;
    std::string org2_filename = path + org2_type + "/" + filename + std::to_string(org2_id) + extension;

    emp::AvidaGP org1 = load_org(org1_filename);
    emp::AvidaGP org2 = load_org(org2_filename);

    stats results = EvalGame(random, org1, org2, 1, false);

    //if (results.invalid == 0) std::cout<< comp_num << " : " << org1_id <<","<<org2_id<< " : "<<results.invalid<<" Winner: "<<results.winner<<std::endl;
    //results = EvalGame(random, org1, org2, 1, false);
    //if (results.invalid == 0) std::cout<< comp_num << " :: " << org1_id <<","<<org2_id<< " : "<<results.invalid<<std::endl<<std::endl;

    if (results.invalid) {
      ++invalid_games;
      continue;
    }

    if (results.winner == 1) ++total_wins_1;
    else if (results.winner == 2) ++total_wins_2;
    else if (results.winner == 0) ++ties;

  }

  std::cout<<org1_type<<": "<<total_wins_1<<" Random: "<<total_wins_2
  <<" Ties: "<<ties<<" Invalid: "<<invalid_games<<std::endl;

  //std::cout<<org1_type + " "<<num_org1<<" " + org2_type + " "<<num_org2<<std::endl;
  //std::cout<<competitions;


  return 0;
}
