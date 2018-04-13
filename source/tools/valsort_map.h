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

template < class Key,                                     // map::key_type
           class T,                                       // map::mapped_type
           class Compare = less<Key>,                     // map::key_compare
           class Alloc = allocator<pair<const Key,T> >    // map::allocator_type
           >
class valsort_map {
public: // internal types
  using key_type = Key;
  using mapped_type = T;
  using value_type = pair<const Key,T>;
  using key_compare = Compare;
  using allocator_type = Alloc;

  using reference = value_type&;
  using const_reference = const value_type&;
private:
  std::map<Key,T,Compare,Alloc> item_map;
  std::set< value_type > val_sort;

public:
};

#endif


