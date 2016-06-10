//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A Deterministic Finite Automata simulator

#ifndef EMP_DFA_H
#define EMP_DFA_H

#include <string>

#include "array.h"
#include "vector.h"

namespace emp {

  class DFA {
  private:
    constexpr static int NUM_SYMBOLS = 128;
    emp::vector< emp::array<int, NUM_SYMBOLS> > transitions;
    emp::vector< char > is_stop;  // 0=no 1=yes (char instead of bool for speed)
  public:
    DFA(int num_states=0) : transitions(num_states), is_stop(num_states, 0) {
      for (auto & t : transitions) t.fill(-1);
    }
    DFA(const DFA &) = default;
    ~DFA() { ; }
    DFA & operator=(const DFA &) = default;

    void Resize(int new_size) {
      auto old_size = transitions.size();
      transitions.resize(new_size);
      is_stop.resize(new_size, 0);
      for (auto i = old_size; i < transitions.size(); i++) transitions[i].fill(-1);
    }

    void SetTransition(int from, int to, int sym) {
      emp_assert(from >= 0 && from < (int) transitions.size());
      emp_assert(to >= 0 && to < (int) transitions.size());
      emp_assert(sym >= 0 && sym < NUM_SYMBOLS);
      transitions[from][sym] = to;
    }

    void SetStop(int state) { is_stop[state] = 1; }

    int Next(int state, int sym) const {
      emp_assert(state >= -1 && state < (int) transitions.size());
      emp_assert(sym >= 0 && sym < NUM_SYMBOLS);
      return (state == -1) ? -1 : transitions[state][sym];
    }

    int Next(int state, std::string sym_set) {
      for (char x : sym_set) state = Next(state, x);
      return state;
    }

    bool IsStop(int state) const { return is_stop[state]; }
  };

}

#endif
