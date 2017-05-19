//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  A simple Othello game state handler.

#ifndef EMP_GAME_OTHELLO_H
#define EMP_GAME_OTHELLO_H

#include <fstream>
#include <iostream>
#include <iomanip>
#include <utility>

#include "../base/array.h"
#include "../base/assert.h"
#include "../base/vector.h"
#include "../tools/math.h"

namespace emp {

  class Othello {
  private:
    using side_t = emp::array<size_t, 7>;

    enum squareState { empty, Black, White };
    vector<squareState> board;    // Current board state
    bool over = false;    // Has the game ended?
    size_t is_B_turn;     // Which player goes next?
    int boardSize = 8;     // How big is the board? (N x N)

  public:
    using move_t = size_t;

    Othello(bool B_first=true) { Reset(B_first); boardSize = 8; }
    ~Othello() { ; }

    void Reset(bool B_first=true) {
      for (int i = 0; i < boardSize * boardSize; i++){ board.push_back(empty);}

      AddDisc(boardSize / 2, boardSize / 2, false);
      AddDisc(boardSize / 2 + 1, boardSize / 2 + 1, false);
      AddDisc(boardSize / 2, boardSize / 2 + 1, true);
      AddDisc(boardSize / 2 + 1, boardSize / 2, true);

      over = false;
      is_B_turn = B_first;
    }

    int GetIndex(int x, int y) { return ((y - 1) * boardSize) + (x - 1); }

    squareState GetSquare(int x, int y) { 
        int idx = GetIndex(x, y);
        return board[idx]; 
    }

    void AddDisc(int x, int y, bool is_B){
        int idx = GetIndex(x, y);

        if (is_B) { board[idx] = Black; }
        else { board[idx] = White; }
    }

    void PrintBoard() {
        std::cout<<std::endl<<"  ";
        for (int i = 1; i <= boardSize; i++){ std::cout<<i<<" "; }
        std::cout<<std::endl;

        for (int y = 1; y <= boardSize; y++) {
            std::cout<< y << " ";
            for (int x = 1; x <= boardSize; x++) {
                char piece;
                int square = GetSquare(x,y);
                
                if (square == 0) { piece = '-'; }
                else if (square == 1) { piece = 'B'; }
                else { piece = 'W'; }

                std::cout<<piece<<" ";
            }
            std::cout<<std::endl;
        }

        std::cout<<std::endl;
    }

    bool IsDone() const { return over; }

    bool TestOver() {

    }

    vector<std::pair<int, int>> ValidMove(int x, int y) {
        vector<std::pair<int,int>> flip_list;
        for (int j = y - 1; j <= y + 1; j++) {
            if ( j < 1 || j > boardSize) { continue; }

            for (int i = x - 1; i <= x + 1; i++) {
                if (i < 1 || i > boardSize) { continue; }

                squareState turn = empty;
                squareState opponent = empty;
                if (is_B_turn) {
                    if (GetSquare(i, j) == White) { turn = Black; opponent = White; }
                }
                else{
                    if (GetSquare(i, j) == Black) { turn = White; opponent = Black; }
                }

                if (turn != empty) {
                    std::cout<<"XY: " <<x<<" "<<y<<std::endl;
                    std::cout<<"IJ: "<<i<<" "<<j<<std::endl;
                    if (x == i && y > j) {
                        std::cout<<"XY: " <<x<<" "<<y<<std::endl;
                        std::cout<<"IJ: "<<i<<" "<<j<<std::endl;
                        for (int k = 1; k < j; k++) { 
                            if (GetSquare(x, k) == turn) { flip_list.push_back(std::make_pair(x,k)); } 
                        }
                    }
                    else if (x == i && y < j) {
                        for (int k = j; k <= boardSize; k++) {
                            if (GetSquare(x, k) == turn) { flip_list.push_back(std::make_pair(x,k)); }
                        }
                    
                    }
                    else if (y == j && x > i) {
                        for (int k = 1; k < i; k++) {
                            if (GetSquare(k, y) == turn) { flip_list.push_back(std::make_pair(k,y)); }
                        }
                    }
                    else if (y == j && x < i) { 
                        for (int k = i; k <= boardSize; k++) {
                            if (GetSquare(k, y) == turn) { flip_list.push_back(std::make_pair(k,y)); }
                        }
                    }
                    else if (x != i && y != j) {
                        std::cout<<"TODO"<<std::endl; //TODO Add checking for diagonals
                    }
                }
            }
        }
        return flip_list;
    }

    bool DoMove(int x, int y) {
        // TODO Check to see if game is over
        
        vector<std::pair<int, int>> flip_list = ValidMove(x, y);

        //TODO flip tiles
        
        if (flip_list.size() != 0) {
            AddDisc(x, y, is_B_turn);
            is_B_turn = !(is_B_turn);
            return true;
        }
        else {
            return false;
        }

    }

    size_t ScoreB() {
        size_t bScore = 0;

        for (int x = 1; x <= boardSize; x++) {
            for (int y = 1; y <= boardSize; y++) {
                squareState disc = GetSquare(x, y);
                if (disc == Black) { bScore++; }
            }
        }

        return bScore;  
    }

    size_t ScoreW() {
        size_t wScore = 0;

        for (int x = 1; x <= boardSize; x++) {
            for (int y = 1; y <= boardSize; y++) {
                squareState disc = GetSquare(x, y);
                if (disc == White) { wScore++; }
            }
        }
        return wScore;
    }
    

    /*
    const vector<int> & GetBoard() const { return board; }
    vector<int> & GetBoard() { return board; }
    //side_t & GetCurSide() { return is_A_turn ? boardA : boardB; }
    //side_t & GetOtherSide() { return is_A_turn ? boardB : boardA; }


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
    bool DoMove(size_t player, move_t cell) {
      emp_assert(player != is_A_turn);  // Verify that we agree on player who goes next!
      return DoMove(cell);
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

    void PrintSmall(std::ostream & os=std::cout) {
      os << "  ";
      for (size_t i = 5; i < 5; i--) {
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
      std::cout << "+---<<<---F-----E-----D-----C-----B-----A---<<<---+ Player B";
      if (is_A_turn == false) std::cout << " ***";
      std::cout << "\n"
                << "|                                                 | \n"
                << "|       (" << std::setw(2) << boardB[5]
                << " ) (" << std::setw(2) << boardB[4]
                << " ) (" << std::setw(2) << boardB[3]
                << " ) (" << std::setw(2) << boardB[2]
                << " ) (" << std::setw(2) << boardB[1]
                << " ) (" << std::setw(2) << boardB[0]
                << " )       |\n";
      std::cout << "v [" << std::setw(2) << boardB[6]
                << " ]                                     ["
                << std::setw(2) << boardA[6] << " ] ^\n";
      std::cout << "|       (" << std::setw(2) << boardA[0]
                << " ) (" << std::setw(2) << boardA[1]
                << " ) (" << std::setw(2) << boardA[2]
                << " ) (" << std::setw(2) << boardA[3]
                << " ) (" << std::setw(2) << boardA[4]
                << " ) (" << std::setw(2) << boardA[5]
                << " )       |\n";
      std::cout << "|                                                 |\n"
                << "+--->>>---A-----B-----C-----D-----E-----F--->>>---+ Player A";
      if (is_A_turn == true) std::cout << " ***";
      std::cout << std::endl << std::endl;
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
*/
  };

}

#endif
