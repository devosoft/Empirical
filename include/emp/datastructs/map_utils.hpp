/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2017
 *
 *  @file map_utils.hpp
 *  @brief A set of simple functions to manipulate maps.
 *  @note Status: BETA
 */

#ifndef EMP_DATASTRUCTS_MAP_UTILS_HPP_INCLUDE
#define EMP_DATASTRUCTS_MAP_UTILS_HPP_INCLUDE

#include <map>
#include <unordered_map>

#include "../base/map.hpp"
#include "../base/vector.hpp"

namespace emp {

  /// Take any map type, and run find to determine if a key is present.
  template <class MAP_T, class KEY_T>
  inline bool Has( const MAP_T & in_map, const KEY_T & key ) {
    return in_map.find(key) != in_map.end();
  }


  template <class MAP_T>
  inline auto Keys( const MAP_T & in_map) -> emp::vector<typename std::remove_const<decltype(in_map.begin()->first)>::type> {
    using KEY_T = typename std::remove_const<decltype(in_map.begin()->first)>::type;
    emp::vector<KEY_T> keys;
    for (auto it : in_map) {
      keys.push_back(it.first);
    }

    return keys;

  }


  /// Take any map, run find() member function, and return the result found
  /// (or default value if no results found).
  template <class MAP_T, class KEY_T>
  inline auto Find( const MAP_T & in_map, const KEY_T & key, const typename MAP_T::mapped_type & dval) {
    auto val_it = in_map.find(key);
    if (val_it == in_map.end()) return dval;
    return val_it->second;
  }


  /// Take any map and element, run find() member function, and return a reference to
  /// the result found (or default value if no results found).
  template <class MAP_T, class KEY_T>
  inline const auto & FindRef( const MAP_T & in_map, const KEY_T & key, const typename MAP_T::mapped_type & dval) {
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

  // If debug is turned on, support flip_map on emp::map/multimap.
  // Otherwise, emp::map and emp::multimap compile down to std::map/std::multimap, so this function is equivalent to flip_map above.
  #ifndef EMP_NDEBUG
  /// Take an emp::map<A,B> and return the flipped map (now multimap to be safe): emp::multimap<B,A>
  template<typename A, typename B> emp::multimap<B,A> flip_map(const emp::map<A,B> &src)
  {
    emp::multimap<B,A> dst;
    for (const auto & x : src) dst.insert( flip_pair(x) );
    return dst;
  }
  #endif

}

#endif // #ifndef EMP_DATASTRUCTS_MAP_UTILS_HPP_INCLUDE
