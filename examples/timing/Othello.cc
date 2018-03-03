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
#include "tools/Random.h"

int main()
{
  emp::Random random;

  emp::Othello::Player player1 = emp::Othello::Player::DARK;
  emp::Othello::Player player2 = emp::Othello::Player::LIGHT;

  constexpr size_t NUM_BOARDS = 10;
  constexpr size_t NUM_LOOPS = 10000;
  size_t count = 0;

  // Setup the boards.
  std::array<emp::Othello, NUM_BOARDS> games;
  for (size_t board_id = 0; board_id < NUM_BOARDS; board_id++) {
    // Make a bunch of moves on each board!
    for (size_t i = 0; i < 30; i++) {
      auto moves = games[board_id].GetMoveOptions();
      if (moves.size() == 0) break;
      games[board_id].DoNextMove(moves[random.GetUInt(moves.size())]);
    }
  }


  // START TIMER!
  std::clock_t base_start_time = std::clock();

  for (size_t board_id = 0; board_id < NUM_BOARDS; board_id++) {
    for (size_t test = 0; test < NUM_LOOPS; test++) {
      for (size_t i = 0; i < 64; i++) {
        if (games[board_id].GetFlipList(player1,i).size() == games[board_id].GetFlipCount(player1,i)) {
          count++;
        } else {
          std::cout << "Oh oh... didn't match!" << std::endl;
        }
        if (games[board_id].GetFlipList(player2,i).size() == games[board_id].GetFlipCount(player2,i)) {
          count++;
        } else {
          std::cout << "Oh oh... didn't match!" << std::endl;
        }
      }
    }
  }

  std::clock_t base_tot_time = std::clock() - base_start_time;
  std::cout << "count = " << count
            << ";  time = " << 1000.0 * ((double) base_tot_time) / (double) CLOCKS_PER_SEC
            << " ms." << std::endl;

}
