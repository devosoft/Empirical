/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2016-2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/compiler/lexer_utils.hpp
 * @brief A set of utilities to convert between NFAs and DFAs
 * @note Status: BETA
 */

#pragma once

#ifndef INCLUDE_EMP_COMPILER_LEXER_UTILS_HPP_GUARD
#define INCLUDE_EMP_COMPILER_LEXER_UTILS_HPP_GUARD

#include <map>
#include <stddef.h>
#include <utility>  // std::pair

#include "../base/vector.hpp"
#include "../bits/Bits.hpp"
#include "../tools/String.hpp"

#include "DFA.hpp"
#include "NFA.hpp"

namespace emp {

  /** @addtogroup <lexer_utilities>
   *  @{
   */

  /// Converting DFA to DFA -- no change needed.
  static inline const DFA & to_DFA(const DFA & dfa) { return dfa; }

  /// Converting NFA to MFA -- no change needed.
  static inline const NFA & to_NFA(const NFA & nfa) { return nfa; }

  /// Systematic conversion of NFA to DFA...
  static inline DFA to_DFA(const NFA & nfa, bool keep_invalid = false) {
    DFA dfa;
    dfa.AddState();
    std::map<DynamicBits, size_t> id_map;  // Map nfa "state sets" to dfa states.
    std::vector<DynamicBits> state_stack;  // States that still need to be explored.

    state_stack.emplace_back(nfa.GetStart());  // Place the starting state in the state_stack.
    id_map[state_stack[0]] = 0;                // Give starting point ID 0.

    // NFA states with ONLY free transitions can be removed from DFA state sets, since any
    // free transitions out of them will already have been followed.  (Computed once, here.)
    const DynamicBits non_empty_states = ~nfa.GetEmptyStates();

    // Loop through all states not fully explored; remove top state and add new states.
    while (state_stack.size()) {
      // Get the next state to test.
      const DynamicBits cur_state = state_stack.back();
      const size_t cur_id         = id_map.find(cur_state)->second;
      state_stack.pop_back();

      // Propagate stop values and trailing-context markers, always keeping the HIGHEST value.
      for (auto s : cur_state) {
        dfa.AddStop(cur_id, nfa.GetStop(s));        // Should this be a STOP state?
        dfa.AddTCStop(cur_id, nfa.GetTCStop(s));    // ...a trailing-context r1 boundary?
        dfa.AddTCFinal(cur_id, nfa.GetTCFinal(s));  // ...a trailing-context final accept?
      }

      // Account for all possible transitions
      for (size_t sym = 0; sym < NFA::NUM_SYMBOLS; sym++) {
        DynamicBits next_state = nfa.GetNext(sym, cur_state);
        if (next_state.None() && !keep_invalid) {
          continue;  // Discard invalid transitions.
        }

        // Remove NFA states with ONLY free transitions (they will all have been taken already)
        next_state &= non_empty_states;

        // If we need a new state in the DFA, add it and put it on the stack to explore.
        // (try_emplace does a single map lookup for both the test and the insertion.)
        auto [it, inserted] = id_map.try_emplace(next_state, 0);
        if (inserted) {
          it->second = dfa.AddState();
          state_stack.emplace_back(std::move(next_state));
        }

        // Set up the new connection in the DFA
        dfa.SetTransition(cur_id, it->second, sym);
      }
    }

    return dfa;
  }

  /// \deprecated Use the bool version of keep_invalid instead of an int.
  [[deprecated("to_DFA() now takes keep_invalid as a bool; pass true/false instead of an int.")]]
  static inline DFA to_DFA(const NFA & nfa, int keep_invalid) {
    return to_DFA(nfa, keep_invalid != 0);
  }

  /// Systematic up-conversion of DFA to NFA...
  static inline NFA to_NFA(const DFA & dfa) {
    NFA nfa(dfa.GetSize());
    for (size_t from = 0; from < dfa.GetSize(); from++) {
      const auto & t = dfa.GetTransitions(from);
      for (size_t sym = 0; sym < t.size(); sym++) {
        if (t[sym] == -1) { continue; }
        nfa.AddTransition(from, static_cast<size_t>(t[sym]), static_cast<char>(sym));
      }
      if (dfa.IsStop(from)) { nfa.SetStop(from, dfa.GetStop(from)); }
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
  static NFA MergeNFA(T1 && in1, T2 && in2, Ts &&... others) {
    NFA nfa_out(to_NFA(std::forward<T1>(in1)));    // Start out with nfa1.
    nfa_out.Merge(to_NFA(std::forward<T2>(in2)));  // Merge in nfa2;
    return MergeNFA(nfa_out, std::forward<Ts>(others)...);
  }

  /// Merge multiple automata (DFA, NFA, RegEx) into one DFA.
  template <typename T1, typename T2, typename... Ts>
  static DFA MergeDFA(T1 && in1, T2 && in2, Ts &&... others) {
    return to_DFA(
      MergeNFA(std::forward<T1>(in1), std::forward<T2>(in2), std::forward<Ts>(others)...));
  }

  /// Structure to track the current status of a DFA.
  struct DFAStatus {
    size_t state;
    emp::String sequence;

    DFAStatus(size_t _state, const emp::String & _seq) : state(_state), sequence(_seq) { ; }
  };

  /// Method to find an example string that satisfies a DFA.
  static inline emp::String FindExample(const DFA & dfa, const size_t min_size = 1) {
    emp::vector<DFAStatus> traverse_set;
    traverse_set.emplace_back(0, "");

    for (size_t next_id = 0; next_id < traverse_set.size(); ++next_id) {
      const emp::DFAStatus cur_status = traverse_set[next_id];  // pair: cur state and cur sequence
      const emp::array<int, 128> & t =
        dfa.GetTransitions(cur_status.state);  // Array of TO states (or -1 if none)
      // Ignore control symbols (line begin/end and other unprintable specials).
      for (size_t sym = DFA::SYMBOL_MIN_INPUT; sym < 128; sym++) {
        const int next_state = t[sym];
        if (next_state == -1) {
          continue;  // Ignore non-transitions
        }
        emp::String cur_str(cur_status.sequence);
        cur_str += (char) sym;  // Figure out current string
        // If this is a valid answer, return it.
        if (min_size <= cur_str.size() && dfa.IsStop(next_state)) { return cur_str; }
        traverse_set.emplace_back(next_state, cur_str);  // Continue searching from here.
      }
    }

    return "";
  }

  // Close Doxygen group
  /** @}*/
}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_COMPILER_LEXER_UTILS_HPP_GUARD
