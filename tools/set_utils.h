//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This file contains a set of simple functions to manipulate std::set objects.
//  Status: ALPHA (just one function so far..)

#ifndef EMP_SET_UTILS_H
#define EMP_SET_UTILS_H

#include <set>

namespace emp {

  template <typename T>
  void insert(std::set<T> & s1, const std::set<T> & s2) {
    s1.insert(s2.begin(), s2.end());
  }

  template <typename T>
  bool Has(const std::set<T> & s, const T & val) {
    return s.count(val);
  }
}

#endif
