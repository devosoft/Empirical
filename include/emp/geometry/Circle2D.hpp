/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2021-2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/geometry/Circle2D.hpp
 * @brief A class to manage circles in a 2D plane.
 */

#pragma once

#ifndef INCLUDE_EMP_GEOMETRY_CIRCLE2D_HPP_GUARD
#define INCLUDE_EMP_GEOMETRY_CIRCLE2D_HPP_GUARD

#include "../math/constants.hpp"

#include "Point2D.hpp"

namespace emp {

  class Circle2D {
  private:
    double radius;
    Point2D center{0.0, 0.0};

  public:
    constexpr Circle2D(const Point2D & _c, double _r = 0) : radius(_r), center(_c) {}

    constexpr Circle2D(double _r = 0.0) : radius(_r) {}

    constexpr Circle2D(const Circle2D &) = default;

    Circle2D & operator=(const Circle2D & _in)        = default;

    constexpr int operator<=>(const Circle2D &) const = default;

    constexpr const Point2D & GetCenter() const { return center; }

    constexpr double GetCenterX() const { return center.GetX(); }

    constexpr double GetCenterY() const { return center.GetY(); }

    constexpr double GetRadius() const { return radius; }

    constexpr double GetSquareRadius() const { return radius * radius; }

    constexpr double GetArea() const { return PI * radius * radius; }

    constexpr double GetCircumference() const { return PI * 2.0 * radius; }

    Circle2D & SetCenter(const Point2D & c) {
      center = c;
      return *this;
    }

    Circle2D & SetCenterX(double x) {
      center.SetX(x);
      return *this;
    }

    Circle2D & SetCenterY(double y) {
      center.SetY(y);
      return *this;
    }

    Circle2D & SetRadius(double new_radius) {
      radius = new_radius;
      return *this;
    }

    Circle2D & Set(const Point2D & c, double r) {
      center = c;
      radius = r;
      return *this;
    }

    Circle2D & Translate(Point2D shift) {
      center += shift;
      return *this;
    }

    constexpr bool Contains(const Point2D & point) const {
      return center.SquareDistance(point) < GetSquareRadius();
    }

    constexpr bool Contains(const Circle2D & other) const {
      return (other.center.Distance(center) + other.GetRadius()) < GetRadius();
    }

    bool HasOverlap(const Circle2D & other) const {
      const double min_dist = radius + other.radius;
      return center.SquareDistance(other.center) < (min_dist * min_dist);
    }
  };

  using Circle = Circle2D;
}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_GEOMETRY_CIRCLE2D_HPP_GUARD
