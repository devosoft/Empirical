//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "../../games/Othello.h"
#include "../../hardware/AvidaGPOthello.h"
#include "../../hardware/InstLib.h"
#include "../../tools/Random.h"
#include "../../tools/memo_function.h"
#include "../../evo/World.h"
#include "../../../eco-ea-mancala/source/TestcaseSet.h"
#include <stdlib.h>
#include <utility>

constexpr size_t POP_SIZE = 200;
constexpr size_t GENOME_SIZE = 100;
constexpr size_t EVAL_TIME = 3500;
constexpr size_t UPDATES = 1500;
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
  emp::Othello game(cur_player==0); // Check to see if Black's turn
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

using input_t = emp::array<int, 64>;
using output_t = std::set<int>;

int main(int argc, char* argv[])
{
  size_t seed = 0;
  if (argc == 2) seed = std::atoi( argv[1] );

  emp::Random random;
  random.ResetSeed(seed);
  std::cout<<"SEED: "<<random.GetSeed()<<std::endl;
  //emp::evo::EAWorld<emp::AvidaGP, emp::evo::FitCacheOn> world(random, "AvidaWorld"); // FitCache on
  emp::evo::EAWorld<emp::AvidaGP> world(random, "AvidaWorld"); //FitCache off
  auto testcases = TestcaseSet<64>("../../../games/data/game_0.csv", &random);
  std::function<std::set<int>(emp::array<int, 64>)> cornerFunc = [](emp::array<int, 64> board){
    std::set<int> moves;
    emp::Othello game(0);
    game.SetBoard(board);
    game.TestOver();
    emp::vector<std::pair<int, int> > coords = game.GetMoveOptions(1);
    for (auto item : coords) {
      int move = game.GetIndex(item.first, item.second);
      if (move == 0 || move == 7 || move == 56 || move == 63) moves.insert(move);
    }
    return moves;
  };

  std::function<std::set<int>(emp::array<int, 64>)> edgeFunc = [](emp::array<int, 64> board){
    std::set<int> moves;
    emp::Othello game(0);
    game.SetBoard(board);
    game.TestOver();
    emp::vector<std::pair<int, int> > coords = game.GetMoveOptions(1);
    for (auto item : coords) {
      int move = game.GetIndex(item.first, item.second);
      if (move % 8 == 0 || move % 8 == 7 || move >= 56 || move <= 7) moves.insert(move);
    }
    return moves;
  };
  
  std::function<std::set<int>(emp::array<int, 64>)> totalColorFunc = [](emp::array<int, 64> board){
    std::set<int> moves;
    emp::Othello game(0);
    game.SetBoard(board);
    game.TestOver();
    size_t player = 1;
    int max_score = game.GetScore(1);
    int best_move = 0;
    emp::vector<std::pair<int, int> > coords = game.GetMoveOptions(1);
    for (auto item : coords) {
      size_t move = game.GetIndex(item.first, item.second);
      game.DoMove(player, item, 0);
      if (game.GetScore(1) > max_score) { max_score = game.GetScore(1); best_move = move; moves.clear();}
      else if (game.GetScore(1) == max_score) { moves.insert(move);}
      game.SetBoard(board);
      game.ClearValidMoves();
      game.ClearFlips();
      game.TestOver();
    }
    moves.insert(best_move);
    return moves;
  };

  std::function<std::set<int>(emp::array<int, 64>)> EnemyMovesFunc = [](emp::array<int, 64> board){
    std::set<int> moves;
    emp::Othello game(0);
    game.SetBoard(board);
    game.TestOver();
    size_t player = 1;
    emp::vector<std::pair<int, int> > coords = game.GetMoveOptions(1);
    int min_score = game.GetMoveOptions(2).size();
    int best_move = game.GetIndex(coords[0].first, coords[0].second);
    for (auto item : coords) {
      size_t move = game.GetIndex(item.first, item.second);
      game.DoMove(player, item, 0);
      if (game.GetMoveOptions(2).size() < min_score) { 
        min_score = game.GetMoveOptions(2).size(); moves.clear(); moves.insert(move);
      }
      else if (game.GetMoveOptions(2).size() == min_score) { moves.insert(move);}
      game.SetBoard(board);
      game.ClearValidMoves();
      game.ClearFlips();
      game.TestOver();
    }
    return moves;
  };
  
  testcases.AddGroup(cornerFunc);
  testcases.AddGroup(edgeFunc);
  testcases.AddGroup(totalColorFunc);
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

  // Setup the fitness function.
  emp::memo_function<double(emp::AvidaGP*)> fit_fun =
    [&random, &world](emp::AvidaGP * org) {
      emp::AvidaGP & rand_org = world.GetRandomOrg();
      bool cur_player = random.P(0.5);
      double best = EvalGame(*org, rand_org, cur_player);
      for (int i = 0; i < 4; i++){
        double temp = EvalGame(*org, rand_org, cur_player);
        if (temp > best) best = temp;
      }
      return best;
    };

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
    emp::vector<size_t> choices = testcases.GetValidSubset();
    for (auto org : world){
      emp::Othello game(0);
      for (size_t choice : choices){
        game.SetBoard(tests[choice].first);
        int move = EvalMove(game, *org);
        for (int i = 0; i < correct_choices.size(); i++) {
          if (correct_choices[i][choice].find(move) != correct_choices[i][choice].end()){
            scores[i]++;
          }
        }
      }
      for (int i = 0; i < correct_choices.size(); i++) {
          org->SetTrait(i, scores[i]);
      }
    }
    // Keep the best individual.
    world.EliteSelect(fit_fun, 1, 1);
    // Run a tournament for each spot.
    //world.TournamentSelect(fit_fun, TOURNY_SIZE, POP_SIZE-1);
    //fit_set.push_back(fit_fun);
    //world.LexicaseSelect(fit_set, POP_SIZE-1);
    world.EcoSelect(fit_fun, fit_set, 100, TOURNY_SIZE, POP_SIZE-1);
    //world.EcoSelectGradation(fit_fun, fit_set, 100, TOURNY_SIZE, POP_SIZE-1);
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
