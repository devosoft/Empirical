/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file  AvidaGP.h
 *  @brief A simple, efficient hardware for use as a basis for organisms in Avida or AvidaGP.
 *
 *  @todo We should clean up how we handle scope; the root scope is zero, so the arg-based
 *    scopes are 1-16 (or however many).  Right now we increment the value in various places
 *    and should be more consistent.
 */


#ifndef EMP_AVIDA_HARDWARE_H
#define EMP_AVIDA_HARDWARE_H

#include <fstream>
#include <iostream>
#include <map>

#include "../base/array.h"
#include "../base/Ptr.h"
#include "../base/vector.h"
#include "../tools/map_utils.h"
#include "../tools/Random.h"
#include "../tools/string_utils.h"

namespace emp {

  /// Core hardware for building an avida CPU.
  /// @param CPU_SIZE determines the number of registers, stacks, etc. to use.

  template <size_t CPU_SIZE=16>
  class AvidaHardware {
  public:
    using this_t = AvidaHardware<CPU_SIZE>;
    using stack_t = emp::vector<double>;

    /// ScopeType is used for scopes that we need to do something special at the end.
    /// Eg: LOOP needs to go back to beginning of loop; FUNCTION needs to return to call.
    enum class ScopeType { NONE=0, ROOT, BASIC, LOOP, FUNCTION };

    /// As different scopes are stepped through, this class provides information about each one.
    struct ScopeInfo {
      size_t scope;        ///< What is the depth of this scope?
      ScopeType type;      ///< What type is this scope? (ROOT, BASIC, LOOP, or FUNCTION)
      size_t start_pos;    ///< Where in the code did this scope start?

      ScopeInfo() : scope(0), type(ScopeType::BASIC), start_pos(0) { ; }
      ScopeInfo(size_t _s, ScopeType _t, size_t _p) : scope(_s), type(_t), start_pos(_p) { ; }
    };

    /// Information about a register that is backed up, to be restored when current scope is exited.
    struct RegBackup {
      size_t scope;   ///< What scope should this register be restored in?
      size_t reg_id;  ///< Which register is this?
      double value;   ///< What value is being backed up?

      RegBackup() : scope(0), reg_id(0), value(0.0) { ; }
      RegBackup(size_t _s, size_t _r, double _v) : scope(_s), reg_id(_r), value(_v) { ; }
    };

    // Hardware Components
    emp::array<double, CPU_SIZE> regs;       ///< Registers used in the hardware.
    std::unordered_map<int, double> inputs;  ///< Map of all available inputs (position -> value)
    std::unordered_map<int, double> outputs; ///< Map of all outputs (position -> value)
    emp::array< stack_t, CPU_SIZE > stacks;  ///< Stacks for long-term storage.
    emp::array< int, CPU_SIZE > fun_starts;  ///< Postions where functions being in genome.

    size_t inst_ptr;                         ///< Which code position should be executed next?
    emp::vector<ScopeInfo> scope_stack;      ///< What scopes are we nested in?
    emp::vector<RegBackup> reg_stack;        ///< What registers have been backed up?
    emp::vector<size_t> call_stack;          ///< What function calls have to be returned from?

    size_t error_count;                      ///< How many errors have occurred?

    /// A simple way of recording which traits a CPU has demonstrated, and at what qaulity.
    emp::vector<double> traits;


  public:
    /// Create a new AvidaGP seeding it with a genome.
    AvidaHardware<CPU_SIZE>()
      : regs(), inputs(), outputs(), stacks(), fun_starts(), inst_ptr(0)
      , scope_stack(), reg_stack(), call_stack(), error_count(0), traits()
    {
      scope_stack.emplace_back(0, ScopeType::ROOT, 0);  // Initial scope.
      for (size_t i = 0; i < CPU_SIZE; i++) {
        regs[i] = (double) i;
        fun_starts[i] = -1;
      }
    }

    /// Copy constructor
    AvidaHardware<CPU_SIZE>(const AvidaHardware<CPU_SIZE> &) = default;

    /// Move constructor
    AvidaHardware<CPU_SIZE>(AvidaHardware<CPU_SIZE> &&) = default;

    /// Destructor
    virtual ~AvidaHardware<CPU_SIZE>() { ; }

    /// Run every time we need to exit the current scope.
    void ExitScope() {
      emp_assert(scope_stack.size() > 1, CurScope());
      emp_assert(scope_stack.size() <= CPU_SIZE, CurScope());

      // Restore any backed-up registers from this scope...
      while (reg_stack.size() && reg_stack.back().scope == CurScope()) {
        regs[reg_stack.back().reg_id] = reg_stack.back().value;
        reg_stack.pop_back();
      }

      // Remove the inner-most scope.
      scope_stack.pop_back();
    }

    //// Reset the CPU hardware to default state.
    virtual void Reset() {
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
      error_count = 0;        // Clear all errors.
      traits.resize(0);           // Clear out traits
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
    emp::vector<RegBackup> GetRegStack() const { return reg_stack; }
    emp::vector<size_t> GetCallStack() const { return call_stack; }
    size_t GetNumErrors() const { return error_count; }
    double GetTrait(size_t id) const { return traits[id]; }
    const emp::vector<double> &  GetTraits() { return traits; }
    size_t GetNumTraits() const { return traits.size(); }

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
      if (stacks[id].size() >= CPU_SIZE) return;
      stacks[id].push_back(value);
    }
    void SetFunStart(size_t id, int value) { fun_starts[id] = value; }
    void SetIP(size_t pos) { inst_ptr = pos; }
    void PushRegInfo(size_t scope_id, size_t reg_id) {
      reg_stack.emplace_back(scope_id, reg_id, regs[reg_id]);
    }
    void PushCallInfo(size_t pos) { call_stack.push_back(pos); }
    void IncErrors() { error_count++; }
    void SetTrait(size_t id, double val) {
      if (id >= traits.size()) traits.resize(id+1, 0.0);
      traits[id] = val;
    }
    void PushTrait(double val) { traits.push_back(val); }

    /// Print out the state of the virtual CPU.
    void PrintState(std::ostream & os=std::cout) const;


    // Instruction helpers.
    void IncReg(size_t reg_id) { ++regs[reg_id]; }
    void DecReg(size_t reg_id) { --regs[reg_id]; }
    void NotReg(size_t reg_id) { regs[reg_id] = (regs[reg_id] == 0.0); }
    void AddRegs(size_t reg0_id, size_t reg1_id, size_t reg2_id) {
      regs[reg2_id] = regs[reg0_id] + regs[reg1_id];
    }
    void SubRegs(size_t reg0_id, size_t reg1_id, size_t reg2_id) {
      regs[reg2_id] = regs[reg0_id] - regs[reg1_id];
    }
    void MultRegs(size_t reg0_id, size_t reg1_id, size_t reg2_id) {
      regs[reg2_id] = regs[reg0_id] * regs[reg1_id];
    }
    void DivRegs(size_t reg0_id, size_t reg1_id, size_t reg2_id) {
      const double denom = regs[reg1_id];
      if (denom == 0.0) ++error_count;
      else regs[reg2_id] = regs[reg0_id] / denom;
    }
    void ModRegs(size_t reg0_id, size_t reg1_id, size_t reg2_id) {
      const double base = regs[reg1_id];
      if (base == 0.0) ++error_count;
      else regs[reg2_id] = regs[reg0_id] / base;
    }

    void RegTestEqu(size_t reg0_id, size_t reg1_id, size_t reg2_id) {
      regs[reg2_id] = (regs[reg0_id] == regs[reg1_id]);
    }
    void RegTestNEqu(size_t reg0_id, size_t reg1_id, size_t reg2_id) {
      regs[reg2_id] = (regs[reg0_id] != regs[reg1_id]);
    }
    void RegTestLess(size_t reg0_id, size_t reg1_id, size_t reg2_id) {
      regs[reg2_id] = (regs[reg0_id] < regs[reg1_id]);
    }

  };

}


#endif
