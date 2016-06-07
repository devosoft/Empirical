//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A Non-deterministic Finite Automata simulator

#ifndef EMP_NFA_H
#define EMP_NFA_H

#include <map>

#include "BitSet.h"
#include "vector.h"

namespace emp {

  class NFA {
  private:
    constexpr static int NUM_SYMBOLS = 128;
    using opts_t = BitSet<NUM_SYMBOLS>;

    struct Transition {
      int to_id;
      opts_t symbols;
    };
    struct State {
      std::map<int, Transition> trans;
    };

    emp::vector<State> states;
  public:
    NFA(int num_states=0) : states(num_states) { ; }
    ~NFA() { ; }

    void Resize(int new_size) { states.resize(new_size); }
    void AddTransition(int from, int to, char sym) {
      emp_assert(from >= 0 && from < (int) states.size(), from, states.size());
      emp_assert(to >= 0 && to < (int) states.size(), to, states.size());
      emp_assert(sym >= 0, sym);

      auto & tmap = states[from].trans;
      if (tmap.find(to) == tmap.end()) { tmap[to].to_id = to; }
      tmap[to].symbols[sym] = true;
    }
  };

}

#endif
