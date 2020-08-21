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
#include "Var.h"

namespace emp {

  class VarInfo {
  private:
    const Type & type;      ///< What type is this variable?
    std::string var_name;   ///< What is the unique name for this variable?
    size_t mem_pos;         ///< Where in memory is this variable stored?

  public:
    VarInfo(const Type & _type, const std::string & _name, size_t _pos)
      : type(_type), var_name(_name), mem_pos(_pos) { ; }

    const Type & GetType() const { return type; }
    const std::string & GetName() const { return var_name; }
    size_t GetMemPos() const { return mem_pos; }

    Var GetVar(MemoryImage & memory) const {
      return Var(type, memory, mem_pos);
    }

    void DefaultConstruct(MemoryImage & memory) const {
      type.DefaultConstruct(mem_pos, memory);
    }

    void CopyConstruct(const MemoryImage & from_memory, MemoryImage & to_memory) const {
      type.CopyConstruct(mem_pos, from_memory, to_memory);
    }

    void CopyAssign(const MemoryImage & from_memory, MemoryImage & to_memory) const {
      type.CopyAssign(mem_pos, from_memory, to_memory);
    }

    void Destruct(MemoryImage & memory) const {
      type.Destruct(mem_pos, memory);
    }
  };


}

#endif