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
#include "../base/vector.h"
#include "../tools/math.h"

namespace emp {

  class Mancala {
  private:
    emp::array<size_t, 14> board;   // Current board state.
    bool over = false;              // Has the game ended?
    size_t cur_player = 0;          // Which player goes next?

    void TestOver() {
      bool side_0_empty = true;
      bool side_1_empty = true;

      for (size_t i = 1; i < 7; i++) {
        if (board[i] > 0) { side_0_empty = false; }
        if (board[i+7] > 0) { side_1_empty = false; }
      }

      over = ( (!cur_player && side_0_empty) || (cur_player && side_1_empty));
    }

  public:
    using move_t = size_t;

    Mancala(bool first_player=0) { Reset(first_player); }
    ~Mancala() { ; }

    void Reset(bool first_player=0) {
      for (size_t i = 0; i < board.size(); i++) { board[i] = 4; }
      board[0] = board[7] = 0;
      over = false;
      cur_player = first_player;
    }

    size_t operator[](size_t i) const { return board[i]; }

    const emp::array<size_t, 14>& GetBoard() const { return board; }

    emp::array<size_t, 14> GetFlippedBoard() {
      emp::array<size_t, 14> flipped(board);
      for (size_t i = 0; i < 7; i++) std::swap(flipped[i], flipped[i+7]);
      return flipped;
    }

    // Returns bool indicating whether player can go again
    bool DoMove(move_t cell) {
      emp_assert(cell < 14);               // You cannot choose a cell out of bounds.
      emp_assert(cell != 0 && cell != 7);  // You can't choose the end cells
      emp_assert(board[cell] != 0);        // You cannot choose an empty cell.
      // It must be your turn for the cell chosen!
      emp_assert((cell < 7 && !cur_player) || (cell > 7 && cur_player), cell, cur_player);

      size_t stone_count = board[cell];
      size_t cur_cell = cell;
      size_t home_cell = 7 * (size_t) (cell < 7);  // Track own store; base on start.
      size_t skip_cell = 7 * (size_t) (cell > 7);  // Skip opponent's store; base on start.

      board[cell] = 0;

      while (stone_count > 0) {
        cur_cell = (cur_cell + 1) % 14;
        if (cur_cell == skip_cell) cur_cell++;

        board[cur_cell]++;
        stone_count--;
      }

      // Go again if you ended in your store
      if (cur_cell == home_cell) {
        TestOver();
        return true;
      }

      // Capturing
      if (board[cur_cell] == 1 &&
          ((cur_cell < 7 && home_cell == 0) || (cur_cell > 7 && home_cell == 7))) {
        size_t opposite = 14 - cur_cell;
        board[home_cell] += board[opposite];
        board[opposite] = 0;
      }

      cur_player = (size_t) !cur_player;
      TestOver();
      return false;
    }

    // Setup a DoMove from either player's viewpoint.
    bool DoMove(size_t player, move_t cell) {
      emp_assert(cell > 0 && cell < 7, cell);  // Moves can only be 1 through 6.
      if (player == 1) cell += 7;
      return DoMove(cell);
    }

    bool IsDone() const { return over; }

    bool IsMoveValid(move_t move) const {
      // Exclude never-valid moves or empty pits
      if (move >= 13 || move <= 0 || !board[move]) { return false; }

      // Check if the current player is allowed to make this move.
      if (!cur_player && move < 7) { return true; }
      if (cur_player && move > 7) { return true; }
      return false;
    }

    // Provide all of the legal
    emp::vector<move_t> GetMoveOptions() {
      emp::vector<move_t> out_v;
      for (size_t i = 1; i <= 6; i++) {
        if (board[i]) out_v.push_back(i);
      }
      return out_v;
    }

    void Print(std::ostream & os=std::cout) {
      os << "  ";
      for (size_t i = 6; i > 0; i--) {
        os << board[i] << " ";
      }
      os << std::endl;
      os << board[7] << "              " << board[0] << std::endl;
      os << "  ";
      for (size_t i = 8; i < 14; i++) {
        os << board[i] << " ";
      }
      os << std::endl;
    }

    size_t GetCurrPlayer() const { return cur_player; }
    bool IsTurnA() const { return cur_player == 0; }
    bool IsTurnB() const { return cur_player == 1; }

    size_t ScoreA() const {
      size_t score = board[7];
      for (size_t i = 1; i < 7; i++) {
        score += board[i];
      }
      return score;
    }

    size_t ScoreB() const {
      size_t score = board[0];
      for (size_t i = 8; i < 14; i++) {
        score += board[i];
      }
      return score;
    }

    double GetScore(size_t player) {
      if (player == 0) return (double) ScoreA();
      else if (player == 1) return (double) ScoreB();
      emp_assert(false);  // Only a two player game!
      return 0.0;
    }

  };

}

#endif
