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

    vector<size_t> board;    // Current board state
    bool over = false;    // Has the game ended?
    size_t is_B_turn;     // Which player goes next?
    int boardSize = 8;     // How big is the board? (N x N)
    vector<std::pair<int, int>> flip_list;
    vector<std::pair<int, int>> valid_moves_B;
    vector<std::pair<int, int>> valid_moves_W;

  public:
    using move_t = std::pair<int, int>;

    Othello(bool B_first=true) { Reset(B_first); boardSize = 8; }
    ~Othello() { ; }

    void Reset(bool B_first=true) {
      for (int i = 0; i < boardSize * boardSize; i++){ board.push_back(0);}

      AddDisc(boardSize / 2, boardSize / 2, false);
      AddDisc(boardSize / 2 + 1, boardSize / 2 + 1, false);
      AddDisc(boardSize / 2, boardSize / 2 + 1, true);
      AddDisc(boardSize / 2 + 1, boardSize / 2, true);

      over = false;
      is_B_turn = B_first;
    }

    void ClearFlips() { flip_list.clear(); }

    void ClearValidMoves() { valid_moves_B.clear(); valid_moves_W.clear(); }

    vector<move_t> GetMoveOptions() {
        if (is_B_turn) { return valid_moves_B; }
        else { return valid_moves_W; }
    }

    int GetIndex(int x, int y) { return ((y - 1) * boardSize) + (x - 1); }

    move_t GetCoord(int idx) { return std::make_pair((idx % boardSize) + 1, (idx / boardSize) + 1); }

    size_t GetCurrPlayer() { if (is_B_turn) { return 1; } else { return 2; } }

    size_t GetSquare(int x, int y) { 
        int idx = GetIndex(x, y);
        return board[idx]; 
    }

    size_t GetSquare(int idx) { return board[idx]; }

    int GetBoardSize() { return boardSize; }

    void AddDisc(int x, int y, bool is_B){
        int idx = GetIndex(x, y);

        if (is_B) { board[idx] = 1; }
        else { board[idx] = 2; }
    }

    void Flip(vector<std::pair<int, int>> flip_list) {
        for (auto disc : flip_list) {
            int idx = GetIndex(disc.first, disc.second);
            board[idx] = GetCurrPlayer();
        }
    }

    void Print(std::ostream & os=std::cout) {
        os<<std::endl<<"  ";
        for (int i = 1; i <= boardSize; i++){ std::cout<<i<<" "; }
        os<<std::endl;

        for (int y = 1; y <= boardSize; y++) {
            os<< y << " ";
            for (int x = 1; x <= boardSize; x++) {
                char piece;
                int square = GetSquare(x,y);
                
                if (square == 0) { piece = '-'; }
                else if (square == 1) { piece = 'B'; }
                else { piece = 'W'; }

                os<<piece<<" ";
            }
            os<<std::endl;
        }

        os<<std::endl;
    }

    bool IsDone() const { return over; }

    bool IsMoveValid(size_t player, move_t move) {
        int x = move.first;
        int y = move.second;
        bool is_B = 0;
        if (player == 1) { is_B = 1; }

        //if (GetSquare(x, y) != 0) { return 0; }

        for (int j = y - 1; j <= y + 1; j++) {
            if ( j < 1 || j > boardSize) { continue; }

            for (int i = x - 1; i <= x + 1; i++) {
                if (i < 1 || i > boardSize) { continue; }           

                size_t turn = 0;

                if (is_B) {
                    if (GetSquare(i, j) == 2) { turn = 1; }
                }
                else{
                    if (GetSquare(i, j) == 1) { turn = 2; }
                }

                if (turn != 0) {
                    vector<std::pair<int, int>> potential_flips; 

                    if (x == i && y > j) {
                        for (int k = j; k >= 1; k--) { 
                            if (GetSquare(x, k) == 0) { break; }
                            if (GetSquare(x, k) == turn) {
                                for (auto disc : potential_flips) { flip_list.push_back(disc); } 
                                break;
                            }
                            potential_flips.push_back(std::make_pair(x,k));
                        } 
                    }
                    else if (x == i && y < j) {
                        for (int k = j; k <= boardSize; k++) {
                            if (GetSquare(x, k) == 0) { break; }

                            if (GetSquare(x, k) == turn) { 
                                for (auto disc : potential_flips) { flip_list.push_back(disc); }
                                break;
                            }
                            potential_flips.push_back(std::make_pair(x,k));
                        }
                    
                    }
                    else if (y == j && x > i) {
                        for (int k = i; k >= 1; k--) {
                            if (GetSquare(k, y) == 0) { break; }

                            if (GetSquare(k, y) == turn) { 
                                for (auto disc : potential_flips) { flip_list.push_back(disc); }
                                break;
                            }
                            potential_flips.push_back(std::make_pair(k, y));
                        }
                    }
                    else if (y == j && x < i) { 
                        for (int k = i; k <= boardSize; k++) {
                            if (GetSquare(k, y) == 0) { break; }

                            if (GetSquare(k, y) == turn) { 
                                for (auto disc : potential_flips) { flip_list.push_back(disc); }
                                break;
                            }
                            potential_flips.push_back(std::make_pair(k, y));
                        }
                    }
                    else if (x < i && y < j) {
                        int l = j;
                        for (int k = i; k <= boardSize; k++) {
                            if (GetSquare(k, l) == 0) { break; }

                            if (GetSquare(k, l) == turn) {
                                for (auto disc : potential_flips) { flip_list.push_back(disc); }
                                break;
                            }
                            potential_flips.push_back(std::make_pair(k, l));
                            l++;
                            if (l > boardSize) { break; }
                        }
                    }
                    else if (x > i && y > j) {
                        int l = j;
                        for (int k = i; k >= 1; k--) {
                            if (GetSquare(k, l) == 0) { break; }

                            if (GetSquare(k, l) == turn) {
                                for (auto disc : potential_flips) { flip_list.push_back(disc); }
                                break;
                            }
                            potential_flips.push_back(std::make_pair(k, l));
                            l--;
                            if (l < 1) { break; }
                        }
                    }
                    else if (x > i && y < j) { // Top Right Diagonal
                        int l = j;
                        for (int k = i; k >= 1; k--) {
                            if (GetSquare(k, l) == 0) { break; }

                            if (GetSquare(k, l) == turn) {
                                for (auto disc : potential_flips) { flip_list.push_back(disc); }
                                break;
                            }
                            potential_flips.push_back(std::make_pair(k, l));
                            l++;
                            if (l > boardSize) { break; }
                        }
                    }
                    else if (x < i && y > j) {
                        
                        int l = j;
                        for (int k = i; k <= boardSize; k++) {
                            if (GetSquare(k, l) == 0) { break; }

                            if (GetSquare(k, l) == turn) {
                                for (auto disc : potential_flips) { flip_list.push_back(disc); }
                                break;
                            }
                            potential_flips.push_back(std::make_pair(k, l));
                            l--;
                            if (l < 1) { break; }
                        }
                    }

                    potential_flips.clear();
                }
            }
        }
       // if (flip_list.size() > 0) {std::cout<<x<<" "<<y<<std::endl;}
        return flip_list.size();
    }

    size_t TestOver() {
        bool moveW = false;
        bool moveB = false;

        if (GetScore(1) == 0) { over = true; return 0; }
        if (GetScore(2) == 0) { over = true; return 0; }
        ClearValidMoves();

        for (int x = 1; x <= boardSize; x++) {
            for (int y = 1; y <= boardSize; y++) {
                move_t move = std::make_pair(x, y);
                size_t square = GetSquare(move.first, move.second);
                if (square == 0) {
                    ClearFlips();
                    bool validB = IsMoveValid(1, move);
                    ClearFlips();
                    bool validW = IsMoveValid(2, move);
                    ClearFlips();

                    if (validB && validW) {
                        moveW = moveB = true;
                        valid_moves_B.push_back(move);
                        valid_moves_W.push_back(move);
                    }
                    else if (validB && !validW) { 
                        moveB = true;
                        valid_moves_B.push_back(move);
                    
                    }
                    else if (!validB && validW) { 
                        moveW = true;
                        valid_moves_W.push_back(move);
                    }
                }
            }
        }

        if (!moveB && !moveW) { over = true; }
        else if (moveB && !moveW) { return 1; }
        else if (!moveB && moveW) { return 2; }
        return 0;

    }

    bool DoMove(size_t player, move_t move, bool verbose=0) {

        int x = move.first;
        int y = move.second;

        if (player == 1) { is_B_turn = 1; }
        else { is_B_turn = 0; }
        
        AddDisc(x, y, is_B_turn);
        if (verbose) {
            for (auto el : flip_list) { std::cout<<el.first<<" "<<el.second<<std::endl; }
        }
        Flip(flip_list);
        //is_B_turn = !(is_B_turn);
        size_t test = TestOver();

        if (player == test) { return true; }
        else { return false; }

    }

    double GetScore(size_t player) {
        double bScore = 0;
        double wScore = 0;

        for (int x = 1; x <= boardSize; x++) {
            for (int y = 1; y <= boardSize; y++) {
                size_t disc = GetSquare(x, y);
                if (disc == 1) { bScore++; }
                else if (disc == 2) { wScore++; }
            }
        }

        if (player == 1) { return bScore; }
        else if (player == 2) { return wScore; }
    }
    
  };

}

#endif
