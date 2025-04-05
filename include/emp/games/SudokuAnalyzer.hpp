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
#include "emp/bits/BitSet.hpp"
#include "emp/math/ComboSet.hpp"
#include "emp/math/Random.hpp"
#include "emp/tools/String.hpp"

#include "Puzzle.hpp"

namespace emp {

  class SudokuAnalyzer : public GridPuzzleAnalyzer<9, 9, 9> {
  private:
    static constexpr size_t NUM_STATES = 9;
    static constexpr size_t NUM_ROWS = 9;
    static constexpr size_t NUM_COLS = 9;
    static constexpr size_t NUM_SQUARES = 9;

    static constexpr size_t NUM_CELLS = NUM_ROWS * NUM_COLS;                  // 81
    static constexpr size_t NUM_REGIONS = NUM_ROWS + NUM_COLS + NUM_SQUARES;  // 27
    static constexpr size_t REGIONS_PER_CELL = 3;   // Each cell is part of three regions.
    static constexpr size_t NO_REGION = static_cast<size_t>(-1);

    // Calculate multi-cell overlaps between regions; each row/col overlaps with 3 square regions.
    static constexpr size_t NUM_OVERLAPS = (NUM_ROWS + NUM_COLS) * 3 ;        // 54    

    using base_t = GridPuzzleAnalyzer<NUM_ROWS, NUM_COLS, NUM_STATES>;
    using grid_bits_t = BitSet<NUM_CELLS>;
    using state_bits_t = BitSet<NUM_STATES+1>;
    using region_bits_t = BitSet<NUM_REGIONS>;
    using region_pair_t = std::pair<size_t, size_t>;
    using symbol_set_t = emp::array<char, NUM_STATES+1>;

    // Track which cells are in each region.
    static const emp::array<grid_bits_t, NUM_REGIONS> & RegionMap() {
      static emp::array<grid_bits_t, NUM_REGIONS> bit_regions = BuildRegionMap();
      return bit_regions;
    }
    static emp::array<grid_bits_t, NUM_REGIONS> BuildRegionMap() {
      emp::array<grid_bits_t, NUM_REGIONS> bit_regions;

      // Rows (Overlaps: 111000000, 000111000, 000000111)
      bit_regions[0 ] = "{  0,  1,  2,  3,  4,  5,  6,  7,  8 }"; // Overlap with 18, 19, 20
      bit_regions[1 ] = "{  9, 10, 11, 12, 13, 14, 15, 16, 17 }"; // Overlap with 18, 19, 20
      bit_regions[2 ] = "{ 18, 19, 20, 21, 22, 23, 24, 25, 26 }"; // Overlap with 18, 19, 20
      bit_regions[3 ] = "{ 27, 28, 29, 30, 31, 32, 33, 34, 35 }"; // Overlap with 21, 22, 23
      bit_regions[4 ] = "{ 36, 37, 38, 39, 40, 41, 42, 43, 44 }"; // Overlap with 21, 22, 23
      bit_regions[5 ] = "{ 45, 46, 47, 48, 49, 50, 51, 52, 53 }"; // Overlap with 21, 22, 23
      bit_regions[6 ] = "{ 54, 55, 56, 57, 58, 59, 60, 61, 62 }"; // Overlap with 24, 25, 26
      bit_regions[7 ] = "{ 63, 64, 65, 66, 67, 68, 69, 70, 71 }"; // Overlap with 24, 25, 26
      bit_regions[8 ] = "{ 72, 73, 74, 75, 76, 77, 78, 79, 80 }"; // Overlap with 24, 25, 26

      // Columns (Overlaps: 111000000, 000111000, 000000111)
      bit_regions[9 ] = "{  0,  9, 18, 27, 36, 45, 54, 63, 72 }";  // Overlap with 18, 21, 24
      bit_regions[10] = "{  1, 10, 19, 28, 37, 46, 55, 64, 73 }";  // Overlap with 18, 21, 24
      bit_regions[11] = "{  2, 11, 20, 29, 38, 47, 56, 65, 74 }";  // Overlap with 18, 21, 24
      bit_regions[12] = "{  3, 12, 21, 30, 39, 48, 57, 66, 75 }";  // Overlap with 19, 22, 25
      bit_regions[13] = "{  4, 13, 22, 31, 40, 49, 58, 67, 76 }";  // Overlap with 19, 22, 25
      bit_regions[14] = "{  5, 14, 23, 32, 41, 50, 59, 68, 77 }";  // Overlap with 19, 22, 25
      bit_regions[15] = "{  6, 15, 24, 33, 42, 51, 60, 69, 78 }";  // Overlap with 20, 23, 26
      bit_regions[16] = "{  7, 16, 25, 34, 43, 52, 61, 70, 79 }";  // Overlap with 20, 23, 26
      bit_regions[17] = "{  8, 17, 26, 35, 44, 53, 62, 71, 80 }";  // Overlap with 20, 23, 26

      // Box Regions (Overlaps: 111000000, 000111000, 000000111, 100100100, 010010010, 001001001)
      bit_regions[18] = "{  0,  1,  2,  9, 10, 11, 18, 19, 20 }"; // Overlap with 0, 1, 2,  9, 10, 11
      bit_regions[19] = "{  3,  4,  5, 12, 13, 14, 21, 22, 23 }"; // Overlap with 0, 1, 2, 12, 13, 14
      bit_regions[20] = "{  6,  7,  8, 15, 16, 17, 24, 25, 26 }"; // Overlap with 0, 1, 2, 15, 16, 17
      bit_regions[21] = "{ 27, 28, 29, 36, 37, 38, 45, 46, 47 }"; // Overlap with 3, 4, 5,  9, 10, 11
      bit_regions[22] = "{ 30, 31, 32, 39, 40, 41, 48, 49, 50 }"; // Overlap with 3, 4, 5, 12, 13, 14
      bit_regions[23] = "{ 33, 34, 35, 42, 43, 44, 51, 52, 53 }"; // Overlap with 3, 4, 5, 15, 16, 17
      bit_regions[24] = "{ 54, 55, 56, 63, 64, 65, 72, 73, 74 }"; // Overlap with 6, 7, 8,  9, 10, 11
      bit_regions[25] = "{ 57, 58, 59, 66, 67, 68, 75, 76, 77 }"; // Overlap with 6, 7, 8, 12, 13, 14
      bit_regions[26] = "{ 60, 61, 62, 69, 70, 71, 78, 79, 80 }"; // Overlap with 6, 7, 8, 15, 16, 17

      return bit_regions;
    }
    static inline const grid_bits_t & RegionMap(size_t id) { return RegionMap()[id]; }
    static inline const grid_bits_t & RowMap(size_t id) { return RegionMap()[id]; }
    static inline const grid_bits_t & ColMap(size_t id) { return RegionMap()[id+NUM_ROWS]; }
    static inline const grid_bits_t & BoxMap(size_t id) { return RegionMap()[id+NUM_ROWS+NUM_COLS]; }


    // Track which region ids each cell belongs to.
    static const emp::array<region_bits_t, NUM_CELLS> & CellMemberships() {
      static emp::array<region_bits_t, NUM_CELLS> cell_regions = BuildCellMemberships();
      return cell_regions;
    }
    static emp::array<region_bits_t, NUM_CELLS> BuildCellMemberships() {
      emp::array<region_bits_t, NUM_CELLS> cell_regions;

      // Flip the region map.
      const auto & regions = RegionMap();
      for (size_t reg_id = 0; reg_id < NUM_REGIONS; ++reg_id) {
        const auto region = regions[reg_id];
        region.ForEach([&cell_regions, reg_id](size_t cell_id){
          cell_regions[cell_id].Set(reg_id);
        });
      }

      return cell_regions;
    }
    static const region_bits_t CellMemberships(size_t cell_id) {
      return CellMemberships()[cell_id];
    }

    // Track which cells are in the same regions as each other cell.
    // E.g., Cell 23 is at CellLinks()[23], with 1's in all positions with neighbor cell.
    //       If cell 23 is set to symbol 6, you can: bit_options[6] &= ~CellLinks()[23]
    static const emp::array<grid_bits_t, NUM_CELLS> & CellLinks() {
      static emp::array<grid_bits_t, NUM_CELLS> cell_links = BuildCellLinks();
      return cell_links;
    }
    static emp::array<grid_bits_t, NUM_CELLS> BuildCellLinks() {
      emp::array<grid_bits_t, NUM_CELLS> cell_links;

      // Link all cells to each other within each region.
      for (const auto & region : RegionMap()) {
        region.ForEachPair( [&cell_links](size_t id1, size_t id2) {
          cell_links[id1].Set(id2);
          cell_links[id2].Set(id1);
        } );
      }

      return cell_links;
    }
    static inline const grid_bits_t & CellLinks(size_t cell_id) {
      return CellLinks()[cell_id];
    }

    // Track which pairs of regions have multiple cells in common.
    static const emp::array<region_pair_t, NUM_OVERLAPS> & RegionOverlaps() {
      static emp::array<region_pair_t, NUM_OVERLAPS> overlaps = BuildRegionOverlaps();
      return overlaps;
    }
    static emp::array<region_pair_t, NUM_OVERLAPS> BuildRegionOverlaps() {
      emp::array<region_pair_t, NUM_OVERLAPS> overlaps;
      size_t overlap_id = 0;
      for (size_t region1 = 1; region1 < NUM_REGIONS; ++region1) {
        for (size_t region2 = 0; region2 < region1; ++region2) {
          auto cur_overlap = RegionMap(region1) & RegionMap(region2);
          if (cur_overlap.CountOnes() > 1) {
            overlaps[overlap_id].first = region1;
            overlaps[overlap_id].second = region2;
            ++overlap_id;
          }
        }
      }
      emp_assert(overlap_id == NUM_OVERLAPS);
      return overlaps;
    }

    /// Convert a set of region bits into the combined grid cells in those regions.
    grid_bits_t ComboRegion(region_bits_t region_ids) {
      grid_bits_t out;
      region_ids.ForEach([this,&out](size_t region_id){ out |= RegionMap(region_id); });
      return out;
    }


  public:
    SudokuAnalyzer() {
      // Set up the solver functions
      AddSolveFunction("CellLastState",  1.0, [this](){ return Solve_FindLastCellState(); });
      AddSolveFunction("RegionLastCell", 2.0, [this](){ return Solve_FindLastRegionState(); });
      AddSolveFunction("RegionOverlap",  3.0, [this](){ return Solve_FindRegionOverlap(); });
      AddSolveFunction("LimitedCells2",  4.0, [this](){ return Solve_FindLimitedCells2(); });
      AddSolveFunction("LimitedStates2", 5.0, [this](){ return Solve_FindLimitedStates2(); });
      AddSolveFunction("Swordfish2-RC",  6.0, [this](){ return Solve_FindSwordfish2_ROW_COL(); });
      AddSolveFunction("Swordfish2-Box", 7.0, [this](){ return Solve_FindSwordfish2_BOX(); });
      AddSolveFunction("LimitedCells3",  8.0, [this](){ return Solve_FindLimitedCells3(); });
      AddSolveFunction("LimitedStates3", 9.0, [this](){ return Solve_FindLimitedStates3(); });
      AddSolveFunction("Swordfish3-RC",  10.0, [this](){ return Solve_FindSwordfish3_ROW_COL(); });
      AddSolveFunction("Swordfish4-RC",  11.0, [this](){ return Solve_FindSwordfish4_ROW_COL(); });

      symbols = symbol_set_t{'-', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
      Clear();
    }
    SudokuAnalyzer(const SudokuAnalyzer &) = default;
    ~SudokuAnalyzer() { ; }

    static constexpr size_t GetNumCells() { return NUM_CELLS; }
    static constexpr size_t GetNumMoveTypes() { return 11; }

    SudokuAnalyzer& operator=(const SudokuAnalyzer &) = default;

    // Set the value of an individual cell; remove option from linked cells.
    // Return true/false based on whether progress was made toward solving the puzzle.
    bool Set(size_t cell, uint8_t state) override {
      if (base_t::Set(cell, state)) {
        // Make sure this state is also blocked from all linked cells.
        bit_options[state] &= ~CellLinks(cell);
        return true;
      }
      return false;
    }
    

    // Print the current state of the puzzle, including all options available.
    void Print(bool verbose=true, std::ostream & out=std::cout) override {
      if (verbose) PrintDetails(out);
      else PrintSimple(out);
    }

    void PrintSimple(std::ostream & out=std::cout) {
      for (size_t r = 0; r < NUM_ROWS; ++r) {
        for (size_t c = 0; c < NUM_COLS; ++c) {
          size_t id = r*9+c;
          if (!values[id]) out << " -";
          else out << " " << values[id];
        }
        out << std::endl;
      }
    }

    void PrintDetails(std::ostream & out=std::cout) {      
      out << " +-----------------------+-----------------------+-----------------------+"
          << std::endl;;
      for (size_t r = 0; r < NUM_ROWS; r++) {       // Puzzle row
        for (uint8_t s = 1; s <= NUM_STATES; s+=3) {    // Subset row
          for (size_t c = 0; c < NUM_COLS; c++) {   // Puzzle col
            size_t id = r*9+c;
            if (c%3==0) out << " |";
            else out << "  ";
            if (!values[id]) {
              out << " " << (HasOption(id,s)   ? ColorSymbol(s) : ".")
                  << " " << (HasOption(id,s+1) ? ColorSymbol(s+1) : ".")
                  << " " << (HasOption(id,s+2) ? ColorSymbol(s+2) : ".");
            } else {
              if (s==1) out << "      ";
              if (s==4) out << "   " << ColorSymbol(values[id], true) << "  ";
              if (s==7) out << "      ";
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
    // Return true if solvable, false if unsolvable.
    // @CAO - Change to use a stack of backup states, remove recursion, and track set counts for faster ID of failure (i.e., no 3's left, but only 8 of 9 set)
    bool ForceSolve(uint8_t cur_state=1) {
      emp_assert(cur_state > 0 && cur_state <= NUM_STATES);
      
      // Continue for as long as we have a state that still needs to be considered.
      while (cur_state <= NUM_STATES) {
        // Advance the start state if the current one is done.
        if (bit_options[cur_state].None()) {
          ++cur_state;
          continue;
        }

        // Try out the next option found:
        SudokuAnalyzer backup_state(*this);                 // Backup current state
        size_t cell_id = bit_options[cur_state].FindOne();  // ID the next cell to try
        Set(cell_id, cur_state);                            // Set next cell
        bool solved = ForceSolve(cur_state);                // Solve with new state
        if (solved) return true;                            // If solved, we're done!
        *this = backup_state;                               // Otherwise, restore from backup
        Block(cell_id, cur_state);                          // ...and eliminate this option
      }
      
      return IsSolved();      
    }

    // More human-focused solving techniques:

    // If there's only one state a cell can be, pick it!
    emp::vector<PuzzleMove> Solve_FindLastCellState() {
      emp::vector<PuzzleMove> moves;

      grid_bits_t unique_cells = emp::FindUniqueOnes(bit_options, 1);

      // Create a move for each cell with a unique option left.
      unique_cells.ForEach([this,&moves](size_t cell_id){
        uint8_t state = FindOption(cell_id);
        emp_assert(state != UNKNOWN_STATE);
        moves.push_back(PuzzleMove{PuzzleMove::SET_STATE, cell_id, state});
      });

      return moves;
    }

    // If there's only one cell that can have a certain state in a region, choose it!
    emp::vector<PuzzleMove> Solve_FindLastRegionState() {
      emp::vector<PuzzleMove> moves;

      // Loop through each state for each region testing.
      for (uint8_t state = 1; state <= NUM_STATES; ++state) {
        for (const grid_bits_t & region : RegionMap()) {
          grid_bits_t region_map = bit_options[state] & region;
          if (region_map.CountOnes() == 1) {
            moves.push_back(PuzzleMove{PuzzleMove::SET_STATE, region_map.FindOne(), state});
          }
        }
      }
      
      return moves;
    }

    // If only cells that can have a state in region A are all also in region
    // B, no other cell in region B can have that state as a possibility.
    emp::vector<PuzzleMove> Solve_FindRegionOverlap() {
      emp::vector<PuzzleMove> moves;

      for (auto [r1, r2] : RegionOverlaps()) {
        const auto overlap = RegionMap(r1) & RegionMap(r2);
        if ((overlap & ~is_set).CountOnes() < 2) continue;       // Enough free cells to matter?
        for (uint8_t state = 1; state <= NUM_STATES; ++state) {
          auto overlap_opts = bit_options[state] & overlap;
          bool o1 = ((bit_options[state] & r1) == overlap_opts);
          bool o2 = ((bit_options[state] & r2) == overlap_opts);
          if (o1 == o2) continue;  // Neither region can limit the other.
          if (o1) { // o1 limits o2
            auto clear_options = (bit_options[state] & r2) & ~overlap_opts;
            clear_options.ForEach([&moves,state](size_t pos){
              moves.push_back(PuzzleMove{PuzzleMove::SET_STATE, pos, state});
            });
          } else { // o2 limits o1
            auto clear_options = (bit_options[state] & r1) & ~overlap_opts;
            clear_options.ForEach([&moves,state](size_t pos){
              moves.push_back(PuzzleMove{PuzzleMove::SET_STATE, pos, state});
            });
          }
        }
      }
      
      return moves;
    }
    
    // If 2 cells in a region are all limited to the same 2 states, eliminate those states
    // from all other cells in the same region.
    emp::vector<PuzzleMove> Solve_FindLimitedCells2() {
      emp::vector<PuzzleMove> moves;

      // Identify which sites have exactly two options.
      grid_bits_t two_ones = FindTwoOnes(bit_options);

      // Try all (9*8/2=36) pairs of states and measure which sites have both options.
      for (uint8_t state1 = 1; state1 < NUM_STATES; ++state1) {
        for (uint8_t state2 = state1+1; state2 <= NUM_STATES; ++state2) {
          grid_bits_t both_states = bit_options[state1] & bit_options[state2] & two_ones;

          // If too few cells have exactly these two states, move on!
          if (both_states.CountOnes() < 2) continue;

          // Scan for relevant regions.
          for (const auto & region : RegionMap()) {
            // Does this region have the two instances to qualify?
            grid_bits_t both_states_r = both_states & region;
            if (both_states_r.CountOnes() < 2) continue;

            // Does it have OTHER sites to clean up?
            grid_bits_t state1_clear = bit_options[state1] & region & ~both_states;
            grid_bits_t state2_clear = bit_options[state2] & region & ~both_states;

            state1_clear.ForEach([state1,&moves](size_t pos){
              moves.push_back(PuzzleMove{PuzzleMove::BLOCK_STATE, pos, state1});
            });
            state2_clear.ForEach([state2,&moves](size_t pos){
              moves.push_back(PuzzleMove{PuzzleMove::BLOCK_STATE, pos, state2});
            });

          }
        }
      }

      return moves;
    }


    // If 3 cells in a region are all limited to the same 3 states, eliminate those states
    // from all other cells in the same region.
    emp::vector<PuzzleMove> Solve_FindLimitedCells3() {
      emp::vector<PuzzleMove> moves;

      // Identify which sites have exactly two or three options.
      grid_bits_t two_ones = FindTwoOnes(bit_options);
      grid_bits_t three_ones = FindThreeOnes(bit_options);
      grid_bits_t valid = two_ones | three_ones;

      // Try all (9*8*7/6=84) triples of states and measure which sites have these options.
      for (uint8_t state1 = 1; state1 < NUM_STATES-1; ++state1) {
        for (uint8_t state2 = state1+1; state2 < NUM_STATES; ++state2) {
          for (uint8_t state3 = state2+1; state3 <= NUM_STATES; ++state3) {

            // Which sites have at least two of these states and no others?
            grid_bits_t test_sites =
              (bit_options[state1] & bit_options[state2] & two_ones) |
              (bit_options[state1] & bit_options[state3] & two_ones) |
              (bit_options[state2] & bit_options[state3] & two_ones) |
              (bit_options[state1] & bit_options[state2] & bit_options[state3] & three_ones);
              
            // If too few cells have exactly these two states, move on!
            if (test_sites.CountOnes() < 3) continue;

            // Scan for relevant regions.
            for (const auto & region : RegionMap()) {
              // Does this region have the two instances to qualify?
              grid_bits_t test_sites_r = test_sites & region;
              if (test_sites_r.CountOnes() < 3) continue;

              // Does it have OTHER sites to clean up?
              grid_bits_t state1_clear = bit_options[state1] & region & ~test_sites;
              grid_bits_t state2_clear = bit_options[state2] & region & ~test_sites;
              grid_bits_t state3_clear = bit_options[state3] & region & ~test_sites;

              state1_clear.ForEach([state1,&moves](size_t pos){
                moves.push_back(PuzzleMove{PuzzleMove::BLOCK_STATE, pos, state1});
              });
              state2_clear.ForEach([state2,&moves](size_t pos){
                moves.push_back(PuzzleMove{PuzzleMove::BLOCK_STATE, pos, state2});
              });
              state3_clear.ForEach([state3,&moves](size_t pos){
                moves.push_back(PuzzleMove{PuzzleMove::BLOCK_STATE, pos, state3});
              });

            }
          }
        }
      }

      // Try all (9*8*7/6=84) triples of states?

      return moves;
    }


    // Eliminate all other possibilities from 2 cells if they are the only
    // ones that can possess 2 states in a single region.
    emp::vector<PuzzleMove> Solve_FindLimitedStates2() {
      emp::vector<PuzzleMove> moves;

      // Try all (9*8/2=36) pairs of states and measure which sites have both options.
      for (uint8_t state1 = 1; state1 < NUM_STATES; ++state1) {
        for (uint8_t state2 = state1+1; state2 <= NUM_STATES; ++state2) {
          grid_bits_t both_states = bit_options[state1] & bit_options[state2];
          grid_bits_t one_state = bit_options[state1] ^ bit_options[state2];

          // If too few cells have exactly these two states, move on!
          if (both_states.CountOnes() < 2) continue;

          // Scan for relevant regions.
          for (const auto & region : RegionMap()) {
            // Does this region have exactly two instances for this pair?
            grid_bits_t both_states_r = both_states & region;
            if (both_states_r.CountOnes() != 2) continue;

            // These have to be the only two sites
            grid_bits_t one_state_r = one_state & region;
            if (one_state_r.Any()) continue;

            size_t pos1 = both_states_r.FindOne();
            size_t pos2 = both_states_r.FindOne(pos1+1);

            // Do either of those have OTHER states to block?
            for (uint8_t block_state = 1; block_state <= NUM_STATES; ++block_state) {
              if (block_state == state1 || block_state == state2) continue;
              if (bit_options[block_state].Has(pos1)) {
                moves.push_back(PuzzleMove{PuzzleMove::BLOCK_STATE, pos1, block_state});                
              }
              if (bit_options[block_state].Has(pos2)) {
                moves.push_back(PuzzleMove{PuzzleMove::BLOCK_STATE, pos2, block_state});                
              }
            }
          }
        }
      }

      // Try all (9*8*7/6=84) triples of states?

      return moves;
    }

    // Eliminate all other possibilities from 3 cells if they are the only
    // ones that can possess 3 states in a single region.
    emp::vector<PuzzleMove> Solve_FindLimitedStates3() {
      emp::vector<PuzzleMove> moves;

      // Check each region.
      for (const auto & region : RegionMap()) {
        // Try all (9*8*7/2*3=84) triples of states and measure which sites have both options.
        for (uint8_t state1 = 1; state1 < NUM_STATES-1; ++state1) {
          grid_bits_t state1_r = bit_options[state1] & region;
          if (state1_r.None() || state1_r.CountOnes() > 3) continue;

          for (uint8_t state2 = state1+1; state2 < NUM_STATES; ++state2) {
            grid_bits_t state2_r = bit_options[state2] & region;
            if (state2_r.None() || state2_r.CountOnes() > 3) continue;

            for (uint8_t state3 = state2+1; state3 <= NUM_STATES; ++state3) {
              grid_bits_t state3_r = bit_options[state3] & region;
              if (state3_r.None() || state3_r.CountOnes() > 3) continue;

              // Are these states combined limited to just 3 sites?
              grid_bits_t final_sites = state1_r | state2_r | state3_r;
              if (final_sites.CountOnes() != 3) continue;

              size_t pos1 = final_sites.FindOne();
              size_t pos2 = final_sites.FindOne(pos1+1);
              size_t pos3 = final_sites.FindOne(pos2+1);

              // Do either of those have OTHER states to block?
              for (uint8_t block_state = 1; block_state <= NUM_STATES; ++block_state) {
                if (block_state == state1 || block_state == state2 || block_state == state3) continue;
                if (bit_options[block_state].Has(pos1)) {
                  // std::cout << "STATES " << (state1+1) << (state2+1) << (state3+1)
                  //   << " at sites "
                  //   << CellToCoords(pos1) << ", "
                  //   << CellToCoords(pos2) << ", "
                  //   << CellToCoords(pos3) << ", "
                  //   << "blocking state " << (block_state+1) << " at pos1"
                  //   << std::endl;
                  moves.push_back(PuzzleMove{PuzzleMove::BLOCK_STATE, pos1, block_state});                
                }
                if (bit_options[block_state].Has(pos2)) {
                  moves.push_back(PuzzleMove{PuzzleMove::BLOCK_STATE, pos2, block_state});                
                }
                if (bit_options[block_state].Has(pos3)) {
                  moves.push_back(PuzzleMove{PuzzleMove::BLOCK_STATE, pos2, block_state});                
                }
              }
            }
          }
        }
      }

      return moves;
    }

    // If there are X rows/cols where a certain state can only be in one of X other regions,
    // then no other cells in those latter regions can be that state.
    emp::vector<PuzzleMove> Solve_FindSwordfish2_ROW_COL() {
      emp::vector<PuzzleMove> moves;

      for (uint8_t state = 1; state <= NUM_STATES; ++state) {
        for (size_t region1_id = 0; region1_id < NUM_ROWS+NUM_COLS; ++region1_id) {
          auto region1 = RegionMap(region1_id) & bit_options[state];
          if (region1.CountOnes() != 2) continue;
          for (size_t region2_id = region1_id+1; region2_id % NUM_ROWS; ++region2_id) {
            auto region2 = RegionMap(region2_id) & bit_options[state];
            if (region2.CountOnes() != 2) continue;

            size_t cell1a = region1.FindOne();
            size_t cell1b = region1.FindOne(cell1a+1);
            size_t cell2a = region2.FindOne();
            size_t cell2b = region2.FindOne(cell2a+1);

            region_bits_t a_regions = CellMemberships(cell1a) & CellMemberships(cell2a);
            region_bits_t b_regions = CellMemberships(cell1b) & CellMemberships(cell2b);

            // If both cell pairs don't share at least one region, we can't do a swordfish
            if (a_regions.None() || b_regions.None()) continue;

            // For each cross region, remove additional "state" options.
            grid_bits_t target_cells = bit_options[state] & ~region1 & ~region2
                & ComboRegion(a_regions | b_regions);

            target_cells.ForEach([&moves,state](size_t cell_id){
              moves.push_back(PuzzleMove{PuzzleMove::BLOCK_STATE, cell_id, state});
            });

          }
        }
      }

      return moves;
    }


    // If there are 2 regions where a certain state can only be in one of 2 other regions,
    // then no other cells in those latter regions can be that state.  In this case, one
    // of those original regions should be a box.
    emp::vector<PuzzleMove> Solve_FindSwordfish2_BOX() {
      emp::vector<PuzzleMove> moves;

      for (uint8_t state = 1; state <= NUM_STATES; ++state) {
        for (size_t box_id = NUM_ROWS+NUM_COLS; box_id < NUM_REGIONS; ++box_id) {
          auto region1 = RegionMap(box_id) & bit_options[state];
          if (region1.CountOnes() != 2) continue;
          for (size_t region2_id = 0; region2_id < NUM_ROWS+NUM_COLS; ++region2_id) {
            auto region2 = RegionMap(region2_id) & bit_options[state];
            if (region2.CountOnes() != 2) continue;
            if ((region1 & region2).Any()) continue; // No double-counting of cells.

            size_t cell1a = region1.FindOne();
            size_t cell1b = region1.FindOne(cell1a+1);
            size_t cell2a = region2.FindOne();
            size_t cell2b = region2.FindOne(cell2a+1);

            region_bits_t aa_regions = CellMemberships(cell1a) & CellMemberships(cell2a);
            region_bits_t bb_regions = CellMemberships(cell1b) & CellMemberships(cell2b);

            // If both cell pairs don't share at least one region, we can't do a swordfish
            if (aa_regions.Any() && bb_regions.Any()) {
              // For each cross region, remove additional "state" options.
              grid_bits_t target_cells = bit_options[state] & ~region1 & ~region2
                  & ComboRegion(aa_regions | bb_regions);

              target_cells.ForEach([&moves,state](size_t cell_id){
                moves.push_back(PuzzleMove{PuzzleMove::BLOCK_STATE, cell_id, state});
              });

              // std::cout << "REGION 1 (" << box_id << "): " << CellToCoords(cell1a) << " and " << CellToCoords(cell1b)
              //   << "; REGION 2 (" << region2_id << "): " << CellToCoords(cell2a) << " and " << CellToCoords(cell2b)
              //   << std::endl;
            }

            region_bits_t ab_regions = CellMemberships(cell1a) & CellMemberships(cell2b);
            region_bits_t ba_regions = CellMemberships(cell1b) & CellMemberships(cell2a);

            if (ab_regions.Any() && ba_regions.Any()) {
              // For each cross region, remove additional "state" options.
              grid_bits_t target_cells = bit_options[state] & ~region1 & ~region2
                  & ComboRegion(ab_regions | ba_regions);

              target_cells.ForEach([&moves,state](size_t cell_id){
                moves.push_back(PuzzleMove{PuzzleMove::BLOCK_STATE, cell_id, state});
              });

              // std::cout << "REGION 1 (" << box_id << "): " << CellToCoords(cell1a) << " and " << CellToCoords(cell2b)
              //   << "; REGION 2 (" << region2_id << "): " << CellToCoords(cell2a) << " and " << CellToCoords(cell1b)
              //   << std::endl;
            }

          }
        }
      }

      return moves;
    }


    // If there are 3 rows/cols where a certain state can only be in one of 3 other regions,
    // then no other cells in those latter regions can be that state.
    emp::vector<PuzzleMove> Solve_FindSwordfish3_ROW_COL() {
      emp::vector<PuzzleMove> moves;

      for (uint8_t state = 1; state <= NUM_STATES; ++state) {
        for (size_t region1_id = 0; region1_id < NUM_ROWS+NUM_COLS; ++region1_id) {
          auto region1 = RegionMap(region1_id) & bit_options[state];
          if (region1.CountOnes() != 3) continue;
          for (size_t region2_id = region1_id+1; region2_id % NUM_ROWS; ++region2_id) {
            auto region2 = RegionMap(region2_id) & bit_options[state];
            if (region2.CountOnes() != 3) continue;
            for (size_t region3_id = region2_id+1; region3_id % NUM_ROWS; ++region3_id) {
              auto region3 = RegionMap(region3_id) & bit_options[state];
              if (region3.CountOnes() != 3) continue;

              size_t cell1a = region1.FindOne();
              size_t cell1b = region1.FindOne(cell1a+1);
              size_t cell1c = region1.FindOne(cell1b+1);
              size_t cell2a = region2.FindOne();
              size_t cell2b = region2.FindOne(cell2a+1);
              size_t cell2c = region2.FindOne(cell2b+1);
              size_t cell3a = region3.FindOne();
              size_t cell3b = region3.FindOne(cell3a+1);
              size_t cell3c = region3.FindOne(cell3b+1);

              region_bits_t a_regions = CellMemberships(cell1a) & CellMemberships(cell2a) & CellMemberships(cell3a);
              region_bits_t b_regions = CellMemberships(cell1b) & CellMemberships(cell2b) & CellMemberships(cell3b);
              region_bits_t c_regions = CellMemberships(cell1c) & CellMemberships(cell2c) & CellMemberships(cell3c);

              // If all cell groups don't share at least one region, we can't do a swordfish
              if (a_regions.None() || b_regions.None() || c_regions.None()) continue;

              // For each cross region, remove additional "state" options.
              grid_bits_t target_cells = bit_options[state] & ~region1 & ~region2 & ~region3
                  & ComboRegion(a_regions | b_regions | c_regions);

              // std::cout << "\nSF3 BLOCKING: state=" << (state+1)
              //   << " cell1a=" << CellToCoords(cell1a)
              //   << " cell1b=" << CellToCoords(cell1b)
              //   << " cell1c=" << CellToCoords(cell1c)
              //   << " cell2a=" << CellToCoords(cell2a)
              //   << " cell2b=" << CellToCoords(cell2b)
              //   << " cell2c=" << CellToCoords(cell2c)
              //   << " cell3a=" << CellToCoords(cell3a)
              //   << " cell3b=" << CellToCoords(cell3b)
              //   << " cell3c=" << CellToCoords(cell3c)
              //   << std::endl;

              target_cells.ForEach([&moves,state](size_t cell_id){
                moves.push_back(PuzzleMove{PuzzleMove::BLOCK_STATE, cell_id, state});
              });

            }
          }
        }
      }

      return moves;
    }


    // If there are 4 rows/cols where a certain state can only be in one of 4 other regions,
    // then no other cells in those latter regions can be that state.
    emp::vector<PuzzleMove> Solve_FindSwordfish4_ROW_COL() {
      emp::vector<PuzzleMove> moves;

      for (uint8_t state = 1; state <= NUM_STATES; ++state) {
        for (size_t region1_id = 0; region1_id < NUM_ROWS+NUM_COLS; ++region1_id) {
          auto region1 = RegionMap(region1_id) & bit_options[state];
          if (region1.CountOnes() != 4) continue;
          for (size_t region2_id = region1_id+1; region2_id % NUM_ROWS; ++region2_id) {
            auto region2 = RegionMap(region2_id) & bit_options[state];
            if (region2.CountOnes() != 4) continue;
            for (size_t region3_id = region2_id+1; region3_id % NUM_ROWS; ++region3_id) {
              auto region3 = RegionMap(region3_id) & bit_options[state];
              if (region3.CountOnes() != 4) continue;
              for (size_t region4_id = region3_id+1; region4_id % NUM_ROWS; ++region4_id) {
                auto region4 = RegionMap(region4_id) & bit_options[state];
                if (region4.CountOnes() != 4) continue;

                size_t cell1a = region1.FindOne();
                size_t cell1b = region1.FindOne(cell1a+1);
                size_t cell1c = region1.FindOne(cell1b+1);
                size_t cell1d = region1.FindOne(cell1c+1);
                size_t cell2a = region2.FindOne();
                size_t cell2b = region2.FindOne(cell2a+1);
                size_t cell2c = region2.FindOne(cell2b+1);
                size_t cell2d = region2.FindOne(cell2c+1);
                size_t cell3a = region3.FindOne();
                size_t cell3b = region3.FindOne(cell3a+1);
                size_t cell3c = region3.FindOne(cell3b+1);
                size_t cell3d = region3.FindOne(cell3c+1);
                size_t cell4a = region4.FindOne();
                size_t cell4b = region4.FindOne(cell4a+1);
                size_t cell4c = region4.FindOne(cell4b+1);
                size_t cell4d = region4.FindOne(cell4c+1);

                region_bits_t a_regions = CellMemberships(cell1a) & CellMemberships(cell2a)
                                        & CellMemberships(cell3a) & CellMemberships(cell4a);
                region_bits_t b_regions = CellMemberships(cell1b) & CellMemberships(cell2b)
                                        & CellMemberships(cell3b) & CellMemberships(cell4b);
                region_bits_t c_regions = CellMemberships(cell1c) & CellMemberships(cell2c)
                                        & CellMemberships(cell3c) & CellMemberships(cell4c);
                region_bits_t d_regions = CellMemberships(cell1d) & CellMemberships(cell2d)
                                        & CellMemberships(cell3d) & CellMemberships(cell4d);

                // If all cell groups don't share at least one region, we can't do a swordfish
                if (a_regions.None() || b_regions.None() || c_regions.None() || d_regions.None()) continue;

                // For each cross region, remove additional "state" options.
                grid_bits_t target_cells = bit_options[state] & ~region1 & ~region2 & ~region3 & ~region4
                    & ComboRegion(a_regions | b_regions | c_regions | d_regions);

                // std::cout << "\nSF4 BLOCKING: state=" << (state+1)
                //   << " cell1a=" << CellToCoords(cell1a)
                //   << " cell1b=" << CellToCoords(cell1b)
                //   << " cell1c=" << CellToCoords(cell1c)
                //   << " cell1d=" << CellToCoords(cell1d)
                //   << " cell2a=" << CellToCoords(cell2a)
                //   << " cell2b=" << CellToCoords(cell2b)
                //   << " cell2c=" << CellToCoords(cell2c)
                //   << " cell2d=" << CellToCoords(cell2d)
                //   << " cell3a=" << CellToCoords(cell3a)
                //   << " cell3b=" << CellToCoords(cell3b)
                //   << " cell3c=" << CellToCoords(cell3c)
                //   << " cell3d=" << CellToCoords(cell3d)
                //   << " cell4a=" << CellToCoords(cell4a)
                //   << " cell4b=" << CellToCoords(cell4b)
                //   << " cell4c=" << CellToCoords(cell4c)
                //   << " cell4d=" << CellToCoords(cell4d)
                //   << std::endl;

                target_cells.ForEach([&moves,state](size_t cell_id){
                  moves.push_back(PuzzleMove{PuzzleMove::BLOCK_STATE, cell_id, state});
                });

              }
            }
          }
        }
      }

      return moves;
    }


    // Make sure the current state is consistant.
    bool OK() {
      return true;
    }
  };

  
} // END emp namespace

#endif
