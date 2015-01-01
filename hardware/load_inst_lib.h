#ifndef EMP_LOAD_INST_LIB_H
#define EMP_LOAD_INST_LIB_H

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file contains tools to load instructions sets.
//

#include <functional>
using namespace std::placeholders;

#include "HardwareCPU.h"
#include "InstLib.h"

namespace emp {

  template <int CPU_SCALE=8, int STACK_SIZE=16>
  void Load4StackDefault(InstLib<HardwareCPU<CPU_SCALE, STACK_SIZE>, Instruction> & lib) {
    // Load as many nops as we need.  This we be called Nop-0, Nop-1, Nop-2, etc.
    for (int i = 0; i < CPU_SCALE; i++) {
      std::string inst_name = "Nop-";
      inst_name += std::to_string(i);
      lib.Add(inst_name, "No-operation instruction; usable as modifier.",
              std::bind(&HardwareCPU<>::Inst_Nop, _1), i, 1);
    }

    // Load in single-argument math operations.
    lib.Add("Inc", "Increment top of ?Stack-B? by one",
            std::bind(&HardwareCPU<>::Inst_1I_Math<1,0>, _1, [](int a){return a+1;}));
    lib.Add("Dec", "Decrement top of ?Stack-B? by one",
            std::bind(&HardwareCPU<>::Inst_1I_Math<1,0>, _1, [](int a){return a-1;}));
    lib.Add("Shift-L", "Shift bits of top of ?Stack-B? to the left by one",
            std::bind(&HardwareCPU<>::Inst_1I_Math<1,0>, _1, [](int a){return a<<1;}));
    lib.Add("Shift-R", "Shift bits of top of ?Stack-B? to the right by one",
            std::bind(&HardwareCPU<>::Inst_1I_Math<1,0>, _1, [](int a){return a>>1;}));

    // Load in double-argument math operations.
    lib.Add("Nand", "Compute: ?Stack-B?-top nand ?Stack-C?-top and push result to ?Stack-B?",
            std::bind(&HardwareCPU<>::Inst_2I_Math<>, _1, [](int a, int b){ return ~(a&b); }));
    lib.Add("Add", "Compute: ?Stack-B?-top plus ?Stack-C?-top and push result to ?Stack-B?",
            std::bind(&HardwareCPU<>::Inst_2I_Math<>, _1, [](int a, int b){ return a+b; }));
    lib.Add("Sub", "Compute: ?Stack-B?-top minus ?Stack-C?-top and push result to ?Stack-B?",
            std::bind(&HardwareCPU<>::Inst_2I_Math<>, _1, [](int a, int b){ return a-b; }));
    lib.Add("Mult", "Compute: ?Stack-B?-top times ?Stack-C?-top and push result to ?Stack-B?",
            std::bind(&HardwareCPU<>::Inst_2I_Math<>, _1, [](int a, int b){ return a*b; }));

    // @CAO For the next two, ideally if b==0, we should have the instruction return false...
    lib.Add("Div", "Compute: ?Stack-B?-top div ?Stack-C?-top and push result to ?Stack-B?",
            std::bind(&HardwareCPU<>::Inst_2I_Math<>, _1, [](int a, int b){ return b?a/b:0; }));
    lib.Add("Mod", "Compute: ?Stack-B?-top mod ?Stack-C?-top and push result to ?Stack-B?",
            std::bind(&HardwareCPU<>::Inst_2I_Math<>, _1, [](int a, int b){ return b?a%b:0; }));

    // Conditionals
    lib.Add("Test-Equal", "Test if ?Stack-B?-top == ?Stack-C?-top and push result to ?Stack-D?",
            std::bind(&HardwareCPU<>::Inst_2I_Math<1,1,3>, _1, [](int a, int b){ return a==b; }));
    lib.Add("Test-NEqual", "Test if ?Stack-B?-top != ?Stack-C?-top and push result to ?Stack-D?",
            std::bind(&HardwareCPU<>::Inst_2I_Math<1,1,3>, _1, [](int a, int b){ return a!=b; }));
    lib.Add("Test-Less", "Test if ?Stack-B?-top < ?Stack-C?-top and push result to ?Stack-D?",
            std::bind(&HardwareCPU<>::Inst_2I_Math<1,1,3>, _1, [](int a, int b){ return a<b; }));
    lib.Add("Test-AtStart", "Test if ?Head-Read? is at mem position 0 and push result to ?Stack-D?",
                     std::mem_fn(&HardwareCPU<>::Inst_TestAtStart));

    // Load in Jump operations  [we neeed to do better...  push and pop heads?]
    lib.Add("Jump", "Move ?Head-IP? to position of ?Head-Flow?",
            std::mem_fn(&HardwareCPU<>::Inst_MoveHeadToHead<0, 3>));
    lib.Add("Jump-If0", "Move ?Head-IP? to position of ?Head-Flow? only if ?Stack-D?-top == 0",
            std::bind(&HardwareCPU<>::Inst_MoveHeadToHeadIf<0,3,3>, _1, [](int a){ return a==0; }));
    lib.Add("Jump-IfN0", "Move ?Head-IP? to position of ?Head-Flow? only if ?Stack-D?-top != 0",
            std::bind(&HardwareCPU<>::Inst_MoveHeadToHeadIf<0,3,3>, _1, [](int a){ return a!=0; }));
    lib.Add("Bookmark", "Move ?Head-Flow? to position of ?Head-IP?",
            std::mem_fn(&HardwareCPU<>::Inst_MoveHeadToHead<3, 0>));
    lib.Add("Set-Memory", "Move ?Head-Write? to position 0 in ?Memory-1?",
            std::mem_fn(&HardwareCPU<>::Inst_MoveHeadToMem<2, 1>));
    // "Find-Label" ********** - Jumps the flow head to a complement label (?...) in its current memory.    

    // Juggle stack contents
    lib.Add("Val-Move", "Pop ?Stack-B? and push value onto ?Stack-C?",
            std::bind(&HardwareCPU<>::Inst_1I_Math<1,1>, _1, [](int a){return a;}));
    lib.Add("Val-Copy", "Copy top of ?Stack-B? onto ?Stack-C?",
            std::bind(&HardwareCPU<>::Inst_1I_Math<1,1,false>, _1, [](int a){return a;}));
    lib.Add("Val-Delete", "Pop ?Stack-B? and discard value",
            std::mem_fn(&HardwareCPU<>::Inst_ValDelete));

    // Load in "Biological" instructions
    lib.Add("Build-Inst", "Add new instruction to end of ?Memory-1? copied from ?Head-Read?",
            std::mem_fn(&HardwareCPU<>::Inst_BuildInst));
    // "Divide" **********      - Moves memory space 1 (?1) into its own hardware.  Needs callback!
    // "Get-Input" **********   - Needs callback
    // "Get-Output" **********  - Needs callback
    // "Inject" ?? **********   - Needs callback
  }

};

#endif
