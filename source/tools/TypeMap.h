/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  TypeMap.h
 *  @brief A class that will map types to values of a designated type.
 *  @note Status: BETA
 */

#ifndef EMP_TYPE_MAP_H
#define EMP_TYPE_MAP_H

#include <unordered_map>

#include "../meta/TypeID.h"

namespace emp {

  /// A map of types to objects.
  template <typename T>
  class TypeMap {
  private:
    // Internally, types are converted to numbers, so we will buld a map of size_t to the desired value
    // type.  Since order doesn't make sense for types, we'll use unordered_map for the speedup.
    using val_map_t = std::unordered_map<size_t, T>;
    val_map_t val_map;

  public:
    using mapped_type = T;
    using iterator = typename val_map_t::iterator;
    using const_iterator = typename val_map_t::const_iterator;
    using local_iterator = typename val_map_t::local_iterator;
    using const_local_iterator = typename val_map_t::const_local_iterator;
    using size_type = typename val_map_t::size_type;
    using difference_type = typename val_map_t::difference_type;

    bool empty() const noexcept { return val_map.empty(); }
    size_t size() const noexcept { return val_map.size(); }
    size_t max_size() const noexcept { return val_map.max_size(); }

    iterator begin() noexcept { return val_map.begin();  }
    const_iterator begin() const noexcept { return val_map.begin(); }
    const_iterator cbegin() const noexcept { return val_map.begin(); }

    iterator end() noexcept { return val_map.end(); }
    const_iterator end() const noexcept { return val_map.end();  }
    const_iterator cend() const noexcept { return val_map.end();  }
    
    template <typename INDEX_TYPE>
    mapped_type & Get() { return val_map[GetTypeValue<INDEX_TYPE>()]; }

    template <typename INDEX_TYPE>
    mapped_type & at() { return val_map.at(GetTypeValue<INDEX_TYPE>()); }

    template <typename INDEX_TYPE>
    const mapped_type & at() const { return val_map.at(GetTypeValue<INDEX_TYPE>()); }

    template <typename INDEX_TYPE>
    iterator find() { return val_map.find( GetTypeValue<INDEX_TYPE>() ); }

    template <typename INDEX_TYPE>
    const_iterator find() const { return val_map.find( GetTypeValue<INDEX_TYPE>() ); }

    template <typename INDEX_TYPE>
    size_t count() const { return val_map.count( GetTypeValue<INDEX_TYPE>() ); }

    iterator erase (const_iterator position) { return val_map.erase(position); }

    template <typename INDEX_TYPE>
    size_type erase() { return val_map.erase( GetTypeValue<INDEX_TYPE>() ); }

    void clear() noexcept { val_map.clear(); }
  };
}

#endif
