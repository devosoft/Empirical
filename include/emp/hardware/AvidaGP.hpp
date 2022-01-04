/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017-2021.
 *
 *  @file AvidaGP.hpp
 *  @brief This is a simple, efficient CPU for and applied version of Avida.
 *
 *  @todo Should we save a copy of the original genome?  (or create a new "memory" member)
 *  @todo We should clean up how we handle scope; the root scope is zero, so the arg-based
 *    scopes are 1-16 (or however many).  Right now we increment the value in various places
 *    and should be more consistent.
 *  @todo How should Avida-GP organisms take an action?  Options include sending ALL outputs and
 *    picking the maximum field; sending a single output and using its value; having specialized
 *    commands...
 */

#ifndef EMP_HARDWARE_AVIDAGP_HPP_INCLUDE
#define EMP_HARDWARE_AVIDAGP_HPP_INCLUDE


#include <fstream>
#include <iostream>
#include <map>

#include "../base/array.hpp"
#include "../base/Ptr.hpp"
#include "../base/vector.hpp"
#include "../datastructs/map_utils.hpp"
#include "../datastructs/vector_utils.hpp"
#include "../io/File.hpp"
#include "../math/Random.hpp"
#include "../tools/string_utils.hpp"

#include "AvidaCPU_InstLib.hpp"
#include "Genome.hpp"

namespace emp {

  template <typename HARDWARE>
  class AvidaCPU_Base {
  public:
    static constexpr size_t CPU_SIZE = 16;   // Num arg values (for regs, stacks, functions, etc)
    static constexpr size_t INST_ARGS = 3;   // Max num args per instruction.
    static constexpr size_t STACK_CAP = 16;  // Max size for stacks.

    struct Instruction;

    using this_t = AvidaCPU_Base<HARDWARE>;
    using hardware_t = HARDWARE;
    using inst_t = Instruction;
    using arg_t = size_t;                                               // Args are indecies.
    using inst_lib_t = AvidaCPU_InstLib<hardware_t, arg_t, INST_ARGS>;
    using genome_t = Genome<Instruction, inst_lib_t>;

    using stack_t = emp::vector<double>;
    using arg_set_t = emp::array<arg_t, INST_ARGS>;

    struct Instruction {
      size_t id;
      arg_set_t args;

      Instruction(size_t _id=0, size_t a0=0, size_t a1=0, size_t a2=0)
        : id(_id), args() { args[0] = a0; args[1] = a1; args[2] = a2; }
      Instruction(const Instruction &) = default;
      Instruction(Instruction &&) = default;

      Instruction & operator=(const Instruction &) = default;
      Instruction & operator=(Instruction &&) = default;
      bool operator<(const Instruction & in) const {
          return std::tie(id, args) < std::tie(in.id, in.args);
      }
      bool operator==(const Instruction & in) const { return id == in.id && args == in.args; }
      bool operator!=(const Instruction & in) const { return !(*this == in); }
      bool operator>(const Instruction & in) const { return in < *this; }
      bool operator>=(const Instruction & in) const { return !(*this < in); }
      bool operator<=(const Instruction & in) const { return !(in < *this); }

      void Set(size_t _id, size_t _a0=0, size_t _a1=0, size_t _a2=0)
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

    // Virtual CPU Components!
    genome_t genome;
    emp::array<double, CPU_SIZE> regs;       // Registers used in the hardware.
    std::unordered_map<int, double> inputs;  // Map of all available inputs (position -> value)
    std::unordered_map<int, double> outputs; // Map of all outputs (position -> value)
    emp::array< stack_t, CPU_SIZE > stacks;  // Stacks for long-term storage.
    emp::array< int, CPU_SIZE > fun_starts;  // Postions where functions being in genome.

    size_t inst_ptr;
    emp::vector<ScopeInfo> scope_stack;
    emp::vector<RegBackup> reg_stack;
    emp::vector<size_t> call_stack;

    size_t errors;

    // A simple way of recording which traits a CPU has demonstrated, and at what qaulity.
    emp::vector<double> traits;

    // Run every time we need to exit the current scope.
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
        if (inst_ptr >= genome.GetSize()) {          // Test if call occured at end of genome.
          ResetIP();                              // ...and reset to the begnning if so.
        } else {
          call_stack.pop_back();                  // Clear the return position from the call stack.
          ExitScope();                            // Leave the function scope.
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
      while (inst_ptr+1 < genome.GetSize()) {
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
    /// Create a new AvidaCPU seeding it with a genome.
    AvidaCPU_Base(const genome_t & in_genome)
      : genome(in_genome), regs(), inputs(), outputs(), stacks(), fun_starts()
      , inst_ptr(0), scope_stack(), reg_stack(), call_stack(), errors(0), traits()
    {
      scope_stack.emplace_back(0, ScopeType::ROOT, 0);  // Initial scope.
      for (size_t i = 0; i < CPU_SIZE; i++) {
        regs[i] = (double) i;
        fun_starts[i] = -1;
      }
    }

    /// Create a default AvidaCPU (no genome sequence, default instruction set)
    AvidaCPU_Base() : AvidaCPU_Base(genome_t(inst_lib_t::DefaultInstLib())) { ; }

    /// Create an AvidaCPU with a specified instruction set (but no genome sequence)
    AvidaCPU_Base(Ptr<const inst_lib_t> inst_lib) : AvidaCPU_Base(genome_t(inst_lib)) { ; }
    AvidaCPU_Base(const inst_lib_t & inst_lib) : AvidaCPU_Base(genome_t(&inst_lib)) { ; }

    /// Copy constructor
    AvidaCPU_Base(const AvidaCPU_Base &) = default;

    /// Move constructor
    AvidaCPU_Base(AvidaCPU_Base &&) = default;

    /// Destructor
    virtual ~AvidaCPU_Base() { ; }

    bool operator<(const this_t & other) const {
      return genome < other.genome;
    }

    bool operator!=(const this_t & other) const {
      return genome != other.genome;
    }

    /// Reset the entire CPU to a starting state, without a genome.
    void Reset() {
      genome.resize(0);    // Clear out genome
      traits.resize(0);    // Clear out traits
      ResetHardware();     // Reset the full hardware
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
    Ptr<const inst_lib_t> GetInstLib() const { return genome.GetInstLib(); }
    inst_t GetInst(size_t pos) const { return genome[pos]; }
    inst_t& operator[](size_t pos) {return genome[pos]; } // Alias for compatability with tools
    const genome_t & GetGenome() const { return genome; }
    size_t GetSize() const { return genome.GetSize(); }
    size_t size() const { return GetSize(); } // Alias for compatability with tools
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
    ScopeType GetScopeType(size_t id) { return GetInstLib()->GetScopeType(id); }
    emp::vector<RegBackup> GetRegStack() const { return reg_stack; }
    emp::vector<size_t> GetCallStack() const { return call_stack; }
    size_t GetNumErrors() const { return errors; }
    double GetTrait(size_t id) const { return traits[id]; }
    const emp::vector<double> &  GetTraits() { return traits; }
    size_t GetNumTraits() const { return traits.size(); }

    void SetInst(size_t pos, const inst_t & inst) { genome[pos] = inst; }
    void SetInst(size_t pos, size_t id, size_t a0=0, size_t a1=0, size_t a2=0) {
      genome[pos].Set(id, a0, a1, a2);
    }
    void SetGenome(const genome_t & g) { genome = g; }
    void SetReg(size_t id, double val) { regs[id] = val; }
    void SetInput(int input_id, double value) { inputs[input_id] = value; }
    void SetInputs(const std::unordered_map<int,double> & vals) { inputs = vals; }
    void SetInputs(std::unordered_map<int,double> && vals) { inputs = std::move(vals); }
    void SetInputs(const emp::vector<double> & vals) { inputs = emp::ToUMap<int,double>(vals); }
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
      return inst_t(rand.GetUInt(GetInstLib()->GetSize()),
                    rand.GetUInt(CPU_SIZE), rand.GetUInt(CPU_SIZE), rand.GetUInt(CPU_SIZE));
    }

    void RandomizeInst(size_t pos, Random & rand) { SetInst(pos, GetRandomInst(rand) ); }

    /// Add a new instruction to the end of the genome, by ID and args.
    void PushInst(size_t id, size_t a0=0, size_t a1=0, size_t a2=0) {
      genome.emplace_back(id, a0, a1, a2);
    }

    /// Add a new instruction to the end of the genome, by NAME and args.
    void PushInst(const std::string & name, size_t a0=0, size_t a1=0, size_t a2=0) {
      PushInst(GetInstLib()->GetID(name), a0, a1, a2);
    }

    /// Add a specified new instruction to the end of the genome.
    void PushInst(const Instruction & inst) { genome.emplace_back(inst); }

    /// Add multiple copies of a specified instruction to the end of the genome.
    void PushInst(const Instruction & inst, size_t count) {
      genome.reserve(genome.size() + count);
      for (size_t i = 0; i < count; i++) genome.emplace_back(inst);
    }

    /// Add one or more default instructions to the end of the genome.
    void PushDefaultInst(size_t count=1) { PushInst( Instruction(0), count ); }

    void PushInstString(std::string info) {
      size_t id = GetInstLib()->GetID( string_pop(info) );
      size_t arg1 = info.size() ? from_string<size_t>(string_pop(info)) : 0;
      size_t arg2 = info.size() ? from_string<size_t>(string_pop(info)) : 0;
      size_t arg3 = info.size() ? from_string<size_t>(string_pop(info)) : 0;
      PushInst(id, arg1, arg2, arg3);
    }
    void PushRandom(Random & random, const size_t count=1) {
      for (size_t i = 0; i < count; i++) {
        PushInst(GetRandomInst(random));
      }
    }

    // Loading whole genomes.
    bool Load(std::istream & input);
    bool Load(const std::string & filename) { std::ifstream is(filename); return Load(is); }

    /// Process a specified instruction, provided by the caller.
    void ProcessInst(const inst_t & inst) { GetInstLib()->ProcessInst(ToPtr(this), inst); }

    /// Determine the scope associated with a particular instruction.
    size_t InstScope(const inst_t & inst) const;

    /// Process the NEXT instruction pointed to be the instruction pointer
    void SingleProcess() {
      emp_assert(genome.GetSize() > 0);  // A genome must exist to be processed.
      if (inst_ptr >= genome.GetSize()) ResetIP();
      GetInstLib()->ProcessInst(ToPtr(this), genome[inst_ptr]);
      inst_ptr++;
    }

    /// Process the next SERIES of instructions, directed by the instruction pointer.
    void Process(size_t num_inst) { for (size_t i = 0; i < num_inst; i++) SingleProcess(); }

    /// Print out a single instruction, with its arguments.
    void PrintInst(const inst_t & inst, std::ostream & os=std::cout) const;

    /// Print out this program.
    void PrintGenome(std::ostream & os=std::cout) const;
    void PrintGenome(const std::string & filename) const;

    /// Print out a short version of the genome as a single string.
    void PrintSymbols(std::ostream & os=std::cout) const;

    /// Convert the current state to a string.
    std::string ToString() const {
      std::stringstream ss;
      PrintSymbols(ss);
      return ss.str();
    }

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

  };

  template <typename HARDWARE>
  bool AvidaCPU_Base<HARDWARE>::Load(std::istream & input) {
    File file(input);
    file.RemoveComments("--");  // Remove all comments beginning with -- (including --> and ----)
    file.CompressWhitespace();  // Trim down remaining whitespace.
    file.Apply( [this](std::string & info){ PushInstString(info); } );
    return true;
  }

  template <typename HARDWARE>
  size_t AvidaCPU_Base<HARDWARE>::InstScope(const typename AvidaCPU_Base<HARDWARE>::inst_t & inst) const {
    if (GetInstLib()->GetScopeType(inst.id) == ScopeType::NONE) return 0;
    return inst.args[ GetInstLib()->GetScopeArg(inst.id) ] + 1;
  }

  template <typename HARDWARE>
  void AvidaCPU_Base<HARDWARE>::PrintInst(const inst_t & inst, std::ostream & os) const {
    os << GetInstLib()->GetName(inst.id);
    const size_t num_args = GetInstLib()->GetNumArgs(inst.id);
    for (size_t i = 0; i < num_args; i++) {
      os << ' ' << inst.args[i];
    }
  }

  template <typename HARDWARE>
  void AvidaCPU_Base<HARDWARE>::PrintGenome(std::ostream & os) const {
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

  template <typename HARDWARE>
  void AvidaCPU_Base<HARDWARE>::PrintGenome(const std::string & filename) const {
    std::ofstream of(filename);
    PrintGenome(of);
    of.close();
  }

  template <typename HARDWARE>
  void AvidaCPU_Base<HARDWARE>::PrintSymbols(std::ostream & os) const {
    // Example output: t(12)u()b(A5C)m(8)

    for (const inst_t & inst : genome) {
      os << GetInstLib()->GetSymbol(inst.id) << "[";
      const size_t num_args = GetInstLib()->GetNumArgs(inst.id);
      for (size_t i = 0; i < num_args; i++) {
        size_t arg_id = inst.args[i];
        if (arg_id < 10) os << arg_id;
        else os << ('A' + (char) (arg_id - 10));
      }
      os << "]";
    }
    os << '\n';
  }

  template <typename HARDWARE>
  size_t AvidaCPU_Base<HARDWARE>::PredictNextInst() const {
    // Determine if we are changing scope.
    size_t new_scope = CPU_SIZE+1;  // Default to invalid scope.
    if (inst_ptr >= genome.GetSize()) new_scope = 0;
    else {
      size_t inst_scope = InstScope(genome[inst_ptr]);
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
      if (next_pos >= genome.GetSize()) next_pos = 0;
      return next_pos;
    }

    // If we have run past the end of the genome, we will start over.
    if (inst_ptr >= genome.GetSize()) return 0;

    // Otherwise, we exit the scope normally.
    return inst_ptr;
  }

  template <typename HARDWARE>
  void AvidaCPU_Base<HARDWARE>::PrintState(std::ostream & os) const {
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
    if (next_inst < genome.GetSize()) { // For interpreter mode
        PrintInst(genome[next_inst], os);
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

  class AvidaGP : public AvidaCPU_Base<AvidaGP> {
  public:
    using base_t = AvidaCPU_Base<AvidaGP>;
    using typename base_t::genome_t;
    using typename base_t::inst_lib_t;

    AvidaGP(const genome_t & in_genome) : AvidaCPU_Base(in_genome) { ; }
    AvidaGP(Ptr<const inst_lib_t> inst_lib) : AvidaCPU_Base(genome_t(inst_lib)) { ; }
    AvidaGP(const inst_lib_t & inst_lib) : AvidaCPU_Base(genome_t(&inst_lib)) { ; }

    AvidaGP() = default;
    AvidaGP(const AvidaGP &) = default;
    AvidaGP(AvidaGP &&) = default;

    virtual ~AvidaGP() { ; }
  };
}

namespace std {

  /// operator<< to work with ostream (must be in std to work)
  inline std::ostream & operator<<(std::ostream & out, const emp::AvidaGP & org) {
    org.PrintGenome(out);
    return out;
  }
}

#endif // #ifndef EMP_HARDWARE_AVIDAGP_HPP_INCLUDE
