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
//    Define                          :  LABEL, SCOPE            (build a function / call it)
//    Call                            :  LABEL                   (build a function / call it)
//    Label, Jump, JumpIf0, JumpIfN0  :  LABEL, TEST
//    Push, Pop, Input, Output        :  VAL
//    CopyVal                         :  FROM, TO
//    ScopeReg                        :  REG                     (move var into current scope)
//    ResetReg                        :  REG
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

    enum class InstID {
      Inc, Dec, Not, Add, Sub, Mult, Div, Mod,TestEqu, TestNEqu, TestLess,
      If, While, DoCount, Break, Scope, Define, Call, Label, Jump, JumpIf0, JumpIfN0,
      Push, Pop, Input, Output, CopyVal, ScopeVar,
      Unknown
    };

    // ScopeType is used for scopes that we need to do something special at the end.
    // Eg: WHILE needs to go back to beginning of loop; FUNCTION needs to return to call.
    enum class ScopeType { BASIC, WHILE, FUNCTION };

    struct Instruction {
      using id_t = InstID;
      using arg_t = int;              // All arguments are non-negative ints (indecies!)

      id_t id;
      arg_t arg1;  arg_t arg2;  arg_t arg3;

      Instruction(InstID _id, int _a1, int _a2, int _a3)
	      : id(_id), arg1(_a1), arg2(_a2), arg3(_a3) { ; }
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

    int String2Arg(const std::string & str) {
      if (str == "") return 0;
      if (str == "RegA") return 0;
      if (str == "RegB") return 1;
      if (str == "RegC") return 2;
      if (str == "RegD") return 3;
      if (str == "RegE") return 4;
      if (str == "RegF") return 5;
      if (str == "RegG") return 6;
      if (str == "RegH") return 7;
      if (str == "RegI") return 8;
      if (str == "RegJ") return 9;
      if (str == "RegK") return 10;
      if (str == "RegL") return 11;
      if (str == "RegM") return 12;
      if (str == "RegN") return 13;
      if (str == "RegO") return 14;
      if (str == "RegP") return 15;
      emp_assert(false, str);
    }

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

    void PushInst(InstID inst, int arg1=0, int arg2=0, int arg3=0) {
      genome.emplace_back(inst, arg1, arg2, arg3);
    }

    // Loading whole genomes.
    bool Load(std::istream & input);

    /// Process a specified instruction, provided by the caller.
    void ProcessInst(const inst_t & inst);

    /// Process the NEXT instruction pointed to be the instruction pointer
    void SingleProcess();

    /// Process the next SERIES of instructions, directed by the instruction pointer.
    void Process(size_t num_inst) { for (size_t i = 0; i < num_inst; i++) SingleProcess(); }
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
    case InstID::Inc: ++regs[inst.arg1]; break;
    case InstID::Dec: --regs[inst.arg1]; break;
    case InstID::Not: regs[inst.arg1] = !regs[inst.arg1]; break;
    case InstID::Add: regs[inst.arg3] = regs[inst.arg1] + regs[inst.arg2]; break;
    case InstID::Sub: regs[inst.arg3] = regs[inst.arg1] - regs[inst.arg2]; break;
    case InstID::Mult: regs[inst.arg3] = regs[inst.arg1] * regs[inst.arg2]; break;

    case InstID::Div: {
      const double denom = regs[inst.arg2];
      if (denom == 0.0) ++errors;
      else regs[inst.arg3] = regs[inst.arg1] / denom;
    }
      break;

    case InstID::Mod: {
      const double base = regs[inst.arg2];
      if (base == 0.0) ++errors;
      else regs[inst.arg3] = regs[inst.arg1] / base;
    }
      break;

    case InstID::TestEqu: regs[inst.arg3] = (regs[inst.arg1] == regs[inst.arg2]); break;
    case InstID::TestNEqu: regs[inst.arg3] = (regs[inst.arg1] != regs[inst.arg2]); break;
    case InstID::TestLess: regs[inst.arg3] = (regs[inst.arg1] < regs[inst.arg2]); break;

    case InstID::If: // arg1 = test, arg2 = scope
      if (UpdateScope(inst.arg2) == false) break;     // If previous scope is unfinished, stop!
      if (!regs[inst.arg1]) BypassScope(inst.arg2);   // If test fails, move to scope end.
      break;                                          // Continue in current code.

    case InstID::While:
      // UpdateScope returns false if previous scope isn't finished (e.g., while needs to loop)
      if (UpdateScope(inst.arg2, ScopeType::WHILE) == false) break;
      if (!regs[inst.arg1]) BypassScope(inst.arg2);   // If test fails, move to scope end.
      break;

    case InstID::Break: BypassScope(inst.arg1); break;
    case InstID::Scope: UpdateScope(inst.arg1); break;

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
    case InstID::ScopeVar: break;
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

}


#endif
