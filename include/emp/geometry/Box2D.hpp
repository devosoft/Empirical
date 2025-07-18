/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2024-2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/geometry/Box2D.hpp
 * @brief A class to manage rectangles in a 2D plane.
 */

#pragma once

#ifndef INCLUDE_EMP_GEOMETRY_BOX2D_HPP_GUARD
#define INCLUDE_EMP_GEOMETRY_BOX2D_HPP_GUARD

#include "../math/constants.hpp"

#include "Point2D.hpp"

namespace emp {

  class Box2D {
  private:
    Point2D ul_corner;
    Size2D size;

  public:
    constexpr Box2D(const Point2D & ul_corner=Point2D{}, const Size2D & size=Size2D{1.0, 1.0})
      : ul_corner(ul_corner), size(size) {}
    constexpr Box2D(const Size2D & size)
      : ul_corner(0.0, 0.0), size(size) {}

    constexpr Box2D(const Box2D &)       = default;
    constexpr Box2D(Box2D &&)            = default;
    Box2D & operator=(const Box2D & _in) = default;
    Box2D & operator=(Box2D && _in)      = default;

    [[nodiscard]] constexpr auto operator<=>(const Box2D &) const = default;

    [[nodiscard]] constexpr const Point2D & GetUL() const { return ul_corner; }
    [[nodiscard]] constexpr const Size2D & GetSize() const { return size; }

    [[nodiscard]] constexpr double GetLeft() const { return ul_corner.GetX(); }
    [[nodiscard]] constexpr double GetRight() const { return ul_corner.GetX() + size.Width(); }
    [[nodiscard]] constexpr double GetTop() const { return ul_corner.GetY(); }
    [[nodiscard]] constexpr double GetBottom() const { return ul_corner.GetY() + size.Height(); }

    [[nodiscard]] constexpr double GetArea() const { return size.Area(); }
    [[nodiscard]] constexpr double GetPerimeter() const {
      return 2.0 * (size.Width() + size.Height());
    }

    constexpr void SetPosition(const Point2D & in) { ul_corner = in; }
    constexpr void SetSize(const Size2D & in) { size = in; }

    [[nodiscard]] constexpr Box2D operator+(Point2D shift) { return {ul_corner + shift, size}; }
    [[nodiscard]] constexpr Box2D operator-(Point2D shift) { return {ul_corner - shift, size}; }
    [[nodiscard]] constexpr Box2D operator*(Point2D dilate) { return {ul_corner, size * dilate}; }
    [[nodiscard]] constexpr Box2D operator/(Point2D dilate) { return {ul_corner, size / dilate}; }

    constexpr Box2D & operator+=(Point2D shift) {
      ul_corner += shift;
      return *this;
    }

    constexpr Box2D & operator-=(Point2D shift) {
      ul_corner -= shift;
      return *this;
    }

    constexpr Box2D & operator*=(double dilate) {
      size *= dilate;
      return *this;
    }

    constexpr Box2D & operator/=(double dilate) {
      size /= dilate;
      return *this;
    }

    constexpr Box2D & operator*=(Point2D dilate) {
      size *= dilate;
      return *this;
    }

    constexpr Box2D & operator/=(Point2D dilate) {
      size /= dilate;
      return *this;
    }

    [[nodiscard]] constexpr bool Contains(const Point2D & point) const {
      return point.GetX() >= GetLeft() && point.GetX() <= GetRight() && point.GetY() >= GetTop() &&
             point.GetY() <= GetBottom();
    }

    // Is "other" fully contained inside of this box?
    [[nodiscard]] constexpr bool Contains(const Box2D & other) const {
      return other.GetLeft() >= GetLeft() && other.GetRight() <= GetRight() &&
             other.GetTop() >= GetTop() && other.GetBottom() <= GetBottom();
    }

    [[nodiscard]] constexpr bool HasOverlap(const Box2D & other) const {
      // Make sure THIS is higher than OTHER.
      if (other.GetTop() < GetTop()) { return other.HasOverlap(*this); }

      return (other.GetTop() <= GetBottom() &&  // Other box is not too low.
              other.GetLeft() <= GetRight() &&  // ... not too far left
              other.GetRight() >= GetLeft());   // ... and not too far right.
    }
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_GEOMETRY_BOX2D_HPP_GUARD
