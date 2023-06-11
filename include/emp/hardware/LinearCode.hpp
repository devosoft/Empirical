/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file LinearCode.hpp
 *  @brief A linear sequence of instructions.
 */

#ifndef EMP_HARDWARE_LINEARCODE_HPP_INCLUDE
#define EMP_HARDWARE_LINEARCODE_HPP_INCLUDE

#include "../base/array.hpp"
#include "../base/vector.hpp"

namespace emp {

  /// A single instruction in a linear genome.
  template <size_t ARG_COUNT, typename ARG_TYPE=size_t>
  struct Instruction {
    using args_t = emp::array<ARG_TYPE, ARG_COUNT>;

    size_t id;    ///< Unique value identifying which instruction this is.
    args_t args;  ///< A set of arguments defining the specific behavior of this instruction.

    Instruction(size_t _id=0, const args_t & in_args={}) : id(_id), args(in_args) { ; }
    Instruction(const Instruction &) = default;
    Instruction(Instruction &&) = default;

    /// Copy operator.
    Instruction & operator=(const Instruction &) = default;

    /// Move operator.
    Instruction & operator=(Instruction &&) = default;

    /// Test if this instruction is less than another.
    bool operator<(const Instruction & other) const {
      if (id == other.id) return args < other.args;
      return id < other.id;
    }

    /// Explicitly set the instruction type and specific arguments for this instruction.
    void Set(size_t _id, const args_t & in_args) { id = _id; args = in_args; }

    /// Test if this instruction is identical to another.
    bool operator==(const Instruction & in) const { return id == in.id && args == in.args; }
  };


  // template <size_t ARG_COUNT=3, typename ARG_TYPE=size_t>
  // using LinearCode = emp::vector<Instruction<ARG_COUNT, ARG_TYPE>>;

  template <size_t ARG_COUNT=3, typename ARG_TYPE=size_t>
  class LinearCode : public emp::vector<Instruction<ARG_COUNT, ARG_TYPE>> {
  public:
    void AddInst(size_t id, const emp::array<ARG_TYPE, ARG_COUNT> & in_args={}) {
      emp::vector<Instruction<ARG_COUNT, ARG_TYPE>>::push_back(Instruction<ARG_COUNT, ARG_TYPE>(id, in_args));
    }
  };

}


#endif // #ifndef EMP_HARDWARE_LINEARCODE_HPP_INCLUDE
