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

    static constexpr size_t NUM_CELLS = NUM_ROWS * NUM_COLS;                  // 81
    static constexpr size_t NUM_REGIONS = NUM_ROWS + NUM_COLS + NUM_SQUARES;  // 27
    static constexpr size_t REGIONS_PER_CELL = 3;   // Each cell is part of three regions.
    static constexpr size_t NO_REGION = static_cast<size_t>(-1);

    // Calculate multi-cell overlaps between regions; each row/col overlaps with 3 square regions.
    static constexpr size_t NUM_OVERLAPS = (NUM_ROWS + NUM_COLS) * 3 ;        // 54
    
    static constexpr uint8_t UNKNOWN_STATE = NUM_STATES; // Lower values are actual states.

    using grid_bits_t = BitSet<NUM_CELLS>;
    using state_bits_t = BitSet<NUM_STATES>;
    using region_bits_t = BitSet<NUM_REGIONS>;
    using region_pairs_t = std::pair<size_t, size_t>;

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
    static const emp::array<region_pairs_t, NUM_OVERLAPS> & RegionOverlaps() {
      static emp::array<region_pairs_t, NUM_OVERLAPS> overlaps = BuildRegionOverlaps();
      return overlaps;
    }
    static emp::array<region_pairs_t, NUM_OVERLAPS> BuildRegionOverlaps() {
      emp::array<region_pairs_t, NUM_OVERLAPS> overlaps;
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

    ////////////////////////////////////
    //                                //
    //    --- Member Variables ---    //
    //                                //
    ////////////////////////////////////


    // Which symbols are we using in this puzzle? (default to standard)
    emp::array<char, NUM_STATES> symbols = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};

    emp::array<uint8_t,NUM_CELLS> value;      // Known value for cells
    // emp::array<uint32_t, NUM_CELLS> options;  // Options still available to each cell

    // Track the available options by state, across the whole puzzle.
    // E.g., symbol 5 is at bit_options[5] and has 81 bits indicating if each cell can be '5'.
    emp::array<grid_bits_t, NUM_STATES> bit_options;
    grid_bits_t is_set;

    
  public:
    SudokuAnalyzer() { Clear(); }
    SudokuAnalyzer(const SudokuAnalyzer &) = default;
    ~SudokuAnalyzer() { ; }

    SudokuAnalyzer& operator=(const SudokuAnalyzer &) = default;

    uint8_t GetValue(size_t cell) const { return value[cell]; }
    // uint32_t GetOptions(size_t cell) const { return options[cell]; }
    size_t SymbolToState(char symbol) const {
      for (size_t i=0; i < NUM_STATES; ++i) {
        if (symbols[i] == symbol) return i;
      }
      return UNKNOWN_STATE;
    }

    /// Test if a cell is allowed to be a particular state.
    bool HasOption(size_t cell, uint8_t state) {
      emp_assert(cell < 81, cell);
      emp_assert(state >= 0 && state < NUM_STATES, state);
      return bit_options[state].Has(cell);
    }

    /// Return a currently valid option for provided cell; may not be correct solution
    uint8_t FindOption(size_t cell) {
      for (uint8_t state = 0; state < NUM_STATES; ++state) {
        if (HasOption(cell, state)) return state;
      }
      return UNKNOWN_STATE;
    }
    bool IsSet(size_t cell) const { return value[cell] != UNKNOWN_STATE; }
    bool IsSolved() const { return is_set.All(); }
    
    // Clear out the old solution info when starting a new solve attempt.
    void Clear() {
      value.fill(UNKNOWN_STATE);
      for (auto & val_options : bit_options) {
        val_options.SetAll();
      }
      is_set.Clear();
    }

    // Load a board state from a stream.
    void Load(std::istream & is) {
      // Format: Provide site by site with a dash for empty; whitespace is ignored.
      char cur_char;
      size_t cell_id = 0;
      while (cell_id < NUM_CELLS) {
        is >> cur_char;
        if (emp::is_whitespace(cur_char)) continue;
        ++cell_id;
        if (cur_char == '-') continue;
        uint8_t state_id = SymbolToState(cur_char);
        if (state_id == UNKNOWN_STATE) {
          emp::notify::Warning("Unknown sudoku symbol '", cur_char, "'.  Ignoring.");
          continue;
        }
        Set(cell_id-1, state_id);
      }
    }

    // Load a board state from a file.
    void Load(const emp::String & filename) {
      std::ifstream file(filename);
      return Load(file);
    }

    // Set the value of an individual cell; remove option from linked cells.
    // Return true/false based on whether progress was made toward solving the puzzle.
    bool Set(size_t cell, uint8_t state) {
      emp_assert(cell < NUM_CELLS);   // Make sure cell is in a valid range.
      emp_assert(state < NUM_STATES); // Make sure state is in a valid range.

      if (value[cell] == state) return false;  // If state is already set, SKIP!

      emp_assert(HasOption(cell,state));     // Make sure state is allowed.
      is_set.Set(cell);
      value[cell] = state;                   // Store found value!

      // Clear this cell from all sets of options.
      for (size_t s=0; s < NUM_STATES; ++s) bit_options[s].Clear(cell);
      
      // Now make sure this state is blocked from all linked cells.
      bit_options[state] &= ~CellLinks(cell);

      return true;
    }
    
    // Remove a symbol option from a particular cell.
    void Block(size_t cell, uint8_t state) {
      bit_options[state].Clear(cell);
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

    // Scan for contradictions or lack of options that would make puzzle unsolvable.
    bool IsUnsolvable() {
      // Identify which cells are either set or still have options.
      grid_bits_t has_options = is_set;
      for (uint8_t state = 0; state < NUM_STATES; ++state) {
        has_options |= bit_options[state];
      }

      return !has_options.All();
    }

    // Use a brute-force approach to completely solve this puzzle.
    // Return true if solvable, false if unsolvable.
    // @CAO - Change to use a stack of backup states, remove recursion, and track set counts for faster ID of failure (i.e., no 3's left, but only 8 of 9 set)
    bool ForceSolve(uint8_t cur_state=0) {
      emp_assert(cur_state <= NUM_STATES);
      
      // Continue for as long as we have a state that still needs to be considered.
      while (cur_state < NUM_STATES) {
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
      std::cout << "TESTING 1:LastCState" << std::endl;
      emp::vector<PuzzleMove> moves;

      grid_bits_t unique_cells = emp::FindUniqueOnes(bit_options);

      // Create a move for each cell with a unique option left.
      unique_cells.ForEach([this,&moves](size_t cell_id){
        uint8_t state = FindOption(cell_id);
        moves.push_back(PuzzleMove{PuzzleMove::SET_STATE, cell_id, state});
      });

      return moves;
    }

    // If there's only one cell that can have a certain state in a region, choose it!
    emp::vector<PuzzleMove> Solve_FindLastRegionState() {
      std::cout << "TESTING 2:LastRState" << std::endl;

      emp::vector<PuzzleMove> moves;

      // Loop through each state for each region testing.
      for (uint8_t state = 0; state < NUM_STATES; ++state) {
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
      std::cout << "TESTING 3:RegionOverlap" << std::endl;
      emp::vector<PuzzleMove> moves;

      for (auto [r1, r2] : RegionOverlaps()) {
        const auto overlap = RegionMap(r1) & RegionMap(r2);
        if ((overlap & ~is_set).CountOnes() < 2) continue;       // Enough free cells to matter?
        for (uint8_t state = 0; state < NUM_STATES; ++state) {
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
    
    // If K cells in a region are all limited to the same K states, eliminate those states
    // from all other cells in the same region.
    emp::vector<PuzzleMove> Solve_FindLimitedCells2() {
      std::cout << "TESTING 4:LimitedCells" << std::endl;

      emp::vector<PuzzleMove> moves;

      // Identify which sites have exactly two options.
      grid_bits_t two_ones = FindTwoOnes(bit_options);

      // Try all (9*8/2=36) pairs of states and measure which sites have both options.
      for (uint8_t state1 = 0; state1 < NUM_STATES-1; ++state1) {
        for (uint8_t state2 = state1+1; state2 < NUM_STATES; ++state2) {
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

      // Try all (9*8*7/6=84) triples of states?

      return moves;
    }
    
    // Eliminate all other possibilities from K cells if they are the only
    // ones that can possess K states in a single region.
    emp::vector<PuzzleMove> Solve_FindLimitedStates2() {
      std::cout << "TESTING 5:LimitedStates" << std::endl;
      Print();

      emp::vector<PuzzleMove> moves;

      // Try all (9*8/2=36) pairs of states and measure which sites have both options.
      for (uint8_t state1 = 0; state1 < NUM_STATES-1; ++state1) {
        for (uint8_t state2 = state1+1; state2 < NUM_STATES; ++state2) {
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
            for (uint8_t block_state = 0; block_state < NUM_STATES; ++block_state) {
              if (block_state == state1 || block_state == state2) continue;
              if (bit_options[block_state].Has(pos1)) {
                // std::cout << "Target states " << (state1+1) << " and " << (state2+1) << ": "
                //           << "blocking state " << (block_state+1) << " at position " << pos1
                //           << " (" << (pos1%9) << "," << (pos1/9) << ")"
                //           << std::endl;
                moves.push_back(PuzzleMove{PuzzleMove::BLOCK_STATE, pos1, block_state});                
                // static int prints = 0;
                // prints++;
                // if (prints == 10) exit(0);
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

    // If there are X rows (cols) where a certain state can only be in one of 
    // X cols (rows), then no other row in this cols can be that state.
    emp::vector<PuzzleMove> Solve_FindSwordfish2_ROW() {
      emp::vector<PuzzleMove> moves;

      for (uint8_t state = 0; state < NUM_STATES; ++state) {
        emp::array<uint16_t, 9> row_size;
        for (size_t row1_id = 0; row1_id < NUM_ROWS; ++row1_id) {
          auto row1 = RowMap(row1_id) & bit_options[state];
          row_size[row1_id] = row1.CountOnes();
          if (row_size[row1_id] != 2) continue;
          for (size_t row2_id = 0; row2_id < row1_id; ++row2_id) {
            if (row_size[row2_id] != 2) continue;
            auto row2 = RowMap(row2_id) & bit_options[state];
            auto combo = row1 | row2;
            // Both rows have two instances.  See if those instances are share two other regions
            // (cols or boxes); only step through allowed options.
            size_t region1 = NO_REGION;
            size_t region2 = NO_REGION;
            for (size_t test_region = NUM_ROWS; test_region < NUM_REGIONS; ++test_region ) {
              if ((RegionMap(test_region) & combo).CountOnes() == 2) {
                if (region1 == NO_REGION) {
                  region1 = test_region;
                  continue;
                } else {
                  region2 = test_region;
                  break;
                }
              }
            }

            // If we have identified BOTH relevant regions, strip any extra states from them.
            if (region2 != NO_REGION) {
              auto clear_ids = (RegionMap(region1) | RegionMap(region2)) & ~combo & bit_options[state];
              clear_ids.ForEach([state,&moves](size_t pos){
                moves.push_back(PuzzleMove{PuzzleMove::BLOCK_STATE, pos, state});
              });
            }

            // size_t cell1a = row1.FindOne();
            // size_t cell2a = row2.FindOne();
            // auto mem1a = CellMemberships(cell1a);
            // auto mem2a = CellMemberships(cell2a);
          }
        }
      }

      return moves;
    }

    // Calculate the full solving profile based on the other techniques.
    PuzzleProfile CalcProfile()
    {
      PuzzleProfile profile;

      using move_set_t = emp::vector<PuzzleMove>;
      using solve_fun_t = move_set_t();
      emp::vector<std::function<solve_fun_t>> solve_funs;

      // The list of moves to try for this puzzle.
      solve_funs.push_back([this](){ return Solve_FindLastCellState(); });
      solve_funs.push_back([this](){ return Solve_FindLastRegionState(); });
      solve_funs.push_back([this](){ return Solve_FindRegionOverlap(); });
      solve_funs.push_back([this](){ return Solve_FindLimitedCells2(); });
      solve_funs.push_back([this](){ return Solve_FindLimitedStates2(); });
      solve_funs.push_back([this](){ return Solve_FindSwordfish2_ROW(); });

      move_set_t moves;
      size_t move_id = 0;
      while (move_id < solve_funs.size()) {
        moves = solve_funs[move_id]();
        if (moves.size() > 0) {
          Move(moves);                              // Trigger the full set of found moves.
          profile.AddMoves(move_id, moves.size());  // Place move record into solve profile.
          move_id = 0;                              // Start from the easiest move after a change.
          if (IsUnsolvable()) break;
        }
        else {  
          ++move_id;  // This move didn't work; shift to the next one.
        }
      }

      return profile;
    }

    // Make sure the current state is consistant.
    bool OK() {
      return true;
    }
  };

  
} // END emp namespace

#endif
