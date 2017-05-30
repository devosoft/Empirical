//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  A simple way to track value ranges
//  Status: BETA

#ifndef EMP_RANGE_H
#define EMP_RANGE_H

#include "../base/assert.h"
#include "../base/vector.h"

namespace emp {

  template <typename T>
  class Range {
  private:
    T lower;
    T upper;

  public:
    Range(T _l, T _u) : lower(_l), upper(_u) { emp_assert(_l < _u); }

    T GetLower() const { return lower; }
    T GetUpper() const { return upper; }

    Range & operator=(const Range&) = default;
    bool operator==(const Range& _in) const { return lower==_in.lower && upper==_in.upper; }
    bool operator!=(const Range& _in) const { return lower!=_in.lower || upper!=_in.upper; }

    void SetLower(T l) { lower = l; }
    void SetUpper(T u) { upper = u; }
    void Set(T _l, T _u) { emp_assert(_l < _u); lower = _l; upper = _u; }

    bool Valid(T value) { return value >= lower && value <= upper; }

    emp::vector<T> Spread(size_t s) {
      emp_assert(s >= 1);
      emp::vector<T> out(s);
      out[0] = lower;
      if (s > 1) {
        T range = upper - lower;
        for (size_t i = 1; i < s; i++) {
          out[i] = lower + (T) ((((double) i) / (double)(s-1)) * range);
        }
      }
      return out;
    }
  };

  template <typename T> Range<T> MakeRange(T _l, T _u) { return Range<T>(_l,_u); }
  inline Range<int> IntRange(int _l, int _u) { return Range<int>(_l,_u); }
  inline Range<double> DRange(double _l, double _u) { return Range<double>(_l,_u); }
}

#endif
