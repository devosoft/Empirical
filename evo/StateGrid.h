//  This file is part of Empirical, https://github.com/devosoft/Empirical/
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  StateGrid maintains a rectilinear grid that agents can traverse.

#ifndef EMP_EVO_STATE_GRID_H
#define

namespace emp {
namespace evo {

  class StateGrid {
  private:
    std::vector<int> states;
    size_t width;
    size_t height;

  public:
    StateGrid(size_t _w, size_t _h) : width(_w), height(_h) { ; }
    StateGrid(const StateGrid &) = default;
    StateGrid(StateGrid &&) = default;
    ~StateGrid() { ; }
  };

}
}

#endif
