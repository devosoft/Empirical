//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This is an example file demonstrating Othello.

#include "../../source/games/Othello.h"

size_t BOARD_SIZE = 8;

using space_t = emp::Othello::BoardSpace;

int main()
{
  // Make a new othello board.
  emp::Othello othello(BOARD_SIZE);
  std::cout << "---- Initial Board ----" << std::endl;
  othello.Print();
  std::cout << "-----------------------" << std::endl;


  while (!othello.IsOver()) {
    size_t player = othello.GetCurPlayer();
    space_t disk_type = othello.GetDiskType(player);
    if (disk_type == emp::Othello::DarkDisk()) std::cout << "Dark player's turn!" << std::endl;
    else std::cout << "Light player's turn!" << std::endl;

    std::string raw_move;
    std::cin >> raw_move;

    int moveX = 0;
    int moveY = 0;

    // Convert move into a useful format
    if (!std::isdigit(raw_move[0])) {
      moveX = std::tolower(raw_move[0]) - 96 - 1;
      moveY = raw_move[1] - 48;
    }
    else if (!std::isdigit(raw_move[1])){
      moveX = std::tolower(raw_move[1]) - 96 - 1;
      moveY = raw_move[0] - 48;
    }

    std::cout << "(move: " << moveX << ", " << moveY << ")" << std::endl;

    // Is input a valid position?
    if (!othello.IsValidPos(moveX, moveY)) {
      std::cout << "Invalid position!" << std::endl;
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      continue;
    }

    if (!(othello.GetPosValue(moveX, moveY) == emp::Othello::OpenSpace())) {
      std::cout << "Invalid move! Cannot move to non-empty position!" << std::endl;
      continue;
    }
    if (!othello.IsMoveValid(player, moveX, moveY)) {
      std::cout << "Invalid move! Must flank at least one opponent disk." << std::endl;
      continue;
    }

    othello.DoMove(player, moveX, moveY);
    std::cout << "------- Board -------" << std::endl;
    othello.Print();
    std::cout << "Scores: {DARK: " << othello.GetScore(emp::Othello::DarkPlayerID()) << " , LIGHT: " << othello.GetScore(emp::Othello::LightPlayerID()) << " }" << std::endl;
    std::cout << "---------------------" << std::endl;
  }

  double dscore = othello.GetScore(emp::Othello::DarkPlayerID());
  double lscore = othello.GetScore(emp::Othello::LightPlayerID());
  if (dscore == lscore) { std::cout << "Tie!" << std::endl; }
  else if (dscore > lscore) { std::cout << "Dark wins!" << std::endl; }
  else { std::cout << "Light wins!" << std::endl; }

  return 0;
}
