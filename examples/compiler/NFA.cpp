//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::NFA

#include <iostream>

#include "emp/compiler/NFA.hpp"

int main()
{
  std::cout << "Ping!" << std::endl;

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
  state.Print();
  state.Next('a');
  state.Print();
  state.Next('a');
  state.Print();

  emp::NFA_State state2(nfa);
  state2.Print();
  state2.Next("aaaa");
  state2.Print();
}
