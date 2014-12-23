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
//   MAX_ARGS = Maximum number of arguments needed by any instruction.
//

#include "HardwareCPU_Base.h"

namespace emp {

  template <int NUM_STACKS=8, int STACK_SIZE=16, int NUM_ARG_NOPS=8, int MAX_ARGS=3> class HardwareCPU
    : public HardwareCPU_Base<Instruction> {
  protected:
    // Hardware components...
    CPUStack<STACK_SIZE> stacks[NUM_STACKS];
  public:
    HardwareCPU() { ; }
    ~HardwareCPU() { ; }

    // Instruction Definitions
    // NOTE: Any arguments need to be bound during instruction library construction.
    bool Inst_Nop() { return true; }

    // -------- Single-argument Math Instructions --------

    bool Inst_ShiftL(const int shift, const int default_in, const int default_out) {
      const int in_stack = ChooseStack(default_in);
      const int out_stack = ChooseStack(default_out);
      const int result = (shift > 0) ? stacks[in_stack].Top() << shift : stacks[in_stack].Top() >> -shift;
      stacks[out_stack].Push(result);
      return true;
    }

    // -------- Two-argument Math Instructions --------

    template <std::function<int(int,int)> MATH2_FUN>
    bool Inst_2I_Math(const int default_in1, const int default_in2, const int default_out) {
      const int in1_stack = ChooseStack(default_in1);
      const int in2_stack = ChooseStack(default_in2);
      const int out_stack = ChooseStack(default_out);
      const int result = math2_fun(stacks[in1_stack].Top(), stacks[in2_stack].Top());
      stacks[out_stack].Push(result);
      return true;
    }

    bool Inst_Nand(const int default_in1, const int default_in2, const int default_out) {
      return Inst_2I_Math<[](int a, int b) { return ~(a&b); }>(default_in1, default_in2, default_out);
    }
    bool Inst_Add(const int default_in1, const int default_in2, const int default_out) {
      return Inst_2I_Math<[](int a, int b) { return a+b; }>(default_in1, default_in2, default_out);
    }
    bool Inst_Sub(const int default_in1, const int default_in2, const int default_out) {
      return Inst_2I_Math<[](int a, int b) { return a-b; }>(default_in1, default_in2, default_out);
    }
    bool Inst_Mult(const int default_in1, const int default_in2, const int default_out) {
      return Inst_2I_Math<[](int a, int b) { return a*b; }>(default_in1, default_in2, default_out);
    }
    bool Inst_Div(const int default_in1, const int default_in2, const int default_out) {
      if (b == 0) return false;
      return Inst_2I_Math<[](int a, int b) { return a/b; }>(default_in1, default_in2, default_out);
    }
    bool Inst_Mod(const int default_in1, const int default_in2, const int default_out) {
      if (b == 0) return false;
      return Inst_2I_Math<[](int a, int b) { return a%b; }>(default_in1, default_in2, default_out);
    }

  };

};

#endif

