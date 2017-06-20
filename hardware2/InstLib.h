//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file maintains information about instructions availabel in virtual hardware.

#ifndef EMP_INST_LIB_H
#define EMP_INST_LIB_H

#include <map>
#include <string>

#include "../base/vector.h"
#include "../tools/map_utils.h"

namespace emp {

  template <typename HARDWARE_T, typename ARG_T=size_t>
  class InstLib {
  protected:
    using hardware_t = HARDWARE_T;
    using arg_t = ARG_T;
    using fun_t = std::function<void(hardware_t &, const emp::vector<arg_t> &)>;
    constexpr size_t max_args = MAX_ARGS;

    struct InstDef {
      std::string name;
      fun_t fun_call;
      size_t num_args;
      std::string desc;

      InstDef(const std::string & _n, fun_t _fun, size_t _args, const std::string & _d)
        : name(_n), fun_call(_fun), num_args(_args), desc(_d) { ; }
      InstDef(const InstDef &) = default;
    };

    emp::vector<InstDef> inst_lib;              // Full definitions for instructions.
    emp::vector<fun_t> inst_funs;               // Map of instruction IDs to their functions.
    std::map<std::string, size_t> name_map;     // How do names link to instructions?
    std::map<std::string, inst_arg_t> arg_map;  // How are different arguments named?

  public:
    InstLib() : inst_lib(), inst_funs(), name_map(), arg_map() { ; }
    ~InstLib() { ; }

    const std::string & GetName(size_t id) const { return inst_lib[(size_t) id].name; }
    const std::string & GetDesc(size_t id) const { return inst_lib[(size_t) id].desc; }
    size_t GetNumArgs(size_t id) const { return inst_lib[(size_t) id].num_args; }
    static constexpr char GetSymbol(size_t id) {
      if (id < 26) return ('a' + id);
      if (id < 52) return ('A' + (id - 26));
      if (id < 62) return ('0' + (id - 52));
      return '+';
    }
    size_t GetID(const std::string name) const {
      emp_assert(Has(name_map, name), name);
      return Find(name_map, name, (size_t) -1);
    }
    static constexpr size_t GetID(char symbol) {
      if (symbol >= 'a' && symbol <= 'z') return (size_t) (symbol - 'a');
      if (symbol >= 'A' && symbol <= 'Z') return (size_t) (symbol - 'A' + 26);
      if (symbol >= '0' && symbol <= '9') return (size_t) (symbol - '0' + 52);
      return (size_t) 62;
    }

    inst_arg_t GetArg(const std::string & name) {
      emp_assert(Has(arg_map, name));
      return arg_map[name];
    }

    void AddInst(const std::string & name,
                 fun_t fun_call,
                 size_t num_args=0,
                 const std::string & desc="")
    {
      const size_t id = inst_lib.size();
      inst_lib.emplace_back(name, desc, fun_call, num_args);
      inst_funs.emplace_back(fun_call);
      name_map[name] = id;
    }

    void AddArg(const std::string & name, inst_arg_t value) {
      emp_assert(!Has(arg_map, name));
      arg_map[name] = value;
    }

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

  };

}

#endif
