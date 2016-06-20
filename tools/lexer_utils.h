//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A set of utilities to convert among RegEx, NFA, DFA, and fully lexers.

#ifndef EMP_LEXER_UTILS_H
#define EMP_LEXER_UTILS_H

#include <map>

#include "DFA.h"
#include "NFA.h"
#include "RegEx.h"

namespace emp {

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

      // Dtermine is this state should be a STOP state.
      for (int s : cur_state) if (nfa.IsStop(s)) { dfa.SetStop(cur_id); break; }

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

  static NFA to_NFA(const RegEx & regex) {
    NFA nfa(2);  // State 0 = start, state 1 = stop.
    nfa.SetStop(1);
    regex.AddToNFA(nfa, 0, 1);
    return nfa;
  }

  static DFA to_DFA(const RegEx & regex) {
    NFA nfa = to_NFA(regex);
    return to_DFA(nfa);
  }
}

#endif
