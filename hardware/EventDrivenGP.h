#ifndef EMP_EVENT_DRIVEN_GP_H
#define EMP_EVENT_DRIVEN_GP_H

#include <functional>
#include <unordered_map>
#include <deque>
#include <utility>
#include "InstLib.h"
#include "EventLib.h"
#include "../tools/BitSet.h"
#include "../tools/BitVector.h"
#include "../tools/map_utils.h"
#include "../tools/string_utils.h"
#include "../tools/Random.h"
#include "../base/vector.h"
#include "../base/Ptr.h"
#include "../base/array.h"
#include "../control/SignalControl.h"
#include "../control/Signal.h"

// Notes:
//  * Important concept: Main state (bottom-most call state on core 0's call stack).
//    * The first function will be main (unless the fp on the initially created state is otherwise manipulated).
//    * Main state behaves differently than any other state.

// @amlalejini - TODO:
//  [ ] Write some halfway decent documentation. --> Use doxygen notation. Every instance variable, every function.
//  [ ] Write up a nice description.

namespace emp {
  template<size_t AFFINITY_WIDTH>
  class EventDrivenGP_AW {
  public:
    /// Maximum number of instruction arguments. Currently hardcoded. At some point, will make flexible.
    static constexpr size_t MAX_INST_ARGS = 3;

    using EventDrivenGP_t = EventDrivenGP_AW<AFFINITY_WIDTH>;  //< Resolved type for this templated class.
    using mem_key_t = int;                                     //< Hardware memory map key type.
    using mem_val_t = double;                                  //< Hardware memory map value type.
    using memory_t = std::unordered_map<mem_key_t, mem_val_t>; //< Hardware memory map type.
    using arg_t = int;                                         //< Instruction argument type.
    using arg_set_t = emp::array<arg_t, MAX_INST_ARGS>;        //< Instruction argument set type.
    using affinity_t = BitSet<AFFINITY_WIDTH>;                 //< Affinity type alias.
    using properties_t = std::unordered_set<std::string>;      //< Event/Instruction properties type.

    // A few default values. WARNING: I have no actual reason to believe these are the best defaults.
    static constexpr size_t DEFAULT_MAX_CORES = 8;
    static constexpr size_t DEFAULT_MAX_CALL_DEPTH = 128;
    static constexpr mem_val_t DEFAULT_MEM_VALUE = 0.0;
    static constexpr double DEFAULT_MIN_BIND_THRESH = 0.5;

    /// Struct to represent an instance of an Event.
    /// Events have an associated ID, affinity, message, and set of properties.
    struct Event {
      size_t id;               //< Event ID. Used to lookup event type in event library.
      affinity_t affinity;     //< Event affinity. Used to match what function this event should bind to.
      memory_t msg;            //< Event message. Packet of information associated with event.
      properties_t properties; //< Event properties. Properties of this instance of an event.

      Event(size_t _id=0, const affinity_t & aff=affinity_t(), const memory_t & _msg=memory_t(),
            const properties_t & _properties=properties_t())
      : id(_id), affinity(aff), msg(_msg), properties(_properties) { ; }
      Event(const Event &) = default;
      Event(Event &&) = default;

      Event & operator=(const Event &) = default;
      Event & operator=(Event &&) = default;

      bool HasProperty(std::string property) const { return properties.count(property); }
    };

    /// Currently only 3 Block types:
    ///   * NONE:  Not a block.
    ///   * BASIC: Anything that's not a loop. Once closed/@end of block, execution can just continue.
    ///   * LOOP:  Once closed/@end of block, execution needs to jump back to beginning of block.
    enum class BlockType { NONE=0, BASIC, LOOP };
    /// Struct to store information relevant to a 'code block' (e.g. if statements, while loops, etc).
    /// Maintains the beginning, end, and type of block.
    struct Block {
      size_t begin;    //< Instruction position where block begins.
      size_t end;      //< Instruction position where block ends.
      BlockType type;  //< Block type.

      Block(size_t _begin=0, size_t _end=0, BlockType _type=BlockType::BASIC)
        : begin(_begin), end(_end), type(_type) { ; }
    };

    /// Struct to maintain local program state for a given function call.
    /// A local program state has an associated: local memory map, input memory map, output memory map,
    /// function pointer, instruction pointer, and block stack.
    struct State {
      memory_t local_mem;     //< Local memory map. By default, most instructions operate on local memory.
      memory_t input_mem;     //< Input memory map.
      memory_t output_mem;    //< Output memory map.
      double default_mem_val; //< Default memory value. If memory map is accessed using key that doesn't exist in the map, the default memory value is returned.

      size_t func_ptr;                  //< Function pointer.
      size_t inst_ptr;                  //< Instruction pointer.
      emp::vector<Block> block_stack;   //< Stack of blocks (top is current block status).
      bool is_main;                     //< Indicates if this state is main or not.

      State(mem_val_t _default_mem_val = 0.0, bool _is_main=false)
        : local_mem(), input_mem(), output_mem(), default_mem_val(_default_mem_val),
          func_ptr(0), inst_ptr(0), block_stack(), is_main(_is_main) { ; }
      State(const State &) = default;
      State(State &&) = default;

      void Reset() {
        local_mem.clear();
        input_mem.clear();
        output_mem.clear();
        func_ptr = 0; inst_ptr = 0;
        block_stack.clear();
      }

      size_t GetFP() const { return func_ptr; }
      size_t GetIP() const { return inst_ptr; }
      mem_val_t GetDefaultMemValue() const { return default_mem_val; }
      void SetIP(size_t ip) { inst_ptr = ip; }
      void SetFP(size_t fp) { func_ptr = fp; }
      void SetDefaultMemValue(mem_val_t val) { default_mem_val = val; }
      void AdvanceIP(size_t inc = 1) { inst_ptr += inc; }
      bool IsMain() const { return is_main; }

      memory_t & GetLocalMemory() { return local_mem; }
      memory_t & GetInputMemory() { return input_mem; }
      memory_t & GetOutputMemory() { return output_mem; }

      /// GetXMemory functions return value at memory location if memory location exists.
      /// Otherwise, these functions return default memory value.
      mem_val_t GetLocal(mem_key_t key) const { return Find(local_mem, key, default_mem_val); }
      mem_val_t GetInput(mem_key_t key) const { return Find(input_mem, key, default_mem_val); }
      mem_val_t GetOutput(mem_key_t key) const { return Find(output_mem, key, default_mem_val); }

      /// SetXMemory functions set memory location (specified by key) to value.
      void SetLocal(mem_key_t key, mem_val_t value) { local_mem[key] = value; }
      void SetInput(mem_key_t key, mem_val_t value) { input_mem[key] = value; }
      void SetOutput(mem_key_t key, mem_val_t value) { output_mem[key] = value; }

      /// AccessXMemory functions return reference to memory location value if that location exists.
      /// If the location does not exist, set to default memory value and return reference to memory location value.
      mem_val_t & AccessLocal(mem_key_t key) {
        if (!Has(local_mem, key)) local_mem[key] = default_mem_val;
        return local_mem[key];
      }
      mem_val_t & AccessInput(mem_key_t key) {
        if (!Has(input_mem, key)) input_mem[key] = default_mem_val;
        return input_mem[key];
      }
      mem_val_t & AccessOutput(mem_key_t key) {
        if (!Has(output_mem, key)) output_mem[key] = default_mem_val;
        return output_mem[key];
      }
    };

    /// Struct to maintain EventDrivenGP Instruction information.
    /// Each instruction has an associated:
    ///   * id: Instruction ID. Used to lookup instruction type using an instruction library.
    ///   * args: Instruction arguments. Currently hardcoded maximum of 3.
    ///   * affinity: Instruction affinity.
    struct Instruction {
      size_t id;            //< Instruction ID. Used to lookup instruction type using an instruction library.
      arg_set_t args;       //< Instruction arguments. Currently hardcoded maximum of 3.
      affinity_t affinity;  //< Instruction affinity.

      Instruction(size_t _id=0, arg_t a0=0, arg_t a1=0, arg_t a2=0, const affinity_t & _aff=affinity_t())
        : id(_id), args(), affinity(_aff) { args[0] = a0; args[1] = a1; args[2] = a2; }
      Instruction(const Instruction &) = default;
      Instruction(Instruction &&) = default;

      Instruction & operator=(const Instruction &) = default;
      Instruction & operator=(Instruction &&) = default;

      void Set(size_t _id, arg_t _a0=0, arg_t _a1=0, arg_t _a2=0, const affinity_t & _aff=affinity_t())
        { id = _id; args[0] = _a0; args[1] = _a1; args[2] = _a2; affinity = _aff; }

      void Set(const Instruction & other) {
        id = other.id;
        args[0] = other.args[0]; args[1] = other.args[1]; args[2] = other.args[2];
        affinity = other.affinity;
      }

      bool operator==(const Instruction & in) const {
        return id == in.id && args == in.args && affinity == in.affinity;
      }
      bool operator!=(const Instruction & in) const { return !(*this == in); }
    };

    using inst_t = Instruction;                    //< Convenient Instruction type alias.
    using event_t = Event;                         //< Event type alias.
    using inst_lib_t = InstLib<EventDrivenGP_t>;   //< Instruction library type alias.
    using event_lib_t = EventLib<EventDrivenGP_t>; //< Event library type alias.

    /// Function struct. Defines an EventDrivenGP function.
    /// Each function has an associated:
    ///   * affinity: Function affinity. Analogous to the function's name.
    ///   * inst_seq: Instruction sequence. Sequence of instructions that make up the function.
    struct Function {
      using inst_seq_t = emp::vector<inst_t>; //< Convenient type alias for instruction sequence.

      affinity_t affinity;          //< Function affinity. Analogous to the function's name.
      inst_seq_t inst_seq;          //< Instruction sequence. Sequence of instructions that make up the function.

      Function(const affinity_t & _aff=affinity_t(), const inst_seq_t & _seq=inst_seq_t())
        : affinity(_aff), inst_seq(_seq) { ; }

      size_t GetSize() const { return inst_seq.size(); }

      inst_t & operator[](size_t id) { return inst_seq[id]; }
      const inst_t & operator[](size_t id) const { return inst_seq[id]; }

      bool operator==(const Function & in) const {
        return inst_seq == in.inst_seq && affinity == in.affinity;
      }
      bool operator!=(const Function & in) const { return !(*this == in); }

      void PushInst(size_t id, arg_t a0, arg_t a1, arg_t a2, const affinity_t & aff) {
        inst_seq.emplace_back(id, a0, a1, a2, aff);
      }

      void PushInst(const inst_t & inst) {
        inst_seq.emplace_back(inst);
      }

      void SetInst(size_t pos, size_t id, arg_t a0, arg_t a1, arg_t a2, const affinity_t & aff) {
        inst_seq[pos].Set(id, a0, a1, a2);
      }

      void SetInst(size_t pos, const inst_t & inst) {
        inst_seq[pos].Set(inst);
      }

    };

    /// Program struct. Defines an EventDrivenGP program.
    /// A programs consists of a set of functions where each function is a named sequence of instructions.
    /// Function names are bit strings (stored as a BitSet).
    /// Programs require an associated instruction library to give meaning to the instructions that make up
    /// their functions.
    struct Program {
      using program_t = emp::vector<Function>;  //< Convenient type alias for sequence of functions.
      using inst_seq_t = typename Function::inst_seq_t;

      Ptr<const inst_lib_t> inst_lib;  //< Pointer to const instruction library associated with this program.
      program_t program;               //< Sequence of functions that make up this program.

      Program(Ptr<const inst_lib_t> _ilib, const program_t & _prgm=program_t())
      : inst_lib(_ilib), program(_prgm) { ; }
      Program(const Program &) = default;

      void Clear() { program.clear(); }

      Function & operator[](size_t id) { return program[id]; }
      const Function & operator[](size_t id) const { return program[id]; }

      bool operator==(const Program & in) const { return program == in.program; }
      bool operator!=(const Program & in) const { return !(*this == in); }

      size_t GetSize() const { return program.size(); }

      size_t GetInstCnt() const {
        size_t cnt = 0;
        for (size_t i = 0; i < GetSize(); ++i) cnt += program[i].GetSize();
        return cnt;
      }

      Ptr<const inst_lib_t> GetInstLib() const { return inst_lib; }

      bool ValidPosition(size_t fID, size_t pos) const {
        return fID < program.size() && pos < program[fID].GetSize();
      }
      bool ValidFunction(size_t fID) const { return fID < program.size(); }

      void SetProgram(const program_t & _program) { program = _program; }

      void PushFunction(const Function & _function) { program.emplace_back(_function); }
      void PushFunction(const affinity_t & _aff=affinity_t(), const inst_seq_t & _seq=inst_seq_t()) {
        program.emplace_back(_aff, _seq);
      }
      /// Push new instruction to program.
      /// If no function pointer is provided and no functions exist yet, add new function to
      /// program and push to that. If no function pointer is provided and functions exist, push to
      /// last function in program. If function pointer is provided, push to that function.
      void PushInst(size_t id, arg_t a0=0, arg_t a1=0, arg_t a2=0,
                    const affinity_t & aff=affinity_t(), int fID=-1)
      {
        size_t fp;
        if (program.empty()) { program.emplace_back(); fp = 0; }
        else if (fID < 0 || fID >= (int)program.size()) { fp = program.size() - 1; }
        else fp = (size_t)fID;
        program[fp].PushInst(id, a0, a1, a2, aff);
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
        if (program.empty()) { program.emplace_back(); fp = 0; }
        else if (fID < 0 || fID >= (int)program.size()) { fp = program.size() - 1; }
        else fp = (size_t)fID;
        program[fp].PushInst(id, a0, a1, a2, aff);
      }

      /// Push new instruction to program.
      /// If no function pointer is provided and no functions exist yet, add new function to
      /// program and push to that. If no function pointer is provided and functions exist, push to
      /// last function in program. If function pointer is provided, push to that function.
      void PushInst(const inst_t & inst, int fID=-1) {
        size_t fp;
        if (program.empty()) { program.emplace_back(); fp = 0; }
        else if (fID < 0 || fID >= (int)program.size()) { fp = program.size() - 1; }
        else fp = (size_t)fID;
        program[fp].PushInst(inst);
      }

      void SetInst(size_t fID, size_t pos, size_t id, arg_t a0=0, arg_t a1=0, arg_t a2=0,
                   const affinity_t & aff=affinity_t()) {
        emp_assert(ValidPosition(fID, pos));
        program[fID].SetInst(pos, id, a0, a1, a2, aff);
      }

      void SetInst(size_t fID, size_t pos, const inst_t & inst) {
        emp_assert(ValidPosition(fID, pos));
        program[fID].SetInst(pos, inst);
      }

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
        for (size_t fID = 0; fID < GetSize(); fID++) {
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

    };

    using program_t = Program;              //< Program type alias.
    using exec_stk_t = emp::vector<State>;  //< Execution Stack/Core type alias.
    /// Event handler function type alias.
    using fun_event_handler_t = std::function<void(EventDrivenGP_t &, const event_t &)>;

  protected:
    Ptr<const event_lib_t> event_lib;     //< Pointer to const event library associated with this hardware.
    Ptr<Random> random_ptr;               //< Pointer to random object to use.
    bool random_owner;                    //< Does this hardware own it's random object? (necessary for cleanup responsibility resolution)
    program_t program;                    //< Hardware's associated program (set of functions).
    memory_t shared_mem;                  //< Hardware's shared memory map. All cores have access to the same shared memory.
    std::deque<event_t> event_queue;      //< Hardware's event queue. Where events go to be handled (in order of reception).
    emp::vector<double> traits;           //< Generic traits vector. Whatever uses the hardware must define/keep track of what traits mean.
    size_t errors;                        //< Errors committed by hardware while executing. (e.g. divide by 0, etc.)
    size_t max_cores;                     //< Maximum number of parallel execution stacks that can be spawned. Increasing this value drastically slows things down.
    size_t max_call_depth;                //< Maximum depth of calls per execution stack.
    double default_mem_value;             //< Default value for memory access.
    double min_bind_thresh;               //< Minimum bit string match threshold for function calls/event binding, etc.
    bool stochastic_fun_call;             //< Are candidate function calls with == binding strength chosen stochastically?
    emp::vector<exec_stk_t> cores;        //< Vector of cores. Not all will be active at all given points in time.
    emp::vector<size_t> active_cores;     //< Vector of active core IDs. Maintains relative ordering or active cores.
    emp::vector<size_t> inactive_cores;   //< Vector of inactive core IDs.
    std::deque<size_t> pending_cores;     //< Queue of core IDs pending activation.
    size_t exec_core_id;                  //< core ID of the currently executing core.
    bool is_executing;                    //< True when mid-execution of all cores. (On every CPU cycle: execute all cores).

    // TODO: disallow configuration of hardware while executing. (and any other functions that could sent things into a bad state)
    /// Garbage function for debugging.
    void PrintCoreStates() {
      std::cout << "Active core ids:" << std::endl;
      for (size_t i = 0; i < active_cores.size(); ++i) std::cout << " " << active_cores[i];
      std::cout << "Inactive core ids: " << std::endl;
      for (size_t i = 0; i < inactive_cores.size(); ++i) std::cout << " " << inactive_cores[i];
      std::cout << "Pending core ids: " << std::endl;
      for (size_t i = 0; i < pending_cores.size(); ++i) std::cout << " " << pending_cores[i];
    }

  public:
    /// EventDrivenGP constructor. Give instance variables reasonable defaults. Allow for configuration
    /// post-construction.
    EventDrivenGP_AW(Ptr<const inst_lib_t> _ilib, Ptr<const event_lib_t> _elib, Ptr<Random> rnd=nullptr)
      : event_lib(_elib),
        random_ptr(rnd), random_owner(false),
        program(_ilib),
        shared_mem(),
        event_queue(),
        traits(), errors(0),
        max_cores(DEFAULT_MAX_CORES), max_call_depth(DEFAULT_MAX_CALL_DEPTH),
        default_mem_value(DEFAULT_MEM_VALUE), min_bind_thresh(DEFAULT_MIN_BIND_THRESH),
        stochastic_fun_call(true),
        cores(max_cores), active_cores(), inactive_cores(max_cores), pending_cores(),
        exec_core_id(0), is_executing(false)
    {
      // If no random provided, create one.
      if (!rnd) NewRandom();
      // Add all available cores to inactive.
      for (size_t i = 0; i < inactive_cores.size(); ++i)
        inactive_cores[i] = (inactive_cores.size() - 1) - i;
      // Spin up main core (will spin up on function ID = 0).
      SpawnCore(0, memory_t(), true);
    }

    EventDrivenGP_AW(inst_lib_t & _ilib, event_lib_t & _elib, Ptr<Random> rnd=nullptr)
      : EventDrivenGP_AW(&_ilib, &_elib, rnd) { ; }

    EventDrivenGP_AW(Ptr<const event_lib_t> _elib, Ptr<Random> rnd=nullptr)
      : EventDrivenGP_AW(DefaultInstLib(), _elib, rnd) { ; }

    EventDrivenGP_AW(Ptr<Random> rnd=nullptr)
      : EventDrivenGP_AW(DefaultInstLib(), DefaultEventLib(), rnd) { ; }

    EventDrivenGP_AW(EventDrivenGP_t && in)
      : event_lib(in.event_lib),
        random_ptr(in.random_ptr), random_owner(in.random_owner),
        program(in.program),
        shared_mem(in.shared_mem),
        event_queue(in.event_queue),
        traits(in.traits), errors(in.errors),
        max_cores(in.max_cores), max_call_depth(in.max_call_depth),
        default_mem_value(in.default_mem_value), min_bind_thresh(in.min_bind_thresh),
        stochastic_fun_call(in.stochastic_fun_call),
        cores(in.cores),
        active_cores(in.active_cores), inactive_cores(in.inactive_cores),
        pending_cores(in.pending_cores),
        exec_core_id(in.exec_core_id), is_executing(in.is_executing)
    {
      in.random_ptr = nullptr;
      in.event_lib = nullptr;
      in.program.inst_lib = nullptr;
    }

    EventDrivenGP_AW(const EventDrivenGP_t & in)
      : event_lib(in.event_lib),
        random_ptr(nullptr), random_owner(false),
        program(in.program),
        shared_mem(in.shared_mem),
        event_queue(in.event_queue),
        traits(in.traits), errors(in.errors),
        max_cores(in.max_cores), max_call_depth(in.max_call_depth),
        default_mem_value(in.default_mem_value), min_bind_thresh(in.min_bind_thresh),
        stochastic_fun_call(in.stochastic_fun_call),
        cores(in.cores),
        active_cores(in.active_cores), inactive_cores(in.inactive_cores),
        pending_cores(in.pending_cores),
        exec_core_id(in.exec_core_id), is_executing(in.is_executing)
    {
      if (in.random_owner) NewRandom();
      else random_ptr = in.random_ptr;
    }

    // @amlalejini - TODO: define operator= (move version and copy version)

    /// Destructor - clean up: execution stacks, shared memory.
    ~EventDrivenGP_AW() {
      if (random_owner) random_ptr.Delete();
    }

    // -- Control --
    /// Reset everything, including program.
    void Reset() {
      ResetHardware();
      traits.clear();
      program.Clear();
    }

    /// Reset only CPU hardware stuff, not program.
    void ResetHardware() {
      shared_mem.clear();
      event_queue.clear();
      for (size_t i = 0; i < cores.size(); ++i) cores[i].clear();
      active_cores.clear();
      pending_cores.clear();
      inactive_cores.resize(max_cores);
      // Add all available cores to inactive.
      for (size_t i = 0; i < inactive_cores.size(); ++i)
        inactive_cores[i] = (inactive_cores.size() - 1) - i;
      exec_core_id = -1;
      errors = 0;
      is_executing = false;
    }

    // -- Accessors --
    Ptr<const inst_lib_t> GetInstLib() const { return program.GetInstLib(); }
    Ptr<const event_lib_t> GetEventLib() const { return event_lib; }

    Random & GetRandom() { return *random_ptr; }
    Ptr<Random> GetRandomPtr() { return random_ptr; }

    const program_t & GetProgram() const { return program; }
    const Function & GetFunction(size_t fID) const {
      emp_assert(ValidFunction(fID));
      return program[fID];
    }
    const inst_t & GetInst(size_t fID, size_t pos) const {
      emp_assert(ValidPosition(fID, pos));
      return program[fID].inst_seq[pos];
    }

    double GetTrait(size_t id) const { return traits[id]; }

    size_t GetNumErrors() const { return errors; }

    double GetMinBindThresh() const { return min_bind_thresh; }
    size_t GetMaxCores() const { return max_cores; }
    size_t GetMaxCallDepth() const { return max_call_depth; }
    mem_val_t GetDefaultMemValue() const { return default_mem_value; }
    bool IsStochasticFunCall() const { return stochastic_fun_call; }

    size_t GetCurCoreID() { return exec_core_id; }
    exec_stk_t & GetCurCore() { return cores[exec_core_id]; }
    State & GetCurState() { emp_assert(!cores[exec_core_id].size()); return cores[exec_core_id].back(); }

    bool ValidPosition(size_t fID, size_t pos) const { return program.ValidPosition(fID, pos); }
    bool ValidFunction(size_t fID) const { return program.ValidFunction(fID); }

    memory_t & GetSharedMem() { return shared_mem; }
    mem_val_t GetShared(mem_key_t key) const { return Find(shared_mem, key, default_mem_value); }

    void SetShared(mem_key_t key, mem_val_t value) { shared_mem[key] = value; }
    mem_val_t & AccessShared(mem_key_t key) {
      if (!Has(shared_mem, key)) shared_mem[key] = default_mem_value;
      return (shared_mem)[key];
    }

    // ------- Configuration -------
    // TODO: disallow while executing.
    /// Set minimum binding threshold.
    /// REQ: val >= 0
    void SetMinBindThresh(double val) { emp_assert(val >= 0.0); min_bind_thresh = val; }
    /// WARNING: If you decrease max cores, you may kill actively running cores.
    /// We make no guarantees about which particular cores will not be killed.
    /// Exception: ...
    void SetMaxCores(size_t val) { max_cores = val; } // TODO: think about reprocussions of changing mid-execution.
    void SetMaxCallDepth(size_t val) { max_call_depth = val; }
    void SetDefaultMemValue(mem_val_t val) {
      default_mem_value = val;
      // Propagate default mem value through execution stacks.
      for (size_t i = 0; i < cores.size(); ++i)
        for (size_t k = 0; k < cores[i].size(); ++k)
          cores[i][k].SetDefaultMemValue(val);
    }
    void SetStochasticFunCall(bool val) { stochastic_fun_call = val; }

    void SetTrait(size_t id, double val) {
      if (id >= traits.size()) traits.resize(id+1, 0.0);
      traits[id] = val;
    }

    void PushTrait(double val) { traits.push_back(val); }

    void SetInst(size_t fID, size_t pos, const inst_t & inst) {
      emp_assert(ValidPosition(fID, pos));
      program[fID].inst_seq[pos] = inst;
    }

    void SetInst(size_t fID, size_t pos, size_t id, arg_t a0=0, arg_t a1=0, arg_t a2=0,
                 const affinity_t & aff=affinity_t()) {
      emp_assert(ValidPosition(fID, pos));
      program[fID].inst_seq[pos].Set(id, a0, a1, a2, aff);
    }

    void SetProgram(const program_t & _program) { program = _program; }

    void PushFunction(const Function & _function) { program.PushFunction(_function); }

    /// Push new instruction to program.
    /// If no function pointer is provided and no functions exist yet, add new function to
    /// program and push to that. If no function pointer is provided and functions exist, push to
    /// last function in program. If function pointer is provided, push to that function.
    void PushInst(size_t id, arg_t a0=0, arg_t a1=0, arg_t a2=0,
                  const affinity_t & aff=affinity_t(), int fID=-1)
    { program.PushInst(id, a0, a1, a2, aff, fID); }

    /// Push new instruction to program.
    /// If no function pointer is provided and no functions exist yet, add new function to
    /// program and push to that. If no function pointer is provided and functions exist, push to
    /// last function in program. If function pointer is provided, push to that function.
    void PushInst(const std::string & name, arg_t a0=0, arg_t a1=0, arg_t a2=0,
                  const affinity_t & aff=affinity_t(), int fID=-1)
    { program.PushInst(name, a0, a1, a2, aff, fID); }

    /// Push new instruction to program.
    /// If no function pointer is provided and no functions exist yet, add new function to
    /// program and push to that. If no function pointer is provided and functions exist, push to
    /// last function in program. If function pointer is provided, push to that function.
    void PushInst(const inst_t & inst, int fID=-1) { program.PushInst(inst, fID); }

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
      Ptr<const inst_lib_t> inst_lib = program.inst_lib;
      int depth_counter = 1;
      while (true) {
        if (!ValidPosition(fp, ip)) break;
        const inst_t & inst = GetInst(fp, ip);
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
      State & state = GetCurState();
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
      State & state = GetCurState();
      state.block_stack.emplace_back(begin, end, type);
    }

    /// If there's a block to break out of, break out (to eob).
    /// Otherwise, do nothing.
    void BreakBlock() {
      State & state = GetCurState();
      if (!state.block_stack.empty()) {
        state.SetIP(state.block_stack.back().end);
        state.block_stack.pop_back();
        if (ValidPosition(state.GetFP(), state.GetIP())) state.AdvanceIP();
      }
    }

    /// Find best matching functions (by ID) given affinity.
    emp::vector<size_t> FindBestFuncMatch(const affinity_t & affinity, double threshold) {
      emp::vector<size_t> best_matches;
      for (size_t i=0; i < program.GetSize(); ++i) {
        double bind = SimpleMatchCoeff(program[i].affinity, affinity);
        if (bind == threshold) best_matches.push_back(i);
        else if (bind > threshold) {
          best_matches.resize(1);
          best_matches[0] = i;
          threshold = bind;
        }
      }
      return best_matches;
    }

    /// Spawn core with function that has best match to provided affinity. Do nothing if no
    /// functions match above the provided threshold.
    /// Initialize function state with provided input memory.
    /// Will fail if no inactive cores to claim.
    void SpawnCore(const affinity_t & affinity, double threshold, const memory_t & input_mem=memory_t(), bool is_main=false) {
      if (!inactive_cores.size()) return; // If there are no unclaimed cores, just return.
      size_t fID;
      emp::vector<size_t> best_matches(FindBestFuncMatch(affinity, threshold));
      if (best_matches.empty()) return;
      if (best_matches.size() == 1.0) fID = best_matches[0];
      else if (stochastic_fun_call) fID = best_matches[(size_t)random_ptr->GetUInt(0, best_matches.size())];
      else fID = best_matches[0];
      SpawnCore(fID, input_mem, is_main);
    }

    /// Spawn core with function specified by fID.
    /// Initialize function state with provided input memory.
    /// Will fail if no inactive cores to claim.
    void SpawnCore(size_t fID, const memory_t & input_mem=memory_t(), bool is_main=false) {
      if (!inactive_cores.size()) return; // If there are no unclaimed cores, just return.
      // Which core should we spin up?
      size_t core_id = inactive_cores.back();
      inactive_cores.pop_back(); // Claim that core!
      exec_stk_t & exec_stk = cores[core_id];
      exec_stk.clear(); // Make sure we clear out the core (in case anyone left behind their dirty laundry).
      exec_stk.emplace_back(default_mem_value, is_main);
      State & state = exec_stk.back();
      state.input_mem = input_mem;
      state.SetIP(0);
      state.SetFP(fID);
      // Spin up new core.
      // Mark core as pending if currently executing; otherwise, mark it as active.
      if (is_executing) pending_cores.push_back(core_id);
      else active_cores.push_back(core_id);
    }

    /// Call function with best affinity match above threshold.
    /// If not candidate functions found, do nothing.
    void CallFunction(const affinity_t & affinity, double threshold) {
      // Are we at max call depth? -- If so, call fails.
      if (GetCurCore().size() >= max_call_depth) return;
      size_t fID;
      emp::vector<size_t> best_matches(FindBestFuncMatch(affinity, threshold));
      if (best_matches.empty()) return;
      if (best_matches.size() == 1.0) fID = best_matches[0];
      else if (stochastic_fun_call) fID = best_matches[(size_t)random_ptr->GetUInt(0, best_matches.size())];
      else fID = best_matches[0];
      CallFunction(fID);
    }

    /// Call function specified by fID.
    /// REQ: core must be active (must have a local state on execution stack).
    void CallFunction(size_t fID) {
      emp_assert(ValidPosition(fID, 0));
      exec_stk_t & core = GetCurCore();
      // Are we at max call depth? -- If so, call fails.
      if (core.size() >= max_call_depth) return;
      // Push new state onto stack.
      core.emplace_back();
      State & new_state = core.back();
      State & caller_state = core[core.size() - 2];
      // Configure new state.
      new_state.SetFP(fID);
      new_state.SetIP(0);
      for (auto mem : caller_state.local_mem) {
        new_state.SetInput(mem.first, mem.second);
      }
    }

    /// Return from current function call (cur_state) in current core (cur_core).
    /// Upon returning, put values in output memory of returning state into local memory of caller state.
    void ReturnFunction() {
      // Grab the returning state and then pop it off the call stack.
      State & returning_state = GetCurState();
      // No returning from main.
      if (returning_state.IsMain()) return;
      // Is there anything to return to?
      exec_stk_t & core = GetCurCore();
      if (core.size() > 1) {
        // If so, copy returning state's output memory into caller state's local memory.
        State & caller_state = core[core.size() - 2];
        for (auto mem : returning_state.output_mem) caller_state.SetLocal(mem.first, mem.second);
      }
      // Pop returned state.
      core.pop_back();
    }

    // -- Execution --
    /// Process a single instruction, provided by the caller.
    void ProcessInst(const inst_t & inst) { program.inst_lib->ProcessInst(*this, inst); }
    /// Handle an event (on this hardware).
    void HandleEvent(const event_t & event) { event_lib->HandleEvent(*this, event); }
    /// Trigger an event (from this hardware).
    void TriggerEvent(const event_t & event) { event_lib->TriggerEvent(*this, event); }
    /// Trigger an event (from this hardware).
    void TriggerEvent(const std::string & name, const affinity_t & affinity=affinity_t(),
                      const memory_t & msg=memory_t(), const properties_t & properties=properties_t()) {
      const size_t id = event_lib->GetID(name);
      event_t event(id, affinity, msg, properties);
      event_lib->TriggerEvent(*this, event);
    }
    /// Trigger an event (from this hardware).
    void TriggerEvent(size_t id, const affinity_t & affinity=affinity_t(),
                      const memory_t & msg=memory_t(), const properties_t & properties=properties_t()) {
      event_t event(id, affinity, msg, properties);
      event_lib->TriggerEvent(*this, event);
    }
    /// Queue an event (to be handled by this hardware).
    void QueueEvent(const event_t & event) { event_queue.emplace_back(event); }
    /// Queue event by name.
    void QueueEvent(const std::string & name, const affinity_t & affinity=affinity_t(),
                    const memory_t & msg=memory_t(), const properties_t & properties=properties_t()) {
      const size_t id = event_lib->GetID(name);
      event_queue.emplace_back(id, affinity, msg, properties);
    }
    /// Queue event by id.
    void QueueEvent(size_t id, const affinity_t & affinity=affinity_t(),
                    const memory_t & msg=memory_t(), const properties_t & properties=properties_t()) {
      event_queue.emplace_back(id, affinity, msg, properties);
    }

    /// Advance hardware by single instruction.
    void SingleProcess() {
      emp_assert(program.GetSize()); // Must have a program before this is allowed.
      // Handle events.
      while (!event_queue.empty()) {
        HandleEvent(event_queue.front());
        event_queue.pop_front();
      }
      // Distribute 1 unit of computational time to each core.
      size_t active_core_idx = 0;
      // size_t core_cnt = execution_stacks.size();
      size_t core_cnt = active_cores.size();
      size_t adjust = 0;
      is_executing = true;
      while (active_core_idx < core_cnt) {
        // Set the current core to core at core_idx.
        exec_core_id = active_cores[active_core_idx]; // Here's the core we're about to execute.
        // Do we need to move current core over in the execution core vector to make it contiguous?
        if (adjust) {
          active_cores[active_core_idx] = -1;
          active_cores[active_core_idx - adjust] = exec_core_id;
        }
        // Execute the core.
        //  * What function/instruction am I on?
        State & cur_state = GetCurState();
        const size_t ip = cur_state.inst_ptr;
        const size_t fp = cur_state.func_ptr;
        // fp needs to be valid here (and always, really). Shame shame if it's not.
        emp_assert(ValidFunction(fp));
        // If instruction pointer hanging off end of function sequence:
        if (ip >= program[fp].GetSize()) {
          if (!cur_state.block_stack.empty()) {
            //    - If there's a block to close, close it.
            CloseBlock();
          } else if (cur_state.is_main && GetCurCore().size() == 1) {
            //    - If this is the main function, and we're at the bottom of the call stack, wrap.
            cur_state.inst_ptr = 0;
          } else {
            //    - Otherwise, return from function call.
            ReturnFunction(); // NOTE: This might invalidate our cur_state reference.
          }
        } else { // If instruction pointer is valid:
          // First, advance the instruction pointer by 1. This may invalidate the IP, but that's okay.
          cur_state.inst_ptr += 1;
          // Run instruction @ fp, ip.
          program.inst_lib->ProcessInst(*this, program[fp].inst_seq[ip]);
        }
        // After processing, is the core still active?
        if (GetCurCore().empty()) {
          // Free core. Mark as inactive.
          active_cores[active_core_idx - adjust] = -1;
          inactive_cores.emplace_back(exec_core_id);
          adjust += 1;
        }
        ++active_core_idx;
      }
      is_executing = false;
      // Update execution stack size to be accurate.
      active_cores.resize(core_cnt - adjust);
      // Set cur core to be first execution stack (which should always be main).
      if (active_cores.size()) exec_core_id = active_cores[0];
      // Spawn any cores that happened during execution.
      while (pending_cores.size()) {
        active_cores.emplace_back(pending_cores.front());
        pending_cores.pop_front();
      }
    }

    /// Advance hardware by some number instructions.
    void Process(size_t num_inst) {
      for (size_t i = 0; i < num_inst; i++) SingleProcess();
    }

    // -- Printing --
    void PrintEvent(const event_t & event, std::ostream & os=std::cout) {
      os << "[" << event_lib->GetName(event.id) << ","; event.affinity.Print(os); os << ",(";
      for (const auto & mem : event.msg) std::cout << "{" << mem.first << ":" << mem.second << "}";
      os << "),(Properties:";
      for (const auto & property : event.properties) std::cout << " " << property;
      os << ")]";
    }

    /// Print out a single instruction with its arguments.
    void PrintInst(const inst_t & inst, std::ostream & os=std::cout) {
      program.PrintInst(inst, os);
    }

    /// Print out hardware traits.
    void PrintTraits(std::ostream & os=std::cout) {
      if (traits.size() == 0) { os << "[]"; return; }
      os << "[";
      for (size_t i = 0; i < traits.size() - 1; ++i) {
        os << traits[i] << ", ";
      } os << traits[traits.size() - 1] << "]";
    }

    /// Print out entire program.
    void PrintProgram(std::ostream & os=std::cout) {
      program.PrintProgram(os);
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
      for (auto mem : shared_mem) os << '{' << mem.first << ':' << mem.second << '}'; os << '\n';
      os << "Traits: "; PrintTraits(os); os << "\n";
      os << "Errors: " << errors << "\n";
      // Print events.
      os << "Event queue: ";
      for (auto event : event_queue) { PrintEvent(event, os); os << " "; }
      os << "\n";
      // Print each active core.
      for (size_t i = 0; i < active_cores.size(); ++i) {
        size_t core_id = active_cores[i];
        const exec_stk_t & core = cores[core_id];
        os << "Core " << i << "(CID=" << core_id << "):\n" << "  Call stack (" << core.size() << "):\n    --TOP--\n";
        for (size_t k = core.size() - 1; k < core.size(); --k) {
          emp_assert(core.size() != (size_t)-1);
          // IP, FP, local mem, input mem, output mem
          const State & state = core[k];
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
    static void Inst_Inc(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      ++state.AccessLocal(inst.args[0]);
    }

    static void Inst_Dec(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      --state.AccessLocal(inst.args[0]);
    }

    static void Inst_Not(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      state.SetLocal(inst.args[0], state.GetLocal(inst.args[0]) == 0.0);
    }

    static void Inst_Add(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      state.SetLocal(inst.args[2], state.AccessLocal(inst.args[0]) + state.AccessLocal(inst.args[1]));
    }

    static void Inst_Sub(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      state.SetLocal(inst.args[2], state.AccessLocal(inst.args[0]) - state.AccessLocal(inst.args[1]));
    }

    static void Inst_Mult(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      state.SetLocal(inst.args[2], state.AccessLocal(inst.args[0]) * state.AccessLocal(inst.args[1]));
    }

    static void Inst_Div(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      const double denom = state.AccessLocal(inst.args[1]);
      if (denom == 0.0) ++hw.errors;
      else state.SetLocal(inst.args[2], state.AccessLocal(inst.args[0]) / denom);
    }

    static void Inst_Mod(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      const int base = (int)state.AccessLocal(inst.args[1]);
      if (base == 0) ++hw.errors;
      else state.SetLocal(inst.args[2], (int)state.AccessLocal(inst.args[0]) % base);
    }

    static void Inst_TestEqu(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      state.SetLocal(inst.args[2], state.AccessLocal(inst.args[0]) == state.AccessLocal(inst.args[1]));
    }

    static void Inst_TestNEqu(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      state.SetLocal(inst.args[2], state.AccessLocal(inst.args[0]) != state.AccessLocal(inst.args[1]));
    }

    static void Inst_TestLess(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      state.SetLocal(inst.args[2], state.AccessLocal(inst.args[0]) < state.AccessLocal(inst.args[1]));
    }

    static void Inst_If(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
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

    static void Inst_While(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
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

    static void Inst_Countdown(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
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

    static void Inst_Break(EventDrivenGP_t & hw, const inst_t & inst) {
      hw.BreakBlock();
    }

    static void Inst_Close(EventDrivenGP_t & hw, const inst_t & inst) {
      hw.CloseBlock();
    }

    static void Inst_Call(EventDrivenGP_t & hw, const inst_t & inst) {
      hw.CallFunction(inst.affinity, hw.GetMinBindThresh());
    }

    static void Inst_Return(EventDrivenGP_t & hw, const inst_t & inst) {
      hw.ReturnFunction();
    }

    static void Inst_SetMem(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      state.SetLocal(inst.args[0], (double)inst.args[1]);
    }

    static void Inst_CopyMem(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      state.SetLocal(inst.args[1], state.AccessLocal(inst.args[0]));
    }

    static void Inst_SwapMem(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      double val0 = state.AccessLocal(inst.args[0]);
      state.SetLocal(inst.args[0], state.GetLocal(inst.args[1]));
      state.SetLocal(inst.args[1], val0);
    }

    static void Inst_Input(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      state.SetLocal(inst.args[1], state.AccessInput(inst.args[0]));
    }

    static void Inst_Output(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      state.SetOutput(inst.args[1], state.AccessLocal(inst.args[0]));
    }

    static void Inst_Commit(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      hw.SetShared(inst.args[1], state.AccessLocal(inst.args[0]));
    }

    static void Inst_Pull(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      state.SetLocal(inst.args[1], hw.AccessShared(inst.args[0]));
    }

    static void Inst_Nop(EventDrivenGP_t & hw, const inst_t & inst) { ; }

    static void Inst_BroadcastMsg(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      hw.TriggerEvent("Message", inst.affinity, state.output_mem, {"broadcast"});
    }

    static void Inst_SendMsg(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      hw.TriggerEvent("Message", inst.affinity, state.output_mem, {"send"});
    }

    static Ptr<const InstLib<EventDrivenGP_t>> DefaultInstLib() {
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
        inst_lib.AddInst("SetMem", Inst_SetMem, 2, "Local memory: Arg1 = numerical value of Arg2");
        inst_lib.AddInst("CopyMem", Inst_CopyMem, 2, "Local memory: Arg1 = Arg2");
        inst_lib.AddInst("SwapMem", Inst_SwapMem, 2, "Local memory: Swap values of Arg1 and Arg2.");
        inst_lib.AddInst("Input", Inst_Input, 2, "Input memory Arg1 => Local memory Arg2.");
        inst_lib.AddInst("Output", Inst_Output, 2, "Local memory Arg1 => Output memory Arg2.");
        inst_lib.AddInst("Commit", Inst_Commit, 2, "Local memory Arg1 => Shared memory Arg2.");
        inst_lib.AddInst("Pull", Inst_Pull, 2, "Shared memory Arg1 => Shared memory Arg2.");
        inst_lib.AddInst("BroadcastMsg", Inst_BroadcastMsg, 0, "Broadcast output memory as message event.", ScopeType::BASIC, 0, {"affinity"});
        inst_lib.AddInst("SendMsg", Inst_SendMsg, 0, "Send output memory as message event.", ScopeType::BASIC, 0, {"affinity"});
        inst_lib.AddInst("Nop", Inst_Nop, 0, "No operation.");
      }
      return &inst_lib;
    }

    // Default event handlers.
    static void HandleEvent_Message(EventDrivenGP_t & hw, const event_t & event) {
      // Spawn new core.
      hw.SpawnCore(event.affinity, hw.GetMinBindThresh(), event.msg);
    }

    /// Define default events. NOTE: default events have no registered dispatch functions.
    static Ptr<const EventLib<EventDrivenGP_t>> DefaultEventLib() {
      static event_lib_t event_lib;
      if (event_lib.GetSize() == 0) {
        event_lib.AddEvent("Message", HandleEvent_Message, "Event for exchanging messages (agent-agent, world-agent, etc.)");
      }
      return &event_lib;
    }
  };

  using EventDrivenGP = EventDrivenGP_AW<8>;
}

#endif
