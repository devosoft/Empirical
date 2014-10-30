#ifndef EMP_CIRCLE_2D_H
#define EMP_CIRCLE_2D_H

////////////////////////////////////////////////////////////////
//
//  A class to manage circles in a 2D plane.
//

#include "Point2D.h"

namespace emp {

  template <typename TYPE> class Circle {
  private:
    Point<TYPE> center;
    TYPE radius;

  public:
    Circle(const Point<TYPE> & _c, TYPE _r=0) : center(_c), radius(_r) { ; }
    Circle(TYPE _r=0) : center(0.0, 0.0), radius(_r) { ; }

    const Point<TYPE> & GetCenter() const { return center; }
    TYPE GetRadius() const { return radius; }

    Circle<TYPE> & SetCenter(const Point<TYPE> & new_center) { center = new_center; return *this; }
    Circle<TYPE> & SetCenterX(TYPE new_x) { center.SetX(new_x); return *this; }
    Circle<TYPE> & SetCenterY(TYPE new_y) { center.SetY(new_y); return *this; }
    Circle<TYPE> & SetRadius(TYPE new_radius) { radius = new_radius; return *this; }

    Circle<TYPE> & Translate(Point<TYPE> shift) { center += shift; return *this; }

    bool HasOverlap(const Circle<TYPE> & other) const {
      const TYPE min_dist = radius + other.radius;
      return center.SquareDistance(other.center) < (min_dist * min_dist);
    }
  };
};

#endif
