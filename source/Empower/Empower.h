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

    static constexpr size_t undefined_id = (size_t) -1;

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
      Var(size_t _id, MemoryImage & mem) : info_id(_id), mem_ptr(&mem) { ; }
      Var(const Var &) = default;
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
    Empower() : memory(), vars(), types(), var_map(), type_map() { ; }
    ~Empower() { ; }

    /// Convert a type (provided as a template argument) to its index in types vector.
    /// If type is not already in Empower, add it.
    template <typename T>
    size_t GetTypeID() {
      using base_t = typename std::decay<T>::type;

      // size_t type_hash = typeid(T).hash_code();
      std::string type_name = typeid(base_t).name();

      // If this type already exists stop here!
      auto type_it = type_map.find(type_name);
      if (type_it != type_map.end()) return type_it->second;

      size_t type_id = types.size();
      size_t mem_size = sizeof(base_t);
      types.emplace_back(type_id, type_name, mem_size);
      type_map[type_name] = type_id;

      return type_id;
    }

    template <typename T, typename... ARGS>
    Var NewVar(const std::string & name, ARGS... args) {
      size_t type_id = GetTypeID<T>();                ///< Get ID for type (create if needed)
      TypeInfo & type_info = types[type_id];          ///< Create ref to type info for easy access.
      site_t var_id = vars.size();                    ///< New var details go at end of var vector.
      size_t mem_start = memory.size();               ///< Start new var at current end of memory.
      vars.emplace_back(type_id, name, mem_start);    ///< Add this VarInfo to our records.
      memory.resize(mem_start + type_info.mem_size);  ///< Resize memory to fit new variable.

      /// Construct new variable contents in place, where space was allocated.
      new (&memory[mem_start]) T(std::forward<ARGS>(args)...);

      return Var(var_id, memory);
    }
  };

}

#endif
