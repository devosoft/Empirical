/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  Struct.h
 *  @brief Struct is a set of active variables, grouped by name (organized by a specific StructType)
 */

#ifndef EMP_EMPOWER_STRUCT_H
#define EMP_EMPOWER_STRUCT_H

#include "../base/assert.h"
#include "../base/Ptr.h"
#include "../base/vector.h"

#include "StructType.h"
#include "VarInfo.h"
#include "Var.h"

namespace emp {

  /// An instance of a struct type.
  class Struct {
  private:
    const StructType & type;  ///< What type is this Struct (i.e., what members does it have?)
    MemoryImage memory;       ///< Raw memory for storing struct information.
  public:
    Struct(const StructType & _type) : type(_type), memory() {
      type.DefaultConstruct(memory);
    }
    Struct(const Struct & _in) : type(_in.type), memory() {
      type.CopyConstruct(_in.memory, memory);
    }
    ~Struct() {
      type.Destruct(memory);
    }

    Var GetVar(const std::string & name) {
      return type.GetVarInfo(name).GetVar(memory);
    }

    Var GetVar(size_t var_id) {
      return type.GetVarInfo(var_id).GetVar(memory);
    }

    Var operator[](const std::string & name) {
      return type.GetVarInfo(name).GetVar(memory);
    }

    Var operator[](size_t var_id) {
      return type.GetVarInfo(var_id).GetVar(memory);
    }
  };
}

#endif