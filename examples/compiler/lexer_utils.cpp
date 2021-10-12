//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Example code for converting string pattern representations.

#include <iostream>
#include <string>

#include "emp/compiler/DFA.hpp"
#include "emp/compiler/NFA.hpp"
#include "emp/compiler/lexer_utils.hpp"
#include "emp/compiler/RegEx.hpp"

void Test(const emp::DFA & dfa, const std::string & str) {
  int out_state = dfa.Next(0, str);
  std::cout << "String: " << str
            << "  valid=" << (out_state != -1)
            << "  stop=" << dfa.IsStop(out_state)
            << std::endl;
}

void Test(const emp::NFA & nfa, const std::string & str) {
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

  // std::cout << "NFA size = " << nfa2c.GetSize() << std::endl;
  // Test(nfa2c, "ababaabbab");
  // Test(nfa2c, "cc");
  // Test(nfa2c, "ccc");
  // Test(nfa2c, "ababcbc");
  // Test(nfa2c, "cbabab");
  // Test(nfa2c, "ccbabab");
  // Test(nfa2c, "ccbababc");

  // std::cout << std::endl;
  // auto dfa2c = to_DFA(nfa2c);
  // dfa2c.Print();

  // std::cout << "DFA size = " << dfa2c.GetSize() << std::endl;
  // Test(dfa2c, "ababaabbab");
  // Test(dfa2c, "cc");
  // Test(dfa2c, "ccc");
  // Test(dfa2c, "ababcbc");
  // Test(dfa2c, "cbabab");
  // Test(dfa2c, "ccbabab");
  // Test(dfa2c, "ccbababc");

  emp::RegEx re2f("[de]*f[de]*f[de]*");
  // emp::RegEx re2f("([de]*)f([de]*)f([de]*)");
  emp::NFA nfa2f = to_NFA(re2f);
  emp::DFA dfa2f = to_DFA(nfa2f);
  re2f.PrintDebug();
  std::cout << "RegEx NFA size = " << nfa2f.GetSize() << std::endl;
  nfa2f.Print();
  std::cout << "RegEx DFA size = " << dfa2f.GetSize() << std::endl;
  dfa2f.Print();

  Test(dfa2f, "a");
  Test(dfa2f, "d");
  Test(dfa2f, "defdef");
  Test(dfa2f, "fedfed");
  Test(dfa2f, "ffed");
  Test(dfa2f, "edffed");
  Test(dfa2f, "edffedf");
  Test(dfa2f, "defed");
  Test(dfa2f, "ff");

  std::cout << "\nAll same case tests..." << std::endl;
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

  re_lower.PrintDebug();
  nfa_lower.Print();
  nfa_all.Print();
  dfa_all.Print();
  std::cout << "=== nfa_lower ===" << std::endl;
  Test(nfa_lower, "abc");
  Test(nfa_lower, "DEF");
  Test(nfa_lower, "abcDEF");
  Test(nfa_lower, "ABDdef");
  Test(nfa_lower, "ABCDEF");
  Test(nfa_lower, "abcdefghijklmnopqrstuvwxyz");
  Test(nfa_lower, "ABC-DEF");

  std::cout << "=== dfa_all ===" << std::endl;
  Test(dfa_all, "abc");
  Test(dfa_all, "DEF");
  Test(dfa_all, "abcDEF");
  Test(dfa_all, "ABDdef");
  Test(dfa_all, "ABCDEF");
  Test(dfa_all, "abcdefghijklmnopqrstuvwxyz");
  Test(dfa_all, "ABC-DEF");

  std::cout << "DFA Inc:\n";
  dfa_inc.Print();

  // Generate examples of DFAs
  std::cout << "DFA Examples:\n"
            << "dfa_lower example: " << emp::FindExample(dfa_lower) << std::endl
            << "dfa_upper example: " << emp::FindExample(dfa_upper) << std::endl
            << "dfa_inc example:   " << emp::FindExample(dfa_inc) << std::endl
            << "dfa_all example:   " << emp::FindExample(dfa_all) << std::endl
            << "dfa_inc size 5 example: " << emp::FindExample(dfa_inc, 5) << std::endl;
}
