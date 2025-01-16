/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2024.
 *
 *  @file lexer_utils.hpp
 *  @brief A set of utilities to convert between NFAs and DFAs
 *  @note Status: BETA
 */

#ifndef EMP_COMPILER_LEXER_UTILS_HPP_INCLUDE
#define EMP_COMPILER_LEXER_UTILS_HPP_INCLUDE


#include <map>
#include <stddef.h>
#include <utility> // std::pair

#include "../base/vector.hpp"
#include "../bits/Bits.hpp"
#include "../tools/String.hpp"

#include "DFA.hpp"
#include "NFA.hpp"

namespace emp {

  /** \addtogroup <lexer_utilities>
  *  @{
  */

  /// Converting DFA to DFA -- no change needed.
  static inline const DFA & to_DFA(const DFA & dfa) { return dfa; }

  /// Converting NFA to MFA -- no change needed.
  static inline const NFA & to_NFA(const NFA & nfa) { return nfa; }

  /// Systematic conversion of NFA to DFA...
  static inline DFA to_DFA(const NFA & nfa, int keep_invalid=false) {
    DFA dfa;
    dfa.AddState();
    std::map<DynamicBits, size_t> id_map;   // Map nfa "state sets" to dfa states.
    std::vector<DynamicBits> state_stack;   // States that still need to be explored.

    state_stack.emplace_back(nfa.GetStart());    // Place the starting state in the state_stack.
    id_map[state_stack[0]] = 0;                  // Give starting point ID 0.

    // Loop through all states not full explored; remove top state and add new states.
    while (state_stack.size()) {
      // Get the next state to test.
      const DynamicBits cur_state = state_stack.back();
      const size_t cur_id = id_map[cur_state];
      state_stack.pop_back();

      // Determine if this state should be a STOP state and always use HIGHEST stop value.
      for (auto s : cur_state) dfa.AddStop(cur_id, nfa.GetStop(s));

      // Account for all possible transitions
      DynamicBits non_empty_states = ~nfa.GetEmptyStates();
      for (size_t sym = 0; sym < NFA::NUM_SYMBOLS; sym++) {
        DynamicBits next_state = nfa.GetNext(sym, cur_state);
        if (next_state.None() && !keep_invalid) continue;  // Discard invalid transitions.

        // Remove NFA states with ONLY free transitions (they will all have been taken already)
        next_state &= non_empty_states;

        // If we need a new state in the DFA, add it and put it on the stack to explore.
        if (!id_map.contains(next_state)) {
          id_map[next_state] = dfa.AddState();
          state_stack.emplace_back(next_state);
        }

        // Set up the new connection in the DFA
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
        nfa.AddTransition(from, static_cast<size_t>(t[sym]), static_cast<char>(sym));
      }
      if (dfa.IsStop(from)) nfa.SetStop(from, dfa.GetStop(from));
    }
    return nfa;
  }


  /// Merge multiple automata into one NFA (base case; single conversion)
  template <typename T1>
  static NFA MergeNFA(T1 && in) {
    return to_NFA(std::forward<T1>(in));
  }

  /// Merge multiple automata (DFA, NFA, RegEx) into one NFA.
  template <typename T1, typename T2, typename... Ts>
  static NFA MergeNFA(T1 && in1, T2 && in2, Ts &&... others ) {
    NFA nfa_out( to_NFA(std::forward<T1>(in1)) );   // Start out with nfa1.
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
    emp::String sequence;
    DFAStatus(size_t _state, const emp::String & _seq) : state(_state), sequence(_seq) { ; }
  };

  /// Method to find an example string that satisfies a DFA.
  emp::String FindExample(const DFA & dfa, const size_t min_size=1) {
    emp::vector< DFAStatus > traverse_set;
    traverse_set.emplace_back(0, "");

    for (size_t next_id = 0; next_id < traverse_set.size(); ++next_id) {
      const emp::DFAStatus cur_status = traverse_set[next_id];  // pair: cur state and cur sequence
      const emp::array<int,128> & t = dfa.GetTransitions(cur_status.state); // Array of TO states (or -1 if none)
      // Ignore symbols 0 through 8 since they are special characters and unprintable (plus beginning/end symbols)
      for (size_t sym = 9; sym < 128; sym++) {
        const int next_state = t[sym];
        if (next_state == -1) continue;                      // Ignore non-transitions
        emp::String cur_str(cur_status.sequence);
        cur_str += (char) sym;                               // Figure out current string
        // If this is a valid answer, return it.
        if (min_size <= cur_str.size() && dfa.IsStop(next_state)) return cur_str;
        traverse_set.emplace_back(next_state, cur_str);      // Continue searching from here.
      }
    }

    return "";
  }

  // Close Doxygen group
  /** @}*/
}

#endif // #ifndef EMP_COMPILER_LEXER_UTILS_HPP_INCLUDE
