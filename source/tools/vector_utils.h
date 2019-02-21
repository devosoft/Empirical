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

#include <algorithm>
#include <functional>

#include "../base/vector.h"

namespace emp {

  /// Return the first position of a value in a vector (or -1 if none exists)
  template <typename T>
  int FindValue(const emp::vector<T> & v, const T & val, size_t start_pos=0) {
    for (size_t i = start_pos; i < v.size(); i++) {
      if (v[i] == val) return (int) i;
    }
    return -1;
  }

  /// Remove the first value after start_pos with a given value.  Return if removal successful.
  template <typename T>
  bool RemoveValue(emp::vector<T> & v, const T & val, size_t start_pos=0) {
    int pos = FindValue(v, val, start_pos);
    if (pos == -1) return false;
    v.erase(v.begin() + pos);
    return true;
  }

  /// Return whether a value exists in a vector.s
  template <typename T>
  bool Has(const emp::vector<T> & v, const T & val) {
    return FindValue(v, val) >= 0;
  }

  /// Print the contects of a vector.
  template <typename T>
  void Print(const emp::vector<T> & v, std::ostream & os=std::cout, const std::string & spacer=" ") {
    for (size_t id = 0; id < v.size(); id++) {
      if (id) os << spacer; // Put a space before second element and beyond.
      os << v[id];
    }
  }

  /// Find the index with the "optimal" value (picks first in cases of a tie).
  /// @param v Any object allowing indexing (e.g. vector, array, etc.)
  /// @param fun Comparison function; returns true if the first value os more optimal than second.
  template <typename T>
  size_t FindIndex(const T & v,
                   const std::function<bool(typename T::value_type, typename T::value_type)> & fun) {
    emp_assert(v.size() > 0);
    using v_type = typename T::value_type;
    v_type best_val = v[0];
    size_t best_index = 0;
    for (size_t i = 1; i < v.size(); i++) {
      if (fun(v[i], best_val)) {
        best_val = v[i];
        best_index = i;
      }
    }
    return best_index;
  }

  /// Find the index with the minimal value (picks first in cases of a tie).
  template <typename T>
  size_t FindMinIndex(const T & v) {
    using v_type = typename T::value_type;
    return FindIndex(v, [](v_type a, v_type b){ return a < b; });
  }

  /// Find the index with the maximal value (picks first in cases of a tie).
  template <typename T>
  size_t FindMaxIndex(const T & v) {
    using v_type = typename T::value_type;
    return FindIndex(v, [](v_type a, v_type b){ return a > b; });
  }

  /// Find the minimum value in a vector.
  template <typename T>
  T FindMin(const emp::vector<T> & v) { return v[ FindMinIndex(v) ]; }

  /// Find the maximum value in a vector.
  template <typename T>
  T FindMax(const emp::vector<T> & v) { return v[ FindMaxIndex(v) ]; }

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

  /// Scale all elements of a vector by the same value.
  template <typename T>
  void Scale(emp::vector<T> & v, T scale) {
    for (T & x : v) x *= scale;
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

  /// Swap the order of a vector of vectors.  That is, swap rows and columns.
  /// NOTE: All rows must be the same size or smaller than those above for this to work.
  template <typename T>
  emp::vector< emp::vector< T > > Transpose( const emp::vector< emp::vector< T > > & in_vv ) {
    // If the input vector-of-vectors (in_vv) is empty, return it since inversion is trivial.
    if (in_vv.size() == 0) return in_vv;

    // Setup the new vector to have a number of rows equal to number of cols in original.
    emp::vector< emp::vector< T > > out_vv(in_vv[0].size());

    // Assuming a rectangular matrix, reserve enough space to fit each row!
    for (auto & row : out_vv) row.reserve(in_vv.size());

    // Copy over all of the data!
    for (size_t i = 0; i < in_vv.size(); i++) {
      emp_assert(i == 0 || in_vv[i].size() <= in_vv[i-1].size(),
                 "Cannot invert a matrix with increasing row length.",
                 i, in_vv[i].size(), in_vv[i-1].size());
      for (size_t j = 0; j < in_vv[i].size(); j++) {
        out_vv[j].push_back( in_vv[i][j] );
      }
    }

    return out_vv;
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
