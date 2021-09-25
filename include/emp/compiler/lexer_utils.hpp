/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2017
 *
 *  @file lexer_utils.hpp
 *  @brief A set of utilities to convert between NFAs and DFAs
 *  @note Status: BETA
 */

#ifndef EMP_COMPILER_LEXER_UTILS_HPP_INCLUDE
#define EMP_COMPILER_LEXER_UTILS_HPP_INCLUDE


#include <map>
#include <utility> // std::pair

#include "../base/vector.hpp"
#include "../bits/BitVector.hpp"

#include "DFA.hpp"
#include "NFA.hpp"

namespace emp {

  /// Converting DFA to DFA -- no change needed.
  static inline const DFA & to_DFA(const DFA & dfa) { return dfa; }

  /// Converting NFA to MFA -- no change needed.
  static inline const NFA & to_NFA(const NFA & nfa) { return nfa; }

  /// Systematic conversion of NFA to DFA...
  static inline DFA to_DFA(const NFA & nfa, int keep_invalid=false) {
    DFA dfa(1);                                  // Setup zero to be the start state.
    std::map<std::set<size_t>, size_t> id_map;   // How do nfa state sets map to dfa states?
    std::vector<std::set<size_t>> state_stack;   // Which states still need to be explored?
    state_stack.emplace_back(nfa.GetStart());    // Place the starting point in the state_stack.
    id_map[state_stack[0]] = 0;                  // Give starting point ID 0.

    // Loop through all states not full explored; remove top state and add new states.
    while (state_stack.size()) {
      // Get the next state to test.
      std::set<size_t> cur_state = state_stack.back();
      const size_t cur_id = id_map[cur_state];
      state_stack.pop_back();

      // Determine if this state should be a STOP state and always use HIGHEST stop value.
      for (auto s : cur_state) dfa.AddStop(cur_id, nfa.GetStop(s));

      // Run through all possible transitions
      for (size_t sym = 0; sym < NFA::NUM_SYMBOLS; sym++) {
        std::set<size_t> next_state = nfa.GetNext(sym, cur_state);
        if (next_state.size() == 0 && !keep_invalid) continue;  // Discard invalid transitions.

        // Remove NFA states with ONLY free transisions (they will all have been taken already)
        // @CAO do more elegantly!
        emp::vector<size_t> remove_set;
        for (auto x : next_state) if (nfa.IsEmpty(x)) remove_set.push_back(x);
        for (auto x : remove_set) next_state.erase(x);

        // Determine if we have a new state in the DFA.
        if (id_map.find(next_state) == id_map.end()) {
          const size_t next_id = dfa.GetSize();
          id_map[next_state] = next_id;
          dfa.Resize(next_id + 1);
          state_stack.emplace_back(next_state);
        }

        // Setup the new connection in the DFA
        const size_t next_id = id_map[next_state];
        dfa.SetTransition(cur_id, next_id, sym);
      }

    }

    return dfa;
  }

  /// Systematic up-conversion of DFA to NFA...
  static inline NFA to_NFA(const DFA & dfa) {
    NFA nfa(dfa.GetSize());
    for (size_t from = 0; from < dfa.GetSize(); from++) {
      const auto & t = dfa.GetTransitions(from);
      for (size_t sym = 0; sym < t.size(); sym++) {
        if (t[sym] == -1) continue;
        nfa.AddTransition(from, (size_t) t[sym], sym);
      }
      if (dfa.IsStop(from)) nfa.SetStop(from, dfa.GetStop(from));
    }
    return nfa;
  }


  /// Merge multiple automata into one NFA (base case, single converstion)
  template <typename T1>
  static NFA MergeNFA(T1 && in) {
    return to_NFA(std::forward<T1>(in));
  }

  /// Merge multiple automata (DFA, NFA, RegEx) into one NFA.
  template <typename T1, typename T2, typename... Ts>
  static NFA MergeNFA(T1 && in1, T2 && in2, Ts &&... others ) {
    NFA nfa_out( to_NFA(std::forward<T1>(in1)) );   // Start out identical to nfa1.
    nfa_out.Merge( to_NFA(std::forward<T2>(in2)) ); // Merge in nfa2;
    return MergeNFA(nfa_out, std::forward<Ts>(others)...);
  }


  /// Merge multiple automata (DFA, NFA, RegEx) into one DFA.
  template <typename T1, typename T2, typename... Ts>
  static DFA MergeDFA(T1 && in1, T2 && in2, Ts &&... others ) {
    return to_DFA( MergeNFA(in1, in2, others...) );
  }

  /// Structure to track the current status of a DFA.
  struct DFAStatus {
    size_t state;
    std::string sequence;
    DFAStatus(size_t _state, const std::string & _seq) : state(_state), sequence(_seq) { ; }
  };

  /// Method to find an example string that satisfies a DFA.
  std::string FindExample(const DFA & dfa, const size_t min_size=1) {
    emp::vector< DFAStatus > traverse_set;
    traverse_set.emplace_back(0, "");
    // BitVector state_found(dfa.GetSize());

    size_t next_id = 0;
    while (next_id < traverse_set.size()) {
      const auto cur_status = traverse_set[next_id++];       // pair: cur state and cur string
      const auto & t = dfa.GetTransitions(cur_status.state); // int array of TO states (or -1 if none)
      for (size_t sym = 0; sym < t.size(); sym++) {
        const int next_state = t[sym];
        if (next_state == -1) continue;                      // Ignore non-transitions
        std::string cur_str(cur_status.sequence);
        cur_str += (char) sym;                               // Figure out current string
        if (min_size <= cur_str.size() ) {             // If the DFA is big enough...
          if (dfa.IsStop(next_state)) return cur_str;        //  return if this is a legal answer
        }
        traverse_set.emplace_back(next_state, cur_str);      // Continue searching from here.
      }
    }

    return "";
  }
}

#endif // #ifndef EMP_COMPILER_LEXER_UTILS_HPP_INCLUDE
