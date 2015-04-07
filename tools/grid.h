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

    // Helper functions
    int GetX(int id) const { return id % width; }
    int GetY(int id) const { return id / width; }
    int GetID(int x, int y) const { return y*width + x; }

    int GetTopID(int id) const { return id; }                              // ID of edge at top
    int GetBottomID(int id) const { return id + width; }                   // ID of edge at bottom
    int GetLeftID(int id) const { return GetY(id)*(width+1) + GetX(id); }  // ID of edge at left
    int GetRightID(int id) const { return GetLeftID(id)+1; }               // ID of edge at right
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
    StateSet<CELL_TYPE> cell_states;    // States of cells in grid.
    StateSet<EDGE_TYPE> edge_states_h;  // States of horizontal edges.
    StateSet<EDGE_TYPE> edge_states_v;  // States of vertical edges.
    StateSet<POINT_TYPE> point_states;  // States of points (where edges cross)

  public:
    Board(const Layout & in_layout)
      : layout(in_layout)
      , cell_states(layout.GetWidth(), layout.GetHeight())
      , edge_states_h(layout.GetWidth(), layout.GetHeight()+1)
      , edge_states_v(layout.GetWidth()+1, layout.GetHeight())
      , point_states(layout.GetWidth()+1, layout.GetHeight()+1)
    {
      // std::cout << "Built Board with " << cell_states.GetSize() << " cells!" << std::endl;
      // std::cout << "Built Board with " << edge_states_h.GetSize() << " h edges!" << std::endl;
      // std::cout << "Built Board with " << edge_states_v.GetSize() << " v edges!" << std::endl;
      // std::cout << "Built Board with " << point_states.GetSize() << " points!" << std::endl
      //           << std::endl;
    }

    const Layout & GetLayout() const { return layout; }

    CELL_TYPE GetCell(int id) const { return cell_states[id]; }
    EDGE_TYPE GetEdgeH(int id) const { return edge_states_h[id]; }
    EDGE_TYPE GetEdgeV(int id) const { return edge_states_v[id]; }
    POINT_TYPE GetPoint(int id) const { return point_states[id]; }
  };

};
};


#endif
