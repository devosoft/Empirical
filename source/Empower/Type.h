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

  /// Information about a single type used in Empower.
  struct Type {
    size_t type_id;          ///< Unique value for this type
    std::string type_name;   ///< Name of this type (from std::typeid)
    size_t mem_size;         ///< Bytes needed for this type (from sizeof)      
    
    /// default constructor type
    using dconstruct_fun_t = std::function<void(size_t mem_pos, MemoryImage &)>;

    /// copy constructor function type
    using cconstruct_fun_t = std::function<void(size_t mem_pos, const MemoryImage &, MemoryImage &)>;

    /// copy assignment function type
    using copy_fun_t = std::function<void(size_t mem_pos, const MemoryImage &, MemoryImage &)>;

    /// destructor function type
    using destruct_fun_t = std::function<void(size_t mem_pos, MemoryImage &)>;

    dconstruct_fun_t dconstruct_fun; ///< Function to fun default constructor on var of this type
    cconstruct_fun_t cconstruct_fun; ///< Function to run copy constructor on var of this type
    copy_fun_t copy_fun;             ///< Function to copy var of this type across memory images
    destruct_fun_t destruct_fun;     ///< Function to run destructor on var of this type
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
  };

}

#endif
