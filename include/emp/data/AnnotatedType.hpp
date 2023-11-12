/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file AnnotatedType.hpp
 *  @brief A base class to provide a DataMap and accessors to derived classes.
 *  @note Status: ALPHA
 *
 */

#ifndef EMP_DATA_ANNOTATEDTYPE_HPP_INCLUDE
#define EMP_DATA_ANNOTATEDTYPE_HPP_INCLUDE

#include "../base/assert.hpp"
#include "../meta/TypeID.hpp"
#include "../tools/string_utils.hpp"

#include "DataMap.hpp"

namespace emp {

  /// A generic base class implementing the use of dynamic traits via DataMaps.
  class AnnotatedType {
  private:
    emp::DataMap data_map;   ///< Dynamic variables assigned to this class.

  public:
    emp::DataMap & GetDataMap() { return data_map; }
    const emp::DataMap & GetDataMap() const { return data_map; }

    void SetDataMap(emp::DataMap & in_dm) { data_map = in_dm; }

    emp::DataLayout & GetDataLayout() { return data_map.GetLayout(); }
    const emp::DataLayout & GetDataLayout() const { return data_map.GetLayout(); }

    bool HasTraitID(size_t id) const { return data_map.HasID(id); }
    bool HasTrait(const std::string & name) const { return data_map.HasName(name); }
    template <typename T>
    bool TestTraitType(size_t id) const { return data_map.IsType<T>(id); }
    template <typename T>
    bool TestTraitType(const std::string & name) const { return data_map.IsType<T>(name); }

    size_t GetTraitID(const std::string & name) const { return data_map.GetID(name); }

    template <typename T, typename KEY_T>
    auto & GetTrait(KEY_T && key) {
      return data_map.Get<T>(std::forward<KEY_T>(key));
    }

    template <typename T, typename KEY_T>
    auto GetTrait(KEY_T && key, size_t count) {
      return data_map.Get<T>(std::forward<KEY_T>(key), count);
    }

    template <typename T, typename KEY_T>
    const auto & GetTrait(KEY_T && key) const {
      return data_map.Get<T>(std::forward<KEY_T>(key));
    }

    template <typename T, typename KEY_T>
    auto GetTrait(KEY_T && key, size_t count) const {
      return data_map.Get<T>(std::forward<KEY_T>(key), count);
    }

    template <typename T>
    T & SetTrait(size_t id, const T & val) { return data_map.Set<T>(id, val); }

    template <typename T>
    T & SetTrait(const std::string & name, const T & val) { return data_map.Set<T>(name, val); }

    emp::TypeID GetTraitType(size_t id) const { return data_map.GetType(id); }
    emp::TypeID GetTraitType(const std::string & name) const { return data_map.GetType(name); }

    double GetTraitAsDouble(size_t id) const { return data_map.GetAsDouble(id); }

    double GetTraitAsDouble(size_t trait_id, emp::TypeID type_id) const {
      return data_map.GetAsDouble(trait_id, type_id);
    }

    std::string GetTraitAsString(size_t id) const { return data_map.GetAsString(id); }

    std::string GetTraitAsString(size_t trait_id, emp::TypeID type_id, size_t count=1) const {
      return data_map.GetAsString(trait_id, type_id, count);
    }
  };

}

#endif // #ifndef EMP_DATA_ANNOTATEDTYPE_HPP_INCLUDE
