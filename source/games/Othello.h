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
  /// NOTE: This game could be made more black-box.
  ///   - Hide almost everything. Only give users access to game-advancing functions (don't allow
  ///     willy-nilly board manipulation, etc). This would let us make lots of assumptions about
  ///     calculating flip lists, scores, etc, which would speed up asking for flip lists, etc. multiple
  ///     times during a turn.
  class Othello {
  public:
    enum BoardSpace { DARK, LIGHT, OPEN };        ///< All possible states of a board space.
    using board_t = emp::vector<BoardSpace>;
    // Player IDs.
    static constexpr size_t PLAYER_ID__DARK = 0;  ///< Darkplayer ID
    static constexpr size_t PLAYER_ID__LIGHT = 1; ///< Lightplayer ID
    // Direction IDs.
    static constexpr size_t DIRECTION_ID__N = 0;
    static constexpr size_t DIRECTION_ID__NE = 1;
    static constexpr size_t DIRECTION_ID__E = 2;
    static constexpr size_t DIRECTION_ID__SE = 3;
    static constexpr size_t DIRECTION_ID__S = 4;
    static constexpr size_t DIRECTION_ID__W = 5;
    static constexpr size_t DIRECTION_ID__SW = 6;
    static constexpr size_t DIRECTION_ID__NW = 7;
    // Useful array of all directions.
    static constexpr size_t NUM_DIRECTIONS = 8;   ///< Number of neighbors each board space has.

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

  protected:

    std::array<size_t, NUM_DIRECTIONS> ALL_DIRECTIONS;
    emp::vector<int> neighbors; ///< On construction, pre-compute adjacency network.

    bool over = false;    ///< Is the game over?
    size_t cur_player;    ///< Is it DARK player's turn?
    size_t board_size;    ///< Game board is board_size X board_size
    board_t game_board;   ///< Game board

    /// Internal function for accessing the neighbors vector.
    size_t GetNeighborIndex(size_t posID, size_t dir) const {
      return (posID * NUM_DIRECTIONS) + dir;
    }

    /// Internal GetNeighbor function (actually does work).
    int GetNeighbor__Internal(size_t id, size_t dir) const {
      emp_assert(dir >= 0 && dir <= 7);
      size_t x = GetPosX(id);
      size_t y = GetPosY(id);
      return GetNeighbor__Internal(x, y, dir);
    }

    /// Internal function to get location adjacent to x,y position on board in given direction.
    /// (actually does work) Used when generating the adjacency network.
    int GetNeighbor__Internal(size_t x, size_t y, size_t dir) const {
      emp_assert(dir >= 0 && dir <= 7);
      int facing_x = 0, facing_y = 0;
      switch(dir) {
        case N():  { facing_x = x;   facing_y = y-1; break; }
        case S():  { facing_x = x;   facing_y = y+1; break; }
        case E():  { facing_x = x+1; facing_y = y;   break; }
        case W():  { facing_x = x-1; facing_y = y;   break; }
        case NE(): { facing_x = x+1; facing_y = y-1; break; }
        case NW(): { facing_x = x-1; facing_y = y-1; break; }
        case SE(): { facing_x = x+1; facing_y = y+1; break; }
        case SW(): { facing_x = x-1; facing_y = y+1; break; }
        default:
          std::cout << "Bad direction!" << std::endl;
          break;
      }
      if (!IsValidPos(facing_x, facing_y)) return -1;
      return GetPosID(facing_x, facing_y);
    }

    /// Generates neighbor network (populates neighbors member variable).
    /// Only used during construction.
    void GenerateNeighborNetwork() {
      neighbors.resize(game_board.size() * NUM_DIRECTIONS);
      for (size_t posID = 0; posID < game_board.size(); ++posID) {
        for (size_t dir : ALL_DIRECTIONS) {
          neighbors[GetNeighborIndex(posID, dir)] = GetNeighbor__Internal(posID, dir);
        }
      }
    }

  public:
    Othello(size_t side_len)
      : ALL_DIRECTIONS(), board_size(side_len), game_board(board_size*board_size)
    {
      emp_assert(board_size >= 4);
      ALL_DIRECTIONS = {{N(), NE(), E(), SE(), S(), W(), SW(), NW()}};
      GenerateNeighborNetwork();
      Reset();
    }

    ~Othello() { ; }

    /// Reset the board to the starting condition.
    void Reset() {
      // Reset the board.
      for (size_t i = 0; i < game_board.size(); ++i) game_board[i] = OpenSpace();

      // Setup Initial board
      //  ........
      //  ...LD...
      //  ...DL...
      //  ........
      SetPos(board_size/2 - 1, board_size/2 - 1, LightDisk());
      SetPos(board_size/2 - 1, board_size/2, DarkDisk());
      SetPos(board_size/2, board_size/2 - 1, DarkDisk());
      SetPos(board_size/2, board_size/2, LightDisk());

      over = false;
      cur_player = DarkPlayerID();
    }

    size_t GetBoardWidth() const { return board_size; }
    size_t GetBoardHeight() const { return board_size; }
    size_t GetBoardSize() const { return game_board.size(); }
    size_t GetCurPlayer() const { return cur_player; }

    /// Get opponent ID of give player ID.
    size_t GetOpponentID(size_t playerID) const {
      emp_assert(IsValidPlayer(playerID));
      if (playerID == DarkPlayerID()) return LightPlayerID();
      return DarkPlayerID();
    }

    /// Get x location in board grid given loc ID.
    size_t GetPosX(size_t id) const { return id % board_size; }

    /// Get Y location in board grid given loc ID.
    size_t GetPosY(size_t id) const { return id / board_size; }

    /// Get board ID of  given an x, y position.
    size_t GetPosID(size_t x, size_t y) const { return (y * board_size) + x; }

    /// Is the given x,y position valid?
    bool IsValidPos(size_t x, size_t y) const {
      return x < board_size && y < board_size;
    }

    /// Is the given board position ID a valid position on the board?
    bool IsValidPos(size_t id) const { return id < game_board.size(); }

    /// Is the given player ID a valid player?
    bool IsValidPlayer(size_t playerID) const { return (playerID == DarkPlayerID()) || (playerID == LightPlayerID()); }

    /// Get location adjacent to ID in direction dir.
    /// GetNeighbor function is save with garbage ID values.
    int GetNeighbor(size_t id, size_t dir) const {
      emp_assert(dir >= 0 && dir <= 7);
      if (!IsValidPos(id)) return -1; // If not valid position to lookup, return -1.
      return neighbors[GetNeighborIndex(id, dir)];
    }

    /// Get location adjacent to x,y position on board in given direction.
    int GetNeighbor(size_t x, size_t y, size_t dir) const {
      emp_assert(dir >= 0 && dir <= 7);
      return GetNeighbor(GetPosID(x,y), dir);
    }

    /// Given a player ID (0 or 1 are valid), what Disk type does that player use?
    BoardSpace GetDiskType(size_t player_id) {
      emp_assert(IsValidPlayer(player_id));
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

    board_t & GetBoard() { return game_board; }
    const board_t & GetBoard() const { return game_board; }

    /// Is give move (move_x, move_y) valid?
    bool IsMoveValid(size_t playerID, size_t move_x, size_t move_y) {
      return IsMoveValid(playerID, GetPosID(move_x, move_y));
    }

    /// Is given move valid?
    bool IsMoveValid(size_t playerID, size_t move_id) {
      emp_assert(IsValidPlayer(playerID));
      // 1) Is move_id valid position on the board?
      if (!IsValidPos(move_id)) return false;
      // 2) The move position must be empty.
      if (GetPosValue(move_id) != OpenSpace()) return false;
      // 3) A non-zero number of tiles must flip.
      return (bool)GetFlipList(playerID, move_id, true).size();
    }

    bool IsOver() const { return over; }

    /// Get positions that would flip if a player (playerID) made a particular move (move_id).
    /// - Does not check move validity.
    /// - If only_first_valid: return the first valid flip set (in any direction).
    emp::vector<size_t> GetFlipList(size_t playerID, size_t move_id, bool only_first_valid=false) {
      emp::vector<size_t> flip_list;
      size_t prev_len = 0;
      for (size_t dir : ALL_DIRECTIONS) {
        int neighborID = GetNeighbor(move_id, dir);
        while (neighborID != -1) {
          if (GetPosValue(neighborID) == OpenSpace()) {
            flip_list.resize(prev_len);
            break;
          } else if (GetPosOwner(neighborID) == GetOpponentID(playerID)) {
            flip_list.emplace_back(neighborID);
          } else {
            prev_len = flip_list.size();
            break;
          }
          neighborID = GetNeighbor((size_t)neighborID, dir);
        }
        flip_list.resize(prev_len);
        if (only_first_valid && flip_list.size()) break;
      }
      return flip_list;
    }

    /// Get a list of valid move options for given disk_type.
    emp::vector<size_t> GetMoveOptions(BoardSpace disk_type) {
      return GetMoveOptions(GetPlayerID(disk_type));
    }

    /// Get a list of valid moves for the given player ID on the current board state.
    emp::vector<size_t> GetMoveOptions(size_t playerID) {
      emp_assert(IsValidPlayer(playerID));
      emp::vector<size_t> valid_moves;
      for (size_t i = 0; i < game_board.size(); ++i) {
        if (IsMoveValid(playerID, i)) valid_moves.emplace_back(i);
      }
      return valid_moves;
    }

    /// Get the current score for a given player.
    double GetScore(size_t playerID) {
      emp_assert(IsValidPlayer(playerID));
      double score = 0;
      for (size_t i = 0; i < game_board.size(); ++i) {
        if (GetPosOwner(i) == playerID) score++;
      }
      return score;
    }

    /// Get number of frontier positions for given player.
    /// Frontier position -- number empty squares adjacent to a player's pieces.
    size_t GetFrontierPosCnt(size_t playerID) {
      emp_assert(IsValidPlayer(playerID));
      size_t frontier_size = 0;
      for (size_t i = 0; i < game_board.size(); ++i) {
        // Are we looking at an empty space?
        if (game_board[i] == OpenSpace()) {
          // If adjacent to playerID's token: increment player's frontier size.
          if (IsAdjacentTo(i, GetDiskType(playerID))) ++frontier_size;
        }
      }
      return frontier_size;
    }

    /// Is position given by ID adjacent to the space type given by space?
    bool IsAdjacentTo(size_t id, BoardSpace space) {
      for (size_t dir : ALL_DIRECTIONS) {
        int nID = GetNeighbor(id, dir);
        if (nID == -1) continue;
        if (GetPosValue((size_t)nID) == space) return true;
      }
      return false;
    }

    /// Is position given by x,y adjacent to the space type given by space?
    bool IsAdjacentTo(size_t x, size_t y, BoardSpace space) {
      return IsAdjacentTo(GetPosID(x,y), space);
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
      emp_assert(IsValidPlayer(playerID));
      SetPos(id, GetDiskType(playerID));
    }

    /// Set positions given by ids to belong to given player.
    void SetPositions(emp::vector<size_t> ids, size_t playerID) {
      for (size_t i = 0; i < ids.size(); ++i) SetPos(ids[i], playerID);
    }

    /// Set positions given by ids to be given board space type.
    void SetPositions(emp::vector<size_t> ids, BoardSpace space) {
      for (size_t i = 0; i < ids.size(); ++i) SetPos(ids[i], space);
    }

    /// Set board position (x,y) to disk type used by playerID.
    void SetPos(size_t x, size_t y, size_t playerID) {
      emp_assert(IsValidPlayer(playerID));
      SetPos(GetPosID(x, y), GetDiskType(playerID));
    }

    /// Configure board as given by copy_board input.
    /// copy_board size must match game_board's size.
    void SetBoard(const board_t & other_board) {
      emp_assert(other_board.size() == game_board.size());
      for (size_t i = 0; i < game_board.size(); ++i) {
        game_board[i] = other_board[i];
      }
    }

    /// Set current board to be the same as board from other othello game.
    void SetBoard(const Othello & other_othello) { SetBoard(other_othello.GetBoard()); }

    /// Set the current player.
    void SetCurPlayer(size_t playerID) {
      emp_assert(IsValidPlayer(playerID));
      cur_player = playerID;
    }

    /// Do current player's move (moveID).
    /// Return bool indicating whether current player goes again. (false=new cur player or game over)
    bool DoNextMove(size_t moveID) {
      emp_assert(IsValidPos(moveID));
      return DoMove(cur_player, moveID);
    }

    /// Do current player's move (moveID).
    /// Return bool indicating whether current player goes again. (false=new cur player or game over)
    bool DoNextMove(size_t x, size_t y) {
      return DoNextMove(GetPosID(x,y));
    }

    /// Do move from any player's perspective.
    bool DoMove(size_t playerID, size_t x, size_t y) {
      return DoMove(playerID, GetPosID(x,y));
    }

    /// Do move (moveID) for player (playerID). Return bool whether player can go again.
    /// After making move, update current player.
    /// NOTE: Does not check validity.
    /// Will switch cur_player from playerID to Opp(playerID) if opponent has a move to make.
    bool DoMove(size_t playerID, size_t moveID) {
      emp_assert(IsValidPlayer(playerID));
      // 1) Take position for playerID.
      SetPos(moveID, playerID);
      // 2) Affect board appropriately: flip tiles!
      DoFlips(playerID, moveID);
      // 3) Who's turn is next?
      emp::vector<size_t> player_moves = GetMoveOptions(playerID);
      emp::vector<size_t> opp_moves = GetMoveOptions(GetOpponentID(playerID));
      bool go_again = false;
      if (!player_moves.size() && !opp_moves.size()) {
        // No one has any moves! Game over!
        over = true;
      } else if (!opp_moves.size()) {
        // Opponent has no moves! PlayerID goes again!
        go_again = true;
      } else {
        // Opponent has a move to make! Give 'em a shot at it.
        cur_player = GetOpponentID(playerID);
      }
      return go_again;
    }

    /// NOTE: does not check for move validity.
    void DoFlips(size_t playerID, size_t moveID) {
      emp_assert(IsValidPlayer(playerID));
      vector<size_t> flip_list = GetFlipList(playerID, moveID);
      for (size_t flip : flip_list) { SetPos(flip, playerID); }
    }

    /// Print board state to given ostream.
    void Print(std::ostream & os=std::cout, std::string dark_token = "D",
                std::string light_token = "L", std::string open_space = "O") {
      // Output column labels.
      unsigned char letter = 'A';
      os << "\n  ";
      for (size_t i = 0; i < board_size; ++i) os << char(letter + i) << " ";
      os << "\n";
      // Output row labels and board information.
      for (size_t y = 0; y < board_size; ++y) {
        os << y << " ";
        for (size_t x = 0; x < board_size; ++x) {
          BoardSpace space = GetPosValue(x,y);
          if (space == DarkDisk())  { os << dark_token << " "; }
          else if (space == LightDisk()) { os << light_token << " "; }
          else { os << open_space << " "; }
        }
        os << "\n";
      }
    }
  };
}

#endif
