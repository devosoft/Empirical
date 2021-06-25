/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  Othello8.hpp
 *  @brief A simple Othello game state handler limited to an 8x8 board.
 *
 *  @todo Add Hash for boards to be able to cachce moves.
 *  @todo Setup OPTIONAL caching of expensive board measures.
 */

#ifndef EMP_GAME_OTHELLO8_H
#define EMP_GAME_OTHELLO8_H

#include <fstream>
#include <iostream>
#include <iomanip>
#include <unordered_map>

#include "../base/array.hpp"
#include "../base/assert.hpp"
#include "../base/vector.hpp"
#include "../bits/bitset_utils.hpp"
#include "../math/math.hpp"

namespace emp {
  /// NOTE: This game could be made more black-box.
  ///   - Hide almost everything. Only give users access to game-advancing functions (don't allow
  ///     willy-nilly board manipulation, etc). This would let us make lots of assumptions about
  ///     calculating flip lists, scores, etc, which would speed up asking for flip lists, etc. multiple
  ///     times during a turn.

  /// Class for size-8 othello games.
  class Othello8 {
  public:
    enum Player { DARK=0, LIGHT=1, NONE };          ///< All possible states of a board space.
    enum Facing { N, NE, E, SE, S, SW, W, NW };     ///< All possible directions
    static constexpr size_t NUM_DIRECTIONS = 8;     ///< Number of neighbors each board space has.
    static constexpr size_t BOARD_SIZE = 8;         ///< Size of a side of the board.
    static constexpr size_t NUM_CELLS = 64;         ///< Number of cells on total board.
    using this_t = Othello8;

    struct Index {
      size_t pos;

      constexpr Index() : pos(NUM_CELLS) { ; }  // Default constructor is invalid position.
      constexpr Index(size_t _pos) : pos(_pos) { emp_assert(pos <= NUM_CELLS); }
      constexpr Index(size_t x, size_t y) : pos() { Set(x,y); }
      constexpr Index(const Index & _in) : pos(_in.pos) { emp_assert(pos <= NUM_CELLS); }

      operator size_t() const { return pos; }
      size_t x() const { return pos & 7; }
      size_t y() const { return pos >> 3; }
      void Set(size_t x, size_t y) { pos = (x<BOARD_SIZE && y<BOARD_SIZE) ? (x+(y<<3)) : NUM_CELLS; }
      bool IsValid() const { return pos < NUM_CELLS; }

      Index CalcNeighbor(Facing dir) {
        Index faced_id;
        switch(dir) {
          case Facing::N:  { faced_id.Set(x()    , y() - 1); break; }
          case Facing::S:  { faced_id.Set(x()    , y() + 1); break; }
          case Facing::E:  { faced_id.Set(x() + 1, y()    ); break; }
          case Facing::W:  { faced_id.Set(x() - 1, y()    ); break; }
          case Facing::NE: { faced_id.Set(x() + 1, y() - 1); break; }
          case Facing::NW: { faced_id.Set(x() - 1, y() - 1); break; }
          case Facing::SE: { faced_id.Set(x() + 1, y() + 1); break; }
          case Facing::SW: { faced_id.Set(x() - 1, y() + 1); break; }
        }
        return faced_id;
      }
    };

    struct Board {
      uint64_t occupied;
      uint64_t player;

      void Clear() { occupied = 0; player = 0; }
      void Clear(Index pos) { uint64_t mask = ~(((uint64_t) 1) << pos); occupied &= mask; player &= mask; }
      Player Owner(Index pos) const {
        uint64_t id = (((uint64_t) 1) << pos);
        if (occupied & id) return (player & id) ? Player::LIGHT : Player::DARK;
        else return Player::NONE;
      }
      void SetOwner(Index pos, Player owner) {
        const uint64_t id = ((uint64_t) 1) << pos;
        occupied |= id;
        if (owner == Player::DARK) player &= ~id;
        else player |= id;
      }
      bool Occupied(Index pos) { return (occupied >> pos) & (size_t) 1; }

      size_t Score(Player owner) {
        if (owner == Player::DARK) return count_bits(occupied & ~player);
        return count_bits(player);
      }
    };

  protected:
    /// All eight cardinal directions.
    static const auto & ALL_DIRECTIONS() {
      static std::array<Facing, NUM_DIRECTIONS> dirs =
        { Facing::N, Facing::NE, Facing::E, Facing::SE,
          Facing::S, Facing::SW, Facing::W, Facing::NW };
      return dirs;
    }

    /// Internal function for accessing the neighbors vector.
    static size_t GetNeighborIndex(Index pos, Facing dir) {
      return (((size_t) pos) * NUM_DIRECTIONS) + (size_t) dir;
    }

    /// Precalculated neighbors
    static const auto & NEIGHBORS() {
      static emp::vector<Index> neighbors;
      if (neighbors.size() == 0) {
        neighbors.resize(NUM_CELLS * NUM_DIRECTIONS);
        for (size_t posID = 0; posID < NUM_CELLS; ++posID) {
          Index pos(posID);
          for (Facing dir : ALL_DIRECTIONS()) {
            neighbors[GetNeighborIndex(posID, dir)] = pos.CalcNeighbor(dir);
          }
        }
      }
      return neighbors;
    }

    bool over = false;    ///< Is the game over?
    Player cur_player;    ///< Who is the current player set to move next?
    Board game_board;     ///< Game board

    using flip_list_t = emp::vector<Index>;
    std::array<flip_list_t, NUM_CELLS> light_flips;
    std::array<flip_list_t, NUM_CELLS> dark_flips;
    bool cache_ok;

  public:
    Othello8() : cur_player(Player::DARK), game_board(), light_flips(), dark_flips(), cache_ok(false) { Reset(); }

    ~Othello8() { ; }

    /// Reset the board to the starting condition.
    void Reset() {
      // Reset the board.
      game_board.Clear();

      // Setup Initial board
      //  ........
      //  ...LD...
      //  ...DL...
      //  ........
      SetPos({BOARD_SIZE/2 - 1, BOARD_SIZE/2 - 1}, Player::LIGHT);
      SetPos({BOARD_SIZE/2 - 1, BOARD_SIZE/2    }, Player::DARK);
      SetPos({BOARD_SIZE/2,     BOARD_SIZE/2 - 1}, Player::DARK);
      SetPos({BOARD_SIZE/2,     BOARD_SIZE/2    }, Player::LIGHT);

      over = false;
      cur_player = Player::DARK;

      cache_ok = false;
    }

    static constexpr Index GetIndex(size_t x, size_t y) { return Index(x, y); }
    static constexpr size_t GetBoardWidth() { return BOARD_SIZE; }
    size_t GetNumCells() const { return NUM_CELLS; }
    Player GetCurPlayer() const { return cur_player; }
    size_t GetHash() const { return game_board.occupied; }

    /// Get opponent ID of give player ID.
    Player GetOpponent(Player player) const {
      return (player == Player::DARK) ? Player::LIGHT : Player::DARK;
    }

    /// Is the given player ID a valid player?
    bool IsValidPlayer(Player player) const { return (player == Player::DARK) || (player == Player::LIGHT); }

    /// Get location adjacent to ID in direction dir.
    /// GetNeighbor function is save with garbage ID values.
    Index GetNeighbor(Index id, Facing dir) {
      if (!id.IsValid()) return Index();
      return NEIGHBORS()[GetNeighborIndex(id, dir)];
    }

    /// Get the value (light, dark, or open) at a position on the board.
    Player GetPosOwner(Index id) const {
      emp_assert(id.IsValid());
      return game_board.Owner(id);
    }

    Board & GetBoard() { return game_board; }
    const Board & GetBoard() const { return game_board; }

    /// Is given move valid?
    bool IsValidMove(Player player, Index pos) {
      emp_assert(IsValidPlayer(player));
      if (!pos.IsValid()) return false;               // Is pos even on the board?
      if (game_board.Occupied(pos)) return false;     // Is pos empty?
      return HasValidFlips(player, pos);              // Will any tiles flip?
    }

    bool IsOver() const { return over; }

    void SetupCache() {
      for (size_t pos = 0; pos < NUM_CELLS; pos++) {
        GetFlipList(Player::LIGHT, pos);
        GetFlipList(Player::DARK, pos);
      }
      cache_ok = true;
    }

    /// Get positions that would flip if a player (player) made a particular move (pos).
    /// Note: May be called before or after piece is placed.
    const emp::vector<Index> & GetFlipList(Player player, Index pos) {
      emp_assert(player != Player::NONE);
      emp::vector<Index> & flip_list = (player == Player::LIGHT) ? light_flips[pos] : dark_flips[pos];
      if (cache_ok == false) {
        flip_list.resize(0);
        size_t prev_len = 0;
        const Player opponent = GetOpponent(player);
        for (Facing dir : ALL_DIRECTIONS()) {
          Index neighbor_pos = GetNeighbor(pos, dir);
          // Collect opponent spaces in this direction.
          while (neighbor_pos.IsValid() && GetPosOwner(neighbor_pos) == opponent) {
            flip_list.emplace_back(neighbor_pos);
            neighbor_pos = GetNeighbor(neighbor_pos, dir);
          }
          // If this line didn't end in current color, throw out everything we found.
          if (!neighbor_pos.IsValid() || !game_board.Occupied(neighbor_pos)) { flip_list.resize(prev_len); }
          // Otherwise keep it and update the locked in flips.
          else { prev_len = flip_list.size(); }
        }
      }
      return flip_list;
    }

    /// Count the number of positions that would flip if we placed a piece at a specific location.
    size_t GetFlipCount(Player player, Index pos) {
      if (cache_ok) {
        return (player == Player::LIGHT) ? light_flips[pos].size() : dark_flips[pos].size();
      }
      size_t flip_count = 0;
      const Player opponent = GetOpponent(player);
      for (Facing dir : ALL_DIRECTIONS()) {
        // Collect opponent spaces in this direction.
        size_t dir_count = 0;
        Index neighbor_pos = GetNeighbor(pos, dir);
        while (neighbor_pos.IsValid() && GetPosOwner(neighbor_pos) == opponent) {
          dir_count++;
          neighbor_pos = GetNeighbor(neighbor_pos, dir);
        }
        // If this line ended in the correct color, add this direction to the total_count.
        if (neighbor_pos.IsValid() && GetPosOwner(neighbor_pos) == player) { flip_count += dir_count; }
      }
      return flip_count;
    }

    /// Are there any valid flips from this position?
    bool HasValidFlips(Player player, Index pos) {
      if (cache_ok) {
        return (player == Player::LIGHT) ? light_flips[pos].size() : dark_flips[pos].size();
      }
      const Player opponent = GetOpponent(player);
      for (Facing dir : ALL_DIRECTIONS()) {             // Loop through directions to explore
        Index neighbor_pos = GetNeighbor(pos, dir);   // Start at first neighbor.
        size_t count = 0;
        // Collect opponent spaces in this direction.
        while (neighbor_pos.IsValid() && GetPosOwner(neighbor_pos) == opponent) {
          count++;
          neighbor_pos = GetNeighbor(neighbor_pos, dir);
        }
        // If this line ended with current color (and has spots to flip) we found a good solution!
        if (count && neighbor_pos.IsValid() && GetPosOwner(neighbor_pos) == player) { return true; }
      }
      return false;
    }


    /// Get a list of valid move options for given player.
    emp::vector<Index> GetMoveOptions(Player player) {
      emp_assert(IsValidPlayer(player));
      emp::vector<Index> valid_moves;
      for (size_t i = 0; i < NUM_CELLS; ++i) {
        if (IsValidMove(player, i)) valid_moves.emplace_back(i);
      }
      return valid_moves;
    }

    /// GetMoveOptions() without a specified player used current player.
    emp::vector<Index> GetMoveOptions() { return GetMoveOptions(cur_player); }

    /// Determine if there are any move options for given player.
    bool HasMoveOptions(Player player) {
      emp_assert(IsValidPlayer(player));
      for (size_t i = 0; i < NUM_CELLS; ++i) {
        if (IsValidMove(player, i)) return true;
      }
      return false;
    }

    /// Get the current score for a given player.
    double GetScore(Player player) {
      emp_assert(IsValidPlayer(player));
      return (double) game_board.Score(player);
    }

    /// Count the number of empty squares adjacent to a player's pieces (frontier size)
    size_t CountFrontierPos(Player player) {
      emp_assert(IsValidPlayer(player));
      size_t frontier_size = 0;
      for (size_t i = 0; i < NUM_CELLS; ++i) {           // Search through all cells
        if (game_board.Occupied(i) == false) {           // Is the test cell empty?
          if (IsAdjacentTo(i, player)) ++frontier_size;  // If so, test if on player's frontier
        }
      }
      return frontier_size;
    }

    /// Is position given by ID adjacent to the given owner?
    bool IsAdjacentTo(Index pos, Player owner) {
      for (Facing dir : ALL_DIRECTIONS()) {
        Index nID = GetNeighbor(pos, dir);
        if (!nID.IsValid()) continue;
        if (GetPosOwner(nID) == owner) return true;
      }
      return false;
    }

    /// Set board position (ID) to given space value.
    void SetPos(Index pos, Player player) {
      emp_assert(pos.IsValid());
      game_board.SetOwner(pos, player);
      cache_ok = false;
    }

    void ClearPos(Index pos) {
      game_board.Clear(pos);
      cache_ok = false;
    }

    /// Set positions given by ids to be owned by the given player.
    void SetPositions(emp::vector<Index> ids, Player player) {
      for (auto x : ids) SetPos(x, player);
    }

    /// Configure board as given by copy_board input.
    /// copy_board size must match game_board's size.
    void SetBoard(const Board & other_board) { game_board = other_board; cache_ok = false; }

    /// Set current board to be the same as board from other othello game.
    void SetBoard(const this_t & other_othello) { SetBoard(other_othello.GetBoard()); }

    /// Set the current player.
    void SetCurPlayer(Player player) {
      emp_assert(IsValidPlayer(player));
      cur_player = player;
    }

    /// Do current player's move (moveID).
    /// Return bool indicating whether current player goes again. (false=new cur player or game over)
    bool DoNextMove(Index pos) { return DoMove(cur_player, pos); }

    /// Do move (at pos) for specified player. Return bool whether player can go again.
    /// After making move, update current player.
    /// NOTE: Does not verify validity.
    /// Will switch cur_player from player to Opp(player) if opponent has a move to make.
    bool DoMove(Player player, Index pos) {
      emp_assert(IsValidPlayer(player) && pos.IsValid());    // Validate position and player.
      emp_assert(GetPosOwner(pos) == Player::NONE);          // Make sure position is empty.
      SetPos(pos, player);                                   // Take position for player.
      DoFlips(player, pos);                                  // Flip tiles on the board.
      auto opp_moves = HasMoveOptions(GetOpponent(player));  // Test if opponent can go.
      if (opp_moves) { cur_player = GetOpponent(player); return false; }

      auto player_moves = HasMoveOptions(player);            // Opponent can't go; test cur player
      if (player_moves) { return true; }                     // This player can go again!

      over = true;                                           // No one can go; game over!
      return false;
    }

    /// NOTE: does not check for move validity.
    void DoFlips(Player player, Index pos) {
      emp_assert(IsValidPlayer(player));
      auto flip_list = GetFlipList(player, pos);
      for (Index flip : flip_list) { SetPos(flip, player); }
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
          Player space = GetPosOwner({x,y});
          if (space == Player::DARK)  { os << dark_token << " "; }
          else if (space == Player::LIGHT) { os << light_token << " "; }
          else { os << open_space << " "; }
        }
        os << "\n";
      }
    }
  };

}

#endif
