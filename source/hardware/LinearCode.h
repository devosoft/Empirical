/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file  LinearCode.h
 *  @brief A linear sequence of instructions.
 */

#ifndef EMP_LINEAR_CODE_H
#define EMP_LINEAR_CODE_H

#include "../base/array.h"
#include "../base/vector.h"

namespace emp {

  struct Instruction {
    size_t id;
    emp::array<size_t, 3> args;

    Instruction(size_t _id=0, size_t a0=0, size_t a1=0, size_t a2=0)
      : id(_id), args() { args[0] = a0; args[1] = a1; args[2] = a2; }
    Instruction(const Instruction &) = default;
    Instruction(Instruction &&) = default;

    Instruction & operator=(const Instruction &) = default;
    Instruction & operator=(Instruction &&) = default;
    bool operator<(const Instruction & other) const {
      return id < other.id;
    }

    void Set(size_t _id, size_t _a0=0, size_t _a1=0, size_t _a2=0)
    { id = _id; args[0] = _a0; args[1] = _a1; args[2] = _a2; }

    bool operator==(const Instruction & in) const { return id == in.id && args == in.args; }
  };

  using LinearCode = emp::vector<Instruction>;

}


#endif
