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

#include "emp/compiler/DFA.hpp"
#include "emp/compiler/lexer_utils.hpp"
#include "emp/compiler/NFA.hpp"
#include "emp/compiler/RegEx.hpp"

TEST_CASE("Test lexer_utils", "[compiler]")
{
  emp::DFA dfa;
  dfa.Resize(3);
  dfa.SetTransition(0, 2, 'a');
  dfa.SetTransition(2, 1, 'b');
  dfa.SetTransition(1, 0, 'c');
  dfa.SetStop(0, 100);

  CHECK(emp::to_DFA(dfa).GetSize() == dfa.GetSize());
  CHECK(emp::to_DFA(dfa).Test("abc") == 100);
  CHECK(emp::to_DFA(dfa).Test("abc") == dfa.Test("abc"));

  emp::NFA nfa = emp::to_NFA(dfa);
  CHECK(nfa.GetSize() == dfa.GetSize());
  emp::BitVector nxt = nfa.GetNext('a');
  CHECK(nxt.Has(2));
  CHECK(!nxt.Has(1));

  dfa.SetStop(0);
  std::string example = emp::FindExample(dfa);
  // std::cout << "FOUND: '" << example << "' : " << example.size() << std::endl;
  CHECK(dfa.Test(example));

  emp::DFA dfa2;
  dfa2.Resize(3);
  CHECK(emp::FindExample(dfa2) == "");
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

  int state;
  state = dfa2f.Next(0, "a");        CHECK(dfa2f.IsStop(state) == false);
  state = dfa2f.Next(0, "d");        CHECK(dfa2f.IsStop(state) == false);
  state = dfa2f.Next(0, "defdef");   CHECK(dfa2f.IsStop(state) == true);
  state = dfa2f.Next(0, "fedfed");   CHECK(dfa2f.IsStop(state) == true);
  state = dfa2f.Next(0, "ffed");     CHECK(dfa2f.IsStop(state) == true);
  state = dfa2f.Next(0, "edffed");   CHECK(dfa2f.IsStop(state) == true);
  state = dfa2f.Next(0, "edffedf");  CHECK(dfa2f.IsStop(state) == false);
  state = dfa2f.Next(0, "defed");    CHECK(dfa2f.IsStop(state) == false);
  state = dfa2f.Next(0, "ff");       CHECK(dfa2f.IsStop(state) == true);

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
  lstate.Reset(); lstate.Next("abc");      CHECK(lstate.IsActive() == true);
  lstate.Reset(); lstate.Next("DEF");      CHECK(lstate.IsActive() == false);
  lstate.Reset(); lstate.Next("abcDEF");   CHECK(lstate.IsActive() == false);
  lstate.Reset(); lstate.Next("ABDdef");   CHECK(lstate.IsActive() == false);
  lstate.Reset(); lstate.Next("ABCDEF");   CHECK(lstate.IsActive() == false);
  lstate.Reset(); lstate.Next("abcdefghijklmnopqrstuvwxyz");  CHECK(lstate.IsActive() == true);
  lstate.Reset(); lstate.Next("ABC-DEF");  CHECK(lstate.IsActive() == false);

  CHECK( dfa_all.Next(0, "abc") > 0 );
  CHECK( dfa_all.Next(0, "DEF") > 0 );
  CHECK( dfa_all.Next(0, "abcDEF") > 0 );
  CHECK( dfa_all.Next(0, "ABDdef") == -1 );
  CHECK( dfa_all.Next(0, "ABCDEF") > 0 );
  CHECK( dfa_all.Next(0, "abcdefghijklmnopqrstuvwxyz") > 0);
  CHECK( dfa_all.Next(0, "ABC-DEF") == -1 );
}
