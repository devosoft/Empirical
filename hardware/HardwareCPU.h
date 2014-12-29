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

namespace emp {

  template <int CPU_SCALE=8, int STACK_SIZE=16> class HardwareCPU
    : public HardwareCPU_Base<Instruction> {
  protected:
    // Hardware components...
    typedef std::vector<emp::Instruction> mem_type;
    mem_type memory[CPU_SCALE];
    CPUStack<STACK_SIZE> stacks[CPU_SCALE];
    CPUHead heads[CPU_SCALE];

    const InstLib<HardwareCPU, Instruction> & inst_lib;

  public:
    // Track the default positions of various heads.
    static const int HEAD_IP    = 0;
    static const int HEAD_READ  = 1;
    static const int HEAD_WRITE = 2;
    static const int HEAD_FLOW  = 3;

    HardwareCPU(const InstLib<HardwareCPU, Instruction> & _inst_lib) : inst_lib(_inst_lib) {
      assert(CPU_SCALE >= 4 && "Minimum 4 heads needed");
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
      assert(stack_id >= 0 && stack_id < CPU_SCALE);
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
      assert(heads[HEAD_IP].IsValid());

      const Instruction & inst = heads[HEAD_IP].GetInst();
      ++heads[HEAD_IP];
      inst_lib.RunInst(*this, inst.GetID());
    }




    // ===============  Instruction Definitions  ===============
    //
    // NOTE: Any arguments need to be bound during instruction library construction.

    bool Inst_Nop() { return true; }

    // -------- Single-argument Math Instructions --------

    // Build a 1-input math instruction on the fly.  See two-input math for examples.
    template <int default_in, int default_out_offset>
    bool Inst_1I_Math(std::function<int(int)> math1_fun) {
      const int in_stack = ChooseTarget(default_in);
      const int out_stack = ChooseTarget((in_stack + default_out_offset) % CPU_SCALE);
      const int result = math1_fun(stacks[in_stack].Pop());
      stacks[out_stack].Push(result);
      return true;
    }

    // Add or subtract a value; use +1 and -1 for Inc and Dec instructions.
    template <int value, int default_in, int default_out_offset> bool Inst_AddConst() {
      const int in_stack = ChooseTarget(default_in);
      const int out_stack = ChooseTarget((in_stack + default_out_offset) % CPU_SCALE);
      const int result = stacks[in_stack].Pop() + value;
      stacks[out_stack].Push(result);
      return true;
    }

    // Positive shift is left, negative shift is right.  I.e., value = value * 2^shift
    template <int shift, int default_in, int default_out_offset> bool Inst_Shift() {
      const int in_stack = ChooseTarget(default_in);
      const int out_stack = ChooseTarget((in_stack + default_out_offset) % CPU_SCALE);
      const int result = (shift > 0) ? stacks[in_stack].Pop() << shift : stacks[in_stack].Pop() >> -shift;
      stacks[out_stack].Push(result);
      return true;
    }

    // -------- Two-argument Math Instructions --------

    template <int default_in1, int default_in2_offset, int default_out>
    bool Inst_2I_Math(std::function<int(int,int)> math2_fun) {
      const int out_stack = ChooseTarget(default_out);
      const int in1_stack = ChooseTarget(default_in1);
      const int in2_stack = ChooseTarget((in1_stack + default_in2_offset) % CPU_SCALE);
      const int result = math2_fun(stacks[in1_stack].Top(), stacks[in2_stack].Top());
      stacks[out_stack].Push(result);
      return true;
    }

    
    // --------  Jump Operations  --------

    template <int default_head_to_move, int default_head_target> bool Inst_MoveHeadToHead() {
      const int head_move = ChooseTarget(default_head_to_move);
      const int head_target = ChooseTarget(default_head_target);
      heads[head_move] = heads[head_target];
    }
  };

};

#endif

