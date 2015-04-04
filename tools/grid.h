#ifndef EMP_GRID_H
#define EMP_GRID_H

///////////////////////////////////////////////////////////////////////////////////////////
//
//  Tools for building grids that can be easily navigated and have state associated with
//  grid cells, edges, and/or intersection points.
//
//  Grid::Board objects are templated based on which of the components they work with.
//  They include three template arguments for the state types associated with cells,
//  edges, and instersection points.  The most commonly use types are:
//
//    int    -- full state; this or an enumerated type should be used for discrete states.
//    bool   -- binary state (on/off)
//    void   -- no state (or extra memory) associated with this component
//
//    A Sudoku puzzle (which only uses cells and regions) might be defined as:
//    Grid::Board<int, void, void>
//
//    A Sliterlink puzzle has binary states at edges and possibly cells (inside/outside):
//    Grid::Board<bool, bool, void>
//
//  Grid::Layout describes the layout of a grid, including its size and which cells
//  should be grouped together into a region.
//
//  Grid::StateSet is a helper templated class the holds a collection of states, or is
//  empty if given a template type of void.
//  

#include <vector>

#include "assert.h"
#include "BitVector.h"

namespace emp {
namespace Grid {

  class Layout {
  private:
    int width;
    int height;
    std::vector< std::vector<int> > regions;
  public:
    Layout(int w, int h) : width(w), height(h) { ; }
    Layout(const Layout &) = default;
    ~Layout() { ; }
    Layout & operator=(const Layout &) = default;

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    int GetNumRegions() const { return (int) regions.size(); }
    const std::vector<int> & GetRegion(int id) { return regions[id]; }

    void AddRegion(const std::vector<int> & in_region) { regions.push_back(in_region); }
  };

  template <typename STATE_TYPE>
  class StateSet {
  private:
    int width;
    std::vector<STATE_TYPE> states;

  public:
    StateSet(int _w, int _h) : width(_w), states(_w*_h) { ; }
    StateSet(const StateSet &) = default;
    ~StateSet() { ; }
    StateSet & operator=(const StateSet &) = default;

    int GetWidth() const { return width; }
    int GetHeight() const { return states.size() / width; }
    int GetSize() const { return states.size(); }

    STATE_TYPE operator()(int x, int y) { return states[y*width+x]; }
    STATE_TYPE operator()(int id) { return states[id]; }
    STATE_TYPE operator[](int id) { return states[id]; }
  };

  template <>
  class StateSet<bool> {
  private:
    int width;
    BitVector states;

  public:
    StateSet(int _w, int _h) : width(_w), states(_w*_h) { ; }
    StateSet(const StateSet &) = default;
    ~StateSet() { ; }
    StateSet & operator=(const StateSet &) = default;

    int GetWidth() const { return width; }
    int GetHeight() const { return states.size() / width; }
    int GetSize() const { return states.size(); }

    bool operator()(int x, int y) { return states[y*width+x]; }
    bool operator()(int id) { return states[id]; }
    bool operator[](int id) { return states[id]; }
  };

  // StateSet is specialized on void: no data is stored.
  template <>
  class StateSet<void> {
  public:
    StateSet(int _w, int _h) { (void) _w; (void) _h; }
    StateSet(const StateSet &) { ; }
    ~StateSet() { ; }
    StateSet & operator=(const StateSet &) { return *this; }

    int GetWidth() const { return -1; }
    int GetHeight() const { return -1; }
    int GetSize() const { return -1; }

    void operator()(int x, int y) { (void) x; (void) y; }
    void operator()(int id) { (void) id; }
    void operator[](int id) { (void) id; }
  };

  template <typename CELL_TYPE=int, typename EDGE_TYPE=void, class POINT_TYPE=void>
  class Board {
  private:
    const Layout & layout;
    StateSet<CELL_TYPE> cell_states;
    StateSet<EDGE_TYPE> edge_states;
    StateSet<POINT_TYPE> point_states;

  public:
    Board(const Layout & in_layout)
      : layout(in_layout)
      , cell_states(layout.GetWidth(), layout.GetHeight())
      , edge_states(layout.GetWidth(), layout.GetHeight())
      , point_states(layout.GetWidth(), layout.GetHeight())
    {
    }
  };

};
};


#endif
