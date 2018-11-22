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
#include "../meta/TypeID.h"

#include "MemoryImage.h"

namespace emp {

  class Type {
  public:
    Type() { ; }
    virtual ~Type() { ; }

    virtual std::string GetName() const = 0;
    virtual size_t GetID() const = 0;
    virtual size_t GetSize() const = 0;

    virtual void DefaultConstruct(size_t mem_pos, MemoryImage & mem_image) const = 0;
    virtual void CopyConstruct(size_t mem_pos, const MemoryImage & mem_from, MemoryImage & mem_to) const = 0;
    virtual void CopyAssign(size_t mem_pos, const MemoryImage & mem_from, MemoryImage & mem_to) const = 0;
    virtual void Destruct(size_t mem_pos, MemoryImage & mem_image) const = 0;

    template <typename T> bool IsType() const;

    virtual void SetString(size_t mem_pos, MemoryImage & mem_image, const std::string & val) const = 0;

    virtual std::string AsString(size_t mem_pos, MemoryImage & mem_image) const = 0;
  };

  /// Information about a single type used in Empower.
  template <typename T>
  class TypeInfo : public Type {
  private:
    using base_t = typename std::decay<T>::type;
  public:
    TypeInfo() { ; }

    std::string GetName() const { return typeid(base_t).name(); }
    size_t GetID() const { return GetTypeValue<base_t>(); }     ///< Unique ID for this type.
    size_t GetSize() const { return sizeof(T); };               ///< How many bytes is this type?

    // Construct an object of type T at a specified MemoryImage position.
    void DefaultConstruct(size_t mem_pos, MemoryImage & mem_image) const {
      new (mem_image.GetPtr<T>(mem_pos).Raw()) T;
    }

    /// Copy constructor for type T at a specified MemoryImage position.
    void CopyConstruct(size_t mem_pos, const MemoryImage & mem_from, MemoryImage & mem_to) const {
      new (mem_to.GetPtr<T>(mem_pos).Raw()) T(mem_from.GetRef<T>(mem_pos));
    }

    /// Copy assignment for type T at a specified MemoryImage position.
    void CopyAssign(size_t mem_pos, const MemoryImage & mem_from, MemoryImage & mem_to) const {
      mem_to.GetRef<T>(mem_pos) = mem_from.GetRef<T>(mem_pos);
    }

    /// Destructor for type T at a specified MemoryImage position
    void Destruct(size_t mem_pos, MemoryImage & mem_image) const {
	    mem_image.GetPtr<T>(mem_pos)->~T();
    }


    void SetString(size_t mem_pos, MemoryImage & mem_image, const std::string & val) const {
      if constexpr (std::is_same<T,std::string>()) {
        mem_image.GetRef<T>(mem_pos) = val;
      }
    }

    std::string AsString(size_t mem_pos, MemoryImage & mem_image) const {
      std::stringstream ss;
      ss << mem_image.GetRef<T>(mem_pos);
      return ss.str();
    }

    /// @todo ADD move function and move constructor?
    /// @todo ADD:  double ToDouble(size_t mem_pos, MemoryImage & mem_image)
    /// @todo ADD:  std::string ToString(size_t mem_pos, MemoryImage & mem_image)

  };

  template <typename T>
  bool Type::IsType() const {
    return dynamic_cast<const TypeInfo<T> *>(this);
  }

}

#endif
