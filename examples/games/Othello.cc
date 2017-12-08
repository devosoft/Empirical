//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This is an example file demonstrating Othello.

#include "../../games/Othello.h"

size_t BOARD_SIZE = 8;

int main()
{
  emp::Othello othello(BOARD_SIZE);

  othello.Print();

  while (othello.IsDone() == false) {
    size_t player = othello.GetCurrPlayer();

    if (player == 1) {std::cout << " X's Move?"<< std::endl; }
    else { std::cout<<" O's Move?"<< std::endl; }

    std::string raw_move;
    std::cin >> raw_move;

    int moveX = 0;
    int moveY = 0;

    // Convert move into a useful format
    if (!std::isdigit(raw_move[0])) {
      moveX = std::tolower(raw_move[0]) - 96;
      moveY = raw_move[1] - 48;
    }
    else if (!std::isdigit(raw_move[1])){
      moveX = std::tolower(raw_move[1]) - 96;
      moveY = raw_move[0] - 48;
    }

    if (moveX < 1 || moveX > BOARD_SIZE) {
      std::cout << "Invalid move!! (choose an X value A to H)" <<  std::endl;
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      continue;
    }
    else if (moveY < 1 || moveY > BOARD_SIZE) {
      std::cout << "Invalid move!! (choose an Y value 1 to " << BOARD_SIZE<<")" <<  std::endl;
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      continue;
    }

    size_t move = othello.GetIndex(moveX, moveY);

    if (othello.GetTile(move) != 0) {
      std::cout << "Error: Cannot move to non-empty tile" << std::endl;
      continue;
    }
    if (othello.IsMoveValid(player, move ) == 0) {
      std::cout << "Invalid Move: Must flank at least one opponent disc" << std::endl;
      continue;
    }

    bool go_again = othello.DoMove(player, move);
    if (!go_again) {
      othello.SetTurn(othello.GetOpponent(player));
    }

    othello.Print();
  }

  const size_t scoreB = othello.GetScore(1);
  const size_t scoreW = othello.GetScore(2);

  std::cout<<"Black: "<<scoreB<<" White: "<<scoreW<<std::endl<<std::endl;

  if (scoreW == scoreB) { std::cout << "TIE GAME!!" << std::endl; }
  else if (scoreW < scoreB) { std::cout << "Black Wins!!" << std::endl; }
  else { std::cout << "White Wins!!" << std::endl; }
}
