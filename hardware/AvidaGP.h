//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This is a hardcoded CPU for Avida.
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
#include "../tools/string_utils.h"

#include "InstLib.h"

namespace emp {

  class AvidaGP {
  public:
    static constexpr size_t REGS = 16;
    static constexpr size_t INST_ARGS = 3;

    enum class InstID {
      Inc, Dec, Not, Add, Sub, Mult, Div, Mod,TestEqu, TestNEqu, TestLess,
      If, While, DoCount, Break, Scope, Define, Call, Label, Jump, JumpIf0, JumpIfN0,
      Push, Pop, Input, Output, CopyVal, ScopeReg, ResetReg,
      Unknown
    };

    // ScopeType is used for scopes that we need to do something special at the end.
    // Eg: WHILE needs to go back to beginning of loop; FUNCTION needs to return to call.
    enum class ScopeType { BASIC, WHILE, FUNCTION };

    struct Instruction {
      using id_t = InstID;
      using arg_t = int;              // All arguments are non-negative ints (indecies!)

      id_t id;
      emp::array<arg_t, 3> args;

      Instruction(InstID _id, int _a0, int _a1, int _a2)
	      : id(_id) { args[0] = _a0; args[1] = _a1; args[2] = _a2; }
      Instruction(const Instruction &) = default;
      Instruction(Instruction &&) = default;

      Instruction & operator=(const Instruction &) = default;
      Instruction & operator=(Instruction &&) = default;
    };

    using inst_t = Instruction;
    using genome_t = emp::vector<inst_t>;

  private:

    // Virtual CPU Components!
    genome_t genome;
    size_t inst_ptr;
    size_t errors;

    emp::array<double, REGS> regs;


    // This function gets run every time scope changed (if, while, scope instructions, etc.)
    // If we are moving to an outer scope (lower value) we need to close the scope we are in,
    // potentially continuing with a loop.
    bool UpdateScope(int scope, ScopeType type=ScopeType::BASIC) { return true; }

    // This function fast-forwards to the end of the specified scope.
    void BypassScope(int scope) { ; }

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

    void PushInst(InstID inst, int arg0=0, int arg1=0, int arg2=0) {
      genome.emplace_back(inst, arg0, arg1, arg2);
    }

    // Loading whole genomes.
    bool Load(std::istream & input);

    /// Process a specified instruction, provided by the caller.
    void ProcessInst(const inst_t & inst);

    /// Process the NEXT instruction pointed to be the instruction pointer
    void SingleProcess();

    /// Process the next SERIES of instructions, directed by the instruction pointer.
    void Process(size_t num_inst) { for (size_t i = 0; i < num_inst; i++) SingleProcess(); }

    static const InstLib<Instruction> & GetInstLib();
  };

  // // @CAO: This function should be incorporated into standard config functions.
  // bool AvidaGP::Load(std::istream & input) {
  //   // Load one line at a time and process each line.
  //   std::string cur_line;
  //
  //   // Loop until eof is hit
  //   while (!input.eof()) {
  //     std::getline(input, cur_line);         // Get the current input line.
  //     ProcessLine(cur_line, extras);         // Clean up line; act on aliases.
  //
  //     if (cur_line == "") continue;          // Skip empty lines.
  //
  //     std::string inst_name = emp::string_pop_word(cur_line);
  //     emp::right_justify(cur_line);
  //
  //     // Remaining info in cur_line is arguments to instructions.
  //     int arg1 = String2Arg(emp::string_pop_word(cur_line));
  //     int arg2 = String2Arg(emp::string_pop_word(cur_line));
  //     int arg3 = String2Arg(emp::string_pop_word(cur_line));
  //
  //     // @CAO Need to load from inst library!
  //
  //     return true;
  //   }
  // }

  void AvidaGP::ProcessInst(const inst_t & inst) {
    switch (inst.id) {
    case InstID::Inc: ++regs[inst.args[0]]; break;
    case InstID::Dec: --regs[inst.args[0]]; break;
    case InstID::Not: regs[inst.args[0]] = !regs[inst.args[0]]; break;
    case InstID::Add: regs[inst.args[2]] = regs[inst.args[0]] + regs[inst.args[1]]; break;
    case InstID::Sub: regs[inst.args[2]] = regs[inst.args[0]] - regs[inst.args[1]]; break;
    case InstID::Mult: regs[inst.args[2]] = regs[inst.args[0]] * regs[inst.args[1]]; break;

    case InstID::Div: {
      const double denom = regs[inst.args[1]];
      if (denom == 0.0) ++errors;
      else regs[inst.args[2]] = regs[inst.args[0]] / denom;
    }
      break;

    case InstID::Mod: {
      const double base = regs[inst.args[1]];
      if (base == 0.0) ++errors;
      else regs[inst.args[2]] = regs[inst.args[0]] / base;
    }
      break;

    case InstID::TestEqu: regs[inst.args[2]] = (regs[inst.args[0]] == regs[inst.args[1]]); break;
    case InstID::TestNEqu: regs[inst.args[2]] = (regs[inst.args[0]] != regs[inst.args[1]]); break;
    case InstID::TestLess: regs[inst.args[2]] = (regs[inst.args[0]] < regs[inst.args[1]]); break;

    case InstID::If: // args[0] = test, args[1] = scope
      if (UpdateScope(inst.args[1]) == false) break;     // If previous scope is unfinished, stop!
      if (!regs[inst.args[0]]) BypassScope(inst.args[1]);   // If test fails, move to scope end.
      break;                                          // Continue in current code.

    case InstID::While:
      // UpdateScope returns false if previous scope isn't finished (e.g., while needs to loop)
      if (UpdateScope(inst.args[1], ScopeType::WHILE) == false) break;
      if (!regs[inst.args[0]]) BypassScope(inst.args[1]);   // If test fails, move to scope end.
      break;

    case InstID::Break: BypassScope(inst.args[0]); break;
    case InstID::Scope: UpdateScope(inst.args[0]); break;

    case InstID::DoCount: break;
    case InstID::Define: break;
    case InstID::Call: break;
    case InstID::Label: break;
    case InstID::Jump: break;
    case InstID::JumpIf0: break;
    case InstID::JumpIfN0: break;
    case InstID::Push: break;
    case InstID::Pop: break;
    case InstID::Input: break;
    case InstID::Output: break;
    case InstID::CopyVal: break;
    case InstID::ScopeReg: break;
    case InstID::ResetReg: break;
    case InstID::Unknown:
    default:
      // This case should never happen!
      emp_assert(false, "Unknown instruction being exectuted!");
    };
  }

  void AvidaGP::SingleProcess() {
    if (inst_ptr >= genome.size()) inst_ptr = 0;
    ProcessInst( genome[inst_ptr] );
    inst_ptr++;
  }

  /// This static function can be used to access the generic AvidaGP instruction library.
  const InstLib<AvidaGP::Instruction> & AvidaGP::GetInstLib() {
    static InstLib<Instruction> inst_lib;
    static bool init = false;

    if (!init) {
      inst_lib.AddInst(InstID::Inc, "Inc", 1, "Increment value in register specified by Arg1");
      inst_lib.AddInst(InstID::Dec, "Dec", 1, "Decrement value in register specified by Arg1");
      inst_lib.AddInst(InstID::Not, "Not", 1, "Logically toggle value in register specified by Arg1");
      inst_lib.AddInst(InstID::Add, "Add", 3, "Arg3 = Arg1 + Arg2");
      inst_lib.AddInst(InstID::Sub, "Sub", 3, "Arg3 = Arg1 - Arg2");
      inst_lib.AddInst(InstID::Mult, "Mult", 3, "Arg3 = Arg1 * Arg2");
      inst_lib.AddInst(InstID::Div, "Div", 3, "Arg3 = Arg1 / Arg2");
      inst_lib.AddInst(InstID::Mod, "Mod", 3, "Arg3 = Arg1 % Arg2");
      inst_lib.AddInst(InstID::TestEqu, "TestEqu", 3, "Arg3 = (Arg1 == Arg2)");
      inst_lib.AddInst(InstID::TestNEqu, "TestNEqu", 3, "Arg3 = (Arg1 != Arg2)");
      inst_lib.AddInst(InstID::TestLess, "TestLess", 3, "Arg3 = (Arg1 < Arg2)");
      inst_lib.AddInst(InstID::If, "If", 2, "If Arg1 != 0, enter scope Arg2; else skip over scope");
      inst_lib.AddInst(InstID::While, "While", 2, "Until Arg1 != 0, repeat scope Arg2; else skip over scope");
      inst_lib.AddInst(InstID::DoCount, "DoCount", 3, "Repeat Arg1 times; set Arg2 to iteration and scope to Arg3");
      inst_lib.AddInst(InstID::Scope, "Scope", 1, "Set scope to Arg1");
      inst_lib.AddInst(InstID::Define, "Define", 2, "Build a function called Arg1 in scope Arg2");
      inst_lib.AddInst(InstID::Call, "Call", 1, "Call previously defined function called Arg1");
      inst_lib.AddInst(InstID::Label, "Label", 1, "Start a label called Arg1");
      inst_lib.AddInst(InstID::Jump, "Jump", 1, "Jump to label Arg1");
      inst_lib.AddInst(InstID::JumpIf0, "JumpIf0", 2, "If Arg2 == 0, jump to label Arg1");
      inst_lib.AddInst(InstID::JumpIfN0, "JumpIfN0", 2, "If Arg2 != 0, jump to label Arg1");
      inst_lib.AddInst(InstID::Push, "Push", 2, "Push register Arg1 onto stack Arg2");
      inst_lib.AddInst(InstID::Pop, "Pop", 2, "Pop stack Arg1 into register Arg2");
      inst_lib.AddInst(InstID::Input, "Input", 2, "Pull next value from input buffer Arg1 into register Arg2");
      inst_lib.AddInst(InstID::Output, "Output", 2, "Push reg Arg1 into output buffer Arg2");
      inst_lib.AddInst(InstID::CopyVal, "CopyVal", 2, "Copy reg Arg1 into reg Arg2");
      inst_lib.AddInst(InstID::ScopeReg, "ScopeReg", 1, "Backup reg Arg1; restore at end of scope");
      inst_lib.AddInst(InstID::ResetReg, "ResetReg", 1, "Restore Arg1 to its starting value");
      inst_lib.AddInst(InstID::Unknown, "Unknown", 0, "Error: Unknown instruction used.");

      for (char i = 0; i < AvidaGP::REGS; i++) {
        inst_lib.AddArg(to_string("Reg", 'A'+i), i);
      }

      init = true;
    }

    return inst_lib;
  }

}


#endif
