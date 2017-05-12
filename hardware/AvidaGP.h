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
//  * We should clean up how we handle scope; the root scope is zero, so the arg-based
//    scopes are 1-16 (or however many).  Right now we increment the value in various places
//    and should be more consistent.


#ifndef EMP_AVIDA_GP_H
#define EMP_AVIDA_GP_H

#include "../base/array.h"
#include "../base/vector.h"
#include "../tools/Random.h"
#include "../tools/string_utils.h"

#include "InstLib.h"

namespace emp {

  class AvidaGP {
  public:
    static constexpr size_t REGS = 16;
    static constexpr size_t INST_ARGS = 3;
    static constexpr size_t STACK_CAP = 16;

    enum class InstID {
      Inc, Dec, Not, SetReg, Add, Sub, Mult, Div, Mod, TestEqu, TestNEqu, TestLess,
      If, While, Countdown, Break, Scope, Define, Call,
      Push, Pop, Input, Output, CopyVal, ScopeReg,
      Unknown
    };

    // ScopeType is used for scopes that we need to do something special at the end.
    // Eg: LOOP needs to go back to beginning of loop; FUNCTION needs to return to call.
    enum class ScopeType { ROOT, BASIC, LOOP, FUNCTION };

    struct Instruction {
      using id_t = InstID;
      using arg_t = int;              // All arguments are non-negative ints (indecies!)

      id_t id;
      emp::array<arg_t, 3> args;

      Instruction(InstID _id=(InstID)0, int _a0=0, int _a1=0, int _a2=0)
	      : id(_id) { args[0] = _a0; args[1] = _a1; args[2] = _a2; }
      Instruction(const Instruction &) = default;
      Instruction(Instruction &&) = default;

      Instruction & operator=(const Instruction &) = default;
      Instruction & operator=(Instruction &&) = default;

      void Set(InstID _id, int _a0=0, int _a1=0, int _a2=0)
	      { id = _id; args[0] = _a0; args[1] = _a1; args[2] = _a2; }

    };

    struct ScopeInfo {
      size_t scope;
      ScopeType type;
      size_t start_pos;

      ScopeInfo() : scope(0), type(ScopeType::BASIC), start_pos(0) { ; }
      ScopeInfo(size_t _s, ScopeType _t, size_t _p) : scope(_s), type(_t), start_pos(_p) { ; }
    };

    struct RegBackup {
      size_t scope;
      size_t reg_id;
      double value;

      RegBackup() : scope(0), reg_id(0), value(0.0) { ; }
      RegBackup(size_t _s, size_t _r, double _v) : scope(_s), reg_id(_r), value(_v) { ; }
    };

    using inst_t = Instruction;
    using genome_t = emp::vector<inst_t>;

  private:

    // Virtual CPU Components!
    genome_t genome;
    emp::array<double, REGS> regs;
    emp::array<double, REGS> inputs;
    emp::array<double, REGS> outputs;
    emp::array< emp::vector<double>, REGS > stacks;
    emp::array< int, REGS> fun_starts;

    size_t inst_ptr;
    emp::vector<ScopeInfo> scope_stack;
    emp::vector<RegBackup> reg_stack;
    emp::vector<size_t> call_stack;

    size_t errors;

    double PopStack(size_t id) {
      if (stacks[id].size() == 0) return 0.0;
      double out = stacks[id].back();
      stacks[id].pop_back();
      return out;
    }

    void PushStack(size_t id, double value) {
      if (stacks[id].size() >= STACK_CAP) return;
      stacks[id].push_back(value);
    }

    size_t CurScope() const { return scope_stack.back().scope; }
    ScopeType CurScopeType() const { return scope_stack.back().type; }

    // Run every time we need to exit the current scope.
    void ExitScope() {
      emp_assert(scope_stack.size() > 1, CurScope());
      emp_assert(scope_stack.size() <= REGS, CurScope());

      // Restore any backed-up registers from this scope...
      while (reg_stack.size() && reg_stack.back().scope == CurScope()) {
        regs[reg_stack.back().reg_id] = reg_stack.back().value;
        reg_stack.pop_back();
      }

      // Remove the inner-most scope.
      scope_stack.pop_back();
    }

    // This function is run every time scope changed (if, while, scope instructions, etc.)
    // If we are moving to an outer scope (lower value) we need to close the scope we are in,
    // potentially continuing with a loop.
    bool UpdateScope(size_t new_scope, ScopeType type=ScopeType::BASIC) {
      const size_t cur_scope = CurScope();
      new_scope++;                           // Scopes are stored as one higher than regs (Outer is 0)
      // Test if we are entering a deeper scope.
      if (new_scope > cur_scope) {
        scope_stack.emplace_back(new_scope, type, inst_ptr);
        return true;
      }

      // Otherwise we are potentially exiting the current scope.  Loop back instead?
      if (CurScopeType() == ScopeType::LOOP) {
        inst_ptr = scope_stack.back().start_pos;  // Move back to the beginning of the loop.
        ExitScope();                              // Clear former scope
        ProcessInst( genome[inst_ptr] );          // Process loops start again.
        return false;                             // We did NOT enter the new scope.
      }

      // Or are we exiting a function?
      if (CurScopeType() == ScopeType::FUNCTION) {
        // @CAO Make sure we exit multiple scopes if needed to close the function...
        inst_ptr = call_stack.back();             // Return from the function call.
        if (inst_ptr >= genome.size()) ResetIP(); // Call may have occured at end of genome.
        else {
          call_stack.pop_back();                    // Clear the return position from the call stack.
          ExitScope();                              // Leave the function scope.
        }
        ProcessInst( genome[inst_ptr] );          // Process the new instruction instead.
        return false;                             // We did NOT enter the new scope.
      }

      // If we made it here, we must simply exit the current scope and test again.
      ExitScope();

      return UpdateScope(new_scope, type);
    }

    // This function fast-forwards to the end of the specified scope.
    // NOTE: Bypass scope always drops out of the innermost scope no matter the arg provided.
    void BypassScope(size_t scope) {
      scope++;                           // Scopes are stored as one higher than regs (Outer is 0)
      if (CurScope() < scope) return;    // Only continue if break is relevant for current scope.

      ExitScope();
      while (inst_ptr+1 < genome.size()) {
        inst_ptr++;
        const size_t test_scope = InstScope(genome[inst_ptr]);

        // If this instruction sets the scope AND it's outside the one we want to end, stop here!
        if (test_scope && test_scope <= scope) {
          inst_ptr--;
          break;
        }
      }
    }

  public:
    AvidaGP() : inst_ptr(0), errors(0) {
      scope_stack.emplace_back(0, ScopeType::ROOT, 0);  // Initial scope.
      Reset();
    }
    ~AvidaGP() { ; }

    /// Reset the entire CPU to a starting state, without a genome.
    void Reset() {
      genome.resize(0);  // Clear out genome
      ResetHardware();   // Reset the full hardware
    }

    /// Reset just the CPU hardware, but keep the genome.
    void ResetHardware() {
      // Initialize registers to their posision.  So Reg0 = 0 and Reg11 = 11.
      for (size_t i = 0; i < REGS; i++) {
        regs[i] = (double) i;
        inputs[i] = 0.0;
        outputs[i] = 0.0;
        stacks[i].resize(0);
        fun_starts[i] = -1;
      }
      errors = 0;
      ResetIP();
    }

    /// Reset the instruction pointer to the beginning of the genome AND reset scope.
    void ResetIP() {
      inst_ptr = 0;
      while (scope_stack.size() > 1) ExitScope();  // Forcibly exit all scopes except root.
      call_stack.resize(0);
    }

    // Accessors
    inst_t GetInst(size_t pos) const { return genome[pos]; }
    const genome_t & GetGenome() const { return genome; }
    double GetReg(size_t id) const { return regs[id]; }
    size_t GetIP() const { return inst_ptr; }
    double GetOutput(size_t id) const { return outputs[id]; }

    void SetInst(size_t pos, const inst_t & inst) { genome[pos] = inst; }
    void SetInst(size_t pos, InstID id, int a0=0, int a1=0, int a2=0) {
      genome[pos].Set(id, a0, a1, a2);
    }
    void SetGenome(const genome_t & g) { genome = g; }
    void SetInput(size_t input_id, double value) { inputs[input_id] = value; }
    void RandomizeInst(size_t pos, Random & rand) {
      SetInst(pos, (InstID) rand.GetUInt((uint32_t) InstID::Unknown),
              rand.GetInt(REGS), rand.GetInt(REGS), rand.GetInt(REGS) );
    }

    void PushInst(InstID id, int a0=0, int a1=0, int a2=0) { genome.emplace_back(id, a0, a1, a2); }
    void PushInst(const Instruction & inst) { genome.emplace_back(inst); }
    void PushRandom(Random & rand, const size_t count=1) {
      for (size_t i = 0; i < count; i++) {
        PushInst((InstID) rand.GetUInt((uint32_t) InstID::Unknown),
                rand.GetInt(REGS), rand.GetInt(REGS), rand.GetInt(REGS) );
      }
    }

    // Loading whole genomes.
    bool Load(std::istream & input);

    /// Process a specified instruction, provided by the caller.
    void ProcessInst(const inst_t & inst);

    /// Determine the scope associated with a particular instruction.
    size_t InstScope(const inst_t & inst);

    /// Process the NEXT instruction pointed to be the instruction pointer
    void SingleProcess();

    /// Process the next SERIES of instructions, directed by the instruction pointer.
    void Process(size_t num_inst) { for (size_t i = 0; i < num_inst; i++) SingleProcess(); }

    /// Print out a single instruction, with its arguments.
    void PrintInst(const inst_t & inst, std::ostream & os=std::cout);

    /// Print out this program.
    void PrintGenome(std::ostream & os=std::cout);

    /// Figure out which instruction is going to actually be run next SingleProcess()
    size_t PredictNextInst();

    /// Print out the state of the virtual CPU.
    void PrintState(std::ostream & os=std::cout);

    /// Trace the instructions being exectured, with full CPU details.
    void Trace(size_t num_inst) {
      for (size_t i = 0; i < num_inst; i++) { PrintState(); SingleProcess(); }
    }

    static const InstLib<Instruction> & GetInstLib();
  };

  void AvidaGP::ProcessInst(const inst_t & inst) {
    switch (inst.id) {
    case InstID::Inc: ++regs[inst.args[0]]; break;
    case InstID::Dec: --regs[inst.args[0]]; break;
    case InstID::Not: regs[inst.args[0]] = !regs[inst.args[0]]; break;
    case InstID::SetReg: regs[inst.args[0]] = inst.args[1]; break;
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
      if (UpdateScope(inst.args[1]) == false) break;      // If previous scope is unfinished, stop!
      if (!regs[inst.args[0]]) BypassScope(inst.args[1]); // If test fails, move to scope end.
      break;                                              // Continue in current code.

    case InstID::While:
      // UpdateScope returns false if previous scope isn't finished (e.g., while needs to loop)
      if (UpdateScope(inst.args[1], ScopeType::LOOP) == false) break;
      if (!regs[inst.args[0]]) BypassScope(inst.args[1]);   // If test fails, move to scope end.
      break;

    case InstID::Countdown:  // Same as while, but auto-decriments test each loop.
      // UpdateScope returns false if previous scope isn't finished (e.g., while needs to loop)
      if (UpdateScope(inst.args[1], ScopeType::LOOP) == false) break;
      if (!regs[inst.args[0]]) BypassScope(inst.args[1]);   // If test fails, move to scope end.
      else regs[inst.args[0]]--;
      break;

    case InstID::Break: BypassScope(inst.args[0]); break;
    case InstID::Scope: UpdateScope(inst.args[0]); break;

    case InstID::Define: {
        if (UpdateScope(inst.args[1]) == false) break;  // Update which scope we are in.
        fun_starts[inst.args[0]] = inst_ptr;            // Record where function should be exectuted.
        BypassScope(inst.args[1]);                      // Skip over the function definition for now.
      }
      break;
    case InstID::Call: {
        size_t def_pos = fun_starts[inst.args[0]];
        // Make sure function exists and is still in place.
        if (def_pos >= genome.size() || genome[def_pos].id != InstID::Define) break;
        // Go back into the function's original scope (call is in that scope)
        size_t fun_scope = genome[def_pos].args[1];
        if (UpdateScope(fun_scope, ScopeType::FUNCTION) == false) break;
        call_stack.push_back(inst_ptr+1);                 // Back up the call position
        inst_ptr = def_pos+1;                             // Jump to the function body (will adavance)
      }
      break;

    case InstID::Push: PushStack(inst.args[1], regs[inst.args[0]]); break;
    case InstID::Pop: regs[inst.args[1]] = PopStack(inst.args[0]); break;
    case InstID::Input: {
        size_t input_id = (size_t) regs[ inst.args[0] ];  // Grab ID from register.
        input_id = input_id & (REGS-1);                   // Mod ID into range.
        regs[inst.args[1]] = inputs[input_id];            // Set target reg to appropriate input.
      }
      break;
    case InstID::Output: {
        size_t output_id = (size_t) regs[ inst.args[1] ]; // Grab ID from register.
        output_id = output_id & (REGS-1);                 // Mod ID into range.
        outputs[output_id] = regs[inst.args[0]];          // Copy target reg to appropriate output.
      }
      break;
    case InstID::CopyVal: regs[inst.args[1]] = regs[inst.args[0]]; break;

    case InstID::ScopeReg:
      reg_stack.emplace_back(CurScope(), inst.args[0], regs[inst.args[0]]);
      break;

    case InstID::Unknown:
    default:
      // This case should never happen!
      emp_assert(false, "Unknown instruction being exectuted!");
    };
  }

  size_t AvidaGP::InstScope(const inst_t & inst) {
    switch (inst.id) {
    case InstID::If:
    case InstID::While:
    case InstID::Countdown:  // Same as while, but auto-decriments test each loop.
    case InstID::Define:
      return inst.args[1]+1;

    case InstID::Scope:
      return inst.args[0]+1;

    default:
      return 0;
    };
  }

  void AvidaGP::SingleProcess() {
    if (inst_ptr >= genome.size()) ResetIP();
    ProcessInst( genome[inst_ptr] );
    inst_ptr++;
  }

  void AvidaGP::PrintInst(const inst_t & inst, std::ostream & os) {
    const auto & inst_lib = GetInstLib();
    os << inst_lib.GetName(inst.id);
    const size_t num_args = inst_lib.GetNumArgs(inst.id);
    for (size_t i = 0; i < num_args; i++) {
      os << ' ' << inst.args[i];
    }
  }

  void AvidaGP::PrintGenome(std::ostream & os) {
    size_t cur_scope = 0;

    for (const inst_t & inst : genome) {
      size_t new_scope = InstScope(inst);

      if (new_scope) {
        if (new_scope == cur_scope) {
          for (size_t i = 0; i < cur_scope; i++) os << ' ';
          os << "----\n";
        }
        if (new_scope < cur_scope) {
          cur_scope = new_scope-1;
        }
      }

      for (size_t i = 0; i < cur_scope; i++) os << ' ';
      PrintInst(inst, os);
      if (new_scope) {
        if (new_scope > cur_scope) os << " --> ";
        cur_scope = new_scope;
      }
      os << '\n';
    }
  }

  size_t AvidaGP::PredictNextInst() {
    // Determine if we are changing scope.
    int new_scope = -1;
    if (inst_ptr >= genome.size()) new_scope = 0;
    else {
      size_t inst_scope = InstScope(genome[inst_ptr]);
      if (inst_scope) new_scope = inst_scope;
    }

    // If we are not changing scope OR we are going to a deeper scope, execute next!
    if (new_scope == -1 || new_scope > CurScope()) return inst_ptr;

    // If we are at the end of a loop, assume we will jump back to the beginning.
    if (CurScopeType() == ScopeType::LOOP) {
      return scope_stack.back().start_pos;
    }

    // If we are at the end of a function, assume we will jump back to the call.
    if (CurScopeType() == ScopeType::FUNCTION) {
      size_t next_pos = call_stack.back();
      if (next_pos >= genome.size()) next_pos = 0;
      return next_pos;
    }

    // If we have run past the end of the genome, we will start over.
    if (inst_ptr >= genome.size()) return 0;

    // Otherwise, we exit the scope normally.
    return inst_ptr;
  }

  void AvidaGP::PrintState(std::ostream & os) {
    size_t next_inst = PredictNextInst();

    os << " REGS: ";
    for (size_t i = 0; i < REGS; i++) os << "[" << regs[i] << "] ";
    os << "\n INPUTS: ";
    for (size_t i = 0; i < REGS; i++) os << "[" << inputs[i] << "] ";
    os << "\n OUTPUTS: ";
    for (size_t i = 0; i < REGS; i++) os << "[" << outputs[i] << "] ";
    os << std::endl;

    os << "IP:" << inst_ptr;
    if (inst_ptr != next_inst) os << "(-> " << next_inst << ")";
    os << " scope:" << CurScope()
       << " (";
    PrintInst(genome[next_inst], os);
    os << ")"
       << " errors: " << errors
       << std::endl;

    // @CAO Still need:
    // emp::array< emp::vector<double>, REGS > stacks;
    // emp::array< int, REGS> fun_starts;
    // emp::vector<RegBackup> reg_stack;
    // emp::vector<size_t> call_stack;
  }

  /// This static function can be used to access the generic AvidaGP instruction library.
  const InstLib<AvidaGP::Instruction> & AvidaGP::GetInstLib() {
    static InstLib<Instruction> inst_lib;
    static bool init = false;

    if (!init) {
      inst_lib.AddInst(InstID::Inc, "Inc", 1, "Increment value in reg Arg1");
      inst_lib.AddInst(InstID::Dec, "Dec", 1, "Decrement value in reg Arg1");
      inst_lib.AddInst(InstID::Not, "Not", 1, "Logically toggle value in reg Arg1");
      inst_lib.AddInst(InstID::SetReg, "SetReg", 2, "Set reg Arg1 to numerical value Arg2");
      inst_lib.AddInst(InstID::Add, "Add", 3, "regs: Arg3 = Arg1 + Arg2");
      inst_lib.AddInst(InstID::Sub, "Sub", 3, "regs: Arg3 = Arg1 - Arg2");
      inst_lib.AddInst(InstID::Mult, "Mult", 3, "regs: Arg3 = Arg1 * Arg2");
      inst_lib.AddInst(InstID::Div, "Div", 3, "regs: Arg3 = Arg1 / Arg2");
      inst_lib.AddInst(InstID::Mod, "Mod", 3, "regs: Arg3 = Arg1 % Arg2");
      inst_lib.AddInst(InstID::TestEqu, "TestEqu", 3, "regs: Arg3 = (Arg1 == Arg2)");
      inst_lib.AddInst(InstID::TestNEqu, "TestNEqu", 3, "regs: Arg3 = (Arg1 != Arg2)");
      inst_lib.AddInst(InstID::TestLess, "TestLess", 3, "regs: Arg3 = (Arg1 < Arg2)");
      inst_lib.AddInst(InstID::If, "If", 2, "If reg Arg1 != 0, scope -> Arg2; else skip scope");
      inst_lib.AddInst(InstID::While, "While", 2, "Until reg Arg1 != 0, repeat scope Arg2; else skip");
      inst_lib.AddInst(InstID::Countdown, "Countdown", 2, "Countdown reg Arg1 to zero; scope to Arg2");
      inst_lib.AddInst(InstID::Break, "Break", 1, "Break out of scope Arg1");
      inst_lib.AddInst(InstID::Scope, "Scope", 1, "Enter scope Arg1");
      inst_lib.AddInst(InstID::Define, "Define", 2, "Build function Arg1 in scope Arg2");
      inst_lib.AddInst(InstID::Call, "Call", 1, "Call previously defined function Arg1");
      inst_lib.AddInst(InstID::Push, "Push", 2, "Push reg Arg1 onto stack Arg2");
      inst_lib.AddInst(InstID::Pop, "Pop", 2, "Pop stack Arg1 into reg Arg2");
      inst_lib.AddInst(InstID::Input, "Input", 2, "Pull next value from input Arg1 into reg Arg2");
      inst_lib.AddInst(InstID::Output, "Output", 2, "Push reg Arg1 into output Arg2");
      inst_lib.AddInst(InstID::CopyVal, "CopyVal", 2, "Copy reg Arg1 into reg Arg2");
      inst_lib.AddInst(InstID::ScopeReg, "ScopeReg", 1, "Backup reg Arg1; restore at end of scope");
      inst_lib.AddInst(InstID::Unknown, "Unknown", 0, "Error: Unknown instruction used.");

      for (char i = 0; i < AvidaGP::REGS; i++) {
        inst_lib.AddArg(to_string(i), i);             // Args can be called by value
        inst_lib.AddArg(to_string("Reg", 'A'+i), i);  // ...or as a register.
      }

      init = true;
    }

    return inst_lib;
  }

}


#endif
