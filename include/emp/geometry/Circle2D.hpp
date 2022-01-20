/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file Circle2D.hpp
 *  @brief A class to manage circles in a 2D plane.
 */

#ifndef EMP_GEOMETRY_CIRCLE2D_HPP_INCLUDE
#define EMP_GEOMETRY_CIRCLE2D_HPP_INCLUDE

#include "Point2D.hpp"

namespace emp {

  template <typename TYPE=double> class Circle2D {
  private:
    Point2D<TYPE> center;
    TYPE radius;

  public:
    constexpr Circle2D(const Point2D<TYPE> & _c, TYPE _r=0) : center(_c), radius(_r) { ; }
    constexpr Circle2D(TYPE _x, TYPE _y, TYPE _r=0) : center(_x,_y), radius(_r) { ; }
    constexpr Circle2D(TYPE _r=0) : center(0.0, 0.0), radius(_r) { ; }

    constexpr const Point2D<TYPE> & GetCenter() const { return center; }
    constexpr TYPE GetCenterX() const { return center.GetX(); }
    constexpr TYPE GetCenterY() const { return center.GetY(); }
    constexpr TYPE GetRadius() const { return radius; }
    constexpr TYPE GetSquareRadius() const { return radius * radius; }

    Circle2D<TYPE> & SetCenter(const Point2D<TYPE> & c) { center = c; return *this; }
    Circle2D<TYPE> & SetCenter(TYPE x, TYPE y) { center.Set(x,y); return *this; }
    Circle2D<TYPE> & SetCenterX(TYPE x) { center.SetX(x); return *this; }
    Circle2D<TYPE> & SetCenterY(TYPE y) { center.SetY(y); return *this; }
    Circle2D<TYPE> & SetRadius(TYPE new_radius) { radius = new_radius; return *this; }
    Circle2D<TYPE> & Set(const Point2D<TYPE> & c, TYPE r) { center = c; radius = r; return *this; }
    Circle2D<TYPE> & Set(TYPE x, TYPE y, TYPE r) { center.Set(x,y); radius = r; return *this; }

    Circle2D<TYPE> & Translate(Point2D<TYPE> shift) { center += shift; return *this; }

    constexpr bool Contains(const Point2D<TYPE> & point) const {
      return center.SquareDistance(point) < GetSquareRadius();
    }
    constexpr bool Contains(TYPE x, TYPE y) const {
      return center.SquareDistance(Point2D<TYPE>(x,y)) < GetSquareRadius();
    }
    constexpr bool Contains(const Circle2D<TYPE> & other) const {
      return (other.center.Distance(center) + other.GetRadius()) < GetRadius();
    }
    bool HasOverlap(const Circle2D<TYPE> & other) const {
      const TYPE min_dist = radius + other.radius;
      return center.SquareDistance(other.center) < (min_dist * min_dist);
    }
  };

  using Circle = Circle2D<double>;
}

#endif // #ifndef EMP_GEOMETRY_CIRCLE2D_HPP_INCLUDE
