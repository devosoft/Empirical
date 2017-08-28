#include "AvidaOrg.h"

#include "Evo/World.h"
#include "hardware/InstLib.h"

class AvidaWorld : public emp::World<AvidaOrg> {
private:
  emp::InstLib<AvidaOrg> inst_lib;

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
    inst_lib.AddInst("While", emp::AvidaGP::Inst_While, 2, "Until reg Arg1 != 0, repeat scope Arg2; else skip", emp::ScopeType::LOOP, 1);
    inst_lib.AddInst("Countdown", emp::AvidaGP::Inst_Countdown, 2, "Countdown reg Arg1 to zero; scope to Arg2", emp::ScopeType::LOOP, 1);
    inst_lib.AddInst("Break", emp::AvidaGP::Inst_Break, 1, "Break out of scope Arg1");
    inst_lib.AddInst("Scope", emp::AvidaGP::Inst_Scope, 1, "Enter scope Arg1", emp::ScopeType::BASIC, 0);
    inst_lib.AddInst("Input", emp::AvidaGP::Inst_Input, 2, "Pull next value from input Arg1 into reg Arg2");
    inst_lib.AddInst("Output", emp::AvidaGP::Inst_Output, 2, "Push reg Arg1 into output Arg2");
    inst_lib.AddInst("CopyVal", emp::AvidaGP::Inst_CopyVal, 2, "Copy reg Arg1 into reg Arg2");
    inst_lib.AddInst("ScopeReg", emp::AvidaGP::Inst_ScopeReg, 1, "Backup reg Arg1; restore at end of scope");
  }
  ~AvidaWorld() { ; }

};
