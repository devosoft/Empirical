//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This file contains a set of simple functions to manipulate maps.

#ifndef EMP_MAP_UTILS_H
#define EMP_MAP_UTILS_H

#include <map>

namespace emp {

  // A common test it to determine if an element is present in a map.
  template <class KEY, class T, class Compare, class Alloc>
  inline bool Has( std::map<KEY,T,Compare,Alloc> in_map, const KEY & key ) {
    return in_map.find(key) != in_map.end();
  }

  // A class to retrieve a map element if it exists, otherwise return a default.
  template <class KEY, class T, class Compare, class Alloc>
  inline const T & Find( std::map<KEY,T,Compare,Alloc> in_map, const KEY & key, const T & dval) {
    auto val_it = in_map.find(key);
    if (val_it == in_map.end()) return dval;
    return val_it->second;
  }


  // The following two functions are from:
  // http://stackoverflow.com/questions/5056645/sorting-stdmap-using-value
  template<typename A, typename B> constexpr std::pair<B,A> flip_pair(const std::pair<A,B> &p)
  {
    return std::pair<B,A>(p.second, p.first);
  }

  template<typename A, typename B> std::multimap<B,A> flip_map(const std::map<A,B> &src)
  {
    std::multimap<B,A> dst;
    for (const auto & x : src) dst.insert( flip_pair(x) );
    return dst;
  }
}

#endif
