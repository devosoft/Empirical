//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  An emp::Instruction object hold all of the information for a single CPU Instruction.


#ifndef EMP_INSTRUCTION_H
#define EMP_INSTRUCTION_H

namespace emp {

  template <typename T, size_t ARGS>
  struct Instruction {
    T id;
    int args[ARGS];

    constexpr static size_t GetNumArgs() { return ARGS; }
  };

}

#endif
