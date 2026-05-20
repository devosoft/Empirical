/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2018-2022 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/math/Histogram.hpp
 * @brief Simple tools for tracking a set of small integer counts.
 * @note Status: ALPHA
 **/

#pragma once

#ifndef INCLUDE_EMP_MATH_HISTOGRAM_HPP_GUARD
#define INCLUDE_EMP_MATH_HISTOGRAM_HPP_GUARD

#include "../base/vector.hpp"

namespace emp {

  class Histogram {
  private:
    emp::vector<size_t> counts;

  public:
    [[nodiscard]] size_t size() const { return counts.size(); }
    void reserve(size_t in) { counts.reserve(in); }

    [[nodiscard]] size_t Get(size_t pos) const {
      if (pos >= counts.size()) return 0;
      return counts[pos];
    }
    [[nodiscard]] size_t operator[](size_t pos) const { return Get(pos); }

    void Reset() { counts.resize(0); }

    void Insert(size_t pos) {
      if (counts.size() <= pos) { counts.resize(pos+1, 0); }
      ++counts[pos];
    }

    void Remove(size_t pos) {
      emp_assert(pos < counts.size());
      emp_assert(counts[pos] > 0);
      --counts[pos];
    }

    Histogram & operator+=(const Histogram & in) {
      if (counts.size() < in.size()) counts.resize(in.size(), 0);
      for (size_t i = 0; i < in.counts.size(); ++i) {
        counts[i] += in.counts[i];
      }
      return *this;
    }
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_MATH_HISTOGRAM_HPP_GUARD
