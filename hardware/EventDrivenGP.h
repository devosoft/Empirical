#ifndef EMP_EVENT_DRIVEN_GP_H
#define EMP_EVENT_DRIVEN_GP_H

#include <unordered_map>
#include <queue>
#include "InstLib.h"
#include "../tools/BitVector.h"
#include "../base/vector.h"
#include "../base/Ptr.h"
#include "../base/array.h"

// EventDrivenGP -- Event handling, core management, interprets instruction sequences.
//  * CPU responsibilities
//  * vector<vector<CPU_State*>*> execution_stacks;
//  * cur_core
//  * memory_t* shared_memory (responsible)
//  * vector<Function> program
// State (internal struct)
//  * local_memory
//  * input_buffer
//  * output_buffer

namespace emp {
  class EventDrivenGP {
  public:
    static constexpr size_t CPU_SIZE = 8;
    static constexpr size_t AFFINITY_WIDTH = 4;
    static constexpr size_t MAX_INST_ARGS = 3;
    static constexpr size_t MAX_CORES = 64;         // Maximum number of parallel execution stacks that can be spawned.
    static constexpr size_t MAX_CALL_DEPTH = 128;   // Maximum depth of calls per execution stack.

    using memory_t = std::unordered_map<int, double>;
    using arg_t = size_t;
    using arg_set_t = emp::array<arg_t, MAX_INST_ARGS>;

    enum class EventType { NONE=0, MSG, SIGNAL };
    struct Event {
      memory_t msg;
      EventType type;
      Event(EventType _type=EventType::NONE) : msg(), type(_type) { ; }
      Event(memory_t & _msg, EventType _type=EventType::NONE) : msg(_msg), type(_type) { ; }
    };

    enum class BlockType { NONE=0, BASIC, LOOP };
    struct Block {
      size_t begin;
      size_t end;
      BlockType type;
      Block(size_t _begin=0, size_t _end=0, BlockType _type=BlockType::BASIC)
        : begin(_begin), end(_end), type(_type) { ; }
    };

    struct State {
      Ptr<memory_t> shared_mem_ptr;
      memory_t local_mem;
      memory_t input_mem;
      memory_t output_mem;

      size_t func_ptr;
      size_t inst_ptr;
      emp::vector<Block> block_stack;

      State(Ptr<memory_t> _shared_mem_ptr)
        : shared_mem_ptr(_shared_mem_ptr), local_mem(), input_mem(), output_mem(),
          func_ptr(0), inst_ptr(0), block_stack() { ; }

      void Reset() {
        local_mem.clear();
        input_mem.clear();
        output_mem.clear();
        func_ptr = 0; inst_ptr = 0;
        block_stack.clear();
      }
    };

    struct Instruction {
      size_t id;
      arg_set_t args;

      Instruction(size_t _id=0, size_t a0=0, size_t a1=0, size_t a2=0)
        : id(_id), args() { args[0] = a0; args[1] = a1; args[2] = a2; }
      Instruction(const Instruction &) = default;
      Instruction(Instruction &&) = default;

      Instruction & operator=(const Instruction &) = default;
      Instruction & operator=(Instruction &&) = default;

      void Set(size_t _id, size_t _a0=0, size_t _a1=0, size_t _a2=0)
        { id = _id; args[0] = _a0; args[1] = _a1; args[2] = _a2; }

    };

    using inst_t = Instruction;
    using inst_lib_t = InstLib<EventDrivenGP>;

    struct Function {
      BitVector affinity;
      emp::vector<inst_t> inst_seq;

      Function() : affinity(), inst_seq() { ; }
      Function(BitVector & _aff) : affinity(_aff), inst_seq() { ; }
    };

    using program_t = emp::vector<Function>;

  protected:
    Ptr<inst_lib_t> inst_lib;      // Instruction library.
    Ptr<memory_t> shared_mem_ptr;  // Pointer to shared memory.
    program_t program; // Program (set of functions).
    emp::vector< Ptr< emp::vector<Ptr<State>> > > execution_stacks;
    Ptr<emp::vector<Ptr<State>>> cur_core;
    std::queue<Event> event_queue;

  public:
    // Constructors
    EventDrivenGP(Ptr<inst_lib_t> _ilib)
      : inst_lib(_ilib), shared_mem_ptr(new memory_t()), program(), execution_stacks(),
        cur_core(nullptr), event_queue()
    { ; }

    EventDrivenGP(inst_lib_t & _ilib) : EventDrivenGP(&_ilib) { ; }
    //EventDrivenGP() : EventDrivenGP(DefaultInstLib()) { ; }
    EventDrivenGP(const EventDrivenGP &) = default;
    EventDrivenGP(EventDrivenGP &&) = default;

    /// Destructor - clean up: execution stacks, shared memory.
    ~EventDrivenGP() {
      Reset();
      delete shared_mem_ptr;
    }

    /// Reset everything, including program.
    void Reset() {
      program.resize(0);  // Clear out program.
      ResetHardware();
    }

    /// Reset only CPU hardware stuff, not program.
    void ResetHardware() {
      shared_mem_ptr->clear();
      // Clear event queue
      while (!event_queue.empty()) event_queue.pop();
      // Clear out execution stacks.
      for (size_t i = 0; i < execution_stacks.size(); i++) {
        for (size_t k = 0; k < execution_stacks[i]->size(); k++) {
          delete execution_stacks[i]->at(k);
        } delete execution_stacks[i];
      }
      execution_stacks.resize(0);
      cur_core = nullptr;
    }

    // Accessors
    Ptr<inst_lib_t> GetInstLib() const { return inst_lib; }
    const Function & GetFunction(size_t fID) const { emp_assert(fID < program.size()); return program[fID]; }
    inst_t GetInst(size_t fID, size_t pos) {
      emp_assert(fID < program.size() && pos < program[fID].inst_seq.size());
      return program[fID].inst_seq[pos];
    }
    const program_t & GetProgram() const { return program; }
    Ptr<emp::vector<Ptr<State>>> GetCurExecStack() { return cur_core; }
    Ptr<State> GetCurState() {
      if (cur_core) return cur_core->back();
      else return nullptr;
    }

    // Configuration
    void SetInst(size_t fID, size_t pos, const inst_t & inst) {
      emp_assert(fID < program.size() && pos < program[fID].inst_seq.size());
      program[fID].inst_seq[pos] = inst;
    }
    void SetInst(size_t fID, size_t pos, size_t id, size_t a0=0, size_t a1=0, size_t a2=0) {
      emp_assert(fID < program.size() && pos < program[fID].inst_seq.size());
      program[fID].inst_seq[pos].Set(id, a0, a1, a2);
    }
    void SetProgram(const program_t & _program) { program = _program; } // @amlalejini - TODO: test
    void AddFunction(const Function & _function) { program.emplace_back(_function); } // @amlalejini - TODO: test

    // Execution
    /// Process a single instruction, provided by the caller.
    void ProcessInst(const inst_t & inst) { inst_lib->ProcessInst(*this, inst); }
    /// Advance hardware by single unit of computational time.
    void SingleProcess() {
      // Distribute 1 unit of computational time to each core.
      size_t core_idx = 0;
      size_t core_cnt = execution_stacks.size();
      size_t adjust = 0;
      while (core_idx < core_cnt) {
        // Set the current core to core at core_idx.
        cur_core = execution_stacks[core_idx];
        // Do we need to move current core over in the execution core vector to make it contiguous?
        if (adjust) {
          execution_stacks[core_idx] = nullptr;
          execution_stacks[core_idx - adjust] = cur_core;
        }
        // Execute the core.
        //  * What function/instruction am I on?
        Ptr<State> cur_state = cur_core->back();
        // If instruction pointer hanging off end of function sequence:
        //    - If there's a block to close, close it.
        //    - If this is the main function, and we're at the bottom of the call stack, wrap.
        // If not:
        //  - Grab instruction.
        //  - Advance instruction pointer.
        //  - Run instruction.

        //inst_lib->ProcessInst(*this, program[cur_state->func_ptr].inst_seq[cur_state->inst_ptr]);

        // After processing, is the core still active?
        if (cur_core->empty()) {
          // Free core.
          delete execution_stacks[core_idx - adjust];
          execution_stacks[core_idx - adjust] = nullptr;
          adjust += 1;
        }
      }
      // Update execution stack size to be accurate.
      execution_stacks.resize(core_cnt - adjust);
    }

  };
}

#endif
