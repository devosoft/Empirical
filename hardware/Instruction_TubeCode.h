//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This class represents a single instruction in a program, with a set of arguments.
//
//  Each argument can be:
//    A const value  (number, character, or code position label)
//    A register ID
//    A variable ID  (for intermediate code)
//    An array ID    (for intermediate code)
//
//  Each argument stores:
//    its type
//    its value (direct or indirect)
//    a set of relevant flags (?)
//    a string to record its original content (its lexeme)
//
//  These instructions were originally designed to be used in a simplified, but
//  semi-realistic virtual CPU for educational goals.


#ifndef EMP_INSTRUCTION_TUBECODE_H
#define EMP_INSTRUCTION_TUBECODE_H

#include "../base/assert.h"

namespace emp {

  class Instruction_TubeCode {
  protected:
    enum class Type { VALUE, CHAR, LABEL, REG, VAR, ARRAY };

    Type type;
    double value;
    std::string lexeme;

  public:
    Instruction_TubeCode(const std::string & in_lexeme) : lexeme(in_lexeme)
    {
      // Based in the lexeme, we should be able to make sense of this argument.
      // number (with possible decimal point) -> const value
      // Single quote followed by single character and another single quote -> literal char
      // 'a' followed by int -> array variable
      // 's' followed by int -> scalar variable
      // "reg" followed by a capital letter -> register
      // Any other letter combination -> label
    }
    Instruction_TubeCode(const Instruction_TubeCode & in_inst) = default;
    ~Instruction_TubeCode() { ; }

    Instruction_TubeCode & operator=(const Instruction_TubeCode & _in) {
      info = _in.info & FIXED_BIT_MASK;
      return *this;
    }

    // Comparison operators ignore flags.
    bool operator==(const Instruction_TubeCode & _in) const { return GetID() == _in.GetID(); }
    bool operator!=(const Instruction_TubeCode & _in) const { return GetID() != _in.GetID(); }
    bool operator< (const Instruction_TubeCode & _in) const { return GetID() <  _in.GetID(); }
    bool operator<=(const Instruction_TubeCode & _in) const { return GetID() <= _in.GetID(); }
    bool operator> (const Instruction_TubeCode & _in) const { return GetID() >  _in.GetID(); }
    bool operator>=(const Instruction_TubeCode & _in) const { return GetID() >= _in.GetID(); }

    int GetID() const { return (int) info & ID_MASK; }
    int GetArgValue() const { return (int) ((info & ARG_MASK) >> ID_BITS); }

    bool HasCycleCost() const { return GetFlag(CYCLE_COST_BIT); }
    bool HasStability() const { return GetFlag(EXTRA_STABILITY_BIT); }

    Instruction_TubeCode & SetID(uint32_t new_id) {
      emp_assert((new_id & ID_MASK) == new_id);
      info = new_id;
      return *this;
    }
    Instruction_TubeCode & SetArgValue(int arg_value) {
      emp_assert((arg_value >> ARG_BITS) == 0 && "Argument too large to store in Instruction_TubeCode");
      info &= ~ARG_MASK;               // Clear out current arg contenst of instruction.
      info |= (arg_value << ID_BITS);  // Set new arg contents of instruction.
      return *this;
    }

    Instruction_TubeCode & SetCycleCost() { SetFlag(CYCLE_COST_BIT); return *this; }
    Instruction_TubeCode & SetStability() { SetFlag(EXTRA_STABILITY_BIT); return *this; }

    Instruction_TubeCode & ClearCycleCost() { ClearFlag(CYCLE_COST_BIT); return *this; }
    Instruction_TubeCode & ClearStability() { ClearFlag(EXTRA_STABILITY_BIT); return *this; }
  };

};

#endif
