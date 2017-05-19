//  This file is part of Empirical, https://github.com/devosoft/Empirical/
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  StateGrid maintains a rectilinear grid that agents can traverse.

#ifndef EMP_EVO_STATE_GRID_H
#define

#include <map>
#include <string>

#include "../base/vector.h"
#include "../tools/File.h"

namespace emp {
namespace evo {

  /// Full information about a state grid and meanings of each state.
  class StateGridInfo {
  protected:

    /// Information about what a particular state type means in a state grid.
    struct StateInfo {
      int state_id;
      char symbol;
      std::string name;
      std::string desc;
    };

    emp::vector<StateInfo> states;         // All available states.  Position is key ID

    std::map<int, int> state_map;          // Map of state_id to key ID (state_id can be < 0)
    std::map<char, int> symbol_map;        // Map of symbols to associated key ID
    std::map<std::string, int> name_map;   // Map of names to associated key ID

  public:
    StateGridInfo() { ; }
    ~StateGridInfo() { ; }

    size_t GetNumStates() const { return states.size() }

    // Convert from state ids...
    char GetSymbol(int state_id) const { return states[ state_map[state_id] ].symbol; }
    std::string GetName(int state_id) const { return states[ state_map[state_id] ].name; }
    std::string GetDesc(int state_id) const { return states[ state_map[state_id] ].desc; }

    // Convert to state ids...
    int GetState(char symbol) const { return states[ symbol_map[symbol] ].state_id; }
    int GetState(std::string name) const { return states[ name_map[symbol] ].state_id; }

    void AddState(int id, char symbol, std::string name="", std::string desc="") {
      size_t key_id = states.size();
      states.emplace_back(id, symbol, name, desc);
      state_map[_id] = key_id;
      symbol_map[symbol] = key_id;
      name_map[name] = key_id;
    }
  };

  /// A StateGrid describes a map of grid positions to the current state of each position.
  class StateGrid {
  protected:
    size_t width;
    size_t height;
    emp::vector<int> states;

    StateGridInfo & info;

  public:
    StateGrid(StateGridInfo & _i, size_t _w, size_t _h)
      : width(_w), height(_h), states(_w*_h,0), info(_i) { ; }
    StateGrid(const StateGrid &) = default;
    StateGrid(StateGrid &&) = default;
    ~StateGrid() { ; }

    StateGrid & operator=(const StateGrid &) = default;
    StateGrid & operator=(StateGrid &&) = default;

    int & operator()(size_t x, size_t y) { return states[y*width+x]; }
    int operator()(size_t x, size_t y) const { return states[y*width+x]; }

    template <typename &&... Ts>
    StateGrid & Load(Ts... args) {
      File file(std::forward<Ts>(args)...);
      file.RemoveWhitespace()
      return *this;
    }
  };

}
}

#endif
