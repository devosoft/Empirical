/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2018
 *
 *  @file set_utils.h
 *  @brief Tools to save and load data from classes.
 *  @note Status: ALPHA
 */


#ifndef EMP_SET_UTILS_H
#define EMP_SET_UTILS_H

#include <set>
#include <unordered_set>
#include <algorithm>

#include "../base/vector.h"

namespace emp {

  /// Insert the full contents of s2 into s1.
  template <typename T>
  void insert(std::set<T> & s1, const std::set<T> & s2) {
    s1.insert(s2.begin(), s2.end());
  }

  /// Test if an std::set has a particular element without modifying the set in any way.
  template <typename T, typename H, typename V>
  bool Has(const std::set<T,H> & s, const V & val) { return s.count(val); }

  /// Test if an std::multiset has a particular element without modifying the set in any way.
  template <typename T, typename H, typename V>
  bool Has(const std::multiset<T,H> & s, const V & val) { return s.count(val); }

  /// Test if an std::unordered_set has a particular element without modifying the set in any way.
  template <typename T, typename H, typename V>
  bool Has(const std::unordered_set<T,H> & s, const V & val) { return s.count(val); }

  /// Test if an std::unordere_multiset has a particular element without modifying the set in any way.
  template <typename T, typename H, typename V>
  bool Has(const std::unordered_multiset<T,H> & s, const V & val) { return s.count(val); }

  // Note: the following functions allow the use of sets or vectors. Sets are passed
  // by reference and vectors are passed by value, because these functions only work
  // on sorted data. Therefore, vectors must be sorted first, which happens in place.

  /// Compute the set difference of @param s1 and @param s2 (elements that are in S1 but no S2)
  template <typename T>
  std::set<T> difference(std::set<T> & s1, std::set<T> & s2) {
    // Based on PierreBdR's answer to https://stackoverflow.com/questions/283977/c-stl-set-difference
    std::set<T> result;
    std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.end()));
    return result;
  }

  /// Compute the set difference of @param s1 and @param s2 (elements that are in S1 but no S2)
  template <typename T>
  std::set<T> difference(emp::vector<T> s1, emp::vector<T> & s2) {
    // Based on PierreBdR's answer to https://stackoverflow.com/questions/283977/c-stl-set-difference
    std::sort(s1.begin(), s1.end()); // set_difference expects sorted things
    std::sort(s2.begin(), s2.end()); // set_difference expects sorted things

    std::set<T> result;
    std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.end()));
    return result;
  }

  /// Compute the set difference of @param s1 and @param s2 (elements that are in S1 but not S2)
  template <typename T>
  std::set<T> difference(std::set<T> & s1, emp::vector<T> s2) {
    // Based on PierreBdR's answer to https://stackoverflow.com/questions/283977/c-stl-set-difference
    std::sort(s2.begin(), s2.end()); // set_difference expects sorted things
    std::set<T> result;
    std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.end()));
    return result;
  }

  /// Compute the set difference of @param s1 and @param s2 (elements that are in S1 but no S2)
  template <typename T>
  std::set<T> difference(emp::vector<T> s1, std::set<T> & s2) {
    // Based on PierreBdR's answer to https://stackoverflow.com/questions/283977/c-stl-set-difference
    std::sort(s1.begin(), s1.end()); // set_difference expects sorted things
    std::set<T> result;
    std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.end()));
    return result;
  }

  /// Compute the set intersection of @param s1 and @param s2 (elements that are in both S1 and S2)
  template <typename T>
  std::set<T> intersection(std::set<T> & s1, std::set<T> & s2) {
    std::set<T> result;
    std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.end()));
    return result;
  }

  /// Compute the set intersection of @param s1 and @param s2 (elements that are in both S1 and S2)
  template <typename T>
  std::set<T> intersection(emp::vector<T> s1, emp::vector<T> s2) {
    std::sort(s1.begin(), s1.end()); // set_intersection expects sorted things
    std::sort(s2.begin(), s2.end()); // set_intersection expects sorted things

    std::set<T> result;
    std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.end()));
    return result;
  }

  /// Compute the set intersection of @param s1 and @param s2 (elements that are in both S1 and S2)
  template <typename T>
  std::set<T> intersection(std::set<T> & s1, emp::vector<T> s2) {
    std::sort(s2.begin(), s2.end()); // set_intersection expects sorted things

    std::set<T> result;
    std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.end()));
    return result;
  }

  /// Compute the set intersection of @param s1 and @param s2 (elements that are in both S1 and S2)
  template <typename T>
  std::set<T> intersection(emp::vector<T> s1, std::set<T> & s2) {
    std::sort(s1.begin(), s1.end()); // set_intersection expects sorted things

    std::set<T> result;
    std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.end()));
    return result;
  }

  /// Compute the set union of @param s1 and @param s2 (elements that are in either S1 or S2)
  template <typename T>
  std::set<T> set_union(std::set<T> & s1, std::set<T> & s2) {
    std::set<T> result;
    std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.end()));
    return result;
  }

  /// Compute the set union of @param s1 and @param s2 (elements that are in either S1 or S2)
  template <typename T>
  std::set<T> set_union(emp::vector<T> s1, emp::vector<T> s2) {
    std::sort(s1.begin(), s1.end()); // set_union expects sorted things
    std::sort(s2.begin(), s2.end()); // set_union expects sorted things

    std::set<T> result;
    std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.end()));
    return result;
  }

  /// Compute the set union of @param s1 and @param s2 (elements that are in either S1 or S2)
  template <typename T>
  std::set<T> set_union(std::set<T> & s1, emp::vector<T> s2) {
    std::sort(s2.begin(), s2.end()); // set_union expects sorted things

    std::set<T> result;
    std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.end()));
    return result;
  }

  /// Compute the set union of @param s1 and @param s2 (elements that are in either S1 or S2)
  template <typename T>
  std::set<T> set_union(emp::vector<T> s1, std::set<T> & s2) {
    std::sort(s1.begin(), s1.end()); // set_union expects sorted things

    std::set<T> result;
    std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.end()));
    return result;
  }

 /// Compute the set symmetric_difference of @param s1 and @param s2 (elements that are in either S1 or S2 but not both)
  template <typename T>
  std::set<T> symmetric_difference(std::set<T> & s1, std::set<T> & s2) {
    std::set<T> result;
    std::set_symmetric_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.end()));
    return result;
  }

  /// Compute the set symmetric_difference of @param s1 and @param s2 (elements that are in either S1 or S2 but not both)
  template <typename T>
  std::set<T> symmetric_difference(emp::vector<T> s1, emp::vector<T> s2) {
    std::sort(s1.begin(), s1.end()); // set_symmetric_difference expects sorted things
    std::sort(s2.begin(), s2.end()); // set_symmetric_difference expects sorted things

    std::set<T> result;
    std::set_symmetric_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.end()));
    return result;
  }

  /// Compute the set symmetric_difference of @param s1 and @param s2 (elements that are in either S1 or S2 but not both)
  template <typename T>
  std::set<T> symmetric_difference(std::set<T> & s1, emp::vector<T> s2) {
    std::sort(s2.begin(), s2.end()); // set_symmetric_difference expects sorted things

    std::set<T> result;
    std::set_symmetric_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.end()));
    return result;
  }

  /// Compute the set symmetric_difference of @param s1 and @param s2 (elements that are in either S1 or S2 but not both)
  template <typename T>
  std::set<T> symmetric_difference(emp::vector<T> s1, std::set<T> & s2) {
    std::sort(s1.begin(), s1.end()); // set_symmetric_difference expects sorted things

    std::set<T> result;
    std::set_symmetric_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.end()));
    return result;
  }


}

#endif
