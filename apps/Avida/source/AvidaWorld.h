/**
 *  @note This file is part of The Avida Digital Evolution Platform
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file  AvidaWorld.h
 *  @brief This is the default, Avida-specific world.
 */

#include "AvidaOrg.h"

#include "Evo/World.h"
#include "hardware/InstLib.h"

class AvidaWorld : public emp::World<AvidaOrg> {
public:
  using inst_lib_t = emp::InstLib<AvidaOrg>;

private:
  inst_lib_t inst_lib;

  double energy_threshold;  ///< Base energy requirement for an organism to replicate.
  double energy_inflow;     ///< Base amount of energy collected per update for each organism.
public:
  AvidaWorld() : inst_lib() {
    // Build the instruction library...
    inst_lib.AddInst("Inc", emp::AvidaGP::Inst_Inc, 1, "Increment value in reg Arg1");
    inst_lib.AddInst("Dec", emp::AvidaGP::Inst_Dec, 1, "Decrement value in reg Arg1");
    inst_lib.AddInst("Not", emp::AvidaGP::Inst_Not, 1, "Logically toggle value in reg Arg1");
    inst_lib.AddInst("SetReg", emp::AvidaGP::Inst_SetReg, 2, "Set reg Arg1 to numerical value Arg2");
    inst_lib.AddInst("Add", emp::AvidaGP::Inst_Add, 3, "regs: Arg3 = Arg1 + Arg2");
    inst_lib.AddInst("Sub", emp::AvidaGP::Inst_Sub, 3, "regs: Arg3 = Arg1 - Arg2");
    inst_lib.AddInst("Mult", emp::AvidaGP::Inst_Mult, 3, "regs: Arg3 = Arg1 * Arg2");
    inst_lib.AddInst("Div", emp::AvidaGP::Inst_Div, 3, "regs: Arg3 = Arg1 / Arg2");
    inst_lib.AddInst("Mod", emp::AvidaGP::Inst_Mod, 3, "regs: Arg3 = Arg1 % Arg2");
    inst_lib.AddInst("TestEqu", emp::AvidaGP::Inst_TestEqu, 3, "regs: Arg3 = (Arg1 == Arg2)");
    inst_lib.AddInst("TestNEqu", emp::AvidaGP::Inst_TestNEqu, 3, "regs: Arg3 = (Arg1 != Arg2)");
    inst_lib.AddInst("TestLess", emp::AvidaGP::Inst_TestLess, 3, "regs: Arg3 = (Arg1 < Arg2)");
    inst_lib.AddInst("If", emp::AvidaGP::Inst_If, 2, "If reg Arg1 != 0, scope -> Arg2; else skip scope", emp::ScopeType::BASIC, 1);
    inst_lib.AddInst("While", emp::AvidaGP::Inst_While, 2, "Until reg Arg1 != 0, repeat scope Arg2; else skip", emp::ScopeType::LOOP, 1);
    inst_lib.AddInst("Countdown", emp::AvidaGP::Inst_Countdown, 2, "Countdown reg Arg1 to zero; scope to Arg2", emp::ScopeType::LOOP, 1);
    inst_lib.AddInst("Break", emp::AvidaGP::Inst_Break, 1, "Break out of scope Arg1");
    inst_lib.AddInst("Scope", emp::AvidaGP::Inst_Scope, 1, "Enter scope Arg1", emp::ScopeType::BASIC, 0);
    inst_lib.AddInst("Define", emp::AvidaGP::Inst_Define, 2, "Build function Arg1 in scope Arg2", emp::ScopeType::FUNCTION, 1);
    inst_lib.AddInst("Call", emp::AvidaGP::Inst_Call, 1, "Call previously defined function Arg1");
    inst_lib.AddInst("Push", emp::AvidaGP::Inst_Push, 2, "Push reg Arg1 onto stack Arg2");
    inst_lib.AddInst("Pop", emp::AvidaGP::Inst_Pop, 2, "Pop stack Arg1 into reg Arg2");
    inst_lib.AddInst("Input", emp::AvidaGP::Inst_Input, 2, "Pull next value from input Arg1 into reg Arg2");
    inst_lib.AddInst("Output", emp::AvidaGP::Inst_Output, 2, "Push reg Arg1 into output Arg2");
    inst_lib.AddInst("CopyVal", emp::AvidaGP::Inst_CopyVal, 2, "Copy reg Arg1 into reg Arg2");
    inst_lib.AddInst("ScopeReg", emp::AvidaGP::Inst_ScopeReg, 1, "Backup reg Arg1; restore at end of scope");

    inst_lib.AddInst("Replicate", AvidaOrg::Inst_Replicate, 0,
                     "Spend energy needed to reproduce Organism.");
  }
  ~AvidaWorld() { ; }

  const inst_lib_t & GetInstLib() const { return inst_lib; }
  double GetEnergyThreshold() const { return energy_threshold; }
  double GetEnergyInflow() const { return energy_inflow; }

};
