//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This file contains a set of simple functions to manipulate strings.

#ifndef EMP_VECTOR_UTILS_H
#define EMP_VECTOR_UTILS_H

#include "vector.h"

namespace emp {

  template <typename T>
  void Print(const emp::vector<T> & v, std::ostream & os=std::cout, const std::string & spacer=" ") {
    for (size_t id = 0; id < v.size(); id++) {
      if (id) os << spacer; // Put a space before second element and beyond.
      os << v[id];
    }
  }

  // Tree manipulation in vectors.
  constexpr size_t tree_left(size_t id) { return id*2+1; }
  constexpr size_t tree_right(size_t id) { return id*2+2; }
  constexpr size_t tree_parent(size_t id) { return (id-1)/2; }

  // == Heap manipulation ==

  // Heapify an individual node.
  template <typename T>
  void Heapify(emp::vector<T> & v, size_t id) {
    const size_t id_left = tree_left(id);
    if (id_left >= v.size()) return;  // Nothing left to heapify.

    const T val = v[id];
    const T val_left = v[id_left];

    const size_t id_right = tree_right(id);
    if (id_right < v.size()) {
      const T val_right = v[id_right];
      if (val_right > val_left && val_right > val) {
        v[id] = val_right;
        v[id_right] = val;
        Heapify(v, id_right);
        return;
      }
    }

    if (val_left > val) {
      v[id] = val_left;
      v[id_left] = val;
      Heapify(v, id_left);
    }
  }

  // Heapify all elements in a vector.
  template <typename T>
  void Heapify(emp::vector<T> & v) {
    size_t id = v.size();
    while (id-- > 0) emp::Heapify(v, id);
  }
}

#endif
