/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2024.
 *
 *  @file
 *  @brief  Analyzes a sudoku instance to determine the solving experience for a human player.
 *
 *  DEVELOPER NOTES:
 *   For the moment, we will assume that all boards are 9x9 with a standard Sudoku layout.
 *   In the future, we should make the board configuration more flexible.
*/

#ifndef EMP_GAMES_SUDOKU_ANALYZER_HPP
#define EMP_GAMES_SUDOKU_ANALYZER_HPP

#include <array>
#include <fstream>
#include <istream>
#include <set>
#include <vector>

#include "emp/base/array.hpp"
#include "emp/base/assert.hpp"
#include "emp/math/Random.hpp"
#include "emp/tools/String.hpp"

#include "Puzzle.hpp"

namespace emp {

  class SudokuAnalyzer {
  private:
    static constexpr size_t NUM_STATES = 9;
    static constexpr size_t NUM_ROWS = 9;
    static constexpr size_t NUM_COLS = 9;
    static constexpr size_t NUM_SQUARES = 9;
    static constexpr size_t NUM_OVERLAPS = 54;      // Multi-cell overlaps between two regions
    static constexpr size_t NUM_CELLS = NUM_ROWS * NUM_COLS;                  // 81
    static constexpr size_t NUM_REGIONS = NUM_ROWS + NUM_COLS + NUM_SQUARES;  // 27
    static constexpr size_t REGIONS_PER_CELL = 3;   // Each cell is part of three regions.
    
    static constexpr uint8_t UNKNOWN_STATE = NUM_STATES; // Lower values are actual states.

    // Which symbols are we using in this puzzle?
    emp::array<char, NUM_STATES> symbols = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};

    emp::array<uint8_t,NUM_CELLS> value;         // Known value for cells
    emp::array<uint32_t, NUM_CELLS> options;  // Options still available to each cell

    // "members" tracks which cell ids are members of each region.
    static constexpr size_t members[NUM_REGIONS][NUM_STATES] = {
      // Rows (Overlaps: 111000000, 000111000, 000000111)
      {  0,  1,  2,  3,  4,  5,  6,  7,  8 },  // Region 0  (Overlap with 18, 19, 20)
      {  9, 10, 11, 12, 13, 14, 15, 16, 17 },  // Region 1  (Overlap with 18, 19, 20)
      { 18, 19, 20, 21, 22, 23, 24, 25, 26 },  // Region 2  (Overlap with 18, 19, 20)
      { 27, 28, 29, 30, 31, 32, 33, 34, 35 },  // Region 3  (Overlap with 21, 22, 23)
      { 36, 37, 38, 39, 40, 41, 42, 43, 44 },  // Region 4  (Overlap with 21, 22, 23)
      { 45, 46, 47, 48, 49, 50, 51, 52, 53 },  // Region 5  (Overlap with 21, 22, 23)
      { 54, 55, 56, 57, 58, 59, 60, 61, 62 },  // Region 6  (Overlap with 24, 25, 26)
      { 63, 64, 65, 66, 67, 68, 69, 70, 71 },  // Region 7  (Overlap with 24, 25, 26)
      { 72, 73, 74, 75, 76, 77, 78, 79, 80 },  // Region 8  (Overlap with 24, 25, 26)

      // Columns (Overlaps: 111000000, 000111000, 000000111)
      { 0,  9, 18, 27, 36, 45, 54, 63, 72 },   // Region 9  (Overlap with 18, 21, 24)
      { 1, 10, 19, 28, 37, 46, 55, 64, 73 },   // Region 10 (Overlap with 18, 21, 24)
      { 2, 11, 20, 29, 38, 47, 56, 65, 74 },   // Region 11 (Overlap with 18, 21, 24)
      { 3, 12, 21, 30, 39, 48, 57, 66, 75 },   // Region 12 (Overlap with 19, 22, 25)
      { 4, 13, 22, 31, 40, 49, 58, 67, 76 },   // Region 13 (Overlap with 19, 22, 25)
      { 5, 14, 23, 32, 41, 50, 59, 68, 77 },   // Region 14 (Overlap with 19, 22, 25)
      { 6, 15, 24, 33, 42, 51, 60, 69, 78 },   // Region 15 (Overlap with 20, 23, 26)
      { 7, 16, 25, 34, 43, 52, 61, 70, 79 },   // Region 16 (Overlap with 20, 23, 26)
      { 8, 17, 26, 35, 44, 53, 62, 71, 80 },   // Region 17 (Overlap with 20, 23, 26)

      // Box Regions (Overlaps: 111000000, 000111000, 000000111, 100100100, 010010010, 001001001)
      {  0,  1,  2,  9, 10, 11, 18, 19, 20 },  // Region 18 (Overlap with 0, 1, 2,  9, 10, 11)
      {  3,  4,  5, 12, 13, 14, 21, 22, 23 },  // Region 19 (Overlap with 0, 1, 2, 12, 13, 14)
      {  6,  7,  8, 15, 16, 17, 24, 25, 26 },  // Region 20 (Overlap with 0, 1, 2, 15, 16, 17)
      { 27, 28, 29, 36, 37, 38, 45, 46, 47 },  // Region 21 (Overlap with 3, 4, 5,  9, 10, 11)
      { 30, 31, 32, 39, 40, 41, 48, 49, 50 },  // Region 22 (Overlap with 3, 4, 5, 12, 13, 14)
      { 33, 34, 35, 42, 43, 44, 51, 52, 53 },  // Region 23 (Overlap with 3, 4, 5, 15, 16, 17)
      { 54, 55, 56, 63, 64, 65, 72, 73, 74 },  // Region 24 (Overlap with 6, 7, 8,  9, 10, 11)
      { 57, 58, 59, 66, 67, 68, 75, 76, 77 },  // Region 25 (Overlap with 6, 7, 8, 12, 13, 14)
      { 60, 61, 62, 69, 70, 71, 78, 79, 80 }   // Region 26 (Overlap with 6, 7, 8, 15, 16, 17)
    };
    
    // "regions" tracks which regions each cell is a member of.
    static constexpr size_t regions[NUM_CELLS][REGIONS_PER_CELL] = {
      // { ROW, COLUMN, BOX }
      { 0,  9, 18 }, { 0, 10, 18 }, { 0, 11, 18 },  // Cells  0- 2
      { 0, 12, 19 }, { 0, 13, 19 }, { 0, 14, 19 },  // Cells  3- 5
      { 0, 15, 20 }, { 0, 16, 20 }, { 0, 17, 20 },  // Cells  6- 8
      { 1,  9, 18 }, { 1, 10, 18 }, { 1, 11, 18 },  // Cells  9-11
      { 1, 12, 19 }, { 1, 13, 19 }, { 1, 14, 19 },  // Cells 12-14
      { 1, 15, 20 }, { 1, 16, 20 }, { 1, 17, 20 },  // Cells 15-17
      { 2,  9, 18 }, { 2, 10, 18 }, { 2, 11, 18 },  // Cells 18-20
      { 2, 12, 19 }, { 2, 13, 19 }, { 2, 14, 19 },  // Cells 21-23
      { 2, 15, 20 }, { 2, 16, 20 }, { 2, 17, 20 },  // Cells 24-26
      
      { 3,  9, 21 }, { 3, 10, 21 }, { 3, 11, 21 },  // Cells 27-29
      { 3, 12, 22 }, { 3, 13, 22 }, { 3, 14, 22 },  // Cells 30-32
      { 3, 15, 23 }, { 3, 16, 23 }, { 3, 17, 23 },  // Cells 33-35
      { 4,  9, 21 }, { 4, 10, 21 }, { 4, 11, 21 },  // Cells 36-38
      { 4, 12, 22 }, { 4, 13, 22 }, { 4, 14, 22 },  // Cells 39-41
      { 4, 15, 23 }, { 4, 16, 23 }, { 4, 17, 23 },  // Cells 42-44
      { 5,  9, 21 }, { 5, 10, 21 }, { 5, 11, 21 },  // Cells 45-47
      { 5, 12, 22 }, { 5, 13, 22 }, { 5, 14, 22 },  // Cells 48-50
      { 5, 15, 23 }, { 5, 16, 23 }, { 5, 17, 23 },  // Cells 51-53
      
      { 6,  9, 24 }, { 6, 10, 24 }, { 6, 11, 24 },  // Cells 54-56
      { 6, 12, 25 }, { 6, 13, 25 }, { 6, 14, 25 },  // Cells 57-59
      { 6, 15, 26 }, { 6, 16, 26 }, { 6, 17, 26 },  // Cells 60-62
      { 7,  9, 24 }, { 7, 10, 24 }, { 7, 11, 24 },  // Cells 63-65
      { 7, 12, 25 }, { 7, 13, 25 }, { 7, 14, 25 },  // Cells 66-68
      { 7, 15, 26 }, { 7, 16, 26 }, { 7, 17, 26 },  // Cells 69-71
      { 8,  9, 24 }, { 8, 10, 24 }, { 8, 11, 24 },  // Cells 72-74
      { 8, 12, 25 }, { 8, 13, 25 }, { 8, 14, 25 },  // Cells 75-77
      { 8, 15, 26 }, { 8, 16, 26 }, { 8, 17, 26 }   // Cells 78-80
    };

    // Which *other* cells is each cell linked to by at least one region?
    static constexpr size_t links[NUM_CELLS][20] = {
      { 1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 18, 19, 20, 27, 36, 45, 54, 63, 72 }, // Cell 0
      { 0,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 18, 19, 20, 28, 37, 46, 55, 64, 73 }, // Cell 1
      { 0,  1,  3,  4,  5,  6,  7,  8,  9, 10, 11, 18, 19, 20, 29, 38, 47, 56, 65, 74 },
      { 0,  1,  2,  4,  5,  6,  7,  8, 12, 13, 14, 21, 22, 23, 30, 39, 48, 57, 66, 75 },
      { 0,  1,  2,  3,  5,  6,  7,  8, 12, 13, 14, 21, 22, 23, 31, 40, 49, 58, 67, 76 },
      { 0,  1,  2,  3,  4,  6,  7,  8, 12, 13, 14, 21, 22, 23, 32, 41, 50, 59, 68, 77 },
      { 0,  1,  2,  3,  4,  5,  7,  8, 15, 16, 17, 24, 25, 26, 33, 42, 51, 60, 69, 78 },
      { 0,  1,  2,  3,  4,  5,  6,  8, 15, 16, 17, 24, 25, 26, 34, 43, 52, 61, 70, 79 },
      { 0,  1,  2,  3,  4,  5,  6,  7, 15, 16, 17, 24, 25, 26, 35, 44, 53, 62, 71, 80 },
      { 0,  1,  2, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 27, 36, 45, 54, 63, 72 },
      { 0,  1,  2,  9, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 28, 37, 46, 55, 64, 73 },
      { 0,  1,  2,  9, 10, 12, 13, 14, 15, 16, 17, 18, 19, 20, 29, 38, 47, 56, 65, 74 },
      { 3,  4,  5,  9, 10, 11, 13, 14, 15, 16, 17, 21, 22, 23, 30, 39, 48, 57, 66, 75 },
      { 3,  4,  5,  9, 10, 11, 12, 14, 15, 16, 17, 21, 22, 23, 31, 40, 49, 58, 67, 76 },
      { 3,  4,  5,  9, 10, 11, 12, 13, 15, 16, 17, 21, 22, 23, 32, 41, 50, 59, 68, 77 },
      { 6,  7,  8,  9, 10, 11, 12, 13, 14, 16, 17, 24, 25, 26, 33, 42, 51, 60, 69, 78 },
      { 6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 17, 24, 25, 26, 34, 43, 52, 61, 70, 79 },
      { 6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 24, 25, 26, 35, 44, 53, 62, 71, 80 },
      { 0,  1,  2,  9, 10, 11, 19, 20, 21, 22, 23, 24, 25, 26, 27, 36, 45, 54, 63, 72 },
      { 0,  1,  2,  9, 10, 11, 18, 20, 21, 22, 23, 24, 25, 26, 28, 37, 46, 55, 64, 73 },
      { 0,  1,  2,  9, 10, 11, 18, 19, 21, 22, 23, 24, 25, 26, 29, 38, 47, 56, 65, 74 },
      { 3,  4,  5, 12, 13, 14, 18, 19, 20, 22, 23, 24, 25, 26, 30, 39, 48, 57, 66, 75 },
      { 3,  4,  5, 12, 13, 14, 18, 19, 20, 21, 23, 24, 25, 26, 31, 40, 49, 58, 67, 76 },
      { 3,  4,  5, 12, 13, 14, 18, 19, 20, 21, 22, 24, 25, 26, 32, 41, 50, 59, 68, 77 },
      { 6,  7,  8, 15, 16, 17, 18, 19, 20, 21, 22, 23, 25, 26, 33, 42, 51, 60, 69, 78 },
      { 6,  7,  8, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 26, 34, 43, 52, 61, 70, 79 },
      { 6,  7,  8, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 35, 44, 53, 62, 71, 80 },
      { 0,  9, 18, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 45, 46, 47, 54, 63, 72 },
      { 1, 10, 19, 27, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 45, 46, 47, 55, 64, 73 },
      { 2, 11, 20, 27, 28, 30, 31, 32, 33, 34, 35, 36, 37, 38, 45, 46, 47, 56, 65, 74 },
      { 3, 12, 21, 27, 28, 29, 31, 32, 33, 34, 35, 39, 40, 41, 48, 49, 50, 57, 66, 75 },
      { 4, 13, 22, 27, 28, 29, 30, 32, 33, 34, 35, 39, 40, 41, 48, 49, 50, 58, 67, 76 },
      { 5, 14, 23, 27, 28, 29, 30, 31, 33, 34, 35, 39, 40, 41, 48, 49, 50, 59, 68, 77 },
      { 6, 15, 24, 27, 28, 29, 30, 31, 32, 34, 35, 42, 43, 44, 51, 52, 53, 60, 69, 78 },
      { 7, 16, 25, 27, 28, 29, 30, 31, 32, 33, 35, 42, 43, 44, 51, 52, 53, 61, 70, 79 },
      { 8, 17, 26, 27, 28, 29, 30, 31, 32, 33, 34, 42, 43, 44, 51, 52, 53, 62, 71, 80 },
      { 0,  9, 18, 27, 28, 29, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 54, 63, 72 },
      { 1, 10, 19, 27, 28, 29, 36, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 55, 64, 73 },
      { 2, 11, 20, 27, 28, 29, 36, 37, 39, 40, 41, 42, 43, 44, 45, 46, 47, 56, 65, 74 },
      { 3, 12, 21, 30, 31, 32, 36, 37, 38, 40, 41, 42, 43, 44, 48, 49, 50, 57, 66, 75 },
      { 4, 13, 22, 30, 31, 32, 36, 37, 38, 39, 41, 42, 43, 44, 48, 49, 50, 58, 67, 76 },
      { 5, 14, 23, 30, 31, 32, 36, 37, 38, 39, 40, 42, 43, 44, 48, 49, 50, 59, 68, 77 },
      { 6, 15, 24, 33, 34, 35, 36, 37, 38, 39, 40, 41, 43, 44, 51, 52, 53, 60, 69, 78 },
      { 7, 16, 25, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 44, 51, 52, 53, 61, 70, 79 },
      { 8, 17, 26, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 51, 52, 53, 62, 71, 80 },
      { 0,  9, 18, 27, 28, 29, 36, 37, 38, 46, 47, 48, 49, 50, 51, 52, 53, 54, 63, 72 },
      { 1, 10, 19, 27, 28, 29, 36, 37, 38, 45, 47, 48, 49, 50, 51, 52, 53, 55, 64, 73 },
      { 2, 11, 20, 27, 28, 29, 36, 37, 38, 45, 46, 48, 49, 50, 51, 52, 53, 56, 65, 74 },
      { 3, 12, 21, 30, 31, 32, 39, 40, 41, 45, 46, 47, 49, 50, 51, 52, 53, 57, 66, 75 },
      { 4, 13, 22, 30, 31, 32, 39, 40, 41, 45, 46, 47, 48, 50, 51, 52, 53, 58, 67, 76 },
      { 5, 14, 23, 30, 31, 32, 39, 40, 41, 45, 46, 47, 48, 49, 51, 52, 53, 59, 68, 77 },
      { 6, 15, 24, 33, 34, 35, 42, 43, 44, 45, 46, 47, 48, 49, 50, 52, 53, 60, 69, 78 },
      { 7, 16, 25, 33, 34, 35, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 53, 61, 70, 79 },
      { 8, 17, 26, 33, 34, 35, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 62, 71, 80 },
      { 0,  9, 18, 27, 36, 45, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 72, 73, 74 },
      { 1, 10, 19, 28, 37, 46, 54, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 72, 73, 74 },
      { 2, 11, 20, 29, 38, 47, 54, 55, 57, 58, 59, 60, 61, 62, 63, 64, 65, 72, 73, 74 },
      { 3, 12, 21, 30, 39, 48, 54, 55, 56, 58, 59, 60, 61, 62, 66, 67, 68, 75, 76, 77 },
      { 4, 13, 22, 31, 40, 49, 54, 55, 56, 57, 59, 60, 61, 62, 66, 67, 68, 75, 76, 77 },
      { 5, 14, 23, 32, 41, 50, 54, 55, 56, 57, 58, 60, 61, 62, 66, 67, 68, 75, 76, 77 },
      { 6, 15, 24, 33, 42, 51, 54, 55, 56, 57, 58, 59, 61, 62, 69, 70, 71, 78, 79, 80 },
      { 7, 16, 25, 34, 43, 52, 54, 55, 56, 57, 58, 59, 60, 62, 69, 70, 71, 78, 79, 80 },
      { 8, 17, 26, 35, 44, 53, 54, 55, 56, 57, 58, 59, 60, 61, 69, 70, 71, 78, 79, 80 },
      { 0,  9, 18, 27, 36, 45, 54, 55, 56, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74 },
      { 1, 10, 19, 28, 37, 46, 54, 55, 56, 63, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74 },
      { 2, 11, 20, 29, 38, 47, 54, 55, 56, 63, 64, 66, 67, 68, 69, 70, 71, 72, 73, 74 },
      { 3, 12, 21, 30, 39, 48, 57, 58, 59, 63, 64, 65, 67, 68, 69, 70, 71, 75, 76, 77 },
      { 4, 13, 22, 31, 40, 49, 57, 58, 59, 63, 64, 65, 66, 68, 69, 70, 71, 75, 76, 77 },
      { 5, 14, 23, 32, 41, 50, 57, 58, 59, 63, 64, 65, 66, 67, 69, 70, 71, 75, 76, 77 },
      { 6, 15, 24, 33, 42, 51, 60, 61, 62, 63, 64, 65, 66, 67, 68, 70, 71, 78, 79, 80 },
      { 7, 16, 25, 34, 43, 52, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 71, 78, 79, 80 },
      { 8, 17, 26, 35, 44, 53, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 78, 79, 80 },
      { 0,  9, 18, 27, 36, 45, 54, 55, 56, 63, 64, 65, 73, 74, 75, 76, 77, 78, 79, 80 },
      { 1, 10, 19, 28, 37, 46, 54, 55, 56, 63, 64, 65, 72, 74, 75, 76, 77, 78, 79, 80 },
      { 2, 11, 20, 29, 38, 47, 54, 55, 56, 63, 64, 65, 72, 73, 75, 76, 77, 78, 79, 80 },
      { 3, 12, 21, 30, 39, 48, 57, 58, 59, 66, 67, 68, 72, 73, 74, 76, 77, 78, 79, 80 },
      { 4, 13, 22, 31, 40, 49, 57, 58, 59, 66, 67, 68, 72, 73, 74, 75, 77, 78, 79, 80 },
      { 5, 14, 23, 32, 41, 50, 57, 58, 59, 66, 67, 68, 72, 73, 74, 75, 76, 78, 79, 80 },
      { 6, 15, 24, 33, 42, 51, 60, 61, 62, 69, 70, 71, 72, 73, 74, 75, 76, 77, 79, 80 },
      { 7, 16, 25, 34, 43, 52, 60, 61, 62, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 80 },
      { 8, 17, 26, 35, 44, 53, 60, 61, 62, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79 }
    };
    
    // Given a binary representation of options, which bit position is the first available?
    static constexpr uint8_t next_opt[512] = {
      UNKNOWN_STATE, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      8, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    };

    // Given a binary representation of state options for a cell, how many are available?
    static constexpr size_t opts_count[512] = {
      0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
      1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
      1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
      1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
      3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
      1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
      3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
      3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
      3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
      4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
      1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
      3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
      3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
      3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
      4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
      3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
      3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
      4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
      3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
      4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
      4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
      5, 6, 6, 7, 6, 7, 7, 8, 6, 7, 7, 8, 7, 8, 8, 9
    };

    // Which cells are in region overlaps (horizontal or vertical)?
    static constexpr size_t overlaps[NUM_OVERLAPS][3] = {
      {  0,  1,  2 }, {  3,  4,  5 }, {  6,  7,  8 },  // Overlap sets  0,  1,  2 (Row 0)
      {  9, 10, 11 }, { 12, 13, 14 }, { 15, 16, 17 },  // Overlap sets  3,  4,  5 (Row 1)
      { 18, 19, 20 }, { 21, 22, 23 }, { 24, 25, 26 },  // Overlap sets  6,  7,  8 (Row 2)
      { 27, 28, 29 }, { 30, 31, 32 }, { 33, 34, 35 },  // Overlap sets  9, 10, 11 (Row 3)
      { 36, 37, 38 }, { 39, 40, 41 }, { 42, 43, 44 },  // Overlap sets 12, 13, 14 (Row 4)
      { 45, 46, 47 }, { 48, 49, 50 }, { 51, 52, 53 },  // Overlap sets 15, 16, 17 (Row 5)
      { 54, 55, 56 }, { 57, 58, 59 }, { 60, 61, 62 },  // Overlap sets 18, 19, 20 (Row 6)
      { 63, 64, 65 }, { 66, 67, 68 }, { 69, 70, 71 },  // Overlap sets 21, 22, 23 (Row 7)
      { 72, 73, 74 }, { 75, 76, 77 }, { 78, 79, 80 },  // Overlap sets 24, 25, 26 (Row 8)

      {  0,  9, 18 }, { 27, 36, 45 }, { 54, 63, 72 },  // Overlap sets 27, 28, 29 (Col 0)
      {  1, 10, 19 }, { 28, 37, 46 }, { 55, 64, 73 },  // Overlap sets 30, 31, 32 (Col 1)
      {  2, 11, 20 }, { 29, 38, 47 }, { 56, 65, 74 },  // Overlap sets 33, 34, 35 (Col 2)
      {  3, 12, 21 }, { 30, 39, 48 }, { 57, 66, 75 },  // Overlap sets 36, 37, 38 (Col 3)
      {  4, 13, 22 }, { 31, 40, 49 }, { 58, 67, 76 },  // Overlap sets 39, 40, 41 (Col 4)
      {  5, 14, 23 }, { 32, 41, 50 }, { 59, 68, 77 },  // Overlap sets 42, 43, 44 (Col 5)
      {  6, 15, 24 }, { 33, 42, 51 }, { 60, 69, 78 },  // Overlap sets 45, 46, 47 (Col 6)
      {  7, 16, 25 }, { 34, 43, 52 }, { 61, 70, 79 },  // Overlap sets 48, 49, 50 (Col 7)
      {  8, 17, 26 }, { 35, 44, 53 }, { 62, 71, 80 },  // Overlap sets 51, 52, 53 (Col 8)
    };

    // Which overlaps are square regions made up from? (each square is listed twice).
    static constexpr size_t square_overlaps[18][3] = {
      {  0,  3,  6 }, {  1,  4,  7 }, {  2,  5,  8 },
      {  9, 12, 15 }, { 10, 13, 16 }, { 11, 14, 17 },
      { 18, 21, 24 }, { 19, 22, 25 }, { 20, 23, 26 },

      { 27, 30, 33 }, { 28, 31, 34 }, { 29, 32, 35 },
      { 36, 39, 42 }, { 37, 40, 43 }, { 38, 41, 44 },
      { 45, 48, 51 }, { 46, 49, 52 }, { 47, 50, 53 }
    };

    // Which region is each overlap associated with? (raw row/col first, then square id)
    static constexpr size_t overlap_regions[NUM_OVERLAPS][2] = {
      {  0, 0 }, {  0, 1 }, {  0, 2 },
      {  1, 0 }, {  1, 1 }, {  1, 2 },
      {  2, 0 }, {  2, 1 }, {  2, 2 },
      {  3, 3 }, {  3, 4 }, {  3, 5 },
      {  4, 3 }, {  4, 4 }, {  4, 5 },
      {  5, 3 }, {  5, 4 }, {  5, 5 },
      {  6, 6 }, {  6, 7 }, {  6, 8 },
      {  7, 6 }, {  7, 7 }, {  7, 8 },
      {  8, 6 }, {  8, 7 }, {  8, 8 },

      {  9, 9 }, {  9, 10 }, {  9, 11 },
      { 10, 9 }, { 10, 10 }, { 10, 11 },
      { 11, 9 }, { 11, 10 }, { 11, 11 },
      { 12, 12 }, { 12, 13 }, { 12, 14 },
      { 13, 12 }, { 13, 13 }, { 13, 14 },
      { 14, 12 }, { 14, 13 }, { 14, 14 },
      { 15, 15 }, { 15, 16 }, { 15, 17 },
      { 16, 15 }, { 16, 16 }, { 16, 17 },
      { 17, 15 }, { 17, 16 }, { 17, 17 }
    };

    // static constexpr size_t overlap_regions[NUM_OVERLAPS][2] = {
    //   {  0, 18 }, {  0, 19 }, {  0, 20 },
    //   {  1, 18 }, {  1, 19 }, {  1, 20 },
    //   {  2, 18 }, {  2, 19 }, {  2, 20 },
    //   {  3, 21 }, {  3, 22 }, {  3, 23 },
    //   {  4, 21 }, {  4, 22 }, {  4, 23 },
    //   {  5, 21 }, {  5, 22 }, {  5, 23 },
    //   {  6, 24 }, {  6, 25 }, {  6, 26 },
    //   {  7, 24 }, {  7, 25 }, {  7, 26 },
    //   {  8, 24 }, {  8, 25 }, {  8, 26 },

    //   {  9, 18 }, {  9, 21 }, {  9, 24 },
    //   { 10, 18 }, { 10, 21 }, { 10, 24 },
    //   { 11, 18 }, { 11, 21 }, { 11, 24 },
    //   { 12, 19 }, { 12, 22 }, { 12, 25 },
    //   { 13, 19 }, { 13, 22 }, { 13, 25 },
    //   { 14, 19 }, { 14, 22 }, { 14, 25 },
    //   { 15, 20 }, { 15, 23 }, { 15, 26 },
    //   { 16, 20 }, { 16, 23 }, { 16, 26 },
    //   { 17, 20 }, { 17, 23 }, { 17, 26 }
    // };
    
  public:
    SudokuAnalyzer() { Clear(); }
    SudokuAnalyzer(const SudokuAnalyzer &) = default;
    ~SudokuAnalyzer() { ; }

    SudokuAnalyzer& operator=(const SudokuAnalyzer &) = default;

    uint8_t GetValue(size_t cell) const { return value[cell]; }
    uint32_t GetOptions(size_t cell) const { return options[cell]; }
    size_t SymbolToState(char symbol) const {
      for (size_t i=0; i < NUM_STATES; ++i) {
        if (symbols[i] == symbol) return i;
      }
      return UNKNOWN_STATE;
    }
    size_t CountOptions(size_t cell) const {
      // if (cell < 0 || cell >= 81) std::cout << "cell=" << cell << std::endl;
      emp_assert(cell >= 0 && cell < 81, cell);
      return opts_count[options[cell]];
    }
    bool HasOption(size_t cell, size_t state) {
      emp_assert(cell < 81, cell);
      emp_assert(state >= 0 && state < 9, state);
      return options[cell] & (1 << state);
    }
    bool IsSet(size_t cell) const { return value[cell] != UNKNOWN_STATE; }
    bool IsSolved() {
      for (uint32_t o : options) if (o) return false;
      return true;
    }
    
    // Clear out the old solution info when starting a new solve attempt.
    void Clear() {
      value.fill(UNKNOWN_STATE);
      options.fill(511);  // Set all options to one.  or 0b111111111
    }

    // Load a state from a stream.
    void Load(std::istream & is) {
      // Format: Provide site by site with a dash for empty; whitespace is ignored.
      char cur_char;
      size_t cell_id = 0;
      while (cell_id < NUM_CELLS) {
        is >> cur_char;
        if (emp::is_whitespace(cur_char)) continue;
        ++cell_id;
        if (cur_char == '-') continue;
        size_t state_id = SymbolToState(cur_char);
        if (state_id == UNKNOWN_STATE) {
          emp::notify::Warning("Unknown sudoku symbol '", cur_char, "'.  Ignoring.");
          continue;
        }
        Set(cell_id-1, state_id);
      }
    }

    // Load a state from a file.
    void Load(const emp::String & filename) {
      std::ifstream file(filename);
      return Load(file);
    }

    // Find the next available option for a cell.
    uint8_t FindNext(size_t cell) { return next_opt[options[cell]]; }

    // Set the value of an individual cell; remove option from linked cells.
    // Return true/false based on whether progress was made toward solving the puzzle.
    bool Set(size_t cell, uint8_t state) {
      emp_assert(cell < NUM_CELLS);    // Make sure cell is in a valid range.
      emp_assert(state >= 0 && state < NUM_STATES); // Make sure state is in a valid range.

      if (value[cell] == state) return false;       // If state is already set, SKIP!

      emp_assert(HasOption(cell,state));     // Make sure state is allowed.
      value[cell] = state;                   // Store found value!
      options[cell] = 0;                     // No options available to locked cells.
      
      // Now make sure this state is blocked from all linked cells.
      for (size_t id : links[cell]) Block(id, state);

      return true;
    }
    
    // Remove a symbol option from a particular cell.
    void Block(size_t cell, size_t state) {
      options[cell] &= ~(1 << state);
    }

    // Operate on a "move" object.
    void Move(const PuzzleMove & move) {
      emp_assert(move.pos_id >= 0 && move.pos_id < NUM_CELLS, move.pos_id);
      emp_assert(move.state >= 0 && move.state < NUM_STATES, move.state);
      
      switch (move.type) {
      case PuzzleMove::SET_STATE:   Set(move.pos_id, move.state);   break;
      case PuzzleMove::BLOCK_STATE: Block(move.pos_id, move.state); break;
      default:
        emp_assert(false);   // One of the previous move options should have been triggered!
      }
    }
    
    // Operate on a set of "move" objects.
    void Move(const emp::vector<PuzzleMove> & moves) {
      for (const auto & move : moves) { Move(move); }
    }

    // Print the current state of the puzzle, including all options available.
    void Print(std::ostream & out=std::cout) {
      out << " +-----------------------+-----------------------+-----------------------+"
          << std::endl;;
      for (size_t r = 0; r < 9; r++) {       // Puzzle row
        for (size_t s = 0; s < 9; s+=3) {    // Subset row
          for (size_t c = 0; c < 9; c++) {   // Puzzle col
            size_t id = r*9+c;
            if (c%3==0) out << " |";
            else out << "  ";
            if (value[id] == UNKNOWN_STATE) {
              out << " " << (char) (HasOption(id,s)   ? symbols[s] : '.')
                  << " " << (char) (HasOption(id,s+1) ? symbols[s+1] : '.')
                  << " " << (char) (HasOption(id,s+2) ? symbols[s+2] : '.');
            } else {
              if (s==0) out << "      ";
              if (s==3) out << "   " << symbols[value[id]] << "  ";
              if (s==6) out << "      ";
              // if (s==0) out << " /   \\";
              // if (s==3) out << " | " << symbols[value[id]] << " |";
              // if (s==6) out << " \\   /";
            }
          }
          out << " |" << std::endl;
        }
        if (r%3==2) {
          out << " +-----------------------+-----------------------+-----------------------+";
        }
        else {
          out << " |                       |                       |                       |";
        }
        out << std::endl;
      }      
    }

    // Use a brute-force approach to completely solve this puzzle.
    // Return true if solved, false if unsolvable.
    bool ForceSolve(size_t start=0) {
      emp_assert(start >= 0 && start <= NUM_CELLS);
      
      // Advance the start position until we find a cell with a choice to be made.
      while (start < NUM_CELLS) {
        const size_t opt_count = CountOptions(start);
        if (opt_count == 0 && !IsSet(start)) return false;      // No option & unlocked -> backtrack!
        else if (opt_count == 1) Set( start, FindNext(start) ); // One option -> lock it!
        else if (opt_count > 1) break;                          // Multiple options -> move on!
      
        start++;   // Must have locked option, increment and keep looping!
      }

      // If we've made it through all positions stop here.
      if (start == 81) return true;
      
      // Step through possibilities of first cell with multiple options.
      for (size_t i = 0; i < NUM_STATES; i++) {
        if (HasOption(start,i) == false) continue;  // Skip values that are not an option.
        
        SudokuAnalyzer backup_state(*this);    // backup the current state.
        Set(start, i);                      // set this cell to next possible value.
        bool solved = ForceSolve(start+1);  // continue attempt to solve!
        if (solved) return true;            // if solved, we're done!
        *this = backup_state;               // otherwise, restore from backup and loop.
      }

      // If we made it this far, we were unable to find a solution.
      return false;      
    }

    // More human-focused solving techniques:

    // If there's only one state a cell can be, pick it!
    std::vector<PuzzleMove> Solve_FindLastCellState() {
      std::vector<PuzzleMove> moves;

      // For each cell, check if it has only one state left.
      for (size_t i = 0; i < NUM_CELLS; i++) {
        if (CountOptions(i) == 1) {
          // Find last value.
          moves.emplace_back(PuzzleMove{PuzzleMove::SET_STATE, i, FindNext(i)});
        }
      }

      return moves;
    }

    // If there's only one cell that can have a certain state in a region, choose it!
    std::vector<PuzzleMove> Solve_FindLastRegionState() {
      std::vector<PuzzleMove> moves;

      // For each region, check if it has any states with only one available cell.
      for (const auto & region : members) {
        uint32_t opt_any = 0;     // Is a state an option in ANY cell?
        uint32_t opt_multi = 0;   // Is a state an option in MULTIPLE cells?
        for (const size_t c : region) {
          opt_multi |= (options[c] & opt_any);  // If we already had an option AND see a new one.
          opt_any |= options[c];                // Mark these options as possible.
        }
        const uint32_t opt_once = opt_any & ~opt_multi;

        // If any options are only available in one cell, find them and lock them in.
        if (opt_once) {
          for (const size_t c : region) {
            const uint32_t opt_unique = options[c] & opt_once;
            if (opt_unique) {
              moves.emplace_back(PuzzleMove{PuzzleMove::SET_STATE, c, next_opt[opt_unique]});
            }
          }
        }
      }
      
      return moves;
    }

    // If only cells that can have a state in region A are all also in region
    // B, no other cell in region B can have that state as a possibility.
    std::vector<PuzzleMove> Solve_FindRegionOverlap() {
      std::vector<PuzzleMove> moves;

      // Determine what options are available in each overlap region.
      std::array<uint32_t, NUM_OVERLAPS> overlap_options;
      for (size_t i = 0; i < 54; i++) {
        overlap_options[i] =
          options[overlaps[i][0]] | options[overlaps[i][1]] | options[overlaps[i][2]];
      }

      // If an option is available in only one overlap, than it must be there
      // (and cannot be elsewhere in the OTHER region that shares that overlap.)

      // Start with row/col overlaps, which are in groups of three.
      for (size_t i = 0; i < NUM_OVERLAPS; i += 3) {
        uint32_t single_opts =
          (overlap_options[i] ^ overlap_options[i+1] ^ overlap_options[i+2]) &
          ~(overlap_options[i] & overlap_options[i+1] & overlap_options[i+2]);

        if (!single_opts) continue;

        // If we made it this far, there is a move. Find the SQUARE region for this overlap.
        const size_t square_id = overlap_regions[i][1];
        for (size_t oid : square_overlaps[square_id]) {
          if (oid == i) continue;
          uint32_t extra_opts = single_opts & overlap_options[oid];

          // We found options to block!  Lets step through all of the cells and options.
          while (extra_opts) {
            const uint8_t opt_id = next_opt[extra_opts];  // Determine this option.
            extra_opts &= ~(1 << opt_id);             // Remove this option for future checks.
            for (size_t cell_id : overlaps[oid]) {
              if (HasOption(cell_id, opt_id)) {
                moves.emplace_back(PuzzleMove{PuzzleMove::BLOCK_STATE, cell_id, opt_id});
              }
            }
          }
          
        }
      }  
      
      return moves;
    }
    
    // If K cells are all limited to the same K states, eliminate those states
    // from all other cells in the same region.
    std::vector<PuzzleMove> Solve_FindLimitedCells() {
      std::vector<PuzzleMove> moves;
      return moves;
    }
    
    // Eliminate all other possibilities from K cells if they are the only
    // ones that can possess K states in a single region.
    std::vector<PuzzleMove> Solve_FindLimitedStates() {
      std::vector<PuzzleMove> moves;
      return moves;
    }

    // If there are X rows (cols) where a certain state can only be in one of 
    // X cols (rows), then no other row in this cols can be that state.
    std::vector<PuzzleMove> Solve_FindSwordfish() {
      std::vector<PuzzleMove> moves;
      return moves;
    }

    // Calculate the full solving profile based on the other techniques.
    PuzzleProfile CalcProfile()
    {
      PuzzleProfile profile;

      while (true) {    
        auto moves = Solve_FindLastCellState();
        if (moves.size() > 0) {
          Move(moves);
          profile.AddMoves(0, moves.size());
          continue;
        }
        
        moves = Solve_FindLastRegionState();
        if (moves.size() > 0) {
          Move(moves);
          profile.AddMoves(1, moves.size());
          continue;
        }
        
        break;  // No new moves found!
      }

      return profile;
    }

    // Make sure the current state is consistant.
    bool OK() {
      // Run tests on each cell...
      for (size_t cell = 0; cell < 81; cell++) {
        // Make sure any set values are the only allowed option.
        if (value[cell] != UNKNOWN_STATE) {
          emp_assert(options[cell] == 0);
        }
        
        // Make sure that the opt_counts are equal to the number of options available.
        [[maybe_unused]] int count = 0;
        for (int i = 0; i < 9; i++) {
          if (HasOption(cell,i)) count++;
        }
        emp_assert(CountOptions(cell) == count);
      }

      return true;
    }
  };

  
} // END emp namespace

#endif
