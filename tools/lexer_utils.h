//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A set of utilities to convert among RegEx, NFA, DFA, and fully lexers.

#ifndef EMP_LEXER_UTILS_H
#define EMP_LEXER_UTILS_H

#include <map>

#include "DFA.h"
#include "NFA.h"

namespace emp {

  static DFA to_DFA(const NFA & nfa) {
    DFA dfa;
    std::map<std::set<int>, int> id_map;
    

    return dfa;
  }

}

#endif

