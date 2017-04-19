//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2017.
//  Released under the MIT Software license; see doc/LICENSE

#ifndef EMP_INSTRUCTION_H
#define EMP_INSTRUCTION_H

namespace emp {

  template <size_t ARGS>
  struct Instruction {
    size_t id;
    int args[ARGS];

    constexpr static size_t GetNumArgs() { return ARGS; }
  };

}

#endif
