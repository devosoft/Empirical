// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

#ifndef EMP_INSTRUCTION_EVO_H
#define EMP_INSTRUCTION_EVO_H

//////////////////////////////////////////////////////////////////////////////////////////
//
// This class represents a single instruction in a program, with no arguments.
// It holds the information about which operation it is associated with as well as any
// relevant flags about that operation.
//
// The entire instruction is stored in a 32-bit unsgined int.  The first 10 bits are set
// aside for the instruction ID (max 1024 distinct instructions).  The next 6 are for
// quick-check information about the instruction (such as can it be used as an argument
// for other instructions?), and the final 16 bits are for tracking run-time information
// (has this site been mutated?  If recombined, which parent is it from, etc.)
//
// Since this instruction type was designed for use in genomes in large populations of
// evolving computer programs, the main goals were to keep it small and allow for rapid
// execution.
//

#include "../tools/assert.h"

namespace emp {

  class Instruction_Evo {
  private:
    typedef const uint32_t ctype;                         // Type for all constants in definition.
    static ctype ID_BITS = 10;                            // # of bits to track instruction ID.
    static ctype ID_MASK = (1 << ID_BITS) - 1;            // Mask to extract the ID (also, ID max)
    static ctype ARG_BITS = 6;                            // Allows for 64 distinct arguments.
    static ctype ARG_MASK = ((1<<ARG_BITS)-1) << ID_BITS; // Mask to extract the argument info.

    // We have 32 bits left.  We can either specify them, as below, or keep them flexible.
    static ctype CYCLE_COST_BIT = 16;                     // Does this inst cost more than 1 cycle?
    static ctype EXTRA_STABILITY_BIT = 17;                // Is this inst less likely to be mutated?

    static ctype FIXED_BIT_COUNT = 18;                    // How many bits to be copied w/ instruction?
    static ctype FIXED_BIT_MASK = (1<<FIXED_BIT_COUNT)-1; // Mask for copying instructions


    bool GetFlag(int id) const { return (info >> id) & 1; }
    void SetFlag(int id) { info |= (1 << id); }
    void ClearFlag(int id) { info &= ~(1 << id); }

    uint32_t info;  // Full information about this instruction; both ID and flags.

  public:
    Instruction_Evo(uint32_t id=0, uint32_t arg=0,
                bool extra_cycle_cost=false, bool extra_stability=false)
      : info(id + (arg<<ID_BITS))
    {
      emp_assert((id >> ID_BITS)   == 0 && "Too many bits in id!");
      emp_assert((arg >> ARG_BITS) == 0 && "Too many bits in arg!");
      if (extra_cycle_cost) SetCycleCost();
      if (extra_stability) SetStability();
    }
    Instruction_Evo(const Instruction_Evo & in_inst) : info(in_inst.info & FIXED_BIT_MASK) { ; }
    ~Instruction_Evo() { ; }

    Instruction_Evo & operator=(const Instruction_Evo & _in) {
      info = _in.info & FIXED_BIT_MASK;
      return *this;
    }

    // Comparison operators ignore flags.
    bool operator==(const Instruction_Evo & _in) const { return GetID() == _in.GetID(); }
    bool operator!=(const Instruction_Evo & _in) const { return GetID() != _in.GetID(); }
    bool operator< (const Instruction_Evo & _in) const { return GetID() <  _in.GetID(); }
    bool operator<=(const Instruction_Evo & _in) const { return GetID() <= _in.GetID(); }
    bool operator> (const Instruction_Evo & _in) const { return GetID() >  _in.GetID(); }
    bool operator>=(const Instruction_Evo & _in) const { return GetID() >= _in.GetID(); }

    int GetID() const { return (int) info & ID_MASK; }
    int GetArgValue() const { return (int) ((info & ARG_MASK) >> ID_BITS); }

    bool HasCycleCost() const { return GetFlag(CYCLE_COST_BIT); }
    bool HasStability() const { return GetFlag(EXTRA_STABILITY_BIT); }

    Instruction_Evo & SetID(uint32_t new_id) {
      emp_assert((new_id & ID_MASK) == new_id);
      info = new_id;
      return *this;
    }
    Instruction_Evo & SetArgValue(int arg_value) {
      emp_assert((arg_value >> ARG_BITS) == 0 && "Argument too large to store in Instruction_Evo");
      info &= ~ARG_MASK;               // Clear out current arg contenst of instruction.
      info |= (arg_value << ID_BITS);  // Set new arg contents of instruction.
      return *this;
    }

    Instruction_Evo & SetCycleCost() { SetFlag(CYCLE_COST_BIT); return *this; }
    Instruction_Evo & SetStability() { SetFlag(EXTRA_STABILITY_BIT); return *this; }

    Instruction_Evo & ClearCycleCost() { ClearFlag(CYCLE_COST_BIT); return *this; }
    Instruction_Evo & ClearStability() { ClearFlag(EXTRA_STABILITY_BIT); return *this; }
  };

};

#endif
