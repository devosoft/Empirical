//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A Deterministic Finite Automata simulator
//
//  Member functions:
//   int GetSize() const
//     How many states is this DFA using?
//
//   void Resize(int new_size)
//     Add Additional empty states.
//
//   const emp::array<int, NUM_SYMBOLS> & GetTransitions(int from) const
//     Return an array of all transitions associated with a specified state.
//
//   void SetTransition(int from, int to, int sym) {
//     Add a specific transition associated with an input symbol.
//
//   void SetStop(int state, stop_t stop_val=1)
//   void AddStop(int state, stop_t stop_val=1)
//     Either set the stop value or add an additional stop value (only max stop is returned).
//
//   bool IsStop(int state) const
//   stop_t GetStop(int state) const
//     Test if a state has a stop or get the stop value associated with that state.
//
//   bool IsActive(int state) const
//     Test if a state is still valid.
//
//   int Next(int state, int sym) const
//   int Next(int state, std::string sym_set) const
//     Return the new state after a single symbol or a string of symbols.
//
//   void Print()
//     Output the state of the DFA. (still mostly debug)
//
//  Details for number of possible input symbols and stop type can be specified with tDFA, but:
//   using DFA = tDFA<128, char>;

#ifndef EMP_DFA_H
#define EMP_DFA_H

#include <string>

#include "array.h"
#include "string_utils.h"
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

    // Function to set the stop value (no matter what it currently is)
    void SetStop(int state, stop_t stop_val=1) {
      emp_assert(state >= -1 && state < (int) transitions.size());
      is_stop[state] = stop_val;
    }

    // Function to set the stop value only if it's higher than the current stop value.
    void AddStop(int state, stop_t stop_val=1) {
      emp_assert(state >= -1 && state < (int) transitions.size());
      // If we are given a stop value and its higher than our previous stop, use it!
      if (stop_val > is_stop[state]) is_stop[state] = stop_val;
    }

    stop_t GetStop(int state) const { return (state == -1) ? 0 : is_stop[state]; }
    bool IsActive(int state) const { return state != -1; }
    bool IsStop(int state) const { return (state == -1) ? false : is_stop[state]; }

    int Next(int state, int sym) const {
      emp_assert(state >= -1 && state < (int) transitions.size());
      // emp_assert(sym >= 0 && sym < NUM_SYMBOLS, sym, (char) sym);
      return (state < 0 || sym >= NUM_SYMBOLS) ? -1 : transitions[state][sym];
    }

    int Next(int state, std::string sym_set) const {
      for (char x : sym_set) state = Next(state, x);
      return state;
    }

    stop_t Test(const std::string & str) const {
      int out = Next(0, str);
      return GetStop(out);
    }

    void Print(std::ostream & os=std::cout) {
      os << "Num states = " << GetSize() << std::endl << "Stop IDs:";
      for (int i = 0; i < GetSize(); i++) if(IsStop(i)) os << " " << i;
      os << std::endl;

      for (int i = 0; i < (int) transitions.size(); i++) {
        os << " " << i << " ->";
        for (int s = 0; s < NUM_SYMBOLS; s++) {
          if (transitions[i][s] == -1) continue;
          os << " " << to_literal((char) s) << ":" << transitions[i][s];
        }
        if (IsStop(i)) os << " [STOP=" << ((int) GetStop(i)) << "]";
        os << std::endl;
      }

    }

  };

  using DFA = tDFA<128, uint8_t>;

}

#endif
