/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  Type.h
 *  @brief A collection of information about how to manage variables of a specified type.
 */

#ifndef EMP_EMPOWER_TYPE_H
#define EMP_EMPOWER_TYPE_H

#include <functional>
#include <string>

#include "../base/assert.h"

#include "MemoryImage.h"

namespace emp {

  class Type {
  private:
    size_t type_id;          ///< Unique value for this type
    std::string type_name;   ///< Name of this type (from std::typeid)

  public:
    Type(const std::string & _name) : type_name(_name) { ; }

    const std::string & GetName() const { return type_name; }
    size_t GetID() const { return type_id; }
    virtual size_t GetSize() const = 0;

    virtual void DefaultConstruct(size_t mem_pos, MemoryImage & mem_image) = 0;
    virtual void CopyConsturct(size_t mem_pos, const MemoryImage & mem_from, MemoryImage & mem_to) = 0;
    virtual void CopyAssign(size_t mem_pos, const MemoryImage & mem_from, MemoryImage & mem_to) = 0;
    virtual void Destruct(size_t mem_pos, MemoryImage & mem_image) = 0;
  };

  /// Information about a single type used in Empower.
  template <typename T>
  class TypeInfo : public Type {
  private:
    size_t mem_size;         ///< Bytes needed for this type (from sizeof)      
  
    using base_t = typename std::decay<T>::type;
  public:
    TypeInfo() : Type(typeid(base_t).name()) { ; }

    // How many bytes is this type?
    size_t GetSize() const { return sizeof(T); };

    // Construct an object of type T at a specified MemoryImage position.
    void DefaultConstruct(size_t mem_pos, MemoryImage & mem_image) {
      new (mem_image.GetPtr<T>(mem_pos).Raw()) T;
    }

    /// Copy constructor for type T at a specified MemoryImage position.
    void CopyConsturct(size_t mem_pos, const MemoryImage & mem_from, MemoryImage & mem_to) {
      new (mem_to.GetPtr<T>(mem_pos).Raw()) T(mem_from.GetRef<T>(mem_pos));
    }

    /// Copy assignment for type T at a specified MemoryImage position.
    void CopyAssign(size_t mem_pos, const MemoryImage & mem_from, MemoryImage & mem_to) {
      mem_to.GetRef<T>(mem_pos) = mem_from.GetRef<T>(mem_pos);
    }

    /// Destructor for type T at a specified MemoryImage position
    void Destruct(size_t mem_pos, MemoryImage & mem_image) {
	    mem_image.GetPtr<T>(mem_pos)->~T();
    }

    /// @todo ADD move function and move constructor?
    /// @todo ADD:  double ToDouble(size_t mem_pos, MemoryImage & mem_image)
    /// @todo ADD:  std::string ToString(size_t mem_pos, MemoryImage & mem_image)

  };

}

#endif
