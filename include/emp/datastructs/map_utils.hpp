/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2016-2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/datastructs/map_utils.hpp
 * @brief A set of simple functions to manipulate maps.
 * @note Status: BETA
 */

#pragma once

#ifndef INCLUDE_EMP_DATASTRUCTS_MAP_UTILS_HPP_GUARD
#define INCLUDE_EMP_DATASTRUCTS_MAP_UTILS_HPP_GUARD

#include <algorithm>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>

#include "../base/vector.hpp"

namespace emp {

  template <typename MAP_T>
  std::string MapToString(const MAP_T & in_map) {
    std::stringstream ss;
    bool use_comma = false;
    for (const auto & [key, value] : in_map) {
      if (use_comma) { ss << ","; }
      ss << "{" << key << ":" << value << "}";
      use_comma = true;
    }
    return ss.str();
  }

  /// Take any map type, and run find to determine if a key is present.
  template <class MAP_T, class KEY_T>
  inline bool Has(const MAP_T & in_map, const KEY_T & key) {
    return in_map.find(key) != in_map.end();
  }

  /// Take a map where the value is an integer and a key.
  /// Increment value associated with that key if its present
  /// or if its not add it and set it to 1
  template <class MAP_T, class KEY_T>
  inline void IncrementCounter(MAP_T & in_map, const KEY_T & key) {
    static_assert(std::is_same<typename MAP_T::key_type, int>::value);
    if (emp::Has(in_map, key)) in_map[key]++;
    else in_map[key] = 1;
  }

  // Check to see if any of the elements in a map satisfy a function.
  template <class MAP_T, typename FUN_T>
  [[nodiscard]] bool AnyOf(const MAP_T & c, FUN_T fun) {
    using mapped_t = MAP_T::mapped_type;
    // If the provided function takes just the element type, that's all we should give it.
    if constexpr (std::is_invocable_r<bool, FUN_T, mapped_t>()) {
      return std::any_of(c.begin(), c.end(), [fun](auto x){ return fun(x.second); });
    }

    // Otherwise provide both key and element.
    else {
      return std::any_of(c.begin(), c.end(), [fun](auto x) { return fun(x.first, x.second); });
    }
  }

  // Check to see if any of the elements in a map satisfy a function.
  template <class MAP_T, typename FUN_T>
  bool AllOf(const MAP_T & c, FUN_T fun) {
    using mapped_t = MAP_T::mapped_type;

    // If the provided function takes just the element type, that's all we should give it.
    if constexpr (std::is_invocable_r<bool, FUN_T, mapped_t>()) {
      return std::all_of(c.begin(), c.end(), [fun](auto x) { return fun(x.second); });
    }

    // Otherwise provide both key and element.
    else {
      return std::all_of(c.begin(), c.end(), [fun](auto x) { return fun(x.first, x.second); });
    }
  }

  // Check to see if any of the elements in a map satisfy a function.
  template <class MAP_T, typename FUN_T>
  bool NoneOf(const MAP_T & c, FUN_T fun) {
    using mapped_t = MAP_T::mapped_type;

    // If the provided function takes just the element type, that's all we should give it.
    if constexpr (std::is_invocable_r<bool, FUN_T, mapped_t>()) {
      return std::none_of(c.begin(), c.end(), [fun](auto x) { return fun(x.second); });
    }

    // Otherwise provide both key and element.
    else {
      return std::none_of(c.begin(), c.end(), [fun](auto x) { return fun(x.first, x.second); });
    }
  }

  template <class MAP_T>
  inline auto Keys(const MAP_T & in_map) {
    emp::vector<typename MAP_T::key_type> keys;
    for (auto it : in_map) { keys.push_back(it.first); }
    return keys;
  }

  /// Take any map, run find() member function, and return the result found
  /// (or default value if no results found).
  template <class MAP_T, class KEY_T>
  inline auto Find(const MAP_T & in_map,
                   const KEY_T & key,
                   const typename MAP_T::mapped_type & dval) {
    auto val_it = in_map.find(key);
    if (val_it == in_map.end()) { return dval; }
    return val_it->second;
  }

  /// Take any map and element, run find() member function, and return a reference to
  /// the result found (or default value if no results found).
  template <class MAP_T, class KEY_T>
  inline const auto & FindRef(const MAP_T & in_map,
                              const KEY_T & key,
                              const typename MAP_T::mapped_type & dval) {
    auto val_it = in_map.find(key);
    if (val_it == in_map.end()) { return dval; }
    return val_it->second;
  }

  /// Take any map and element, run find() member function, and return a reference to
  /// the result found; trip assert if the result is not present.
  template <class MAP_T, class KEY_T>
  inline const auto & GetConstRef(const MAP_T & in_map, const KEY_T & key) {
    auto val_it = in_map.find(key);
    emp_assert(val_it != in_map.end());
    return val_it->second;
  }

  // The following two functions are from:
  // http://stackoverflow.com/questions/5056645/sorting-stdmap-using-value

  /// Take an std::pair<A,B> and return the flipped pair std::pair<B,A>
  template <typename A, typename B>
  constexpr std::pair<B, A> flip_pair(const std::pair<A, B> & p) {
    return std::pair<B, A>(p.second, p.first);
  }

  /// Take an std::map<A,B> and return the flipped map (now multimap to be safe): std::multimap<B,A>
  template <typename A, typename B, typename OUT_MAP = std::multimap<B, A>>
  OUT_MAP flip_map(const std::map<A, B> & src) {
    OUT_MAP dst;
    for (const auto & x : src) { dst.insert(flip_pair(x)); }
    return dst;
  }

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_DATASTRUCTS_MAP_UTILS_HPP_GUARD

// Local settings for Empecable file checker.
// empecable_words: dst stdmap dval
