/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2021
*/
/**
 *  @file
 */

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/compiler/NFA.hpp"

TEST_CASE("Test NFA", "[compiler]")
{
  // NFA class
  emp::NFA nfa(5);
  nfa.AddTransition(0,1,"ac");
  nfa.AddTransition(1,1,"b");
  nfa.AddTransition(1,2,"ae");
  nfa.AddTransition(1,4,"bd");
  nfa.AddTransition(2,2,"b");
  nfa.AddTransition(4,4,"e");
  nfa.AddTransition(4,3,"ce");
  nfa.AddTransition(3,1,"c");
  nfa.AddTransition(3,0,"d");

  size_t a = 97;

  // GetSymbolOptions
  emp::DynamicBits s;
  s.Set(0);
  emp::BitSet<128> symbolOpts = nfa.GetSymbolOptions(s);
  REQUIRE(symbolOpts[97]); // ASCII value for a
  REQUIRE(symbolOpts[99]); // c
  REQUIRE(symbolOpts.count() == 2);
  s.Set(1);
  s.Set(2);
  s.Set(3);
  s.Set(4);
  symbolOpts = nfa.GetSymbolOptions(s);
  REQUIRE(symbolOpts[97]);  // ASCII a value
  REQUIRE(symbolOpts[98]);  // b
  REQUIRE(symbolOpts[99]);  // c
  REQUIRE(symbolOpts[100]); // d
  REQUIRE(symbolOpts[101]); // e
  REQUIRE(symbolOpts.count() == 5);

  // GetNext
  emp::DynamicBits nxt = nfa.GetNext(a);
  REQUIRE(nxt.Has(1));
  REQUIRE(!nxt.Has(0));

  // assignment operator=
  emp::NFA nfa2 = nfa;
  REQUIRE(nfa2.GetSize() == nfa.GetSize());

  // HasFreeTransitions AddFreeTransition
  REQUIRE(!nfa2.HasFreeTransitions(2));
  nfa2.AddFreeTransition(2,0);
  REQUIRE(nfa2.HasFreeTransitions(2));

  // IsStart
  REQUIRE(nfa.IsStart(0));
  REQUIRE(!nfa.IsStart(1));

  // NFA_State class
  emp::NFA_State state(nfa);
  REQUIRE(state.GetSize() == 1);

  // GetNFA
  REQUIRE(state.GetNFA().GetSize() == nfa.GetSize());

  // GetStateSet
  emp::DynamicBits states = state.GetStateSet();
  REQUIRE(states.count() == 1);
  REQUIRE(states.Has(0));
  state.Next("a");
  states = state.GetStateSet();
  REQUIRE(states.count() == 1);
  REQUIRE(states.Has(1));

  // IsStop
  REQUIRE(!state.IsStop());
  nfa.SetStop(1);
  REQUIRE(state.IsStop());
  REQUIRE(state.HasState(1));

  // SetStateSet
  emp::DynamicBits set1;
  set1.Set(0);
  set1.Set(2);
  state.SetStateSet(set1);
  REQUIRE(state.GetStateSet() == set1);

}

TEST_CASE("Another Test NFA", "[compiler]")
{
  emp::NFA nfa(10);
  nfa.AddTransition(0, 1, 'a');
  nfa.AddTransition(0, 2, 'a');
  nfa.AddTransition(0, 3, 'a');
  nfa.AddTransition(0, 4, 'a');

  nfa.AddTransition(1, 2, 'b');
  nfa.AddTransition(2, 3, 'c');
  nfa.AddTransition(3, 4, 'd');

  nfa.AddTransition(0, 1, 'e');
  nfa.AddTransition(0, 1, 'f');
  nfa.AddTransition(0, 1, 'g');

  nfa.AddTransition(2, 3, 'a');
  nfa.AddTransition(3, 4, 'a');
  nfa.AddTransition(2, 4, 'a');

  nfa.AddTransition(2, 2, 'e');
  nfa.AddTransition(3, 3, 'e');
  nfa.AddTransition(4, 4, 'e');

  nfa.AddFreeTransition(1,5);

  nfa.AddTransition(5, 6, 'a');

  nfa.AddFreeTransition(6,7);
  nfa.AddFreeTransition(6,8);
  nfa.AddFreeTransition(6,9);
  nfa.AddFreeTransition(9,0);

  emp::NFA_State state(nfa);
  REQUIRE(state.GetSize() == 1);
  state.Next('a');
  REQUIRE(state.GetSize() == 5);
  state.Next('a');
  REQUIRE(state.GetSize() == 7);

  emp::NFA_State state2(nfa);
  REQUIRE(state2.GetSize() == 1);
  state2.Next("aaaa");
  REQUIRE(state2.GetSize() == 7);
}
