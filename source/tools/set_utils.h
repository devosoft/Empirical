/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2017
 *
 *  @file set_utils.h
 *  @brief Tools to save and load data from classes.
 *  @note Status: ALPHA
 */


#ifndef EMP_SET_UTILS_H
#define EMP_SET_UTILS_H

#include <set>
#include <unordered_set>

#include "../base/vector.h"

namespace emp {

  /// Insert the full contents of s2 into s1.
  template <typename T>
  void insert(std::set<T> & s1, const std::set<T> & s2) {
    s1.insert(s2.begin(), s2.end());
  }

  /// Test if an std::set has a particular element without modifying the set in any way.
  template <typename T, typename H>
  bool Has(const std::set<T,H> & s, const T & val) { return s.count(val); }

  /// Test if an std::multiset has a particular element without modifying the set in any way.
  template <typename T, typename H>
  bool Has(const std::multiset<T,H> & s, const T & val) { return s.count(val); }

  /// Test if an std::unordered_set has a particular element without modifying the set in any way.
  template <typename T, typename H>
  bool Has(const std::unordered_set<T,H> & s, const T & val) { return s.count(val); }

  /// Test if an std::unordere_multiset has a particular element without modifying the set in any way.
  template <typename T, typename H>
  bool Has(const std::unordered_multiset<T,H> & s, const T & val) { return s.count(val); }

  template <typename T>
  std::set<T> difference(std::set<T> s1, std::set<T> s2) {
    // Based on PierreBdR's answer to https://stackoverflow.com/questions/283977/c-stl-set-difference
    std::set<T> result;
    std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.end()));
    return result;
  }

  template <typename T>
  std::set<T> difference(emp::vector<T> s1, emp::vector<T> s2) {
    // Based on PierreBdR's answer to https://stackoverflow.com/questions/283977/c-stl-set-difference
    std::set<T> result;
    std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.end()));
    return result;
  }

  template <typename T>
  std::set<T> difference(std::set<T> s1, emp::vector<T> s2) {
    // Based on PierreBdR's answer to https://stackoverflow.com/questions/283977/c-stl-set-difference
    std::set<T> result;
    std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.end()));
    return result;
  }

  template <typename T>
  std::set<T> difference(emp::vector<T> s1, std::set<T> s2) {
    // Based on PierreBdR's answer to https://stackoverflow.com/questions/283977/c-stl-set-difference
    std::set<T> result;
    std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.end()));
    return result;
  }

  template <typename T>
  std::set<T> intersection(std::set<T> s1, std::set<T> s2) {
    std::set<T> result;
    std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.end()));
    return result;
  }

  template <typename T>
  std::set<T> intersection(emp::vector<T> s1, emp::vector<T> s2) {
    std::set<T> result;
    std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.end()));
    return result;
  }

  template <typename T>
  std::set<T> intersection(std::set<T> s1, emp::vector<T> s2) {
    std::set<T> result;
    std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.end()));
    return result;
  }

  template <typename T>
  std::set<T> intersection(emp::vector<T> s1, std::set<T> s2) {
    std::set<T> result;
    std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.end()));
    return result;
  }


}

#endif
