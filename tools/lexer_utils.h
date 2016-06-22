//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A set of utilities to convert among RegEx, NFA, DFA, and fully lexers.
//
//  Available conversions:
//
//   static const DFA & to_DFA(const DFA & dfa)
//   static DFA to_DFA(const NFA & nfa, int keep_invalid=false)
//   static DFA to_DFA(const RegEx & regex)
//
//   static NFA to_NFA(const DFA & dfa)
//   static const NFA & to_NFA(const NFA & nfa)}
//   static NFA to_NFA(const RegEx & regex, int stop_id=1)



#ifndef EMP_LEXER_UTILS_H
#define EMP_LEXER_UTILS_H

#include <map>

#include "DFA.h"
#include "NFA.h"
#include "RegEx.h"

namespace emp {

  // Simple echo's
  static const DFA & to_DFA(const DFA & dfa) { return dfa; }
  static const NFA & to_NFA(const NFA & nfa) { return nfa; }

  // Systematic conversion of NFA to DFA...
  static DFA to_DFA(const NFA & nfa, int keep_invalid=false) {
    DFA dfa(1);                               // Setup zero to be the start state.
    std::map<std::set<int>, int> id_map;      // How do nfa state sets map to dfa states?
    std::vector<std::set<int>> state_stack;   // Which states still need to be explored?
    state_stack.emplace_back(nfa.GetStart()); // Place the starting point in the state_stack.
    id_map[state_stack[0]] = 0;               // Give starting point ID 0.

    // Loop through all states not full explored; remove top state and add new states.
    while (state_stack.size()) {
      // Get the next state to test.
      std::set<int> cur_state = state_stack.back();
      const int cur_id = id_map[cur_state];
      state_stack.pop_back();

      // Determine if this state should be a STOP state and always use HIGHEST stop value.
      for (int s : cur_state) dfa.AddStop(cur_id, nfa.GetStop(s));

      // Run through all possible transitions
      for (int sym = 0; sym < NFA::NUM_SYMBOLS; sym++) {
        std::set<int> next_state = nfa.GetNext(sym, cur_state);
        if (next_state.size() == 0 && !keep_invalid) continue;  // Discard invalid transitions.

        // Remove NFA states with ONLY free transisions (they will all have been taken already)
        // @CAO do more elegantly!
        emp::vector<int> remove_set;
        for (int x : next_state) if (nfa.IsEmpty(x)) remove_set.push_back(x);
        for (int x : remove_set) next_state.erase(x);

        // Determine if we have a new state in the DFA.
        if (id_map.find(next_state) == id_map.end()) {
          const int next_id = dfa.GetSize();
          id_map[next_state] = next_id;
          dfa.Resize(next_id + 1);
          state_stack.emplace_back(next_state);
        }

        // Setup the new connection in the DFA
        const int next_id = id_map[next_state];
        dfa.SetTransition(cur_id, next_id, sym);
      }

    }

    return dfa;
  }

  // Systematic up-conversion of DFA to NFA...
  static NFA to_NFA(const DFA & dfa) {
    NFA nfa(dfa.GetSize());
    for (int from = 0; from < dfa.GetSize(); from++) {
      auto t = dfa.GetTransitions(from);
      for (int sym = 0; sym < (int) t.size(); sym++) {
        if (t[sym] == -1) continue;
        nfa.AddTransition(from, t[sym], sym);
      }
      if (dfa.IsStop(from)) nfa.SetStop(from, dfa.GetStop(from));
    }
    return nfa;
  }

  // Simple conversion of RegEx to NFA (mostly implemented in RegEx)
  static NFA to_NFA(const RegEx & regex, int stop_id=1) {
    NFA nfa(2);  // State 0 = start, state 1 = stop.
    nfa.SetStop(1, stop_id);
    regex.AddToNFA(nfa, 0, 1);
    return nfa;
  }

  // Conversion of RegEx to DFA, via NFA intermediate.
  static DFA to_DFA(const RegEx & regex) {
    return to_DFA( to_NFA(regex) );
  }

  // Merge two NFAs into one.
  template <typename... T>
  static NFA to_NFA(const NFA & nfa1, const NFA & nfa2, T &&... others ) {
    NFA nfa_out(nfa1);   // Start out identical to nfa1.
    nfa_out.Merge(nfa2); // Merge in nfa2;
    return to_NFA(nfa_out, std::forward<T>(others)...);
  }
}

#endif
