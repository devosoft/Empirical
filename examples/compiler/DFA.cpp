//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::NFA

#include <iostream>

#include "emp/compiler/DFA.hpp"

emp::DFA dfa(10);

void TestDFA(const std::string & test_str)
{
  std::cout << "String " << test_str << ": ";
  int state = dfa.Next(0, test_str);
  std::cout << state << '\n';
}

int main()
{
  std::cout << "Ping!" << '\n';

  dfa.SetTransition(0, 1, 'a');
  dfa.SetTransition(1, 2, 'a');
  dfa.SetTransition(2, 0, 'a');
  dfa.SetTransition(0, 3, 'b');

  int state = 0;
  std::cout << (state = dfa.Next(state, 'a')) << '\n';
  std::cout << (state = dfa.Next(state, 'a')) << '\n';
  std::cout << (state = dfa.Next(state, 'a')) << '\n';
  std::cout << (state = dfa.Next(state, 'b')) << '\n';
  std::cout << (state = dfa.Next(state, 'b')) << '\n';
  std::cout << (state = dfa.Next(state, 'b')) << '\n';
  std::cout << (state = dfa.Next(state, 'b')) << '\n';

  TestDFA("aaaaaab");
  TestDFA("aaaaab");
  TestDFA("aaaaaabb");
  TestDFA("a");
  TestDFA("aa");
  TestDFA("aaa");
  TestDFA("b");

}
