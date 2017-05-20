//  This file is part of Empirical, https://github.com/devosoft/Empirical/
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  StateGrid maintains a rectilinear grid that agents can traverse.
//
//
//  Developer Notes:
//  * Functions such as Load() should throw exceptions (or equilv.), not use asserts.

#ifndef EMP_EVO_STATE_GRID_H
#define EMP_EVO_STATE_GRID_H

#include <map>
#include <string>

#include "../base/vector.h"
#include "../tools/File.h"
#include "../tools/map_utils.h"

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

      StateInfo(int _id, char _sym, const std::string & _name, const std::string & _desc)
      : state_id(_id), symbol(_sym), name(_name), desc(_desc) { ; }
    };

    emp::vector<StateInfo> states;         // All available states.  Position is key ID

    std::map<int, size_t> state_map;          // Map of state_id to key ID (state_id can be < 0)
    std::map<char, size_t> symbol_map;        // Map of symbols to associated key ID
    std::map<std::string, size_t> name_map;   // Map of names to associated key ID

    size_t GetKey(int state_id) const { return Find(state_map, state_id, 0); }
    size_t GetKey(char symbol) const { return Find(symbol_map, symbol, 0); }
    size_t GetKey(const std::string & name) const { return Find(name_map, name, 0); }
  public:
    StateGridInfo() { ; }
    ~StateGridInfo() { ; }

    size_t GetNumStates() const { return states.size(); }

    // Convert from state ids...
    char GetSymbol(int state_id) const { return states[ GetKey(state_id) ].symbol; }
    std::string GetName(int state_id) const { return states[ GetKey(state_id) ].name; }
    std::string GetDesc(int state_id) const { return states[ GetKey(state_id) ].desc; }

    // Convert to state ids...
    int GetState(char symbol) const { return states[ GetKey(symbol) ].state_id; }
    int GetState(const std::string & name) const { return states[ GetKey(name) ].state_id; }

    void AddState(int id, char symbol, std::string name="", std::string desc="") {
      size_t key_id = states.size();
      states.emplace_back(id, symbol, name, desc);
      state_map[id] = key_id;
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
    StateGrid(StateGridInfo & _i, size_t _w=1, size_t _h=1, int init_val=0)
      : width(_w), height(_h), states(_w*_h,init_val), info(_i) { ; }
    StateGrid(StateGridInfo & _i, const std::string & filename)
      : info(_i) { Load(filename); }
    StateGrid(const StateGrid &) = default;
    StateGrid(StateGrid &&) = default;
    ~StateGrid() { ; }

    StateGrid & operator=(const StateGrid &) = default;
    StateGrid & operator=(StateGrid &&) = default;

    size_t GetWidth() const { return width; }
    size_t GetHeight() const { return height; }
    size_t GetSize() const { return states.size(); }

    int & operator()(size_t x, size_t y) { return states[y*width+x]; }
    int operator()(size_t x, size_t y) const { return states[y*width+x]; }

    template <typename... Ts>
    StateGrid & Load(Ts &&... args) {
      // Load this data from a stream or a file.
      File file(std::forward<Ts>(args)...);
      file.RemoveWhitespace();

      // Determine the size of the new grid.
      height = file.GetNumLines();
      emp_assert(height > 0);
      width = file[0].size();
      emp_assert(width > 0);

      // Now that we have the new size, resize the state grid.
      size_t size = width * height;
      states.resize(size);

      // Load in the specific states.
      for (size_t row = 0; row < height; row++) {
        emp_assert(file[row].size == width);  // Make sure all rows are the same size.
        for (size_t col = 0; col < width; col++) {
          states[row*width+col] = info.GetState(file[row][col]);
        }
      }

      return *this;
    }
  };

}
}

#endif
