/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2017
 *
 *  @file NFA.hpp
 *  @brief A Non-deterministic Finite Automata simulator
 *  @note Status: BETA
 *
 *  To build a standard NFA, use emp::NFA.  If you want to have more symbols or more stop states,
 *  use emp::tNFA<S,T> where S is the number of symbols and T is the type used for stop.
 *  (defaults are 128 for ASCII-128 and uint8_t respectively.)
 *
 *  The constructor can take as parameters the number of states and the id of the start state (both
 *  default to 0)
 *
 *  @note DFA's use SetTransition(), but NFA's use AddTransition.  This distinction is intentional
 *        since in a DFA a second SetTransition with the same start state and symbol will override
 *        first, while in an NFA a second AddTransition will always add a new option.
 */

#ifndef EMP_COMPILER_NFA_HPP_INCLUDE
#define EMP_COMPILER_NFA_HPP_INCLUDE


#include <map>
#include <set>

#include "../base/vector.hpp"
#include "../bits/BitSet.hpp"
#include "../datastructs/set_utils.hpp"

namespace emp {

  /// A dynamic NFA class, for easily building non-determanistic finite automata.
  template <size_t S=128, typename STOP_TYPE=uint8_t>
  class tNFA {
  public:
    static const constexpr size_t NUM_SYMBOLS = S;
    using opts_t = BitSet<NUM_SYMBOLS>;
    using stop_t = STOP_TYPE;

  private:
    struct Transition {
      opts_t symbols;
      Transition() : symbols() { }
    };
    struct State {
      std::map<size_t, Transition> trans;   ///< What symbol transitions are available?
      std::set<size_t> free_to;             ///< What other states can you move to for free?
      std::set<size_t> free_from;           ///< What other states can move here for free?
      State() : trans(), free_to(), free_from() { }
    };

    emp::vector<State> states;           ///< Information about available states.
    size_t start;                        ///< Main start state (others might be reached for free.)
    emp::vector< STOP_TYPE > is_stop;    ///< 0=no 1=yes (char instead of bool for speed)

  public:
    tNFA(size_t num_states=1, size_t start_state=0)
      : states(num_states), start(start_state), is_stop(num_states, 0) {
        if (num_states > start) states[start].free_to.insert(start);
      }
    tNFA(const tNFA<S,STOP_TYPE> &) = default;
    ~tNFA() { ; }
    tNFA<S,STOP_TYPE> & operator=(const tNFA<S,STOP_TYPE> &) = default;

    /// Return the current number of states.
    size_t GetSize() const { return states.size(); }

    /// Return start state and all others reachable through empty transitions.
    const std::set<size_t> & GetStart() const {
      emp_assert(start < states.size());
      return states[start].free_to;
    }

    /// Return the states reachable from the current state given the provided symbol.
    std::set<size_t> GetNext(size_t sym, size_t from_id=0) const {
      std::set<size_t> to_set;
      for (auto & t : states[from_id].trans) {
        if (t.second.symbols[sym]) {
          to_set.insert(t.first);
          insert(to_set, states[t.first].free_to);
        }
      }
      return to_set;
    }

    /// return the states reachable from the current set of states given the provided symbol.
    std::set<size_t> GetNext(size_t sym, const std::set<size_t> from_set) const {
      std::set<size_t> to_set;
      for (size_t from_id : from_set) {
        for (auto & t : states[from_id].trans) {
          if (t.second.symbols[sym]) {
            to_set.insert(t.first);
            insert(to_set, states[t.first].free_to);
          }
        }
      }
      return to_set;
    }

    /// Does the provided state have free transitions?
    bool HasFreeTransitions(size_t id) const { return states[id].free_to.size(); }

    /// Does the provided state have symbol-transitions?
    bool HasSymTransitions(size_t id) const { return states[id].trans.size(); }

    /// Return an emp::BitSet indicating the symbols available from the provided set of states.
    opts_t GetSymbolOptions(const std::set<size_t> & test_set) const {
      opts_t options;
      for (size_t id : test_set) {
        for (const auto & t : states[id].trans) {
          options |= t.second.symbols;
        }
      }
      return options;
    }

    /// Change the number of available states.
    void Resize(size_t new_size) {
      states.resize(new_size);
      is_stop.resize(new_size, 0);
      if (new_size > start) states[start].free_to.insert(start);
    }

    /// Add a new state into the NFA and return its id.
    size_t AddNewState() { size_t new_state = GetSize(); Resize(new_state+1); return new_state; }

    /// Add a transition between states 'from' and 'to' that can be taken with the provided symbol.
    void AddTransitionSymbol(size_t from, size_t to, size_t sym) {
      emp_assert(from < states.size(), from, states.size());
      emp_assert(to < states.size(), to, states.size());

      states[from].trans[to].symbols[sym] = true;
    }

    /// Add a transition between states 'from' and 'to' that can be taken with a char symbol.
    void AddTransition(size_t from, size_t to, const char sym) {
      AddTransitionSymbol(from, to, sym);
    }

    /// Add a transition between states 'from' and 'to' that can be taken with the provided symbols.
    void AddTransition(size_t from, size_t to, const std::string & sym_set) {
      for (char x : sym_set) AddTransitionSymbol(from, to, (size_t) x);
    }

    /// Add a transition between states 'from' and 'to' that can be taken with the provided symbols.
    void AddTransition(size_t from, size_t to, const char * sym_set) {
      AddTransition(from, to, std::string(sym_set));
    }

    /// Add a transition between states 'from' and 'to' that can be taken with the provided symbols.
    void AddTransition(size_t from, size_t to, const BitSet<NUM_SYMBOLS> & sym_set) {
      emp_assert(from < states.size(), from, states.size());
      emp_assert(to < states.size(), to, states.size());

      states[from].trans[to].symbols |= sym_set;
    }

    /// Create a free transition between 'from' and 'to'.
    void AddFreeTransition(size_t from, size_t to) {
      emp_assert(from < states.size(), from, states.size());
      emp_assert(to < states.size(), to, states.size());

      // Keep track of where free transitions could have come from and can continue to.
      auto extend_to = states[to].free_to;
      auto extend_from = states[from].free_from;
      extend_to.insert(to);
      extend_from.insert(from);

      // Insert all combinations of where new moves can be coming from or going to.
      for (auto x : extend_from) {
        for (auto y : extend_to) {
          states[x].free_to.insert(y);
          states[y].free_from.insert(x);
        }
      }

    }

    /// Set the specified state to be a stop state (with an optional stop value.)
    template <typename T=stop_t>
    void SetStop(size_t state, T stop_val=1) { is_stop[state] = (stop_t) stop_val; }

    /// Get any stop value associated with the provided state.
    stop_t GetStop(size_t state) const { return is_stop[state]; }

    /// Test if NFA begins at provided state (may have free transitions to other states)
    bool IsStart(size_t state) const { return state == start; }

    ///  Test if this state is a legal endpoint for the NFA.
    bool IsStop(size_t state) const { return is_stop[state]; }

    ///  Test if this state has only empty transitions from it, and not stop state.
    bool IsEmpty(size_t state) const { return !HasSymTransitions(state) && !IsStop(state); }

    /// Merge another NFA into this one.
    void Merge(const tNFA<NUM_SYMBOLS,STOP_TYPE> & nfa2) {
      const size_t offset = GetSize();                  // How far should we offset new NFA states?
      const size_t new_start = offset + nfa2.GetSize(); // Locate the new start node.
      Resize(offset + nfa2.GetSize() + 1);              // Make room for new NFA states + new start.
      AddFreeTransition(new_start, start);              // Free transition from new start to
      AddFreeTransition(new_start, nfa2.start+offset);  //    starts of both original NFAs.
      start = new_start;                                // Set the new start node.

      // Loop through new states and add them in.
      for (size_t i = 0; i < nfa2.GetSize(); i++) {
        // Move transitions.
        for (const auto & t : nfa2.states[i].trans) {
          AddTransition(i+offset, t.first+offset, t.second.symbols);
        }
        for (auto to : nfa2.states[i].free_to) {
          AddFreeTransition(i+offset, to+offset);
        }
        SetStop(i+offset, nfa2.is_stop[i]);   // Maintain the stop value for this state.
      }
    }

    /// Print information about this NFA (for debugging)
    void Print() const {
      std::cout << states.size() << " States:" << std::endl;
      for (size_t i = 0; i < states.size(); i++) {
        std::cout << " state " << i << " - ";
        for (const auto & t : states[i].trans) {
          std::cout << "(";
          for (size_t s = 0; s < NUM_SYMBOLS; s++) if (t.second.symbols[s]) std::cout << ((char) s);
          std::cout << "):" << t.first << " ";
        }
        if (states[i].free_to.size()) {
          std::cout << "free to:";
          for (auto f : states[i].free_to) std::cout << " " << f;
        }
        if (IsStop(i)) std::cout << " STOP(" << (int) GetStop(i) << ")";
        std::cout << std::endl;
      }
    }

    /// Identify free moves in NFA (for debugging)
    void PrintFreeMoves() {
      for (int i = 0; i < states.size(); i++) {
        std::cout << "Free from ( ";
        for (auto x : states[i].free_from) std::cout << x << " ";
        std::cout << ") to " << i << std::endl;
        std::cout << "Free from " << i << " to ( ";
        for (auto x : states[i].free_to) std::cout << x << " ";
        std::cout << ")" << std::endl;
      }
    }
  };

  /// Information about the current full state (i.e., set of legal states) of an NFA.
  template <size_t NUM_SYMBOLS=128, typename STOP_TYPE=uint8_t>
  class tNFA_State {
  private:
    const tNFA<NUM_SYMBOLS,STOP_TYPE> & nfa;  ///< Which NFA is this state set associated with?
    std::set<size_t> state_set;               ///< Which states are currently legal?
  public:
    tNFA_State(const tNFA<NUM_SYMBOLS,STOP_TYPE> & _nfa) : nfa(_nfa), state_set() {
      state_set = nfa.GetStart();
    }
    ~tNFA_State() { ; }

    /// Get the NFA associated with this state.
    const tNFA<NUM_SYMBOLS,STOP_TYPE> & GetNFA() const { return nfa; }

    /// Get a set of states that are currently active.
    const std::set<size_t> & GetStateSet() const { return state_set; }

    /// Are there currently any legal NFA states?
    bool IsActive() const { return state_set.size(); }

    /// Can we legally stop in any of the current states?
    bool IsStop() const {
      for (auto s : state_set) if (nfa.IsStop(s)) return true;
      return false;
    }

    /// Is a particular NFA state currently included?
    bool HasState(size_t id) { return state_set.count(id); }

    /// How many states are currently included?
    size_t GetSize() { return state_set.size(); }

    /// Set the current states.
    void SetStateSet(const std::set<size_t> & in) { state_set = in; }

    /// Change current states to start + free transitions from start.
    void Reset() { state_set = nfa.GetStart(); }

    /// Update states given a new input symbol.
    void Next(size_t sym) {
      state_set = nfa.GetNext(sym, state_set);
    }

    /// Update states given a new series of input symbols (as a string)
    void Next(const std::string & sym_set) {
      for (char x : sym_set) Next((size_t) x);
    }

    /// Print out current information about this NFA State (for debugging)
    void Print() {
      std::cout << "cur states:";
      for (auto s : state_set) {
        std::cout << " " << s;
      }
      std::cout << std::endl;
    }
  };

  /// NFA is the most standard tNFA setup.
  using NFA = tNFA<128, uint8_t>;

  /// NFA_State is the most standard tNFA_State setup.
  using NFA_State = tNFA_State<128, uint8_t>;
}

#endif // #ifndef EMP_COMPILER_NFA_HPP_INCLUDE
