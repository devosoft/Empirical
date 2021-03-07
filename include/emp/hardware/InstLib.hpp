/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file  InstLib.hpp
 *  @brief This file maintains information about instructions availabel in virtual hardware.
 */

#ifndef EMP_INST_LIB_H
#define EMP_INST_LIB_H

#include <map>
#include <string>
#include <unordered_set>

#include "../base/array.hpp"
#include "../base/Ptr.hpp"
#include "../base/vector.hpp"
#include "../datastructs/map_utils.hpp"
#include "../tools/string_utils.hpp"

namespace emp {

  /// ScopeType is used for scopes that we need to do something special at the end.
  /// Eg: LOOP needs to go back to beginning of loop; FUNCTION needs to return to call.
  enum class ScopeType { NONE=0, ROOT, BASIC, LOOP, FUNCTION };

  /// @brief InstLib maintains a set of instructions for use in virtual hardware.
  /// @param HARDWARE_T Type of the virtual hardware class to track instructions.
  /// @param ARG_T What types of arguments are associated with instructions.
  /// @param ARG_COUNT Max number of arguments on an instruction.
  template <typename HARDWARE_T, typename ARG_T=size_t, size_t ARG_COUNT=3>
  class InstLib {
  public:
    using hardware_t = HARDWARE_T;
    using inst_t = typename hardware_t::inst_t;
    using genome_t = emp::vector<inst_t>;
    using arg_t = ARG_T;
    using fun_t = std::function<void(hardware_t &, const inst_t &)>;
    using inst_properties_t = std::unordered_set<std::string>;

    struct InstDef {
      std::string name;             ///< Name of this instruction.
      fun_t fun_call;               ///< Function to call when executing.
      size_t num_args;              ///< Number of args needed by function.
      std::string desc;             ///< Description of function.
      ScopeType scope_type;         ///< How does this instruction affect scoping?
      size_t scope_arg;             ///< Which arg indicates new scope (if any).
      inst_properties_t properties; ///< Are there any generic properties associated with this inst def?

      InstDef(const std::string & _n, fun_t _fun, size_t _args, const std::string & _d,
              ScopeType _s_type, size_t _s_arg, const inst_properties_t & _properties = inst_properties_t())
        : name(_n), fun_call(_fun), num_args(_args), desc(_d)
        , scope_type(_s_type), scope_arg(_s_arg), properties(_properties) { ; }
      InstDef(const InstDef &) = default;
    };

  protected:
    emp::vector<InstDef> inst_lib;           ///< Full definitions for instructions.
    emp::vector<fun_t> inst_funs;            ///< Map of instruction IDs to their functions.
    std::map<std::string, size_t> name_map;  ///< How do names link to instructions?
    std::map<std::string, arg_t> arg_map;    ///< How are different arguments named?

  public:
    InstLib() : inst_lib(), inst_funs(), name_map(), arg_map() { ; }  ///< Default Constructor
    InstLib(const InstLib &) = delete;                               ///< Copy Constructor
    InstLib(InstLib &&) = delete;                                    ///< Move Constructor
    ~InstLib() { ; }                                                  ///< Destructor

    InstLib & operator=(const InstLib &) = default;                   ///< Copy Operator
    InstLib & operator=(InstLib &&) = default;                        ///< Move Operator

    /// Return the name associated with the specified instruction ID.
    const std::string & GetName(size_t id) const { return inst_lib[id].name; }

    /// Return the function associated with the specified instruction ID.
    const fun_t & GetFunction(size_t id) const { return inst_lib[id].fun_call; }

    /// Return the number of arguments expected for the specified instruction ID.
    size_t GetNumArgs(size_t id) const { return inst_lib[id].num_args; }

    /// Return the provided description for the provided instruction ID.
    const std::string & GetDesc(size_t id) const { return inst_lib[id].desc; }

    /// What type of scope does this instruction state?  ScopeType::NONE is default.
    ScopeType GetScopeType(size_t id) const { return inst_lib[id].scope_type; }

    /// If this instruction alters scope, identify which argument does so.
    size_t GetScopeArg(size_t id) const { return inst_lib[id].scope_arg; }

    /// Return the set of properties for the provided instruction ID.
    const inst_properties_t & GetProperties(size_t id) const { return inst_lib[id].properties; }

    /// Does the given instruction ID have the given property value?
    bool HasProperty(size_t id, std::string property) const { return inst_lib[id].properties.count(property); }

    /// Get the number of instructions in this set.
    size_t GetSize() const { return inst_lib.size(); }

    /// Retrieve a unique letter associated with the specified instruction ID.
    static constexpr char GetSymbol(size_t id) {
      if (id < 26) return ('a' + id);
      if (id < 52) return ('A' + (id - 26));
      if (id < 62) return ('0' + (id - 52));
      return '+';
    }

    bool IsInst(const std::string name) const {
        return Has(name_map, name);
    }

    /// Return the ID of the instruction that has the specified name.
    size_t GetID(const std::string & name) const {
      emp_assert(Has(name_map, name), name);
      return Find(name_map, name, (size_t) -1);
    }

    /// Return the ID of the instruction associated with the specified symbol.
    static constexpr size_t GetID(char symbol) {
      if (symbol >= 'a' && symbol <= 'z') return (size_t) (symbol - 'a');
      if (symbol >= 'A' && symbol <= 'Z') return (size_t) (symbol - 'A' + 26);
      if (symbol >= '0' && symbol <= '9') return (size_t) (symbol - '0' + 52);
      return (size_t) 62;
    }

    /// Return the argument value associated with the provided keyword.
    arg_t GetArg(const std::string & name) {
      emp_assert(Has(arg_map, name));
      return arg_map[name];
    }

    /// @brief Add a new instruction to the set.
    /// @param name A unique string name for this instruction.
    /// @param fun_call The function that should be called when this instruction is executed.
    /// @param num_args How many arguments does this function require? (default=0)
    /// @param desc A description of how this function operates. (default="")
    /// @param scope_type Type of scope does this instruction creates. (default=ScopeType::NONE)
    /// @param scope_arg If instruction changes scope, which argument specified new scope? (default=-1)
    void AddInst(const std::string & name,
                 const fun_t & fun_call,
                 size_t num_args=0,
                 const std::string & desc="",
                 ScopeType scope_type=ScopeType::NONE,
                 size_t scope_arg=(size_t) -1,
                 const inst_properties_t & inst_properties=inst_properties_t())
    {
      const size_t id = inst_lib.size();
      inst_lib.emplace_back(name, fun_call, num_args, desc, scope_type, scope_arg, inst_properties);
      inst_funs.emplace_back(fun_call);
      name_map[name] = id;
    }

    /// Specify a keyword and arg value.
    void AddArg(const std::string & name, arg_t value) {
      emp_assert(!Has(arg_map, name));
      arg_map[name] = value;
    }

    /// Process a specified instruction in the provided hardware.
    void ProcessInst(hardware_t & hw, const inst_t & inst) const {
      inst_funs[inst.id](hw, inst);
    }

    /// Process a specified instruction on hardware that can be converted to the correct type.
    template <typename IN_HW>
    void ProcessInst(emp::Ptr<IN_HW> hw, const inst_t & inst) const {
      emp_assert( dynamic_cast<hardware_t*>(hw.Raw()) );
      inst_funs[inst.id](*(hw.template Cast<hardware_t>()), inst);
    }


    /// Write out a full genome to the provided ostream.
    void WriteGenome(const genome_t & genome, std::ostream & os=std::cout) const {
      for (const inst_t & inst : genome) {
        os << inst.id << " " << GetName(inst.id);
        const size_t num_args = GetNumArgs(inst.id);
        for (size_t i = 0; i < num_args; i++) {
          os << ' ' << inst.args[i];
        }
        os << '\n';
      }
    }

    /// Read the instruction in the provided info and append it to the provided genome.
    void ReadInst(genome_t & genome, std::string info) const {
      std::string name = emp::string_pop_word(info);
      size_t id = GetID(name);
      genome.emplace_back(id);
      size_t num_args = GetNumArgs(id);
      for (size_t i = 0; i < num_args; i++) {
        std::string arg_name = emp::string_pop_word(info);
        // @CAO: Should check to make sure arg name is real.
        if (emp::Has(arg_map, arg_name) == false) {
          std::cerr << "Unknown argument '" << arg_name << "'.  Ignoring." << std::endl;
        }
        genome.back().args[i] = arg_map[arg_name];
      }
    }

    /// Print out summary of instruction library.
    void PrintManifest(std::ostream & os=std::cout) const {
      os << "id" << "," << "name" << std::endl;
      for (size_t i = 0; i < inst_lib.size(); ++i) {
        os << i << "," << inst_lib[i].name << std::endl;
      }
    }

  };

}

#endif
