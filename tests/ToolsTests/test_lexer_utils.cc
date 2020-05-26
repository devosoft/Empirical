#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include "tools/lexer_utils.h"

#include <sstream>
#include <iostream>

TEST_CASE("Test lexer_utils", "[tools]")
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