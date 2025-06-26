/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2021-2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/geometry/Polygon.hpp
 * @brief A class to manage arbitrary polygons in a 2D plane.
 */

#pragma once

#ifndef INCLUDE_EMP_GEOMETRY_POLYGON_HPP_GUARD
#define INCLUDE_EMP_GEOMETRY_POLYGON_HPP_GUARD

#include "../base/vector.hpp"
#include "../math/constants.hpp"

#include "Point2D.hpp"

namespace emp {

  class Polygon {
  private:
    Point2D anchor{0.0, 0.0};
    emp::vector<Point2D> other_points; // Relative to the anchor.

  public:
    Polygon(const Point2D & a_, const emp::vector<Point2D> & other_)
      : anchor(a_), other_points(other_) {}

    Polygon(const Polygon &) = default;
    Polygon(Polygon &&) = default;
    Polygon & operator=(const Polygon & _in)        = default;
    Polygon & operator=(Polygon && _in)        = default;

    int operator<=>(const Polygon &) const = default;

    const Point2D & GetAnchor() const { return anchor; }

    Polygon & SetAnchor(const Point2D & in) {
      anchor = in;
      return *this;
    }

    Polygon & SetOther(const emp::vector<Point2D> & in) {
      other_points = in;
      return *this;
    }

    Polygon & Translate(Point2D shift) {
      anchor += shift;
      return *this;
    }

  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_GEOMETRY_POLYGON_HPP_GUARD
