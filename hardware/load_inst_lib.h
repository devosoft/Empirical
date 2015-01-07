#ifndef EMP_LOAD_INST_LIB_H
#define EMP_LOAD_INST_LIB_H

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file contains tools to load instructions sets.
//

#include <functional>
using namespace std::placeholders;

#include "../tools/errors.h"
#include "../tools/string_utils.h"
#include "HardwareCPU.h"
#include "InstLib.h"

namespace emp {


  template <int CPU_SCALE=8, int STACK_SIZE=16>
  bool LoadInst(InstLib<HardwareCPU<CPU_SCALE, STACK_SIZE>, Instruction> & lib,
                std::string inst_info)
  {
    // Determine the instruction name.
    compress_whitespace(inst_info);
    std::string full_name = string_pop_word(inst_info);  // Full name of instruction;  eg: Nop:0
    std::string name_info = full_name;                   // Extra info at end of name; eg: 0 
    std::string name_base = string_pop(name_info, ':');  // Base name of instruction;  eg: Nop

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
    if (name_base == "Nop") {
      int mod_id = std::stoi(name_info) % CPU_SCALE;
      if (mod_id < 0) mod_id += CPU_SCALE;
      lib.Add(full_name, "No-operation instruction; usable as modifier.",
              std::bind(&HardwareCPU<>::Inst_Nop, _1), mod_id, 1);
    }

    // Check for single-argument math operations.
    else if (name_base == "Inc") {
      lib.AddMath(full_name, "Increment top of ?Stack-B? by one", [](int a){return a+1;});
    }
    else if (name_base == "Dec") {
      lib.AddMath(full_name, "Decrement top of ?Stack-B? by one", [](int a){return a-1;});
    }
    else if (name_base == "Shift-L") {
      lib.AddMath(full_name, "Shift bits of top of ?Stack-B? left by one", [](int a){return a<<1;});
    }
    else if (name_base == "Shift-R") {
      lib.AddMath(full_name, "Shift bits of top of ?Stack-B? right by one", [](int a){return a>>1;});
    }

    // Load in double-argument math operations.
    else if (name_base == "Nand") {
      lib.AddMath(full_name, "Compute: ?Stack-B?-top nand ?Stack-C?-top and push result to ?Stack-B?",
                  [](int a, int b){ return ~(a&b); });
    }
    else if (name_base == "Add") {
      lib.AddMath(full_name, "Compute: ?Stack-B?-top plus ?Stack-C?-top and push result to ?Stack-B?",
                  [](int a, int b){ return a+b; });
    }
    else if (name_base == "Sub") {
      lib.AddMath(full_name, "Compute: ?Stack-B?-top minus ?Stack-C?-top and push result to ?Stack-B?",
                  [](int a, int b){ return a-b; });
    }
    else if (name_base == "Mult") {
      lib.AddMath(full_name, "Compute: ?Stack-B?-top times ?Stack-C?-top and push result to ?Stack-B?",
                  [](int a, int b){ return a*b; });
    }

    // @CAO For the next two, ideally if b==0, we should have the instruction return false...
    else if (name_base == "Div") {
      lib.AddMath(full_name, "Compute: ?Stack-B?-top div ?Stack-C?-top and push result to ?Stack-B?",
                  [](int a, int b){ return b?a/b:0; });
    }
    else if (name_base == "Mod") {
      lib.AddMath(full_name, "Compute: ?Stack-B?-top mod ?Stack-C?-top and push result to ?Stack-B?",
                  [](int a, int b){ return b?a%b:0; });
    }

    // Conditionals
    else if (name_base == "Test-Equal") {
      lib.AddTest(full_name,
                  "Test if ?Stack-B?-top == ?Stack-C?-top and push result to ?Stack-D?",
                  [](int a, int b){ return a==b; });
    }
    else if (name_base == "Test-NEqual") {
      lib.AddTest(full_name,
                  "Test if ?Stack-B?-top != ?Stack-C?-top and push result to ?Stack-D?",
                  [](int a, int b){ return a!=b; });
    }
    else if (name_base == "Test-Less") {
      lib.AddTest(full_name,
                  "Test if ?Stack-B?-top < ?Stack-C?-top and push result to ?Stack-D?",
                  [](int a, int b){ return a<b; });
    }
    else if (name_base == "Test-AtStart") {
      lib.Add(full_name, "Test if ?Head-Read? is at mem position 0 and push result to ?Stack-D?",
              std::mem_fn(&HardwareCPU<>::Inst_TestAtStart));
    }

    // Load in Jump operations  [we neeed to do better...  push and pop heads?]
    else if (name_base == "Jump") {
      lib.Add(full_name, "Move ?Head-IP? to position of ?Head-Flow?",
              std::mem_fn(&HardwareCPU<>::Inst_MoveHeadToHead<0, 3>));
    }
    else if (name_base == "Jump-If0") {
      lib.Add(full_name, "Move ?Head-IP? to position of ?Head-Flow? only if ?Stack-D?-top == 0",
              std::bind(&HardwareCPU<>::Inst_MoveHeadToHeadIf<0,3,3>, _1, [](int a){ return a==0; }));
    }
    else if (name_base == "Jump-IfN0") {
      lib.Add(full_name, "Move ?Head-IP? to position of ?Head-Flow? only if ?Stack-D?-top != 0",
              std::bind(&HardwareCPU<>::Inst_MoveHeadToHeadIf<0,3,3>, _1, [](int a){ return a!=0; }));
    }
    else if (name_base == "Bookmark") {
      lib.Add(full_name, "Move ?Head-Flow? to position of ?Head-IP?",
              std::mem_fn(&HardwareCPU<>::Inst_MoveHeadToHead<3, 0>));
    }
    else if (name_base == "Set-Memory") {
      lib.Add(full_name, "Move ?Head-Write? to position 0 in ?Memory-1?",
              std::mem_fn(&HardwareCPU<>::Inst_MoveHeadToMem<2, 1>));
    }
    // "Find-Label" ********** - Jumps the flow head to a complement label (?...) in its current memory.    

    // Juggle stack contents
    else if (name_base == "Val-Move") {
      lib.Add(full_name, "Pop ?Stack-B? and push value onto ?Stack-C?",
              std::bind(&HardwareCPU<>::Inst_1I_Math<1,1>, _1, [](int a){return a;}));
    }
    else if (name_base == "Val-Copy") {
      lib.Add(full_name, "Copy top of ?Stack-B? onto ?Stack-C?",
              std::bind(&HardwareCPU<>::Inst_1I_Math<1,1,false>, _1, [](int a){return a;}));
    }
    else if (name_base == "Val-Delete") {
      lib.Add(full_name, "Pop ?Stack-B? and discard value",
              std::mem_fn(&HardwareCPU<>::Inst_ValDelete));
    }

    // Check for "Biological" instructions
    else if (name_base == "Build-Inst") {
      lib.Add(full_name, "Add new instruction to end of ?Memory-1? copied from ?Head-Read?",
              std::mem_fn(&HardwareCPU<>::Inst_BuildInst));
    }
    // "Divide" **********      - Moves memory space 1 (?1) into its own hardware.  Needs callback!
    // "Get-Input" **********   - Needs callback
    // "Get-Output" **********  - Needs callback
    // "Inject" ?? **********   - Needs callback


    // If we made it this far, we FAILED to find this instruction!
    else {
      std::stringstream ss;
      ss << "Failed to load instruction '" << name_base << "'.  Ignoring.";
      NotifyError(ss.str());

      return false;
    }

    return true;
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
    LoadInst(lib, "Inc");
    LoadInst(lib, "Inc:v2");
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
