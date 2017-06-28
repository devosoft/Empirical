#ifndef EMP_META_GP_MACHINE_H
#define EMP_META_GP_MACHINE_H

#include <iostream>
#include <unordered_map>

#include "../../tools/BitVector.h"
#include "../../base/Ptr.h"

namespace emp {
  namespace MetaGP {

    enum class MachineType { SIMPLE_GP=0, RANDO_GP, GP_X, GP_Y, UNKNOWN };

    // Base class for machine states.
    // Machine state should indicate corresponding machine type.
    // Every machine internal state must have standardized:
    //  * ptr to shared_memory (machine state not responsible for cleanup)
    //  * local memory
    //  * input buffer
    //  * output buffer
    class MachineState_Base {
    protected:
      using memory_t = std::unordered_map<int, double>;

      MachineType type;
      Ptr<memory_t> shared_memory_ptr;
      memory_t local_memory;
      memory_t input_buffer;
      memory_t output_buffer;

    public:
      MachineState_Base()
      : type(MachineType::UNKNOWN), shared_memory_ptr(nullptr), local_memory(), input_buffer(), output_buffer()
      { ; }

      MachineType GetType() const { return type; }
      Ptr<memory_t> GetSharedMemoryPtr() { return shared_memory_ptr; }
      memory_t & GetLocalMemory() { return local_memory; }
      memory_t & GetInputBuffer() { return input_buffer; }
      memory_t & GetOutputBuffer() { return output_buffer; }

      void SetType(MachineType _type) { type = _type; }
      void SetSharedMemory(Ptr<memory_t> _shared_memory_ptr) { shared_memory_ptr = _shared_memory_ptr; }

    };

    class Machine_Base {
    protected:
      using machine_state_t = MachineState_Base;
      BitVector affinity;
      MachineType type;

    public:
      Machine_Base(BitVector & _affinity, MachineType _type)
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
  }
}

#endif
