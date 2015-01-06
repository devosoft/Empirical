#ifndef EMP_LOAD_INST_LIB_H
#define EMP_LOAD_INST_LIB_H

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file contains tools to load instructions sets.
//

#include <functional>
using namespace std::placeholders;

#include "../tools/string_utils.h"
#include "HardwareCPU.h"
#include "InstLib.h"

namespace emp {

  template <int CPU_SCALE=8, int STACK_SIZE=16>
  void LoadInst(InstLib<HardwareCPU<CPU_SCALE, STACK_SIZE>, Instruction> & lib,
                std::string inst_info)
  {
    // Determine the instruction name.
    compress_whitespace(inst_info);
    const std::string inst_name = string_pop_word(inst_info);
    const std::string inst_base = string_get(inst_name, ':');
    std::string inst_arg = "";
    if (inst_name.size() != inst_base.size()) {
      inst_arg = string_get(inst_name, ':', inst_base.size()+1);
    }

    // Collect additional arguments.
    left_justify(inst_info);
    while(inst_info.size() > 0) {
      std::string arg_info = string_pop_word(inst_info);
      std::string arg_name = string_pop(arg_info, '=');

      if (arg_name == "cycle_cost") {
        // @CAO Continue here...
      }
      else if (arg_name == "weight") {
        // @CAO Continue here...
      }
      else {
        // @CAO Continue here... (With an error!)
      }
    }
    

    // Determine which argument we have and load it!

    // Start with Nop:0, etc, instructions.
    if (inst_name == "Nop") {
      int mod_id = std::stoi(inst_arg) % CPU_SCALE;
      if (mod_id < 0) mod_id += CPU_SCALE;
      lib.Add(inst_name, "No-operation instruction; usable as modifier.",
              std::bind(&HardwareCPU<>::Inst_Nop, _1), mod_id, 1);
    }

    // Check for single-argument math operations.
    else if (inst_name == "Inc") {
      lib.Add("Inc", "Increment top of ?Stack-B? by one",
              std::bind(&HardwareCPU<>::Inst_1I_Math<1,0>, _1, [](int a){return a+1;}));
    }
    else if (inst_name == "Dec") {
      lib.Add("Dec", "Decrement top of ?Stack-B? by one",
              std::bind(&HardwareCPU<>::Inst_1I_Math<1,0>, _1, [](int a){return a-1;}));
    }
    else if (inst_name == "Shift-L") {
      lib.Add("Shift-L", "Shift bits of top of ?Stack-B? to the left by one",
              std::bind(&HardwareCPU<>::Inst_1I_Math<1,0>, _1, [](int a){return a<<1;}));
    }
    else if (inst_name == "Shift-R") {
      lib.Add("Shift-R", "Shift bits of top of ?Stack-B? to the right by one",
              std::bind(&HardwareCPU<>::Inst_1I_Math<1,0>, _1, [](int a){return a>>1;}));
    }

    // Load in double-argument math operations.
    else if (inst_name == "Nand") {
      lib.Add("Nand", "Compute: ?Stack-B?-top nand ?Stack-C?-top and push result to ?Stack-B?",
              std::bind(&HardwareCPU<>::Inst_2I_Math<>, _1, [](int a, int b){ return ~(a&b); }));
    }
    else if (inst_name == "Add") {
      lib.Add("Add", "Compute: ?Stack-B?-top plus ?Stack-C?-top and push result to ?Stack-B?",
              std::bind(&HardwareCPU<>::Inst_2I_Math<>, _1, [](int a, int b){ return a+b; }));
    }
    else if (inst_name == "Sub") {
      lib.Add("Sub", "Compute: ?Stack-B?-top minus ?Stack-C?-top and push result to ?Stack-B?",
              std::bind(&HardwareCPU<>::Inst_2I_Math<>, _1, [](int a, int b){ return a-b; }));
    }
    else if (inst_name == "Mult") {
      lib.Add("Mult", "Compute: ?Stack-B?-top times ?Stack-C?-top and push result to ?Stack-B?",
              std::bind(&HardwareCPU<>::Inst_2I_Math<>, _1, [](int a, int b){ return a*b; }));
    }

    // @CAO For the next two, ideally if b==0, we should have the instruction return false...
    else if (inst_name == "Div") {
      lib.Add("Div", "Compute: ?Stack-B?-top div ?Stack-C?-top and push result to ?Stack-B?",
              std::bind(&HardwareCPU<>::Inst_2I_Math<>, _1, [](int a, int b){ return b?a/b:0; }));
    }
    else if (inst_name == "Mod") {
      lib.Add("Mod", "Compute: ?Stack-B?-top mod ?Stack-C?-top and push result to ?Stack-B?",
              std::bind(&HardwareCPU<>::Inst_2I_Math<>, _1, [](int a, int b){ return b?a%b:0; }));
    }

    // Conditionals
    else if (inst_name == "Test-Equal") {
      lib.Add("Test-Equal", "Test if ?Stack-B?-top == ?Stack-C?-top and push result to ?Stack-D?",
              std::bind(&HardwareCPU<>::Inst_2I_Math<1,1,3>, _1, [](int a, int b){ return a==b; }));
    }
    else if (inst_name == "Test-NEqual") {
      lib.Add("Test-NEqual", "Test if ?Stack-B?-top != ?Stack-C?-top and push result to ?Stack-D?",
              std::bind(&HardwareCPU<>::Inst_2I_Math<1,1,3>, _1, [](int a, int b){ return a!=b; }));
    }
    else if (inst_name == "Test-Less") {
      lib.Add("Test-Less", "Test if ?Stack-B?-top < ?Stack-C?-top and push result to ?Stack-D?",
              std::bind(&HardwareCPU<>::Inst_2I_Math<1,1,3>, _1, [](int a, int b){ return a<b; }));
    }
    else if (inst_name == "Test-AtStart") {
      lib.Add("Test-AtStart", "Test if ?Head-Read? is at mem position 0 and push result to ?Stack-D?",
              std::mem_fn(&HardwareCPU<>::Inst_TestAtStart));
    }

    // Load in Jump operations  [we neeed to do better...  push and pop heads?]
    else if (inst_name == "Jump") {
      lib.Add("Jump", "Move ?Head-IP? to position of ?Head-Flow?",
              std::mem_fn(&HardwareCPU<>::Inst_MoveHeadToHead<0, 3>));
    }
    else if (inst_name == "Jump-If0") {
      lib.Add("Jump-If0", "Move ?Head-IP? to position of ?Head-Flow? only if ?Stack-D?-top == 0",
              std::bind(&HardwareCPU<>::Inst_MoveHeadToHeadIf<0,3,3>, _1, [](int a){ return a==0; }));
    }
    else if (inst_name == "Jump-IfN0") {
      lib.Add("Jump-IfN0", "Move ?Head-IP? to position of ?Head-Flow? only if ?Stack-D?-top != 0",
              std::bind(&HardwareCPU<>::Inst_MoveHeadToHeadIf<0,3,3>, _1, [](int a){ return a!=0; }));
    }
    else if (inst_name == "Bookmark") {
      lib.Add("Bookmark", "Move ?Head-Flow? to position of ?Head-IP?",
              std::mem_fn(&HardwareCPU<>::Inst_MoveHeadToHead<3, 0>));
    }
    else if (inst_name == "Set-Memory") {
      lib.Add("Set-Memory", "Move ?Head-Write? to position 0 in ?Memory-1?",
              std::mem_fn(&HardwareCPU<>::Inst_MoveHeadToMem<2, 1>));
    }
    // "Find-Label" ********** - Jumps the flow head to a complement label (?...) in its current memory.    

    // Juggle stack contents
    else if (inst_name == "Val-Move") {
      lib.Add("Val-Move", "Pop ?Stack-B? and push value onto ?Stack-C?",
              std::bind(&HardwareCPU<>::Inst_1I_Math<1,1>, _1, [](int a){return a;}));
    }
    else if (inst_name == "Val-Copy") {
      lib.Add("Val-Copy", "Copy top of ?Stack-B? onto ?Stack-C?",
              std::bind(&HardwareCPU<>::Inst_1I_Math<1,1,false>, _1, [](int a){return a;}));
    }
    else if (inst_name == "Val-Delete") {
      lib.Add("Val-Delete", "Pop ?Stack-B? and discard value",
              std::mem_fn(&HardwareCPU<>::Inst_ValDelete));
    }

    // Check for "Biological" instructions
    else if (inst_name == "Build-Inst") {
      lib.Add("Build-Inst", "Add new instruction to end of ?Memory-1? copied from ?Head-Read?",
              std::mem_fn(&HardwareCPU<>::Inst_BuildInst));
    }
    // "Divide" **********      - Moves memory space 1 (?1) into its own hardware.  Needs callback!
    // "Get-Input" **********   - Needs callback
    // "Get-Output" **********  - Needs callback
    // "Inject" ?? **********   - Needs callback
  }

  template <int CPU_SCALE=8, int STACK_SIZE=16>
  void Load4StackDefault(InstLib<HardwareCPU<CPU_SCALE, STACK_SIZE>, Instruction> & lib) {
    // Load as many nops as we need.  This we be called Nop-0, Nop-1, Nop-2, etc.
    for (int i = 0; i < CPU_SCALE; i++) {
      std::string inst_name = "Nop:";
      inst_name += std::to_string(i);
      LoadInst(lib, inst_name);
    }

    // Load in single-argument math operations.
    LoadInst(lib, "Inc");
    LoadInst(lib, "Dec");
    LoadInst(lib, "Shift-L");
    LoadInst(lib, "Shift-R");

    // Load in double-argument math operations.
    LoadInst(lib, "Nand");
    LoadInst(lib, "Add");
    LoadInst(lib, "Sub");
    LoadInst(lib, "Mult");

    // @CAO For the next two, ideally if b==0, we should have the instruction return false...
    LoadInst(lib, "Div");
    LoadInst(lib, "Mod");

    // Conditionals
    LoadInst(lib, "Test-Equal");
    LoadInst(lib, "Test-NEqual");
    LoadInst(lib, "Test-Less");
    LoadInst(lib, "Test-AtStart");

    // Load in Jump operations  [we neeed to do better...  push and pop heads?]
    LoadInst(lib, "Jump");
    LoadInst(lib, "Jump-If0");
    LoadInst(lib, "Jump-IfN0");
    LoadInst(lib, "Bookmark");
    LoadInst(lib, "Set-Memory");
    // "Find-Label" ********** - Jumps the flow head to a complement label (?...) in its current memory.    

    // Juggle stack contents
    LoadInst(lib, "Val-Move");
    LoadInst(lib, "Val-Copy");
    LoadInst(lib, "Val-Delete");

    // Load in "Biological" instructions
    LoadInst(lib, "Build-Inst");
    // "Divide" **********      - Moves memory space 1 (?1) into its own hardware.  Needs callback!
    // "Get-Input" **********   - Needs callback
    // "Get-Output" **********  - Needs callback
    // "Inject" ?? **********   - Needs callback
  }

};

#endif
