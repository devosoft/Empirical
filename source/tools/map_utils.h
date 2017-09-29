/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2017
 *
 *  @file  map_utils.h
 *  @brief A set of simple functions to manipulate maps.
 *  @note Status: BETA
 */

#ifndef EMP_MAP_UTILS_H
#define EMP_MAP_UTILS_H

#include <map>
#include <unordered_map>

namespace emp {

  /// Test if an element is present in an std::map.
  template <class KEY, class T, class Compare, class Alloc>
  inline bool Has( const std::map<KEY,T,Compare,Alloc> & in_map, const KEY & key ) {
    return in_map.find(key) != in_map.end();
  }

  /// Test if an element is present in an std::multimap.
  template <class KEY, class T, class Compare, class Alloc>
  inline bool Has( const std::multimap<KEY,T,Compare,Alloc> & in_map, const KEY & key ) {
    return in_map.find(key) != in_map.end();
  }

  /// Test if an element is present in an std::unordered_map.
  template <class KEY, class T, class Compare, class Alloc>
  inline bool Has( const std::unordered_map<KEY,T,Compare,Alloc> & in_map, const KEY & key ) {
    return in_map.find(key) != in_map.end();
  }

  /// Retrieve an std::map element if it exists, otherwise return a default.
  template <class KEY, class T, class Compare, class Alloc, class KEY2, class T2>
  inline T Find( const std::map<KEY,T,Compare,Alloc> & in_map, const KEY2 & key, const T2 & dval) {
    auto val_it = in_map.find(key);
    if (val_it == in_map.end()) return dval;
    return val_it->second;
  }

  /// Retrieve an std::unordered_map element if it exists, otherwise return a default.
  template <class KEY, class T, class Hash, class Pred, class Alloc, class KEY2, class T2>
  inline T Find( const std::unordered_map<KEY,T,Hash,Pred,Alloc> & in_map, const KEY2 & key, const T2 & dval) {
    auto val_it = in_map.find(key);
    if (val_it == in_map.end()) return dval;
    return val_it->second;
  }


  /// Retrieve an std::map element if it exists, otherwise return a default.
  template <class KEY, class T, class Compare, class Alloc, class KEY2, class T2>
  inline const T & FindRef( const std::map<KEY,T,Compare,Alloc> & in_map, const KEY2 & key, const T2 & dval) {
    auto val_it = in_map.find(key);
    if (val_it == in_map.end()) return dval;
    return val_it->second;
  }

  /// Retrieve an std::unordered_map element if it exists, otherwise return a default.
  template <class KEY, class T, class Hash, class Pred, class Alloc, class KEY2, class T2>
  inline const T & FindRef( const std::unordered_map<KEY,T,Hash,Pred,Alloc> & in_map, const KEY2 & key, const T2 & dval) {
    auto val_it = in_map.find(key);
    if (val_it == in_map.end()) return dval;
    return val_it->second;
  }


  // The following two functions are from:
  // http://stackoverflow.com/questions/5056645/sorting-stdmap-using-value

  /// Take an std::pair<A,B> and return the flipped pair std::pair<B,A>
  template<typename A, typename B> constexpr std::pair<B,A> flip_pair(const std::pair<A,B> &p)
  {
    return std::pair<B,A>(p.second, p.first);
  }

  /// Take an std::map<A,B> and return the flipped map (now multimap to be safe): std::multimap<B,A>
  template<typename A, typename B> std::multimap<B,A> flip_map(const std::map<A,B> &src)
  {
    std::multimap<B,A> dst;
    for (const auto & x : src) dst.insert( flip_pair(x) );
    return dst;
  }
}

#endif
