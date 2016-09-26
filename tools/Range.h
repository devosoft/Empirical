//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  A simple way to track value ranges [lower, upper)

#ifndef EMP_RANGE_H
#define EMP_RANGE_H

#include "assert.h"

namespace emp {

  template <typename T>
  struct Range {
    T lower;
    T upper;

    Range(T _l, T _u) : lower(_l), upper(_u) { emp_assert(_l < _u); }

    Range & operator=(const Range&) = default;
    bool operator==(const Range& _in) const { return lower==_in.lower && upper==_in.upper; }
    bool operator!=(const Range& _in) const { return lower!=_in.lower || upper!=_in.upper; }

    void Set(T _l, T _u) { emp_assert(_l < _u); lower = _l; upper = _u; }
    bool Valid(T value) { return value >= lower && value < upper; }
  };

  template <typename T> Range<T> MakeRange(T _l, T _u) { return Range<T>(_l,_u); }
  Range<int> IntRange(int _l, int _u) { return Range<int>(_l,_u); }
  Range<double> DRange(double _l, double _u) { return Range<double>(_l,_u); }
}

#endif
