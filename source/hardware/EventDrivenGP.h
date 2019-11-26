#ifndef EMP_EVENT_DRIVEN_GP_H
#define EMP_EVENT_DRIVEN_GP_H

#include <functional>
#include <tuple>
#include <unordered_map>
#include <deque>
#include <utility>
#include <algorithm>
#include <ratio>
#include "InstLib.h"
#include "EventLib.h"
#include "../tools/BitSet.h"
#include "../tools/BitVector.h"
#include "../tools/map_utils.h"
#include "../tools/string_utils.h"
#include "../tools/Random.h"
#include "../tools/MatchBin.h"
#include "../base/vector.h"
#include "../base/Ptr.h"
#include "../base/array.h"
#include "../control/SignalControl.h"
#include "../control/Signal.h"

// Developer Notes:
//  * Program struct's PrintProgram prints program in a maximally readable format. However, this format
//    is not currently acceptable for input into EventDrivenGP's Load function.
//    * Will remedy this by adding another PrintProgram function to print in a format acceptable by EventDrivenGP's
//      Load function. This PrintProgram will be less readable, but will more fully describe the program (it won't hide anything).
//  * @amlalejini - TODO:
//   [ ] operator= overrides.
//   [ ] Add in warnings about 'no actively running core' if active_cores.size() == 0 (can happen post-ResetHardware())

namespace emp {

  /**
   *  @brief A linear GP (inspired by AvidaGP) virtual hardware CPU that supports an event-driven programming paradigm.
   *  @note The terminology used throughout this class is out of date. EventDrivenGP will eventually change to 'SignalGP',
    *       and our terminology will be updated throughout.
   *  @details
   *  The EventDrivenGP virtual hardware runs programs where each program is a set of named functions.
   *  Function names are mutable bit strings, or affinities, and each function consists of a sequence
   *  of instructions. Functions are called by name, and can be called from within the hardware (via Call
   *  instructions) or from outside the hardware (via Events).
   *
   *  The EventDrivenGP virtual hardware CPU is capable of multi-core/parallel processing. This hardware
   *  maintains a (bounded) set of cores. Each core maintains its own program call stack that stores
   *  information about the active functions on that core. Cores execute in simulated parallel. For
   *  every single CPU cycle (see SingleProcess function) given to the virtual hardware, each core is
   *  given the opportunity to advance by a single cycle.
   *
   *  When functions are called from within the hardware (via Call instructions), a new call state is
   *  pushed onto the program call stack on the core from which the function was called. No new core
   *  is spawned (unless using a non-standard Call instruction). When functions are called from outside
   *  the hardware (via Events), if there is an available inactive core, an inactive core will be made
   *  active with the called function.
   *
   *  Affinity matching/binding is used to determine which functions should be called by events or by
   *  Call instructions. The hardware looks at the event/instruction's associated affinity and finds
   *  the best match (using a simple matching coefficient) among function affinities in the hardware's
   *  program. The best match must be better than a given minimum threshold (min_bind_thresh) for
   *  that function to be called. If there are not functions that meet the minimum threshold, no function
   *  is called. If two or more functions are tied for best match and are above the minimum threshold,
   *  one is selected randomly to be called; otherwise, if there is no tie, the best matching function
   *  is called.
   *
   *  Terminology:
   *    * Programs
   *      * The EventDrivenGP virtual hardware runs programs of type EventDrivenGP::Program. These programs
   *        are a set of named functions.
   *    * Functions
   *      * Functions in EventDrivenGP programs are named and consist of a sequence of instructions.
   *        Function names, or affinities, are bit strings whose length are defined by AFFINITY_WIDTH.
   *    * Instructions
   *      * EventDrivenGP hardware instructions are managed by an instruction library (InstLib.h).
   *      * Instructions have IDs, affinities, arguments, and properties.
   *    * Affinity
   *      * An affinity is a sequence of bits whose length is determined by AFFINITY_WIDTH (by default, 8).
   *    * Core
   *      * A core is a single program call stack where the call stack stores information about the
   *        active functions running on the core.
   *    * Call State
   *      * Local state information relative to a particular function call. Each call state consists
   *        of: Local memory, Input memory, Output memory, a Code Block Stack, a function pointer,
   *        and an instruction pointer.
   *    * Events
   *      * Events are defined and managed through the an event library (EventLib.h). Events are flexible
   *        packages of information that can be used to interact with EventDrivenGP hardware. Events
   *        are minimally defined by default as they are heavily dependent on the context of where/how
   *        the EventDrivenGP hardware is being used.
   *      * Events have an associated ID (managed by the event library), an affinity, a memory map,
   *        and a set of properties (string set).
   *      * When an event is triggered the set of dispatcher functions that are registered to that
   *        particular event's type (managed by the event library) are called. Events
   *        events are often triggered by instructions or by external processes. For example, the
   *        default SendMsg instruction triggers a Message type event which causes all of the Message
   *        event dispatchers registered to the hardware's event library to be called.
   *      * Each event type has a registered event handler that gets called to handle a dispatched
   *        event.
   */
  template<size_t AFFINITY_WIDTH, typename TRAIT_T=emp::vector<double>
    , typename MATCHBIN_T=emp::MatchBin<size_t, emp::HammingMetric<16>, emp::RankedSelector<std::ratio<16+8, 16>>>
    >
  class EventDrivenGP_AW {
  public:
    /// Maximum number of instruction arguments. Currently hardcoded. At some point, will make flexible.
    static constexpr size_t MAX_INST_ARGS = 3;

    static constexpr size_t affinity_width = AFFINITY_WIDTH;

    using EventDrivenGP_t = EventDrivenGP_AW<AFFINITY_WIDTH, TRAIT_T, MATCHBIN_T>;  //< Resolved type for this templated class.
    using mem_key_t = int;                                     //< Hardware memory map key type.
    using mem_val_t = double;                                  //< Hardware memory map value type.
    using memory_t = std::unordered_map<mem_key_t, mem_val_t>; //< Hardware memory map type.
    using arg_t = int;                                         //< Instruction argument type.
    using arg_set_t = emp::array<arg_t, MAX_INST_ARGS>;        //< Instruction argument set type.
    using affinity_t = BitSet<AFFINITY_WIDTH>;                 //< Affinity type alias.
    using properties_t = std::unordered_set<std::string>;      //< Event/Instruction properties type.
    using trait_t = TRAIT_T;
    using matchbin_t = MATCHBIN_T;

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

      /// Does event object have given property?
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

      /// Reset state object.
      void Reset() {
        local_mem.clear();
        input_mem.clear();
        output_mem.clear();
        func_ptr = 0; inst_ptr = 0;
        block_stack.clear();
      }

      /// Get function pointer.
      size_t GetFP() const { return func_ptr; }

      /// Get instruction pointer.
      size_t GetIP() const { return inst_ptr; }

      /// Get default memory value.
      mem_val_t GetDefaultMemValue() const { return default_mem_val; }

      /// Set instruction pointer to given value, ip.
      void SetIP(size_t ip) { inst_ptr = ip; }

      /// Set function pointer to given value, fp.
      void SetFP(size_t fp) { func_ptr = fp; }

      /// Set default memory value to given value, val.
      void SetDefaultMemValue(mem_val_t val) { default_mem_val = val; }

      /// Advance instruction pointer by amount given by inc.
      void AdvanceIP(size_t inc = 1) { inst_ptr += inc; }

      /// Is this a main state?
      bool IsMain() const { return is_main; }

      /// Get a reference to the local memory map for this state.
      memory_t & GetLocalMemory() { return local_mem; }

      /// Get a reference to the input memory map for this state.
      memory_t & GetInputMemory() { return input_mem; }

      /// Get a reference to the output memory map for this state.
      memory_t & GetOutputMemory() { return output_mem; }

      /// Get value at requested local memory location (key) if that memory location exists.
      /// Otherwise, return default memory value.
      mem_val_t GetLocal(mem_key_t key) const { return Find(local_mem, key, default_mem_val); }

      /// Get value at requested input memory location (key) if that memory location exists.
      /// Otherwise, return default memory value.
      mem_val_t GetInput(mem_key_t key) const { return Find(input_mem, key, default_mem_val); }

      /// Get value at requested output memory location (key) if that memory location exists.
      /// Otherwise, return default memory value.
      mem_val_t GetOutput(mem_key_t key) const { return Find(output_mem, key, default_mem_val); }

      /// Set local memory specified by key to value.
      void SetLocal(mem_key_t key, mem_val_t value) { local_mem[key] = value; }

      /// Set input memory specified by key to value.
      void SetInput(mem_key_t key, mem_val_t value) { input_mem[key] = value; }

      /// Set output memory specified by key to value.
      void SetOutput(mem_key_t key, mem_val_t value) { output_mem[key] = value; }

      /// Access local memory. This function returns a reference to memory location value if that location exists.
      /// If the location does not exist, set to default memory value and return reference to memory location value.
      mem_val_t & AccessLocal(mem_key_t key) {
        if (!Has(local_mem, key)) local_mem[key] = default_mem_val;
        return local_mem[key];
      }

      /// Access input memory. This function returns a reference to memory location value if that location exists.
      /// If the location does not exist, set to default memory value and return reference to memory location value.
      mem_val_t & AccessInput(mem_key_t key) {
        if (!Has(input_mem, key)) input_mem[key] = default_mem_val;
        return input_mem[key];
      }

      /// Access output memory. This function returns a reference to memory location value if that location exists.
      /// If the location does not exist, set to default memory value and return reference to memory location value.
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

      bool operator<(const Instruction & other) const {
          return std::tie(id, args, affinity) < std::tie(other.id, other.args, other.affinity);
      }

    };

    using inst_t = Instruction;                    //< Convenient Instruction type alias.
    using inst_seq_t = emp::vector<inst_t>;        //< Convenient type alias for instruction sequence.
    using event_t = Event;                         //< Event type alias.
    using inst_lib_t = InstLib<EventDrivenGP_t>;   //< Instruction library type alias.
    using event_lib_t = EventLib<EventDrivenGP_t>; //< Event library type alias.

    /// Function struct. Defines an EventDrivenGP function.
    /// Each function has an associated:
    ///   * affinity: Function affinity. Analogous to the function's name.
    ///   * inst_seq: Instruction sequence. Sequence of instructions that make up the function.
    class Function {

    private:
      affinity_t affinity;          //< Function affinity. Analogous to the function's name.

    public:
      inst_seq_t inst_seq;          //< Instruction sequence. Sequence of instructions that make up the function.

    private:
      std::function<void()> fun_matchbin_refresh;          //< Callback to refresh matchbin.

    public:
      Function(
        const affinity_t & _aff=affinity_t(),
        const inst_seq_t & _seq=inst_seq_t(),
        std::function<void()> _fun_matchbin_refresh=[](){}
      ) : affinity(_aff)
      , inst_seq(_seq)
      , fun_matchbin_refresh(_fun_matchbin_refresh)
      { ; }

      Function& operator=(const Function& other) {

        if (this != &other) {
          // don't copy over fun_matchbin_refresh
          // if we're copying into an object (i.e., one already in a program)
          // we want to keep the current callback
          this->affinity = other.affinity;
          this->inst_seq = other.inst_seq;
          fun_matchbin_refresh();
        }

        return *this;
      }

      inst_t & operator[](size_t id) { return inst_seq[id]; }
      const inst_t & operator[](size_t id) const { return inst_seq[id]; }

      bool operator==(const Function & in) const {
        return inst_seq == in.inst_seq && affinity == in.affinity;
      }

      bool operator!=(const Function & in) const { return !(*this == in); }

      bool operator<(const Function & other) const {
          return std::tie(inst_seq, affinity) < std::tie(other.inst_seq, other.affinity);
      }

      size_t GetSize() const { return inst_seq.size(); }

      /// If this function is loaded onto hardware, we need to refresh the
      /// MatchBin whenever we make certain changes here
      void SetMatchBinRefreshFun(std::function<void()> fun) {
        fun_matchbin_refresh = fun;
      }

      const affinity_t & GetAffinity() { return affinity; }

      void SetAffinity(const affinity_t & aff) {
        if (affinity != aff) {
          affinity = aff;
          fun_matchbin_refresh();
         }
      }

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
    class Program {

    protected:
      using program_t = emp::vector<Function>;  //< Convenient type alias for sequence of functions.

      Ptr<const inst_lib_t> inst_lib;  //< Pointer to const instruction library associated with this program.
      program_t program;               //< Sequence of functions that make up this program.
      std::function<void()> fun_matchbin_refresh;

    public:
      Program(
        Ptr<const inst_lib_t> _ilib,
        const program_t & _prgm=program_t()
      ) : inst_lib(_ilib)
      , program(_prgm)
      , fun_matchbin_refresh([](){})
      { ; }
      Program(const Program &) = default;

      void Clear() {
        program.clear();
      }

      Function & operator[](size_t id) { return program[id]; }
      const Function & operator[](size_t id) const { return program[id]; }

      bool operator==(const Program & in) const { return program == in.program; }
      bool operator!=(const Program & in) const { return !(*this == in); }

      bool operator<(const Program & other) const {
          return program < other.program;
      }

      /// Get number of functions that make up this program.
      size_t GetSize() const { return program.size(); }

      /// Get the total number of instructions across all functions that make up this program.
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

      /// If this program is loaded onto hardware, we need to refresh the
      /// MatchBin whenever we make certain changes here
      void SetMatchBinRefreshFun(std::function<void()> fun) {
        fun_matchbin_refresh = fun;
      }

      void SetProgram(const program_t & _program) {
        program = _program;
        fun_matchbin_refresh();
      }

      void PushFunction(const Function & _function) {
        program.emplace_back(_function);
        program.back().SetMatchBinRefreshFun(
          [this](){ this->fun_matchbin_refresh(); }
        );
        fun_matchbin_refresh();
      }

      void DeleteFunction(const size_t fID) {
        program[fID] = program[GetSize() - 1];
        program.pop_back();
        fun_matchbin_refresh();
      }

      void PushFunction(const affinity_t & _aff=affinity_t(), const inst_seq_t & _seq=inst_seq_t()) {
        program.emplace_back(
          _aff,
          _seq,
          [this](){ this->fun_matchbin_refresh(); }
        );
        fun_matchbin_refresh();
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

      /// Load entire program from input stream.
      /// Warning: This function accepts a slightly different than what the Program's PrintProgram
      /// function prints out (for now, will add a PrintProgram variant that prints in this load function's accepted format).
      /// Program format:
      /// Fn-AFFINITY:
      ///   INST_NAME[AFFINITY](arg_0, ..., arg_max)
      ///   ...
      /// Fn-AFFINITY:
      ///   ...
      void Load(std::istream & input) {
        // Clear current program.
        Clear();
        std::string cur_line;
        emp::vector<std::string> line_components;
        while (!input.eof()) {
          std::getline(input, cur_line);
          remove_whitespace(cur_line); // Clear out whitespace.
          if (cur_line == empty_string()) continue; // Skip empty lines.
          // Are we looking the beginning of a function?
          slice(cur_line, line_components, '-');
          if (to_lower(line_components[0]) == "fn" && line_components.size() > 1) {
            // Extract function affinity.
            std::string & aff_str = line_components[1];
            affinity_t fun_aff;
            for (size_t i = 0; i < aff_str.size(); ++i) {
              if (i >= fun_aff.GetSize()) break;
              if (aff_str[i] == '1') fun_aff.Set(fun_aff.GetSize() - i - 1, true);
            }
            PushFunction(fun_aff);
          } else {
            // We must be looking at an instruction.
            affinity_t inst_aff;
            int a0 = 0; int a1 = 0; int a2 = 0;
            // Is there an affinity?
            size_t aff_begin = cur_line.find_first_of('[');
            size_t aff_end = cur_line.find_first_of(']');
            if ((aff_begin != std::string::npos) && (aff_end != std::string::npos) && (aff_begin < aff_end)) {
              // Found affinity.
              std::string aff_str = string_get_range(cur_line, aff_begin+1, aff_end-(aff_begin+1));
              for (size_t i = 0; i < aff_str.size(); ++i) {
                if (i >= inst_aff.GetSize()) break;
                if (aff_str[i] == '1') inst_aff.Set(inst_aff.GetSize() - i - 1, true);
              }
              // Pop affinity from cur_line.
              cur_line = string_get_range(cur_line, 0, aff_begin) + string_get_word(cur_line, aff_end+1);
            }
            // Are there arguments?
            size_t args_begin = cur_line.find_first_of('(');
            size_t args_end = cur_line.find_first_of(')');
            size_t args_cnt = 0;
            if ((args_begin != std::string::npos) && (args_end != std::string::npos) && (args_begin < args_end)) {
              // Found some arguments.
              std::string args_str = string_get_range(cur_line, args_begin+1, args_end-(args_begin+1));
              line_components.clear();
              // Extract arguments from arg str.
              slice(args_str, line_components, ',');
              if (args_cnt < line_components.size()) {
                emp_assert(is_valid(line_components[args_cnt], [](char c){ return is_digit(c) || c=='-'; })); // Yes yes, this doesn't catch case when '-' is in middle of the number...oh well.
                a0 = std::stoi(line_components[args_cnt]); ++args_cnt;
              }
              if (args_cnt < line_components.size()) {
                emp_assert(is_valid(line_components[args_cnt], [](char c){ return is_digit(c) || c=='-'; }));
                a1 = std::stoi(line_components[args_cnt]); ++args_cnt;
              }
              if (args_cnt < line_components.size()) {
                emp_assert(is_valid(line_components[args_cnt], [](char c){ return is_digit(c) || c=='-'; }));
                a2 = std::stoi(line_components[args_cnt]); ++args_cnt;
              }
              // Pop arguments from current line.
              cur_line = string_get_range(cur_line, 0, args_begin) + string_get_word(cur_line, args_end+1);
            }
            // All that's left should be the instruction name.
            emp_assert(inst_lib->GetID(cur_line) != (size_t)-1);
            // Push instruction to program.
            PushInst(cur_line, a0, a1, a2, inst_aff);
          }
        }
      }

      /// Print out a single instruction with its arguments.
      void PrintInst(const inst_t & inst, std::ostream & os=std::cout) const {
        os << inst_lib->GetName(inst.id);
        if (inst_lib->HasProperty(inst.id, "affinity")) {
          os << ' '; inst.affinity.Print(os);
        }
        const size_t num_args = inst_lib->GetNumArgs(inst.id);
        for (size_t i = 0; i < num_args; i++) {
          os << ' ' << inst.args[i];
        }
      }

      /// Fully print out a single instruction with its arguments/affinity.
      void PrintInstFull(const inst_t & inst, std::ostream & os=std::cout) const {
        os << inst_lib->GetName(inst.id);
        os << '['; inst.affinity.Print(os); os << ']';
        os << '(';
        for (size_t i = 0; i < MAX_INST_ARGS - 1; i++) {
          os << inst.args[i] << ',';
        } if (MAX_INST_ARGS > 0) { os << inst.args[MAX_INST_ARGS-1]; }
        os << ')';
      }

      /// Print out entire program.
      void PrintProgram(std::ostream & os=std::cout) const {
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

      /// Print out entire program.
      void PrintProgramFull(std::ostream & os=std::cout) const {
        for (size_t fID = 0; fID < GetSize(); fID++) {
          // Print out function name (affinity).
          os << "Fn-";
          program[fID].affinity.Print(os);
          os << ":\n";
          int depth = 0;
          for (size_t i = 0; i < program[fID].GetSize(); i++) {
            const inst_t & inst = program[fID][i];
            int num_spaces = 2 + (2 * depth);
            for (int s = 0; s < num_spaces; s++) os << ' ';
            PrintInstFull(inst, os);
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
    using trait_printer_t = std::function<void(std::ostream& os, TRAIT_T t)>;
  protected:
    Ptr<const event_lib_t> event_lib;     //< Pointer to const event library associated with this hardware.
    Ptr<Random> random_ptr;               //< Pointer to random object to use.
    bool random_owner;                    //< Does this hardware own it's random object? (necessary for cleanup responsibility resolution)
    program_t program;                    //< Hardware's associated program (set of functions).
    memory_t shared_mem;                  //< Hardware's shared memory map. All cores have access to the same shared memory.
    std::deque<event_t> event_queue;      //< Hardware's event queue. Where events go to be handled (in order of reception).
    TRAIT_T traits;                    //< Generic traits vector. Whatever uses the hardware must define/keep track of what traits mean.
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
    MATCHBIN_T matchBin;
    trait_printer_t fun_trait_print = [](std::ostream& os, TRAIT_T){os << "UNCONFIGURED TRAIT PRINT FUNCTION\n";};

    // TODO: disallow configuration of hardware while executing. (and any other functions that could sent things into a bad state)

  public:
    /// EventDrivenGP constructor. Give instance variables reasonable defaults. Allow for configuration
    /// post-construction.
    EventDrivenGP_AW(Ptr<const inst_lib_t> _ilib, Ptr<const event_lib_t> _elib, Ptr<Random> rnd=nullptr)
      : event_lib(_elib),
        // if no random pointer provided, create one
        random_ptr(rnd ? rnd : emp::NewPtr<Random>(-1)),
        random_owner(!rnd),
        program(_ilib),
        shared_mem(),
        event_queue(),
        traits(), errors(0),
        max_cores(DEFAULT_MAX_CORES), max_call_depth(DEFAULT_MAX_CALL_DEPTH),
        default_mem_value(DEFAULT_MEM_VALUE), min_bind_thresh(DEFAULT_MIN_BIND_THRESH),
        stochastic_fun_call(true),
        cores(max_cores), active_cores(), inactive_cores(max_cores), pending_cores(),
        exec_core_id(0), is_executing(false),
        matchBin(*random_ptr)
    {
      // Give the program our matchbin clear cache callback.
      program.SetMatchBinRefreshFun( [this](){ this->RefreshMatchBin(); } );

      // Add all available cores to inactive.
      for (size_t i = 0; i < inactive_cores.size(); ++i)
        inactive_cores[i] = (inactive_cores.size() - 1) - i;
      // Spin up main core (will spin up on function ID = 0).
      SpawnCore(0, memory_t(), true);

    }

    EventDrivenGP_AW(const inst_lib_t & _ilib, const event_lib_t & _elib, Ptr<Random> rnd=nullptr)
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
        exec_core_id(in.exec_core_id), is_executing(in.is_executing),
        fun_trait_print(in.fun_trait_print)
    {
      in.random_ptr = nullptr;
      in.random_owner = false;
      in.event_lib = nullptr;
      in.program.inst_lib = nullptr;
      program.SetMatchBinRefreshFun( [this](){ this->RefreshMatchBin(); } );
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
        exec_core_id(in.exec_core_id), is_executing(in.is_executing),
        fun_trait_print(in.fun_trait_print)
    {
      if (in.random_owner) NewRandom();
      else random_ptr = in.random_ptr;
      program.SetMatchBinRefreshFun( [this](){ this->RefreshMatchBin(); } );
    }

    ~EventDrivenGP_AW() {
      if (random_owner) random_ptr.Delete();
    }

    // ---------- Hardware Control ----------
    /// Reset everything, including program.
    /// Not allowed to Reset during execution.
    void Reset() {
      emp_assert(!is_executing);
      ResetHardware();
      traits = TRAIT_T();
      program.Clear();
    }

    /// clear program, this also requires resetting hardware
    void ResetProgram() {
      emp_assert(!is_executing);
      ResetHardware();
      program.Clear();
    }

    /// Reset only hardware, not program.
    /// Not allowed to reset hardware during execution.
    void ResetHardware() {
      emp_assert(!is_executing);
      shared_mem.clear();
      event_queue.clear();
      for (size_t i = 0; i < cores.size(); ++i) cores[i].clear();
      active_cores.clear();
      pending_cores.clear();
      inactive_cores.resize(max_cores);
      // Add all available cores to inactive.
      for (size_t i = 0; i < inactive_cores.size(); ++i)
        inactive_cores[i] = (inactive_cores.size() - 1) - i;
      exec_core_id = (size_t)-1;
      errors = 0;
      is_executing = false;
    }

    /// Spawn core with function that has best match to provided affinity. Do nothing if no
    /// functions match above the provided threshold.
    /// Initialize function state with provided input memory.
    /// Will fail if no inactive cores to claim.
    void SpawnCore(const affinity_t & affinity, double threshold, const memory_t & input_mem=memory_t(), bool is_main=false) {
      if (!inactive_cores.size()) return; // If there are no unclaimed cores, just return.
      size_t fID;
      emp::vector<size_t> best_matches{FindBestFuncMatch(affinity)};
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

    // ---------- Accessors ----------
    /// Get instruction library associated with hardware's program.
    Ptr<const inst_lib_t> GetInstLib() const { return program.GetInstLib(); }

    /// Get event library associated with hardware.
    Ptr<const event_lib_t> GetEventLib() const { return event_lib; }

    /// Get reference to random number generator used by this hardware.
    Random & GetRandom() { return *random_ptr; }

    /// Get pointer to random number generator used by this hardware.
    Ptr<Random> GetRandomPtr() { return random_ptr; }

    /// Get program loaded on this hardware.
    const program_t & GetConstProgram() const { return program; }
    program_t & GetProgram() { return program; }


    /// Get reference to a particular function in hardware's program.
    const Function & GetFunction(size_t fID) const {
      emp_assert(ValidFunction(fID));
      return program[fID];
    }

    /// Get reference to particular instruction in hardware's program given function ID and instruction position.
    const inst_t & GetInst(size_t fID, size_t pos) const {
      emp_assert(ValidPosition(fID, pos));
      return program[fID].inst_seq[pos];
    }

    /// Get the stored trait in hardware's program.
    TRAIT_T& GetTrait() { return traits; }

    /// Get the stored trait in hardware's program.
    const TRAIT_T& GetTrait() const { return traits; }

    /// Get current number of errors committed by this hardware.
    size_t GetNumErrors() const { return errors; }

    /// Get hardware's minimum binding threshold (threshold used to determine if two affinities are
    /// close enough to bind).
    double GetMinBindThresh() const { return min_bind_thresh; }

    /// Get the maximum number of cores allowed to run simultaneously on this hardware object.
    size_t GetMaxCores() const { return max_cores; }

    /// Get the maximum call depth allowed on this hardware object (max call states allowed on a single core's call stack at a time).
    size_t GetMaxCallDepth() const { return max_call_depth; }

    /// Get the default memory value for local/shared/input/output memory maps.
    mem_val_t GetDefaultMemValue() const { return default_mem_value; }

    /// Is this hardware object configured to allow stochasticity in function calling?
    /// Hardware is only stochastic when calling/event affinity is equidistant from two or more functions.
    bool IsStochasticFunCall() const { return stochastic_fun_call; }

    /// Get all hardware cores.
    /// NOTE: use responsibly!
    emp::vector<exec_stk_t> & GetCores() { return cores; }

    /// Get all hardware cores.
    /// NOTE: use responsibly!
    const emp::vector<exec_stk_t> & GetCores() const { return cores; }

    /// Get the currently executing core ID. If hardware is not in the middle of an execution cycle
    /// (the SingleProcess function), this will return the first core ID in active_cores, which will
    /// typically be the core on which main is running.
    size_t GetCurCoreID() { return exec_core_id; }

    /// Get a reference to the current core/execution stack.
    exec_stk_t & GetCurCore() { emp_assert(exec_core_id < cores.size()); return cores[exec_core_id]; }

    /// Get a reference to the current local call state.
    State & GetCurState() {
      emp_assert(exec_core_id < cores.size() && cores[exec_core_id].size());
      return cores[exec_core_id].back();
    }

    /// Get a reference to hardware's shared memory map.
    memory_t & GetSharedMem() { return shared_mem; }

    /// Get a particular value in shared memory map stored @ location indicated by key.
    /// If key cannot be found, return the default memory value.
    mem_val_t GetShared(mem_key_t key) const { return Find(shared_mem, key, default_mem_value); }

    /// Get a reference to a location in the shared memory map as indicated by key.
    /// If key cannot be found, add key:default_mem_value to map and return newly added location.
    mem_val_t & AccessShared(mem_key_t key) {
      if (!Has(shared_mem, key)) shared_mem[key] = default_mem_value;
      return (shared_mem)[key];
    }

    // ------- Configuration -------
    /// Set minimum binding threshold.
    /// Requirement: minimum binding threshold >= 0.0
    void SetMinBindThresh(double val) {
      emp_assert(val >= 0.0);
      min_bind_thresh = emp::Max(val, 0.0);
    }

    /// Set the maximum number of cores that are allowed to be running/active simultaneously on
    /// this hardware object.
    /// Warning: If you decrease max cores, you may kill actively running cores.
    /// Warning: If you decrease max cores, we make no guarantees about which particular cores are killed. This could have adverse effects.
    /// Requirement: Must have max cores > 0 and cannot set max cores while executing (while in SingleProcess function).
    /// To sum up, be careful if you're going to decreasing max cores after you've run the hardware.
    void SetMaxCores(size_t val) {
      emp_assert(val > 0 && !is_executing);
      // Resize cores to max_cores.
      cores.resize(val);
      if (val > max_cores) {
        // Increasing total available cores, add new cores to inactive_cores vector.
        // To decrease risk of unexpected behavior, maintain reverse ordering of inactive_cores.
        //  - Because this is a configuration function, it's fine for it to be pretty slow. As such, I'll take the easy way out here.
        for (size_t i = max_cores; i < val; i++)
          inactive_cores.insert(inactive_cores.begin(), i);
      } else if (val < max_cores) {
        // Decreasing total available cores, adjust active and inactive core vectors (maintain relative ordering in each).
        //  - No need to worry about pending core queue as SetMaxCores is ill-defined/not allowed when is_executing is true.
        // Fix active_cores (maintain relative ordering).
        size_t ac_idx = 0;
        size_t ac_cnt = active_cores.size();
        size_t ac_adjust = 0;
        while (ac_idx < ac_cnt) {
          size_t core_id = active_cores[ac_idx];
          if (core_id >= val) { // Do we need to eliminate this core_id from active cores?
            // If yes, set to -1 and increment adjust.
            active_cores[ac_idx - ac_adjust] = (size_t)-1;
            ++ac_adjust;
          } else if (ac_adjust) { // Still valid core ID, so do we need to defragment?
            active_cores[ac_idx] = (size_t)-1;
            active_cores[ac_idx - ac_adjust] = core_id;
          }
          ++ac_idx;
        }
        active_cores.resize(ac_cnt - ac_adjust);
        // Fix inactive cores (maintain relative ordering).
        size_t ic_idx = 0;
        size_t ic_cnt = inactive_cores.size();
        size_t ic_adjust = 0;
        while (ic_idx < ic_cnt) {
          size_t core_id = inactive_cores[ic_idx];
          if (core_id >= val) { // Do we need to eliminate this core_id from inactive cores?
            // If yes, set to -1 and increment adjust.
            inactive_cores[ic_idx - ic_adjust] = (size_t)-1;
            ++ic_adjust;
          } else if (ic_adjust) { // Still valid core ID, so do we need to defragment?
            inactive_cores[ic_idx] = (size_t)-1;
            inactive_cores[ic_idx - ic_adjust] = core_id;
          }
          ++ic_idx;
        }
        inactive_cores.resize(ic_cnt - ic_adjust);
        // Make sure exec_core_id is still valid.
        if (active_cores.size()) exec_core_id = active_cores[0];
      } // No need to do anything if val == max_cores.

      max_cores = val; // Update max_cores.
    }

    /// Configure max call depth.
    /// Warning: will not retroactively enforce new max call depth.
    /// Requirement: max call depth must be > 0.
    void SetMaxCallDepth(size_t val) { emp_assert(val > 0); max_call_depth = val; }

    /// Configure the default memory value.
    void SetDefaultMemValue(mem_val_t val) {
      default_mem_value = val;
      // Propagate default mem value through execution stacks.
      for (size_t i = 0; i < cores.size(); ++i)
        for (size_t k = 0; k < cores[i].size(); ++k)
          cores[i][k].SetDefaultMemValue(val);
    }

    /// Configure whether or not function calls should be stochastic if we have two or more matches
    /// that are equidistant from caller/event affinity.
    void SetStochasticFunCall(bool val) { stochastic_fun_call = val; }

    /// Set trait in traits vector given by id to value given by val.
    /// Will resize traits vector if given id is greater than current traits vector size.
    void SetTrait(TRAIT_T t) {
      traits = t;
    }

    /// Shortcut to this hardware object's program's SetInst function of the same signature.
    void SetInst(size_t fID, size_t pos, const inst_t & inst) {
      emp_assert(ValidPosition(fID, pos));
      program.SetInst(fID, pos, inst);
    }

    /// Shortcut to this hardware object's program's SetInst function of the same signature.
    void SetInst(size_t fID, size_t pos, size_t id, arg_t a0=0, arg_t a1=0, arg_t a2=0,
                 const affinity_t & aff=affinity_t()) {
      emp_assert(ValidPosition(fID, pos));
      program.SetInst(fID, pos, id, a0, a1, a2, aff);
    }

    /// Set program for this hardware object.
    void SetProgram(const program_t & _program) {
      program = _program;
      program.SetMatchBinRefreshFun( [this](){ this->RefreshMatchBin(); } );
      RefreshMatchBin();
    }

    /// Shortcut to this hardware object's program's PushFunction operation of the same signature.
    void PushFunction(const Function & _function) { program.PushFunction(_function); }

    /// Shortcut to this hardware object's program's PushFunction operation of the same signature.
    void PushFunction(const affinity_t & _aff=affinity_t(), const inst_seq_t & _seq=inst_seq_t()) {
      program.PushFunction(_aff, _seq);
    }

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

    /// Load entire program from input stream.
    /// Warning: This function accepts a slightly different than what the Program's PrintProgram
    /// function prints out (for now, will add a PrintProgram variant that prints in this load function's accepted format).
    /// Program format:
    /// Fn-AFFINITY:
    ///   INST_NAME[AFFINITY](arg_0, ..., arg_max)
    ///   ...
    /// Fn-AFFINITY:
    ///   ...
    void Load(std::istream & input) { program.Load(input); }

    // ---------- Hardware Utilities ----------
    /// Generate new random number generator for this hardware object with the given seed value.
    void NewRandom(int seed=-1) {
      if (random_owner) random_ptr.Delete();
      else random_ptr = nullptr;
      random_ptr.New(seed);
      random_owner = true;
    }

    /// Is program position defined by the given function ID (fID) and instruction position (pos)
    /// a valid position in this hardware object's program?
    bool ValidPosition(size_t fID, size_t pos) const { return program.ValidPosition(fID, pos); }

    /// Is the function given by function ID (fID) a valid function in this hardware object's program?
    bool ValidFunction(size_t fID) const { return program.ValidFunction(fID); }

    /// Set given shared memory map location (key) to given value.
    void SetShared(mem_key_t key, mem_val_t value) { shared_mem[key] = value; }

    /// Given valid function pointer and instruction pointer, find next end of block (at current block level).
    /// This is not guaranteed to return a valid IP. At worst, it'll return an IP == function.inst_seq.size().
    size_t FindEndOfBlock(size_t fp, size_t ip) {
      emp_assert(ValidFunction(fp));
      Ptr<const inst_lib_t> inst_lib = program.GetInstLib();
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

    /// Close current block in the current local program state if there is one to close.
    /// If not, do nothing.
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

    /// Open a block in the current local program state as specified by begin, end, and type.
    void OpenBlock(size_t begin, size_t end, BlockType type) {
      State & state = GetCurState();
      state.block_stack.emplace_back(begin, end, type);
    }

    /// If there's a block to break out of in current local program state, break out (to eob).
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
    emp::vector<size_t> FindBestFuncMatch(const affinity_t & affinity) {
      // no need to transform to values because we're using
      // matchbin uids equivalent to function uids
      // also, we've delegated responsibility RE: the number of matches to
      // return to the MatchBin Selector
      return matchBin.Match(affinity);
    }

    MATCHBIN_T& GetMatchBin(){
      return matchBin;
    }

    const MATCHBIN_T& GetMatchBin() const {
      return matchBin;
    }

    void RefreshMatchBin(){
      matchBin.Clear();
      for (size_t i = 0; i < program.GetSize(); ++i) {
        matchBin.Set(i, program[i].GetAffinity(), i);
      }
    }

    /// Call function with best affinity match above threshold.
    /// If not candidate functions found, do nothing.
    void CallFunction(const affinity_t & affinity, double threshold) {
      // Are we at max call depth? -- If so, call fails.
      if (GetCurCore().size() >= max_call_depth) return;
      size_t fID;
      emp::vector<size_t> best_matches{FindBestFuncMatch(affinity)};
      if (best_matches.empty()) return;
      if (best_matches.size() == 1) fID = best_matches[0];
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

    // ---------- Hardware Execution ----------
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
      size_t core_cnt = active_cores.size();
      size_t adjust = 0;
      is_executing = true;
      while (active_core_idx < core_cnt) {
        // Set the current core to core at core_idx.
        exec_core_id = active_cores[active_core_idx]; // Here's the core we're about to execute.
        // Do we need to move current core over in the execution core vector to make it contiguous?
        if (adjust) {
          active_cores[active_core_idx] = (size_t)-1;
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
          program.GetInstLib()->ProcessInst(*this, program[fp].inst_seq[ip]);
        }
        // After processing, is the core still active?
        if (GetCurCore().empty()) {
          // Free core. Mark as inactive.
          active_cores[active_core_idx - adjust] = (size_t)-1;
          inactive_cores.emplace_back(exec_core_id);
          ++adjust;
        }
        ++active_core_idx;
      }
      is_executing = false;
      // Update execution stack size to be accurate.
      active_cores.resize(core_cnt - adjust);
      // Spawn any cores that happened during execution.
      while (pending_cores.size()) {
        active_cores.emplace_back(pending_cores.front());
        pending_cores.pop_front();
      }
      // Set cur core to be first execution stack (which should always be main).
      if (active_cores.size()) exec_core_id = active_cores[0];
    }

    /// Advance hardware by some arbitrary number instructions.
    void Process(size_t num_inst) {
      for (size_t i = 0; i < num_inst; i++) SingleProcess();
    }

    // ---------- Printing ----------
    /// Print given event using given output stream (default = std::cout).
    void PrintEvent(const event_t & event, std::ostream & os=std::cout) {
      os << "[" << event_lib->GetName(event.id) << ","; event.affinity.Print(os); os << ",(";
      for (const auto & mem : event.msg) std::cout << "{" << mem.first << ":" << mem.second << "}";
      os << "),(Properties:";
      for (const auto & property : event.properties) std::cout << " " << property;
      os << ")]";
    }

    /// Print given instruction using given output stream (default = std::cout).
    void PrintInst(const inst_t & inst, std::ostream & os=std::cout) {
      program.PrintInst(inst, os);
    }

    /// Print hardware traits using given output stream (default = std::cout).
    void PrintTraits(std::ostream & os=std::cout) {
      fun_trait_print(os, traits);
    }

    void SetTraitPrintFun(const trait_printer_t& t){
      fun_trait_print = t;
    }

    /// Print out entire program using given output stream (default = std::cout).
    void PrintProgram(std::ostream & os=std::cout) {
      program.PrintProgram(os);
    }

    /// Print out entire program using given output stream (default = std::cout).
    void PrintProgramFull(std::ostream & os=std::cout) {
      program.PrintProgramFull(os);
    }

    /// Print out current state (full) of virtual hardware using given output stream (default = std::cout).
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
      for (auto mem : shared_mem) os << '{' << mem.first << ':' << mem.second << '}';
      os << '\n';
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
          if (ValidPosition(state.func_ptr, state.inst_ptr)){
            inst_t inst = GetInst(state.func_ptr, state.inst_ptr);
            PrintInst(inst, os);
            os << ")"<<"\n";
            emp::vector<std::string> additional_state_info{"Call","Fork","SetRegulator","AdjRegulator"};
            if (std::find(additional_state_info.begin(), additional_state_info.end(), GetInstLib()->GetName(inst.id)) != additional_state_info.end()){
              std::unordered_map<size_t, double> probabilities;
              for(unsigned int i = 0; i < 100; ++i){
                emp::vector<size_t> matches = FindBestFuncMatch(inst.affinity);
                if(matches.size() == 1){
                  if(probabilities.find(matches[0])==probabilities.end()){probabilities[matches[0]] = 0;}
                  ++probabilities[matches[0]];
                }
              }
              emp::vector<std::pair<double, size_t>>best_matches;
              for(auto &[id, prob] : probabilities){best_matches.emplace_back(prob, id);}
              std::sort(best_matches.begin(), best_matches.end());
              std::reverse(best_matches.begin(), best_matches.end());
              for(auto &[prob, id] : best_matches){os <<"      Fn-"<< id << ": " << prob <<"%\n";}
            }
          }

          else
            os << "NONE"<<")"<<"\n";
          os << "    Func ptr: " << state.func_ptr << "\n";
          os << "    Input memory: ";
          for (auto mem : state.input_mem) os << "{" << mem.first << ":" << mem.second << "}";
          os << "\n";
          os << "    Local memory: ";
          for (auto mem : state.local_mem) os << "{" << mem.first << ":" << mem.second << "}";
          os << "\n";
          os << "    Output memory: ";
          for (auto mem : state.output_mem) os << "{" << mem.first << ":" << mem.second << "}";
          os << "\n";
          os << "    ---\n";
        }
      }
    }

    // -- Default Instructions --
    /// Default instruction: Inc
    /// Number arguments: 1
    /// Description: Increment value in local memory[Arg1].
    static void Inst_Inc(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      ++state.AccessLocal(inst.args[0]);
    }

    /// Default instruction: Dec
    /// Number of arguments: 1
    /// Description: Decrement value in local memory[Arg1].
    static void Inst_Dec(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      --state.AccessLocal(inst.args[0]);
    }

    /// Default instruction: Not
    /// Number of arguments: 1
    /// Description: Logically toggle value in Local[Arg1].
    static void Inst_Not(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      state.SetLocal(inst.args[0], state.GetLocal(inst.args[0]) == 0.0);
    }

    /// Default instruction: Add
    /// Number of arguments: 3
    /// Description: Local[Arg3] = Local[Arg1] + Local[Arg2]
    static void Inst_Add(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      state.SetLocal(inst.args[2], state.AccessLocal(inst.args[0]) + state.AccessLocal(inst.args[1]));
    }

    /// Default instruction: Sub
    /// Number of arguments: 3
    /// Description: Local[Arg3] = Local[Arg1] - Local[Arg2]
    static void Inst_Sub(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      state.SetLocal(inst.args[2], state.AccessLocal(inst.args[0]) - state.AccessLocal(inst.args[1]));
    }

    /// Default instruction: Mult
    /// Number of arguments: 3
    /// Description: Local[Arg3] = Local[Arg1] * Local[Arg2]
    static void Inst_Mult(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      state.SetLocal(inst.args[2], state.AccessLocal(inst.args[0]) * state.AccessLocal(inst.args[1]));
    }

    /// Default instruction: Div
    /// Number of arguments: 3
    /// Description: Local[Arg3] = Local[Arg1] / Local[Arg2]
    /// If Local[Arg2] == 0, division fails and increment hardware errors.
    static void Inst_Div(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      const double denom = state.AccessLocal(inst.args[1]);
      if (denom == 0.0) ++hw.errors;
      else state.SetLocal(inst.args[2], state.AccessLocal(inst.args[0]) / denom);
    }

    /// Default instruction: Mod
    /// Number of arguments: 3
    /// Description: Local[Arg3] = Local[Arg1] % Local[Arg2]
    /// If Local[Arg2] == 0, modulus fails and increment hardware errors.
    static void Inst_Mod(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      const int base = (int)state.AccessLocal(inst.args[1]);
      const int num = (int)state.AccessLocal(inst.args[0]);
      if (base == 0) ++hw.errors;
      else state.SetLocal(inst.args[2], static_cast<int64_t>(num) % static_cast<int64_t>(base));
    }

    /// Default instruction: TestEqu
    /// Number of arguments: 3
    /// Description: Local[Arg3] = Local[Arg1] == Local[Arg2]
    static void Inst_TestEqu(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      state.SetLocal(inst.args[2], state.AccessLocal(inst.args[0]) == state.AccessLocal(inst.args[1]));
    }

    /// Default instruction: TestNEqu
    /// Number of arguments: 3
    /// Description: Local[Arg3] = Local[Arg1] != Local[Arg2]
    static void Inst_TestNEqu(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      state.SetLocal(inst.args[2], state.AccessLocal(inst.args[0]) != state.AccessLocal(inst.args[1]));
    }

    /// Default instruction: TestLess
    /// Number of arguments: 3
    /// Description: Local[Arg3] = Local[Arg1] < Local[Arg2]
    static void Inst_TestLess(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      state.SetLocal(inst.args[2], state.AccessLocal(inst.args[0]) < state.AccessLocal(inst.args[1]));
    }

    /// Default instruction: If
    /// Number of arguments: 1
    /// Description: If (Local[Arg1] != 0) { execute block } else { skip block }
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

    /// Default instruction: While
    /// Number of arguments: 1
    /// Description: While (Local[Arg1] != 0) { execute block }
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

    /// Default instruction: Countdown
    /// Number of arguments: 1
    /// Description: While (Local[Arg1] != 0) { Local[Arg1]-- then execute block }
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

    /// Default instruction: Break
    /// Number of arguments: 0
    /// Description: Break out of current block if there's a block to close.
    static void Inst_Break(EventDrivenGP_t & hw, const inst_t & inst) {
      hw.BreakBlock();
    }

    /// Default instruction: Close
    /// Number of arguments: 0
    /// Description: Marks the end of a block.
    static void Inst_Close(EventDrivenGP_t & hw, const inst_t & inst) {
      hw.CloseBlock();
    }

    /// Default instruction: Call
    /// Number of arguments: 0
    /// Description: Call function with the strongest affinity match to call affinity.
    static void Inst_Call(EventDrivenGP_t & hw, const inst_t & inst) {
      hw.CallFunction(inst.affinity, hw.GetMinBindThresh());
    }

    /// Default instruction: Return
    /// Number of arguments: 0
    /// Description: Return from current function call unless in main function call.
    static void Inst_Return(EventDrivenGP_t & hw, const inst_t & inst) {
      hw.ReturnFunction();
    }

    /// Default instruction: SetMem
    /// Number of arguments: 2
    /// Description: Local[Arg1] = ValueOf(Arg2)
    static void Inst_SetMem(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      state.SetLocal(inst.args[0], (double)inst.args[1]);
    }

    /// Default instruction: CopyMem
    /// Number of arguments: 2
    /// Description: Local[Arg2] = Local[Arg1]
    static void Inst_CopyMem(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      state.SetLocal(inst.args[1], state.AccessLocal(inst.args[0]));
    }

    /// Default instruction: SwapMem
    /// Number of arguments: 2
    /// Description: Swap(Local[Arg1], Local[Arg2])
    static void Inst_SwapMem(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      double val0 = state.AccessLocal(inst.args[0]);
      state.SetLocal(inst.args[0], state.GetLocal(inst.args[1]));
      state.SetLocal(inst.args[1], val0);
    }

    /// Default instruction: Input
    /// Number of arguments: 2
    /// Description: Local[Arg2] = Input[Arg1]
    static void Inst_Input(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      state.SetLocal(inst.args[1], state.AccessInput(inst.args[0]));
    }

    /// Default instruction: Output
    /// Number of arguments: 2
    /// Description: Output[Arg2] = Local[Arg1]
    static void Inst_Output(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      state.SetOutput(inst.args[1], state.AccessLocal(inst.args[0]));
    }

    /// Default instruction: Commit
    /// Number of arguments: 2
    /// Description: Shared[Arg2] = Local[Arg1]
    static void Inst_Commit(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      hw.SetShared(inst.args[1], state.AccessLocal(inst.args[0]));
    }

    /// Default instruction: Pull
    /// Number of arguments: 2
    /// Description: Local[Arg2] = Shared[Arg1]
    static void Inst_Pull(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      state.SetLocal(inst.args[1], hw.AccessShared(inst.args[0]));
    }

    /// Default instruction: Fork
    /// Number of instruction arguments: 0
    /// Description: Self-signal. Fork a new thread, using tag-based referencing to determine the appropriate
    ///              function to call on the new thread.
    static void Inst_Fork(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      hw.SpawnCore(inst.affinity, hw.GetMinBindThresh(), state.local_mem, false);
    }

    /// Default instruction: Terminate
    /// Number of instruction arguments: 0
    /// Description: Terminate the thread that executes this instruction.
    /// WARNING: This instruction does not respect any 'main' function calls.
    ///          *Any* thread where this is called is terminated.
    static void Inst_Terminate(EventDrivenGP_t & hw, const inst_t & inst) {
      // Pop all the call states from current core.
      exec_stk_t & core = hw.GetCurCore();
      core.resize(0);
    }

    /// Default instruction: Nop
    /// Number of arguments: 0
    /// Description: No operation.
    static void Inst_Nop(EventDrivenGP_t & hw, const inst_t & inst) { ; }

    /// Default instruction: BroadcastMsg
    /// Number of arguments: 0
    /// Description: Trigger a Message event where the event's affinity is equal to this instructions
    /// affinity and the event's message payload is equal to the current local program state output buffer.
    /// Event properties will indicate that this is a broadcast.
    static void Inst_BroadcastMsg(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      hw.TriggerEvent("Message", inst.affinity, state.output_mem, {"broadcast"});
    }

    /// Default instruction: SendMsg
    /// Number of arguments: 0
    /// Description: Trigger a Message event where the event's affinity is equal to this instructions
    /// affinity and the event's message payload is equal to the current local program state output buffer.
    /// Event properties will indicate that this is a send.
    static void Inst_SendMsg(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      hw.TriggerEvent("Message", inst.affinity, state.output_mem, {"send"});
    }

    /// Non-default instruction: RngDouble
    /// Number of arguments: 1
    /// Description: Draw a value between 0 and 1 from the onboard RNG and store it in Local[Arg1]
    static void Inst_RngDouble(EventDrivenGP_t & hw, const inst_t & inst) {
      State & state = hw.GetCurState();
      state.SetLocal(inst.args[0], hw.GetRandom().GetDouble());
    }

    /// Non-default instruction: SetRegulator
    /// Number of arguments: 2
    /// Description: Sets the regulator of a tag in the matchbin.
    static void Inst_SetRegulator(EventDrivenGP_t & hw, const inst_t & inst){
      const State & state = hw.GetCurState();
      emp::vector<size_t> best_fun = hw.GetMatchBin().MatchRaw(
        inst.affinity,
        1
      );

      if (best_fun.size() == 0){ return; }

      double regulator = state.GetLocal(inst.args[0]);
      if(regulator < 0){
        regulator = std::max(regulator, std::numeric_limits<double>::min());
        regulator /= std::numeric_limits<double>::min();
      }else{
        regulator += 1.0;
      }
      hw.GetMatchBin().SetRegulator(best_fun[0], regulator);

      const size_t dur = 2 + state.GetLocal(inst.args[1]);
      hw.GetMatchBin().GetVal(best_fun[0]) = dur;

    }

    /// Non-default instruction: SetOwnRegulator
    /// Number of arguments: 2
    /// Description: Sets the regulator of the currently executing function.
    static void Inst_SetOwnRegulator(EventDrivenGP_t & hw, const inst_t & inst){
      const State & state = hw.GetCurState();
      double regulator = state.GetLocal(inst.args[0]);
      if(regulator < 0){
        regulator = std::max(regulator, std::numeric_limits<double>::min());
        regulator /= std::numeric_limits<double>::min();
      }else{
        regulator += 1.0;
      }
      hw.GetMatchBin().SetRegulator(state.GetFP(), regulator);

      const size_t dur = 2 + state.GetLocal(inst.args[1]);
      hw.GetMatchBin().GetVal(state.GetFP()) = dur;

    }

    /// Non-default instruction: AdjRegulator
    /// Number of arguments: 3
    /// Description: adjusts the regulator of a tag in the matchbin
    /// towards a target.
    static void Inst_AdjRegulator(EventDrivenGP_t & hw, const inst_t & inst){
      const State & state = hw.GetCurState();
      emp::vector<size_t> best_fun = hw.GetMatchBin().MatchRaw(
        inst.affinity,
        1
      );
      if (!best_fun.size()) return;

      double target = state.GetLocal(inst.args[0]);
      if(target < 0) {
        target = std::max(target, std::numeric_limits<double>::min());
        target /= std::numeric_limits<double>::min();
      } else {
        target += 1.0;
      }

      const double budge = emp::Mod(state.GetLocal(inst.args[1])+0.2, 1.0);
      const double cur = hw.GetMatchBin().ViewRegulator(best_fun[0]);

      hw.GetMatchBin().SetRegulator(
        best_fun[0],
        target * budge + cur * (1 - budge)
      );

      const size_t dur = 2 + state.GetLocal(inst.args[2]);
      hw.GetMatchBin().GetVal(best_fun[0]) = dur;

    }

    /// Non-default instruction: AdjOwnRegulator
    /// Number of arguments: 3
    /// Description: adjusts the regulator of a tag in the matchbin
    /// towards a target.
    static void Inst_AdjOwnRegulator(EventDrivenGP_t & hw, const inst_t & inst){
      const State & state = hw.GetCurState();

      double target = state.GetLocal(inst.args[0]);
      if(target < 0) {
        target = std::max(target, std::numeric_limits<double>::min());
        target /= std::numeric_limits<double>::min();
      } else {
        target += 1.0;
      }

      const double budge = emp::Mod(state.GetLocal(inst.args[1])+0.2, 1.0);
      const double cur = hw.GetMatchBin().ViewRegulator(state.GetFP());

      hw.GetMatchBin().SetRegulator(
        state.GetFP(),
        target * budge + cur * (1 - budge)
      );

      const size_t dur = 2 + state.GetLocal(inst.args[2]);
      hw.GetMatchBin().GetVal(state.GetFP()) = dur;

    }

    /// Non-default instruction: ExtRegulator
    /// Number of arguments: 1
    /// Description: extends the decay counter of a
    /// regulator of a tag in the matchbin.
    static void Inst_ExtRegulator(EventDrivenGP_t & hw, const inst_t & inst){
      const State & state = hw.GetCurState();
      emp::vector<size_t> best_fun = hw.GetMatchBin().MatchRaw(
        inst.affinity,
        1
      );
      if (!best_fun.size()) return;

      const size_t dur = 1 + state.GetLocal(inst.args[0]);
      hw.GetMatchBin().GetVal(best_fun[0]) += dur;

    }


    /// Non-default instruction: SenseRegulator
    /// Number of arguments: 1
    /// Description: senses the value of the regulator of another function.
    static void Inst_SenseRegulator(EventDrivenGP_t & hw, const inst_t & inst){
      State & state = hw.GetCurState();
      emp::vector<size_t> best_fun = hw.GetMatchBin().MatchRaw(
        inst.affinity,
        1
      );
      if (best_fun.size() == 1){
        state.SetLocal(
          inst.args[0],
          hw.GetMatchBin().ViewRegulator(best_fun[0])
        );
      }
    }

    /// Non-default instruction: SenseOwnRegulator
    /// Number of arguments: 1
    /// Description: senses the value of the regulator the current function.
    static void Inst_SenseOwnRegulator(EventDrivenGP_t & hw, const inst_t & inst){
      State & state = hw.GetCurState();
      state.SetLocal(
        inst.args[0],
        hw.GetMatchBin().ViewRegulator(state.GetFP())
      );
    }

    /// Non-default instruction: Terminal
    /// Number of arguments: 1
    /// Description: writes a genetically-encoded value into a register.
    template<typename MaxRatio=std::ratio<1>, typename MinRatio=std::ratio<0>>
    static void Inst_Terminal(EventDrivenGP_t & hw, const inst_t & inst) {

      constexpr double max = static_cast<double>(MaxRatio::num) / MaxRatio::den;
      constexpr double min = static_cast<double>(MinRatio::num) / MinRatio::den;

      State & state = hw.GetCurState();
      const auto & tag = inst.affinity;

      const double val = (
        tag.GetDouble() / tag.MaxDouble()
      ) * (max - min) - min;

      state.SetLocal(inst.args[0], val);

    }


    /// Get a pointer to const default instruction library. Will only construct the default instruction library once.
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
        inst_lib.AddInst("Fork", Inst_Fork, 0, "Fork a new thread, using tag-based referencing to determine which function to call on the new thread.", ScopeType::BASIC, 0, {"affinity"});
        inst_lib.AddInst("Terminate", Inst_Terminate, 0, "Terminate current thread.");
        inst_lib.AddInst("Nop", Inst_Nop, 0, "No operation.");
      }
      return &inst_lib;
    }

    /// Default event handler: Message
    /// Description: Handle a message by spawning a new core (if we're not already maxed out)
    /// with the function that best matches the messages affinity. Set the function's input buffer
    /// to be equal to the contents of the message event's message contents.
    static void HandleEvent_Message(EventDrivenGP_t & hw, const event_t & event) {
      // Spawn new core.
      hw.SpawnCore(event.affinity, hw.GetMinBindThresh(), event.msg);
    }

    /// Get a pointer to const default event library. Will only construct the default event library once.
    /// Note: the default event library does not construct any default dispatch functions. This is
    /// the responsibility of whatever is using the EventDrivenGP hardware.
    static Ptr<const EventLib<EventDrivenGP_t>> DefaultEventLib() {
      static event_lib_t event_lib;
      if (event_lib.GetSize() == 0) {
        event_lib.AddEvent("Message", HandleEvent_Message, "Event for exchanging messages (agent-agent, world-agent, etc.)");
      }
      return &event_lib;
    }
  };

  /// A convenient shortcut for using EventDrivenGP_AW class with affinity width set to a default of 8.
  using EventDrivenGP = EventDrivenGP_AW<8>;
}

#endif
