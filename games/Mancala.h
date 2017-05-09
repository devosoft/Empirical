//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  A simple Malcala game state handler.

#ifndef EMP_GAME_MANCALA_H
#define EMP_GAME_MANCALA_H

#include <iostream>
#include <fstream>

#include "../base/array.h"
#include "../base/assert.h"
#include "../tools/math.h"

namespace emp {

  class Mancala {
  private:
    emp::array<int, 14> board;   // Current board state.
    bool over = false;           // Has the game ended?
    int curr_player = 0;         // Which player goes next?

  public:

    Mancala() { Reset(); }
    ~Mancala() { ; }

    void Reset() {
      for (size_t i = 0; i < board.size(); i++) { board[i] = 4; }
      board[0] = board[7] = 0;
      over = false;
      curr_player = 0;
    }

    int operator[](int i) const { return board[i]; }

    const emp::array<int, 14>& GetBoard() const { return board; }

    emp::array<int, 14> GetFlippedBoard() {
      emp::array<int, 14> flipped(board);
      for (size_t i = 0; i < 7; i++) std::swap(flipped[i], flipped[i+7]);
      return flipped;
    }

    void TestOver() {
      bool side_1_empty = true;
      bool side_2_empty = true;

      for (int i = 1; i < 7; i++) {
        if (board[i] > 0) { side_1_empty = false; }
        if (board[i+7] > 0) { side_2_empty = false; }
      }

      over = ( (!curr_player && side_1_empty) || (curr_player && side_2_empty));
    }

    // Returns bool indicating whether player can go again
    bool ChooseCell(int cell) {
      emp_assert(cell != 0 && cell != 7); // You can't choose the end cells
      emp_assert((cell < 7 && !curr_player) || (cell > 7 && curr_player));

      int stone_count = board[cell];
      int curr_cell = cell;
      size_t home_cell = 7 * (size_t) (cell > 7);  // Track own store; base on start.
      size_t skip_cell = 7 * (size_t) (cell < 7);  // Skip opponent's store; base on start.

      board[cell] = 0;

      while (stone_count > 0) {
        curr_cell = (curr_cell + 1) % 14;
        if (curr_cell == skip_cell) curr_cell++;

        board[curr_cell]++;
        stone_count--;
      }

      // Go again if you ended in your store
      if (curr_cell == home_cell) {
        TestOver();
        return true;
      }

      // Capturing
      if (board[curr_cell] == 1 &&
          ((curr_cell < 7 && home_cell == 0) || (curr_cell > 7 && home_cell == 7))) {
        size_t opposite = 14 - curr_cell;
        board[home_cell] += board[opposite];
        board[opposite] = 0;
      }

      curr_player = (int)!curr_player;
      TestOver();
      return false;
    }

    bool IsMoveValid(int move) const {
      // Exclude never-valid moves or empty pits
      if (move >= 13 || move <= 0 || !board[move]) { return false; }

      // Check if the current player is allowed to make this move.
      if (!curr_player && move < 7) { return true; }
      if (curr_player && move > 7) { return true; }
      return false;
    }

    bool IsOver() const { return over; }


    void PrintBoard(std::ostream & os=std::cout) {
      os << "  ";
      for (int i = 6; i > 0; i--) {
        os << board[i] << " ";
      }
      os << std::endl;
      os << board[7] << "              " << board[0] << std::endl;
      os << "  ";
      for (int i = 8; i < 14; i++) {
        os << board[i] << " ";
      }
      os << std::endl;
    }

    int GetWinner() {
      int player1 = board[0];
      int player0 = board[7];

      for (int i = 1; i < 7; i++) { player0 += board[i]; }
      for (int i = 8; i < 14; i++) { player1 += board[i]; }

      return (int) (player1 > player0);
    }

    int GetCurrPlayer() const { return curr_player; }
    bool IsTurnA() const { return curr_player == 0; }
    bool IsTurnB() const { return curr_player == 1; }

    size_t ScoreA() const {
      int score = board[7];
      for (size_t i = 1; i < 7; i++) {
        score += board[i];
      }
      return score;
    }

    size_t ScoreB(int player) const {
      int score = board[0];
      for (size_t) i = 8; i < 14; i++) {
        score += board[i];
      }
      return score;
    }

  };

}

#endif
