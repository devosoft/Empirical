//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include <experimental/filesystem>

#include "../../games/Mancala.h"
#include "../../hardware/AvidaGP.h"
#include "../../hardware/InstLib.h"
#include "../../tools/Random.h"
#include "../../evo/World.h"

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
  // org.PrintState();
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

  return ((double) game.ScoreA()) - ((double) game.ScoreB()) - ((double) errors * 0.0);
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

emp::AvidaGP LoadOrgFromFile(std::string filepath) {
    // std::cout << filepath << std::endl;
    std::ifstream file(filepath);
    emp::AvidaGP ai;
    emp::Ptr<const emp::InstLib<emp::AvidaGP> > lib = ai.GetInstLib();

    std::string line;
    while (std::getline(file, line)) {
        // std::cout << line << std::endl;
        if (!emp::has_alphanumeric(line)) {
            continue;
        }
        emp::remove_punctuation(line);
        emp::right_justify(line);
        emp::left_justify(line);
        emp::vector<std::string> command = emp::slice(line, ' ');

        if (command.size() == 0) {
            continue;
        }

        if (!lib->IsInst(command[0])) {
            std::cout << "Unknown instruction " << command[0] << std::endl;
            std::cout << ">> ";
            continue;
        }

        emp::vector<std::string> string_args(command.begin()+1, command.end());
        emp::vector<int> args = emp::from_strings<int>(string_args);
        // There's probably a clever way to do this, but I'm not going to try right now
        switch(args.size()) {
        case 0:
            ai.PushInst(command[0]);
            break;
        case 1:
            ai.PushInst(command[0], args[0]);
            break;
        case 2:
            ai.PushInst(command[0], args[0], args[1]);
            break;
        case 3:
            ai.PushInst(command[0], args[0], args[1], args[2]);
            break;
        default:
            std::cout << "Not implemented error. This inst has more than 3 args." <<
                         " Fix the swtich statement in the interpreter" << std::endl;
        }
    }

    return ai;
}

namespace fs = std::experimental::filesystem;

int main()
{
  emp::Random random;
  emp::vector<std::string> orgs;
  for (auto& p: fs::directory_iterator(".")){
      if (fs::is_directory(p.status())) {
          for (auto& p2 : fs::directory_iterator(p)) {
              if (fs::is_directory(p2.status())) {
                  for (auto& p3 : fs::directory_iterator(p2)) {
                    //   std::cout << "p3: " << p3.path() << std::endl;
                      emp::vector<std::string> vec_path = emp::slice(p3.path(), '/');
                    //   std::cout << "filename: " << vec_path[vec_path.size()-1] << std::endl;
                      if (vec_path[vec_path.size()-1] == "mancala_save.org") {
                          orgs.push_back(p3.path());
                      }
                  }
              }
          }
      }
  }
  emp::vector<double> scores;

  for (std::string org1 : orgs) {
    double score = 0;
    for (std::string org2 : orgs) {
        emp::AvidaGP ai1 = LoadOrgFromFile(org1);
        emp::AvidaGP ai2 = LoadOrgFromFile(org2);
        score +=  EvalGame(ai1, ai2, 0) > 0;
        score +=  EvalGame(ai1, ai2, 1) > 0;
        // std::cout << s << std::endl;
    }
    // scores.push_back(score);
    std::cout << org1 << " " << score << std::endl;
  }
  return 0;
}
