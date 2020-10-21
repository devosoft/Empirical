/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  TypeManager.hpp
 *  @brief Handles creation or retrieval of type objects.
 */

#ifndef EMP_EMPOWER_TYPE_MANAGER_H
#define EMP_EMPOWER_TYPE_MANAGER_H

#include <functional>
#include <string>
#include <unordered_map>

#include "../base/assert.hpp"
#include "../base/Ptr.hpp"

#include "Type.h"

namespace emp {

  class TypeManager {
  private:
    std::unordered_map< size_t, emp::Ptr<Type> > type_map;       ///< Map type values to Type objects.

  public:
    TypeManager() : type_map() { ; }
    ~TypeManager() {
      // Delete all types being managed
      for (auto & x : type_map) x.second.Delete();
    }

    template <typename T>
    const Type & GetType() {
      using base_t = typename std::decay<T>::type;

      size_t type_hash = GetTypeValue<base_t>();
      std::string type_name = typeid(base_t).name();

      // If this type already exists stop here!
      auto type_it = type_map.find(type_hash);
      if (type_it != type_map.end()) return *(type_it->second);

      Ptr<Type> type_ptr = emp::NewPtr< TypeInfo<T> >();
      type_map[type_hash] = type_ptr;

      return *type_ptr;
    }

    static TypeManager & Default() {
      static TypeManager type_manager;
      return type_manager;
    }
  };


}

#endif
