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
#include "set_utils.h"
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
      std::map<int, Transition> trans;   // What symbol transitions are available?
      std::set<int> free_to;             // What other states can you move to for free?
      std::set<int> free_from;           // What other states can move here for free?
    };

    emp::vector<State> states;
    int start;
  public:
    NFA(int num_states=0, int start_state=0) : states(num_states), start(start_state) { ; }
    ~NFA() { ; }

    int GetStart() const { return start; }
    std::set<int> GetNext(int sym, int from_id=0) {
      std::set<int> to_set;
      for (auto & t : states[from_id].trans) {
        if (t.second.symbols[sym]) {
          to_set.insert(t.first);
          insert(to_set, states[t.first].free_to);
        }
      }
      return to_set;
    }
    std::set<int> GetNext(int sym, const std::set<int> from_set) {
      std::set<int> to_set;
      for (int from_id : from_set) {
        for (auto & t : states[from_id].trans) {
          if (t.second.symbols[sym]) {
            to_set.insert(t.first);
            insert(to_set, states[t.first].free_to);
          }
        }
      }
      return to_set;
    }

    void Resize(int new_size) { states.resize(new_size); }
    void AddTransition(int from, int to, int sym) {
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
    void AddFreeTransition(int from, int to) {
      emp_assert(from >= 0 && from < (int) states.size(), from, states.size());
      emp_assert(to >= 0 && to < (int) states.size(), to, states.size());

      // Keep track of where free transitions could have come from and can continue to.
      auto extend_to = states[to].free_to;
      auto extend_from = states[from].free_from;

      // Update the immediate free moves with the new transition.
      states[from].free_to.insert(to);
      states[to].free_from.insert(from);

      // from can get to all other free moves from to (and vice versa)
      states[from].free_to.insert(extend_to.begin(), extend_to.end());
      states[to].free_from.insert(extend_from.begin(), extend_from.end());

      // everything that can get to from can now get to everything to can get to (and vv).
      for (int x : extend_from) states[x].free_to.insert(extend_to.begin(), extend_to.end());
      for (int x : extend_to) states[x].free_from.insert(extend_from.begin(), extend_from.end());
    }

  };

  class NFA_State {
  private:
    NFA & nfa;
    std::set<int> state_set;
  public:
    NFA_State(NFA & _nfa) : nfa(_nfa) { state_set.insert(nfa.GetStart()); }
    ~NFA_State() { ; }

    const NFA & GetNFA() { return nfa; }
    const std::set<int> & GetStateSet() const { return state_set; }

    void SetStateSet(const std::set<int> & in) { state_set = in; }

    void Next(int sym) {
      state_set = nfa.GetNext(sym, state_set);
    }

    void Next(const std::string & sym_set) {
      for (char x : sym_set) Next(x);
    }

    void Print() {
      std::cout << "cur states:";
      for (int s : state_set) {
        std::cout << " " << s;
      }
      std::cout << std::endl;
    }
  };

}

#endif
