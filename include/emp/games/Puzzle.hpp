/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2024.
*/
/**
 *  @file
 *  @brief  Tools for all puzzle-analysis techniques.
*/

#ifndef EMP_GAMES_PUZZLE_HPP
#define EMP_GAMES_PUZZLE_HPP

#include <iostream>

#include "../tools/String.hpp"

namespace emp {

  struct PuzzleMove {
    enum MoveType { BLOCK_STATE, SET_STATE };

    MoveType type;
    size_t pos_id;
    uint8_t state;

    emp::String TypeString() const {
      if (type == BLOCK_STATE) return "BLOCKING";
      else return "SETTING";
    }

    emp::String ToString() const {
      return MakeString(TypeString(), " state ", state+1, " at position ", pos_id);
    }

    std::ostream & Print(std::ostream & os=std::cout) const {
      return os << ToString();
    }
  };

  struct PuzzleSolveFun {
    using move_set_t = emp::vector<PuzzleMove>;
    using solve_fun_t = move_set_t();

    std::function<solve_fun_t> solve_fun;
    String move_name;
    double difficulty;
  };

  struct PuzzleProfile {
    struct Slice {
      size_t level;  // How difficult is the easiest move at this point?
      size_t count;  // How many options are there for this move?

      String ToString() const { return MakeString(level, ":", count); }
    };

    enum FinalState { SOLVED, UNSOLVED, UNSOLVABLE };

    std::vector<Slice> slices; // Actual solving profile.
    FinalState final_state = UNSOLVED;

    bool IsSolved() const { return final_state == SOLVED; }
    bool IsUnsolved() const { return final_state == UNSOLVED; }
    bool IsUnsolvable() const { return final_state == UNSOLVABLE; }
    
    void AddMoves(size_t level, size_t count) {
      slices.push_back(Slice{level, count});
    }

    void SetSolved()     { final_state = SOLVED; }
    void SetUnsolved()   { final_state = UNSOLVED; }
    void SetUnsolvable() { final_state = UNSOLVABLE; }
    
    void Clear() { slices.resize(0); }

    String ToString() const {
      String out;
      for (auto & slice : slices) { out.Append(slice.ToString(), " "); }
      return out;
    }

    void Print(std::ostream & out=std::cout) const {
      out << ToString() << std::endl;
    }
  };
  

  /// @brief A generic analyzer for puzzles that have a set of positions, each in a set of values.
  template <size_t NUM_POSITIONS, size_t NUM_STATES>
  struct PuzzleAnalyzer {
    static constexpr size_t num_pos = NUM_POSITIONS;
    static constexpr size_t num_vals = NUM_STATES;

    using symbol_set_t = emp::array<char, NUM_STATES>;
  };
} // End namespace: emp

#endif