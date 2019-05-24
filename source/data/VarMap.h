/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  VarMap.h
 *  @brief VarMaps track arbitrary data by name (slow) or id (faster).
 *  @note Status: ALPHA
 */

#ifndef EMP_VAR_MAP_H
#define EMP_VAR_MAP_H

#include <string>

#include "../base/assert.h"
#include "../base/Ptr.h"
#include "../base/unordered_map.h"
#include "../base/vector.h"
#include "../tools/map_utils.h"
#include "../tools/string_utils.h"

namespace emp {

  class VarMap {

    struct VarBase {
      std::string name;
    };

    template <typename T>
    struct VarInfo : public VarBase {
      T value;
    };

    emp::vector<emp::Ptr<VarBase>> vars;
    emp::unordered_map<std::string, size_t> id_map;
  };

}

#endif
