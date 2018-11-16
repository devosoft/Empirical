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
    bool active;

  public:
    StructType(TypeManager & _tmanager) : type_manager(_tmanager), num_bytes(0), active(false) { ; }
    ~StructType() { ; }

    size_t GetSize() const { return num_bytes; }
    bool IsActive() const { return active; }

    template <typename T>
    void AddMemberVar(const std::string & name) {
      emp_assert(active == false, "Cannot add member variables to an instantiated struct!");
      const Type & type = type_manager.GetType<T>();
      vars.push_back(type, name, num_bytes);
      num_bytes += type.GetSize();
    }

    void DefaultConstruct(MemoryImage & memory) const {
      memory.resize(num_bytes);
      for (auto & x : vars) x.DefaultConstruct(memory);
      active = true;
    }
  };

  class Struct {
  private:
    const StructType & type;
    MemoryImage memory;
  public:
    Struct(const StructType & _type) : type(_type), memory(type.GetSize()) {
      type.Initialize(memory);
    }
  };
}

#endif