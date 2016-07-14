//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A set of utilities to convert between NFAs and DFAs
//
//  Available conversions:
//
//   static const DFA & to_DFA(const DFA & dfa)
//   static DFA to_DFA(const NFA & nfa, int keep_invalid=false)
//
//   static NFA to_NFA(const DFA & dfa)
//   static const NFA & to_NFA(const NFA & nfa)}
//
//   (additional to_NFA and to_DFA functions are defined in RegEx.h)
//
//  Available merges:
//
//   static NFA MergeNFA( ... )  - Convert two or more (DFA, NFA or RegEx) to a single NFA.
//   static DFA MergeDFA( ... )  - Convert two or more (DFA, NFA or RegEx) to a single DFA.


#ifndef EMP_LEXER_UTILS_H
#define EMP_LEXER_UTILS_H

#include <map>
#include <utility> // std::pair

#include "BitVector.h"
#include "DFA.h"
#include "NFA.h"
#include "vector.h"

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
      const auto & t = dfa.GetTransitions(from);
      for (int sym = 0; sym < (int) t.size(); sym++) {
        if (t[sym] == -1) continue;
        nfa.AddTransition(from, t[sym], sym);
      }
      if (dfa.IsStop(from)) nfa.SetStop(from, dfa.GetStop(from));
    }
    return nfa;
  }


  // Merge multiple NFAs into one.
  template <typename T1>
  static NFA MergeNFA(T1 && in) {
    return to_NFA(std::forward<T1>(in));
  }

  template <typename T1, typename T2, typename... Ts>
  static NFA MergeNFA(T1 && in1, T2 && in2, Ts &&... others ) {
    NFA nfa_out( to_NFA(std::forward<T1>(in1)) );   // Start out identical to nfa1.
    nfa_out.Merge( to_NFA(std::forward<T2>(in2)) ); // Merge in nfa2;
    return MergeNFA(nfa_out, std::forward<Ts>(others)...);
  }


  template <typename T1, typename T2, typename... Ts>
  static DFA MergeDFA(T1 && in1, T2 && in2, Ts &&... others ) {
    return to_DFA( MergeNFA(in1, in2, others...) );
  }


  struct DFAStatus {
    int state;
    std::string sequence;
    DFAStatus(int _state, const std::string & _seq) : state(_state), sequence(_seq) { ; }
  };

  // Method to find an example string that satisfies a DFA.
  std::string FindExample(const DFA & dfa, const int min_size=1) {
    emp::vector< DFAStatus > traverse_set;
    traverse_set.emplace_back(0, "");
    // BitVector state_found(dfa.GetSize());

    int next_id = 0;
    while (next_id < traverse_set.size()) {
      const auto cur_status = traverse_set[next_id++];     // pair: cur state and cur string
      const auto & t = dfa.GetTransitions(cur_status.state); // int array of TO states (or -1 if none)
      for (int sym = 0; sym < (int) t.size(); sym++) {
        const int next_state = t[sym];
        if (next_state == -1) continue;                     // Ignore non-transitions
        std::string cur_str(cur_status.sequence);
        cur_str += (char) sym;                              // Figure out current string
        if (cur_str.size() >= min_size) {                   // If the DFA is big enough...
          // if (state_found[next_state]) continue;            //  skip if we've already made it here
          if (dfa.IsStop(next_state)) return cur_str;       //  return if this is a legal answer
          // state_found[next_state] = true;                   //  else, don't come again.
        }
        traverse_set.emplace_back(next_state, cur_str);     // Continue searching from here.
      }
    }

    return "";
  }
}

#endif
