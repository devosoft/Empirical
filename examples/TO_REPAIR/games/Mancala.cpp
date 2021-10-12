//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This is an example file demonstrating Manacala.

#include <iostream>

#include "emp/games/Mancala.hpp"

int main()
{
  emp::Mancala mancala;

  size_t move = 0;

  mancala.Print();

  while (mancala.IsDone() == false) {
    size_t player = mancala.GetCurPlayer();
    char symbol = 'A' + (char) player;
    std::cout << symbol << " move? " << std::endl;
    std::cin >> move;

    if (move < 1 || move > 6) {
      std::cout << "Invalid move!! (choose a value 1 to 6)" <<  std::endl;
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      continue;
    }
    move += player * 7;

    if (mancala[move] == 0) {
      std::cout << "Error: Cannot move from empty pit!" << std::endl;
      continue;
    }

    mancala.DoMove(move);

    mancala.Print();
  }

  const size_t scoreA = mancala.ScoreA();
  const size_t scoreB = mancala.ScoreB();

  if (scoreA == scoreB) { std::cout << "TIE GAME!!" << std::endl; }
  else if (scoreA > scoreB) { std::cout << "A Wins!!" << std::endl; }
  else { std::cout << "B Wins!!" << std::endl; }
}
