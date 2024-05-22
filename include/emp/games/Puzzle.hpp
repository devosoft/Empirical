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

#include "../tools/String.hpp"

namespace emp {

  struct PuzzleMove {
    enum MoveType { BLOCK_STATE, SET_STATE };

    MoveType type;
    int pos_id;
    int state;
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
  
} // End namespace: emp

#endif