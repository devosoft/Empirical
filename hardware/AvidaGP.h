//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This is a hardcoded CPU for Avida.
//
//  Instruction Set:
//    Nop-0 through Nop-15
//    Inc, Dec, ShiftL, ShiftR, Not
//    Add, Sub, Mult, Div, Mod  :  IN1, IN2, OUT1
//    If, While, DoRange, Define, End, Call
//    TestEqu, TestNEqu, TestLess
//    Label, Jump, JumpIf0, JumpIfN0
//    Push, Pop, Input, Output
//
//  Developer Notes:
//  * This implementation is intended to run fast, but not be flexible so that it will
//    be quick to implement.  It can be used as a baseline comparison for more flexible
//    implementations later.

namesapce emp {

  class AvidaGP {
  };

}
