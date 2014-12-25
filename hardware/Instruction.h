#ifndef EMP_INSTRUCTION_H
#define EMP_INSTRUCTION_H

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// This class represents a single instruction in a genome.  It holds the information about which
// instruction we are working with as well as any relevant flags about that instruction.
//
// The entire instruction is stores in a 32-bit unsgined int.  The first 10 bits are set aside
// for the instruction ID.  The next 6 are for quick-check information about the instruction
// (such as can it be used as an argument for other instructions?), and the final 16 bits are for
// tracking run-time information (has this site been mutated?  If recombined, which parent is it
// from, etc.)

#include <assert.h>

namespace emp {

  class Instruction {
  public:
    typedef const unsigned int ctype;                    // Type for all constants in definition...
    static ctype ID_BITS = 10;                           // # of bits to track instruction ID.
    static ctype ID_MASK = (1 << ID_BITS) - 1;           // Mask to extract the ID (also, ID max)
    static ctype FIXED_FLAG_BITS = 6;                    // Bits to store fixed inst info.
    static ctype FIXED_FLAG_MASK =
                ((1 << FIXED_FLAG_BITS)-1) << ID_BITS;   // Mask to extract just fixed flags.
    static ctype COPY_BITS = ID_BITS + FIXED_FLAG_BITS;  // How many bits need to be copied?
    static ctype COPY_MASK = (1 << COPY_BITS)-1;         // Mask to extract the copiable bits
    static ctype VAR_FLAG_BITS = 32 - ID_BITS;           // Number of bits leftover for flags
    static ctype VAR_FLAG_MASK = ~COPY_MASK;             // Mask to extract run-time varuable bits
    static ctype FLAG_BITS = 32 - ID_BITS;               // Total of all flags

    static ctype ARG_BIT = 10;                           // Can this instruction be an argument?
    static ctype CYCLE_COST_BIT = 11;                    // Does this inst cost more than 1 cycle?
    static ctype COPY_COST_BIT = 12;                     // Does this inst have extra copy costs?


  private:
    unsigned int info;  // Full information about this instruction; both ID and flags.

  public:
    Instruction(unsigned int id=0) : info(id) { ; }
    Instruction(const Instruction & in_inst) : info(in_inst.info & ID_MASK) { ; }
    ~Instruction() { ; }

    Instruction & operator=(const Instruction & _in) {
      SetID(_in.GetID());
      return *this;
    }

    // Comparison operators ignore flags.
    bool operator==(const Instruction & _in) const { return GetID() == _in.GetID(); }
    bool operator!=(const Instruction & _in) const { return GetID() != _in.GetID(); }
    bool operator< (const Instruction & _in) const { return GetID() <  _in.GetID(); }
    bool operator<=(const Instruction & _in) const { return GetID() <= _in.GetID(); }
    bool operator> (const Instruction & _in) const { return GetID() >  _in.GetID(); }
    bool operator>=(const Instruction & _in) const { return GetID() >= _in.GetID(); }

    int GetID() const { return (int) info & ID_MASK; }
    bool GetFlag(int id) const {
      assert(id < FLAG_BITS);
      return (info & (1 << (id + ID_BITS))) != 0;
    }

    Instruction & SetID(int new_id) {
      assert((new_id & ID_MASK) == new_id);
      info = (unsigned int) new_id;
      return *this;
    }
    Instruction & SetFlag(int id, bool value=true) {
      assert(id >= 0 && id < FLAG_BITS);
      if (value) info |= 1 << (id + ID_BITS);
      else info &= ~(1 << (id + ID_BITS));
      return *this;
    }
  };

};

#endif
