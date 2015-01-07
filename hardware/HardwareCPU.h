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

#include "HardwareCPU_Base.h"
#include "Instruction.h"
#include "InstLib.h"
#include "../tools/assert.h"

namespace emp {

  template <int CPU_SCALE=8, int STACK_SIZE=16> class HardwareCPU
    : public HardwareCPU_Base<Instruction> {
  protected:
    // Hardware components...
    typedef std::vector<emp::Instruction> mem_type;
    typedef HardwareCPU<CPU_SCALE, STACK_SIZE> HARDWARE_TYPE;
    mem_type memory[CPU_SCALE];
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

    mem_type & GetMemory(int mem_id=0) { return memory[mem_id]; }
    
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

  };

};

#endif

