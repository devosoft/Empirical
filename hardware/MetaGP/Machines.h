#ifndef EMP_META_GP_MACHINES_H
#define EMP_META_GP_MACHINES_H

#include <iostream>
#include <unordered_map>

#include "../../tools/BitVector.h"
#include "../../base/Ptr.h"

namespace emp {
  namespace MetaGP {

    enum class MachineType { EVENT_DRIVEN_GP=0, RANDO_GP, GP_X, GP_Y, UNKNOWN };

    // Base class for machine states.
    // Machine state should indicate corresponding machine type.
    // Every machine internal state must have standardized:
    //  * ptr to shared_memory (machine state not responsible for cleanup)
    //  * local memory
    //  * input buffer
    //  * output buffer
    class MachineState_Base {
    protected:
      MachineType type;
      std::unordered_map<int, double> * shared_memory_ptr;
      std::unordered_map<int, double> local_memory;
      std::unordered_map<int, double> input_buffer;
      std::unordered_map<int, double> output_buffer;

    public:
      MachineState_Base()
      : type(MachineType::UNKNOWN), shared_memory_ptr(nullptr), local_memory(), input_buffer(), output_buffer()
      { ; }

      // @amlalejini - TODO: Getter/setter for type
      // @amlalejini - TODO: shared/local memory & input/output buffer manipulation.

    };

    class Machine_Base {
    protected:
      using machine_state_t = MachineState_Base;
      BitVector affinity;
      MachineType type;

    public:
      Machine_Base(BitVector _affinity, MachineType _type)
        : affinity(_affinity), type(_type) { ; }

      virtual ~Machine_Base() { ; }

      BitVector & GetAffinity() { return affinity; }
      const BitVector & GetConstAffinity() const { return affinity; }
      MachineType GetType() { return type; }

      void SetAffinity(BitVector & aff) { affinity = aff; }
      void SetType(MachineType t) { type = t; }

      virtual void SingleProcess(Ptr<machine_state_t> internal_state) { ; }

      virtual std::string PrettyString() const {
        std::stringstream pretty_str;
        pretty_str << "def unknown " << affinity << std::endl;
        return pretty_str.str();
      }
    };

    enum class BlockType { NONE=0, BASIC, LOOP };

    struct Block {
      size_t begin;
      size_t end;
      BlockType type;

      Block(size_t _begin = 0, size_t _end = 0, BlockType _type = BlockType::BASIC)
        : begin(_begin), end(_end), type(_type) { ; }
    };

  }
}

#endif
