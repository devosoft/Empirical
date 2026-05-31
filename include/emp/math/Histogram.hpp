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

#include <cmath>

#include "../base/vector.hpp"
#include "../datastructs/vector_utils.hpp"

namespace emp {

  class Histogram {
  private:
    mutable emp::vector<size_t> counts;  // mutable: Shrink() is a lazy normalization
    size_t num_vals = 0;

  public:
    void Serialize(emp::SerialPod & pod ) {
      pod(counts, num_vals);
    }

    [[nodiscard]] size_t size()        const { return counts.size(); }
    [[nodiscard]] size_t GetNumBins()  const { return counts.size(); }
    [[nodiscard]] size_t GetNumVals()  const { return num_vals; }
    [[nodiscard]] bool   empty()       const { return num_vals == 0; }
    void reserve(size_t in) { counts.reserve(in); }

    [[nodiscard]] size_t Get(size_t pos) const {
      if (pos >= counts.size()) return 0;
      return counts[pos];
    }
    [[nodiscard]] size_t operator[](size_t pos) const { return Get(pos); }

    void Reset() { counts.resize(0); num_vals = 0; }

    void Insert(size_t pos) {
      if (counts.size() <= pos) { counts.resize(pos+1, 0); }
      ++counts[pos];
      ++num_vals;
    }

    void Remove(size_t pos) {
      emp_assert(pos < counts.size());
      emp_assert(counts[pos] > 0);
      --counts[pos];
      --num_vals;
    }

    Histogram & operator+=(const Histogram & in) {
      if (counts.size() < in.size()) counts.resize(in.size(), 0);
      for (size_t i = 0; i < in.counts.size(); ++i) {
        counts[i] += in.counts[i];
      }
      num_vals += in.num_vals;
      return *this;
    }

    [[nodiscard]] Histogram operator+(const Histogram & in) const {
      Histogram out(*this);
      out += in;
      return out;
    }

    void Shrink() const {
      while (counts.size() && counts.back() == 0) counts.pop_back();
    }

    [[nodiscard]] size_t Min() const {
      if (num_vals == 0) return static_cast<size_t>(-1);
      size_t pos = 0;
      while (counts[pos] == 0) ++pos;
      return pos;
    }

    [[nodiscard]] size_t Max() const {
      if (num_vals == 0) return 0;
      Shrink();
      return counts.size() - 1;
    }

    // FindPosition uses 0-based indexing: target=0 returns the first value's bin,
    // target=num_vals-1 returns the last value's bin.
    [[nodiscard]] size_t FindPosition(size_t target) const {
      emp_assert(target < num_vals, target, num_vals);
      size_t pos = 0;
      size_t total = 0;
      while (total + counts[pos] <= target) {
        total += counts[pos];
        ++pos;
      }
      return pos;
    }

    [[nodiscard]] double CalcMean() const {
      if (num_vals == 0) return std::nan("");
      size_t total = 0;
      for (size_t pos = 1; pos < counts.size(); ++pos) {
        total += counts[pos] * pos;
      }
      return static_cast<double>(total) / num_vals;
    }

    [[nodiscard]] double CalcMedian() const {
      if (num_vals == 0) return std::nan("");
      if (num_vals % 2 == 1) {  // odd: single middle value
        return FindPosition(num_vals / 2);
      } else {                  // even: average the two middle values
        return (FindPosition(num_vals / 2 - 1) + FindPosition(num_vals / 2)) / 2.0;
      }
    }

    [[nodiscard]] double CalcMode() const {
      if (num_vals == 0) return std::nan("");
      return emp::FindMaxIndex(counts);
    }
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_MATH_HISTOGRAM_HPP_GUARD
