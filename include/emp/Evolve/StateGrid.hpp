/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2017-2018 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/Evolve/StateGrid.hpp
 * @brief StateGrid maintains a rectilinear grid that agents can traverse.
 *
 * State grids are a matrix of values, representing states of a 2D environment that an organism
 * can traverse.
 *
 * @todo Functions such as Load() should throw exceptions (or equiv.), not use asserts.
 * @todo Need to figure out a default mapping for how outputs translate to moves around a
 *   state grid.  -1 = Back up ; 0 = Turn left ; 1 = Move fast-forwards ; 2 = Turn right
 * @todo Allow StateGridInfo to be built inside of StateGrid (change reference to pointer and
 *   possible ownership)
 */

#pragma once

#ifndef INCLUDE_EMP_EVOLVE_STATE_GRID_HPP_GUARD
#define INCLUDE_EMP_EVOLVE_STATE_GRID_HPP_GUARD


#include <map>
#include <stddef.h>
#include <string>

#include "../base/assert.hpp"
#include "../base/Ptr.hpp"
#include "../base/vector.hpp"
#include "../bits/BitVector.hpp"
#include "../datastructs/map_utils.hpp"
#include "../io/File.hpp"
#include "../math/math.hpp"
#include "../math/Random.hpp"

namespace emp {

  /// Full information about the states available in a state grid and meanings of each state.
  class StateGridInfo {
  public:
    using key_t = uint32_t;
  protected:
    /// Information about what a particular state type means in a state grid.
    struct StateInfo {
      int state_id;         ///< Ordinal id for this state.
      char symbol;          ///< Symbol for printing this state.
      double score_change;  ///< Change amount for organism score by stepping on this square.
      std::string name;     ///< Name of this state.
      std::string desc;     ///< Explanation of this state.

      StateInfo(int _id,
                char _sym,
                double _change,
                const std::string & _name,
                const std::string & _desc)
        : state_id(_id), symbol(_sym), score_change(_change), name(_name), desc(_desc) {
        ;
      }

      StateInfo(const StateInfo &) = default;
      StateInfo(StateInfo &&)      = default;

      ~StateInfo() { ; }

      StateInfo & operator=(const StateInfo &) = default;
      StateInfo & operator=(StateInfo &&)      = default;
    };

    emp::vector<StateInfo> states;  ///< All available states.  Position is key ID

    std::map<int, key_t> state_map;         ///< Map of state_id to key ID (state_id can be < 0)
    std::map<char, key_t> symbol_map;       ///< Map of symbols to associated key ID
    std::map<std::string, key_t> name_map;  ///< Map of names to associated key ID

    [[nodiscard]] key_t GetKey(int state_id) const { return Find(state_map, state_id, 0); }

    [[nodiscard]] key_t GetKey(char symbol) const { return Find(symbol_map, symbol, 0); }

    [[nodiscard]] key_t GetKey(const std::string & name) const { return Find(name_map, name, 0); }
  public:
    size_t GetNumStates() const { return states.size(); }

    // Convert from state ids...
    [[nodiscard]] char GetSymbol(int state_id) const { return states[GetKey(state_id)].symbol; }

    [[nodiscard]] double GetScoreChange(int state_id) const {
      return states[GetKey(state_id)].score_change;
    }

    [[nodiscard]] const std::string & GetName(int state_id) const {
      return states[GetKey(state_id)].name;
    }

    [[nodiscard]] const std::string & GetDesc(int state_id) const {
      return states[GetKey(state_id)].desc;
    }

    // Convert to state ids...
    [[nodiscard]] int GetState(char symbol) const {
      emp_assert(states.size() > GetKey(symbol), states.size(), symbol, (int) symbol);
      return states[GetKey(symbol)].state_id;
    }

    [[nodiscard]] int GetState(const std::string & name) const {
      return states[GetKey(name)].state_id;
    }

    void AddState(int id,
                  char symbol,
                  double mult      = 1.0,
                  std::string name = "",
                  std::string desc = "") {
      key_t key_id = states.size();
      states.emplace_back(id, symbol, mult, name, desc);
      state_map[id]      = key_id;
      symbol_map[symbol] = key_id;
      name_map[name]     = key_id;
    }
  };

  /// A StateGrid describes a map of grid positions to the current state of each position.
  class StateGrid {
  public:
    using key_t = StateGridInfo::key_t;

  protected:
    key_t width  = 0;        ///< Width of the overall grid
    key_t height = 0;        ///< Height of the overall grid
    emp::vector<int> states;  ///< Specific states at each position in the grid.
    StateGridInfo info;       ///< Information about the set of states used in this grid.

  public:
    StateGrid() = default;

    StateGrid(StateGridInfo & _i, key_t _w = 1, key_t _h = 1, int init_val = 0)
      : width(_w), height(_h), states(_w * _h, init_val), info(_i) {
      ;
    }

    StateGrid(StateGridInfo & _i, const std::string & filename)
      : width(1), height(1), states(), info(_i) {
      Load(filename);
    }

    StateGrid(const StateGrid &) = default;
    StateGrid(StateGrid && in)   = default;
    ~StateGrid()                 = default;

    StateGrid & operator=(const StateGrid &) = default;
    StateGrid & operator=(StateGrid &&)      = default;

    [[nodiscard]] key_t GetWidth() const { return width; }

    [[nodiscard]] key_t GetHeight() const { return height; }

    [[nodiscard]] size_t GetSize() const { return states.size(); }

    [[nodiscard]] const emp::vector<int> GetStates() const { return states; }

    [[nodiscard]] const StateGridInfo & GetInfo() const { return info; }

    [[nodiscard]] int & operator()(key_t x, key_t y) {
      emp_assert(x < width, x, width);
      emp_assert(y < height, y, height);
      return states[y * width + x];
    }

    [[nodiscard]] int operator()(key_t x, key_t y) const {
      emp_assert(x < width, x, width);
      emp_assert(y < height, y, height);
      return states[y * width + x];
    }

    [[nodiscard]] int GetState(key_t x, key_t y) const {
      emp_assert(x < width, x, width);
      emp_assert(y < height, y, height);
      return states[y * width + x];
    }

    [[nodiscard]] int GetState(key_t id) const { return states[id]; }

    StateGrid & SetState(key_t x, key_t y, int in) {
      emp_assert(x < width, x, width);
      emp_assert(y < height, y, height);
      states[y * width + x] = in;
      return *this;
    }

    [[nodiscard]] char GetSymbol(key_t x, key_t y) const {
      emp_assert(x < width, x, width);
      emp_assert(y < height, y, height);
      return info.GetSymbol(GetState(x, y));
    }

    [[nodiscard]] double GetScoreChange(key_t x, key_t y) const {
      emp_assert(x < width, x, width);
      emp_assert(y < height, y, height);
      return info.GetScoreChange(GetState(x, y));
    }

    [[nodiscard]] const std::string & GetName(key_t x, key_t y) const {
      emp_assert(x < width, x, width);
      emp_assert(y < height, y, height);
      return info.GetName(GetState(x, y));
    }

    /// Return a BitVector indicating which positions in the state grid have a particular state.
    [[nodiscard]] emp::BitVector IsState(int target_state) const {
      emp::BitVector sites(states.size());
      for (size_t i = 0; i < states.size(); i++) { sites[i] = (states[i] == target_state); }
      return sites;
    }

    /// Setup the StateGridInfo with possible states.
    template <typename... Ts>
    void AddState(Ts &&... args) {
      info.AddState(std::forward<Ts>(args)...);
    }

    /// Load in the contents of a StateGrid using the file information provided.
    template <typename... Ts>
    StateGrid & Load(Ts &&... args) {
      std::cout << "Loading!" << std::endl;

      // Load this data from a stream or a file.
      File file(std::forward<Ts>(args)...);
      file.RemoveWhitespace();

      // Determine the size of the new grid.
      height = file.GetNumLines();
      emp_assert(height > 0);
      width = file[0].size();
      emp_assert(width > 0);

      // Now that we have the new size, resize the state grid.
      key_t size = width * height;
      states.resize(size);

      // Load in the specific states.
      for (key_t row = 0; row < height; row++) {
        emp_assert(file[row].size() == width);  // Make sure all rows are the same size.
        for (key_t col = 0; col < width; col++) {
          states[row * width + col] = info.GetState(file[row][col]);
        }
      }

      return *this;
    }

    /// Print the current status of the StateGrid to an output stream.
    template <typename... Ts>
    const StateGrid & Print(std::ostream & os = std::cout) const {
      std::string out(width * 2 - 1, ' ');
      for (key_t i = 0; i < height; i++) {
        out[0] = info.GetSymbol(states[i * width]);
        for (key_t j = 1; j < width; j++) { out[j * 2] = info.GetSymbol(states[i * width + j]); }
        os << out << std::endl;
      }
      return *this;
    }

    /// Store the current status of the StateGrid to a file.
    template <typename... Ts>
    const StateGrid & Write(Ts &&... args) const {
      File file;
      std::string out;
      for (key_t i = 0; i < height; i++) {
        out.resize(0);
        out += info.GetSymbol(states[i * width]);
        for (key_t j = 1; j < width; j++) {
          out += ' ';
          out += info.GetSymbol(states[i * width + j]);
        }
        file.Append(out);
      }
      file.Write(std::forward<Ts>(args)...);
      return *this;
    }
  };

  /// Information about a particular agent on a state grid.
  class StateGridStatus {
  public:
    using key_t = StateGridInfo::key_t;

  protected:
    struct State {
      key_t x   = 0;  ///< X-coordinate of this agent
      key_t y   = 0;  ///< Y-coordinate of this agent.
      int facing = 1;  ///< 0=UL, 1=Up, 2=UR, 3=Right, 4=DR, 5=Down, 6=DL, 7=Left (+=Clockwise)

      State() = default;

      State(key_t _x, key_t _y, key_t _f = 1) : x(_x), y(_y), facing((int) _f) { ; }

      bool IsAt(key_t _x, key_t _y) const { return x == _x && y == _y; }
    };

    State cur_state;             ///< Position and facing currently used.
    bool track_moves;            ///< Should we record every move made by this organism?
    emp::vector<State> history;  ///< All previous positions and facings in this path.

    // --- Helper Functions ---

    /// If we are tracking moves, store the current position in the history.
    void UpdateHistory() {
      if (track_moves) { history.push_back(cur_state); }
    }

    /// Move explicitly in the x direction (regardless of facing).
    void MoveX(const StateGrid & grid, int steps = 1) {
      emp_assert(grid.GetWidth(), grid.GetWidth());
      cur_state.x = (key_t) Mod(steps + (int) cur_state.x, (int) grid.GetWidth());
    }

    /// Move explicitly in the y direction (regardless of facing).
    void MoveY(const StateGrid & grid, int steps = 1) {
      emp_assert(grid.GetHeight(), grid.GetHeight());
      cur_state.y = (key_t) Mod(steps + (int) cur_state.y, (int) grid.GetHeight());
    }

  public:
    StateGridStatus()                        = default;
    StateGridStatus(const StateGridStatus &) = default;
    StateGridStatus(StateGridStatus &&)      = default;
    ~StateGridStatus()                       = default;

    StateGridStatus & operator=(const StateGridStatus &) = default;
    StateGridStatus & operator=(StateGridStatus &&)      = default;

    [[nodiscard]] key_t GetX() const { return cur_state.x; }

    [[nodiscard]] key_t GetY() const { return cur_state.y; }

    [[nodiscard]] key_t GetFacing() const {
      emp_assert(cur_state.facing >= 0 && cur_state.facing < 8);
      return (key_t) cur_state.facing;
    }

    [[nodiscard]] bool IsAt(key_t x, key_t y) const { return cur_state.IsAt(x, y); }

    [[nodiscard]] bool WasAt(key_t x, key_t y) const {
      for (const State & state : history) {
        if (state.IsAt(x, y)) { return true; }
      }
      return false;
    }

    /// Get a BitVector indicating the full history of which positions this organism has traversed.
    [[nodiscard]] emp::BitVector GetVisited(const StateGrid & grid) const {
      emp::BitVector at_array(grid.GetSize());
      for (const State & state : history) {
        key_t pos = state.x + grid.GetWidth() * state.y;
        at_array.Set(pos);
      }
      return at_array;
    }

    StateGridStatus & TrackMoves(bool track = true) {
      bool prev   = track_moves;
      track_moves = track;
      if (!prev && track_moves) {
        history.push_back(cur_state);
      } else {
        history.resize(0);
      }
      return *this;
    }

    StateGridStatus & Set(key_t _x, key_t _y, key_t _f) {
      cur_state.x      = _x;
      cur_state.y      = _y;
      cur_state.facing = (int) _f;
      UpdateHistory();
      return *this;
    }

    StateGridStatus & SetX(key_t _x) {
      cur_state.x = _x;
      UpdateHistory();
      return *this;
    }

    StateGridStatus & SetY(key_t _y) {
      cur_state.y = _y;
      UpdateHistory();
      return *this;
    }

    StateGridStatus & SetPos(key_t _x, key_t _y) {
      cur_state.x = _x;
      cur_state.y = _y;
      UpdateHistory();
      return *this;
    }

    StateGridStatus & SetFacing(key_t _f) {
      cur_state.facing = (int) _f;
      UpdateHistory();
      return *this;
    }

    /// Move in the direction currently faced.
    void Move(const StateGrid & grid, int steps = 1) {
      // std::cout << "steps = " << steps
      //           << "  facing = " << cur_state.facing
      //           << "  start = (" << cur_state.x << "," << cur_state.y << ")";
      switch (cur_state.facing) {
        case 0:
          MoveX(grid, -steps);
          MoveY(grid, -steps);
          break;
        case 1: MoveY(grid, -steps); break;
        case 2:
          MoveX(grid, +steps);
          MoveY(grid, -steps);
          break;
        case 3: MoveX(grid, +steps); break;
        case 4:
          MoveX(grid, +steps);
          MoveY(grid, +steps);
          break;
        case 5: MoveY(grid, +steps); break;
        case 6:
          MoveX(grid, -steps);
          MoveY(grid, +steps);
          break;
        case 7: MoveX(grid, -steps); break;
      }
      UpdateHistory();
      // std::cout << " end = (" << cur_state.x << "," << cur_state.y << ")"
      //           << "  facing = " << cur_state.facing
      //           << std::endl;
    }

    /// Rotate starting from current facing.
    void Rotate(int turns = 1) {
      cur_state.facing = Mod(cur_state.facing + turns, 8);
      UpdateHistory();
    }

    /// Move the current status to a random position and orientation.
    void Randomize(const StateGrid & grid, Random & random) {
      const uint32_t width = grid.GetWidth();
      const uint32_t height = grid.GetHeight();
      Set(random.GetUInt32(width), random.GetUInt32(height), random.GetUInt32(8));
    }

    /// Examine state of current position.
    int Scan(const StateGrid & grid) {
      return grid(cur_state.x, cur_state.y);
      // @CAO: Should we be recording the scan somehow in history?
    }

    /// Set the current position in the state grid.
    void SetState(StateGrid & grid, int new_state) {
      grid.SetState(cur_state.x, cur_state.y, new_state);
    }

    /// Print the history of an organism moving around a state grid.
    void PrintHistory(StateGrid & grid, std::ostream & os = std::cout) const {
      emp_assert(history.size(), "You can only print history of a StateGrid if you track it!");
      const key_t width  = grid.GetWidth();
      const key_t height = grid.GetHeight();
      std::string out(width * 2 - 1, ' ');
      for (key_t i = 0; i < height; i++) {
        for (key_t j = 1; j < width; j++) {
          out[j * 2] = grid.GetSymbol(j, i);
          if (WasAt(j, i)) { out[j * 2] = '*'; }
        }
        os << out << std::endl;
      }
    }
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_EVOLVE_STATE_GRID_HPP_GUARD

// Local settings for Empecable file checker.
// empecable_words: equiv
