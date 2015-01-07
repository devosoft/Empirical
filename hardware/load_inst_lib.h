#ifndef EMP_LOAD_INST_LIB_H
#define EMP_LOAD_INST_LIB_H

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file contains tools to load instructions sets.
//

#include <functional>
#include <map>
#include <string>

using namespace std::placeholders;

#include "../tools/errors.h"
#include "../tools/functions.h"
#include "../tools/string_utils.h"
#include "HardwareCPU.h"
#include "InstLib.h"

namespace emp {

  template <int CPU_SCALE=8, int STACK_SIZE=16>
  const std::map<std::string, InstDefinition<HardwareCPU<CPU_SCALE, STACK_SIZE> > > &
  GetInstDefs(InstLib<HardwareCPU<CPU_SCALE, STACK_SIZE>, Instruction> & lib)
  {
    (void) lib;  // Used to auto-fill tempaltes (and handle overloading in the future)
    typedef HardwareCPU<CPU_SCALE, STACK_SIZE> HARDWARE_TYPE;

    // This function will produce a unique defs map.  If we already have it, just return it.
    static std::map<std::string, InstDefinition<HARDWARE_TYPE> > defs;
    if (defs.size()) return defs;

    defs["Nop"]        = { "No-operation instruction; usable as modifier.",
                           std::bind(&HARDWARE_TYPE::Inst_Nop, _1) };

    // Add single-argument math operations.
    defs["Inc"]        = { "Increment top of ?Stack-B? by one",
                           HARDWARE_TYPE::BuildMathInst([](int a){return a+1;}) };

    defs["Dec"]        = { "Decrement top of ?Stack-B? by one",
                           HARDWARE_TYPE::BuildMathInst([](int a){return a-1;}) };

    defs["Shift-L"]    = { "Shift bits of top of ?Stack-B? left by one",
                           HARDWARE_TYPE::BuildMathInst([](int a){return a<<1;}) };

    defs["Shift-R"]    = { "Shift bits of top of ?Stack-B? right by one",
                           HARDWARE_TYPE::BuildMathInst([](int a){return a>>1;}) };
    
    // Add double-argument math operations.
    defs["Nand"]       = { "Compute: ?Stack-B?-top nand ?Stack-C?-top and push result to ?Stack-B?",
                           HARDWARE_TYPE::BuildMathInst([](int a, int b){ return ~(a&b); }) };

    defs["Add"]        = { "Compute: ?Stack-B?-top plus ?Stack-C?-top and push result to ?Stack-B?",
                           HARDWARE_TYPE::BuildMathInst([](int a, int b){ return a+b; }) };

    defs["Sub"]        = { "Compute: ?Stack-B?-top minus ?Stack-C?-top and push result to ?Stack-B?",
                           HARDWARE_TYPE::BuildMathInst([](int a, int b){ return a-b; }) };

    defs["Mult"]       = { "Compute: ?Stack-B?-top times ?Stack-C?-top and push result to ?Stack-B?",
                           HARDWARE_TYPE::BuildMathInst([](int a, int b){ return a*b; }) };
    
    // @CAO For the next two, ideally if b==0, we should have the instruction return false...
    defs["Div"]        = { "Compute: ?Stack-B?-top div ?Stack-C?-top and push result to ?Stack-B?",
                           HARDWARE_TYPE::BuildMathInst([](int a, int b){ return b?a/b:0; }) };

    defs["Mod"]        = { "Compute: ?Stack-B?-top mod ?Stack-C?-top and push result to ?Stack-B?",
                           HARDWARE_TYPE::BuildMathInst([](int a, int b){ return b?a%b:0; }) };

    // Conditionals
    defs["Test-Equal"] = { "Test if ?Stack-B?-top == ?Stack-C?-top and push result to ?Stack-D?",
                           HARDWARE_TYPE::BuildTestInst([](int a, int b){ return a==b; }) };

    defs["Test-NEqual"] = { "Test if ?Stack-B?-top != ?Stack-C?-top and push result to ?Stack-D?",
                            HARDWARE_TYPE::BuildTestInst([](int a, int b){ return a!=b; }) };

    defs["Test-Less"]  = { "Test if ?Stack-B?-top < ?Stack-C?-top and push result to ?Stack-D?",
                           HARDWARE_TYPE::BuildTestInst([](int a, int b){ return a<b; }) };

    defs["Test-AtStart"] = { "Test if ?Head-Read? is at mem position 0 and push result to ?Stack-D?",
                             std::mem_fn(&HARDWARE_TYPE::Inst_TestAtStart) };
    
    // Load in Jump operations  [we neeed to do better...  push and pop heads?]
    defs["Jump"]       = { "Move ?Head-IP? to position of ?Head-Flow?",
                             std::mem_fn(&HARDWARE_TYPE::template Inst_MoveHeadToHead<0, 3>) };
    defs["Jump-If0"]   = { "Move ?Head-IP? to position of ?Head-Flow? only if ?Stack-D?-top == 0",
                           std::bind(&HARDWARE_TYPE::template Inst_MoveHeadToHeadIf<0,3,3>,
                                     _1, [](int a){ return a==0; }) };
    defs["Jump-IfN0"]  = { "Move ?Head-IP? to position of ?Head-Flow? only if ?Stack-D?-top != 0",
                           std::bind(&HARDWARE_TYPE::template Inst_MoveHeadToHeadIf<0,3,3>,
                                     _1, [](int a){ return a!=0; }) };
    defs["Bookmark"]   = { "Move ?Head-Flow? to position of ?Head-IP?",
                           std::mem_fn(&HARDWARE_TYPE::template Inst_MoveHeadToHead<3, 0>) };
    defs["Set-Memory"] = { "Move ?Head-Write? to position 0 in ?Memory-1?",
                           std::mem_fn(&HARDWARE_TYPE::template Inst_MoveHeadToMem<2, 1>) };
    // "Find-Label" ********** - Jumps the flow head to a complement label (?...) in current memory.
      
    // Juggle stack contents
    defs["Val-Move"]   = { "Pop ?Stack-B? and push value onto ?Stack-C?",
                           std::bind(&HARDWARE_TYPE::template Inst_1I_Math<1,1>, _1, [](int a){return a;}) };
    defs["Val-Copy"]   = { "Copy top of ?Stack-B? onto ?Stack-C?",
                           std::bind(&HARDWARE_TYPE::template Inst_1I_Math<1,1,false>, _1, [](int a){return a;}) };
    defs["Val-Delete"] = { "Pop ?Stack-B? and discard value",
                           std::mem_fn(&HARDWARE_TYPE::Inst_ValDelete) };

      // Check for "Biological" instructions
    defs["Build-Inst"] = { "Add new instruction to end of ?Memory-1? copied from ?Head-Read?",
                           std::mem_fn(&HARDWARE_TYPE::Inst_BuildInst) };
    // "Divide" **********      - Moves memory space 1 (?1) into its own hardware.  Needs callback!
    // "Get-Input" **********   - Needs callback
    // "Get-Output" **********  - Needs callback
    // "Inject" ?? **********   - Needs callback

    return defs;
  }
                 

  

  // The following function will load an instruction specification into an instruction library.
  // The library and the descriptor string should be passed in.  Some instructions can have a
  // more detailed specification as part of the name.  For example, "Nop:3" is a No-operation
  // instruction that is associated with component 3 when used as an argument.  All instructions
  // can also have additional information placed after a ':' that is ignored, but attached to
  // the name.  So "Inc:MyFavoriteInst" will behave the same as "Inc".  Likewise "Nop:3:v2" will
  // behave the same as "Nop:3".

  template <int CPU_SCALE=8, int STACK_SIZE=16>
  bool LoadInst(InstLib<HardwareCPU<CPU_SCALE, STACK_SIZE>, Instruction> & lib,
                std::string inst_info)
  {
    typedef HardwareCPU<CPU_SCALE, STACK_SIZE> HARDWARE_TYPE;

    // Determine the instruction name.
    compress_whitespace(inst_info);
    std::string full_name = string_pop_word(inst_info);  // Full name of instruction  eg: Nop:3:v2
    std::string name_info = full_name;                   // Extra info at end of name eg: 3:v2
    std::string name_base = string_pop(name_info, ':');  // Base name of instruction  eg: Nop
    std::string name_spec = string_get(name_info, ':');  // First info after ':'      eg: 3
    int mod_id = name_spec.size() ? emp::mod( std::stoi(name_spec), CPU_SCALE ) : -1;

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

    auto inst_defs = GetInstDefs(lib);
    if (inst_defs.find(name_base) == inst_defs.end()) {
      std::stringstream ss;
      ss << "Failed to find instruction '" << name_base << "'.  Ignoring.";
      NotifyError(ss.str());

      return false;
    }
    
    auto cur_def = inst_defs[name_base];

    lib.Add(full_name, cur_def.desc, cur_def.call, mod_id);

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
