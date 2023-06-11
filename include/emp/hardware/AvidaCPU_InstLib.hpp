/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file AvidaCPU_InstLib.hpp
 *  @brief A specialized version of InstLib to handle AvidaCPU Instructions.
 */

#ifndef EMP_HARDWARE_AVIDACPU_INSTLIB_HPP_INCLUDE
#define EMP_HARDWARE_AVIDACPU_INSTLIB_HPP_INCLUDE

#include "../math/math.hpp"

#include "InstLib.hpp"

namespace emp {

  /// AvidaCPU_InstLib is a pure-virtual class that defines a series of instructions that
  /// can be used with AvidaCPU_Base or any of its derived classes.

  template <typename HARDWARE_T, typename ARG_T=size_t, size_t ARG_COUNT=3>
  struct AvidaCPU_InstLib : public InstLib<HARDWARE_T, ARG_T, ARG_COUNT> {
    using hardware_t = HARDWARE_T;
    using arg_t = ARG_T;
    using this_t = AvidaCPU_InstLib<HARDWARE_T, ARG_T, ARG_COUNT>;
    using inst_t = typename hardware_t::inst_t;

    static constexpr size_t arg_count = ARG_COUNT;

    // Instructions

    // One-input math
    static void Inst_Inc(hardware_t & hw, const inst_t & inst) { ++hw.regs[inst.args[0]]; }
    static void Inst_Dec(hardware_t & hw, const inst_t & inst) { --hw.regs[inst.args[0]]; }

    // Two-input math
    static void Inst_Not(hardware_t & hw, const inst_t & inst) {
      hw.regs[inst.args[0]] = (hw.regs[inst.args[0]] == 0.0);
    }
    static void Inst_SetReg(hardware_t & hw, const inst_t & inst) {
      hw.regs[inst.args[0]] = (double) inst.args[1];
    }
    static void Inst_Add(hardware_t & hw, const inst_t & inst) {
      hw.regs[inst.args[2]] = hw.regs[inst.args[0]] + hw.regs[inst.args[1]];
    }
    static void Inst_Sub(hardware_t & hw, const inst_t & inst) {
      hw.regs[inst.args[2]] = hw.regs[inst.args[0]] - hw.regs[inst.args[1]];
    }
    static void Inst_Mult(hardware_t & hw, const inst_t & inst) {
      hw.regs[inst.args[2]] = hw.regs[inst.args[0]] * hw.regs[inst.args[1]];
    }

    static void Inst_Div(hardware_t & hw, const inst_t & inst) {
      const double denom = hw.regs[inst.args[1]];
      if (denom == 0.0) ++hw.errors;
      else hw.regs[inst.args[2]] = hw.regs[inst.args[0]] / denom;
    }

    static void Inst_Mod(hardware_t & hw, const inst_t & inst) {
      const double base = hw.regs[inst.args[1]];
      if (base == 0.0) ++hw.errors;
      else hw.regs[inst.args[2]] = emp::Mod( hw.regs[inst.args[0]], base);
    }

    // Comparisons
    static void Inst_TestEqu(hardware_t & hw, const inst_t & inst) {
      hw.regs[inst.args[2]] = (hw.regs[inst.args[0]] == hw.regs[inst.args[1]]);
    }
    static void Inst_TestNEqu(hardware_t & hw, const inst_t & inst) {
      hw.regs[inst.args[2]] = (hw.regs[inst.args[0]] != hw.regs[inst.args[1]]);
    }
    static void Inst_TestLess(hardware_t & hw, const inst_t & inst) {
      hw.regs[inst.args[2]] = (hw.regs[inst.args[0]] < hw.regs[inst.args[1]]);
    }

    // Others...
    static void Inst_If(hardware_t & hw, const inst_t & inst) { // args[0] = test, args[1] = scope
      if (hw.UpdateScope(inst.args[1]) == false) return;      // If previous scope is unfinished, stop!
      if (hw.regs[inst.args[0]] == 0.0) hw.BypassScope(inst.args[1]); // If test fails, move to scope end.
    }

    static void Inst_While(hardware_t & hw, const inst_t & inst) {
      // UpdateScope returns false if previous scope isn't finished (e.g., while needs to loop)
      if (hw.UpdateScope(inst.args[1], ScopeType::LOOP) == false) return;
      if (hw.regs[inst.args[0]] == 0.0) hw.BypassScope(inst.args[1]); // If test fails, move to scope end.
    }

    static void Inst_Countdown(hardware_t & hw, const inst_t & inst) {  // Same as while, but auto-decriments test each loop.
      // UpdateScope returns false if previous scope isn't finished (e.g., while needs to loop)
      if (hw.UpdateScope(inst.args[1], ScopeType::LOOP) == false) return;
      if (hw.regs[inst.args[0]] == 0.0) hw.BypassScope(inst.args[1]);   // If test fails, move to scope end.
      else hw.regs[inst.args[0]]--;
    }

    static void Inst_Break(hardware_t & hw, const inst_t & inst) { hw.BypassScope(inst.args[0]); }
    static void Inst_Scope(hardware_t & hw, const inst_t & inst) { hw.UpdateScope(inst.args[0]); }

    static void Inst_Define(hardware_t & hw, const inst_t & inst) {
      if (hw.UpdateScope(inst.args[1]) == false) return; // Update which scope we are in.
      hw.fun_starts[inst.args[0]] = (int) hw.inst_ptr;     // Record where function should be exectuted.
      hw.BypassScope(inst.args[1]);                     // Skip over the function definition for now.
    }

    static void Inst_Call(hardware_t & hw, const inst_t & inst) {
      // Make sure function exists and is still in place.
      size_t def_pos = (size_t) hw.fun_starts[inst.args[0]];
      if (def_pos >= hw.genome.size()
          || hw.GetScopeType(hw.genome[def_pos].id) != ScopeType::FUNCTION) return;

      // Go back into the function's original scope (call is in that scope)
      size_t fun_scope = hw.genome[def_pos].args[1];
      if (hw.UpdateScope(fun_scope, ScopeType::FUNCTION) == false) return;
      hw.call_stack.push_back(hw.inst_ptr+1);        // Back up the call position
      hw.inst_ptr = def_pos+1;                       // Jump to the function body (will adavance)
    }

    static void Inst_Push(hardware_t & hw, const inst_t & inst) {
      hw.PushStack(inst.args[1], hw.regs[inst.args[0]]);
    }
    static void Inst_Pop(hardware_t & hw, const inst_t & inst) {
      hw.regs[inst.args[1]] = hw.PopStack(inst.args[0]);
    }

    static void Inst_Input(hardware_t & hw, const inst_t & inst) {
      // Determine the input ID and grab it if it exists; if not, return 0.0
      int input_id = (int) hw.regs[ inst.args[0] ];
      hw.regs[inst.args[1]] = Find(hw.inputs, input_id, 0.0);
    }

    static void Inst_Output(hardware_t & hw, const inst_t & inst) {
      // Save the date in the target reg to the specified output position.
      int output_id = (int) hw.regs[ inst.args[1] ];  // Grab ID from register.
      hw.outputs[output_id] = hw.regs[inst.args[0]];     // Copy target reg to appropriate output.
    }

    static void Inst_CopyVal(hardware_t & hw, const inst_t & inst) {
      hw.regs[inst.args[1]] = hw.regs[inst.args[0]];
    }

    static void Inst_ScopeReg(hardware_t & hw, const inst_t & inst) {
      hw.reg_stack.emplace_back(hw.CurScope(), inst.args[0], hw.regs[inst.args[0]]);
    }

    static const this_t & DefaultInstLib() {
      static this_t inst_lib;

      if (inst_lib.GetSize() == 0) {
        inst_lib.AddInst("Inc", Inst_Inc, 1, "Increment value in reg Arg1");
        inst_lib.AddInst("Dec", Inst_Dec, 1, "Decrement value in reg Arg1");
        inst_lib.AddInst("Not", Inst_Not, 1, "Logically toggle value in reg Arg1");
        inst_lib.AddInst("SetReg", Inst_SetReg, 2, "Set reg Arg1 to numerical value Arg2");
        inst_lib.AddInst("Add", Inst_Add, 3, "regs: Arg3 = Arg1 + Arg2");
        inst_lib.AddInst("Sub", Inst_Sub, 3, "regs: Arg3 = Arg1 - Arg2");
        inst_lib.AddInst("Mult", Inst_Mult, 3, "regs: Arg3 = Arg1 * Arg2");
        inst_lib.AddInst("Div", Inst_Div, 3, "regs: Arg3 = Arg1 / Arg2");
        inst_lib.AddInst("Mod", Inst_Mod, 3, "regs: Arg3 = Arg1 % Arg2");
        inst_lib.AddInst("TestEqu", Inst_TestEqu, 3, "regs: Arg3 = (Arg1 == Arg2)");
        inst_lib.AddInst("TestNEqu", Inst_TestNEqu, 3, "regs: Arg3 = (Arg1 != Arg2)");
        inst_lib.AddInst("TestLess", Inst_TestLess, 3, "regs: Arg3 = (Arg1 < Arg2)");
        inst_lib.AddInst("If", Inst_If, 2, "If reg Arg1 != 0, scope -> Arg2; else skip scope", ScopeType::BASIC, 1);
        inst_lib.AddInst("While", Inst_While, 2, "Until reg Arg1 != 0, repeat scope Arg2; else skip", ScopeType::LOOP, 1);
        inst_lib.AddInst("Countdown", Inst_Countdown, 2, "Countdown reg Arg1 to zero; scope to Arg2", ScopeType::LOOP, 1);
        inst_lib.AddInst("Break", Inst_Break, 1, "Break out of scope Arg1");
        inst_lib.AddInst("Scope", Inst_Scope, 1, "Enter scope Arg1", ScopeType::BASIC, 0);
        inst_lib.AddInst("Define", Inst_Define, 2, "Build function Arg1 in scope Arg2", ScopeType::FUNCTION, 1);
        inst_lib.AddInst("Call", Inst_Call, 1, "Call previously defined function Arg1");
        inst_lib.AddInst("Push", Inst_Push, 2, "Push reg Arg1 onto stack Arg2");
        inst_lib.AddInst("Pop", Inst_Pop, 2, "Pop stack Arg1 into reg Arg2");
        inst_lib.AddInst("Input", Inst_Input, 2, "Pull next value from input Arg1 into reg Arg2");
        inst_lib.AddInst("Output", Inst_Output, 2, "Push reg Arg1 into output Arg2");
        inst_lib.AddInst("CopyVal", Inst_CopyVal, 2, "Copy reg Arg1 into reg Arg2");
        inst_lib.AddInst("ScopeReg", Inst_ScopeReg, 1, "Backup reg Arg1; restore at end of scope");

        for (size_t i = 0; i < hardware_t::CPU_SIZE; i++) {
          inst_lib.AddArg(to_string((int)i), i);                   // Args can be called by value
          inst_lib.AddArg(to_string("Reg", 'A'+(char)i), i);  // ...or as a register.
        }
      }

      return inst_lib;
    }
  };

}

#endif // #ifndef EMP_HARDWARE_AVIDACPU_INSTLIB_HPP_INCLUDE
