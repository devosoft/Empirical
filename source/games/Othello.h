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
  public:
    enum BoardSpace { DARK, LIGHT, OPEN };

    using board_t = emp::vector<BoardSpace>;


  protected:

    emp::vector<size_t> ALL_DIRECTIONS;

    static constexpr size_t PLAYER_ID__DARK = 0;
    static constexpr size_t PLAYER_ID__LIGHT = 1;
    static constexpr size_t DIRECTION_ID__N = 0;
    static constexpr size_t DIRECTION_ID__S = 1;
    static constexpr size_t DIRECTION_ID__E = 2;
    static constexpr size_t DIRECTION_ID__W = 3;
    static constexpr size_t DIRECTION_ID__NE = 4;
    static constexpr size_t DIRECTION_ID__NW = 5;
    static constexpr size_t DIRECTION_ID__SE = 6;
    static constexpr size_t DIRECTION_ID__SW = 7;

    board_t game_board;  ///< Game board

    bool over = false;    ///< Is the game over?
    size_t cur_player;    ///< Is it DARK player's turn?

    size_t board_size;    ///< Game board is board_size X board_size

  public:

    Othello(size_t side_len)
      : board_size(side_len), game_board(board_size)
    {
      emp_assert(board_size > 0);
      ALL_DIRECTIONS = {N(), S(), E(), W(), NE(), NW(), SE(), SW()};
      Reset();
    }

    ~Othello() { ; }

    static inline constexpr size_t DarkPlayerID() { return PLAYER_ID__DARK; }
    static inline constexpr size_t LightPlayerID() { return PLAYER_ID__LIGHT; }
    static inline constexpr BoardSpace DarkDisk() { return BoardSpace::DARK; }
    static inline constexpr BoardSpace LightDisk() { return BoardSpace::LIGHT; }
    static inline constexpr BoardSpace OpenSpace() { return BoardSpace::OPEN; }
    static inline constexpr size_t N() { return DIRECTION_ID__N; }
    static inline constexpr size_t S() { return DIRECTION_ID__S; }
    static inline constexpr size_t E() { return DIRECTION_ID__E; }
    static inline constexpr size_t W() { return DIRECTION_ID__W; }
    static inline constexpr size_t NE() { return DIRECTION_ID__NE; }
    static inline constexpr size_t NW() { return DIRECTION_ID__NW; }
    static inline constexpr size_t SE() { return DIRECTION_ID__SE; }
    static inline constexpr size_t SW() { return DIRECTION_ID__SW; }


    void Reset() {
      // Reset the board.
      for (size_t i = 0; i < game_board.size(); ++i) game_board[i] = OpenSpace();

      // Setup Initial board
      //  ........
      //  ...LD...
      //  ...DL...
      //  ........
      SetPos(board_size/2, (board_size/2)+1, DarkDisk());
      SetPos(board_size/2, board_size/2, LightDisk());
      SetPos((board_size/2)+1, board_size/2, DarkDisk());
      SetPos((board_size/2)+1, (board_size/2)+1, LightDisk());

      over = false;
      cur_player = DarkPlayerID();
    }

    ///
    size_t GetBoardWidth() const { return board_size; }
    size_t GetBoardHeight() const { return board_size; }
    size_t GetBoardSize() const { return game_board.size(); }

    size_t GetCurPlayer() const { return cur_player; }

    size_t OpponentID(size_t playerID) const {
      emp_assert(playerID == DarkPlayerID() || playerID == LightPlayerID());
      if (playerID == DarkPlayerID()) return LightPlayerID();
      return DarkPlayerID();
    }

    /// Get x location in board grid given loc ID.
    size_t GetPosX(size_t id) const { return id % board_size; }

    /// Get Y location in board grid given loc ID.
    size_t GetPosY(size_t id) const { return id / board_size; }

    /// Get board ID of  given an x, y position.
    size_t GetPosID(size_t x, size_t y) const { return (y * board_size) + x; }

    bool IsValidPos(size_t x, size_t y) const {
      return x < board_size && y < board_size;
    }

    bool IsValidPos(size_t id) const { return id < game_board.size(); }

    /// Get location adjacent to ID in direction dir.
    int GetNeighbor(size_t id, size_t dir) const {
      emp_assert(dir >= 0 && dir <= 7 && id < game_board.size());
      size_t x = GetPosX(id);
      size_t y = GetPosY(id);
      return GetNeighbor(x, y, dir);
    }

    int GetNeighbor(size_t x, size_t y, size_t dir) const {
      emp_assert(dir >= 0 && dir <= 7);
      int facing_x = 0, facing_y = 0;
      switch(dir) {
        case N():  facing_x = x;   facing_y = y+1; break;
        case E():  facing_x = x-1; facing_y = y;   break;
        case W():  facing_x = x+1; facing_y = y;   break;
        case S():  facing_x = x;   facing_y = y-1; break;
        case NE(): facing_x = x-1; facing_y = y+1; break;
        case NW(): facing_x = x+1; facing_y = y+1; break;
        case SE(): facing_x = x-1; facing_y = y-1; break;
        case SW(): facing_x = x+1; facing_y = y-1; break;
        default:
          std::cout << "Bad direction!" << std::endl;
          break;
      }
      if (!IsValidPos(facing_x, facing_y)) return -1;
      return GetPosID(facing_x, facing_y);
    }

    /// Given a player ID (0 or 1 are valid), what Disk type does that player use?
    BoardSpace GetDiskType(size_t player_id) {
      emp_assert(player_id == 0 || player_id == 1);
      if (player_id == DarkPlayerID()) return DarkDisk();
      else return LightDisk();
    }

    /// Given a disk type (light or dark), get the player id that uses that disk type.
    size_t GetPlayerID(BoardSpace disk_type) {
      emp_assert(disk_type == DarkDisk() || disk_type == LightDisk());
      if (disk_type == DarkDisk()) return DarkPlayerID();
      else return LightPlayerID();
    }

    /// Get the value (light, dark or open) at an x,y location on the board.
    BoardSpace GetPosValue(size_t x, size_t y) const {
      return GetPosValue(GetPosID(x, y));
    }

    /// Get the value (light, dark, or open) at a position on the board.
    BoardSpace GetPosValue(size_t id) const {
      emp_assert(id < game_board.size());
      return game_board[id];
    }

    /// Get the owner (playerID) at a position on the board.
    /// Returns a -1 if no owner.
    int GetPosOwner(size_t id) {
      if (GetPosValue(id) == DarkDisk()) {
        return (int)DarkPlayerID();
      } else if (GetPosValue(id) == LightDisk()) {
        return (int)LightPlayerID();
      } else {
        return -1;
      }
    }

    /// Get the owner (playerID) at an x,y position on the board.
    /// Returns a -1 if no owner.
    int GetPosOwner(size_t x, size_t y) {
      return GetPosOwner(GetPosID(x, y));
    }

    // TODO: BoardAsInput ==> Should not be in Othello.h
    board_t & GetBoard() { return game_board; }

    bool IsMoveValid(size_t playerID, size_t move_x, size_t move_y) {
      return IsMoveValid(playerID, GetPosID(move_x, move_y));
    }

    /// Is given move valid?
    bool IsMoveValid(size_t playerID, size_t move_id) {
      emp_assert(playerID == DarkPlayerID() || playerID == LightPlayerID());
      // 1) The move position must be empty.
      if (GetPosValue(move_id) != OpenSpace()) return false;
      // 2) A non-zero number of tiles must flip.
      return (bool)GetFlipList(playerID, move_id, true).size();
    }

    emp::vector<size_t> GetFlipList(size_t playerID, size_t move_id, bool only_first_valid=false) {
      emp::vector<size_t> flip_list;
      size_t prev_len = 0;
      for (size_t dir : ALL_DIRECTIONS) {
        int neighborID = GetNeighbor(move_id, dir);
        while (neighborID != -1) {
          if (GetPosValue(neighborID) == OpenSpace()) {
            flip_list.resize(prev_len);
            break;
          } else if (GetPosOwner(neighborID) == OpponentID(playerID)) {
            flip_list.emplace_back(neighborID);
          } else {
            prev_len = flip_list.size();
            break;
          }
          neighborID = GetNeighbor((size_t)neighborID, dir);
        }
        if (only_first_valid && flip_list.size()) break;
      }
      return flip_list;
    }

    /// Set board position (ID) to given space value.
    void SetPos(size_t id, BoardSpace space) {
      emp_assert(id < game_board.size());
      game_board[id] = space;
    }

    /// Set board position (x,y) to given space value.
    void SetPos(size_t x, size_t y, BoardSpace space) {
      SetPos(GetPosID(x, y), space);
    }

    /// Set board position (ID) to disk type used by playerID.
    void SetPos(size_t id, size_t playerID) {
      emp_assert(playerID == LightPlayerID() || playerID == DarkPlayerID());
      SetPos(id, GetDiskType(playerID));
    }

    void SetPositions(emp::vector<size_t> ids, size_t playerID) {
      for (size_t i = 0; i < ids.size(); ++i) SetPos(ids[i], playerID);
    }

    void SetPositions(emp::vector<size_t> ids, BoardSpace space) {
      for (size_t i = 0; i < ids.size(); ++i) SetPos(ids[i], space);
    }

    /// Set board position (x,y) to disk type used by playerID.
    void SetPos(size_t x, size_t y, size_t playerID) {
      emp_assert(playerID == LightPlayerID() || playerID == DarkPlayerID());
      SetPos(GetPosID(x, y), GetDiskType(playerID));
    }

  };

}

#endif
