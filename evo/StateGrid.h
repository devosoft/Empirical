//  This file is part of Empirical, https://github.com/devosoft/Empirical/
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  StateGrid maintains a rectilinear grid that agents can traverse.

#ifndef EMP_EVO_STATE_GRID_H
#define

#include "../base/vector.h"

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

  public:
    StateGridInfo() { ; }
    ~StateGridInfo() { ; }

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
  };

}
}

#endif
