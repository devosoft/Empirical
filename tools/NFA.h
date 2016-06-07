//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A Non-deterministic Finite Automata simulator

#ifndef EMP_NFA_H
#define EMP_NFA_H

#include <map>
#include <set>

#include "BitSet.h"
#include "vector.h"

namespace emp {

  class NFA {
  private:
    constexpr static int NUM_SYMBOLS = 128;
    using opts_t = BitSet<NUM_SYMBOLS>;

    struct Transition {
      opts_t symbols;
    };
    struct State {
      std::map<int, Transition> trans;
    };

    emp::vector<State> states;
    int start;
  public:
    NFA(int num_states=0, int start_state=0) : states(num_states), start(start_state) { ; }
    ~NFA() { ; }

    int GetStart() const { return start; }
    std::set<int> GetNext(char sym, int from_id=0) {
      std::set<int> to_set;
      for (auto & t : states[from_id].trans) {
        if (t.second.symbols[sym]) to_set.insert(t.first);
      }
      return to_set;
    }
    std::set<int> GetNext(char sym, const std::set<int> from_set) {
      std::set<int> to_set;
      for (int from_id : from_set) {
        for (auto & t : states[from_id].trans) {
          if (t.second.symbols[sym]) to_set.insert(t.first);
        }
      }
      return to_set;
    }

    void Resize(int new_size) { states.resize(new_size); }
    void AddTransition(int from, int to, char sym) {
      emp_assert(from >= 0 && from < (int) states.size(), from, states.size());
      emp_assert(to >= 0 && to < (int) states.size(), to, states.size());
      emp_assert(sym >= 0, sym);

      states[from].trans[to].symbols[sym] = true;
    }
    void AddTransition(int from, int to, const BitSet<NUM_SYMBOLS> & sym_set) {
      emp_assert(from >= 0 && from < (int) states.size(), from, states.size());
      emp_assert(to >= 0 && to < (int) states.size(), to, states.size());

      states[from].trans[to].symbols |= sym_set;
    }

  };

  class NFA_State {
  private:
    NFA & nfa;
    std::set<int> state;
  public:
    NFA_State(NFA & _nfa) : nfa(_nfa) { state.insert(nfa.GetStart()); }
    ~NFA_State() { ; }
  };

}

#endif
