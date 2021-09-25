/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2021
 *
 *  @file Mancala.hpp
 *  @brief A simple Malcala game state handler.
 */

#ifndef EMP_GAMES_MANCALA_HPP_INCLUDE
#define EMP_GAMES_MANCALA_HPP_INCLUDE

#include <fstream>
#include <iomanip>
#include <iostream>
#include <unordered_map>

#include "../base/array.hpp"
#include "../base/assert.hpp"
#include "../base/vector.hpp"
#include "../math/math.hpp"

namespace emp {

  class Mancala {
  private:
    using side_t = emp::array<size_t, 7>;

    side_t boardA;        // Current board state for side A.
    side_t boardB;        // Current board state for side B.
    bool over = false;    // Has the game ended?
    size_t is_A_turn;     // Which player goes next?

    void TestOver() {
      bool side_A_empty = true;
      bool side_B_empty = true;

      for (size_t i = 0; i < 6; i++) {
        if (boardA[i] > 0) { side_A_empty = false; }
        if (boardB[i] > 0) { side_B_empty = false; }
      }

      over = ( (is_A_turn && side_A_empty) || (!is_A_turn && side_B_empty));
    }

  public:
    using move_t = size_t;

    Mancala(bool A_first=true) : boardA(), boardB(), over(false), is_A_turn(true) {
      Reset(A_first);
    }
    ~Mancala() { ; }

    void Reset(bool A_first=true) {
      for (size_t i = 0; i < 6; i++) { boardA[i] = 4; boardB[i] = 4; }
      boardA[6] = boardB[6] = 0;
      over = false;
      is_A_turn = A_first;
    }

    size_t GetA(size_t i) const { return boardA[i]; }
    size_t GetB(size_t i) const { return boardB[i]; }

    const side_t & GetSideA() const { return boardA; }
    const side_t & GetSideB() const { return boardB; }
    const side_t & GetCurSide() const { return is_A_turn ? boardA : boardB; }
    const side_t & GetOtherSide() const { return is_A_turn ? boardB : boardA; }
    side_t & GetSideA() { return boardA; }
    side_t & GetSideB() { return boardB; }
    side_t & GetCurSide() { return is_A_turn ? boardA : boardB; }
    side_t & GetOtherSide() { return is_A_turn ? boardB : boardA; }

    std::unordered_map<int, double> AsInput(size_t player_id) const {
      std::unordered_map<int, double> input_map;
      size_t offset = (player_id == 0) ? 0 : 7;
      for (size_t i = 0; i < 7; i++) {
        input_map[(int)(i+offset)] = (double) boardA[i];
        input_map[(int)(i+7-offset)] = (double) boardB[i];
      }

      return input_map;
    }

    emp::vector<double> AsVectorInput(size_t player_id) const {
      emp::vector<double> input_map(14);                   // 14 bins, 7 for each player.
      const size_t offset = player_id * 7;                 // Player 0 has offset zero; player 1 has offset 7.
      for (size_t i = 0; i < 7; i++) {
        input_map[(int)(i+offset)] = (double) boardA[i];
        input_map[(int)(i+7-offset)] = (double) boardB[i];
      }

      return input_map;
    }

    // Returns bool indicating whether player can go again
    bool DoMove(move_t cell) {
      emp_assert(cell < 6);                // You cannot choose a cell out of bounds.

      side_t & cur_board   = GetCurSide();
      side_t & other_board = GetOtherSide();

      emp_assert(cur_board[cell] != 0);        // You cannot choose an empty cell.

      size_t stone_count = cur_board[cell];
      size_t cur_cell = cell;

      cur_board[cell] = 0;

      while (stone_count > 0) {
        cur_cell = (cur_cell+1) % 13;  // 6 pits on either side + 1 allowed home.
        if (cur_cell < 7) cur_board[cur_cell]++;
        else other_board[cur_cell-7]++;
        stone_count--;
      }

      // Go again if you ended in your store
      if (cur_cell == 6) {
        TestOver();
        return true;
      }

      // If you didn't end in your home, see if you captured!
      // You must be on your side of the board...
      //  and the only cell in the last pit is the one you just put there.
      if (cur_cell < 6 && cur_board[cur_cell] == 1) {
        size_t clear_pos = 5 - cur_cell;
        cur_board[6] += other_board[clear_pos];
        other_board[clear_pos] = 0;
      }

      is_A_turn = (size_t) !is_A_turn;
      TestOver();
      return false;
    }

    // Setup a DoMove from either player's viewpoint.
    bool DoMove([[maybe_unused]] size_t player, move_t cell) {
      emp_assert(player != is_A_turn);  // Verify that we agree on player who goes next!
      return DoMove(cell);
    }

    void SetBoard(side_t a, side_t b) {
        boardA = a;
        boardB = b;
    }

    bool IsDone() const { return over; }

    bool IsMoveValid(size_t move) const {
      // Exclude never-valid moves or empty pits
      if (move >= 6 || GetCurSide()[move] == 0) { return false; }
      return true;
    }

    // Provide all of the legal moves.
    emp::vector<move_t> GetMoveOptions() {
      emp::vector<move_t> out_v;
      for (size_t i = 0; i < 6; i++) {
        if (GetCurSide()[i]) out_v.push_back(i);
      }
      return out_v;
    }

    void PrintSmall(const std::string & title="", std::ostream & os=std::cout) {
      if (title.size()) os << title << ":\n";
      os << "  ";
      for (size_t i = 5; i < 6; i--) {
        os << boardB[i] << " ";
      }
      os << std::endl;
      os << boardB[6] << "              " << boardA[6] << std::endl;
      os << "  ";
      for (size_t i = 0; i < 6; i++) {
        os << boardA[i] << " ";
      }
      os << std::endl;
    }

    void Print(std::ostream & os=std::cout) {
      os << "+---<<<---F-----E-----D-----C-----B-----A---<<<---+ Player B";
      if (is_A_turn == false) os << " ***";
      os << "\n"
         << "|                                                 | \n"
         << "|       (" << std::setw(2) << boardB[5]
         << " ) (" << std::setw(2) << boardB[4]
         << " ) (" << std::setw(2) << boardB[3]
         << " ) (" << std::setw(2) << boardB[2]
         << " ) (" << std::setw(2) << boardB[1]
         << " ) (" << std::setw(2) << boardB[0]
         << " )       |\n";
      os << "v [" << std::setw(2) << boardB[6]
         << " ]                                     ["
         << std::setw(2) << boardA[6] << " ] ^\n";
      os << "|       (" << std::setw(2) << boardA[0]
         << " ) (" << std::setw(2) << boardA[1]
         << " ) (" << std::setw(2) << boardA[2]
         << " ) (" << std::setw(2) << boardA[3]
         << " ) (" << std::setw(2) << boardA[4]
         << " ) (" << std::setw(2) << boardA[5]
         << " )       |\n";
      os << "|                                                 |\n"
         << "+--->>>---A-----B-----C-----D-----E-----F--->>>---+ Player A";
      if (is_A_turn == true) os << " ***";
      os << std::endl << std::endl;
    }

    size_t GetCurPlayer() const { return !is_A_turn; }
    bool IsTurnA() const { return is_A_turn; }
    bool IsTurnB() const { return !is_A_turn; }

    size_t ScoreA() const {
      size_t score = 0;
      for (size_t i = 0; i < 7; i++) {
        score += boardA[i];
      }
      return score;
    }

    size_t ScoreB() const {
      size_t score = 0;
      for (size_t i = 0; i < 7; i++) {
        score += boardB[i];
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

#endif // #ifndef EMP_GAMES_MANCALA_HPP_INCLUDE
