/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  StructType.h
 *  @brief StructType maps variables to a MemoryImage; Struct is an instance of StructType
 */

#ifndef EMP_EMPOWER_STRUCT_TYPE_H
#define EMP_EMPOWER_STRUCT_TYPE_H

#include "../base/assert.h"
#include "../base/Ptr.h"
#include "../base/vector.h"

#include "MemoryImage.h"
#include "TypeManager.h"
#include "VarInfo.h"

namespace emp {

  class StructType {
  private:
    emp::vector<VarInfo> vars;
    TypeManager & type_manager;

  public:
    StructType(TypeManager & _tmanager) : type_manager(_tmanager) { ; }
    ~StructType() { ; }

    template <typename T>
    void AddMemberVar(const std::string & name) {
      vars.push_back(type_manager.GetType<T>(), name, pos);
    }
  };


}

#endif