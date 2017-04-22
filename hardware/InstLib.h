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
    std::array<size_t,128> symbol_chart;

  public:
    InstLib() { ; }
    ~InstLib() { ; }

    size_t AddInst(ID_TYPE inst_id, const std::string & name, const std::string & desc) {
      size_t id = (size_t) inst_id;
      if (inst_lib.size() <= id) inst_lib.resize(id+1);
      inst_lib[id] = InstDef(inst_id, name, desc);
      name_map[name] = id;
    }
  };

}

#endif
