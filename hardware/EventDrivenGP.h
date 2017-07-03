#ifndef EMP_EVENT_DRIVEN_GP_H
#define EMP_EVENT_DRIVEN_GP_H

#include <unordered_map>
#include <queue>
#include "InstLib.h"
#include "../tools/BitSet.h"
#include "../tools/map_utils.h"
#include "../tools/Random.h"
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

// Notes:
//  * Important concept: Main state (bottom-most call state on core 0's call stack).
//    * The first function will be main (unless the fp on the initially created state is otherwise manipulated).
//    * Main state behaves differently than any other state.

// @amlalejini - TODO's:
//  [ ] Go through and make convenient/obvious accessors for structs (Event, State, Block, etc.)
//  [x] Switch from BitVector affinities -> BitSet affinities (affinity_t)
//  [ ] Completely switch to using Ptr.h's .New()/.Delete()? Or, wait until Charles gets the new/delete operators working.

namespace emp {
  class EventDrivenGP {
  public:
    static constexpr size_t CPU_SIZE = 8;
    static constexpr size_t AFFINITY_WIDTH = 4;
    static constexpr size_t MAX_INST_ARGS = 3;
    static constexpr size_t MAX_CORES = 64;         // Maximum number of parallel execution stacks that can be spawned.
    static constexpr size_t MAX_CALL_DEPTH = 128;   // Maximum depth of calls per execution stack.
    static constexpr double DEFAULT_MEM_VALUE = 0.0;
    static constexpr double MIN_BIND_THRESH = 0.5;

    using mem_key_t = int;
    using mem_val_t = double;
    using memory_t = std::unordered_map<mem_key_t, mem_val_t>;
    using arg_t = int;
    using arg_set_t = emp::array<arg_t, MAX_INST_ARGS>;
    using affinity_t = BitSet<AFFINITY_WIDTH>;

    enum class EventType { NONE=0, MSG, SIGNAL };
    struct Event {
      memory_t msg;
      EventType type;
      affinity_t affinity;
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

      size_t GetFP() const { return func_ptr; }
      size_t GetIP() const { return inst_ptr; }
      void SetIP(size_t ip) { inst_ptr = ip; }
      void SetFP(size_t fp) { func_ptr = fp; }
      void AdvanceIP(size_t inc = 1) { inst_ptr += inc; }
      bool IsMain() const { return is_main; }

      memory_t & GetLocalMemory() { return local_mem; }
      memory_t & GetInputMemory() { return input_mem; }
      memory_t & GetOutputMemory() { return output_mem; }

      /// GetXMemory functions return value at memory location if memory location exists.
      /// Otherwise, these functions return default memory value.
      mem_val_t GetLocal(mem_key_t key) {
        if (!Has(local_mem, key)) return DEFAULT_MEM_VALUE;
        return local_mem[key];
      }
      mem_val_t GetInput(mem_key_t key) {
        if (!Has(input_mem, key)) return DEFAULT_MEM_VALUE;
        return input_mem[key];
      }
      mem_val_t GetOutput(mem_key_t key) {
        if (!Has(output_mem, key)) return DEFAULT_MEM_VALUE;
        return output_mem[key];
      }
      mem_val_t GetShared(mem_key_t key) {
        if (!Has(*shared_mem_ptr, key)) return DEFAULT_MEM_VALUE;
        return (*shared_mem_ptr)[key];
      }

      /// SetXMemory functions set memory location (specified by key) to value.
      void SetLocal(mem_key_t key, mem_val_t value) { local_mem[key] = value; }
      void SetInput(mem_key_t key, mem_val_t value) { input_mem[key] = value; }
      void SetOutput(mem_key_t key, mem_val_t value) { output_mem[key] = value; }
      void SetShared(mem_key_t key, mem_val_t value) { (*shared_mem_ptr)[key] = value; }

      /// AccessXMemory functions return reference to memory location value if that location exists.
      /// If the location does not exist, set to default memory value and return reference to memory location value.
      mem_val_t & AccessLocal(mem_key_t key) {
        if (!Has(local_mem, key)) local_mem[key] = DEFAULT_MEM_VALUE;
        return local_mem[key];
      }
      mem_val_t & AccessInput(mem_key_t key) {
        if (!Has(input_mem, key)) input_mem[key] = DEFAULT_MEM_VALUE;
        return input_mem[key];
      }
      mem_val_t & AccessOutput(mem_key_t key) {
        if (!Has(output_mem, key)) output_mem[key] = DEFAULT_MEM_VALUE;
        return output_mem[key];
      }
      mem_val_t & AccessShared(mem_key_t key) {
        if (!Has(*shared_mem_ptr, key)) (*shared_mem_ptr)[key] = DEFAULT_MEM_VALUE;
        return (*shared_mem_ptr)[key];
      }

    };

    struct Instruction {
      size_t id;
      arg_set_t args;
      affinity_t affinity;

      Instruction(size_t _id=0, arg_t a0=0, arg_t a1=0, arg_t a2=0, const affinity_t & _aff=affinity_t())
        : id(_id), args(), affinity(_aff) { args[0] = a0; args[1] = a1; args[2] = a2; }
      Instruction(const Instruction &) = default;
      Instruction(Instruction &&) = default;

      Instruction & operator=(const Instruction &) = default;
      Instruction & operator=(Instruction &&) = default;

      void Set(size_t _id, arg_t _a0=0, arg_t _a1=0, arg_t _a2=0, const affinity_t & _aff=affinity_t())
        { id = _id; args[0] = _a0; args[1] = _a1; args[2] = _a2; affinity = _aff; }

    };

    using inst_t = Instruction;
    using inst_lib_t = InstLib<EventDrivenGP>;

    struct Function {
      affinity_t affinity;
      emp::vector<inst_t> inst_seq;

      Function(const affinity_t & _aff=affinity_t()) : affinity(_aff), inst_seq() { ; }

      size_t GetSize() const { return inst_seq.size(); }

    };

    using program_t = emp::vector<Function>;

  protected:
    Ptr<inst_lib_t> inst_lib;      // Instruction library.
    Ptr<memory_t> shared_mem_ptr;  // Pointer to shared memory.
    program_t program; // Program (set of functions).
    emp::vector< Ptr< emp::vector<Ptr<State>> > > execution_stacks;
    Ptr<emp::vector<Ptr<State>>> cur_core;
    std::queue<Event> event_queue;  // @amlalejini TODO - incorporate event queue into single process.
    size_t errors;
    Ptr<Random> random_ptr;
    bool random_owner;

  public:
    // Constructors
    EventDrivenGP(Ptr<inst_lib_t> _ilib, Ptr<Random> rnd=nullptr)
      : inst_lib(_ilib), shared_mem_ptr(), program(), execution_stacks(),
        cur_core(nullptr), event_queue(), errors(0), random_ptr(rnd), random_owner(false)
    {
      if (!rnd) NewRandom();
      shared_mem_ptr.New();
      // Spin up main core.
      execution_stacks.emplace_back(new emp::vector<Ptr<State>>());
      cur_core = execution_stacks[0];
      // Initialize/push main program state onto main execution core call stack.
      cur_core->emplace_back(new State(shared_mem_ptr, true));
    }
    EventDrivenGP(inst_lib_t & _ilib, Ptr<Random> rnd=nullptr) : EventDrivenGP(&_ilib, rnd) { ; }
    EventDrivenGP(Ptr<Random> rnd=nullptr) : EventDrivenGP(DefaultInstLib(), rnd) { ; }
    EventDrivenGP(const EventDrivenGP &) = default;
    EventDrivenGP(EventDrivenGP &&) = default;

    /// Destructor - clean up: execution stacks, shared memory.
    ~EventDrivenGP() {
      Reset();
      if (random_owner) random_ptr.Delete();
      shared_mem_ptr.Delete();
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
      errors = 0;
    }

    // -- Accessors --
    Ptr<inst_lib_t> GetInstLib() const { return inst_lib; }
    const Function & GetFunction(size_t fID) const { emp_assert(fID < program.size()); return program[fID]; }
    size_t GetNumErrors() const { return errors; }
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
    bool ValidFunction(size_t fID) const { return fID < program.size(); }
    double GetMinBindThresh() const { return MIN_BIND_THRESH; }

    // -- Configuration --
    // @amlalejini - TODO: add affinity to SetInst
    void SetInst(size_t fID, size_t pos, const inst_t & inst) {
      emp_assert(ValidPosition(fID, pos));
      program[fID].inst_seq[pos] = inst;
    }
    void SetInst(size_t fID, size_t pos, size_t id, arg_t a0=0, arg_t a1=0, arg_t a2=0,
                 const affinity_t & aff=affinity_t()) {
      emp_assert(ValidPosition(fID, pos));
      program[fID].inst_seq[pos].Set(id, a0, a1, a2, aff);
    }
    void SetProgram(const program_t & _program) { program = _program; } // @amlalejini - TODO: test
    void PushFunction(const Function & _function) { program.emplace_back(_function); } // @amlalejini - TODO: test
    //void PushFunction() { ; } // @amlalejini - TODO: make a push function that creates the function struct for you.

    /// Push new instruction to program.
    /// If no function pointer is provided and no functions exist yet, add new function to
    /// program and push to that. If no function pointer is provided and functions exist, push to
    /// last function in program. If function pointer is provided, push to that function.
    void PushInst(size_t id, arg_t a0=0, arg_t a1=0, arg_t a2=0,
                  const affinity_t & aff=affinity_t(), int fID=-1)
    {
      size_t fp;
      if (fID == -1 && !program.size()) { program.emplace_back(); fp = 0; }
      else if (fID == -1 || (fID < 0 && fID >= (int)program.size())) { fp = program.size() - 1; }
      else fp = (size_t)fID;
      program[fp].inst_seq.emplace_back(id, a0, a1, a2, aff);
    }

    /// Push new instruction to program.
    /// If no function pointer is provided and no functions exist yet, add new function to
    /// program and push to that. If no function pointer is provided and functions exist, push to
    /// last function in program. If function pointer is provided, push to that function.
    void PushInst(const std::string & name, arg_t a0=0, arg_t a1=0, arg_t a2=0,
                  const affinity_t & aff=affinity_t(), int fID=-1)
    {
      size_t fp;
      size_t id = inst_lib->GetID(name);
      if (fID == -1 && !program.size()) { program.emplace_back(); fp = 0; }
      else if (fID == -1 || (fID < 0 && fID >= (int)program.size())) { fp = program.size() - 1; }
      else fp = (size_t)fID;
      program[fp].inst_seq.emplace_back(id, a0, a1, a2, aff);
    }

    /// Push new instruction to program.
    /// If no function pointer is provided and no functions exist yet, add new function to
    /// program and push to that. If no function pointer is provided and functions exist, push to
    /// last function in program. If function pointer is provided, push to that function.
    void PushInst(const inst_t & inst, int fID=-1) {
      size_t fp;
      if (fID == -1 && !program.size()) { program.emplace_back(); fp = 0; }
      else if (fID == -1 || (fID < 0 && fID >= (int)program.size())) { fp = program.size() - 1; }
      else fp = (size_t)fID;
      program[fp].inst_seq.emplace_back(inst);
    }

    /// Load entire genome from input stream.
    bool Load(std::istream & input) { ; } // TODO

    void NewRandom(int seed=-1) {
      if (random_owner) random_ptr.Delete();
      random_ptr.New(seed);
      random_owner = true;
    }
    // -- Utilities --
    /// Given valid function pointer and instruction pointer, find next end of block (at current block level).
    /// This is not guaranteed to return a valid IP. At worst, it'll return an IP == function.inst_seq.size().
    size_t FindEndOfBlock(size_t fp, size_t ip) {
      emp_assert(ValidPosition(fp, ip));
      int depth_counter = 1;
      while (true) {
        if (!ValidPosition(fp, ip)) break;
        const inst_t inst = GetInst(fp, ip);
        if (inst_lib->HasProperty(inst.id, "block_def")) {
          ++depth_counter;
        } else if (inst_lib->HasProperty(inst.id, "block_close")) {
          --depth_counter;
          // If depth is ever 0 after subtracting, we've found the close for initial block.
          if (depth_counter == 0) break;
        }
        ++ip;
      }
      return ip;
    }

    /// Close current block in cur_state if there is one to close. If not, do nothing.
    /// Handles closure of known, special block types appropriately:
    ///   * LOOPS - set cur_state's IP to beginning of block.
    void CloseBlock() {
      emp_assert(GetCurState());  // Can't have no nullptrs 'round here.
      State & state = *GetCurState();
      // If there aren't any blocks to close, just return.
      if (state.block_stack.empty()) return;
      Block & block = state.block_stack.back();
      // Any special circumstances (e.g. looping) go below:
      switch (block.type) {
        case BlockType::LOOP:
          // Move IP to beginning of block.
          state.SetIP(block.begin);
          break;
        default:
          break;
      }
      // Pop the block.
      state.block_stack.pop_back();
    }

    void OpenBlock(size_t begin, size_t end, BlockType type) {
      emp_assert(GetCurState());
      State & state = *GetCurState();
      state.block_stack.emplace_back(begin, end, type);
    }

    /// If there's a block to break out of, break out (to eob).
    /// Otherwise, do nothing.
    void BreakBlock() {
      emp_assert(GetCurState());
      State & state = *GetCurState();
      if (!state.block_stack.empty()) {
        state.SetIP(state.block_stack.back().end);
        state.block_stack.pop_back();
        if (ValidPosition(state.GetFP(), state.GetIP())) state.AdvanceIP();
      }
    }

    /// Call function with best affinity match above threshold.
    /// If not candidate functions found, do nothing.
    void CallFunction(affinity_t affinity, double threshold) {
      emp_assert(GetCurState());
      // @amlalejini - TODO: memoize this function.
      size_t fID;
      double max_bind = -1;
      emp::vector<size_t> best_matches;
      for (size_t i=0; i < program.size(); i++) {
        double bind = SimpleMatchCoeff(program[i].affinity, affinity);
        if (bind == max_bind) best_matches.push_back(i);
        else if (bind > max_bind && bind >= threshold) {
          best_matches.resize(0);
          best_matches.push_back(i);
          max_bind = bind;
        }
      }
      if (best_matches.empty()) return;
      if (best_matches.size() == 1.0) fID = best_matches[0];
      else fID = best_matches[(size_t)random_ptr->GetUInt(0, best_matches.size())];
      CallFunction(fID);
    }

    /// Call function specified by fID.
    void CallFunction(size_t fID) {
      emp_assert(GetCurState() && ValidPosition(fID, 0));
      // Are we at max call depth? -- If so, call fails.
      if (GetCurExecStack()->size() >= MAX_CALL_DEPTH) return;
      // Grab pointer to caller.
      Ptr<State> caller_state = GetCurState();
      // Make a new state, push onto call stack.
      GetCurExecStack()->emplace_back(new State(shared_mem_ptr));
      Ptr<State> new_state = GetCurState();
      // Configure new state.
      new_state->SetFP(fID);
      new_state->SetIP(0);
      for (auto mem : caller_state->local_mem) {
        new_state->SetInput(mem.first, mem.second);
      }
    }

    /// Return from current function call (cur_state) in current core (cur_core).
    /// Upon returning, put values in output memory of returning state into local memory of caller state.
    void ReturnFunction() {
      emp_assert(GetCurState());
      // Grab the returning state and then pop it off the call stack.
      Ptr<State> returning_state = GetCurState();
      // No returning from main.
      if (returning_state->IsMain()) return;
      GetCurExecStack()->pop_back();
      // Is there anything to return to?
      if (Ptr<State> caller_state = GetCurState()) {
        // If so, copy returning state's output memory into caller state's local memory.
        for (auto mem : returning_state->output_mem) {
          caller_state->SetLocal(mem.first, mem.second);
        }
      }
      delete returning_state;
    }

    // -- Execution --
    /// Process a single instruction, provided by the caller.
    void ProcessInst(const inst_t & inst) { emp_assert(GetCurState()); inst_lib->ProcessInst(*this, inst); }
    /// Advance hardware by single instruction.
    void SingleProcess() {
      emp_assert(program.size()); // Must have a program before this is allowed.
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
        ++core_idx;
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
      if (inst_lib->HasProperty(inst.id, "affinity")) {
        os << ' '; inst.affinity.Print(os);
      }
      const size_t num_args = inst_lib->GetNumArgs(inst.id);
      for (size_t i = 0; i < num_args; i++) {
        os << ' ' << inst.args[i];
      }
    }

    /// Print out entire program.
    void PrintProgram(std::ostream & os=std::cout) {
      for (size_t fID = 0; fID < program.size(); fID++) {
        // Print out function name (affinity).
        os << "Fn-" << fID << " ";
        program[fID].affinity.Print(os);
        os << ":\n";
        int depth = 0;
        for (size_t i = 0; i < program[fID].GetSize(); i++) {
          const inst_t & inst = program[fID].inst_seq[i];
          int num_spaces = 2 + (2 * depth);
          for (int s = 0; s < num_spaces; s++) os << ' ';
          PrintInst(inst, os);
          os << '\n';
          if (inst_lib->HasProperty(inst.id, "block_def")) {
            // is block def?
            depth++;
          } else if (inst_lib->HasProperty(inst.id, "block_close") && depth > 0) {
            // is block close?
            depth--;
          }
        }
        os << '\n';
      }
    }

    /// Print out current state (full) of virtual hardware.
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
      os << "Errors: " << errors << "\n";
      // Print each core.
      for (size_t i = 0; i < execution_stacks.size(); ++i) {
        const emp::vector<Ptr<State>> & stack = *(execution_stacks[i]);
        os << "Core " << i << ":\n" << "  Call stack (" << stack.size() << "):\n    --TOP--\n";
        for (size_t k = stack.size() - 1; k < stack.size(); --k) {
          emp_assert(stack.size() != (size_t)-1);
          // IP, FP, local mem, input mem, output mem
          const State & state = *(stack[k]);
          os << "    Inst ptr: " << state.inst_ptr << " (";
          if (ValidPosition(state.func_ptr, state.inst_ptr))
            PrintInst(GetInst(state.func_ptr, state.inst_ptr), os);
          else
            os << "NONE";
          os << ")" << "\n";
          os << "    Func ptr: " << state.func_ptr << "\n";
          os << "    Input memory: ";
          for (auto mem : state.input_mem) os << "{" << mem.first << ":" << mem.second << "}"; os << "\n";
          os << "    Local memory: ";
          for (auto mem : state.local_mem) os << "{" << mem.first << ":" << mem.second << "}"; os << "\n";
          os << "    Output memory: ";
          for (auto mem : state.output_mem) os << "{" << mem.first << ":" << mem.second << "}"; os << "\n";
          os << "    ---\n";
        }
      }
    }

    // -- Default Instructions --
    // For all instructions: because memory is implemented as unordered_maps, instructions should
    //  gracefully handle the case where a particular memory position has yet to be used (doesn't
    //  exist in map yet).

    // Instructions to implement:
    //  Mathematical computations:
    //    [x] Inc
    //    [x] Dec
    //    [x] Not
    //    [x] Add
    //    [x] Sub
    //    [x] Mult
    //    [x] Div
    //    [x] Mod
    //    [x] TestEqu
    //    [x] TestNEqu
    //    [x] TestLess
    //  Flow control:
    //    [x] If
    //    [x] While
    //    [x] Countdown
    //    [x] Break
    //    [x] Close
    //    [x] Call
    //    [x] Return
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

    static void Inst_Inc(EventDrivenGP & hw, const inst_t & inst) {
      State & state = *hw.GetCurState();
      ++state.AccessLocal(inst.args[0]);
    }

    static void Inst_Dec(EventDrivenGP & hw, const inst_t & inst) {
      State & state = *hw.GetCurState();
      --state.AccessLocal(inst.args[0]);
    }

    static void Inst_Not(EventDrivenGP & hw, const inst_t & inst) {
      State & state = *hw.GetCurState();
      state.SetLocal(inst.args[0], state.GetLocal(inst.args[0]) == 0.0);
    }

    static void Inst_Add(EventDrivenGP & hw, const inst_t & inst) {
      State & state = *hw.GetCurState();
      state.SetLocal(inst.args[2], state.AccessLocal(inst.args[0]) + state.AccessLocal(inst.args[1]));
    }

    static void Inst_Sub(EventDrivenGP & hw, const inst_t & inst) {
      State & state = *hw.GetCurState();
      state.SetLocal(inst.args[2], state.AccessLocal(inst.args[0]) - state.AccessLocal(inst.args[1]));
    }

    static void Inst_Mult(EventDrivenGP & hw, const inst_t & inst) {
      State & state = *hw.GetCurState();
      state.SetLocal(inst.args[2], state.AccessLocal(inst.args[0]) * state.AccessLocal(inst.args[1]));
    }

    static void Inst_Div(EventDrivenGP & hw, const inst_t & inst) {
      State & state = *hw.GetCurState();
      const double denom = state.AccessLocal(inst.args[1]);
      if (denom == 0.0) ++hw.errors;
      else state.SetLocal(inst.args[2], state.AccessLocal(inst.args[0]) / denom);
    }

    static void Inst_Mod(EventDrivenGP & hw, const inst_t & inst) {
      State & state = *hw.GetCurState();
      const int base = (int)state.AccessLocal(inst.args[1]);
      if (base == 0) ++hw.errors;
      else state.SetLocal(inst.args[2], (int)state.AccessLocal(inst.args[0]) % base);
    }

    static void Inst_TestEqu(EventDrivenGP & hw, const inst_t & inst) {
      State & state = *hw.GetCurState();
      state.SetLocal(inst.args[2], state.AccessLocal(inst.args[0]) == state.AccessLocal(inst.args[1]));
    }

    static void Inst_TestNEqu(EventDrivenGP & hw, const inst_t & inst) {
      State & state = *hw.GetCurState();
      state.SetLocal(inst.args[2], state.AccessLocal(inst.args[0]) != state.AccessLocal(inst.args[1]));
    }

    static void Inst_TestLess(EventDrivenGP & hw, const inst_t & inst) {
      State & state = *hw.GetCurState();
      state.SetLocal(inst.args[2], state.AccessLocal(inst.args[0]) < state.AccessLocal(inst.args[1]));
    }

    static void Inst_If(EventDrivenGP & hw, const inst_t & inst) {
      State & state = *hw.GetCurState();
      // Find EOBLK.
      size_t eob = hw.FindEndOfBlock(state.GetFP(), state.GetIP());
      if (state.AccessLocal(inst.args[0]) == 0.0) {
        // Skip to EOBLK.
        state.SetIP(eob);
        // Advance past the block close if not at eofun.
        if (hw.ValidPosition(state.GetFP(), eob)) state.AdvanceIP();
      } else {
        // Open BLK
        hw.OpenBlock(state.GetIP() - 1, eob, BlockType::BASIC);
      }
    }

    static void Inst_While(EventDrivenGP & hw, const inst_t & inst) {
      State & state = *hw.GetCurState();
      size_t eob = hw.FindEndOfBlock(state.GetFP(), state.GetIP());
      if (state.AccessLocal(inst.args[0]) == 0.0) {
        // Skip to EOBLK.
        state.SetIP(eob);
        // Advance past the block close if not at eofun.
        if (hw.ValidPosition(state.GetFP(), eob)) state.AdvanceIP();
      } else {
        // Open blk.
        hw.OpenBlock(state.GetIP() - 1, eob, BlockType::LOOP);
      }
    }

    static void Inst_Countdown(EventDrivenGP & hw, const inst_t & inst) {
      State & state = *hw.GetCurState();
      size_t eob = hw.FindEndOfBlock(state.GetFP(), state.GetIP());
      if (state.AccessLocal(inst.args[0]) == 0.0) {
        // Skip to EOBLK.
        state.SetIP(eob);
        // Advance past the block close if not at eofun.
        if (hw.ValidPosition(state.GetFP(), eob)) state.AdvanceIP();
      } else {
        // Decrement Arg1
        --state.AccessLocal(inst.args[0]);
        // Open blk.
        hw.OpenBlock(state.GetIP() - 1, eob, BlockType::LOOP);
      }
    }

    static void Inst_Break(EventDrivenGP & hw, const inst_t & inst) {
      hw.BreakBlock();
    }

    static void Inst_Close(EventDrivenGP & hw, const inst_t & inst) {
      hw.CloseBlock();
    }

    static void Inst_Call(EventDrivenGP & hw, const inst_t & inst) {
      hw.CallFunction(inst.affinity, hw.GetMinBindThresh());
    }

    static void Inst_Return(EventDrivenGP & hw, const inst_t & inst) {
      hw.ReturnFunction();
    }

    static void Inst_SetMem(EventDrivenGP & hw, const inst_t & inst) { ; }
    static void Inst_CopyMem(EventDrivenGP & hw, const inst_t & inst) { ; }
    static void Inst_SwapMem(EventDrivenGP & hw, const inst_t & inst) { ; }
    static void Inst_Input(EventDrivenGP & hw, const inst_t & inst) { ; }
    static void Inst_Output(EventDrivenGP & hw, const inst_t & inst) { ; }
    static void Inst_Commit(EventDrivenGP & hw, const inst_t & inst) { ; }
    static void Inst_Pull(EventDrivenGP & hw, const inst_t & inst) { ; }
    static void Inst_Nop(EventDrivenGP & hw, const inst_t & inst) { ; }

    Ptr<InstLib<EventDrivenGP>> DefaultInstLib() {
      static inst_lib_t inst_lib;

      if (inst_lib.GetSize() == 0) {
        inst_lib.AddInst("Inc", Inst_Inc, 1, "Increment value in local memory Arg1");
        inst_lib.AddInst("Dec", Inst_Dec, 1, "Decrement value in local memory Arg1");
        inst_lib.AddInst("Not", Inst_Not, 1, "Logically toggle value in local memory Arg1");
        inst_lib.AddInst("Add", Inst_Add, 3, "Local memory: Arg3 = Arg1 + Arg2");
        inst_lib.AddInst("Sub", Inst_Sub, 3, "Local memory: Arg3 = Arg1 - Arg2");
        inst_lib.AddInst("Mult", Inst_Mult, 3, "Local memory: Arg3 = Arg1 * Arg2");
        inst_lib.AddInst("Div", Inst_Div, 3, "Local memory: Arg3 = Arg1 / Arg2");
        inst_lib.AddInst("Mod", Inst_Mod, 3, "Local memory: Arg3 = Arg1 % Arg2");
        inst_lib.AddInst("TestEqu", Inst_TestEqu, 3, "Local memory: Arg3 = (Arg1 == Arg2)");
        inst_lib.AddInst("TestNEqu", Inst_TestNEqu, 3, "Local memory: Arg3 = (Arg1 != Arg2)");
        inst_lib.AddInst("TestLess", Inst_TestLess, 3, "Local memory: Arg3 = (Arg1 < Arg2)");
        inst_lib.AddInst("If", Inst_If, 1, "Local memory: If Arg1 != 0, proceed; else, skip block.", ScopeType::BASIC, 0, {"block_def"});
        inst_lib.AddInst("While", Inst_While, 1, "Local memory: If Arg1 != 0, loop; else, skip block.", ScopeType::BASIC, 0, {"block_def"});
        inst_lib.AddInst("Countdown", Inst_Countdown, 1, "Local memory: Countdown Arg1 to zero.", ScopeType::BASIC, 0, {"block_def"});
        inst_lib.AddInst("Close", Inst_Close, 0, "Close current block if there is a block to close.", ScopeType::BASIC, 0, {"block_close"});
        inst_lib.AddInst("Break", Inst_Break, 0, "Break out of current block.");
        inst_lib.AddInst("Call", Inst_Call, 0, "Call function that best matches call affinity.", ScopeType::BASIC, 0, {"affinity"});
        inst_lib.AddInst("Return", Inst_Return, 0, "Return from current function if possible.");
        inst_lib.AddInst("Nop", Inst_Nop, 0, "No operation.");
      }

      return &inst_lib;
    }

  };
}

#endif
