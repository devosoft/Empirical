#ifndef EMP_HARDWARE_CPU_H
#define EMP_HARDWARE_CPU_H

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  HardwareCPU is a basic, CPU-style virtual hardware object.
//
//  This is a templated type that allows compile-time configuration of properties.
//   NUM_STACKS = how many stacks are available in the hardware?  (default = 8)
//   STACK_SIZE = maximum number of entries that are allowed in a stack (default=16)
//   NUM_ARG_NOPS = How many nop instructions can be used as arguments? (typically = NUM_STACKS)
//

#include <functional>

#include "HardwareCPU_Base.h"
#include "Instruction.h"
#include "InstLib.h"

namespace emp {

  template <int NUM_STACKS=8, int STACK_SIZE=16, int NUM_ARG_NOPS=8> class HardwareCPU
    : public HardwareCPU_Base<Instruction> {
  protected:
    // Hardware components...
    typedef std::vector<emp::Instruction> mem_type;
    mem_type memory[NUM_STACKS];
    CPUStack<STACK_SIZE> stacks[NUM_STACKS];
    CPUHead heads[NUM_STACKS];

    const InstLib<HardwareCPU, Instruction> & inst_lib;

    // Track the default positions of various heads.
    static const int HEAD_IP    = 0;
    static const int HEAD_READ  = 1;
    static const int HEAD_WRITE = 2;
    static const int HEAD_FLOW  = 3;
  public:
    HardwareCPU(const InstLib<HardwareCPU, Instruction> & _inst_lib) : inst_lib(_inst_lib) {
      assert(NUM_STACKS >= 4 && "Minimum 4 heads needed");
      // Initialize all of the heads to the beginning of the code.
      for (int i=0; i < NUM_STACKS; i++) heads[i].Set(memory[0], 0);
    }
    ~HardwareCPU() { ; }

    // Do a full factory-reset on the virtual hardware.
    void Clear() {
      for (int i = 0; i < NUM_STACKS; i++) {
        stacks[i].Clear();
        heads[i].Set(memory[i], 0);
        memory[i].resize(0);
      }
    }

    CPUStack<STACK_SIZE> & GetStack(int stack_id) {
      assert(stack_id >= 0 && stack_id < NUM_STACKS);
      return stacks[stack_id];
    }

    static int GetNumStacks()  { return NUM_STACKS; }
    static int GetStackSize()  { return STACK_SIZE; }
    static int GetNumArgNops() { return NUM_ARG_NOPS; }

    mem_type & GetMemory(int mem_id=0) { return memory[mem_id]; }
    
    void LoadMemory(const std::vector<emp::Instruction> & in_memory) { memory[0] = in_memory; }

    // Examines the nops following (?) the IP to test if they override the default arguments.
    int ChooseStack(int default_stack) {
      const int arg_value = heads[HEAD_IP].GetInst().GetArgValue();
      if (arg_value) {
        ++heads[HEAD_IP];
        return arg_value - 1;
      }

      return default_stack;
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
    bool Inst_1I_Math(std::function<int(int)> math1_fun, const int default_in,
                      const int default_out_offset) {
      const int in_stack = ChooseStack(default_in);
      const int out_stack = ChooseStack((in_stack + default_out_offset) % NUM_ARG_NOPS);
      const int result = math1_fun(stacks[in_stack].Pop());
      stacks[out_stack].Push(result);
      return true;
    }

    // Add or subtract a value; use +1 and -1 for Inc and Dec instructions.
    bool Inst_AddConst(const int value, const int default_in, const int default_out_offset) {
      const int in_stack = ChooseStack(default_in);
      const int out_stack = ChooseStack((in_stack + default_out_offset) % NUM_ARG_NOPS);
      const int result = stacks[in_stack].Pop() + value;
      stacks[out_stack].Push(result);
      return true;
    }

    // Positive shift is left, negative shift is right.  I.e., value = value * 2^shift
    bool Inst_Shift(const int shift, const int default_in, const int default_out_offset) {
      const int in_stack = ChooseStack(default_in);
      const int out_stack = ChooseStack((in_stack + default_out_offset) % NUM_ARG_NOPS);
      const int result = (shift > 0) ? stacks[in_stack].Pop() << shift : stacks[in_stack].Pop() >> -shift;
      stacks[out_stack].Push(result);
      return true;
    }

    // -------- Two-argument Math Instructions --------

    bool Inst_2I_Math(std::function<int(int,int)> math2_fun, const int default_in1,
                      const int default_in2_offset, const int default_out) {
      const int out_stack = ChooseStack(default_out);
      const int in1_stack = ChooseStack(default_in1);
      const int in2_stack = ChooseStack((in1_stack + default_in2_offset) % NUM_ARG_NOPS);
      const int result = math2_fun(stacks[in1_stack].Top(), stacks[in2_stack].Top());
      stacks[out_stack].Push(result);
      return true;
    }

    bool Inst_Nand(const int d_in1, const int d_in2_offset, const int d_out) {
      return Inst_2I_Math([](int a, int b) { return ~(a&b); }, d_in1, d_in2_offset, d_out);
    }
    bool Inst_Add(const int d_in1, const int d_in2_offset, const int d_out) {
      return Inst_2I_Math([](int a, int b) { return a+b; }, d_in1, d_in2_offset, d_out);
    }
    bool Inst_Sub(const int d_in1, const int d_in2_offset, const int d_out) {
      return Inst_2I_Math([](int a, int b) { return a-b; }, d_in1, d_in2_offset, d_out);
    }
    bool Inst_Mult(const int d_in1, const int d_in2_offset, const int d_out) {
      return Inst_2I_Math([](int a, int b) { return a*b; }, d_in1, d_in2_offset, d_out);
    }
    bool Inst_Div(const int d_in1, const int d_in2_offset, const int d_out) {
      // @CAO Ideally if b==0, we should return false...
      return Inst_2I_Math([](int a, int b) { return (b==0)?0:a/b; }, d_in1, d_in2_offset, d_out);
    }
    bool Inst_Mod(const int d_in1, const int d_in2_offset, const int d_out) {
      // @CAO Ideally if b==0, we should return false...
      return Inst_2I_Math([](int a, int b) { return (b==0)?0:a%b; }, d_in1, d_in2_offset, d_out);
    }

  };

};

#endif

