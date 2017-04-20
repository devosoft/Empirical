//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This is a hardcoded CPU for Avida.
//
//  Instruction Set:
//    Inc, Dec, Not                   :  REG
//    Add, Sub, Mult, Div, Mod        :  IN1, IN2, OUT1
//    TestEqu, TestNEqu, TestLess     :  IN1, IN2, OUT1
//    If, While                       :  TEST, SCOPE
//    DoCount                         :  MAX_VAL, COUNTER, SCOPE
//    Scope                           :  SCOPE                   (change current scope)
//    Define, Call                    :  LABEL                   (build a function / call it)
//    Label, Jump, JumpIf0, JumpIfN0  :  LABEL, TEST
//    Push, Pop, Input, Output        :  VAL
//    CopyVal                         :  FROM, TO
//    ScopeVar                        :  VAR                     (move var into current scope)
//
//
//  Developer Notes:
//  * This implementation is intended to run fast, but not be flexible so that it will
//    be quick to implement.  It can be used as a baseline comparison for timings on more
//    flexible implementations later.


#ifndef EMP_AVIDA_GP_H
#define EMP_AVIDA_GP_H

#include "../base/array.h"
#include "../base/vector.h"


namespace emp {

  class AvidaGP {
  public:
    static constexpr size_t REGS = 16;
    static constexpr size_t INST_ARGS = 3;

    enum class Inst {
      Inc, Dec, Not, Add, Sub, Mult, Div, Mod,TestEqu, TestNEqu, TestLess,
      If, While, DoCount, Break, Scope, Define, Call, Label, Jump, JumpIf0, JumpIfN0,
      Push, Pop, Input, Output, CopyVal, ScopeVar
    };

    struct Instruction {
      Inst id;
      int arg1;  int arg2;  int arg3;

      Instruction(Inst _id, int _a1, int _a2, int _a3)
	      : id(_id), arg1(_a1), arg2(_a2), arg3(_a3) { ; }
    };


    using inst_t = Instruction;
    using genome_t = emp::vector<inst_t>;

  private:
    genome_t genome;
    size_t inst_ptr;
    size_t errors;

    emp::array<double, REGS> regs;

    // This function gets run every time scope changed (if, while, scope instructions, etc.)
    // If we are moving to an outer scope (lower value) we need to close the scope we are in,
    // potentially continuing with a loop.
    bool UpdateScope(int scope) { return true; }

    // This function fast-forwards to the end of the current scope.
    void BypassScope(int scope) { ; }

    // This function indicates that a loop has started and tracks the conditions.
    void EnterWhile(int scope) { ; }

  public:
    AvidaGP() : inst_ptr(0), errors(0) {
      // Initialize registers to their posision.  So Reg0 = 0 and Reg11 = 11.
      for (size_t i = 0; i < REGS; i++) regs[i] = (double) i;
    }
    ~AvidaGP() { ; }

    // Accessors
    inst_t GetInst(size_t pos) const { return genome[pos]; }
    const genome_t & GetGenome() const { return genome; }
    double GetReg(size_t id) const { return regs[id]; }

    void SetInst(size_t pos, const inst_t & inst) { genome[pos] = inst; }
    void SetGneome(const genome_t & g) { genome = g; }

    void PushInst(Inst inst, int arg1=0, int arg2=0, int arg3=0) {
      genome.emplace_back(inst, arg1, arg2, arg3);
    }

    void SingleProcess();
  };


  void AvidaGP::SingleProcess() {
    if (inst_ptr > genome.size()) inst_ptr = 0;

    const inst_t inst = genome[inst_ptr];

    switch (inst.id) {
    case Inst::Inc: ++regs[inst.arg1]; break;
    case Inst::Dec: --regs[inst.arg1]; break;
    case Inst::Not: regs[inst.arg1] = !regs[inst.arg1]; break;
    case Inst::Add: regs[inst.arg3] = regs[inst.arg1] + regs[inst.arg2]; break;
    case Inst::Sub: regs[inst.arg3] = regs[inst.arg1] - regs[inst.arg2]; break;
    case Inst::Mult: regs[inst.arg3] = regs[inst.arg1] * regs[inst.arg2]; break;

    case Inst::Div: {
      const double denom = regs[inst.arg2];
      if (denom == 0.0) ++errors;
      else regs[inst.arg3] = regs[inst.arg1] / denom;
    }
      break;

    case Inst::Mod: {
      const double base = regs[inst.arg2];
      if (base == 0.0) ++errors;
      else regs[inst.arg3] = regs[inst.arg1] / base;
    }
      break;

    case Inst::TestEqu: regs[inst.arg3] = (regs[inst.arg1] == regs[inst.arg2]); break;
    case Inst::TestNEqu: regs[inst.arg3] = (regs[inst.arg1] != regs[inst.arg2]); break;
    case Inst::TestLess: regs[inst.arg3] = (regs[inst.arg1] < regs[inst.arg2]); break;

    case Inst::If: // arg1 = test, arg2 = scope
      if (UpdateScope(inst.arg2) == false) break;     // If previous scope is unfinished, stop!
      if (!regs[inst.arg1]) BypassScope(inst.arg2);   // If test fails, move to scope end.
      break;                                          // Continue in current code.

    case Inst::While:
      if (UpdateScope(inst.arg2) == false) break;     // If previous scope is unfinished, stop!
      if (!regs[inst.arg1]) BypassScope(inst.arg2);   // If test fails, move to scope end.
      EnterWhile(inst.arg2);                          // Track to jump back to while start.
      break;

    case Inst::Break: BypassScope(inst.arg1); break;
    case Inst::Scope: UpdateScope(inst.arg1); break;

    case Inst::DoCount: break;
    case Inst::Define: break;
    case Inst::Call: break;
    case Inst::Label: break;
    case Inst::Jump: break;
    case Inst::JumpIf0: break;
    case Inst::JumpIfN0: break;
    case Inst::Push: break;
    case Inst::Pop: break;
    case Inst::Input: break;
    case Inst::Output: break;
    case Inst::CopyVal: break;
    case Inst::ScopeVar: break;
    };
  }

}


#endif
