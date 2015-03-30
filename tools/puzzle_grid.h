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
//  versions (eg: PuzzleGrid::CellNoState), and null version (eg: PuzzleGrid::CellNull).
//  The difference between CellNoState and CellNull is that the former allows you to navigate
//  relative to the cell, but the latter does not.
//
//  PuzzleGrid::Board object are templated based on which of the components they work with.
//
//  A Sudoku puzzle (which only uses cells and regions) might be defined as:
//    PuzzleGrid::Board<PuzzleGrid::Cell, PuzzleGrid::EdgeNull, PuzzleGrid::PointNULL>
//  
//  A Sliterlink puzzle has binary states at edges and possibly cells (inside/outside), and will
//  use relative positions at points, so:
//    PuzzleGrid::Board<PuzzleGrid::CellBinary, PuzzleGrid::EdgeBinary, PuzzleGrid::PointNoState>
//

namespace emp {
namespace PuzzleGrid {
  
};
};


#endif
