//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Code examining the speed of an Othello board.


#include <algorithm>     // For std::sort
#include <ctime>         // For std::clock
#include <memory>        // For std::shared_ptr
#include <vector>

#include "games/Othello.h"

int main()
{
  emp::Othello game;
  emp::Othello::Player player1 = emp::Othello::Player::DARK;
  emp::Othello::Player player2 = emp::Othello::Player::LIGHT;
  constexpr size_t NUM_LOOPS = 100000;

  std::clock_t base_start_time = std::clock();

  size_t count = 0;
  for (size_t test = 0; test < NUM_LOOPS; test++) {
    for (size_t i = 0; i < 64; i++) {
      if (game.GetFlipList(player1,i).size() == game.GetFlipCount(player1,i)) {
	      count++;
      } else {
	      std::cout << "Oh oh... didn't match!" << std::endl;
      }
      if (game.GetFlipList(player2,i).size() == game.GetFlipCount(player2,i)) {
	      count++;
      } else {
	      std::cout << "Oh oh... didn't match!" << std::endl;
      }
    }
  }

  std::clock_t base_tot_time = std::clock() - base_start_time;
  std::cout << "count = " << count
            << ";  time = " << 1000.0 * ((double) base_tot_time) / (double) CLOCKS_PER_SEC
            << " ms." << std::endl;

}
