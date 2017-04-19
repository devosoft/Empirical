//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This is a hardcoded CPU for Avida.
//
//  Instruction Set:
//    Inc, Dec, Not                   :  IN1, OUT1
//    Add, Sub, Mult, Div, Mod        :  IN1, IN2, OUT1
//    TestEqu, TestNEqu, TestLess     :  IN1, IN2, OUT1
//    If, While                       :  TEST, SCOPE
//    DoRange                         :  START, END, SCOPE
//    End                             :  SCOPE
//    Define, Call                    :  LABEL
//    Label, Jump, JumpIf0, JumpIfN0  :  LABEL, TEST
//    Push, Pop, Input, Output        :  VAL
//    CopyVal                         :  FROM, TO
//    Var                             :  NEW_SCOPE VAR
//
//
//  Developer Notes:
//  * This implementation is intended to run fast, but not be flexible so that it will
//    be quick to implement.  It can be used as a baseline comparison for more flexible
//    implementations later.


#ifndef EMP_AVIDA_GP_H
#define EMP_AVIDA_GP_H

#include "../base/array.h"
#include "../base/vector.h"


namespace emp {

  class AvidaGP {
  public:
    static constexpr size_t REGS = 16;
    static constexpr size_t INST_ARGS = 3;

    enum class Inst { Inc, Dec, Not, Add, Sub, Mult, Div, Mod,TestEqu, TestNEqu, TestLess, If, While, DoRange, End, Define, Call, Label, Jump, JumpIf0, JumpIfN0, Push, Pop, Input, Output, CopyVal, Var };
    
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

    emp::array<double, REGS> regs;

  public:
    AvidaGP() : inst_ptr(0) {
      for (size_t i = 0; i < REGS; i++) regs[i] = (double) i; 
    }
    ~AvidaGP() { ; }

    inst_t GetInst(size_t pos) const { return genome[pos]; }
    const genome_t & GetGenome() const { return genome; }
    double GetReg(size_t id) const { return regs[id]; }

    void PushInst(Inst inst, int arg1=0, int arg2=0, int arg3=0) {
      genome.emplace_back(inst, arg1, arg2, arg3);
    }

    void SingleProcess();
  };


  void AvidaGP::SingleProcess() {
    if (inst_ptr > genome.size()) inst_ptr = 0;

    const inst_t inst = genome[inst_ptr];

    switch (inst.id) {
    case Inst::Inc: regs[inst.arg1]++; break;
    case Inst::Dec: break;
    case Inst::Not: break;
    case Inst::Add: regs[inst.arg3] = regs[inst.arg1] + regs[inst.arg2]; break;
    case Inst::Sub: break;
    case Inst::Mult: break;
    case Inst::Div: break;
    case Inst::Mod: break;
    case Inst::TestEqu: break;
    case Inst::TestNEqu: break;
    case Inst::TestLess: break;
    case Inst::If: break;
    case Inst::While: break;
    case Inst::DoRange: break;
    case Inst::End: break;
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
    case Inst::Var: break;
    };
  }

}


#endif
