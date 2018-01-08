/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file  Processor.h
 *  @brief Processes a LinearCode, updating the provided HardwareState
 *
 *  @todo Should we save a copy of the original genome?  (or create a new "memory" member)
 *  @todo We should clean up how we handle scope; the root scope is zero, so the arg-based
 *    scopes are 1-16 (or however many).  Right now we increment the value in various places
 *    and should be more consistent.
 *  @todo How should Avida-GP organisms take an action?  Options include sending ALL outputs and
 *    picking the maximum field; sending a single output and using its value; having specialized
 *    commands...
 */


#ifndef EMP_PROCESSOR_H
#define EMP_PROCESSOR_H

#include <fstream>
#include <iostream>
#include <map>

#include "../base/array.h"
#include "../base/Ptr.h"
#include "../base/vector.h"
#include "../tools/map_utils.h"
#include "../tools/Random.h"
#include "../tools/string_utils.h"

#include "InstLib.h"

namespace emp {

  template <typename HARDWARE>
  class Processor {
  public:
    using this_t = Processor<HARDWARE>;

    HARDWARE hw;

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
        ProcessInst( genome.sequence[inst_ptr] ); // Process loops start again.
        return false;                             // We did NOT enter the new scope.
      }

      // Or are we exiting a function?
      if (CurScopeType() == ScopeType::FUNCTION) {
        // @CAO Make sure we exit multiple scopes if needed to close the function...
        inst_ptr = call_stack.back();             // Return from the function call.
        if (inst_ptr >= genome.sequence.size()) { // Test if call occured at end of genome.
          ResetIP();                              // ...and reset to the begnning if so.
        } else {
          call_stack.pop_back();                  // Clear the return position from the call stack.
          ExitScope();                            // Leave the function scope.
        }
        ProcessInst( genome.sequence[inst_ptr] ); // Process the new instruction instead.
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
      while (inst_ptr+1 < genome.sequence.size()) {
        inst_ptr++;
        const size_t test_scope = InstScope(genome.sequence[inst_ptr]);

        // If this instruction sets the scope AND it's outside the one we want to end, stop here!
        if (test_scope && test_scope <= scope) {
          inst_ptr--;
          break;
        }
      }
    }

  public:
    /// Create a new AvidaGP seeding it with a genome.
    Processor<HARDWARE>(const genome_t & in_genome)
      : genome(in_genome), regs(), inputs(), outputs(), stacks(), fun_starts()
      , inst_ptr(0), scope_stack(), reg_stack(), call_stack(), errors(0), traits()
    {
      scope_stack.emplace_back(0, ScopeType::ROOT, 0);  // Initial scope.
      for (size_t i = 0; i < CPU_SIZE; i++) {
        regs[i] = (double) i;
        fun_starts[i] = -1;
      }
    }

    /// Create a default AvidaGP (no genome sequence, default instruction set)
    Processor<HARDWARE>() : Processor<HARDWARE>(Genome(DefaultInstLib())) { ; }

    /// Create an AvidaGP with a specified instruction set (but no genome sequence)
    Processor<HARDWARE>(Ptr<const inst_lib_t> inst_lib) : Processor<HARDWARE>(Genome(inst_lib)) { ; }
    Processor<HARDWARE>(const inst_lib_t & inst_lib) : Processor<HARDWARE>(Genome(&inst_lib)) { ; }

    /// Copy constructor
    Processor<HARDWARE>(const Processor<HARDWARE> &) = default;

    /// Move constructor
    Processor<HARDWARE>(Processor<HARDWARE> &&) = default;

    /// Destructor
    virtual ~Processor<HARDWARE>() { ; }

    bool operator<(const this_t & other) const {
      return genome < other.genome;
    }

    /// Reset the entire CPU to a starting state, without a genome.
    void Reset() {
      genome.sequence.resize(0);  // Clear out genome
      traits.resize(0);           // Clear out traits
      ResetHardware();            // Reset the full hardware
    }

    /// Reset just the CPU hardware, but keep the genome and traits.
    virtual void ResetHardware() {
      // Initialize registers to their posision.  So Reg0 = 0 and Reg11 = 11.
      for (size_t i = 0; i < CPU_SIZE; i++) {
        regs[i] = (double) i;
        inputs.clear();
        outputs.clear();
        stacks[i].resize(0);
        fun_starts[i] = -1;
      }
      inst_ptr = 0;           // Move IP back to beginning
      scope_stack.resize(1);  // Reset to outermost scope.
      reg_stack.resize(0);    // Clear saved registers.
      call_stack.resize(0);   // Clear call history.
      errors = 0;             // Clear all errors.
     }

    /// Reset the instruction pointer to the beginning of the genome AND reset scope.
    void ResetIP() {
      inst_ptr = 0;
      while (scope_stack.size() > 1) ExitScope();  // Forcibly exit all scopes except root.
      // Restore all remaining backed-up registers (likely backed up in outer-most scope).
      while (reg_stack.size()) {
        regs[reg_stack.back().reg_id] = reg_stack.back().value;
        reg_stack.pop_back();
      }
      call_stack.resize(0);
    }

    // Accessors
    Ptr<const inst_lib_t> GetInstLib() const { return genome.inst_lib; }
    inst_t GetInst(size_t pos) const { return genome.sequence[pos]; }
    const genome_t & GetGenome() const { return genome; }
    const size_t GetSize() const { return genome.sequence.size(); }
    double GetReg(size_t id) const { return regs[id]; }
    double GetInput(int id) const { return Find(inputs, id, 0.0); }
    const std::unordered_map<int,double> & GetInputs() const { return inputs; }
    size_t GetNumInputs() const { return inputs.size(); }
    double GetOutput(int id) const { return Find(outputs, id, 0.0); }
    const std::unordered_map<int,double> & GetOutputs() const { return outputs; }
    size_t GetNumOutputs() const { return outputs.size(); }
    const stack_t & GetStack(size_t id) const { return stacks[id]; }
    int GetFunStart(size_t id) const { return fun_starts[id]; }
    size_t GetIP() const { return inst_ptr; }
    emp::vector<ScopeInfo> GetScopeStack() const { return scope_stack; }
    size_t CurScope() const { return scope_stack.back().scope; }
    ScopeType CurScopeType() const { return scope_stack.back().type; }
    ScopeType GetScopeType(size_t id) { return genome.inst_lib->GetScopeType(id); }
    emp::vector<RegBackup> GetRegStack() const { return reg_stack; }
    emp::vector<size_t> GetCallStack() const { return call_stack; }
    size_t GetNumErrors() const { return errors; }
    double GetTrait(size_t id) const { return traits[id]; }
    const emp::vector<double> &  GetTraits() { return traits; }
    size_t GetNumTraits() const { return traits.size(); }

    void SetInst(size_t pos, const inst_t & inst) { genome.sequence[pos] = inst; }
    void SetInst(size_t pos, size_t id, size_t a0=0, size_t a1=0, size_t a2=0) {
      genome.sequence[pos].Set(id, a0, a1, a2);
    }
    void SetGenome(const genome_t & g) { genome = g; }
    void SetReg(size_t id, double val) { regs[id] = val; }
    void SetInput(int input_id, double value) { inputs[input_id] = value; }
    void SetInputs(const std::unordered_map<int,double> & vals) { inputs = vals; }
    void SetInputs(std::unordered_map<int,double> && vals) { inputs = std::move(vals); }
    void SetOutput(int output_id, double value) { outputs[output_id] = value; }
    void SetOutputs(const std::unordered_map<int,double> & vals) { outputs = vals; }
    void SetOutputs(std::unordered_map<int,double> && vals) { outputs = std::move(vals); }
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
    void SetFunStart(size_t id, int value) { fun_starts[id] = value; }
    void SetIP(size_t pos) { inst_ptr = pos; }
    void PushRegInfo(size_t scope_id, size_t reg_id) {
      reg_stack.emplace_back(scope_id, reg_id, regs[reg_id]);
    }
    void PushCallInfo(size_t pos) { call_stack.push_back(pos); }
    void IncErrors() { errors++; }
    void SetTrait(size_t id, double val) {
      if (id >= traits.size()) traits.resize(id+1, 0.0);
      traits[id] = val;
    }
    void PushTrait(double val) { traits.push_back(val); }

    inst_t GetRandomInst(Random & rand) {
      return inst_t(rand.GetUInt(genome.inst_lib->GetSize()),
                    rand.GetUInt(CPU_SIZE), rand.GetUInt(CPU_SIZE), rand.GetUInt(CPU_SIZE));
    }

    void RandomizeInst(size_t pos, Random & rand) { SetInst(pos, GetRandomInst(rand) ); }

    void PushInst(size_t id, size_t a0=0, size_t a1=0, size_t a2=0) {
      genome.sequence.emplace_back(id, a0, a1, a2);
    }
    void PushInst(const std::string & name, size_t a0=0, size_t a1=0, size_t a2=0) {
      size_t id = genome.inst_lib->GetID(name);
      genome.sequence.emplace_back(id, a0, a1, a2);
    }
    void PushInst(const Instruction & inst) { genome.sequence.emplace_back(inst); }
    void PushInst(Instruction && inst) { genome.sequence.emplace_back(inst); }
    void PushRandom(Random & rand, const size_t count=1) {
      for (size_t i = 0; i < count; i++) {
        PushInst(GetRandomInst(rand));
      }
    }

    // Loading whole genomes.
    bool Load(std::istream & input);

    /// Process a specified instruction, provided by the caller.
    void ProcessInst(const inst_t & inst) { genome.inst_lib->ProcessInst(*this, inst); }

    /// Determine the scope associated with a particular instruction.
    size_t InstScope(const inst_t & inst) const;

    /// Process the NEXT instruction pointed to be the instruction pointer
    void SingleProcess() {
      emp_assert(genome.sequence.size() > 0);  // A genome must exist to be processed.
      if (inst_ptr >= genome.sequence.size()) ResetIP();
      genome.inst_lib->ProcessInst(*this, genome.sequence[inst_ptr]);
      inst_ptr++;
    }

    /// Process the next SERIES of instructions, directed by the instruction pointer.
    void Process(size_t num_inst) { for (size_t i = 0; i < num_inst; i++) SingleProcess(); }

    /// Print out a single instruction, with its arguments.
    void PrintInst(const inst_t & inst, std::ostream & os=std::cout) const;

    /// Print out this program.
    void PrintGenome(std::ostream & os=std::cout) const;
    void PrintGenome(const std::string & filename) const;

    /// Figure out which instruction is going to actually be run next SingleProcess()
    size_t PredictNextInst() const;

    /// Print out the state of the virtual CPU.
    void PrintState(std::ostream & os=std::cout) const;

    /// Trace the instructions being exectured, with full CPU details.
    void Trace(size_t num_inst, std::ostream & os=std::cout) {
      for (size_t i = 0; i < num_inst; i++) { PrintState(os); SingleProcess(); }
    }
    void Trace(size_t num_inst, const std::string & filename) {
      std::ofstream of(filename);
      Trace(num_inst, of);
      of.close();
    }


    /// Instructions
    static void Inst_Inc(this_t & hw, const inst_t & inst) { ++hw.regs[inst.args[0]]; }
    static void Inst_Dec(this_t & hw, const inst_t & inst) { --hw.regs[inst.args[0]]; }
    static void Inst_Not(this_t & hw, const inst_t & inst) { hw.regs[inst.args[0]] = (hw.regs[inst.args[0]] == 0.0); }
    static void Inst_SetReg(this_t & hw, const inst_t & inst) { hw.regs[inst.args[0]] = (double) inst.args[1]; }
    static void Inst_Add(this_t & hw, const inst_t & inst) { hw.regs[inst.args[2]] = hw.regs[inst.args[0]] + hw.regs[inst.args[1]]; }
    static void Inst_Sub(this_t & hw, const inst_t & inst) { hw.regs[inst.args[2]] = hw.regs[inst.args[0]] - hw.regs[inst.args[1]]; }
    static void Inst_Mult(this_t & hw, const inst_t & inst) { hw.regs[inst.args[2]] = hw.regs[inst.args[0]] * hw.regs[inst.args[1]]; }

    static void Inst_Div(this_t & hw, const inst_t & inst) {
      const double denom = hw.regs[inst.args[1]];
      if (denom == 0.0) ++hw.errors;
      else hw.regs[inst.args[2]] = hw.regs[inst.args[0]] / denom;
    }

    static void Inst_Mod(this_t & hw, const inst_t & inst) {
      const double base = hw.regs[inst.args[1]];
      if (base == 0.0) ++hw.errors;
      else hw.regs[inst.args[2]] = hw.regs[inst.args[0]] / base;
    }

    static void Inst_TestEqu(this_t & hw, const inst_t & inst) { hw.regs[inst.args[2]] = (hw.regs[inst.args[0]] == hw.regs[inst.args[1]]); }
    static void Inst_TestNEqu(this_t & hw, const inst_t & inst) { hw.regs[inst.args[2]] = (hw.regs[inst.args[0]] != hw.regs[inst.args[1]]); }
    static void Inst_TestLess(this_t & hw, const inst_t & inst) { hw.regs[inst.args[2]] = (hw.regs[inst.args[0]] < hw.regs[inst.args[1]]); }

    static void Inst_If(this_t & hw, const inst_t & inst) { // args[0] = test, args[1] = scope
      if (hw.UpdateScope(inst.args[1]) == false) return;      // If previous scope is unfinished, stop!
      if (hw.regs[inst.args[0]] == 0.0) hw.BypassScope(inst.args[1]); // If test fails, move to scope end.
    }

    static void Inst_While(this_t & hw, const inst_t & inst) {
      // UpdateScope returns false if previous scope isn't finished (e.g., while needs to loop)
      if (hw.UpdateScope(inst.args[1], ScopeType::LOOP) == false) return;
      if (hw.regs[inst.args[0]] == 0.0) hw.BypassScope(inst.args[1]); // If test fails, move to scope end.
    }

    static void Inst_Countdown(this_t & hw, const inst_t & inst) {  // Same as while, but auto-decriments test each loop.
      // UpdateScope returns false if previous scope isn't finished (e.g., while needs to loop)
      if (hw.UpdateScope(inst.args[1], ScopeType::LOOP) == false) return;
      if (hw.regs[inst.args[0]] == 0.0) hw.BypassScope(inst.args[1]);   // If test fails, move to scope end.
      else hw.regs[inst.args[0]]--;
    }

    static void Inst_Break(this_t & hw, const inst_t & inst) { hw.BypassScope(inst.args[0]); }
    static void Inst_Scope(this_t & hw, const inst_t & inst) { hw.UpdateScope(inst.args[0]); }

    static void Inst_Define(this_t & hw, const inst_t & inst) {
      if (hw.UpdateScope(inst.args[1]) == false) return; // Update which scope we are in.
      hw.fun_starts[inst.args[0]] = (int) hw.inst_ptr;     // Record where function should be exectuted.
      hw.BypassScope(inst.args[1]);                     // Skip over the function definition for now.
    }

    static void Inst_Call(this_t & hw, const inst_t & inst) {
      // Make sure function exists and is still in place.
      size_t def_pos = (size_t) hw.fun_starts[inst.args[0]];
      if (def_pos >= hw.genome.sequence.size()
          || hw.GetScopeType(hw.genome.sequence[def_pos].id) != ScopeType::FUNCTION) return;

      // Go back into the function's original scope (call is in that scope)
      size_t fun_scope = hw.genome.sequence[def_pos].args[1];
      if (hw.UpdateScope(fun_scope, ScopeType::FUNCTION) == false) return;
      hw.call_stack.push_back(hw.inst_ptr+1);        // Back up the call position
      hw.inst_ptr = def_pos+1;                       // Jump to the function body (will adavance)
    }

    static void Inst_Push(this_t & hw, const inst_t & inst) { hw.PushStack(inst.args[1], hw.regs[inst.args[0]]); }
    static void Inst_Pop(this_t & hw, const inst_t & inst) { hw.regs[inst.args[1]] = hw.PopStack(inst.args[0]); }

    static void Inst_Input(this_t & hw, const inst_t & inst) {
      // Determine the input ID and grab it if it exists; if not, return 0.0
      int input_id = (int) hw.regs[ inst.args[0] ];
      hw.regs[inst.args[1]] = Find(hw.inputs, input_id, 0.0);
    }

    static void Inst_Output(this_t & hw, const inst_t & inst) {
      // Save the date in the target reg to the specified output position.
      int output_id = (int) hw.regs[ inst.args[1] ];  // Grab ID from register.
      hw.outputs[output_id] = hw.regs[inst.args[0]];     // Copy target reg to appropriate output.
    }

    static void Inst_CopyVal(this_t & hw, const inst_t & inst) { hw.regs[inst.args[1]] = hw.regs[inst.args[0]]; }

    static void Inst_ScopeReg(this_t & hw, const inst_t & inst) {
      hw.reg_stack.emplace_back(hw.CurScope(), inst.args[0], hw.regs[inst.args[0]]);
    }

    static const inst_lib_t & DefaultInstLib();
  };

  template <typename HARDWARE>
  size_t Processor<HARDWARE>::InstScope(const inst_t & inst) const {
    if (genome.inst_lib->GetScopeType(inst.id) == ScopeType::NONE) return 0;
    return inst.args[ genome.inst_lib->GetScopeArg(inst.id) ] + 1;
  }

  template <typename HARDWARE>
  void Processor<HARDWARE>::PrintInst(const inst_t & inst, std::ostream & os) const {
    os << genome.inst_lib->GetName(inst.id);
    const size_t num_args = genome.inst_lib->GetNumArgs(inst.id);
    for (size_t i = 0; i < num_args; i++) {
      os << ' ' << inst.args[i];
    }
  }

  template <typename HARDWARE>
  void Processor<HARDWARE>::PrintGenome(std::ostream & os) const {
    size_t cur_scope = 0;

    for (const inst_t & inst : genome.sequence) {
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

  template <typename HARDWARE>
  void Processor<HARDWARE>::PrintGenome(const std::string & filename) const {
    std::ofstream of(filename);
    PrintGenome(of);
    of.close();
  }

  template <typename HARDWARE>
  size_t Processor<HARDWARE>::PredictNextInst() const {
    // Determine if we are changing scope.
    size_t new_scope = CPU_SIZE+1;  // Default to invalid scope.
    if (inst_ptr >= genome.sequence.size()) new_scope = 0;
    else {
      size_t inst_scope = InstScope(genome.sequence[inst_ptr]);
      if (inst_scope) new_scope = inst_scope;
    }

    // If we are not changing scope OR we are going to a deeper scope, execute next!
    if (new_scope > CPU_SIZE || new_scope > CurScope()) return inst_ptr;

    // If we are at the end of a loop, assume we will jump back to the beginning.
    if (CurScopeType() == ScopeType::LOOP) {
      return scope_stack.back().start_pos;
    }

    // If we are at the end of a function, assume we will jump back to the call.
    if (CurScopeType() == ScopeType::FUNCTION) {
      size_t next_pos = call_stack.back();
      if (next_pos >= genome.sequence.size()) next_pos = 0;
      return next_pos;
    }

    // If we have run past the end of the genome, we will start over.
    if (inst_ptr >= genome.sequence.size()) return 0;

    // Otherwise, we exit the scope normally.
    return inst_ptr;
  }

  template <typename HARDWARE>
  void Processor<HARDWARE>::PrintState(std::ostream & os) const {
    size_t next_inst = PredictNextInst();

    os << " REGS: ";
    for (size_t i = 0; i < CPU_SIZE; i++) os << "[" << regs[i] << "] ";
    os << "\n INPUTS: ";
    // for (size_t i = 0; i < CPU_SIZE; i++) os << "[" << Find(inputs, (int)i, 0.0) << "] ";
    for (auto & x : inputs) os << "[" << x.first << "," << x.second << "] ";
    os << "\n OUTPUTS: ";
    //for (size_t i = 0; i < CPU_SIZE; i++) os << "[" << Find(outputs, (int)i, 0.0) << "] ";
    for (auto & x : outputs) os << "[" << x.first << "," << x.second << "] ";
    os << std::endl;

    os << "IP:" << inst_ptr;
    if (inst_ptr != next_inst) os << "(-> " << next_inst << ")";
    os << " scope:" << CurScope()
       << " (";
    if (next_inst < genome.sequence.size()) { // For interpreter mode
        PrintInst(genome.sequence[next_inst], os);
    }
    os << ")"
       << " errors: " << errors
       << std::endl;

    // @CAO Still need:
    // emp::array< emp::vector<double>, CPU_SIZE > stacks;
    // emp::array< int, CPU_SIZE> fun_starts;
    // emp::vector<RegBackup> reg_stack;
    // emp::vector<size_t> call_stack;
  }

  /// This static function can be used to access the generic AvidaGP instruction library.
  template <typename HARDWARE>
  const InstLib<Processor<HARDWARE>> & Processor<HARDWARE>::DefaultInstLib() {
    static inst_lib_t inst_lib;

    if (inst_lib.GetSize() == 0) {
      inst_lib.AddInst("Inc", Inst_Inc, 1, "Increment value in reg Arg1");
      inst_lib.AddInst("Dec", Inst_Dec, 1, "Decrement value in reg Arg1");
      inst_lib.AddInst("Not", Inst_Not, 1, "Logically toggle value in reg Arg1");
      inst_lib.AddInst("SetReg", Inst_SetReg, 2, "Set reg Arg1 to numerical value Arg2");
      inst_lib.AddInst("Add", Inst_Add, 3, "regs: Arg3 = Arg1 + Arg2");
      inst_lib.AddInst("Sub", Inst_Sub, 3, "regs: Arg3 = Arg1 - Arg2");
      inst_lib.AddInst("Mult", Inst_Mult, 3, "regs: Arg3 = Arg1 * Arg2");
      inst_lib.AddInst("Div", Inst_Div, 3, "regs: Arg3 = Arg1 / Arg2");
      inst_lib.AddInst("Mod", Inst_Mod, 3, "regs: Arg3 = Arg1 % Arg2");
      inst_lib.AddInst("TestEqu", Inst_TestEqu, 3, "regs: Arg3 = (Arg1 == Arg2)");
      inst_lib.AddInst("TestNEqu", Inst_TestNEqu, 3, "regs: Arg3 = (Arg1 != Arg2)");
      inst_lib.AddInst("TestLess", Inst_TestLess, 3, "regs: Arg3 = (Arg1 < Arg2)");
      inst_lib.AddInst("If", Inst_If, 2, "If reg Arg1 != 0, scope -> Arg2; else skip scope", ScopeType::BASIC, 1);
      inst_lib.AddInst("While", Inst_While, 2, "Until reg Arg1 != 0, repeat scope Arg2; else skip", ScopeType::LOOP, 1);
      inst_lib.AddInst("Countdown", Inst_Countdown, 2, "Countdown reg Arg1 to zero; scope to Arg2", ScopeType::LOOP, 1);
      inst_lib.AddInst("Break", Inst_Break, 1, "Break out of scope Arg1");
      inst_lib.AddInst("Scope", Inst_Scope, 1, "Enter scope Arg1", ScopeType::BASIC, 0);
      inst_lib.AddInst("Define", Inst_Define, 2, "Build function Arg1 in scope Arg2", ScopeType::FUNCTION, 1);
      inst_lib.AddInst("Call", Inst_Call, 1, "Call previously defined function Arg1");
      inst_lib.AddInst("Push", Inst_Push, 2, "Push reg Arg1 onto stack Arg2");
      inst_lib.AddInst("Pop", Inst_Pop, 2, "Pop stack Arg1 into reg Arg2");
      inst_lib.AddInst("Input", Inst_Input, 2, "Pull next value from input Arg1 into reg Arg2");
      inst_lib.AddInst("Output", Inst_Output, 2, "Push reg Arg1 into output Arg2");
      inst_lib.AddInst("CopyVal", Inst_CopyVal, 2, "Copy reg Arg1 into reg Arg2");
      inst_lib.AddInst("ScopeReg", Inst_ScopeReg, 1, "Backup reg Arg1; restore at end of scope");

      for (size_t i = 0; i < CPU_SIZE; i++) {
        inst_lib.AddArg(to_string((int)i), i);                   // Args can be called by value
        inst_lib.AddArg(to_string("Reg", 'A'+(char)i), i);  // ...or as a register.
      }
    }

    return inst_lib;
  }
}


#endif
