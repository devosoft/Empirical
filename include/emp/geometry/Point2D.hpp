/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2021-2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/geometry/Point2D.hpp
 * @brief A simple class to track value pairs of any kind, optimized for points in 2D Space.
 *
 * @note For maximal efficiency, prefer SquareMagnitude() and SquareDistance()
 * over Magnitude() and Distance() as the latter require a square-root.
 */

#pragma once

#ifndef INCLUDE_EMP_GEOMETRY_POINT2D_HPP_GUARD
#define INCLUDE_EMP_GEOMETRY_POINT2D_HPP_GUARD

#include <cmath>
#include <compare>
#include <iostream>

#include "../math/math.hpp"
#include "../tools/GridSize.hpp"

namespace emp {

  /// A simple 2-dimensional point.
  class Point2D {
  protected:
    double x = 0.0;
    double y = 0.0;

  public:
    constexpr Point2D()                    = default;  // Default = 0,0
    constexpr Point2D(const Point2D & _in) = default;  // Copy constructor

    constexpr Point2D(double x, double y) : x(x), y(y) {}

    constexpr Point2D(const GridPos & pos) : x(pos.Col()), y(pos.Row()) {}

    constexpr Point2D(const Point2D & _in, double new_magnitude)
      : x(_in.x * new_magnitude / _in.Magnitude())
      , y(_in.y * new_magnitude / _in.Magnitude()) {}

    Point2D & operator=(const Point2D & _in)          = default;
    constexpr auto operator<=>(const Point2D &) const = default;

    constexpr double GetX() const { return x;
    }

    constexpr double GetY() const { return y;
    }

    constexpr double X() const { return x;
    }

    constexpr double Y() const { return y;
    }

    template <class Self>
    constexpr Self & SetX(this Self & self, double in_x) {
      self.x = in_x;
      return self;
    }

    template <class Self>
    constexpr Self & SetY(this Self & self, double in_y) {
      self.y = in_y;
      return self;
    }

    template <class Self>
    constexpr Self & Set(this Self & self, double _x, double _y) {
      self.x = _x;
      self.y = _y;
      return self;
    }

    constexpr double SquareMagnitude() const { return x * x + y * y;
    }

    constexpr double Magnitude() const { return sqrt(x * x + y * y);
    }

    constexpr bool AtOrigin() const { return x == 0.0 && y == 0.0;
    }

    constexpr bool NonZero() const { return x != 0.0 || y != 0.0;
    }

    constexpr bool NonNegative() const { return x >= 0.0 || y >= 0.0;
    }

    // Determine a new point, but don't change this one
    template <class Self>
    constexpr Self GetMidpoint(this const Self & self, const Point2D & p2) {
      return (self + p2) / 2.0;
    }

    template <class Self>
    constexpr Self GetRot90(this const Self & self) { return {self.y, -self.x};
  }

    template <class Self>
    constexpr Self GetRot180(this const Self & self) { return {-self.x, -self.y};
  }

    template <class Self>
    constexpr Self GetRot270(this const Self & self) { return {-self.y, self.x};
  }

    template <class Self>
    constexpr Self GetOffsetX(this const Self & self, double in_x) { return {self.x + in_x, self.y};
  }

    template <class Self>
    constexpr Self GetOffsetY(this const Self & self, double in_y) { return {self.x, self.y + in_y};
  }

    template <class Self>
    constexpr Self GetScaled(this const Self & self, double scale) {
      return {self.x * scale, self.y * scale};
    }

    template <class Self>
    constexpr Self GetScaled(this const Self & self, double scale_x, double scale_y) {
      return { self.x * scale_x, self.y * scale_y };
    }

    template <class Self>
    constexpr Self operator+(this const Self & self, const Point2D & _in) {
      return {self.x + _in.x, self.y + _in.y};
    }

    template <class Self>
    constexpr Self operator-(this const Self & self, const Point2D & _in) {
      return {self.x - _in.x, self.y - _in.y};
    }

    template <class Self>
    constexpr Self operator*(this const Self & self, const Point2D & _in) {
      return {self.x * _in.x, self.y * _in.y};
    }

    template <class Self>
    constexpr Self operator/(this const Self & self, const Point2D & _in) {
      return {self.x / _in.x, self.y / _in.y};
    }

    template <class Self>
    constexpr Self operator/(this const Self & self, const GridPos & _in) {
      return {self.x / _in.Col(), self.y / _in.Row()};
    }


    template <class Self>
    constexpr Self operator*(this const Self & self, double mult) {
      return {self.x * mult, self.y * mult};
    }

    template <class Self>
    constexpr Self operator/(this const Self & self, double div) {
      return {self.x / div, self.y / div};
    }

    template <class Self>
    constexpr Self operator*(this const Self & self, int mult) {
      return { self.x * mult, self.y * mult};
    }

    template <class Self>
    constexpr Self operator/(this const Self & self, int div) {
      return { self.x / div, self.y / div};
    }

    template <class Self>
    constexpr Self operator-(this const Self & self) {   // Unary minus
      return { -self.x, -self.y};
    }

    template <class Self>
    constexpr Self Abs(this const Self & self) {  // Absolute value
      return { std::abs(self.x), std::abs(self.y)};
    }

    // Modify this point.
    template <class Self>
    constexpr Self & Scale(this Self & self, double scale_x, double scale_y) {
      self.x *= scale_x;
      self.y *= scale_y;
      return *this;
    }

    template <class Self>
    constexpr Self & ToOrigin(this Self & self) {
      self.x = 0.0;
      self.y = 0.0;
      return *this;
    }

    template <class Self>
    constexpr Self & NegateX(this Self & self) {
      self.x = -self.x;
      return *this;
    }

    template <class Self>
    constexpr Self & NegateY(this Self & self) {
      self.y = -self.y;
      return *this;
    }

    template <class Self>
    constexpr Self & Wrap(this Self & self, const Point2D & limits) {
      self.x = emp::Mod(self.x, limits.x);
      self.y = emp::Mod(self.y, limits.y);
      return *this;
    }

    template <class Self>
    constexpr Self & BoundLower(this Self & self, const Point2D & bound) {
      self.x = std::max(self.x, bound.x);
      self.y = std::max(self.y, bound.y);
      return *this;
    }

    template <class Self>
    constexpr Self & BoundUpper(this Self & self, const Point2D & bound) {
      self.x = std::min(self.x, bound.x);
      self.y = std::min(self.y, bound.y);
      return *this;
    }

    template <class Self>
    constexpr Self & BoundPositive(this Self & self) { return BoundLower({0.0, 0.0}); }

    template <class Self>
    constexpr Self & operator+=(this Self & self, const Point2D & _in) {
      self.x += _in.x;
      self.y += _in.y;
      return *this;
    }

    template <class Self>
    constexpr Self & operator-=(this Self & self, const Point2D & _in) {
      self.x -= _in.x;
      self.y -= _in.y;
      return *this;
    }

    template <class Self>
    constexpr Self & operator*=(this Self & self, double mult) {
      self.x *= mult;
      self.y *= mult;
      return *this;
    }

    template <class Self>
    constexpr Self & operator/=(this Self & self, double val) {
      if (val != 0.0) {
        self.x /= val;
        self.y /= val;
      };
      return *this;
    }

    template <class Self>
    constexpr Self & Rot90(this Self & self) { return Set(self.y, -self.x); }

    template <class Self>
    constexpr Self & Rot180(this Self & self) { return Set(-self.x, -self.y); }

    template <class Self>
    constexpr Self & Rot270(this Self & self) { return Set(-self.y, self.x); }

    // Square-roots are slow to calculate; if we're just doing comparisons, square-distance
    // is usually going to be sufficient.
    constexpr double SquareDistance(const Point2D & _in) const {
      const double x_dist = x - _in.x;
      const double y_dist = y - _in.y;
      return x_dist * x_dist + y_dist * y_dist;
    }

    constexpr double Distance(const Point2D & _in) const { return sqrt(SquareDistance(_in));
    }

    // Convert to a grid postion (typically done after dividing by a unit size)
    constexpr GridPos ToGridPos() {
      return GridPos{static_cast<GridPos::pos_t>(x), static_cast<GridPos::pos_t>(y)};
    }

    // Convert to a grid size (typically done after dividing by a unit size)
    // @CAO: Should be in Size2D, but need preserve Size2D with math first.
    constexpr GridSize ToGridSize() {
      return GridSize{static_cast<GridSize::pos_t>(x), static_cast<GridSize::pos_t>(y)};
    }
  };

  using Point = Point2D;

  class Size2D : public Point2D {
  public:
    constexpr Size2D()               = default;  // Default = 0,0
    constexpr Size2D(const Size2D &) = default;  // Copy constructor

    constexpr explicit Size2D(const Point2D & in) : Point2D(in) {}

    constexpr Size2D(double x, double y) : Point2D(x,y) {}

    constexpr Size2D(const Size2D & _in, double new_magnitude) : Point2D(_in, new_magnitude) {}

    constexpr Size2D & operator=(const Size2D & _in) = default;

    constexpr double Width() const { return x; }
    constexpr double Height() const { return y; }
    constexpr double Area() const { return Width() * Height(); }

    constexpr bool Contains(const Point2D in) const {
      return in.X() <= X() && in.Y() <= Y();
    }

    constexpr Size2D ToCellSize(size_t num_rows, size_t num_cols) const {
      return { x / num_cols, y / num_rows };      
    }
  };

}  // namespace emp

#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace std {
  // Overload ostream to work with points.
  std::ostream & operator<<(std::ostream & os, const emp::Point2D & point) {
    return os << "(" << point.GetX() << "," << point.GetY() << ")";
  }
}  // namespace std
#endif  // #ifndef DOXYGEN_SHOULD_SKIP_THIS

#endif  // #ifndef INCLUDE_EMP_GEOMETRY_POINT2D_HPP_GUARD
