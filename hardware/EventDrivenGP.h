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

// @amlalejini - TODO's:
//   [ ] Go through and make convenient/obvious accessors for structs (Event, State, Block, etc.)

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
      bool is_main;

      State(Ptr<memory_t> _shared_mem_ptr, bool _is_main=false)
        : shared_mem_ptr(_shared_mem_ptr), local_mem(), input_mem(), output_mem(),
          func_ptr(0), inst_ptr(0), block_stack(), is_main(_is_main) { ; }

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
      BitVector affinity;

      Instruction(size_t _id=0, size_t a0=0, size_t a1=0, size_t a2=0, const BitVector & _aff=BitVector())
        : id(_id), args(), affinity(_aff) { args[0] = a0; args[1] = a1; args[2] = a2; }
      Instruction(const Instruction &) = default;
      Instruction(Instruction &&) = default;

      Instruction & operator=(const Instruction &) = default;
      Instruction & operator=(Instruction &&) = default;

      void Set(size_t _id, size_t _a0=0, size_t _a1=0, size_t _a2=0, const BitVector & _aff=BitVector())
        { id = _id; args[0] = _a0; args[1] = _a1; args[2] = _a2; affinity = _aff; }

    };

    using inst_t = Instruction;
    using inst_lib_t = InstLib<EventDrivenGP>;

    struct Function {
      BitVector affinity;
      emp::vector<inst_t> inst_seq;

      Function(const BitVector & _aff=BitVector()) : affinity(_aff), inst_seq() { ; }

      size_t GetSize() const { return inst_seq.size(); }
    };

    using program_t = emp::vector<Function>;

  protected:
    Ptr<inst_lib_t> inst_lib;      // Instruction library.
    Ptr<memory_t> shared_mem_ptr;  // Pointer to shared memory.
    program_t program; // Program (set of functions).
    emp::vector< Ptr< emp::vector<Ptr<State>> > > execution_stacks;
    Ptr<emp::vector<Ptr<State>>> cur_core;
    std::queue<Event> event_queue;


    // - Execution -
    /// Close current block in cur_state if there is one to close. If not, do nothing.
    /// Handles closure of known, special block types appropriately:
    ///   * LOOPS - set cur_state's IP to beginning of block.
    void CloseBlock() {
      emp_assert(GetCurState());  // Can't have no nullptrs 'round here.
      Ptr<State> cur_state = GetCurState();
      // If there aren't any blocks to close, just return.
      if (cur_state->block_stack.empty()) return;
      Block & block = cur_state->block_stack.back();
      // Any special circumstances (e.g. looping) go below:
      switch (block.type) {
        case BlockType::LOOP:
          // Move IP to beginning of block.
          cur_state->inst_ptr = block.begin;
          break;
        default:
          break;
      }
      // Pop the block.
      cur_state->block_stack.pop_back();
    }

    /// Return from current function call (cur_state) in current core (cur_core).
    /// Upon returning, put values in output memory of returning state into local memory of caller state.
    void ReturnFunction() {
      emp_assert(GetCurState());
      // Grab the returning state and then pop it off the call stack.
      Ptr<State> returning_state = GetCurState();
      GetCurExecStack()->pop_back();
      // Is there anything to return to?
      if (Ptr<State> caller_state = GetCurState()) {
        // If so, copy returning state's output memory into caller state's local memory.
        for (auto mem : returning_state->output_mem) {
          caller_state->local_mem[mem.first] = mem.second;
        }
      }
    }

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

    // -- Control --
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

    // -- Accessors --
    Ptr<inst_lib_t> GetInstLib() const { return inst_lib; }
    const Function & GetFunction(size_t fID) const { emp_assert(fID < program.size()); return program[fID]; }
    inst_t GetInst(size_t fID, size_t pos) {
      emp_assert(ValidPosition(fID, pos));
      return program[fID].inst_seq[pos];
    }
    const program_t & GetProgram() const { return program; }
    Ptr<emp::vector<Ptr<State>>> GetCurExecStack() { return cur_core; }
    Ptr<State> GetCurState() {
      if (cur_core && !cur_core->empty()) return cur_core->back();
      else return nullptr;
    }
    bool ValidPosition(size_t fID, size_t pos) const { return fID < program.size() && pos < program[fID].GetSize(); }

    // -- Configuration --
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

    // -- Execution --
    /// Process a single instruction, provided by the caller.
    void ProcessInst(const inst_t & inst) { inst_lib->ProcessInst(*this, inst); }
    /// Advance hardware by single instruction.
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
        const size_t ip = cur_state->inst_ptr;
        const size_t fp = cur_state->func_ptr;
        // fp needs to be valid here (and always, really). Shame shame if it's not.
        emp_assert(fp < program.size());
        // If instruction pointer hanging off end of function sequence:
        if (ip >= program[fp].GetSize()) {
          if (!cur_state->block_stack.empty()) {
            //    - If there's a block to close, close it.
            CloseBlock();
          } else if (cur_state->is_main && cur_core->size() == 1) {
            //    - If this is the main function, and we're at the bottom of the call stack, wrap.
            cur_state->inst_ptr = 0;
          } else {
            //    - Otherwise, return from function call.
            ReturnFunction();
          }
        } else { // If instruction pointer is valid:
          // First, advance the instruction pointer by 1. This may invalidate the IP, but that's okay.
          cur_state->inst_ptr += 1;
          // Run instruction @ fp, ip.
          inst_lib->ProcessInst(*this, program[fp].inst_seq[ip]);
        }
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
    /// Advance hardware by some number instructions.
    void Process(size_t num_inst) { for (size_t i = 0; i < num_inst; i++) SingleProcess(); }

    // -- Printing --
    /// Print out a single instruction with its arguments.
    void PrintInst(const inst_t & inst, std::ostream & os=std::cout) {
      os << inst_lib->GetName(inst.id);
      const size_t num_args = inst_lib->GetNumArgs(inst.id);
      for (size_t i = 0; i < num_args; i++) {
        os << ' ' << inst.args[i];
      }
    }

    /// Print out entire program.
    void PrintProgram(std::ostream & os=std::cout) {
      for (size_t fID = 0; fID < program.size(); fID++) {
        // Print out function name (affinity).
        os << "Fn-" << fID << " " << program[fID].affinity << ":\n";
        int depth = 0;
        for (size_t i = 0; i < program[fID].GetSize(); i++) {
          const inst_t & inst = program[fID].inst_seq[i];
          int num_spaces = 2 + (2 * depth);
          for (int s = 0; s < num_spaces; s++) os << ' ';
          PrintInst(inst, os);
          os << '\n';
          // TODO: increase depth on instructions that define a new code block.
          if (inst_lib->IsBlockDef(inst.id)) {
            // is block def?
            depth++;
          } else if (inst_lib->GetName(inst.id) == "Close" && depth > 0) { // TODO: make block close determination better.
            // is block close?
            depth--;
          }
        }
        os << '\n';
      }
    }

    /// Print out current state (full) of virtual hardware.
    // @amlalejini - TODO: print instruction affinity if instruciton has an affinity.
    void PrintState(std::ostream & os=std::cout) {
      // Print format:
      //  Shared memory: [Key:value, ....]
      //  Core 0:
      //    Call Stack (stack size):
      //      State
      //      ---
      //      State
      //      ---
      //    ...

      // Print shared memory
      os << "Shared memory: ";
      for (auto mem : *shared_mem_ptr) os << '{' << mem.first << ':' << mem.second << '}'; os << '\n';
      // Print each core.
      for (size_t i = 0; i < execution_stacks.size(); i++) {
        const emp::vector<Ptr<State>> & stack = *(execution_stacks[i]);
        os << "Core " << i << ":\n" << "  Call stack (" << stack.size() << "):\n";
        for (size_t k = 0; k < stack.size(); k++) {
          // IP, FP, local mem, input mem, output mem
          const State & state = *(stack[k]);
          os << "Inst ptr: " << state.inst_ptr << "(";
          if (ValidPosition(state.func_ptr, state.inst_ptr))
            PrintInst(GetInst(state.func_ptr, state.inst_ptr), os);
          else
            os << "NONE";
          os << ")" << "\n";
          os << "Func ptr: " << state.func_ptr << "\n";
          os << "Input memory: ";
          for (auto mem : state.input_mem) os << "{" << mem.first << ":" << mem.second << "}"; os << "\n";
          os << "Local memory: ";
          for (auto mem : state.local_mem) os << "{" << mem.first << ":" << mem.second << "}"; os << "\n";
          os << "Output memory: ";
          for (auto mem : state.output_mem) os << "{" << mem.first << ":" << mem.second << "}"; os << "\n";
          os << "---\n";
        }
      }
    }

    // -- Default Instructions --
    // Instructions to implement:
    //  Mathematical computations:
    //    [ ] Inc
    //    [ ] Dec
    //    [ ] Not
    //    [ ] Add
    //    [ ] Sub
    //    [ ] Mult
    //    [ ] Div
    //    [ ] Mod
    //    [ ] TestEqu
    //    [ ] TestNEqu
    //    [ ] TestLess
    //  Flow control:
    //    [ ] If
    //    [ ] While
    //    [ ] Countdown
    //    [ ] Break
    //    [ ] Close
    //    [ ] Call
    //    [ ] Return
    //  Register Manipulation:
    //    [ ] SetMem
    //    [ ] CopyMem
    //    [ ] SwapMem
    //    [ ] Input  (Input mem => Local mem)
    //    [ ] Output (Local mem => Output mem)
    //    [ ] Commit (Local mem => Shared mem)
    //    [ ] Pull   (Shared mem => local mem)
    //  Misc.
    //    [ ] Nop

    static void Inst_Inc(EventDrivenGP & hw, const arg_set_t & args) { ; }
    static void Inst_Dec(EventDrivenGP & hw, const arg_set_t & args) { ; }
    static void Inst_Not(EventDrivenGP & hw, const arg_set_t & args) { ; }
    static void Inst_Add(EventDrivenGP & hw, const arg_set_t & args) { ; }
    static void Inst_Sub(EventDrivenGP & hw, const arg_set_t & args) { ; }
    static void Inst_Mult(EventDrivenGP & hw, const arg_set_t & args) { ; }
    static void Inst_Div(EventDrivenGP & hw, const arg_set_t & args) { ; }
    static void Inst_Mod(EventDrivenGP & hw, const arg_set_t & args) { ; }
    static void Inst_TestEqu(EventDrivenGP & hw, const arg_set_t & args) { ; }
    static void Inst_TestNEqu(EventDrivenGP & hw, const arg_set_t & args) { ; }
    static void Inst_TestLess(EventDrivenGP & hw, const arg_set_t & args) { ; }
    static void Inst_If(EventDrivenGP & hw, const arg_set_t & args) { ; }
    static void Inst_While(EventDrivenGP & hw, const arg_set_t & args) { ; }
    static void Inst_Countdown(EventDrivenGP & hw, const arg_set_t & args) { ; }
    static void Inst_Break(EventDrivenGP & hw, const arg_set_t & args) { ; }
    static void Inst_Close(EventDrivenGP & hw, const arg_set_t & args) { ; }
    static void Inst_Call(EventDrivenGP & hw, const arg_set_t & args) { ; }
    static void Inst_Return(EventDrivenGP & hw, const arg_set_t & args) { ; }
    static void Inst_SetMem(EventDrivenGP & hw, const arg_set_t & args) { ; }
    static void Inst_CopyMem(EventDrivenGP & hw, const arg_set_t & args) { ; }
    static void Inst_SwapMem(EventDrivenGP & hw, const arg_set_t & args) { ; }
    static void Inst_Input(EventDrivenGP & hw, const arg_set_t & args) { ; }
    static void Inst_Output(EventDrivenGP & hw, const arg_set_t & args) { ; }
    static void Inst_Commit(EventDrivenGP & hw, const arg_set_t & args) { ; }
    static void Inst_Pull(EventDrivenGP & hw, const arg_set_t & args) { ; }
    static void Inst_Nop(EventDrivenGP & hw, const arg_set_t & args) { ; }



  };
}

#endif
