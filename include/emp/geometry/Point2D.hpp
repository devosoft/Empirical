/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file Point2D.hpp
 *  @brief A simple class to track value pairs of any kind, optimized for points in 2D Space.
 *
 * @note For maximal efficiency, prefer SquareMagnitude() and SquareDistance()
 * over Magnitude() and Distance() as the latter require a square-root.
 */

#ifndef EMP_GEOMETRY_POINT2D_HPP_INCLUDE
#define EMP_GEOMETRY_POINT2D_HPP_INCLUDE

#include <cmath>
#include <iostream>

#include "../math/math.hpp"

namespace emp {

  template <typename TYPE=double> class Point2D {
  private:
    TYPE x;
    TYPE y;

  public:
    constexpr Point2D() : x(0.0), y(0.0) { ; }                      // Default = 0,0
    constexpr Point2D(const Point2D & _in) : x(_in.x), y(_in.y) { ; } // Copy constructor
    constexpr Point2D(TYPE _x, TYPE _y) : x(_x), y(_y) { ; }        // Two ints -> x and y
    constexpr Point2D(const Point2D & _in, TYPE new_magnitude)
      : x(_in.x * ((double) new_magnitude) / ((double) _in.Magnitude()))
      , y(_in.y * ((double) new_magnitude) / ((double) _in.Magnitude()))
     { ; }
    // ~Point2D() { ; }

    const Point2D & operator=(const Point2D & _in) { x = _in.x; y = _in.y; return *this; }
    constexpr bool operator==(const Point2D & _in) const { return (x == _in.x) && (y == _in.y); }
    constexpr bool operator!=(const Point2D & _in) const { return (x != _in.x) || (y != _in.y); }

    // Inequalities have to do with BOTH ponts.
    constexpr bool operator<=(const Point2D & _in) const { return (x <= _in.x) && (y <= _in.y); }
    constexpr bool operator>=(const Point2D & _in) const { return (x >= _in.x) && (y >= _in.y); }

    constexpr inline TYPE GetX() const { return x; }
    constexpr inline TYPE GetY() const { return y; }
    inline Point2D & SetX(TYPE in_x) { x = in_x; return *this; }
    inline Point2D & SetY(TYPE in_y) { y = in_y; return *this; }
    Point2D & Set(TYPE _x, TYPE _y) { x=_x; y=_y; return *this; }

    constexpr double SquareMagnitude() const { return x*x + y*y; }
    constexpr double Magnitude() const { return sqrt( x*x + y*y ); }
    constexpr bool AtOrigin() const { return x == 0 && y == 0; }
    constexpr bool NonZero() const { return x != 0 || y != 0; }

    // Determine a new point, but don't change this one
    constexpr Point2D GetMidpoint(const Point2D & p2) const { return Point2D((x+p2.x)/2, (y+p2.y)/2); }
    constexpr Point2D GetRot90() const { return Point2D(y, -x); }
    constexpr Point2D GetRot180() const { return Point2D(-x, -y); }
    constexpr Point2D GetRot270() const { return Point2D(-y, x); }
    constexpr Point2D GetOffset(TYPE off_x, TYPE off_y) const { return Point2D(x+off_x, y+off_y); }

    constexpr Point2D operator+(const Point2D & _in) const { return Point2D(x + _in.x , y + _in.y); }
    constexpr Point2D operator-(const Point2D & _in) const { return Point2D(x - _in.x , y - _in.y); }
    constexpr Point2D operator*(double mult) const { return Point2D(((double) x) * mult, ((double) y) * mult); }
    constexpr Point2D operator/(double div) const { return Point2D(((double) x) / div, ((double) y) / div); }
    constexpr Point2D operator*(int mult) const { return Point2D(x * mult, y * mult); }
    constexpr Point2D operator/(int div) const { return Point2D(x / div, y / div); }
    constexpr Point2D operator-() const { return Point2D(-x, -y); } // Unary minus

    constexpr Point2D Abs() const { return Point2D(std::abs(x), std::abs(y)); } // Absolute value

    // Modify this point.
    Point2D & Translate(TYPE shift_x, TYPE shift_y) { x += shift_x; y += shift_y; return *this; }
    Point2D & TranslateX(TYPE shift) { x += shift; return *this; }
    Point2D & TranslateY(TYPE shift) { y += shift; return *this; }
    Point2D & Scale(double scale) { x *= scale; y *= scale; return *this; }
    Point2D & Scale(double scale_x, double scale_y) { x *= scale_x; y *= scale_y; return *this; }
    Point2D & ToOrigin() { x = 0; y = 0; return *this; }
    Point2D & NegateX() { x = -x; return *this; }
    Point2D & NegateY() { y = -y; return *this; }
    Point2D & Mod(const Point2D & limits) { x = emp::Mod(x, limits.x); y = emp::Mod(y, limits.y); return *this; }

    Point2D & operator+=(const Point2D & _in) { x += _in.x; y += _in.y; return *this; }
    Point2D & operator-=(const Point2D & _in) { x -= _in.x; y -= _in.y; return *this; }
    Point2D & operator*=(double mult) { x *= mult; y *= mult; return *this; }
    Point2D & operator/=(double val) { if (val != 0.0) { x /= val;  y /= val; }; return *this; }
    Point2D & operator*=(int mult) { x *= mult; y *= mult; return *this; }
    Point2D & operator/=(int val) { if (val != 0.0) { x /= val;  y /= val; }; return *this; }

    Point2D & Rot90() { return Set(y, -x); }
    Point2D & Rot180() { return Set(-x, -y); }
    Point2D & Rot270() { return Set(-y, x); }

    // Square-roots are slow to calculate; if we're just doing comparisons, square-distance
    // is usualy going to be sufficient.
    TYPE SquareDistance(const Point2D & _in) const {
      const TYPE x_dist = x - _in.x;
      const TYPE y_dist = y - _in.y;
      return x_dist*x_dist + y_dist*y_dist;
    }

    TYPE Distance(const Point2D & _in) const { return sqrt( SquareDistance(_in) ); }
  };

  using Point = Point2D<>;

}

namespace std {
  // Overload ostream to work with points.
  template <typename TYPE> std::ostream & operator<<(std::ostream & os,
                                                    const emp::Point2D<TYPE> & point) {
    return os << "(" << point.GetX() << "," << point.GetY() << ")";
  }
}

#endif // #ifndef EMP_GEOMETRY_POINT2D_HPP_INCLUDE
