#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/compiler/lexer_utils.hpp"
#include "emp/compiler/NFA.hpp"
#include "emp/compiler/RegEx.hpp"
#include "emp/compiler/DFA.hpp"

#include <sstream>
#include <iostream>

TEST_CASE("Test lexer_utils", "[compiler]")
{
	emp::DFA dfa(3);
	dfa.SetTransition(0, 2, 'a');
	dfa.SetTransition(2, 1, 'b');
	dfa.SetTransition(1, 0, 'c');

	REQUIRE(emp::to_DFA(dfa).GetSize() == dfa.GetSize());
	REQUIRE(emp::to_DFA(dfa).Test("abc") == dfa.Test("abc"));

	emp::NFA nfa = emp::to_NFA(dfa);
	REQUIRE(nfa.GetSize() == dfa.GetSize());
    std::set<size_t> nxt = nfa.GetNext('a');
	REQUIRE(nxt.find(2) != nxt.end());
	REQUIRE(nxt.find(1) == nxt.end());

	dfa.SetStop(0);
	REQUIRE(dfa.Test(emp::FindExample(dfa)));

	emp::DFA dfa2(3);
	REQUIRE(emp::FindExample(dfa2) == "");
}

TEST_CASE("Another Test lexer_utils", "[compiler]")
{
  emp::NFA nfa2c(3);  // Must have zero or two c's with any number of a's or b's.
  nfa2c.AddTransition(0,0,"ab");
  nfa2c.AddTransition(0,1,"c");
  nfa2c.AddTransition(1,1,"ab");
  nfa2c.AddTransition(1,2,"c");
  nfa2c.AddTransition(2,2,"ab");
  nfa2c.AddFreeTransition(0,2);
  nfa2c.SetStop(2);

  emp::RegEx re2f("[de]*f[de]*f[de]*");
  // emp::RegEx re2f("([de]*)f([de]*)f([de]*)");
  emp::NFA nfa2f = to_NFA(re2f);
  emp::DFA dfa2f = to_DFA(nfa2f);
  REQUIRE( nfa2f.GetSize() == 15 );
  REQUIRE( dfa2f.GetSize() == 4 );

  int state;
  state = dfa2f.Next(0, "a");        REQUIRE(dfa2f.IsStop(state) == false);
  state = dfa2f.Next(0, "d");        REQUIRE(dfa2f.IsStop(state) == false);
  state = dfa2f.Next(0, "defdef");   REQUIRE(dfa2f.IsStop(state) == true);
  state = dfa2f.Next(0, "fedfed");   REQUIRE(dfa2f.IsStop(state) == true);
  state = dfa2f.Next(0, "ffed");     REQUIRE(dfa2f.IsStop(state) == true);
  state = dfa2f.Next(0, "edffed");   REQUIRE(dfa2f.IsStop(state) == true);
  state = dfa2f.Next(0, "edffedf");  REQUIRE(dfa2f.IsStop(state) == false);
  state = dfa2f.Next(0, "defed");    REQUIRE(dfa2f.IsStop(state) == false);
  state = dfa2f.Next(0, "ff");       REQUIRE(dfa2f.IsStop(state) == true);

  emp::RegEx re_lower("[a-z]+");
  emp::RegEx re_upper("[A-Z]+");
  emp::RegEx re_inc("[a-z]+[A-Z]+");
  emp::NFA nfa_lower = to_NFA(re_lower);
  emp::NFA nfa_upper = to_NFA(re_upper);
  emp::NFA nfa_inc = to_NFA(re_inc);
  emp::NFA nfa_all = MergeNFA(nfa_lower, nfa_upper, nfa_inc);
  emp::DFA dfa_lower = to_DFA(nfa_lower);
  emp::DFA dfa_upper = to_DFA(nfa_upper);
  emp::DFA dfa_inc = to_DFA(nfa_inc);
  emp::DFA dfa_all = to_DFA(nfa_all);

  emp::NFA_State lstate(nfa_lower);
  lstate.Reset(); lstate.Next("abc");      REQUIRE(lstate.IsActive() == true);
  lstate.Reset(); lstate.Next("DEF");      REQUIRE(lstate.IsActive() == false);
  lstate.Reset(); lstate.Next("abcDEF");   REQUIRE(lstate.IsActive() == false);
  lstate.Reset(); lstate.Next("ABDdef");   REQUIRE(lstate.IsActive() == false);
  lstate.Reset(); lstate.Next("ABCDEF");   REQUIRE(lstate.IsActive() == false);
  lstate.Reset(); lstate.Next("abcdefghijklmnopqrstuvwxyz");  REQUIRE(lstate.IsActive() == true);
  lstate.Reset(); lstate.Next("ABC-DEF");  REQUIRE(lstate.IsActive() == false);

  REQUIRE( dfa_all.Next(0, "abc") == 2 );
  REQUIRE( dfa_all.Next(0, "DEF") == 1 );
  REQUIRE( dfa_all.Next(0, "abcDEF") == 3 );
  REQUIRE( dfa_all.Next(0, "ABDdef") == -1 );
  REQUIRE( dfa_all.Next(0, "ABCDEF") == 1 );
  REQUIRE( dfa_all.Next(0, "abcdefghijklmnopqrstuvwxyz") == 2 );
  REQUIRE( dfa_all.Next(0, "ABC-DEF") == -1 );
}
