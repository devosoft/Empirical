/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/geometry/Line2D.hpp
 * @brief A class to manage lines in a 2D plane.
 */

#pragma once

#ifndef INCLUDE_EMP_GEOMETRY_LINE2D_HPP_GUARD
#define INCLUDE_EMP_GEOMETRY_LINE2D_HPP_GUARD

#include <limits>

#include "../math/constants.hpp"

#include "Point2D.hpp"

namespace emp {

  class Line2D {
  private:
    Point2D start_pos;
    Point2D end_offset;

  public:
    constexpr Line2D(const Point2D & start_pos, const Point2D & end_offset)
      : start_pos(start_pos), end_offset(end_offset) {}
    constexpr Line2D(const Point2D & end_offset)
      : start_pos(0.0, 0.0), end_offset(end_offset) {}

    constexpr Line2D(const Line2D &)       = default;
    constexpr Line2D(Line2D &&)            = default;
    Line2D & operator=(const Line2D & _in) = default;
    Line2D & operator=(Line2D && _in)      = default;

    [[nodiscard]] constexpr auto operator<=>(const Line2D &) const = default;

    [[nodiscard]] constexpr const Point2D & GetStartPos() const { return start_pos; }
    [[nodiscard]] constexpr Point2D GetEndPos() const { return start_pos + end_offset; }
    [[nodiscard]] constexpr const Point2D & GetEndOffset() const { return end_offset; }

    constexpr void SetPosition(const Point2D & in) { start_pos = in; }
    constexpr void SetEndOffset(const Point2D & in) { end_offset = in; }
    constexpr void SetEndPos(const Point2D & in) { end_offset = in - start_pos; }

    [[nodiscard]] constexpr Line2D operator+(Point2D shift) {
      return {start_pos + shift, end_offset};
    }

    [[nodiscard]] constexpr Line2D operator-(Point2D shift) {
      return {start_pos - shift, end_offset};
    }

    [[nodiscard]] constexpr Line2D operator*(Point2D dilate) {
      return {start_pos, end_offset * dilate};
    }

    [[nodiscard]] constexpr Line2D operator/(Point2D dilate) {
      return {start_pos, end_offset / dilate};
    }

    constexpr Line2D & operator+=(Point2D shift) {
      start_pos += shift;
      return *this;
    }

    constexpr Line2D & operator-=(Point2D shift) {
      start_pos -= shift;
      return *this;
    }

    constexpr Line2D & operator*=(double dilate) {
      end_offset *= dilate;
      return *this;
    }

    constexpr Line2D & operator/=(double dilate) {
      end_offset /= dilate;
      return *this;
    }

    constexpr Line2D & operator*=(Point2D dilate) {
      end_offset *= dilate;
      return *this;
    }

    constexpr Line2D & operator/=(Point2D dilate) {
      end_offset /= dilate;
      return *this;
    }

    [[nodiscard]] double Slope() const {
      if (end_offset.X() == 0) return std::numeric_limits<double>::infinity();
      return end_offset.Y() / end_offset.X();
    }

    [[nodiscard]] double YIntercept() const {
      return start_pos.Y() - Slope() * start_pos.X();
    }
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_GEOMETRY_LINE2D_HPP_GUARD
