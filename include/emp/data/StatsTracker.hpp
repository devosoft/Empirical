/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/data/StatsTracker.hpp
 * @brief Track summary statistics for a series of numeric values.
 * @note Status: BETA
 */

#pragma once

#ifndef INCLUDE_EMP_DATA_STATS_TRACKER_HPP_GUARD
#define INCLUDE_EMP_DATA_STATS_TRACKER_HPP_GUARD

#include <concepts>
#include <cstddef>
#include <limits>
#include <ostream>
#include <string_view>

namespace emp {

  template <typename T>
  concept Numeric = std::integral<T> || std::floating_point<T>;

  /// Track count, total, minimum, maximum, and most recent value for a stream of numbers.
  template <Numeric VALUE_T>
  class StatsTracker {
  private:
    size_t count  = 0;
    VALUE_T total = 0;
    VALUE_T min   = std::numeric_limits<VALUE_T>::max();
    VALUE_T max   = std::numeric_limits<VALUE_T>::lowest();
    VALUE_T last  = 0;

  public:
    void Add(VALUE_T value) {
      ++count;
      last = value;
      total += value;
      if (value < min) { min = value; }
      if (value > max) { max = value; }
    }

    void Reset() { *this = StatsTracker{}; }

    [[nodiscard]] size_t GetCount() const { return count; }

    [[nodiscard]] VALUE_T GetTotal() const { return total; }

    [[nodiscard]] VALUE_T GetMin() const { return count ? min : VALUE_T{}; }

    [[nodiscard]] VALUE_T GetMax() const { return count ? max : VALUE_T{}; }

    [[nodiscard]] VALUE_T GetLast() const { return last; }

    [[nodiscard]] double GetAverage() const { return count ? (double) total / (double) count : 0.0; }

    [[nodiscard]] bool HasData() const { return count > 0; }

    void Print(std::ostream & os, std::string_view label = "Stats") const {
      if (!HasData()) { return; }

      os << label << ": "
         << count << " sample" << (count == 1 ? "" : "s")
         << "; min " << GetMin()
         << "; max " << GetMax()
         << "; avg " << GetAverage()
         << "; total " << GetTotal();
    }
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_DATA_STATS_TRACKER_HPP_GUARD
