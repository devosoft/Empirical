#ifndef EMP_INSTRUCTION_H
#define EMP_INSTRUCTION_H

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// This class represents a single instruction in a genome.  It holds the information about which
// instruction we are working with as well as any relevant flags about that instruction.
//

#include <assert.h>

namespace emp {

  class Instruction {
  public:
    static const unsigned int ID_BITS = 10;                  // Bits to track which instruction this is.
    static const unsigned int ID_MASK = (1 << ID_BITS) - 1;  // Mask to extract the ID (also, ID max)
    static const unsigned int FLAG_BITS = 32 - ID_BITS;      // Number of bits leftover for flags.

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
      assert(new_id & ID_MASK == new_id);
      info = (unsigned int) new_id;
    }
    Instruction & SetFlag(int id, bool value=true) {
      assert(id < FLAG_BITS);
      if (value) info |= 1 << (id + ID_BITS);
      else info &= ~(1 << (id + ID_BITS));
      return *this;
    }
  };

};

#endif
