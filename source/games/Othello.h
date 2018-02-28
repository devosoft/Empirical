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

  /// Base class for all sizes of Othello games.
  struct Othello_Base {
    enum Player { NONE=0, DARK, LIGHT };         ///< All possible states of a board space.
    enum Facing { N, NE, E, SE, S, SW, W, NW };  ///< All possible directions
    static constexpr size_t NUM_DIRECTIONS = 8;  ///< Number of neighbors each board space has.
  };

  /// Class for othello games of a specific size.
  template <size_t BOARD_SIZE>
  class Othello_Game : public Othello_Base {
  public:
    static constexpr size_t BOARD_CELLS = BOARD_SIZE * BOARD_SIZE;
    using this_t = Othello_Game<BOARD_SIZE>;
    using board_t = std::array<Player, BOARD_CELLS>;

  protected:
    std::array<Facing, NUM_DIRECTIONS> ALL_DIRECTIONS;
    emp::vector<int> neighbors; ///< On construction, pre-compute adjacency network.

    bool over = false;    ///< Is the game over?
    Player cur_player;    ///< Who is the current player set to move next?
    board_t game_board;   ///< Game board

    /// Internal function for accessing the neighbors vector.
    size_t GetNeighborIndex(size_t posID, Facing dir) const {
      return (posID * NUM_DIRECTIONS) + (size_t) dir;
    }

    /// Internal GetNeighbor function.
    int GetNeighbor__Internal(size_t id, Facing dir) const {
      size_t x = GetPosX(id);
      size_t y = GetPosY(id);
      int facing_x = 0, facing_y = 0;
      switch(dir) {
        case Facing::N:  { facing_x = x;   facing_y = y-1; break; }
        case Facing::S:  { facing_x = x;   facing_y = y+1; break; }
        case Facing::E:  { facing_x = x+1; facing_y = y;   break; }
        case Facing::W:  { facing_x = x-1; facing_y = y;   break; }
        case Facing::NE: { facing_x = x+1; facing_y = y-1; break; }
        case Facing::NW: { facing_x = x-1; facing_y = y-1; break; }
        case Facing::SE: { facing_x = x+1; facing_y = y+1; break; }
        case Facing::SW: { facing_x = x-1; facing_y = y+1; break; }
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
        for (Facing dir : ALL_DIRECTIONS) {
          neighbors[GetNeighborIndex(posID, dir)] = GetNeighbor__Internal(posID, dir);
        }
      }
    }

  public:
    Othello_Game()
      : ALL_DIRECTIONS(), game_board()
    {
      emp_assert(BOARD_SIZE >= 4);
      ALL_DIRECTIONS = { Facing::N, Facing::NE, Facing::E, Facing::SE,
                         Facing::S, Facing::SW, Facing::W, Facing::NW };
      GenerateNeighborNetwork();
      Reset();
    }

    ~Othello_Game() { ; }

    /// Reset the board to the starting condition.
    void Reset() {
      // Reset the board.
      for (size_t i = 0; i < game_board.size(); ++i) game_board[i] = Player::NONE;

      // Setup Initial board
      //  ........
      //  ...LD...
      //  ...DL...
      //  ........
      SetPos(BOARD_SIZE/2 - 1, BOARD_SIZE/2 - 1, Player::LIGHT);
      SetPos(BOARD_SIZE/2 - 1, BOARD_SIZE/2, Player::DARK);
      SetPos(BOARD_SIZE/2, BOARD_SIZE/2 - 1, Player::DARK);
      SetPos(BOARD_SIZE/2, BOARD_SIZE/2, Player::LIGHT);

      over = false;
      cur_player = Player::DARK;
    }

    constexpr size_t GetBoardWidth() const { return BOARD_SIZE; }
    size_t GetBoardCells() const { return game_board.size(); }
    Player GetCurPlayer() const { return cur_player; }

    /// Get opponent ID of give player ID.
    Player GetOpponent(Player player) const {
      emp_assert(IsValidPlayer(player));
      if (player == Player::DARK) return Player::LIGHT;
      return Player::DARK;
    }

    /// Get x location in board grid given loc ID.
    size_t GetPosX(size_t id) const { return id % BOARD_SIZE; }

    /// Get Y location in board grid given loc ID.
    size_t GetPosY(size_t id) const { return id / BOARD_SIZE; }

    /// Get board ID of  given an x, y position.
    size_t GetPosID(size_t x, size_t y) const { return (y * BOARD_SIZE) + x; }

    /// Is the given x,y position valid?
    bool IsValidPos(size_t x, size_t y) const { return x < BOARD_SIZE && y < BOARD_SIZE; }

    /// Is the given board position ID a valid position on the board?
    bool IsValidPos(size_t id) const { return id < game_board.size(); }

    /// Is the given player ID a valid player?
    bool IsValidPlayer(Player player) const { return (player == Player::DARK) || (player == Player::LIGHT); }

    /// Get location adjacent to ID in direction dir.
    /// GetNeighbor function is save with garbage ID values.
    int GetNeighbor(size_t id, Facing dir) const {
      if (!IsValidPos(id)) return -1; // If not valid position to lookup, return -1.
      return neighbors[GetNeighborIndex(id, dir)];
    }

    /// Get location adjacent to x,y position on board in given direction.
    int GetNeighbor(size_t x, size_t y, Facing dir) const {
      return GetNeighbor(GetPosID(x,y), dir);
    }

    /// Get the value (light, dark or open) at an x,y location on the board.
    Player GetPosOwner(size_t x, size_t y) const {
      return GetPosOwner(GetPosID(x, y));
    }

    /// Get the value (light, dark, or open) at a position on the board.
    Player GetPosOwner(size_t id) const {
      emp_assert(id < game_board.size());
      return game_board[id];
    }

    board_t & GetBoard() { return game_board; }
    const board_t & GetBoard() const { return game_board; }

    /// Is give move (move_x, move_y) valid?
    bool IsValidMove(Player player, size_t move_x, size_t move_y) {
      return IsValidMove(player, GetPosID(move_x, move_y));
    }

    /// Is given move valid?
    bool IsValidMove(Player player, size_t move_id) {
      emp_assert(IsValidPlayer(player));
      // 1) Is move_id valid position on the board?
      if (!IsValidPos(move_id)) return false;
      // 2) The move position must be empty.
      if (GetPosOwner(move_id) != Player::NONE) return false;
      // 3) A non-zero number of tiles must flip.
      return (bool)GetFlipList(player, move_id, true).size();
    }

    bool IsOver() const { return over; }

    /// Get positions that would flip if a player (player) made a particular move (move_id).
    /// - Does not check move validity.
    /// - If only_first_valid: return the first valid flip set (in any direction).
    emp::vector<size_t> GetFlipList(Player player, size_t move_id, bool only_first_valid=false) {
      emp::vector<size_t> flip_list;
      size_t prev_len = 0;
      for (Facing dir : ALL_DIRECTIONS) {
        int neighbor_pos = GetNeighbor(move_id, dir);
        while (neighbor_pos != -1) {
          if (GetPosOwner(neighbor_pos) == Player::NONE) {
            flip_list.resize(prev_len);
            break;
          } else if (GetPosOwner(neighbor_pos) == GetOpponent(player)) {
            flip_list.emplace_back(neighbor_pos);
          } else {
            prev_len = flip_list.size();
            break;
          }
          neighbor_pos = GetNeighbor((size_t)neighbor_pos, dir);
        }
        flip_list.resize(prev_len);
        if (only_first_valid && flip_list.size()) break;
      }
      return flip_list;
    }

    /// Get a list of valid move options for given player.
    emp::vector<size_t> GetMoveOptions(Player player) {
      emp_assert(IsValidPlayer(player));
      emp::vector<size_t> valid_moves;
      for (size_t i = 0; i < game_board.size(); ++i) {
        if (IsValidMove(player, i)) valid_moves.emplace_back(i);
      }
      return valid_moves;
    }

    /// Get the current score for a given player.
    double GetScore(Player player) {
      emp_assert(IsValidPlayer(player));
      return std::count(game_board.begin(), game_board.end(), player);
    }

    /// Get number of frontier positions for given player.
    /// Frontier position -- number empty squares adjacent to a player's pieces.
    size_t GetFrontierPosCnt(Player player) {
      emp_assert(IsValidPlayer(player));
      size_t frontier_size = 0;
      for (size_t i = 0; i < game_board.size(); ++i) {
        // Are we looking at an empty space?
        if (game_board[i] == Player::NONE) {
          // If adjacent to player's token: increment player's frontier size.
          if (IsAdjacentTo(i, player)) ++frontier_size;
        }
      }
      return frontier_size;
    }

    /// Is position given by ID adjacent to the given owner?
    bool IsAdjacentTo(size_t id, Player owner) {
      for (Facing dir : ALL_DIRECTIONS) {
        int nID = GetNeighbor(id, dir);
        if (nID == -1) continue;
        if (GetPosOwner((size_t)nID) == owner) return true;
      }
      return false;
    }

    /// Is position given by x,y adjacent to the given owner?
    bool IsAdjacentTo(size_t x, size_t y, Player owner) {
      return IsAdjacentTo(GetPosID(x,y), owner);
    }

    /// Set board position (ID) to given space value.
    void SetPos(size_t id, Player player) {
      emp_assert(id < game_board.size());
      game_board[id] = player;
    }

    /// Set board position (x,y) to given space value.
    void SetPos(size_t x, size_t y, Player player) {
      SetPos(GetPosID(x, y), player);
    }

    /// Set positions given by ids to be owned by the given player.
    void SetPositions(emp::vector<size_t> ids, Player player) {
      for (auto x : ids) SetPos(x, player);
    }

    /// Configure board as given by copy_board input.
    /// copy_board size must match game_board's size.
    void SetBoard(const board_t & other_board) {
      emp_assert(other_board.size() == game_board.size());
      game_board = other_board;
    }

    /// Set current board to be the same as board from other othello game.
    void SetBoard(const this_t & other_othello) { SetBoard(other_othello.GetBoard()); }

    /// Set the current player.
    void SetCurPlayer(Player player) {
      emp_assert(IsValidPlayer(player));
      cur_player = player;
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
    bool DoMove(Player player, size_t x, size_t y) {
      return DoMove(player, GetPosID(x,y));
    }

    /// Do move (moveID) for player. Return bool whether player can go again.
    /// After making move, update current player.
    /// NOTE: Does not check validity.
    /// Will switch cur_player from player to Opp(player) if opponent has a move to make.
    bool DoMove(Player player, size_t moveID) {
      emp_assert(IsValidPlayer(player));
      // 1) Take position for player.
      SetPos(moveID, player);
      // 2) Affect board appropriately: flip tiles!
      DoFlips(player, moveID);
      // 3) Who's turn is next?
      emp::vector<size_t> player_moves = GetMoveOptions(player);
      emp::vector<size_t> opp_moves = GetMoveOptions(GetOpponent(player));
      bool go_again = false;
      if (!player_moves.size() && !opp_moves.size()) {
        // No one has any moves! Game over!
        over = true;
      } else if (!opp_moves.size()) {
        // Opponent has no moves! Player goes again!
        go_again = true;
      } else {
        // Opponent has a move to make! Give 'em a shot at it.
        cur_player = GetOpponent(player);
      }
      return go_again;
    }

    /// NOTE: does not check for move validity.
    void DoFlips(Player player, size_t moveID) {
      emp_assert(IsValidPlayer(player));
      vector<size_t> flip_list = GetFlipList(player, moveID);
      for (size_t flip : flip_list) { SetPos(flip, player); }
    }

    /// Print board state to given ostream.
    void Print(std::ostream & os=std::cout, std::string dark_token = "D",
                std::string light_token = "L", std::string open_space = "O") {
      // Output column labels.
      unsigned char letter = 'A';
      os << "\n  ";
      for (size_t i = 0; i < BOARD_SIZE; ++i) os << char(letter + i) << " ";
      os << "\n";
      // Output row labels and board information.
      for (size_t y = 0; y < BOARD_SIZE; ++y) {
        os << y << " ";
        for (size_t x = 0; x < BOARD_SIZE; ++x) {
          Player space = GetPosOwner(x,y);
          if (space == Player::DARK)  { os << dark_token << " "; }
          else if (space == Player::LIGHT) { os << light_token << " "; }
          else { os << open_space << " "; }
        }
        os << "\n";
      }
    }
  };

  using Othello = Othello_Game<8>;
}

#endif
