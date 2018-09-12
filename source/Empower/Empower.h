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
#include <map>
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
    private:
      memory_t memory;                 ///< The specific memory values.
      emp::Ptr<Empower> empower_ptr;   ///< A pointer back to Empower instance this memory uses.

    public:
      MemoryImage(emp::Ptr<Empower> _ptr) : memory(), empower_ptr(_ptr) { ; }
      MemoryImage(const MemoryImage &) = default;
      MemoryImage(MemoryImage &&) = default;

      const memory_t & GetMemory() const { return memory; }
      const emp::Ptr<Empower> GetEmpowerPtr() const { return empower_ptr; }
    };

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

      TypeInfo(size_t _id, const std::string & _name, size_t _size)
       : type_id(_id), type_name(_name), mem_size(_size) { ; }
    };

    memory_t memory;  /// The default memory image.
    emp::vector<VarInfo> vars;
    emp::vector<TypeInfo> types;

    std::map<std::string, size_t> var_map;   ///< Map variable names to index in vars
    std::map<std::string, size_t> type_map;  ///< Map type names (from typeid) to index in types

  public:
    Empower() : memory() { ; }
    ~Empower() { ; }

    template <typename T>
    void AddType() {
      using base_t = typename std::decay<T>::type;

      // size_t type_hash = typeid(T).hash_code();
      std::string type_name = typeid(base_t).name();

      // If this type already exists stop here!
      if (emp::Has(type_map, type_name)) return;

      size_t type_id = types.size();
      size_T mem_size = sizeof(base_t);
      types.emplace_back(type_id, type_name, mem_size);
      type_map[type_name] = type_id;
    }
  };

}

#endif
