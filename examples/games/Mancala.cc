//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This is an example file demonstrating Manacala.

#include <iostream>

#include "../../games/Mancala.h"

int main()
{
  emp::Mancala mancala;

  size_t move = 0;

  mancala.Print();

  while (mancala.IsDone() == false) {
    std::cout << "Move? " << std::endl;
    std::cin >> move;

    mancala.DoMove(move);

    mancala.Print();
  }
}
