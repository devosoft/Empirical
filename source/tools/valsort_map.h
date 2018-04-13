/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  valsort_map.h
 *  @brief This file defines a map that is sorted by value, not key.
 *  @note Status: ALPHA
 */

#ifndef EMP_VALSORT_MAP_H
#define EMP_VALSORT_MAP_H

#include <map>

#include "../base/vector.h"

template <class Key,        // key_type
          class T           // mapped_type
          // @CAO currently not handling special compare or alloc types.
           >
class valsort_map {
public: // internal types
  using key_type = Key;
  using mapped_type = T;
  using value_type = pair<const Key,T>;
private:
  std::map<Key,T> item_map;
  std::set< value_type > val_set;

public:
  valsort_map() : item_map(), val_set() { ; }
  valsort_map(const valsort_map &) = default;
  valsort_map(valsort_map &&) = default;
  ~valsort_map() { ; }

  const mapped_type & Get(key_type key) {
    return item_map[key];
  }

  void Set(key_type key, const mapped_type & value) {
    auto map_it = item_map.find(key);
    if (map_it != item_map.end()) val_set.remove(*map_it);
    map_it->second = value;
    val_set.insert(*map_it);
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

#endif


