#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/compiler/NFA.hpp"

#include <sstream>
#include <iostream>

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
	std::set<size_t> s;
	s.insert(0);
	emp::BitSet<128> symbolOpts = nfa.GetSymbolOptions(s);
	REQUIRE(symbolOpts[97]); // ASCII value for a
	REQUIRE(symbolOpts[99]); // c
	REQUIRE(symbolOpts.count() == 2);
	s.insert(1);
	s.insert(2);
	s.insert(3);
	s.insert(4);
	symbolOpts = nfa.GetSymbolOptions(s);
	REQUIRE(symbolOpts[97]);  // ASCII a value
	REQUIRE(symbolOpts[98]);  // b
	REQUIRE(symbolOpts[99]);  // c
	REQUIRE(symbolOpts[100]); // d
	REQUIRE(symbolOpts[101]); // e
	REQUIRE(symbolOpts.count() == 5);

	// GetNext
	std::set<size_t> nxt = nfa.GetNext(a);
	REQUIRE(nxt.find(1) != nxt.end());
	REQUIRE(nxt.find(0) == nxt.end());

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
	std::set<size_t> states = state.GetStateSet();
	REQUIRE(states.size() == 1);
	REQUIRE(states.find(0) != states.end());
	state.Next("a");
	states = state.GetStateSet();
	REQUIRE(states.size() == 1);
	REQUIRE(states.find(1) != states.end());

	// IsStop
	REQUIRE(!state.IsStop());
	nfa.SetStop(1);
	REQUIRE(state.IsStop());
	REQUIRE(state.HasState(1));

	// SetStateSet
	std::set<size_t> set1;
	set1.insert(0);
	set1.insert(2);
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
