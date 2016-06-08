//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A Deterministic Finite Automata simulator

#ifndef EMP_DFA_H
#define EMP_DFA_H

#include "vector.h"

namespace emp {

  class DFA {
  private:
    constexpr static int NUM_SYMBOLS = 128;
    emp::vector< emp::array<int, NUM_SYMBOLS> > transitions;
  public:
    DFA(int num_states=0) : transitions(num_states) { ; }
    ~DFA() { ; }

    void SetTransition(int from, int to, char sym) {
      emp_assert(from >= 0 && from < (int) transitions.size());
      emp_assert(to >= 0 && to < (int) transitions.size());
      emp_assert(sym >= 0 && sym < NUM_SYMBOLS);
      transitions[from][sym] = to;
    }

    int Next(int state, int sym) const {
      emp_assert(state >= 0 && state < (int) transitions.size());
      emp_assert(sym >= 0 && sym < NUM_SYMBOLS);
      return transitions[state][sym];
    }
  };

}

#endif
