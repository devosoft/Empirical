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
};

#endif


