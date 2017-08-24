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
#include <unordered_map>

#include "../base/array.h"
#include "../base/assert.h"
#include "../base/vector.h"
#include "../tools/math.h"

namespace emp {

  class Othello {
  private:
    vector<size_t> game_board;  /// Current board state
    bool game_over = false;     /// Has the game ended?
    size_t cur_player = 1;      /// ID of player using board
    size_t board_size = 8;      /// Length of one side of N x N board

  public:
    using move_t = size_t;

    Othello(size_t side_len, size_t first_player=1) {
        Reset(first_player);
        board_size = side_len;
    }

    ~Othello() { ; }

    move_t GetIndex(int x, int y) { return ((y - 1) * board_size) + (x - 1); }

    size_t GetOpponent(size_t player) {
        if (player == 1) { return 2; }
        else if (player == 2) { return 1; }
        else { return -1; }
    }

    size_t GetCurrPlayer() { return cur_player; }

    size_t GetTile(move_t tile) { return game_board[tile]; }

    void SetTurn(size_t player) { cur_player = player; }

    bool IsOutOfBounds(int move) {
        return (move < 0 || move >= board_size * board_size);
    }

    bool IsDone() { return game_over; }

    int GetNeighbor(std::string location, move_t move) {
        int neighbor = -1;

        if (location == "above") {
            neighbor = int(move) - board_size;
        }
        else if (location == "below") {
            neighbor = int(move) + board_size;
        }
        else if (location == "left") {
            neighbor = int(move) - 1;
            if (move % board_size == 0) { neighbor = -1; }
        }
        else if (location == "right") {
            neighbor = int(move) + 1;
            if (move % board_size == (board_size - 1)) { neighbor = -1; }
        }
        else if (location == "AboveLeft") {
            neighbor = int(move) - board_size - 1;
            if (move % board_size == 0) { neighbor = -1; }
        }
        else if (location == "BelowLeft") {
            neighbor = int(move) + board_size - 1;
            if (move % board_size == 0) { neighbor = -1; }
        }
        else if (location == "AboveRight") {
            neighbor = int(move) - board_size + 1;
            if (move % board_size == (board_size - 1)) { neighbor = -1; }
        }
        else if (location == "BelowRight") {
            neighbor = int(move) + board_size + 1;
            if (move % board_size == (board_size - 1)) { neighbor = -1; }
        }

        if (IsOutOfBounds(neighbor)) { neighbor = -1; }

        return neighbor;
    }

    void Reset(size_t first_player) {
        for (int i = 0; i < board_size * board_size; i++){
            game_board.push_back(0);
        }

        // Setup initial board for player 1
        vector<move_t> move_list;
        move_list.push_back( GetIndex(board_size / 2, board_size / 2 + 1) );
        move_list.push_back( GetIndex(board_size / 2 + 1, board_size / 2) );
        SetTiles(move_list, 1);

        // Setup initial board for player 2
        move_list.clear();
        move_list.push_back( GetIndex(board_size / 2, board_size / 2) );
        move_list.push_back( GetIndex(board_size / 2 + 1, board_size / 2 + 1) );
        SetTiles(move_list, 2);

        game_over = false;
        cur_player = first_player;
    }

    std::unordered_map<int, double> AsInput(size_t player_id) const {;} // TODO

    bool IsMoveValid(size_t player, move_t move) {
        vector<move_t> flip_list = GetFlipList(player, move, true);

        if (GetTile(move) != 0) { return false; } // Tile must be empty

        return flip_list.size() != 0; // If no tiles flip, not a valid move
    }

    std::unordered_map<size_t, vector<size_t> > GetMoveOptions() {
        std::unordered_map<size_t, vector<size_t> > valid_moves;
        valid_moves[1] = {};
        valid_moves[2] = {};

        for (move_t i = 0; i < board_size * board_size; i++) {
            if (IsMoveValid(1, i)) { valid_moves[1].push_back(i); }
            if (IsMoveValid(2, i)) { valid_moves[2].push_back(i); }
        }

        return valid_moves;
    }

    bool DoMove(size_t player, move_t move) {
        SetTile(move, player);
        FlipTiles(player, move);
        return CheckGoAgain(player);
    }

    bool CheckGoAgain(size_t player){
        std::unordered_map<size_t, vector<size_t> > moves = GetMoveOptions();
        bool again = false;

        if (moves[1].size() <= 0 && moves[2].size() <= 0) { game_over = true; }

        // If opponent has no moves, play again
        else if (moves[GetOpponent(player)].size() <= 0) { again = true; }

        return again;
    }

    void SetTiles(vector<move_t> move_list, size_t player){
        for (move_t move : move_list){
            SetTile(move, player);
        }
    }

    void SetTile(size_t move, size_t player){
        game_board[move] = player;
    }

    void FlipTiles(size_t player, move_t move) {
        vector<move_t> flip_list = GetFlipList(player, move, false);
        for (move_t tile : flip_list) { SetTile(tile, player); }
    }

    double GetScore(size_t player) {
        double score = 0;

        for (size_t i = 0; i < board_size * board_size; i++){
            if (GetTile(i) == player) { score++; }
        }

        return score;
    }

    void Print(std::ostream & os=std::cout) {
        os<<std::endl<<"  ";
        unsigned char letter = 'A';
        for (int i = 0; i < board_size; i++){ std::cout<<char(letter + i) <<" "; }
        os<<std::endl;

        for (int y = 1; y <= board_size; y++) {
            os<< y << " ";
            for (int x = 1; x <= board_size; x++) {
                char piece;
                int square = GetTile(GetIndex(x,y));

                if (square == 0) { piece = '-'; }
                else if (square == 1) { piece = 'X'; }
                else { piece = 'O'; }

                os<<piece<<" ";
            }
            os<<std::endl;
        }

        os<<std::endl;
    }

    vector<move_t> GetFlipList(size_t player, move_t move, bool check_valid=false){
        vector<std::string> directions = {"above", "below", "left", "right",
                                          "AboveLeft", "AboveRight",
                                          "BelowLeft", "BelowRight"};
        vector<move_t> flip_list;

        for (std::string direction : directions) {
            int pos = GetNeighbor(direction, move);
            vector<move_t> temp_list;

            while (pos != -1){
                if (GetTile(pos) == 0) { break; }

                else if (GetTile(pos) != player) { temp_list.push_back(pos); }

                else {
                    for (move_t move : temp_list) { flip_list.push_back(move); }
                    break;
                }

                pos = GetNeighbor(direction, pos);
            }

            temp_list.clear();

            // If just checking that move is valid, we don't need full flip list
            if (check_valid && flip_list.size() > 0) { break; }
        }
        return flip_list;
    }

  };

}

#endif
