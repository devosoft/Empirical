/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  Empower.h
 *  @brief A scripting language built inside of C++
 *
 *  Empower is a scripting language built inside of Empirical to simplify and the use of fast
 *  run-time interpreting.  Internally, and Empower object will track all of the types used and 
 *  all of the variables declared, ensuring that they interact correctly.
 * 
 */

#ifndef EMP_EMPOWER_H
#define EMP_EMPOWER_H

#include <functional>
#include <string>

#include "../base/Ptr.h"
#include "../base/vector.h"

namespace emp {

  class Empower {
  public:
    using byte_t = unsigned char;
    using memory_t = emp::vector<byte_t>;

    /// A MemoryImage is a full set of variable values stored in an Empower instance.
    class MemoryImage {
      memory_t memory;                 ///< The specific memory values.
      emp::Ptr<Empower> empower_ptr;   ///< A pointer back to Empower instance this memory uses.
    }

    /// A Var is an internal variable that has a run-time determined type (which is tracked).
    class Var {
    private:
      size_t info_id;                 ///< Which variable ID is this var associated with?
      emp::Ptr<MemoryImage> mem_ptr;  ///< Which memory image is variable using (by default)
    public:
      Var() : info_id(0) { ; }
    };

  protected:
    /// Information about a single Empower variable, including its type, name, and where to
    /// find it in a memory image.
    struct VarInfo {
      size_t type_id;          ///< What type is this variable?
      std::string var_name;    ///< What is the unique name for this variable?
      size_t mem_pos;          ///< Where in memory is this variable stored?
    };

    /// Information about a single type used in Empower.
    struct TypeInfo {
      size_t type_id;          ///< Unique ID for this type.
      std::string type_name;   ///< Name of this type (from std::typeid)
      size_t mem_size;         ///< Bytes needed for this type (from sizeof)

      // Core conversion functions for this type.
      std::function<double(Var &)> to_double;      ///< Fun to convert type to double (empty=>none)
      std::function<std::string(Var &)> to_string; ///< Fun to convert type to string (empty=>none)
    };

    memory_t memory;  /// The default memory image.

  public:
    Empower() : memory() { ; }
    ~Empower() { ; }


  };

}

#endif
