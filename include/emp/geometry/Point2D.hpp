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
    constexpr Point2D(const Point2D & in) = default;  // Copy constructor

    constexpr Point2D(double x, double y) : x(x), y(y) {}

    constexpr explicit Point2D(const GridPos & pos) : x(pos.Col()), y(pos.Row()) {}

    constexpr Point2D(const Point2D & in, double new_magnitude)
      : x(in.x * new_magnitude / in.Magnitude())
      , y(in.y * new_magnitude / in.Magnitude()) {}

    constexpr Point2D & operator=(const Point2D & in) = default;

    constexpr auto operator<=>(const Point2D &) const = default;

    [[nodiscard]] constexpr double GetX() const { return x; }

    [[nodiscard]] constexpr double GetY() const { return y; }

    [[nodiscard]] constexpr double X() const { return x; }

    [[nodiscard]] constexpr double Y() const { return y; }

    [[nodiscard]] constexpr double & X() { return x; }

    [[nodiscard]] constexpr double & Y() { return y; }

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

    void Reset() { x = 0.0; y = 0.0; }

    [[nodiscard]] constexpr double SquareMagnitude() const { return x * x + y * y; }

    [[nodiscard]] constexpr double Magnitude() const { return sqrt(x * x + y * y); }

    [[nodiscard]] constexpr bool AtOrigin() const { return x == 0.0 && y == 0.0; }

    [[nodiscard]] constexpr bool IsNonZero() const { return x != 0.0 || y != 0.0; }

    [[nodiscard]] constexpr bool IsNonNegative() const { return x >= 0.0 && y >= 0.0; }

    // Determine a new point, but don't change this one
    template <class Self>
    [[nodiscard]] constexpr Self Midpoint(this const Self & self, const Point2D & p2) {
      return (self + p2) / 2.0;
    }

    template <class Self>
    [[nodiscard]] constexpr Self Rot90(this const Self & self) { return {self.y, -self.x}; }

    template <class Self>
    [[nodiscard]] constexpr Self Rot180(this const Self & self) { return {-self.x, -self.y}; }

    template <class Self>
    [[nodiscard]] constexpr Self Rot270(this const Self & self) { return {-self.y, self.x}; }

    template <class Self>
    [[nodiscard]] constexpr Self OffsetX(this const Self & self, double in_x) { return {self.x + in_x, self.y}; }

    template <class Self>
    [[nodiscard]] constexpr Self OffsetY(this const Self & self, double in_y) { return {self.x, self.y + in_y}; }

    template <class Self>
    [[nodiscard]] constexpr Self Scale(this const Self & self, double scale) {
      return {self.x * scale, self.y * scale};
    }

    template <class Self>
    [[nodiscard]] constexpr Self Scale(this const Self & self, double scale_x, double scale_y) {
      return {self.x * scale_x, self.y * scale_y};
    }

    // === Operations with other Point2D's ===

    template <class Self>
    [[nodiscard]] constexpr Self operator+(this const Self & self, const Point2D & in) {
      return {self.x + in.x, self.y + in.y};
    }

    template <class Self>
    [[nodiscard]] constexpr Self operator-(this const Self & self, const Point2D & in) {
      return {self.x - in.x, self.y - in.y};
    }

    template <class Self>
    [[nodiscard]] constexpr Self operator*(this const Self & self, const Point2D & in) {
      return {self.x * in.x, self.y * in.y};
    }

    template <class Self>
    [[nodiscard]] constexpr Self operator/(this const Self & self, const Point2D & in) {
      return {self.x / in.x, self.y / in.y};
    }

    // === Operations with GridPosition ===

    template <class Self>
    [[nodiscard]] constexpr Self operator*(this const Self & self, const GridPos & in) {
      return {self.x * in.Col(), self.y * in.Row()};
    }

    template <class Self>
    [[nodiscard]] constexpr Self operator/(this const Self & self, const GridPos & in) {
      return {self.x / in.Col(), self.y / in.Row()};
    }


    // === Operations with double or int ===

    template <class Self>
    [[nodiscard]] constexpr Self operator*(this const Self & self, double mult) {
      return {self.x * mult, self.y * mult};
    }

    template <class Self>
    [[nodiscard]] constexpr Self operator/(this const Self & self, double div) {
      return {self.x / div, self.y / div};
    }

    template <class Self>
    [[nodiscard]] constexpr Self operator*(this const Self & self, int mult) {
      return { self.x * mult, self.y * mult};
    }

    template <class Self>
    [[nodiscard]] constexpr Self operator/(this const Self & self, int div) {
      return { self.x / div, self.y / div};
    }

    // === Unary operations ===
    template <class Self>
    [[nodiscard]] constexpr Self operator-(this const Self & self) {   // Unary minus
      return { -self.x, -self.y};
    }

    template <class Self>
    [[nodiscard]] constexpr Self Abs(this const Self & self) {  // Absolute value
      return {std::abs(self.x), std::abs(self.y)};
    }

    template <class Self>
    [[nodiscard]] constexpr Self NegateX(this const Self & self) {
      return {-self.x, self.y};
    }

    template <class Self>
    [[nodiscard]] constexpr Self NegateY(this const Self & self) {
      return {self.x, -self.y};
    }

    template <class Self>
    [[nodiscard]] constexpr Self Wrap(this const Self & self, const Point2D & limits) {
      return {emp::Mod(self.x, limits.x), emp::Mod(self.y, limits.y)};
    }

    template <class Self>
    [[nodiscard]] constexpr Self BoundLower(this const Self & self, const Point2D & bound) {
      return {std::max(self.x, bound.x), std::max(self.y, bound.y)};
    }

    template <class Self>
    [[nodiscard]] constexpr Self BoundUpper(this const Self & self, const Point2D & bound) {
      return {std::min(self.x, bound.x), std::min(self.y, bound.y)};
    }

    template <class Self>
    [[nodiscard]] constexpr Self BoundPositive(this const Self & self) {
      return self.BoundLower({0.0, 0.0});
    }

    template <class Self>
    constexpr Self & operator+=(this Self & self, const Point2D & in) {
      self.x += in.x;
      self.y += in.y;
      return self;
    }

    template <class Self>
    constexpr Self & operator-=(this Self & self, const Point2D & in) {
      self.x -= in.x;
      self.y -= in.y;
      return self;
    }

    template <class Self>
    constexpr Self & operator*=(this Self & self, const Point2D & in) {
      self.x *= in.x;
      self.y *= in.y;
      return self;
    }

    template <class Self>
    constexpr Self & operator/=(this Self & self, const Point2D & in) {
      emp_assert(in.X() != 0.0 && in.Y() != 0.0);
      self.x /= in.x;
      self.y /= in.y;
      return self;
    }

    template <class Self>
    constexpr Self & operator*=(this Self & self, double mult) {
      self.x *= mult;
      self.y *= mult;
      return self;
    }

    template <class Self>
    constexpr Self & operator/=(this Self & self, double val) {
      emp_assert(val != 0.0);
      self.x /= val;
      self.y /= val;
      return self;
    }

    template <class Self>
    constexpr Self & Rot90(this Self & self) { return self.Set(self.y, -self.x); }

    template <class Self>
    constexpr Self & Rot180(this Self & self) { return self.Set(-self.x, -self.y); }

    template <class Self>
    constexpr Self & Rot270(this Self & self) { return self.Set(-self.y, self.x); }

    [[nodiscard]] constexpr double Dot(const Point2D & in) const {
      return x * in.x + y * in.y;
    }

    // Square-roots are slow to calculate; if we're just doing comparisons, square-distance
    // is usually going to be sufficient.
    [[nodiscard]] constexpr double SquareDistance(const Point2D & in) const {
      const double x_dist = x - in.x;
      const double y_dist = y - in.y;
      return x_dist * x_dist + y_dist * y_dist;
    }

    [[nodiscard]] constexpr double Distance(const Point2D & in) const { return sqrt(SquareDistance(in)); }

    // Convert to a grid postion (typically done after dividing by a unit size)
    [[nodiscard]] constexpr GridPos ToGridPos() {
      return GridPos{static_cast<GridPos::pos_t>(x), static_cast<GridPos::pos_t>(y)};
    }
  };

  using Point = Point2D;

  class Size2D : public Point2D {
  public:
    constexpr Size2D()               = default;  // Default = 0,0
    constexpr Size2D(const Size2D &) = default;  // Copy constructor

    constexpr explicit Size2D(const Point2D & in) : Point2D(in) {}

    constexpr Size2D(double x, double y) : Point2D(x,y) {}

    constexpr Size2D(const Size2D & in, double new_magnitude) : Point2D(in, new_magnitude) {}

    constexpr Size2D & operator=(const Size2D & in) = default;

    [[nodiscard]] constexpr double Width() const { return x; }
    [[nodiscard]] constexpr double Height() const { return y; }
    [[nodiscard]] constexpr double Area() const { return Width() * Height(); }

    [[nodiscard]] constexpr bool Contains(const Point2D in) const {
      return in.IsNonNegative() && in.X() < X() && in.Y() < Y();
    }

    [[nodiscard]] constexpr Size2D ToCellSize(size_t num_rows, size_t num_cols) const {
      return { x / num_cols, y / num_rows };      
    }

    // Convert to GridSize (typically done after dividing by a unit size)
    [[nodiscard]] constexpr GridSize ToGridSize() {
      return GridSize{static_cast<GridSize::pos_t>(x), static_cast<GridSize::pos_t>(y)};
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
