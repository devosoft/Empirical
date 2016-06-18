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

void TestDFA(const emp::DFA & dfa, const std::string & str) {
  int out_state = dfa.Next(0, str);
  std::cout << "String: " << str
            << "  valid=" << (out_state != -1)
            << "  stop=" << dfa.IsStop(out_state)
            << std::endl;
}

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
  emp::NFA nfa2c(3);  // Must have zero or two c's with any number of a's or b's.
  nfa2c.AddTransition(0,0,"ab");
  nfa2c.AddTransition(0,1,"c");
  nfa2c.AddTransition(1,1,"ab");
  nfa2c.AddTransition(1,2,"c");
  nfa2c.AddTransition(2,2,"ab");
  nfa2c.AddFreeTransition(0,2);
  nfa2c.SetStop(2);

  std::cout << "NFA size = " << nfa2c.GetSize() << std::endl;
  TestNFA(nfa2c, "ababaabbab");
  TestNFA(nfa2c, "cc");
  TestNFA(nfa2c, "ccc");
  TestNFA(nfa2c, "ababcbc");
  TestNFA(nfa2c, "cbabab");
  TestNFA(nfa2c, "ccbabab");
  TestNFA(nfa2c, "ccbababc");

  std::cout << std::endl;
  auto dfa2c = to_DFA(nfa2c);
  dfa2c.Print();

  std::cout << "DFA size = " << dfa2c.GetSize() << std::endl;
  TestDFA(dfa2c, "ababaabbab");
  TestDFA(dfa2c, "cc");
  TestDFA(dfa2c, "ccc");
  TestDFA(dfa2c, "ababcbc");
  TestDFA(dfa2c, "cbabab");
  TestDFA(dfa2c, "ccbabab");
  TestDFA(dfa2c, "ccbababc");

  emp::RegEx re2f("[de]*f[de]*f[de]*");
  // emp::RegEx re2f("([de]*)f([de]*)f([de]*)");
  emp::NFA nfa2f = to_NFA(re2f);
  emp::DFA dfa2f = to_DFA(nfa2f);
  re2f.PrintDebug();
  std::cout << "RegEx NFA size = " << nfa2f.GetSize() << std::endl;
  nfa2f.Print();
  std::cout << "RegEx DFA size = " << dfa2f.GetSize() << std::endl;
  dfa2f.Print();

  TestDFA(dfa2f, "a");
  TestDFA(dfa2f, "d");
  TestDFA(dfa2f, "defdef");
  TestDFA(dfa2f, "fedfed");
  TestDFA(dfa2f, "ffed");
  TestDFA(dfa2f, "edffed");
  TestDFA(dfa2f, "edffedf");
  TestDFA(dfa2f, "ff");
}
