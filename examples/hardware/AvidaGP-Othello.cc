//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>
#include <stdlib.h>
#include <utility>
#include <string>
#include <chrono>

#include "../../games/Othello.h"
#include "../../hardware/AvidaGPOthello.h"
#include "../../hardware/InstLib.h"
#include "../../tools/Random.h"
#include "../../tools/memo_function.h"
#include "../../evo/World.h"
#include "../../../eco-ea-mancala/source/TestcaseSet.h"

using move_t = size_t;

size_t POP_SIZE = 1000;
constexpr size_t GENOME_SIZE = 100;
size_t EVAL_TIME = 3500;
size_t UPDATES = 2000;
constexpr size_t TOURNY_SIZE = 4;
constexpr size_t BOARD_SIZE = 8;


// Determine the next move of a human player.

// size_t EvalMove(emp::Othello & game, std::ostream & os=std::cout, std::istream & is=std::cin) {
//   // Present the current board.
//   game.Print();
//
//   // Request a move from the human.
//   std::string raw_move;
//   std::cin >> raw_move;
//
//   int moveX = 0;
//   int moveY = 0;
//
//   // Convert move into a useful format
//   if (!std::isdigit(raw_move[0])) {
//     moveX = std::tolower(raw_move[0]) - 96;
//     moveY = raw_move[1] - 48;
//   }
//   else if (!std::isdigit(raw_move[1])){
//     moveX = std::tolower(raw_move[1]) - 96;
//     moveY = raw_move[0] - 48;
//   }
//
//   move_t move;
//   bool invalid = true;
//
//   while (invalid) {
//     move = game.GetIndex(move);
//
//     if (moveX < 1 || moveX > BOARD_SIZE) {
//       std::cout << "Invalid move!! (choose an X value A to H)" <<  std::endl;
//       std::cin.clear();
//       std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
//       continue;
//     }
//     if (moveY < 1 || moveY > BOARD_SIZE) {
//       std::cout << "Invalid move!! (choose an Y value 1 to " << BOARD_SIZE<<")" <<  std::endl;
//       std::cin.clear();
//       std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
//       continue;
//     }
//
//
//     if (game.GetTile(moveX, moveY) != 0) {
//       std::cout << "Error: Cannot move to non-empty tile" << std::endl;
//       std::cin.clear();
//       std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
//       continue;
//     }
//
//     if (game.IsMoveValid(game.GetCurrPlayer(), moveXY ) == 0) {
//         std::cout << "Invalid Move: Must flank at least one opponent disc" <<std::endl;
//         std::cin.clear();
//         std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
//         continue;
//     }
//
//     invalid = false;
//   }
//
//   size_t move = game.GetIndex(moveX, moveY);
//
//   return move;
// }


// Determine the next move of an AvidaGP player.
size_t EvalMove(emp::Othello & game, emp::AvidaGP & org) {
  // Setup the hardware with proper inputs.
  org.ResetHardware();

  size_t player = game.GetCurrPlayer();
  //std::cout<<std::endl;

  for (size_t i = 0; i < BOARD_SIZE* BOARD_SIZE; i++) {
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

  for (size_t i = 0; i < BOARD_SIZE * BOARD_SIZE; i++) {
    if (org.GetOutput(best_move) < org.GetOutput(i)) { best_move = i; }
  }

  return best_move;
}

using othello_ai_t = std::function< size_t(emp::Othello & game) >;

// Setup the fitness function for a whole game.
double EvalGame(othello_ai_t & player0, othello_ai_t & player1,
                size_t first_player=1, bool verbose=false) {

  emp::Othello game(BOARD_SIZE, first_player); // Check to see if Black goes first
  size_t round = 0;
  double score;

  while (game.IsDone() == false) {
    // Determine the current player and their move.
    size_t player = game.GetCurrPlayer();
    auto & play_fun = (player == 1) ? player0 : player1;
    size_t best_move = play_fun(game);

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
    if (player == 1){
      //std::cout<<"best move: "<<best_move<<std::endl;
      if (game.IsMoveValid(player, best_move) == 0 || game.GetTile(best_move) != 0){
        if (verbose){std::cout<<"break"<<std::endl;}
        break;
      }
    }
    else{
      while (game.IsMoveValid(player, best_move) == 0 || game.GetTile(best_move) != 0){
        best_move++;
        //if (verbose) {std::cout<<"Player: "<<best_move<<"  "<<player<<std::endl;}
        if (best_move >= BOARD_SIZE*BOARD_SIZE) { best_move = 0; }
      }
    }

    // Do the move and determine who goes next.
    bool go_again = game.DoMove(player, best_move);
    if (!go_again) {game.SetTurn(game.GetOpponent(player));}
    round++;
  }

  score = round; // Score based on total rounds without mistake

  if (game.IsDone()){
    score += 100; // Bonus for completion of a game
    score += ((double) game.GetScore(1)) - ((double) game.GetScore(2));
  }

  if (verbose) {
    game.Print();
    std::cout << "Final scores -- Black: " << game.GetScore(1)
              << "   White: " << game.GetScore(2)
              << std::endl;
  }

  return score;
};

// Build wrappers for AvidaGP
double EvalGame(emp::AvidaGP & org0, emp::AvidaGP & org1, size_t first_player=1, bool verbose=false) {
  othello_ai_t org_fun0 = [&org0](emp::Othello & game){ return EvalMove(game, org0); };
  othello_ai_t org_fun1 = [&org1](emp::Othello & game){ return EvalMove(game, org1); };
  return EvalGame(org_fun0, org_fun1, first_player, verbose);
};

// Otherwise assume a human opponent!
// double EvalGame(emp::AvidaGP & org, size_t first_player=1) {
//   othello_ai_t fun0 = [&org](emp::Othello & game){ return EvalMove(game, org); };
//   othello_ai_t fun1 = [](emp::Othello & game){ return EvalMove(game, std::cout, std::cin); };
//   std::cout<<"START"<<std::endl; //TODO
//   std::exit(-1);
//   return EvalGame(fun0, fun1, first_player, true);
// };


using input_t = emp::array<int, 64>;
using output_t = std::set<int>;

int main(int argc, char* argv[])
{
  // Set up initial world

  std::chrono::high_resolution_clock::time_point start_s = std::chrono::high_resolution_clock::now();
  POP_SIZE = std::atoi( argv[1] );
  EVAL_TIME = std::atoi( argv[2] );
  UPDATES = std::atoi( argv[3] );
  long time = 28800;

  std::cout<<"POP_SIZE: "<<POP_SIZE<<" EVAL_TIME: "<<EVAL_TIME
           <<" UPDATES: "<<UPDATES<<std::endl;

  size_t seed = 0;
  if (argc == 5) seed = std::atoi( argv[4] );
  emp::Random random;
  random.ResetSeed(seed);


  //emp::evo::EAWorld<emp::AvidaGP, emp::evo::FitCacheOn> world(random, "AvidaWorld"); // FitCache on
  emp::evo::EAWorld<emp::AvidaGP> world(random, "AvidaWorld"); //FitCache off
  auto testcases = TestcaseSet<64>("../../data/game_0.csv", &random);

  // Fitness function that encourages playing in corners
  std::function<std::set<int>(emp::array<int, 64>)> cornerFunc = [](emp::array<int, 64> board){
    std::set<int> correct_moves;
    emp::Othello game(BOARD_SIZE, 1);
    game.SetBoard(board);
    emp::vector<size_t> moves = game.GetMoveOptions(1);

    for (size_t move : moves) {
      if (move == 0 || move == 7 || move == 56 || move == 63) correct_moves.insert(move);
    }
    return correct_moves;
  };

  // Fitness function that encourages playing on edges
  std::function<std::set<int>(emp::array<int, 64>)> edgeFunc = [](emp::array<int, 64> board){
    std::set<int> correct_moves;
    emp::Othello game(BOARD_SIZE, 1);
    game.SetBoard(board);
    emp::vector<size_t> moves = game.GetMoveOptions(1);

    for (size_t move : moves) {
      if (move % 8 == 0 || move % 8 == 7 || move >= 56 || move <= 7) correct_moves.insert(move);
    }
    return correct_moves;
  };

  // Fitness function that encourages player to take move that captures the most tiles
  std::function<std::set<int>(emp::array<int, 64>)> tilesTakenFunc = [](emp::array<int, 64> board){
    emp::Othello game(BOARD_SIZE, 1);
    game.SetBoard(board);

    std::set<int> correct_moves;
    size_t player = 1;
    int max_score = game.GetScore(player);
    emp::vector<size_t> moves = game.GetMoveOptions(1);
    int best_move = 0;

    for (size_t move : moves) {
      game.DoMove(player, move);

      if (game.GetScore(1) > max_score) {
        max_score = game.GetScore(1);
        best_move = move;
        correct_moves.clear();
      }
      else if (game.GetScore(1) == max_score) { correct_moves.insert(move);}

      game.SetBoard(board);
    }

    correct_moves.insert(best_move);
    return correct_moves;
  };

  // Fitness function that encourages player to limit opponent moves
  std::function<std::set<int>(emp::array<int, 64>)> EnemyMovesFunc = [](emp::array<int, 64> board){
    emp::Othello game(BOARD_SIZE, 1);
    game.SetBoard(board);

    size_t player = 1;
    std::set<int> correct_moves;
    emp::vector<size_t> moves = game.GetMoveOptions(1);
    int min_score = game.GetMoveOptions(2).size();
    int best_move = moves[0];

    for (size_t move : moves) {
      game.DoMove(player, move);

      if (game.GetMoveOptions(2).size() < min_score) {
        min_score = game.GetMoveOptions(2).size();
        correct_moves.clear();
        correct_moves.insert(move);
      }
      else if (game.GetMoveOptions(2).size() == min_score) { correct_moves.insert(move);}

      game.SetBoard(board);
    }
    return correct_moves;
  };

  testcases.AddGroup(cornerFunc);
  testcases.AddGroup(edgeFunc);
  testcases.AddGroup(tilesTakenFunc);
  testcases.AddGroup(EnemyMovesFunc);

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

  // Setup the main fitness function.
  emp::memo_function<double(emp::AvidaGP*)> fit_fun =
    [&random, &world](emp::AvidaGP * org) {
      emp::AvidaGP & rand_org = world.GetRandomOrg();
      size_t first_player = random.GetInt(1, 3);
      double best = EvalGame(*org, rand_org, first_player);
      for (int i = 0; i < 4; i++){
          first_player = random.GetInt(1, 3);
          emp::AvidaGP & rand_org1 = world.GetRandomOrg();
        double temp = EvalGame(*org, rand_org1, first_player);
        if (temp > best) best = temp;
      }
      return best;
    };

  // Setup TestCases for secondary fitness functions
  emp::vector< std::function<double(emp::AvidaGP*)> > fit_set(testcases.GetNFuncs());
  for (size_t fun_id = 0; fun_id < testcases.GetNFuncs(); fun_id++) {
    // Setup the fitness function.
    fit_set[fun_id] = [fun_id](emp::AvidaGP * org) {
      return org->GetTrait(fun_id);
    };
  }
  auto correct_choices = testcases.GetCorrectChoices();
  emp::vector<std::pair<input_t, output_t> > tests = testcases.GetTestcases();
  emp::vector<int> scores;
  for (int i = 0; i < correct_choices.size(); i++) {
    scores.push_back(0);
  }

  // Do the run...
  for (size_t ud = 0; ud < UPDATES; ud++) {
   // emp::vector<size_t> choices = testcases.GetValidSubset();

    // for (auto org : world){
    //   emp::Othello game(BOARD_SIZE, 1); //TODO: should it be random player first?
    //
    //   for (size_t choice : choices){
    //     game.SetBoard(tests[choice].first);
    //     int move = EvalMove(game, *org);
    //
    //     for (int i = 0; i < correct_choices.size(); i++) {
    //       if (correct_choices[i][choice].find(move) != correct_choices[i][choice].end()){
    //         scores[i]++;
    //       }
    //     }
    //   }
    //   for (int i = 0; i < correct_choices.size(); i++) {
    //       org->SetTrait(i, scores[i]);
    //   }
    // }

    // Keep the best individual.
    world.EliteSelect(fit_fun, 1, 1);

    // Run a selection method for each spot.
    //world.TournamentSelect(fit_fun, TOURNY_SIZE, POP_SIZE-1); //TODO: Make states constant for selection methods
    //fit_set.push_back(fit_fun);
    //world.LexicaseSelect(fit_set, POP_SIZE-1);
    //world.EcoSelect(fit_fun, fit_set, 100, TOURNY_SIZE, POP_SIZE-1);
    world.EcoSelectGradation(fit_fun, fit_set, 100, TOURNY_SIZE, POP_SIZE-1);

    fit_fun.Clear();
    world.Update();

    std::cout << (ud+1) << " : " << 0 << " : " << fit_fun(&(world[0])) << std::endl;
    // if (ud % 10 == 0){
    //   for (int i = 0; i < POP_SIZE; i++){
    //     std::cout<<fit_fun(&(world[i]))<< " ";
    //   }
    //   std::cout<<std::endl;
    //   EvalGame(world[0], world[1], 1, true);
    // }

    // Mutate all but the first organism.
    world.MutatePop(1);

    std::chrono::high_resolution_clock::time_point stop_s = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>( stop_s - start_s ).count();
    if (duration > time){ break; }
  }

  fit_fun(&(world[0]));

  std::cout << std::endl;
  for(int i = 0; i < POP_SIZE; ++i){
    world[i].PrintGenome("orgs/othello_save" + std::to_string(i) +".org");
  }

  //EvalGame(world[0], world[1], 1, true);

  // And try playing it!
  /*
  while (true) {
    std::cout << "NEW GAME: Human vs. AI!\n";
    EvalGame(world[0]);
  }
  */

  return 0;
}
