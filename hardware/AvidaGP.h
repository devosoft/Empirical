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
//    VAR                             :  NEW_SCOPE VAR
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

#include "Instruction.h"


namespace emp {

  class AvidaGP {
  private:
    static constexpr size_t REGS = 16;
    static constexpr size_t INST_ARGS = 3;

    emp::vector< Instruction<INST_ARGS> > genome;
    size_t inst_ptr;

    emp::array<double, REGS> regs;

  public:
    AvidaGP() : inst_ptr(0) { ; }
    ~AvidaGP() { ; }

  };

}


#endif
