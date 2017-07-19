//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>
#include <valarray>

#include "../../games/Mancala.h"
#include "../../hardware/AvidaGP.h"
#include "../../hardware/InstLib.h"
#include "../../tools/Random.h"
#include "../../evo/World.h"

#include "../../../eco-ea-mancala/source/TestcaseSet.h"

constexpr size_t POP_SIZE = 200;
constexpr size_t GENOME_SIZE = 100;
constexpr size_t EVAL_TIME = 500;
constexpr size_t UPDATES = 1000;
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
  emp::evo::EAWorld<emp::AvidaGP> world(random, "AvidaWorld");

  // Build a random initial popoulation.
  for (size_t i = 0; i < POP_SIZE; i++) {
    emp::AvidaGP cpu;
    cpu.PushRandom(random, GENOME_SIZE);
    world.Insert(cpu);
  }

  TestcaseSet<14> t = TestcaseSet<14>("extra_move_testcases.csv", &random);

  // Extra moves
  std::function<std::set<int>(emp::array<int, 14>)> f1 =
      [](emp::array<int,14> test_case){
      std::set<int> s = std::set<int>();
      for (int i = 0; i < 6; i++) {
          if (test_case[i]%13 == 6-i) {
              s.insert(i);
          }
      }
      return s;
  };

  // Extra move from pit 0
  std::function<std::set<int>(emp::array<int, 14>)> f2 =
      [](emp::array<int,14> test_case){
      std::set<int> s = std::set<int>();
      if (test_case[0]%13 == 6) {
          s.insert(0);
      }
      return s;
  };

  // Extra move from pit 1
  std::function<std::set<int>(emp::array<int, 14>)> f3 =
      [](emp::array<int,14> test_case){
      std::set<int> s = std::set<int>();
      if (test_case[1]%13 == 5) {
          s.insert(1);
      }
      return s;
  };

  // Extra move from pit 2
  std::function<std::set<int>(emp::array<int, 14>)> f4 =
      [](emp::array<int,14> test_case){
      std::set<int> s = std::set<int>();
      if (test_case[2]%13 == 4) {
          s.insert(2);
      }
      return s;
  };

  // Extra move from pit 3
  std::function<std::set<int>(emp::array<int, 14>)> f5 =
      [](emp::array<int,14> test_case){
      std::set<int> s = std::set<int>();
      if (test_case[3]%13 == 3) {
          s.insert(3);
      }
      return s;
  };

  // Extra move from pit 4
  std::function<std::set<int>(emp::array<int, 14>)> f6 =
      [](emp::array<int,14> test_case){
      std::set<int> s = std::set<int>();
      if (test_case[4]%13 == 2) {
          s.insert(4);
      }
      return s;
  };

  // Extra move from pit 5
  std::function<std::set<int>(emp::array<int, 14>)> f7 =
      [](emp::array<int,14> test_case){
      std::set<int> s = std::set<int>();
      if (test_case[5]%13 == 1) {
          s.insert(5);
      }
      return s;
  };


  // Capturing
  std::function<std::set<int>(emp::array<int, 14>)> f8 =
      [](emp::array<int,14> test_case){
      std::set<int> s = std::set<int>();
      for (int i = 0; i < 6; i++) {
          if (test_case[i] == 0 && test_case[12-i] != 0) {
              for (int j = 0; j < i; j++){
                  if (test_case[j] == i - j) {
                      s.insert(j);
                  }
              }
              for (int j = i+1; j < 6; j++){
                  if (test_case[j] == 13 + i - j) {
                      s.insert(j);
                  }
              }
          }
      }
      return s;
  };

  // Capturing pit 0
  std::function<std::set<int>(emp::array<int, 14>)> f9 =
      [](emp::array<int,14> test_case){
      std::set<int> s = std::set<int>();
      if (test_case[0] == 0 && test_case[12] != 0) {
          for (int i = 1; i < 6; i++){
              if (test_case[i] == 13 - i) {
                  s.insert(i);
              }
          }

      }
      return s;
  };


  // Capturing pit 1
  std::function<std::set<int>(emp::array<int, 14>)> f10 =
      [](emp::array<int,14> test_case){
      std::set<int> s = std::set<int>();
      if (test_case[1] == 0 && test_case[11] != 0) {
          if (test_case[0] == 1) {
              s.insert(0);
          }
          for (int i = 2; i < 6; i++){
              if (test_case[i] == 14 - i) {
                  s.insert(i);
              }
          }

      }
      return s;
  };

  // Capturing pit 2
  std::function<std::set<int>(emp::array<int, 14>)> f11 =
      [](emp::array<int,14> test_case){
      std::set<int> s = std::set<int>();
      if (test_case[2] == 0 && test_case[10] != 0) {
          for (int i = 0; i < 2; i++){
              if (test_case[i] == 2 - i) {
                  s.insert(i);
              }
          }
          for (int i = 3; i < 6; i++){
              if (test_case[i] == 15 - i) {
                  s.insert(i);
              }
          }

      }
      return s;
  };

  // Capturing pit 3
  std::function<std::set<int>(emp::array<int, 14>)> f12 =
      [](emp::array<int,14> test_case){
      std::set<int> s = std::set<int>();
      if (test_case[3] == 0 && test_case[9] != 0) {
          for (int i = 0; i < 3; i++){
              if (test_case[i] == 3 - i) {
                  s.insert(i);
              }
          }
          for (int i = 4; i < 6; i++){
              if (test_case[i] == 16 - i) {
                  s.insert(i);
              }
          }

      }
      return s;
  };

  // Capturing pit 4
  std::function<std::set<int>(emp::array<int, 14>)> f13 =
      [](emp::array<int,14> test_case){
      std::set<int> s = std::set<int>();
      if (test_case[4] == 0 && test_case[8] != 0) {
          for (int i = 0; i < 4; i++){
              if (test_case[i] == 4 - i) {
                  s.insert(i);
              }
          }
          for (int i = 5; i < 6; i++){
              if (test_case[i] == 17 - i) {
                  s.insert(i);
              }
          }

      }
      return s;
  };

  // Capturing pit 5
  std::function<std::set<int>(emp::array<int, 14>)> f14 =
      [](emp::array<int,14> test_case){
      std::set<int> s = std::set<int>();
      if (test_case[5] == 0 && test_case[7] != 0) {
          for (int i = 0; i < 5; i++){
              if (test_case[i] == 5 - i) {
                  s.insert(i);
              }
          }
      }
      return s;
  };


  t.AddGroup(f1);
  t.AddGroup(f2);
  t.AddGroup(f3);
  t.AddGroup(f4);
  t.AddGroup(f5);
  t.AddGroup(f6);
  t.AddGroup(f7);
  t.AddGroup(f8);
  t.AddGroup(f9);
  t.AddGroup(f10);
  t.AddGroup(f11);
  t.AddGroup(f12);
  t.AddGroup(f13);
  t.AddGroup(f14);

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

  emp::vector<size_t> choices;
  emp::vector<int> best_possible;

  for (int i = 0; i < t.GetNFuncs(); i++) {
      best_possible.push_back(i);
  }

  emp::vector< std::function<double(emp::AvidaGP*)> > fit_set(t.GetNFuncs());
  for (size_t fun_id = 0; fun_id < t.GetNFuncs(); fun_id++) {
    // Setup the fitness function.
    fit_set[fun_id] = [fun_id, &best_possible](emp::AvidaGP * org) {
      return org->GetTrait(fun_id)/best_possible[fun_id];
    };
  }


  std::function<void(emp::AvidaGP*)> calc_resources = [&choices, &t](emp::AvidaGP * org){
      emp::Mancala game(0);
      auto test_cases = t.GetTestcases();
      auto correct_choices = t.GetCorrectChoices();
      emp::vector<int> scores;

      for (int i = 0; i < correct_choices.size(); i++) {
          scores.push_back(0);
      }

      for (int choice : choices) {
          emp::array<size_t, 7> sideA;
          emp::array<size_t, 7> sideB;
          for (int i = 0; i < 7; i++) {
              sideA[i] = test_cases[choice].first[i];
              sideB[i] = test_cases[choice].first[i+7];
          }

          game.SetBoard(sideA, sideB);
          int move = EvalMove(game, *org);

          for (int i = 0; i < correct_choices.size(); i++) {
              if (correct_choices[i][choice].find(move) != correct_choices[i][choice].end()) {
                  scores[i]++;
              }
          }
      }

      for (int i = 0; i < correct_choices.size(); i++) {
          org->SetTrait(i, scores[i]);
      }

  };



  // Do the run...
  for (size_t ud = 0; ud < UPDATES; ud++) {
    choices = t.GetValidSubset();
    best_possible = t.GetBestPossible(choices);

    for (auto org : world) {
        calc_resources(org);
    }

    // Keep the best individual.
    world.EliteSelect(fit_fun, 1, 1);

    // Run a tournament for each spot.
    // world.TournamentSelect(fit_fun, TOURNY_SIZE, POP_SIZE-1);
    // world.LexicaseSelect(fit_set, POP_SIZE-1);
    world.EcoSelectGradation(fit_fun, fit_set, 100, TOURNY_SIZE, POP_SIZE-1);
    world.Update();
    std::cout << (ud+1) << " : " << 0 << " : " << fit_fun(&(world[0])) << std::endl;

    // Mutate all but the first organism.
    world.MutatePop(1);
  }

  fit_fun(&(world[0]));

  std::cout << std::endl;
  emp::Mancala game(0);
  world[0].PrintGenome("mancala_save.org");

  game.DoMove(0);
  world[0].ResetHardware();
  world[0].SetInputs(game.AsInput(game.GetCurPlayer()));
  world[0].Trace(1);

  game.DoMove(5);
  world[0].ResetHardware();
  world[0].SetInputs(game.AsInput(game.GetCurPlayer()));
  world[0].Trace(1);


  // EvalGame(world[0], world[1], 0, true);
  //
  // // And try playing it!
  // while (true) {
  //   std::cout << "NEW GAME: Human vs. AI!\n";
  //   EvalGame(world[0]);
  // }

  return 0;
}
