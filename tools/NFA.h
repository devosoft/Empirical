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
  public:
    constexpr static int NUM_SYMBOLS = 128;
    using opts_t = BitSet<NUM_SYMBOLS>;

  private:
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
    emp::vector< char > is_stop;         // 0=no 1=yes (char instead of bool for speed)

  public:
    NFA(int num_states=0, int start_state=0)
      : states(num_states), start(start_state), is_stop(num_states, 0) {
        if (num_states > start) states[start].free_to.insert(start);
      }
    NFA(const NFA &) = default;
    ~NFA() { ; }

    int GetSize() const { return (int) states.size(); }
    const std::set<int> & GetStart() const {
      emp_assert(states.size() > start);
      return states[start].free_to;
    }
    std::set<int> GetNext(int sym, int from_id=0) const {
      std::set<int> to_set;
      for (auto & t : states[from_id].trans) {
        if (t.second.symbols[sym]) {
          to_set.insert(t.first);
          insert(to_set, states[t.first].free_to);
        }
      }
      return to_set;
    }
    std::set<int> GetNext(int sym, const std::set<int> from_set) const {
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

    opts_t GetSymbolOptions(const std::set<int> & test_set) const {
      opts_t options;
      for (int id : test_set) {
        for (const auto & t : states[id].trans) {
          options |= t.second.symbols;
        }
      }
      return options;
    }

    void Resize(int new_size) {
      states.resize(new_size);
      is_stop.resize(new_size, 0);
      if (new_size > start) states[start].free_to.insert(start);
    }
    int AddNewState() { int new_state = GetSize(); Resize(new_state+1); return new_state; }
    void AddTransition(int from, int to, int sym) {
      emp_assert(from >= 0 && from < (int) states.size(), from, states.size());
      emp_assert(to >= 0 && to < (int) states.size(), to, states.size());
      emp_assert(sym >= 0, sym);

      states[from].trans[to].symbols[sym] = true;
    }
    void AddTransition(int from, int to, const std::string & sym_set) {
      for (char x : sym_set) AddTransition(from, to, x);
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
      extend_to.insert(to);
      extend_from.insert(from);

      // Insert all combinations of where new moves can be coming from or going to.
      for (int x : extend_from) {
        for (int y : extend_to) {
          states[x].free_to.insert(y);
          states[y].free_from.insert(x);
        }
      }

    }

    void SetStop(int state) { is_stop[state] = 1; }
    bool IsStop(int state) const { return is_stop[state]; }

    void PrintFreeMoves() {
      for (int i = 0; i < (int) states.size(); i++) {
        // std::cout << "Free from ( ";
        for (int x : states[i].free_from) std::cout << x << " ";
        // std::cout << ") to " << i << std::endl;
        // std::cout << "Free from " << i << " to ( ";
        for (int x : states[i].free_to) std::cout << x << " ";
        // std::cout << ")" << std::endl;
      }
    }
  };

  class NFA_State {
  private:
    const NFA & nfa;
    std::set<int> state_set;
  public:
    NFA_State(const NFA & _nfa) : nfa(_nfa) { state_set = nfa.GetStart(); }
    ~NFA_State() { ; }

    const NFA & GetNFA() const { return nfa; }
    const std::set<int> & GetStateSet() const { return state_set; }

    bool IsActive() const { return state_set.size(); }
    bool IsStop() const {
      for (int s : state_set) if (nfa.IsStop(s)) return true;
      return false;
    }

    void SetStateSet(const std::set<int> & in) { state_set = in; }
    void Reset() { state_set = nfa.GetStart(); }

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
