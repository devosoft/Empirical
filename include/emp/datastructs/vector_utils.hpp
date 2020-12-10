/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017-2020.
 *
 *  @file vector_utils.hpp
 *  @brief A set of simple functions to manipulate emp::vector
 *  @note Status: BETA
 *
 *
 *  @note consider adding a work-around to avoid vector<bool> ?
 */

#ifndef EMP_VECTOR_UTILS_H
#define EMP_VECTOR_UTILS_H

#include <numeric>
#include <set>
#include <algorithm>
#include <functional>
#include <limits>

#include "../base/vector.hpp"

namespace emp {

  /// Base case for Append; we just have a single vector with nothing to append.
  template <typename T>
  emp::vector<T> & Append(emp::vector<T> & base) {
    return base;
  }

  /// Append one or more vectors on to the end of an existing vector.
  template <typename T, typename V1, typename... Vs>
  emp::vector<T> & Append(emp::vector<T> & base, const V1 & v1, const Vs &... vs) {
    // If the next entry is a single element, push it on the back.
    if constexpr (std::is_convertible<T, V1>()) {
      base.push_back(v1);
    }

    // Otherwise assume we have a container and append all of it.
    else {
      base.insert(base.end(), v1.begin(), v1.end());
    }

    // Recurse.
    return Append(base, vs...);
  }


  /// Concatonate two or more vectors together, creating a new vector.
  template <typename T, typename... Vs>
  emp::vector<T> Concat(const emp::vector<T> & v1, const Vs &... vs) {
    emp::vector<T> out_v = v1;
    Append(out_v, vs...);
    return out_v;
  }

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

  /// Return whether a value exists in a vector
  template <typename T>
  bool Has(const emp::vector<T> & v, const T & val) {
    return FindValue(v, val) >= 0;
  }

  /// Return number of times a value occurs in a vector
  template <typename T>
  int Count(const emp::vector<T> & vec, const T & val) {
    return std::count (vec.begin(), vec.end(), val);
  }

  /// Print the contents of a vector.
  template <typename T>
  void Print(const emp::vector<T> & v, std::ostream & os=std::cout, const std::string & spacer=" ") {
    for (size_t id = 0; id < v.size(); id++) {
      if (id) os << spacer; // Put a space before second element and beyond.
      os << v[id];
    }
  }

  /// Find the first index where the provided function returns true; return -1 otherwise.
  template <typename T, typename FUN>
  int FindEval(const emp::vector<T> & v, const FUN & fun, size_t start_pos=0) {
    for (size_t i = start_pos; i < v.size(); i++) {
      if (fun(v[i])) return (int) i;
    }
    return -1;
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


  /// Sum all of the contents of a vector.
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

  /// Collapse a vector of vectors into a single vector.
  template <typename T>
  emp::vector<T> Flatten( const emp::vector< emp::vector< T > > & vv ) {
    size_t element_count = 0;
    for (const auto & v : vv) element_count += v.size();

    emp::vector<T> out_v;
    out_v.reserve(element_count);

    for (const auto & v : vv) out_v.insert(out_v.end(), v.begin(), v.end());

    return out_v;
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
  
  /// Returns a vector containing the numbers from @param N1 to @param N2
  // from https://stackoverflow.com/questions/13152252/is-there-a-compact-equivalent-to-python-range-in-c-stl
  template <typename T>
  emp::vector <T> NRange(T N1, T N2) {
      emp::vector<T> numbers(N2-N1);
      std::iota(numbers.begin(), numbers.end(), N1);
      return numbers;
  }

  /// Return a new vector containing the same elements as @param v
  /// with any duplicate elements removed. 
  /// Not guarunteed to preserve order
  template <typename T>
  emp::vector<T> RemoveDuplicates(const emp::vector<T> & v) {
    std::set<T> temp_set(v.begin(), v.end());
    emp::vector<T> new_vec(temp_set.begin(), temp_set.end());
    return new_vec;
  }

  /// Build a vector with a range of values from min to max at the provided step size.
  template <typename T>
  static inline emp::vector<T> BuildRange(T min, T max, T step=1) {
    emp_assert(max > min);
    size_t vsize = (size_t) ((max-min) / step) + 1;
    emp::vector<T> out_v(vsize);
    size_t pos = 0;
    for (T i = min; i < max; i += step) {
      out_v[pos++] = i;
    }
    return out_v;
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
      if (val_left < val_right && val < val_right) {
        v[id] = val_right;
        v[id_right] = val;
        Heapify(v, id_right);
        return true;
      }
    }

    if (val < val_left) {
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
    size_t ppos = tree_parent(pos);
    v.push_back(val);
    while (pos > 0 && v[ppos] < v[pos]) {
      std::swap(v[pos], v[ppos]);
      pos = ppos;
      ppos = tree_parent(pos);
    }
  }

}

#endif
