//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  HardwareCPU_TubeCode is a basic virtual CPU intended to represent and idealized Hardware
//  for use in classroom projects.  It was originally developed for "CSE 450: Translation of
//  Programming Languages" at Michigan State University.


#ifndef EMP_HARDWARE_CPU_TUBECODE_H
#define EMP_HARDWARE_CPU_TUBECODE_H

#include <functional>
#include <map>
using namespace std::placeholders;

#include "../base/assert.h"
#include "../base/vector.h"

#include "HardwareCPU_Base.h"
#include "Instruction_TubeCode.h"
#include "InstLib.h"

namespace emp {

  template <int NUM_REGS=8, int MEM_SIZE=65536, typename VAL_T=double>
  class HardwareCPU_TubeCode
    : public HardwareCPU_Base<Instruction_TubeCode> {
  protected:
    // Hardware components...
    using code_t = emp::vector<emp::Instruction_TubeCode>;
    using hardware_t = HardwareCPU_TubeCode<NUM_REGS, MEM_SIZE>;

    code_t code;
    VAL_T memory[MEM_SIZE];                         // Memory!
    VAL_T regs[NUM_REGS];                           // Registers!
    std::map<int, VAL_T> var_map;                   // Variables!
    std::map<int, emp::vector<VAL_T> > array_map;   // Arrays!
    CPUHead IP;                                     // Instruction Pointer!

    const InstLib<HardwareCPU_TubeCode, Instruction_TubeCode> & inst_lib;

  public:
    HardwareCPU_TubeCode(const InstLib<HardwareCPU_TubeCode, Instruction_TubeCode> & _inst_lib)
      : inst_lib(_inst_lib)
    {
      // Initialize all registers and memory.
      for (VAL_T & x : memory) x = 0;
      for (VAL_T & x : regs) x = 0;
      IP.Set(memory, 0);
    }
    HardwareCPU_TubeCode(const HardwareCPU_TubeCode & prototype)
      : HardwareCPU_TubeCode(prototype.inst_lib) { ; }
    ~HardwareCPU_TubeCode() { ; }

    // Do a full factory-reset on the virtual hardware.
    void Clear() {
      // Initialize all registers and memory.
      code.resize(0);
      for (VAL_T & x : memory) x = 0;
      for (VAL_T & x : regs) x = 0;
      var_map.clear();
      array_map.clear();
      IP.Set(memory, 0);
    }

    int GetCodeSize() const { return (int) code.size(); }
    constexpr int GetMemSize() const { return MEM_SIZE; }
    constexpr int GetNumRegs() const { return NUM_REGS; }
    int GetNumVars() const { return (int) var_map.size(); }
    int GetNumArrays() const { return (int) array_map.size(); }

    void LoadCode(const emp::vector<emp::Instruction_TubeCode> & in_code) { code = in_code; }


    // The following function drives the execution of the virtual hardware -- it executes the
    // next instruction pointed to by the IP.
    void SingleProcess() {
      emp_assert(IP.IsValid());

      const Instruction_TubeCode & inst = IP.GetInst();
      ++IP;
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

    static std::function<bool(hardware_t &)> BuildMathInst(const std::function<int(int)> & math_fun)
    {
      return [math_fun](hardware_t & hw){ return hw.Inst_1I_Math(math_fun); };
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

    static std::function<bool(hardware_t&)>
    BuildMathInst(const std::function<int(int,int)> & math_fun) {
      return [math_fun](hardware_t & hw){ return hw.Inst_2I_Math(math_fun); };
    }

    static std::function<bool(hardware_t&)>
    BuildTestInst(const std::function<int(int,int)> & test_fun) {
      return [test_fun](hardware_t & hw){ return hw.template Inst_2I_Math<1,1,3>(test_fun); };
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

    template <int default_head_to_move, int default_mem_target>
    bool Inst_MoveHeadToMem() {
      const int head_move = ChooseTarget(default_head_to_move);
      const int mem_target = ChooseTarget(default_mem_target);
      if (memory[mem_target].size() == 0) return false; // Memory spaces must exist before jump.
      heads[head_move].Set(memory[mem_target], 0);
      return true;
    }


    // --------  Other Generic operations ---------

    template <int default_stack=STACK_IN1> bool Inst_PushConst(int value) {
      const int stack_used = ChooseTarget(default_stack);
      stacks[stack_used].Push(value);
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

    static const std::map<std::string, InstDefinition<hardware_t> > & GetInstDefs()
    {
      // This function will produce a unique defs map.  If we already have it, just return it.
      static std::map<std::string, InstDefinition<hardware_t> > defs;
      if (defs.size()) return defs;

      defs["Nop"]        = { "No-operation instruction; usable as modifier.",
                             [](hardware_t & hw){ return hw.Inst_Nop(); } };
      
      // Add single-argument math operations.
      defs["Inc"]        = { "Increment top of ?Stack-B? by one",
                             hardware_t::BuildMathInst([](int a){ return a+1;}) };
      
      defs["Dec"]        = { "Decrement top of ?Stack-B? by one",
                             hardware_t::BuildMathInst([](int a){ return a-1;}) };
      
      defs["Shift-L"]    = { "Shift bits of top of ?Stack-B? left by one",
                             hardware_t::BuildMathInst([](int a){ return a<<1;}) };
      
      defs["Shift-R"]    = { "Shift bits of top of ?Stack-B? right by one",
                             hardware_t::BuildMathInst([](int a){ return a>>1;}) };
      
      // Add double-argument math operations.
      defs["Nand"]       = { "Compute: ?Stack-B?-top nand ?Stack-C?-top and push result to ?Stack-B?",
                             hardware_t::BuildMathInst([](int a, int b){ return ~(a&b); }) };
      
      defs["Add"]        = { "Compute: ?Stack-B?-top plus ?Stack-C?-top and push result to ?Stack-B?",
                             hardware_t::BuildMathInst([](int a, int b){ return a+b; }) };
      
      defs["Sub"]        = { "Compute: ?Stack-B?-top minus ?Stack-C?-top and push result to ?Stack-B?",
                             hardware_t::BuildMathInst([](int a, int b){ return a-b; }) };
      
      defs["Mult"]       = { "Compute: ?Stack-B?-top times ?Stack-C?-top and push result to ?Stack-B?",
                             hardware_t::BuildMathInst([](int a, int b){ return a*b; }) };
      
      // @CAO For the next two, ideally if b==0, we should have the instruction return false...
      defs["Div"]        = { "Compute: ?Stack-B?-top div ?Stack-C?-top and push result to ?Stack-B?",
                             hardware_t::BuildMathInst([](int a, int b){ return b?a/b:0; }) };
      
      defs["Mod"]        = { "Compute: ?Stack-B?-top mod ?Stack-C?-top and push result to ?Stack-B?",
                             hardware_t::BuildMathInst([](int a, int b){ return b?a%b:0; }) };
      
      // Conditionals
      defs["Test-Equal"] = { "Test if ?Stack-B?-top == ?Stack-C?-top and push result to ?Stack-D?",
                             hardware_t::BuildTestInst([](int a, int b){ return a==b; }) };
      
      defs["Test-NEqual"] = { "Test if ?Stack-B?-top != ?Stack-C?-top and push result to ?Stack-D?",
                              hardware_t::BuildTestInst([](int a, int b){ return a!=b; }) };
      
      defs["Test-Less"]  = { "Test if ?Stack-B?-top < ?Stack-C?-top and push result to ?Stack-D?",
                             hardware_t::BuildTestInst([](int a, int b){ return a<b; }) };
      
      defs["Test-AtStart"] = { "Test if ?Head-Read? is at mem position 0 and push result to ?Stack-D?",
                               std::mem_fn(&hardware_t::Inst_TestAtStart) };
      
      // Load in Jump operations  [we neeed to do better...  push and pop heads?]
      defs["Jump"]       = { "Move ?Head-IP? to position of ?Head-Flow?",
                             std::mem_fn(&hardware_t::template Inst_MoveHeadToHead<0, 3>) };
      defs["Jump-If0"]   = { "Move ?Head-IP? to position of ?Head-Flow? only if ?Stack-D?-top == 0",
                             [](hardware_t & hw){return hw.template Inst_MoveHeadToHeadIf<0,3,3>([](int a){ return a==0; }); } };
      defs["Jump-IfN0"]  = { "Move ?Head-IP? to position of ?Head-Flow? only if ?Stack-D?-top != 0",
                             [](hardware_t & hw){return hw.template Inst_MoveHeadToHeadIf<0,3,3>([](int a){ return a!= 0; }); } };
      defs["Bookmark"]   = { "Move ?Head-Flow? to position of ?Head-IP?",
                             std::mem_fn(&hardware_t::template Inst_MoveHeadToHead<3, 0>) };
      defs["Set-Memory"] = { "Move ?Head-Write? to position 0 in ?Memory-1?",
                             std::mem_fn(&hardware_t::template Inst_MoveHeadToMem<2, 1>) };
      // "Find-Label" ********** - Jumps the flow head to a complement label (?...) in current memory.
      
      // Juggle stack contents
      defs["Val-Move"]   = { "Pop ?Stack-B? and push value onto ?Stack-C?",
                             [](hardware_t & hw){return hw.template Inst_1I_Math<1,1>([](int a){ return a; }); } };
      defs["Val-Copy"]   = { "Copy top of ?Stack-B? onto ?Stack-C?",
                             [](hardware_t & hw){return hw.template Inst_1I_Math<1,1,false>([](int a){ return a; }); } };
      defs["Val-Delete"] = { "Pop ?Stack-B? and discard value",
                             std::mem_fn(&hardware_t::Inst_ValDelete) };
      defs["PushConst"]  = { "Push a specified value onto ?Stack-B?",
                             [](hardware_t & hw, int value){ return hw.Inst_PushConst(value); } };
      
      // Check for "Biological" instructions
      defs["Build-Inst"] = { "Add new instruction to end of ?Memory-1? copied from ?Head-Read?",
                             std::mem_fn(&hardware_t::Inst_BuildInst) };
      // "Divide" **********      - Moves memory space 1 (?1) into its own hardware.  Needs callback!
      // "Get-Input" **********   - Needs callback
      // "Get-Output" **********  - Needs callback
      // "Inject" ?? **********   - Needs callback
      
      return defs;
    }


    // The following function returns a list of default instruction names.

    static const emp::vector<std::string> & GetDefaultInstructions()
    {
      // If we've already generated this list, just return it.
      static emp::vector<std::string> default_insts;
      if (default_insts.size() > 0) return default_insts;

      // Include as many nops as we need.  These will be called Nop-0, Nop-1, Nop-2, etc.
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
      default_insts.push_back("Div");
      default_insts.push_back("Mod");

      // Constants
      default_insts.push_back("PushConst:0 name=Push-Zero");

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

