#ifndef EMP_HARDWARE_CPU_H
#define EMP_HARDWARE_CPU_H

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  HardwareCPU is a basic, CPU-style virtual hardware object.
//
//  This is a templated type that allows compile-time configuration of properties.
//   CPU_SCALE = How many components of each type (stacks, heads, memory, nops) are available?
//   STACK_SIZE = Maximum number of entries that are allowed in a stack (default=16)
//

#include <functional>
using namespace std::placeholders;

#include "HardwareCPU_Base.h"
#include "Instruction.h"
#include "InstLib.h"
#include "../tools/assert.h"

namespace emp {

  template <int CPU_SCALE=8, int STACK_SIZE=16> class HardwareCPU
    : public HardwareCPU_Base<Instruction> {
  protected:
    // Hardware components...
    typedef std::vector<emp::Instruction> MEMORY_TYPE;
    typedef HardwareCPU<CPU_SCALE, STACK_SIZE> HARDWARE_TYPE;
    MEMORY_TYPE memory[CPU_SCALE];
    CPUStack<STACK_SIZE> stacks[CPU_SCALE];
    CPUHead heads[CPU_SCALE];

    const InstLib<HardwareCPU, Instruction> & inst_lib;

  public:
    // Track the default positions of various heads.
    static const int HEAD_IP    = 0;
    static const int HEAD_READ  = 1;
    static const int HEAD_WRITE = 2;  // Not used yet?
    static const int HEAD_FLOW  = 3;
    
    // Track the contents of stacks.
    static const int STACK_BIO          = 0;  // Not used yet?
    static const int STACK_IN1          = 1;
    static const int STACK_IN2          = 2;
    static const int STACK_OUT          = 1;  // Same as IN1 for now.
    static const int STACK_TEST_RESULTS = 3;

    HardwareCPU(const InstLib<HardwareCPU, Instruction> & _inst_lib) : inst_lib(_inst_lib) {
      emp_assert(CPU_SCALE >= 4 && "Minimum 4 heads needed");
      // Initialize all of the heads to the beginning of the code.
      for (int i=0; i < CPU_SCALE; i++) heads[i].Set(memory[0], 0);
    }
    ~HardwareCPU() { ; }

    // Do a full factory-reset on the virtual hardware.
    void Clear() {
      for (int i = 0; i < CPU_SCALE; i++) {
        stacks[i].Clear();
        heads[i].Set(memory[i], 0);
        memory[i].resize(0);
      }
    }

    CPUStack<STACK_SIZE> & GetStack(int stack_id) {
      emp_assert(stack_id >= 0 && stack_id < CPU_SCALE);
      return stacks[stack_id];
    }

    static int GetNumStacks()  { return CPU_SCALE; }
    static int GetStackSize()  { return STACK_SIZE; }
    static int GetNumArgNops() { return CPU_SCALE; }

    MEMORY_TYPE & GetMemory(int mem_id=0) { return memory[mem_id]; }
    
    void LoadMemory(const std::vector<emp::Instruction> & in_memory) { memory[0] = in_memory; }

    // Examines the nops following the IP to test if they override the default arguments.
    int ChooseTarget(int default_target) {
      const int arg_value = heads[HEAD_IP].GetInst().GetArgValue();
      if (arg_value) {
        ++heads[HEAD_IP];
        return arg_value - 1;
      }

      return default_target;
    }


    // The following function drives the execution of the virtual hardware -- it executes the
    // next instruction pointed to by the IP.
    void SingleProcess() {
      emp_assert(heads[HEAD_IP].IsValid());

      const Instruction & inst = heads[HEAD_IP].GetInst();
      ++heads[HEAD_IP];
      inst_lib.RunInst(*this, inst.GetID());
    }




    // ===============  Instruction Definitions  ===============
    //
    // NOTE: Any arguments need to be bound during instruction library construction.

    bool Inst_Nop() { return true; }

    // -------- Generic Single-argument Math Instructions --------

    // Build a 1-input math instruction on the fly.  See two-input math for examples.
    template <int default_in=STACK_IN1, int default_out_offset=0, bool pop_input=true>
    bool Inst_1I_Math(std::function<int(int)> math1_fun) {
      const int in_stack = ChooseTarget(default_in);
      const int out_stack = ChooseTarget((in_stack + default_out_offset) % CPU_SCALE);
      const int in_value = pop_input ? stacks[in_stack].Pop() : stacks[in_stack].Top();
      const int result = math1_fun(in_value);
      stacks[out_stack].Push(result);
      return true;
    }

    static std::function<bool(HARDWARE_TYPE &)> BuildMathInst(const std::function<int(int)> & math_fun)
    {
      return [math_fun](HARDWARE_TYPE & hw){ return hw.Inst_1I_Math(math_fun); };
    }
    

    // -------- Generic Two-argument Math Instructions --------

    template <int default_in1=STACK_IN1, int default_in2_offset=1, int default_out=STACK_OUT,
              bool pop_input1=false, bool pop_input2=false>
    bool Inst_2I_Math(std::function<int(int,int)> math2_fun) {
      const int in1_stack = ChooseTarget(default_in1);
      const int in2_stack = ChooseTarget((in1_stack + default_in2_offset) % CPU_SCALE);
      const int out_stack = ChooseTarget(default_out);
      const int in_value1 = pop_input1 ? stacks[in1_stack].Pop() : stacks[in1_stack].Top();
      const int in_value2 = pop_input2 ? stacks[in2_stack].Pop() : stacks[in2_stack].Top();
      const int result = math2_fun(in_value1, in_value2);
      stacks[out_stack].Push(result);
      return true;
    }

    static std::function<bool(HARDWARE_TYPE&)>
    BuildMathInst(const std::function<int(int,int)> & math_fun)
    {
      return [math_fun](HARDWARE_TYPE & hw){ return hw.Inst_2I_Math(math_fun); };
    }

    static std::function<bool(HARDWARE_TYPE&)>
    BuildTestInst(const std::function<int(int,int)> & test_fun)
    {
      return [test_fun](HARDWARE_TYPE & hw){ return hw.template Inst_2I_Math<1,1,3>(test_fun); };
    }


    
    // --------  Generic Jump Operations  --------

    template <int default_head_to_move, int default_head_target> bool Inst_MoveHeadToHead() {
      const int head_move = ChooseTarget(default_head_to_move);
      const int head_target = ChooseTarget(default_head_target);
      heads[head_move] = heads[head_target];
      return true;
    }

    template <int default_head_to_move, int default_head_target, int default_stack_test>
    bool Inst_MoveHeadToHeadIf(std::function<bool(int)> test_fun) {
      const int head_move = ChooseTarget(default_head_to_move);
      const int head_target = ChooseTarget(default_head_target);
      const int stack_test = ChooseTarget(default_stack_test);
      const bool test_result = test_fun( stacks[stack_test].Pop() );
      if (test_result) heads[head_move] = heads[head_target];
      return true;
    }

    template <int default_head_to_move, int default_mem_target> bool Inst_MoveHeadToMem() {
      const int head_move = ChooseTarget(default_head_to_move);
      const int mem_target = ChooseTarget(default_mem_target);
      if (memory[mem_target].size() == 0) return false; // Memory spaces must exist before jump.
      heads[head_move].Set(memory[mem_target], 0);
      return true;
    }


    // --------  Instruction-specific Operations ---------

    // Test if a head (default: read-head) is at the start of its current memory space (save default=7)
    bool Inst_TestAtStart() {
      const int head_test = ChooseTarget(HEAD_READ);
      const int out_stack = ChooseTarget(STACK_TEST_RESULTS);
      stacks[out_stack].Push(heads[head_test].GetPosition() == 0);
      return true;
    }

    // Delete the top of a stack and discard it.
    bool Inst_ValDelete() {
      const int delete_stack = ChooseTarget(STACK_OUT);
      stacks[delete_stack].Pop();
      return true;
    }
    
    bool Inst_BuildInst() {
      const int mem_target = ChooseTarget(1);
      const int head_from = ChooseTarget(HEAD_READ);
      memory[mem_target].push_back(heads[head_from].GetInst());
      ++heads[head_from];  // Advance the head that was read from.
      return true;
    }


    // The following function generates a map of known instruction names to their definitions.

    static const std::map<std::string, InstDefinition<HARDWARE_TYPE> > & GetInstDefs()
    {
      // This function will produce a unique defs map.  If we already have it, just return it.
      static std::map<std::string, InstDefinition<HARDWARE_TYPE> > defs;
      if (defs.size()) return defs;

      defs["Nop"]        = { "No-operation instruction; usable as modifier.",
                             [](HARDWARE_TYPE & hw){return hw.template Inst_Nop(); } };
      
      // Add single-argument math operations.
      defs["Inc"]        = { "Increment top of ?Stack-B? by one",
                             HARDWARE_TYPE::BuildMathInst([](int a){return a+1;}) };
      
      defs["Dec"]        = { "Decrement top of ?Stack-B? by one",
                             HARDWARE_TYPE::BuildMathInst([](int a){return a-1;}) };
      
      defs["Shift-L"]    = { "Shift bits of top of ?Stack-B? left by one",
                             HARDWARE_TYPE::BuildMathInst([](int a){return a<<1;}) };
      
      defs["Shift-R"]    = { "Shift bits of top of ?Stack-B? right by one",
                             HARDWARE_TYPE::BuildMathInst([](int a){return a>>1;}) };
      
      // Add double-argument math operations.
      defs["Nand"]       = { "Compute: ?Stack-B?-top nand ?Stack-C?-top and push result to ?Stack-B?",
                             HARDWARE_TYPE::BuildMathInst([](int a, int b){ return ~(a&b); }) };
      
      defs["Add"]        = { "Compute: ?Stack-B?-top plus ?Stack-C?-top and push result to ?Stack-B?",
                             HARDWARE_TYPE::BuildMathInst([](int a, int b){ return a+b; }) };
      
      defs["Sub"]        = { "Compute: ?Stack-B?-top minus ?Stack-C?-top and push result to ?Stack-B?",
                             HARDWARE_TYPE::BuildMathInst([](int a, int b){ return a-b; }) };
      
      defs["Mult"]       = { "Compute: ?Stack-B?-top times ?Stack-C?-top and push result to ?Stack-B?",
                             HARDWARE_TYPE::BuildMathInst([](int a, int b){ return a*b; }) };
      
      // @CAO For the next two, ideally if b==0, we should have the instruction return false...
      defs["Div"]        = { "Compute: ?Stack-B?-top div ?Stack-C?-top and push result to ?Stack-B?",
                             HARDWARE_TYPE::BuildMathInst([](int a, int b){ return b?a/b:0; }) };
      
      defs["Mod"]        = { "Compute: ?Stack-B?-top mod ?Stack-C?-top and push result to ?Stack-B?",
                             HARDWARE_TYPE::BuildMathInst([](int a, int b){ return b?a%b:0; }) };
      
      // Conditionals
      defs["Test-Equal"] = { "Test if ?Stack-B?-top == ?Stack-C?-top and push result to ?Stack-D?",
                             HARDWARE_TYPE::BuildTestInst([](int a, int b){ return a==b; }) };
      
      defs["Test-NEqual"] = { "Test if ?Stack-B?-top != ?Stack-C?-top and push result to ?Stack-D?",
                              HARDWARE_TYPE::BuildTestInst([](int a, int b){ return a!=b; }) };
      
      defs["Test-Less"]  = { "Test if ?Stack-B?-top < ?Stack-C?-top and push result to ?Stack-D?",
                             HARDWARE_TYPE::BuildTestInst([](int a, int b){ return a<b; }) };
      
      defs["Test-AtStart"] = { "Test if ?Head-Read? is at mem position 0 and push result to ?Stack-D?",
                               std::mem_fn(&HARDWARE_TYPE::Inst_TestAtStart) };
      
      // Load in Jump operations  [we neeed to do better...  push and pop heads?]
      defs["Jump"]       = { "Move ?Head-IP? to position of ?Head-Flow?",
                             std::mem_fn(&HARDWARE_TYPE::template Inst_MoveHeadToHead<0, 3>) };
      defs["Jump-If0"]   = { "Move ?Head-IP? to position of ?Head-Flow? only if ?Stack-D?-top == 0",
                             [](HARDWARE_TYPE & hw){return hw.template Inst_MoveHeadToHeadIf<0,3,3>([](int a){ return a==0; }); } };
      defs["Jump-IfN0"]  = { "Move ?Head-IP? to position of ?Head-Flow? only if ?Stack-D?-top != 0",
                             [](HARDWARE_TYPE & hw){return hw.template Inst_MoveHeadToHeadIf<0,3,3>([](int a){ return a!= 0; }); } };
      defs["Bookmark"]   = { "Move ?Head-Flow? to position of ?Head-IP?",
                             std::mem_fn(&HARDWARE_TYPE::template Inst_MoveHeadToHead<3, 0>) };
      defs["Set-Memory"] = { "Move ?Head-Write? to position 0 in ?Memory-1?",
                             std::mem_fn(&HARDWARE_TYPE::template Inst_MoveHeadToMem<2, 1>) };
      // "Find-Label" ********** - Jumps the flow head to a complement label (?...) in current memory.
      
      // Juggle stack contents
      defs["Val-Move"]   = { "Pop ?Stack-B? and push value onto ?Stack-C?",
                             [](HARDWARE_TYPE & hw){return hw.template Inst_1I_Math<1,1>([](int a){ return a; }); } };
      defs["Val-Copy"]   = { "Copy top of ?Stack-B? onto ?Stack-C?",
                             [](HARDWARE_TYPE & hw){return hw.template Inst_1I_Math<1,1,false>([](int a){ return a; }); } };
      defs["Val-Delete"] = { "Pop ?Stack-B? and discard value",
                             std::mem_fn(&HARDWARE_TYPE::Inst_ValDelete) };
      
      // Check for "Biological" instructions
      defs["Build-Inst"] = { "Add new instruction to end of ?Memory-1? copied from ?Head-Read?",
                             std::mem_fn(&HARDWARE_TYPE::Inst_BuildInst) };
      // "Divide" **********      - Moves memory space 1 (?1) into its own hardware.  Needs callback!
      // "Get-Input" **********   - Needs callback
      // "Get-Output" **********  - Needs callback
      // "Inject" ?? **********   - Needs callback
      
      return defs;
    }


    // The following function returns a list of default instruction names.

    static const std::vector<std::string> & GetDefaultInstructions()
    {
      // If we've already generated this list, just return it.
      static std::vector<std::string> default_insts;
      if (default_insts.size() > 0) return default_insts;

      // Include as many nops as we need.  This we be called Nop:0, Nop:1, Nop:2, etc.
      for (int i = 0; i < CPU_SCALE; i++) {
        std::stringstream ss;
        ss << "Nop mod_id=" << i << " name=Nop-" << i;
        default_insts.push_back(ss.str());
      }

      // Load in single-argument math operations.
      default_insts.push_back("Inc");
      default_insts.push_back("Dec");
      default_insts.push_back("Shift-L");
      default_insts.push_back("Shift-R");

      // Load in double-argument math operations.
      default_insts.push_back("Nand");
      default_insts.push_back("Add");
      default_insts.push_back("Sub");
      default_insts.push_back("Mult");

      // @CAO For the next two, ideally if b==0, we should have the instruction return false...
      default_insts.push_back("Div");
      default_insts.push_back("Mod");

      // Conditionals
      default_insts.push_back("Test-Equal");
      default_insts.push_back("Test-NEqual");
      default_insts.push_back("Test-Less");
      default_insts.push_back("Test-AtStart");

      // Load in Jump operations  [we neeed to do better...  push and pop heads?]
      default_insts.push_back("Jump");
      default_insts.push_back("Jump-If0");
      default_insts.push_back("Jump-IfN0");
      default_insts.push_back("Bookmark");
      default_insts.push_back("Set-Memory");
      // "Find-Label" ********** - Jumps the flow head to a complement label (?...) in its current memory.    

      // Juggle stack contents
      default_insts.push_back("Val-Move");
      default_insts.push_back("Val-Copy");
      default_insts.push_back("Val-Delete");

      // Load in "Biological" instructions
      default_insts.push_back("Build-Inst");
      // "Divide" **********      - Moves memory space 1 (?1) into its own hardware.  Needs callback!
      // "Get-Input" **********   - Needs callback
      // "Get-Output" **********  - Needs callback

      return default_insts;
    }

  };

};

#endif

