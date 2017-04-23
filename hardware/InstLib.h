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

  template <typename ID_TYPE>
  class InstLib {
  protected:
    struct InstDef {
      ID_TYPE id;
      std::string name;
      std::string desc;

      InstDef() : id(ID_TYPE::Unknown) { ; }
      InstDef(ID_TYPE _id, const std::string & _n, const std::string & _d)
        : id(_id), name(_n), desc(_d) { ; }
      InstDef(const InstDef &) = default;
    };

    emp::vector<InstDef> inst_lib;
    std::map<std::string, size_t> name_map;

  public:
    InstLib() { ; }
    ~InstLib() { ; }

    size_t AddInst(ID_TYPE inst_id, const std::string & name, const std::string & desc="") {
      const size_t id = (size_t) inst_id;
      if (inst_lib.size() <= id) inst_lib.resize(id+1);
      inst_lib[id] = InstDef(inst_id, name, desc);
      name_map[name] = id;
    }

    const std::string & GetName(ID_TYPE id) const { return inst_lib[id].name; }
    const std::string & GetDesc(ID_TYPE id) const { return inst_lib[id].desc; }
    static constexpr char GetSymbol(ID_TYPE id) {
      if (id < 26) return ('a' + id);
      if (id < 52) return ('A' + (id - 26));
      if (id < 62) return ('0' + (id - 52));
      return '+';
    }
    ID_TYPE GetID(const std::string name) const {
      emp_assert(Has(name_map, name), name);
      return name_map[name];
    }
    static constexpr ID_TYPE GetID(char symbol) {
      if (symbol >= 'a' && symbol <= 'z') return (ID_TYPE) (symbol - 'a');
      if (symbol >= 'A' && symbol <= 'Z') return (ID_TYPE) (symbol - 'A' + 26);
      if (symbol >= '0' && symbol <= '9') return (ID_TYPE) (symbol - '0' + 52);
      return (ID_TYPE) 62;
    }

  };

}

#endif
