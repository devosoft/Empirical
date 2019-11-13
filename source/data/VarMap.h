/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  VarMap.h
 *  @brief VarMaps track arbitrary data by name (slow) or id (faster).
 *  @note Status: ALPHA
 */

#ifndef EMP_VAR_MAP_H
#define EMP_VAR_MAP_H

#include <string>

#include "../base/assert.h"
#include "../base/Ptr.h"
#include "../base/unordered_map.h"
#include "../base/vector.h"
#include "../meta/TypeID.h"
#include "../tools/map_utils.h"
#include "../tools/string_utils.h"

namespace emp {

  class VarMap {
  private:
    struct VarBase {
      std::string name;                              ///< Name of this variable.

      VarBase(const std::string & in_name) : name(in_name) { ; }
      virtual ~VarBase() { ; }

      virtual emp::Ptr<VarBase> Clone() const = 0;
      virtual TypeID GetTypeID() const = 0;
      virtual std::string ToString() const = 0;
    };

    template <typename T>
    struct VarInfo : public VarBase {
      T value;                                       ///< Current value of this variable.

      VarInfo(const std::string & name, const T & in_value)
      : VarBase(name)
      , value( in_value )
      { ; }

      emp::Ptr<VarBase> Clone() const override { return emp::NewPtr< VarInfo<T> >(name, value); }
      TypeID GetTypeID() const override { return emp::GetTypeID<T>(); };
      std::string ToString() const override { return emp::to_string(value); }
    };

    emp::vector<emp::Ptr<VarBase>> vars;             ///< Vector of all current variables.
    emp::unordered_map<std::string, size_t> id_map;  ///< Map of names to vector positions.

  public:
    VarMap() { ; }
    VarMap(const VarMap & _in) : vars(_in.vars.size()), id_map(_in.id_map) {
      // Copy over each of the var infos into the correct position of the new vector.
      for (size_t i = 0; i < vars.size(); i++) {
        vars[i] = _in.vars[i]->Clone();
      }
    }
    VarMap(VarMap &&) = default;  // Default behavior will move all pointers over to the new VarMap.
    ~VarMap() {
      // Delete all of the variable info.
      for (auto x : vars) x.Delete();
    }

    const std::string & GetName(size_t id) const { return vars[id]->name; }
    size_t GetID(const std::string & name) const { return Find(id_map, name, (size_t) -1); }
    bool Has(const std::string & name) const { return emp::Has(id_map, name); }

    template <typename T>
    size_t Add(const std::string & name, const T & value) {
      emp_assert( emp::Has(id_map, name) == false );
      const size_t id = vars.size();
      emp::Ptr<VarInfo<T>> new_ptr = NewPtr<VarInfo<T>>( name, value );
      vars.push_back(new_ptr);
      id_map[name] = id;
      return id;
    }

    // Type-specific add functions.
    size_t AddString(const std::string & name, const std::string & value) {
      return Add<std::string>(name, value);
    }
    size_t AddInt(const std::string & name, int value)       { return Add<int>(name, value); }
    size_t AddDouble(const std::string & name, double value) { return Add<double>(name, value); }
    size_t AddChar(const std::string & name, char value)     { return Add<char>(name, value); }
    size_t AddBool(const std::string & name, bool value)     { return Add<bool>(name, value); }

    template <typename T>
    T & Get(size_t id) {
      emp_assert(id < vars.size());
      emp_assert(vars[id]->GetTypeID() = emp::GetTypeID<T>());
      emp::Ptr<VarInfo<T>> ptr = vars[id].Cast<VarInfo<T>>();
      return ptr->value;
    }

    template <typename T>
    T & Get(const std::string & name) {
      emp_assert( emp::Has(id_map, name) );
      const size_t id = id_map[name];
      return Get<T>(id);
    }

    template <typename T>
    const T & Get(size_t id) const {
      emp_assert(id < vars.size());
      emp_assert(vars[id]->GetTypeID() = emp::GetTypeID<T>());
      emp::Ptr<const VarInfo<T>> ptr = vars[id].Cast<const VarInfo<T>>();
      return ptr->value;
    }

    template <typename T>
    const T & Get(const std::string & name) const {
      emp_assert( emp::Has(id_map, name) );
      const size_t id = emp::Find(id_map, name, (size_t) -1);
      return Get<T>(id);
    }

    TypeID GetType(size_t id) const { return vars[id]->GetTypeID(); }
    TypeID GetType(std::string name) const { return GetType(GetID(name)); }

    // Type-specific Accessors    
    std::string & GetString(const std::string & name) { return Get<std::string>(name); }
    int & GetInt(const std::string & name) { return Get<int>(name); }
    double & GetDouble(const std::string & name) { return Get<double>(name); }
    char & GetChar(const std::string & name) { return Get<char>(name); }
    bool & GetBool(const std::string & name) { return Get<bool>(name); }

    // Type-specific const accessors
    const std::string & GetString(const std::string & name) const { return Get<std::string>(name); }
    int GetInt(const std::string & name) const { return Get<int>(name); }
    double GetDouble(const std::string & name) const { return Get<double>(name); }
    char GetChar(const std::string & name) const { return Get<char>(name); }
    bool GetBool(const std::string & name) const { return Get<bool>(name); }

    template <typename T>
    void Set(size_t id, const T & value) {
      emp_assert(id < vars.size());
      emp_assert(vars[id]->GetTypeID() = emp::GetTypeID<T>());
      emp::Ptr<VarInfo<T>> ptr = vars[id].Cast<VarInfo<T>>();
      ptr->value = value;
    }

    template <typename T>
    void Set(const std::string & name, const T & value) {
      if (auto it = id_map.find(name); it != id_map.end()) { Set(it->second, value); }
      else Add(name, value);
    }

  };

}

#endif
