/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  valsort_map.h
 *  @brief This file defines a map that is sorted by value, not key.
 *  @note Status: ALPHA
 * 
 *  valsort_map is a limited implementation of map that also keeps track of the values set, in
 *  order, as you go.  This is accomplished by keeping a second data structure, a sorted set, of
 *  all of the values maintained in the map.
 * 
 *  The member functions cbegin() and cend() work as in map, but cvbegin() and cvend() produce
 *  the same set of results, except in value order.  (reverse versions of each work as well)
 */

#ifndef EMP_VALSORT_MAP_H
#define EMP_VALSORT_MAP_H

#include <map>
#include <set>

#include "../base/vector.h"

namespace emp {

  template <class Key,        // key_type
            class T           // mapped_type
            // @CAO currently not handling special compare or alloc types.
            >
  class valsort_map {
  public: // internal types
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<const Key,T>;
  private:
    struct SortOrder {
      bool operator()(const value_type & in1, const value_type & in2) const {
        if (in1.second == in2.second) return (in1.first < in2.first);
        return in1.second < in2.second;
      }
    };

    std::map<Key,T> item_map;
    std::set< value_type, SortOrder > val_set;

  public:
    valsort_map() : item_map(), val_set() { ; }
    valsort_map(const valsort_map &) = default;
    valsort_map(valsort_map &&) = default;
    ~valsort_map() { ; }

    size_t size() const { return item_map.size(); }

    const mapped_type & Get(key_type key) {
      return item_map[key];
    }

    void Set(key_type key, const mapped_type & value) {
      auto map_it = item_map.find(key);
      if (map_it != item_map.end()) {
        val_set.erase(*map_it);    // Erase the old pair from the set.
        map_it->second = value;    // Update the map
        val_set.emplace(*map_it);  // Place the new map entry into the set.
      } else {
        item_map[key] = value;
        val_set.emplace(*item_map.find(key));
      }
    }

    // For now, don't change values using iterators, just look at them.
    auto cbegin() { return item_map.cbegin(); }
    auto cend() { return item_map.cend(); }
    auto crbegin() { return item_map.crbegin(); }
    auto crend() { return item_map.crend(); }

    // Add iterators that are ordred by value instead of key.
    auto cvbegin() { return val_set.cbegin(); }
    auto cvend() { return val_set.cend(); }
    auto crvbegin() { return val_set.crbegin(); }
    auto crvend() { return val_set.crend(); }

  };
}

#endif


