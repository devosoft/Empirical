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
    //Should end cells be their own members? For now lets say no.
    emp::array<int, 14> board;
    bool over = false;
    bool record = false;
    std::ofstream record_file;
    int record_player = 0;

  public:

    int curr_player = 0;

    void SetRecord(bool r, int player = 0, std::string filename = "mancala_record.csv") {
      record = r;
      record_player = player;
      if (record) {
        record_file.open(filename);
        if (!record_file.good()) {
          std::cout << "Invalid output file. Exiting." << std::endl;
          exit(0);  // @CAO: We should run this through a proper error-processing system...
        }
        record_file << "0,1,2,3,4,5,6,7,8,9,10,11,12,13,move" << std::endl;
      }
    }

    Mancala() {
      Reset();
    }

    ~Mancala() {
      record_file.close();
    }

    void Reset() {
      for (int i = 0; i < (int)board.size(); i++) {
          board[i] = 4;
      }
      board[0] = 0;
      board[7] = 0;
      over = false;
      curr_player = 0;
    }

    int operator[](int i) const {
      return board[i];
    }

    emp::array<int, 14>& GetBoard() {
      return board;
    }

    emp::array<int, 14> GetFlippedBoard() {
      emp::array<int, 14> flipped;
      flipped[7] = board[0];
      flipped[8] = board[1];
      flipped[9] = board[2];
      flipped[10] = board[3];
      flipped[11] = board[4];
      flipped[12] = board[5];
      flipped[13] = board[6];

      flipped[0] = board[7];
      flipped[1] = board[8];
      flipped[2] = board[9];
      flipped[3] = board[10];
      flipped[4] = board[11];
      flipped[5] = board[12];
      flipped[6] = board[13];
      return flipped;
    }

    int GetOpposite(int choice) {
      int opposite = abs(7 - emp::Mod(choice, 7));
      if (choice < 7) {
        opposite += 7;
      }
      return opposite;
    }

    int GetCounterpart(int choice) {
      if (choice < 7) {
        choice += 7;
      } else {
        choice -= 7;
      }
      return choice;
    }

    void UpdateIsOver() {
      bool side_1_empty = true;
      bool side_2_empty = true;

      for (int i = 1; i < 7; i++) {
        if (board[i] > 0) {
          side_1_empty = false;
        }
      }

      for (int i = 8; i < 14; i++) {
        if (board[i] > 0) {
          side_2_empty = false;
        }
      }

      over = ( (!curr_player && side_1_empty) || (curr_player && side_2_empty));
    }

    // Returns bool indicating whether player can go again
    bool ChooseCell(int cell) {
      emp_assert(cell != 0 && cell != 7); //You can't choose the end cells
      emp_assert((cell < 7 && !curr_player) || (cell > 7 && curr_player));
      int count = board[cell];
      int curr_cell = cell;

      if (record && curr_player == record_player) {
        for (int i = 0; i < 14; i++) {
          record_file << board[i] << ",";
        }
        record_file << cell << std::endl;
      }

      board[cell] = 0;

      while (count > 0) {
        curr_cell++;
        if (curr_cell > 13) {
          curr_cell = 0;
        }
        if ((curr_cell == 0 && cell < 7) || (curr_cell == 7 && cell > 7)) {
          curr_cell++;
        }

        board[curr_cell]++;
        count--;
      }

      // Go again if you ended in your store
      if (curr_cell == 0 || curr_cell == 7) {
        UpdateIsOver();
        return true;
      }

      // Capturing
      if (board[curr_cell] == 1 &&
          ((curr_cell < 7 && cell < 7) || (curr_cell > 7 && cell > 7))) {
        int opposite = abs(7 - emp::Mod(curr_cell, 7));
        if (curr_cell < 7) {
          opposite += 7;
          board[7] += board[opposite];
          board[opposite] = 0;
        } else {
          board[0] += board[opposite];
          board[opposite] = 0;
        }

      }

      curr_player = (int)!curr_player;
      UpdateIsOver();
      return false;
    }

    bool IsMoveValid(int move) const {

      if (move > 13 || move < 0 || !board[move]) {
        return false;
      }
      if (!curr_player && move < 7 && move > 0) {
        return true;
      }
      if (curr_player && move > 7) {
        return true;
      }
      return false;
    }

    bool IsOver() const {
      return over;
    }


    void PrintBoard() {
      std::cout << "  ";
      for (int i = 6; i > 0; i--) {
        std::cout << board[i] << " ";
      }
      std::cout << std::endl;
      std::cout << board[7] << "              " << board[0] << std::endl;
      std::cout << "  ";
      for (int i = 8; i < 14; i++) {
        std::cout << board[i] << " ";
      }
      std::cout << std::endl;
    }

    int GetWinner() {
      int player1 = board[0];
      int player0 = board[7];

      for (int i = 1; i < 7; i++) {
        player0 += board[i];
      }

      for (int i = 8; i < 14; i++) {
        player1 += board[i];
      }

      return (int) (player1 > player0);
    }

    int GetCurrPlayer() const {
      return curr_player;
    }

    bool IsTurnA() const {
      return curr_player == 0;
    }

    bool IsTurnB() const {
      return curr_player == 1;
    }


    int ScoreDiff(int player) const {
      int player1 = board[0];
      int player0 = board[7];

      for (int i = 1; i < 7; i++) {
        player0 += board[i];
      }

      for (int i = 8; i < 14; i++) {
        player1 += board[i];
      }

      if (player) {
        return player1 - player0;
      } else {
        return player0 - player1;
      }
    }

    void Forfeit() {
      if (curr_player) {
        for (int i = 0; i < 14; i++) {
          if (i != 0 && i != 7) {
            board[7] += board[i];
            board[i] = 0;
          }
        }
      } else {
        for (int i = 0; i < 14; i++) {
          if (i != 0 && i != 7) {
            board[0] += board[i];
            board[i] = 0;
          }
        }
      }

      over = true;
    }

  };

  bool IsMoveValid(int move, emp::array<int, 14> board, int curr_player) {

    if (move > 13 || move < 0 || !board[move]) {
      return false;
    }
    if (!curr_player && move < 7 && move > 0) {
      return true;
    }
    if (curr_player && move > 7) {
      return true;
    }
    return false;
  }

}

#endif
