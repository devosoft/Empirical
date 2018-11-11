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
    virtual void DefaultConstruct(size_t mem_pos, MemoryImage & mem_image) = 0;
    virtual void CopyConsturct(size_t mem_pos, const MemoryImage & mem_from, MemoryImage & mem_to) = 0;
    virtual void CopyAssign(size_t mem_pos, const MemoryImage & mem_from, MemoryImage & mem_to) = 0;
    virtual void Destruct(size_t mem_pos, MemoryImage & mem_image) {
  };

  /// Information about a single type used in Empower.
  template <typename T>
  class TypeInfo : public Type {
  private:
    size_t type_id;          ///< Unique value for this type
    std::string type_name;   ///< Name of this type (from std::typeid)
    size_t mem_size;         ///< Bytes needed for this type (from sizeof)      
  
  public:
    TypeInfo() { ; }

    // Construct an object of type T at a specified MemoryImage position.
    void DefaultConstruct(size_t mem_pos, MemoryImage & mem_image) {
      new (mem_image.GetPtr<T>(mem_pos).Raw()) T;
    }

    /// Copy constructor for type T at a specified MemoryImage position.
    void CopyConsturct(size_t mem_pos, const MemoryImage & mem_from, MemoryImage & mem_to) {
      new (to_image.GetPtr<T>(mem_pos).Raw()) T(from_image.GetRef<T>(mem_pos));
    }

    /// Copy assignment for type T at a specified MemoryImage position.
    void CopyAssign(size_t mem_pos, const MemoryImage & mem_from, MemoryImage & mem_to) {
      to_image.GetRef<T>(mem_pos) = from_image.GetRef<T>(mem_pos);
    }

    /// Destructor for type T at a specified MemoryImage position
    void Destruct(size_t mem_pos, MemoryImage & mem_image) {
	    mem_image.GetPtr<T>(var_info.mem_pos)->~T();
    }

    /// @todo Also add move function and move constructor?
    
    // Core conversion functions for this type.
    std::function<double(Var &)> to_double;      ///< Fun to convert type to double (empty=>none)
    std::function<std::string(Var &)> to_string; ///< Fun to convert type to string (empty=>none)
    
    Type(size_t _id, const std::string & _name, size_t _size,
          const dconstruct_fun_t & dc_fun, const cconstruct_fun_t & cc_fun,
          const copy_fun_t & c_fun, const destruct_fun_t & d_fun)
      : type_id(_id), type_name(_name), mem_size(_size)
      , dconstruct_fun(dc_fun), cconstruct_fun(cc_fun)
      , copy_fun(c_fun), destruct_fun(d_fun) { ; }

    Type() { ; }
  };

}

#endif
