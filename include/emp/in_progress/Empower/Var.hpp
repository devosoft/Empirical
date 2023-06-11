/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file Var.hpp
 *  @brief A collection of information about a single, instantiated variable in Empower
 */

#ifndef EMP_IN_PROGRESS_EMPOWER_VAR_HPP_INCLUDE
#define EMP_IN_PROGRESS_EMPOWER_VAR_HPP_INCLUDE

#include "../base/assert.hpp"
#include "../base/Ptr.hpp"
#include "../meta/TypeID.hpp"

#include "MemoryImage.h"
#include "Type.h"

namespace emp {

  class Var {
  private:
    const Type & type;       ///< What type is this variable?
    MemoryImage & memory;    ///< Which memory image is variable using (by default)
    size_t mem_pos;          ///< Where is this variable in a memory image?

  public:
    Var(const Type & _type, MemoryImage & _mem, size_t _pos)
      : type(_type), memory(_mem), mem_pos(_pos) { ; }
    Var(const Var &) = default;

    Var & operator=(const Var &) = default;

    /// Restore this variable to its orignal type for direct use.
    template <typename T>
    T & Restore() {
      // Make sure function is restoring the correct type.
      emp_assert( type.GetID() == GetTypeValue<T>(), "Trying to restore Var to incorrect type." );

      // Convert this memory to a reference that can be returned.
      return memory.GetRef<T>(mem_pos);
    }

    Var & SetString(const std::string & val) {
      type.SetString(mem_pos, memory, val);
      return *this;
    }

    Var & operator=(const std::string & val) {
      type.SetString(mem_pos, memory, val);
      return *this;
    }

    Var & operator=(const char * val) {
      type.SetString(mem_pos, memory, val);
      return *this;
    }

    Var & operator=(double val) {
      // type.SetDouble(mem_pos, memory, val);
      return *this;
    }

    template <typename T>
    Var & operator=(T && val) {
      Restore<T>() = val;
      return *this;
    }

    // template <typename T>
    // Var & operator=(T && val) {
    //   using base_t = std::remove_reference< std::remove_cv<T> >;
    //   if constexpr (std::is_same<base_t, char *>()) {
    //     SetString(val);
    //   }
    //   else {
    //     Restore<T>() = val;
    //   }

    //   return *this;
    // }
  };


}

#endif // #ifndef EMP_IN_PROGRESS_EMPOWER_VAR_HPP_INCLUDE
