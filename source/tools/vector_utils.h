/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017-2018
 *
 *  @file vector_utils.h
 *  @brief A set of simple functions to manipulate emp::vector
 *  @note Status: BETA
 */

#ifndef EMP_VECTOR_UTILS_H
#define EMP_VECTOR_UTILS_H

#include "../base/vector.h"

namespace emp {

  /// Return the first position of a value in a vector (or -1 if none exists)
  template <typename T>
  int FindPos(const emp::vector<T> vec, const T & val) {
    for (size_t i = 0; i < vec.size(); i++) {
      if (vec[i] == val) return (int) i;
    }
    return -1;
  }

  /// Return whether a value exists in a vector.
  template <typename T>
  bool Has(const emp::vector<T> vec, const T & val) {
    return FindPos(vec, val) >= 0;
  }

  /// Print the contects of a vector.
  template <typename T>
  void Print(const emp::vector<T> & v, std::ostream & os=std::cout, const std::string & spacer=" ") {
    for (size_t id = 0; id < v.size(); id++) {
      if (id) os << spacer; // Put a space before second element and beyond.
      os << v[id];
    }
  }

  /// Sum up the contents of a vector.
  template <typename T>
  T Sum(const emp::vector<T> & v) {
    T sum = 0;
    for (auto x : v) sum += x;
    return sum;
  }

  /// Multiply all of the contents of a vector.
  template <typename T>
  T Product(const emp::vector<T> & v) {
    T product = 1;
    for (auto x : v) product *= x;
    return product;
  }

  /// A quick shortcut for sorting a vector.
  template <typename T, typename... Ts>
  void Sort(emp::vector<T> & v, Ts... args) {
    std::sort(v.begin(), v.end(), std::forward<Ts>(args)...);
  }

  /// Returns a vector containing a chunk of elements from @param vec
  /// starting at @param start and going up to but not including @param stop.
  template <typename T>
  emp::vector<T> Slice(emp::vector<T> vec, int start, int stop) {
    emp_assert(start < stop, start, stop);
    emp_assert(start < (int)vec.size(), start, vec.size());
    emp_assert(stop <= (int)vec.size(), stop, vec.size());

    emp::vector<T> new_vec;
    for (int i = start; i < stop; i++){
      new_vec.push_back(vec[i]);
    }
    return new_vec;
  }

  /// Tree manipulation in vectors.
  constexpr size_t tree_left(size_t id) { return id*2+1; }
  constexpr size_t tree_right(size_t id) { return id*2+2; }
  constexpr size_t tree_parent(size_t id) { return (id-1)/2; }

  // == Heap manipulation ==

  /// Heapify an individual node in a vector.
  template <typename T>
  bool Heapify(emp::vector<T> & v, size_t id) {
    const size_t id_left = tree_left(id);
    if (id_left >= v.size()) return false;  // Nothing left to heapify.

    const T val = v[id];
    const T val_left = v[id_left];

    const size_t id_right = tree_right(id);
    if (id_right < v.size()) {
      const T val_right = v[id_right];
      if (val_right > val_left && val_right > val) {
        v[id] = val_right;
        v[id_right] = val;
        Heapify(v, id_right);
        return true;
      }
    }

    if (val_left > val) {
      v[id] = val_left;
      v[id_left] = val;
      Heapify(v, id_left);
      return true;
    }

    return false;  // No changes need to be made.
  }

  /// Heapify all elements in a vector.
  template <typename T>
  void Heapify(emp::vector<T> & v) {
    size_t id = v.size();
    while (id-- > 0) emp::Heapify(v, id);
  }

  /// Extraxt maximum element from a heap.
  template <typename T>
  T HeapExtract(emp::vector<T> & v) {
    emp_assert(v.size(), "Cannot extract from an empty heap!");
    T out = v[0];
    if (v.size() > 1) {
      const size_t last_pos = v.size() - 1;
      v[0] = v[last_pos];
      v.resize(last_pos);
      Heapify(v,0);
    }
    else v.resize(0);
    return out;
  }

  /// Insert a new element into a heap.
  template <typename T>
  void HeapInsert(emp::vector<T> & v, T val) {
    size_t pos = v.size();
    v.push_back(val);
    while (pos > 0) {
      pos = tree_parent(pos);
      if (!Heapify(v,pos)) break;
    }
  }

}

#endif
