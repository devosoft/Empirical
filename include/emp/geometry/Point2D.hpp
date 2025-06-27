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

namespace emp {

  /// A simple 2-dimensional point.
  class Point2D {
  protected:
    double x = 0;
    double y = 0;

  public:
    constexpr Point2D()                    = default;  // Default = 0,0
    constexpr Point2D(const Point2D & _in) = default;  // Copy constructor

    constexpr Point2D(double x, double y) : x(x), y(y) {}  // Two ints -> x and y

    constexpr Point2D(const Point2D & _in, double new_magnitude)
      : x(_in.x * new_magnitude / _in.Magnitude())
      , y(_in.y * new_magnitude / _in.Magnitude()) {}

    Point2D & operator=(const Point2D & _in)          = default;
    constexpr auto operator<=>(const Point2D &) const = default;

    constexpr double GetX() const { return x; }

    constexpr double GetY() const { return y; }

    constexpr double X() const { return x; }

    constexpr double Y() const { return y; }

    Point2D & SetX(double in_x) {
      x = in_x;
      return *this;
    }

    Point2D & SetY(double in_y) {
      y = in_y;
      return *this;
    }

    Point2D & Set(double _x, double _y) {
      x = _x;
      y = _y;
      return *this;
    }

    constexpr double SquareMagnitude() const { return x * x + y * y; }

    constexpr double Magnitude() const { return sqrt(x * x + y * y); }

    constexpr bool AtOrigin() const { return x == 0.0 && y == 0.0; }

    constexpr bool NonZero() const { return x != 0.0 || y != 0.0; }

    // Determine a new point, but don't change this one
    constexpr Point2D GetMidpoint(const Point2D & p2) const { return (*this + p2) / 2.0; }

    constexpr Point2D GetRot90() const { return {y, -x}; }

    constexpr Point2D GetRot180() const { return {-x, -y}; }

    constexpr Point2D GetRot270() const { return {-y, x}; }

    constexpr Point2D GetOffset(double in_x, double in_y) const { return {x + in_x, y + in_y}; }

    constexpr Point2D GetOffsetX(double in_x) const { return {x + in_x, y}; }

    constexpr Point2D GetOffsetY(double in_y) const { return {x, y + in_y}; }

    constexpr Point2D GetScaled(double scale) const { return {x * scale, y * scale}; }

    constexpr Point2D GetScaled(double scale_x, double scale_y) const {
      return { x * scale_x, y * scale_y };
    }

    constexpr Point2D operator+(const Point2D & _in) const { return {x + _in.x, y + _in.y}; }

    constexpr Point2D operator-(const Point2D & _in) const { return {x - _in.x, y - _in.y}; }

    constexpr Point2D operator*(double mult) const { return {x * mult, y * mult}; }

    constexpr Point2D operator/(double div) const { return {x / div, y / div}; }

    constexpr Point2D operator*(int mult) const { return { x * mult, y * mult}; }

    constexpr Point2D operator/(int div) const { return { x / div, y / div}; }

    constexpr Point2D operator-() const { return { -x, -y}; }  // Unary minus

    constexpr Point2D Abs() const { return { std::abs(x), std::abs(y)}; }  // Absolute value

    
    // Modify this point.
    Point2D & Translate(double shift_x, double shift_y) {
      x += shift_x;
      y += shift_y;
      return *this;
    }

    Point2D & TranslateX(double shift) {
      x += shift;
      return *this;
    }

    Point2D & TranslateY(double shift) {
      y += shift;
      return *this;
    }

    Point2D & Scale(double scale) {
      x *= scale;
      y *= scale;
      return *this;
    }

    Point2D & Scale(double scale_x, double scale_y) {
      x *= scale_x;
      y *= scale_y;
      return *this;
    }

    Point2D & ToOrigin() {
      x = 0;
      y = 0;
      return *this;
    }

    Point2D & NegateX() {
      x = -x;
      return *this;
    }

    Point2D & NegateY() {
      y = -y;
      return *this;
    }

    Point2D & Mod(const Point2D & limits) {
      x = emp::Mod(x, limits.x);
      y = emp::Mod(y, limits.y);
      return *this;
    }

    Point2D & BoundLower(const Point2D & bound) {
      x = std::max(x, bound.x);
      y = std::max(y, bound.y);
      return *this;
    }

    Point2D & BoundUpper(const Point2D & bound) {
      x = std::min(x, bound.x);
      y = std::min(y, bound.y);
      return *this;
    }

    Point2D & BoundPositive() { return BoundLower({0.0, 0.0}); };

    Point2D & operator+=(const Point2D & _in) {
      x += _in.x;
      y += _in.y;
      return *this;
    }

    Point2D & operator-=(const Point2D & _in) {
      x -= _in.x;
      y -= _in.y;
      return *this;
    }

    Point2D & operator*=(double mult) {
      x *= mult;
      y *= mult;
      return *this;
    }

    Point2D & operator/=(double val) {
      if (val != 0.0) {
        x /= val;
        y /= val;
      };
      return *this;
    }

    Point2D & operator*=(int mult) {
      x *= mult;
      y *= mult;
      return *this;
    }

    Point2D & operator/=(int val) {
      if (val != 0.0) {
        x /= val;
        y /= val;
      };
      return *this;
    }

    Point2D & Rot90() { return Set(y, -x); }

    Point2D & Rot180() { return Set(-x, -y); }

    Point2D & Rot270() { return Set(-y, x); }

    // Square-roots are slow to calculate; if we're just doing comparisons, square-distance
    // is usually going to be sufficient.
    double SquareDistance(const Point2D & _in) const {
      const double x_dist = x - _in.x;
      const double y_dist = y - _in.y;
      return x_dist * x_dist + y_dist * y_dist;
    }

    double Distance(const Point2D & _in) const { return sqrt(SquareDistance(_in)); }
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

    constexpr Size2D ToCellSize(size_t num_rows, size_t num_cols) const {
      return { x / num_cols, y / num_rows };      
    }

    // Repeats from Point -- @CAO to be fixed with auto this in C++ 23:
    constexpr Size2D GetScaled(double scale) const { return {x * scale, y * scale}; }

    constexpr Size2D GetScaled(double scale_x, double scale_y) const {
      return { x * scale_x, y * scale_y };
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
