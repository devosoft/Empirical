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

int main()
{
  emp::NFA nfa2c(3);  // Must have two c's with any number of a's or b's.
  nfa2c.AddTransition(0,0,"ab");
  nfa2c.AddTransition(0,1,"c");
  nfa2c.AddTransition(1,1,"ab");
  nfa2c.AddTransition(1,2,"c");
  nfa2c.AddTransition(2,2,"ab");
  nfa2c.SetStop(2);
}
