#ifndef EMP_CIRCLE_2D_H
#define EMP_CIRCLE_2D_H

////////////////////////////////////////////////////////////////
//
//  A class to manage circles in a 2D plane.
//

#include "Point2D.h"

namespace emp {

  template <typename TYPE=double> class Circle {
  private:
    Point<TYPE> center;
    TYPE radius;

  public:
    Circle(const Point<TYPE> & _c, TYPE _r=0) : center(_c), radius(_r) { ; }
    Circle(TYPE _x, TYPE _y, TYPE _r=0) : center(_x,_y), radius(_r) { ; }
    Circle(TYPE _r=0) : center(0.0, 0.0), radius(_r) { ; }

    const Point<TYPE> & GetCenter() const { return center; }
    TYPE GetCenterX() const { return center.GetX(); }
    TYPE GetCenterY() const { return center.GetY(); }
    TYPE GetRadius() const { return radius; }
    TYPE GetSquareRadius() const { return radius * radius; }

    Circle<TYPE> & SetCenter(const Point<TYPE> & new_center) { center = new_center; return *this; }
    Circle<TYPE> & SetCenterX(TYPE new_x) { center.SetX(new_x); return *this; }
    Circle<TYPE> & SetCenterY(TYPE new_y) { center.SetY(new_y); return *this; }
    Circle<TYPE> & SetRadius(TYPE new_radius) { radius = new_radius; return *this; }

    Circle<TYPE> & Translate(Point<TYPE> shift) { center += shift; return *this; }

    bool Contains(const Point<TYPE> & point) const {
      return center.SquareDistance(point) < GetSquareRadius();
    }
    bool Contains(const Circle<TYPE> & other) const {
      const TYPE max_dist = other.center.Distance(center) + other.GetRadius();
      return max_dist < GetRadius();
    }
    bool HasOverlap(const Circle<TYPE> & other) const {
      const TYPE min_dist = radius + other.radius;
      return center.SquareDistance(other.center) < (min_dist * min_dist);
    }
  };

}

#endif
