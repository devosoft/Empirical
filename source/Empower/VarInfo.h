/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  VarInfo.h
 *  @brief Generic info about a single variable (across MemoryImages)
 */

#ifndef EMP_EMPOWER_VAR_INFO_H
#define EMP_EMPOWER_VAR_INFO_H

#include "../base/assert.h"
#include "../base/Ptr.h"

#include "MemoryImage.h"
#include "Type.h"

namespace emp {

  class VarInfo {
  private:
    Type & type;            ///< What type is this variable?
    std::string var_name;   ///< What is the unique name for this variable?
    size_t mem_pos;         ///< Where in memory is this variable stored?

  public:
    VarInfo(Type & _type, const std::string & _name, size_t _pos)
      : type(&_type), var_name(_name), mem_pos(_pos) { ; }

  };


}

#endif