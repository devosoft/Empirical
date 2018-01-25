//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>
#include <stdlib.h>
#include <utility>
#include <string>
#include <chrono>
#include <fstream>
#include <set>

#include "AvidaGP-Othello.h"
#include "../source/games/Othello.h"
#include "../source/hardware/OthelloGP.h"
#include "../source/hardware/InstLib.h"
#include "../source/tools/Random.h"
#include "../source/tools/memo_function.h"
#include "../source/Evo/World.h"
#include "../../eco-ea-mancala/source/TestcaseSet.h"

using move_t = size_t;
using input_t = emp::array<int, 64>;
using output_t = std::set<int>;
using othello_ai_t = std::function< size_t(emp::Othello & game) >;

constexpr size_t POP_SIZE = 1000;
constexpr size_t GENOME_SIZE = 100;
constexpr size_t EVAL_TIME = 3500;
constexpr size_t UPDATES = 2000;
constexpr size_t TOURNY_SIZE = 4;
constexpr size_t BOARD_SIZE = 8;


bool FileExists(const std::string& filename) {
  std::ifstream ifile(filename.c_str());
  return (bool)ifile;
}

int main(int argc, char* argv[]){

  std::chrono::high_resolution_clock::time_point start_s = std::chrono::high_resolution_clock::now();
  // POP_SIZE = std::atoi( argv[1] );
  // EVAL_TIME = std::atoi( argv[2] ); // TODO Config files
  // UPDATES = std::atoi( argv[3] );
  long time = 28800;
  size_t seed = 0;
  std::string selection = argv[1];
  // if (argc == 5) seed = std::atoi( argv[4] );
  

  std::cout<<"POP_SIZE: "<<POP_SIZE<<" EVAL_TIME: "<<EVAL_TIME
           <<" UPDATES: "<<UPDATES<<std::endl;

  // Setting up the world
  emp::Random random;
  random.ResetSeed(seed);
  emp::World<emp::AvidaGP> world(random, "AvidaWorld");
  world.SetWellMixed(true);

  if (selection == "eco") world.SetCache(0);
  else if (selection == "tourny") world.SetCache(1);
  else { std::cout<<"Invalid Selection Method - "<< selection <<std::endl; exit(-1); }


  std::string filename = "data/game_0.csv";
  std::string otherfile = "../../../../data/game_0.csv";

  if (!FileExists(filename)){
      if (FileExists(otherfile)) filename = otherfile;
      else { 
          std::cout<<"No game data file found at " + filename + " or " + otherfile<<std::endl;
          exit(-1);
      }
  }

  auto testcases = TestcaseSet<64>(filename, &random);

  // Fitness function that encourages playing in corners TODO Move these to their own file?
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

  // Setup the main fitness function.
  std::function<double(emp::AvidaGP &)> fit_fun =
    [&random, &world](emp::AvidaGP & org) 
    {
      emp::vector<double> fit_list;

      //Take the median of 5 games as the organisms fitness
      for (int i = 0; i < 5; i++){
        int first_player = random.GetInt(1, 3);
        bool rand_player = 0;
        emp::AvidaGP & rand_org1 = world.GetRandomOrg();

        if (i > 2) rand_player = 1;

        fit_list.push_back( EvalGame(random, org, rand_org1, BOARD_SIZE, EVAL_TIME, first_player,0, rand_player) );
      }
      std::sort(fit_list.begin(), fit_list.end());

      return fit_list[2]; // Return the median
  };
  
  world.SetFitFun(fit_fun);

  // Setup TestCases for secondary fitness functions
  emp::vector<std::function<double(const emp::AvidaGP &)>> fit_set(testcases.GetNFuncs());
  auto correct_choices = testcases.GetCorrectChoices(); //TODO: Confirm this works
  emp::vector<std::pair<input_t, output_t>> tests = testcases.GetTestcases();
  emp::vector<int> scores;

  for (int i = 0; i < correct_choices.size(); i++)
  {
    scores.push_back(0);
  }

  for (size_t fun_id = 0; fun_id < testcases.GetNFuncs(); fun_id++) {
    // Create list of secondary fitness functions.
    fit_set[fun_id] = [fun_id](const emp::AvidaGP &org) {
      return org.GetTrait(fun_id);
    };
  }

  // Do the run...
  for (size_t ud = 0; ud < UPDATES; ud++) {
    world.ResetHardware();

    emp::vector<size_t> choices = testcases.GetValidSubset();
    for (emp::AvidaGP & org : world)
    {
      emp::Othello game(BOARD_SIZE, 1); //TODO: should it be random player first?

      for (size_t choice : choices){
        game.SetBoard(tests[choice].first);
        int move = EvalMove(game, org, EVAL_TIME);

        for (int i = 0; i < correct_choices.size(); i++) { //TODO: Make this into a function?
          if (correct_choices[i][choice].find(move) != correct_choices[i][choice].end()){
            scores[i]++;
          }
        }
      }

      for (int i = 0; i < correct_choices.size(); i++) {
          org.SetTrait(i, scores[i]);
      }
    }

    // Keep the best individual.
    EliteSelect(world, 1, 1);

    // Run a selection method for each spot.
    if (selection == "tourny") TournamentSelect(world, TOURNY_SIZE, POP_SIZE-1); //TODO: Make states constant for selection methods

    //fit_set.push_back(fit_fun);
    //world.LexicaseSelect(fit_set, POP_SIZE-1);
    else if (selection == "eco") EcoSelect(world, fit_set, 100, TOURNY_SIZE, POP_SIZE-1);

    world.Update();

    std::cout << (ud+1) << " : " << 0 << " : " << fit_fun(world.GetOrg(0)) << std::endl;

    // Mutate all but the first organism.
    world.DoMutations(1);

    std::chrono::high_resolution_clock::time_point stop_s = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>( stop_s - start_s ).count();
    if (duration > time){ break; }
  }

  //EvalGame(world.GetOrg(0), world.GetOrg(1), 1, true);
  world.GetOrg(0).PrintGenome("genome.org");


  return 0;
}
