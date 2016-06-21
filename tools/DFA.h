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

  template <int NUM_SYMBOLS=128, typename STOP_TYPE=uint8_t>
  class tDFA {
  private:
    emp::vector< emp::array<int, NUM_SYMBOLS> > transitions;
    emp::vector< STOP_TYPE > is_stop;  // 0=not stop; other values for STOP return value.
  public:
    tDFA(int num_states=0) : transitions(num_states), is_stop(num_states, 0) {
      for (auto & t : transitions) t.fill(-1);
    }
    tDFA(const tDFA<NUM_SYMBOLS, STOP_TYPE> &) = default;
    ~tDFA() { ; }
    tDFA<NUM_SYMBOLS, STOP_TYPE> & operator=(const tDFA<NUM_SYMBOLS, STOP_TYPE> &) = default;

    using stop_t = STOP_TYPE;

    int GetSize() const { return (int) transitions.size(); }

    void Resize(int new_size) {
      auto old_size = transitions.size();
      transitions.resize(new_size);
      is_stop.resize(new_size, 0);
      for (auto i = old_size; i < transitions.size(); i++) transitions[i].fill(-1);
    }

    const emp::array<int, NUM_SYMBOLS> & GetTransitions(int from) const { return transitions[from]; }

    void SetTransition(int from, int to, int sym) {
      emp_assert(from >= 0 && from < (int) transitions.size());
      emp_assert(to >= 0 && to < (int) transitions.size());
      emp_assert(sym >= 0 && sym < NUM_SYMBOLS);
      transitions[from][sym] = to;
    }

    void SetStop(int state, stop_t stop_val=1) {
      emp_assert(state >= -1 && state < (int) transitions.size());
      is_stop[state] = stop_val;
    }
    stop_t GetStop(int state) const { return (state == -1) ? 0 : is_stop[state]; }
    bool IsActive(int state) const { return state != -1; }
    bool IsStop(int state) const { return (state == -1) ? false : is_stop[state]; }

    int Next(int state, int sym) const {
      emp_assert(state >= -1 && state < (int) transitions.size());
      emp_assert(sym >= 0 && sym < NUM_SYMBOLS);
      return (state == -1) ? -1 : transitions[state][sym];
    }

    int Next(int state, std::string sym_set) const {
      for (char x : sym_set) state = Next(state, x);
      return state;
    }

    void Print() {
      std::cout << "Num states = " << GetSize() << std::endl;
      std::cout << "Start = 0; Stop =";
      for (int i = 0; i < GetSize(); i++) if(IsStop(i)) std::cout << " " << i;
      std::cout << std::endl;

      for (int i = 0; i < (int) transitions.size(); i++) {
        std::cout << " " << i << " ->";
        for (int s = 0; s < NUM_SYMBOLS; s++) {
          if (transitions[i][s] == -1) continue;
          std::cout << " " << ((char) s) << ":" << transitions[i][s];
        }
        std::cout << std::endl;
      }

    }

  };

  using DFA = tDFA<128, char>;

}

#endif
