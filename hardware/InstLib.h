//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file maintains information about instructions availabel in virtual hardware.

#ifndef EMP_INST_LIB_H
#define EMP_INST_LIB_H

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
    };

    emp::vector<InstDef> inst_lib;
    std::map<std::string, size_t> name_map;
    std::array<size_t,128> symbol_chart;

  public:
    InstLib(ID_TYPE unknown_val) { ; }
    ~InstLib() { ; }

    size_t AddInst(ID_TYPE id, const std::string & name, const std::string & desc) {
    }
  };

}

#endif
