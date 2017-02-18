//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A class to manage circles in a 2D plane.


#ifndef EMP_CIRCLE_2D_H
#define EMP_CIRCLE_2D_H


#include "Point2D.h"

namespace emp {

  template <typename TYPE=double> class Circle {
  private:
    Point2D<TYPE> center;
    TYPE radius;

  public:
    constexpr Circle(const Point2D<TYPE> & _c, TYPE _r=0) : center(_c), radius(_r) { ; }
    constexpr Circle(TYPE _x, TYPE _y, TYPE _r=0) : center(_x,_y), radius(_r) { ; }
    constexpr Circle(TYPE _r=0) : center(0.0, 0.0), radius(_r) { ; }

    constexpr const Point2D<TYPE> & GetCenter() const { return center; }
    constexpr TYPE GetCenterX() const { return center.GetX(); }
    constexpr TYPE GetCenterY() const { return center.GetY(); }
    constexpr TYPE GetRadius() const { return radius; }
    constexpr TYPE GetSquareRadius() const { return radius * radius; }

    Circle<TYPE> & SetCenter(const Point2D<TYPE> & new_center) { center = new_center; return *this; }
    Circle<TYPE> & SetCenterX(TYPE new_x) { center.SetX(new_x); return *this; }
    Circle<TYPE> & SetCenterY(TYPE new_y) { center.SetY(new_y); return *this; }
    Circle<TYPE> & SetRadius(TYPE new_radius) { radius = new_radius; return *this; }

    Circle<TYPE> & Translate(Point2D<TYPE> shift) { center += shift; return *this; }

    constexpr bool Contains(const Point2D<TYPE> & point) const {
      return center.SquareDistance(point) < GetSquareRadius();
    }
    constexpr bool Contains(const Circle<TYPE> & other) const {
      return (other.center.Distance(center) + other.GetRadius()) < GetRadius();
    }
    bool HasOverlap(const Circle<TYPE> & other) const {
      const TYPE min_dist = radius + other.radius;
      return center.SquareDistance(other.center) < (min_dist * min_dist);
    }
  };

}

#endif
