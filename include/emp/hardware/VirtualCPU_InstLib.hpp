/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022
 *
 *  @file  VirtualCPU_InstLib.hpp
 *  @brief A specialized version of InstLib to handle VirtualCPU instructions.
 */

#ifndef EMP_VIRTUAL_CPU_INST_LIB_H
#define EMP_VIRTUAL_CPU_INST_LIB_H

#include "../base/error.hpp"
#include "../math/math.hpp"

#include "InstLib.hpp"

namespace emp {

  /// \brief A pure-virtual class that defines a series of instructions for VirtualCPU_Base or any of its derived classes.
  template <typename HARDWARE_T, typename ARG_T=size_t, size_t ARG_COUNT=3>
  struct VirtualCPU_InstLib : public InstLib<HARDWARE_T, ARG_T, ARG_COUNT> {
    using hardware_t = HARDWARE_T;
    using inst_lib_t = InstLib<HARDWARE_T, ARG_T, ARG_COUNT>;
    using arg_t = ARG_T;
    using this_t = VirtualCPU_InstLib<HARDWARE_T, ARG_T, ARG_COUNT>;
    using inst_t = typename hardware_t::inst_t;
    using nop_vec_t = typename hardware_t::nop_vec_t;

    // Instructions
    static void Inst_NopA(hardware_t & hw, const inst_t & inst) { ; }
    static void Inst_NopB(hardware_t & hw, const inst_t & inst) { ; }
    static void Inst_NopC(hardware_t & hw, const inst_t & inst) { ; }
    static void Inst_Inc(hardware_t & hw, const inst_t & inst) { 
      size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
      ++hw.regs[idx];
    }
    static void Inst_Dec(hardware_t & hw, const inst_t & inst) { 
      size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
      --hw.regs[idx];
    }
    static void Inst_If_Not_Equal(hardware_t & hw, const inst_t & inst) { 
      if(hw.expanded_nop_args){
        size_t idx_op_1 = inst.nop_vec.size() < 1 ? 1 : inst.nop_vec[0];
        size_t idx_op_2 = inst.nop_vec.size() < 2 ? hw.GetComplementNop(idx_op_1) : inst.nop_vec[1];
        if(hw.regs[idx_op_1] == hw.regs[idx_op_2])
          hw.AdvanceIP(1);
        hw.AdvanceIP(inst.nop_vec.size()); 
       }
      else{
        size_t idx_1 = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
        size_t idx_2 = hw.GetComplementNop(idx_1);
        if(hw.regs[idx_1] == hw.regs[idx_2])
          hw.AdvanceIP(1);
        if(inst.nop_vec.size()) hw.AdvanceIP(1); 
      }
    }
    static void Inst_If_Less(hardware_t & hw, const inst_t & inst) { 
      if(hw.expanded_nop_args){
        size_t idx_op_1 = inst.nop_vec.size() < 1 ? 1 : inst.nop_vec[0];
        size_t idx_op_2 = inst.nop_vec.size() < 2 ? hw.GetComplementNop(idx_op_1) : inst.nop_vec[1];
        if(hw.regs[idx_op_1] >= hw.regs[idx_op_2])
          hw.AdvanceIP(1);
        hw.AdvanceIP(inst.nop_vec.size()); 
      }
      else{
        size_t idx_1 = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
        size_t idx_2 = hw.GetComplementNop(idx_1);
        if(hw.regs[idx_1] >= hw.regs[idx_2])
          hw.AdvanceIP(1);
        if(inst.nop_vec.size()) hw.AdvanceIP(1); 
      }
    }
    static void Inst_Pop(hardware_t & hw, const inst_t & inst) { 
      size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
      hw.StackPop(idx);
    }
    static void Inst_Push(hardware_t & hw, const inst_t & inst) { 
      size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
      hw.StackPush(idx);
    }
    static void Inst_Swap_Stack(hardware_t & hw, const inst_t & inst) { 
      hw.StackSwap();
    }
    static void Inst_Shift_Right(hardware_t & hw, const inst_t & inst) { 
      size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
      hw.regs[idx] >>= 1;
    }
    static void Inst_Shift_Left(hardware_t & hw, const inst_t & inst) {
      size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
      hw.regs[idx] <<= 1;
    }
    static void Inst_Add(hardware_t & hw, const inst_t & inst) {
      if(hw.expanded_nop_args){
        size_t idx_res = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
        size_t idx_op_1 = inst.nop_vec.size() < 2 ? idx_res : inst.nop_vec[1];
        size_t idx_op_2 = inst.nop_vec.size() < 3 ? hw.GetComplementNop(idx_op_1) : inst.nop_vec[2];
        hw.regs[idx_res] = hw.regs[idx_op_1] + hw.regs[idx_op_2];
      }
      else{
        size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
        size_t idx_2 = hw.GetComplementNop(idx);
        hw.regs[idx] = hw.regs[idx] + hw.regs[idx_2];
      }
    }
    static void Inst_Sub(hardware_t & hw, const inst_t & inst) {
      if(hw.expanded_nop_args){
        size_t idx_res = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
        size_t idx_op_1 = inst.nop_vec.size() < 2 ? idx_res : inst.nop_vec[1];
        size_t idx_op_2 = inst.nop_vec.size() < 3 ? hw.GetComplementNop(idx_op_1) : inst.nop_vec[2];
        hw.regs[idx_res] = hw.regs[idx_op_1] - hw.regs[idx_op_2];
      }
      else{
        size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
        size_t idx_2 = hw.GetComplementNop(idx);
        hw.regs[idx] = hw.regs[idx] - hw.regs[idx_2];
      }
    }
    static void Inst_Nand(hardware_t & hw, const inst_t & inst) {
      if(hw.expanded_nop_args){
        size_t idx_res = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
        size_t idx_op_1 = inst.nop_vec.size() < 2 ? idx_res : inst.nop_vec[1];
        size_t idx_op_2 = inst.nop_vec.size() < 3 ? hw.GetComplementNop(idx_op_1) : inst.nop_vec[2];
        hw.regs[idx_res] = ~(hw.regs[idx_op_1] & hw.regs[idx_op_2]);
      }
      else{
        size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
        size_t idx_2 = hw.GetComplementNop(idx);
        hw.regs[idx] = hw.regs[idx] + hw.regs[idx_2];
        hw.regs[idx] = ~(hw.regs[idx] & hw.regs[idx_2]);
      }
    }
    static void Inst_IO(hardware_t & hw, const inst_t & inst) {
      size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
      std::cout << "Output: " << hw.regs[idx] << std::endl;
      // TODO: Handle input
    }
    static void Inst_H_Alloc(hardware_t & hw, const inst_t & inst) {
      hw.genome_working.resize(hw.genome.size() * 2, hw.GetDefaultInst());
      hw.regs[0] = hw.genome.size();
    }
    static void Inst_H_Divide(hardware_t & hw, const inst_t & inst) {
      if(hw.read_head >= hw.genome.size()){
        hw.genome_working.resize(hw.read_head, 0);
        hw.ResetHardware();
        hw.inst_ptr = hw.genome.size() - 1;
        std::cout << "Divide!" << std::endl;
      }
    }
    static void Inst_H_Copy(hardware_t & hw, const inst_t & inst) {
      hw.genome_working[hw.write_head] = hw.genome_working[hw.read_head];
      hw.copied_inst_id_vec.push_back(hw.genome_working[hw.write_head].id);
      hw.read_head++;
      while(hw.read_head >= hw.genome_working.size()) hw.read_head -= hw.genome_working.size();
      hw.write_head++;
      while(hw.write_head >= hw.genome_working.size()) hw.write_head -= hw.genome_working.size();
      // TODO: Mutation
    }
    static void Inst_H_Search(hardware_t & hw, const inst_t & inst) {
      size_t res = hw.FindNopSequence(hw.GetComplementNopSequence(inst.nop_vec), hw.inst_ptr);
      if(inst.nop_vec.size() == 0 || res == hw.inst_ptr){
        hw.regs[1] = 0;
        hw.regs[2] = 0;
        hw.SetFH(hw.inst_ptr + 1);
      }
      else{
        hw.regs[1] = (res - hw.inst_ptr) > 0 ? res - hw.inst_ptr : res + hw.genome_working.size() - res + hw.inst_ptr;
        hw.regs[2] = inst.nop_vec.size();
        hw.SetFH(res + inst.nop_vec.size() + 1);
      }
    }
    static void Inst_Mov_Head(hardware_t & hw, const inst_t & inst) {
      if(hw.expanded_nop_args){
        size_t dest_idx = hw.flow_head;
        if(inst.nop_vec.size() >= 2) dest_idx = hw.GetModdedHead(inst.nop_vec[1]);
        if(!inst.nop_vec.empty()) hw.SetModdedHead(inst.nop_vec[0], dest_idx);
        else hw.SetIP(dest_idx);
      }
      else{
        if(!inst.nop_vec.empty()){
          // IP is a special case because it auto advances!
          if(inst.nop_vec[0] % 4 == 0) hw.SetIP(hw.flow_head - 1);
          else hw.SetModdedHead(inst.nop_vec[0], hw.flow_head);
        }
        else hw.SetIP(hw.flow_head - 1);
      }
    }
    static void Inst_Jmp_Head(hardware_t & hw, const inst_t & inst) {
      if(hw.expanded_nop_args){
        size_t jump_dist = hw.regs[1];
        if(inst.nop_vec.size() >= 2) jump_dist = hw.regs[inst.nop_vec[1]];
        if(!inst.nop_vec.empty()) hw.AdvanceModdedHead(inst.nop_vec[0], jump_dist);
        else hw.AdvanceIP(jump_dist);
      }
      else{
        if(!inst.nop_vec.empty()) hw.AdvanceModdedHead(inst.nop_vec[0], hw.regs[2]);
        else hw.AdvanceIP(hw.regs[2]);
      }
    }
    static void Inst_Get_Head(hardware_t & hw, const inst_t & inst) {
      if(hw.expanded_nop_args){
        size_t head_val = inst.nop_vec.empty() ? hw.inst_ptr : hw.GetModdedHead(inst.nop_vec[0]);
        if(inst.nop_vec.size() < 2) hw.regs[2] = head_val;
        else hw.regs[inst.nop_vec[1]] = head_val;
      }
      else{
        if(inst.nop_vec.empty()) hw.regs[2] = hw.inst_ptr;
        else hw.regs[2] = hw.GetModdedHead(inst.nop_vec[0]);
      }
    }
    static void Inst_If_Label(hardware_t & hw, const inst_t & inst) {
      hw.AdvanceIP(inst.nop_vec.size());
      if(!hw.CheckIfLastCopied(hw.GetComplementNopSequence(inst.nop_vec))) hw.AdvanceIP();
    }
    static void Inst_Set_Flow(hardware_t & hw, const inst_t & inst) {
      size_t idx = inst.nop_vec.empty() ? 2 : inst.nop_vec[0];
      hw.SetFH(hw.regs[idx]);
    }

    /// Maintain and return a singleton of default instructions
    static const this_t & DefaultInstLib() {
      static this_t inst_lib;
      if (inst_lib.GetSize() == 0) {
        inst_lib.AddInst("NopA", Inst_NopA, 0, "No-operation A");
        inst_lib.AddInst("NopB", Inst_NopB, 0, "No-operation B");
        inst_lib.AddInst("NopC", Inst_NopC, 0, "No-operation C");
        inst_lib.AddInst("IfNEq", Inst_If_Not_Equal, 1, 
            "Skip next inst unless register values match");
        inst_lib.AddInst("IfLess", Inst_If_Less, 1, 
            "Skip next inst unless focal register is less than its complement");
        inst_lib.AddInst("Inc", Inst_Inc, 1, "Increment value in reg Arg1");
        inst_lib.AddInst("Dec", Inst_Dec, 1, "Decrement value in reg Arg1");
        inst_lib.AddInst("Pop", Inst_Pop, 1, "Pop value from active stack into register");
        inst_lib.AddInst("Push", Inst_Push, 1, "Add register's value to active stack");
        inst_lib.AddInst("Swap-Stk", Inst_Swap_Stack, 1, "Swap which stack is active");
        inst_lib.AddInst("ShiftR", Inst_Shift_Right, 1, "Shift register value right by one bit");
        inst_lib.AddInst("ShiftL", Inst_Shift_Left, 1, "Shift register value left by one bit");
        inst_lib.AddInst("Add", Inst_Add, 1, 
            "Add values in registers B and C, then store result in given register");
        inst_lib.AddInst("Sub", Inst_Sub, 1, 
            "Sub values in registers B and C, then store result in given register");
        inst_lib.AddInst("Nand", Inst_Nand, 1, 
            "NAND values in registers B and C, then store result in given register");
        inst_lib.AddInst("IO", Inst_IO, 1, 
            "Output value in given register and then place new input in that register");
        inst_lib.AddInst("HAlloc", Inst_H_Alloc, 1, "Allocate memory for offspring");
        inst_lib.AddInst("HDivide", Inst_H_Divide, 1, "Attempt to split offspring");
        inst_lib.AddInst("HCopy", Inst_H_Copy, 1, "Copy instruction from read head to write head");
        inst_lib.AddInst("HSearch", Inst_H_Search, 1, "Search for label complement");
        inst_lib.AddInst("MovHead", Inst_Mov_Head, 1, "Move a given head to a postiion");
        inst_lib.AddInst("JmpHead", Inst_Jmp_Head, 1, "Move a given head by a relative amount");
        inst_lib.AddInst("GetHead", Inst_Get_Head, 1, "Get location of head");
        inst_lib.AddInst("IfLabel", Inst_If_Label, 1, 
            "Execute next instruction if label was the last thing copied");
        inst_lib.AddInst("SetFlow", Inst_Set_Flow, 1, "Set flow head to register value");
      /*
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
      */

        //for (size_t i = 0; i < hardware_t::NUM_REGS; i++) {
        //  inst_lib.AddArg(to_string((int)i), i);                   // Args can be called by value
        //  inst_lib.AddArg(to_string("Reg", 'A'+(char)i), i);  // ...or as a register.
        //}
      }

      return inst_lib;
    }
  };

}

#endif
