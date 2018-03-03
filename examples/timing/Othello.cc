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
#include "games/Othello8.h"
#include "tools/Random.h"

int main()
{
  emp::Random random;

  constexpr size_t NUM_BOARDS = 100;
  constexpr size_t NUM_LOOPS = 1000;
  size_t count = 0;

  // Setup the boards.
  std::array<emp::Othello, NUM_BOARDS> games;
  std::array<emp::Othello8, NUM_BOARDS> games8;
  for (size_t board_id = 0; board_id < NUM_BOARDS; board_id++) {
    // Make a bunch of moves on each board!
    size_t num_moves = random.GetUInt(10,50);
    for (size_t i = 0; i < num_moves; i++) {
      auto moves = games[board_id].GetMoveOptions();
      if (moves.size() == 0) { board_id--; break; }
      size_t next_move = moves[random.GetUInt(moves.size())];
      games[board_id].DoNextMove(next_move);
      games8[board_id].DoNextMove(next_move);
    }
  }

  std::cout << "Finished generating boards!" << std::endl;

  // START TIMER!
  std::clock_t base_start_time = std::clock();

  {
    emp::Othello::Player player1 = emp::Othello::Player::DARK;
    emp::Othello::Player player2 = emp::Othello::Player::LIGHT;
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
  }

  std::clock_t base_tot_time = std::clock() - base_start_time;
  std::cout << "Othello count = " << count
            << ";  time = " << 1000.0 * ((double) base_tot_time) / (double) CLOCKS_PER_SEC
            << " ms." << std::endl;


  // RESTART TIMER!
  base_start_time = std::clock();

  {
    emp::Othello8::Player player1 = emp::Othello8::Player::DARK;
    emp::Othello8::Player player2 = emp::Othello8::Player::LIGHT;
    count = 0;
    for (size_t board_id = 0; board_id < NUM_BOARDS; board_id++) {
      for (size_t test = 0; test < NUM_LOOPS; test++) {
        for (size_t i = 0; i < 64; i++) {
          if (games8[board_id].GetFlipList(player1,i).size() == games8[board_id].GetFlipCount(player1,i)) {
            count++;
          } else {
            std::cout << "Oh oh... didn't match!" << std::endl;
          }
          if (games8[board_id].GetFlipList(player2,i).size() == games8[board_id].GetFlipCount(player2,i)) {
            count++;
          } else {
            std::cout << "Oh oh... didn't match!" << std::endl;
          }
        }
      }
    }
  }

  base_tot_time = std::clock() - base_start_time;
  std::cout << "Othello8 count = " << count
            << ";  time = " << 1000.0 * ((double) base_tot_time) / (double) CLOCKS_PER_SEC
            << " ms." << std::endl;

}
