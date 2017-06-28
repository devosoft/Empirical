#ifndef EMP_META_GP_MACHINE_SIMPLE_GP_H
#define EMP_META_GP_MACHINE_SIMPLE_GP_H

#include <iostream>
#include <unordered_map>

#include "Machine.h"
#include "../InstLib.h"
#include "../../tools/BitVector.h"
#include "../../base/Ptr.h"
#include "../../base/vector.h"
#include "../../base/array.h"

namespace emp {
  namespace MetaGP {

    namespace {
      enum class BlockType { NONE=0, BASIC, LOOP };

      struct Block {
        size_t begin;
        size_t end;
        BlockType type;

        Block(size_t _begin = 0, size_t _end = 0, BlockType _type = BlockType::BASIC)
          : begin(_begin), end(_end), type(_type) { ; }
      };

    }

    // Simple, AvidaGP-like linear GP CPU internal state representation.
    class MachineState_SimpleGP : public MachineState_Base {
    protected:
      using MachineState_Base::type;
      using MachineState_Base::shared_memory_ptr;
      using MachineState_Base::local_memory;
      using MachineState_Base::input_buffer;
      using MachineState_Base::output_buffer;

      size_t ip;                         // Instruction pointer.
      emp::vector<Block> block_stack; // Stack for keeping track of execution blocks.

    public:
      MachineState_SimpleGP()
        : MachineState_Base()
      {
        SetType(MachineType::SIMPLE_GP);
      }

      // Reset all state-local things (not shared memory).
      void Reset() {
        ip = 0;
        block_stack.resize(0);
        local_memory.clear();
        input_buffer.clear();
        output_buffer.clear();
      }

      size_t GetIP() const { return ip; }
      emp::vector<Block> & GetBlockStack() { return block_stack; }

      void SetIP(size_t _ip) { ip = _ip; }

      // Block stack manipulation.
      bool IsBlock() { return !block_stack.empty(); }
      Block & PeekBlock() { emp_assert(IsBlock()); return block_stack.back(); }
      void PopBlock() { emp_assert(IsBlock()); block_stack.pop_back(); }


    };

    // Simple, AvidaGP-like linear GP CPU implementation.
    class Machine_SimpleGP : public Machine_Base {
    public:
      static constexpr size_t CPU_SIZE = 8;
      static constexpr size_t MAX_INST_ARGS = 3;

      using arg_t = size_t;
      using arg_set_t = emp::array<arg_t, MAX_INST_ARGS>;

      using Machine_Base::type;
      using Machine_Base::affinity;

      struct Instruction {
        size_t id;
        arg_set_t args;

        Instruction(size_t _id=0, size_t a0=0, size_t a1=0, size_t a2=0)
          : id(_id), args() { args[0] = a0; args[1] = a1; args[2] = a2; }
        Instruction(const Instruction & ) = default;
        Instruction(Instruction &&) = default;

        Instruction & operator=(const Instruction &) = default;
        Instruction & operator=(Instruction &&) = default;

        void Set(size_t _id, size_t _a0=0, size_t _a1=0, size_t _a2=0) {
          id = _id; args[0] = _a0; args[1] = _a1; args[2] = _a2;
        }

      };

      using inst_t = Instruction;
      using inst_lib_t = InstLib<Machine_SimpleGP>;
      using state_t = MachineState_SimpleGP;
    protected:
      Ptr<inst_lib_t> inst_lib;
      // CPU components.
      emp::vector<inst_t> inst_sequence;  // Instruction sequence.
      Ptr<state_t> state;                 // Current internal state information (not responsible for cleanup).

    public:
      Machine_SimpleGP(BitVector & _affinity, Ptr<inst_lib_t> _ilib, Ptr<state_t> _state = nullptr)
        : Machine_Base(_affinity, MachineType::SIMPLE_GP), inst_lib(_ilib),
          inst_sequence(), state(_state)
      {
        Reset();
      }

      Machine_SimpleGP(BitVector & _affinity, inst_lib_t & _ilib, Ptr<state_t> _state = nullptr)
        : Machine_SimpleGP(_affinity, &_ilib, _state)
      { ; }

      // Machine_SimpleGP(BitVector _affinity, Ptr<state_t> _state = nullptr)
      //   : Machine_SimpleGP(_affinity, DefaultInstLib(), _state) { ; }
      Machine_SimpleGP(BitVector & _affinity, Ptr<state_t> _state = nullptr)
        : Machine_Base(_affinity, MachineType::SIMPLE_GP), inst_lib(), inst_sequence(), state(_state) { ; }

      // Reset entire machine to starting state, without an instruction sequence.
      void Reset() {
        inst_sequence.resize(0); // Clear out instruction sequence.
        ResetMachine();          // Full machine reset.
      }

      // Reset just the hardware, keep the instruction sequence.
      void ResetMachine() {
        if (state) state->Reset();
      }

      // Accessors
      Ptr<inst_lib_t> GetInstLib() const { return inst_lib; }
      inst_t GetInst(size_t pos) const { emp_assert(pos < inst_sequence.size()); return inst_sequence[pos]; }
      const emp::vector<inst_t> & GetInstSequence() const { return inst_sequence; }
      // double GetLocal(size_t id) const { emp_assert(state); return state->GetLocalMemory()[id]; }

      // static Ptr<inst_lib_t> DefaultInstLib() {
      //
      // }

    };

  }
}

#endif
