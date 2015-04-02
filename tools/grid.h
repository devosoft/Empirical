#ifndef EMP_PUZZLE_GRID_H
#define EMP_PUZZLE_GRID_H

///////////////////////////////////////////////////////////////////////////////////////////
//
//  Tools for building puzzle solvers on grid-based puzzles.
//
//  Puzzles may have several components to consider.
//    PuzzleGrid::Board  -- An entire, manipulable state for a puzzle instance
//    PuzzleGrid::Cell   -- A single element in a puzzle grid
//    PuzzleGrid::Edge   -- An edge between to cells
//    PuzzleGrid::Point  -- A corner of up to four grid cells
//    PuzzleGrid::Region -- A collection of an arbitrary number of related cells.
//
//  Grid components (Cells, Edges, and Points) have three versions.  The default versions have
//  int states, but there are also binary versions (eg, PuzzleGrid::CellBinary), non-state
//  versions (eg: PuzzleGrid::CellNoState), and null version (eg: PuzzleGrid::CellBase).
//  The difference between CellNoState and CellBase is that the former allows you to navigate
//  relative to the cell, but the latter does not.
//
//  PuzzleGrid::Board object are templated based on which of the components they work with.
//
//  A Sudoku puzzle (which only uses cells and regions) might be defined as:
//    PuzzleGrid::Board<PuzzleGrid::Cell, PuzzleGrid::EdgeBase, PuzzleGrid::PointNULL>
//  
//  A Sliterlink puzzle has binary states at edges and possibly cells (inside/outside), and will
//  use relative positions at points, so:
//    PuzzleGrid::Board<PuzzleGrid::CellBinary, PuzzleGrid::EdgeBinary, PuzzleGrid::PointNoState>
//

#include <vector>

#include "assert.h"

namespace emp {
namespace PuzzleGrid {

  // States for a Cell, Edge, or Point can be void, bool, or int.

  class Layout;    // No state, but tracks connections.
  class StateSet;  // A matrix of board states
  class Board;     // All states (for cells, edges, and points) on a board.

  class Layout {
  private:
    int width;
    int height;
    std::vector< std::vector<int> > regions;
  public:
    Layout(int w, int h) : width(w), height(h) { ; }
    Layout(const Layout &) = default;
    ~Layout() { ; }
    opterator=(const Layout &) = default;

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
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
    Board(const BoardLayout & in_layout)
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
