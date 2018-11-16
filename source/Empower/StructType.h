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
    size_t num_bytes;

  public:
    StructType(TypeManager & _tmanager) : type_manager(_tmanager), num_bytes(0) { ; }
    ~StructType() { ; }

    size_t GetSize() const { return num_bytes; }

    template <typename T>
    void AddMemberVar(const std::string & name) {
      const Type & type = type_manager.GetType<T>();
      vars.push_back(type, name, num_bytes);
      num_bytes += type.GetSize();
    }
  };


}

#endif