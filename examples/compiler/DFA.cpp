/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016
 *
 *  @file DFA.cpp
 *  @brief Some examples code for using emp::DFA
 */

#include <iostream>

#include "emp/compiler/DFA.hpp"

emp::DFA dfa(10);

void TestDFA(const std::string & test_str)
{
  std::cout << "String " << test_str << ": ";
  int state = dfa.Next(0, test_str);
  std::cout << state << std::endl;
}

int main()
{
  std::cout << "Ping!" << std::endl;

  dfa.SetTransition(0, 1, 'a');
  dfa.SetTransition(1, 2, 'a');
  dfa.SetTransition(2, 0, 'a');
  dfa.SetTransition(0, 3, 'b');

  int state = 0;
  std::cout << (state = dfa.Next(state, 'a')) << std::endl;
  std::cout << (state = dfa.Next(state, 'a')) << std::endl;
  std::cout << (state = dfa.Next(state, 'a')) << std::endl;
  std::cout << (state = dfa.Next(state, 'b')) << std::endl;
  std::cout << (state = dfa.Next(state, 'b')) << std::endl;
  std::cout << (state = dfa.Next(state, 'b')) << std::endl;
  std::cout << (state = dfa.Next(state, 'b')) << std::endl;

  TestDFA("aaaaaab");
  TestDFA("aaaaab");
  TestDFA("aaaaaabb");
  TestDFA("a");
  TestDFA("aa");
  TestDFA("aaa");
  TestDFA("b");

}
