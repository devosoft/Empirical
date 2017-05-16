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

  template <typename INST_TYPE>
  class InstLib {
  protected:
    using inst_t = INST_TYPE;
    using inst_id_t = typename INST_TYPE::id_t;
    using inst_arg_t = typename INST_TYPE::arg_t;
    using genome_t = emp::vector<inst_t>;

    struct InstDef {
      inst_id_t id;
      std::string name;
      std::string desc;
      size_t num_args;

      InstDef() : id(inst_id_t::Unknown) { ; }
      InstDef(inst_id_t _id, const std::string & _n, const std::string & _d, size_t _args)
        : id(_id), name(_n), desc(_d), num_args(_args) { ; }
      InstDef(const InstDef &) = default;
    };

    emp::vector<InstDef> inst_lib;
    std::map<std::string, size_t> name_map;
    std::map<std::string, inst_arg_t> arg_map;

  public:
    InstLib() { ; }
    ~InstLib() { ; }

    const std::string & GetName(inst_id_t id) const { return inst_lib[(size_t) id].name; }
    const std::string & GetDesc(inst_id_t id) const { return inst_lib[(size_t) id].desc; }
    size_t GetNumArgs(inst_id_t id) const { return inst_lib[(size_t) id].num_args; }
    static constexpr char GetSymbol(inst_id_t id) {
      if (id < 26) return ('a' + id);
      if (id < 52) return ('A' + (id - 26));
      if (id < 62) return ('0' + (id - 52));
      return '+';
    }
    inst_id_t GetID(const std::string name) const {
      emp_assert(Has(name_map, name), name);
      return name_map[name];
    }
    static constexpr inst_id_t GetID(char symbol) {
      if (symbol >= 'a' && symbol <= 'z') return (inst_id_t) (symbol - 'a');
      if (symbol >= 'A' && symbol <= 'Z') return (inst_id_t) (symbol - 'A' + 26);
      if (symbol >= '0' && symbol <= '9') return (inst_id_t) (symbol - '0' + 52);
      return (inst_id_t) 62;
    }

    inst_arg_t GetArg(const std::string & name) {
      emp_assert(Has(arg_map, name));
      return arg_map[name];
    }

    void AddInst(inst_id_t inst_id, const std::string & name,
                 size_t num_args=0,
                 const std::string & desc="")
    {
      const size_t id = (size_t) inst_id;
      if (inst_lib.size() <= id) inst_lib.resize(id+1);
      inst_lib[id] = InstDef(inst_id, name, desc, num_args);
      name_map[name] = id;
    }

    void AddArg(const std::string & name, inst_arg_t value) {
      emp_assert(!Has(arg_map, name));
      arg_map[name] = value;
    }

    void WriteGenome(const genome_t & genome, std::ostream & os=std::cout) const {
      for (const inst_t & inst : genome) {
        os << ((int) inst.id) << " " << GetName(inst.id);
        const size_t num_args = GetNumArgs(inst.id);
        for (size_t i = 0; i < num_args; i++) {
          os << ' ' << inst.args[i];
        }
        os << '\n';
      }
    }

    void ReadInst(genome_t & genome, std::string info) const {
      std::string name = emp::string_pop_word(info);
      inst_id_t id = GetID(name);
      genome.emplace_back(id);
      size_t num_args = GetNumArgs(id);
      for (size_t i = 0; i < num_args; i++) {
        std::string arg_name = emp::string_pop_word(info);
        // @CAO: Should check to make sure arg name is real.
        genome.back().args[i] = arg_map[arg_name];
      }
    }

  };

}

#endif
