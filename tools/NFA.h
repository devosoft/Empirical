//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A Non-deterministic Finite Automata simulator
//
//
//  To build a standard NFA, use emp::NFA.  If you want to have more symbols or more stop states,
//  use emp::tNFA<S,T> where S is the number of symbols and T is the type used for stop.
//  (defaults are 128 for ASCII-128 and uint8_t respectively.)
//
//  The constructor can take as parameters the number of states and the id of the start state (both
//  default to 0)
//
//  size_t GetSize() const
//    return the current number of states.
//
//  std::set<size_t> GetStart() const
//    return start state and all others reachable through empty transitions.
//
//  std::set<size_t> GetNext(size_t sym, size_t from_id=0) const
//  std::set<size_t> GetNext(size_t sym, const std::set<size_t> from_set) const
//    return the states reachable from the current state or set of state given the provided symbol.
//
//  bool HasFreeTransitions(size_t id) const
//  bool HasSymTransitions(size_t id) const
//    Does the provided state has free transitions or symbol-transitions (respecitvely)?
//
//  opts_t GetSymbolOptions(const std::set<size_t> & test_set) const
//    Return an emp::BitSet indicating the symbols available from the provided set of states.
//
//  void Resize(size_t new_size)
//    Change the number of available states.
//
//  size_t AddNewState()
//    Add a new state into the NFA and return its id.
//
//  void AddTransition(size_t from, size_t to, size_t sym)
//  void AddTransition(size_t from, size_t to, const std::string & sym_set)
//  void AddTransition(size_t from, size_t to, const BitSet<NUM_SYMBOLS> & sym_set)
//    Add a transition between states 'from' and 'to' that can be taken with the provided symbols.
//
//  void AddFreeTransition(size_t from, size_t to)
//    Create a free transition between 'from' and 'to'.
//
//  void SetStop(size_t state, stop_t stop_val=1)
//    Set the specified state to be a stop state (with an optional stop value.)
//
//  stop_t GetStop(size_t state) const
//     Return the stop value associated with the specified state.
//
//  bool IsStart(size_t state) const
//  bool IsStop(size_t state) const
//  bool IsEmpty(size_t state) const
//    Testing types of states:
//    START -> This is where the NFA begins (may have free transitions to other states)
//    STOP -> A legal endpoint for the NFA.
//    EMPTY -> A state with only empty transitions from it, and not stop state.
//
//  void Print() const
//    Output the structure of this NFA.
//
//
//  NFAs can also be manipulated using emp::NFA_State objects (or emp::tNFA_State).  Its
//  constructor must be provided with the NFA it should track and it will being in the start state.
//
//  const tNFA<NUM_SYMBOLS,STOP_TYPE> & GetNFA() const
//    Return the NFA being used.
//
//  const std::set<size_t> & GetStateSet() const
//    Return the current set of states accessible with the symbols provided.
//
//  bool IsActive() const
//    Are the current set of states "active" (that is, legal)
//
//  bool IsStop() const
//    Are any of the current states STOP states?
//
//  void SetStateSet(const std::set<size_t> & in)
//    Change the current set of state to the ones specified.
//
//  void Reset()
//    Reset to the start state.
//
//  void Next(size_t sym)
//  void Next(const std::string & sym_set)
//    From the current set of states advance to the next set of states that would be possible
//    given the symbols provided.
//
//  void Print()
//    Output the current set of states being used.
//
//
//  Note: DFA's use SetTransition(), but NFA's use AddTransition.  This distinction is intentional
//        since in a DFA a second SetTransition with the same start state and symbol will override
//        first, while in an NFA a second AddTransition will always add a new option.

#ifndef EMP_NFA_H
#define EMP_NFA_H

#include <map>
#include <set>

#include "BitSet.h"
#include "set_utils.h"
#include "vector.h"

namespace emp {

  template <size_t S=128, typename STOP_TYPE=uint8_t>
  class tNFA {
  public:
    static const constexpr size_t NUM_SYMBOLS = S;
    using opts_t = BitSet<NUM_SYMBOLS>;
    using stop_t = STOP_TYPE;

  private:
    struct Transition {
      opts_t symbols;
    };
    struct State {
      std::map<size_t, Transition> trans;   // What symbol transitions are available?
      std::set<size_t> free_to;             // What other states can you move to for free?
      std::set<size_t> free_from;           // What other states can move here for free?
    };

    emp::vector<State> states;
    size_t start;
    emp::vector< STOP_TYPE > is_stop;         // 0=no 1=yes (char instead of bool for speed)

  public:
    tNFA(size_t num_states=1, size_t start_state=0)
      : states(num_states), start(start_state), is_stop(num_states, 0) {
        if (num_states > start) states[start].free_to.insert(start);
      }
    tNFA(const tNFA<S,STOP_TYPE> &) = default;
    ~tNFA() { ; }
    tNFA<S,STOP_TYPE> & operator=(const tNFA<S,STOP_TYPE> &) = default;

    size_t GetSize() const { return states.size(); }
    const std::set<size_t> & GetStart() const {
      emp_assert(start < states.size());
      return states[start].free_to;
    }
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

    bool HasFreeTransitions(size_t id) const { return states[id].free_to.size(); }
    bool HasSymTransitions(size_t id) const { return states[id].trans.size(); }

    opts_t GetSymbolOptions(const std::set<size_t> & test_set) const {
      opts_t options;
      for (size_t id : test_set) {
        for (const auto & t : states[id].trans) {
          options |= t.second.symbols;
        }
      }
      return options;
    }

    void Resize(size_t new_size) {
      states.resize(new_size);
      is_stop.resize(new_size, 0);
      if (new_size > start) states[start].free_to.insert(start);
    }
    size_t AddNewState() { size_t new_state = GetSize(); Resize(new_state+1); return new_state; }
    void AddTransition(size_t from, size_t to, size_t sym) {
      emp_assert(from < states.size(), from, states.size());
      emp_assert(to < states.size(), to, states.size());

      states[from].trans[to].symbols[sym] = true;
    }
    void AddTransition(size_t from, size_t to, const std::string & sym_set) {
      for (char x : sym_set) AddTransition(from, to, (size_t) x);
    }
    void AddTransition(size_t from, size_t to, const BitSet<NUM_SYMBOLS> & sym_set) {
      emp_assert(from < states.size(), from, states.size());
      emp_assert(to < states.size(), to, states.size());

      states[from].trans[to].symbols |= sym_set;
    }
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

    template <typename T=stop_t>
    void SetStop(size_t state, T stop_val=1) { is_stop[state] = (stop_t) stop_val; }
    stop_t GetStop(size_t state) const { return is_stop[state]; }

    // Testing types of states:
    //  START -> This is where the NFA begins (may have free transitions to other states)
    //  STOP -> A legal endpoint for the NFA.
    //  EMPTY -> A state with only empty transitions from it, and not stop state.
    bool IsStart(size_t state) const { return state == start; }
    bool IsStop(size_t state) const { return is_stop[state]; }
    bool IsEmpty(size_t state) const { return !HasSymTransitions(state) && !IsStop(state); }

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

  template <size_t NUM_SYMBOLS=128, typename STOP_TYPE=uint8_t>
  class tNFA_State {
  private:
    const tNFA<NUM_SYMBOLS,STOP_TYPE> & nfa;
    std::set<size_t> state_set;
  public:
    tNFA_State(const tNFA<NUM_SYMBOLS,STOP_TYPE> & _nfa) : nfa(_nfa) { state_set = nfa.GetStart(); }
    ~tNFA_State() { ; }

    const tNFA<NUM_SYMBOLS,STOP_TYPE> & GetNFA() const { return nfa; }
    const std::set<size_t> & GetStateSet() const { return state_set; }

    bool IsActive() const { return state_set.size(); }
    bool IsStop() const {
      for (auto s : state_set) if (nfa.IsStop(s)) return true;
      return false;
    }
    bool HasState(size_t id) { return state_set.count(id); }
    size_t GetSize() { return state_set.size(); }

    void SetStateSet(const std::set<size_t> & in) { state_set = in; }
    void Reset() { state_set = nfa.GetStart(); }

    void Next(size_t sym) {
      state_set = nfa.GetNext(sym, state_set);
    }

    void Next(const std::string & sym_set) {
      for (char x : sym_set) Next((size_t) x);
    }

    void Print() {
      std::cout << "cur states:";
      for (auto s : state_set) {
        std::cout << " " << s;
      }
      std::cout << std::endl;
    }
  };


  using NFA = tNFA<128, uint8_t>;
  using NFA_State = tNFA_State<128, uint8_t>;
}

#endif
