//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Example code for converting string pattern representations.

#include <iostream>
#include <string>

#include "../../tools/DFA.h"
#include "../../tools/NFA.h"
#include "../../tools/lexer_utils.h"

void TestNFA(const emp::NFA & nfa, const std::string & str) {
  emp::NFA_State nfa_state(nfa);
  nfa_state.Next(str);
  std::cout << "String: " << str
            << "  valid=" << nfa_state.IsActive()
            << "  stop=" << nfa_state.IsStop()
            << std::endl;
}

int main()
{
  emp::NFA nfa2c(3);  // Must have two c's with any number of a's or b's.
  nfa2c.AddTransition(0,0,"ab");
  nfa2c.AddTransition(0,1,"c");
  nfa2c.AddTransition(1,1,"ab");
  nfa2c.AddTransition(1,2,"c");
  nfa2c.AddTransition(2,2,"ab");
  nfa2c.SetStop(2);

  TestNFA(nfa2c, "cc");
  TestNFA(nfa2c, "ccc");
  TestNFA(nfa2c, "ababcbc");
  TestNFA(nfa2c, "cbabab");
  TestNFA(nfa2c, "ccbabab");
  TestNFA(nfa2c, "ccbababc");


  auto dfa2c = to_DFA(nfa2c);
}
