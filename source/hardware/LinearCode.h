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

  template <size_t ARG_COUNT, typename ARG_TYPE=size_t>
  struct Instruction {
    using args_t = emp::array<ARG_TYPE, ARG_COUNT>;

    size_t id;
    args_t args;

    Instruction(size_t _id=0) : id(_id), args({}) { ; }
    Instruction(size_t _id, const args_t & in_args) : id(_id), args(in_args) { ; }
    Instruction(const Instruction &) = default;
    Instruction(Instruction &&) = default;

    Instruction & operator=(const Instruction &) = default;
    Instruction & operator=(Instruction &&) = default;
    bool operator<(const Instruction & other) const {
      if (id == other.id) return args < other.args;
      return id < other.id;
    }

    void Set(size_t _id, size_t _a0=0, size_t _a1=0, size_t _a2=0)
    { id = _id; args[0] = _a0; args[1] = _a1; args[2] = _a2; }

    bool operator==(const Instruction & in) const { return id == in.id && args == in.args; }
  };

  using LinearCode = emp::vector<Instruction<3>>;

}


#endif
