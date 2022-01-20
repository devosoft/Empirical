/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file Othello.cpp
 *  @brief Code examining the speed of an Othello board.
 */

#include <algorithm>     // For std::sort
#include <ctime>         // For std::clock
#include <memory>        // For std::shared_ptr
#include <vector>

#include "emp/games/Othello8.hpp"
#include "emp/games/Othello.hpp"
#include "emp/math/Random.hpp"

int main()
{
  emp::Random random;

  constexpr size_t NUM_BOARDS = 100;
  constexpr size_t NUM_LOOPS = 1000;
  size_t count = 0;

  // Setup the boards.
  std::array<emp::Othello, NUM_BOARDS> games;
  std::array<emp::Othello8, NUM_BOARDS> games8;

  games[0].Print();
  std::cout << std::endl;
  games8[0].Print();

  for (size_t board_id = 0; board_id < NUM_BOARDS; board_id++) {
    // Make a bunch of moves on each board!
    size_t num_moves = random.GetUInt(10,60);
    for (size_t i = 0; i < num_moves; i++) {
      auto moves = games[board_id].GetMoveOptions();
      if (moves.size() == 0) { board_id--; break; }
      size_t next_move = moves[random.GetUInt(moves.size())];
      bool again = games[board_id].DoNextMove(next_move);
      bool again8 = games8[board_id].DoNextMove(next_move);
      if (again != again8) std::cout << "Ack!  Game boards do not agree!" << std::endl;
    }
  }

  games[0].Print();
  std::cout << std::endl;
  games8[0].Print();

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

  // RESTART TIMER!
  base_start_time = std::clock();

  {
    emp::Othello8::Player player1 = emp::Othello8::Player::DARK;
    emp::Othello8::Player player2 = emp::Othello8::Player::LIGHT;
    count = 0;
    for (size_t board_id = 0; board_id < NUM_BOARDS; board_id++) {
      games8[board_id].SetupCache();
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
  std::cout << "Othello8 count (with Caching) = " << count
            << ";  time = " << 1000.0 * ((double) base_tot_time) / (double) CLOCKS_PER_SEC
            << " ms." << std::endl;
}
