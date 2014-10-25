#ifndef EMP_CIRCLE_H
#define EMP_CIRCLE_H

////////////////////////////////////////////////////////////////
//
//  A class to manage circles in a 2D plane.
//

#include "Point.h"

namespace emp {

  class Circle {
  private:
    Point center;
    double radius;

  public:
    Circle(const Point & _c, double _r=0.0) : center(_c), radius(_r) { ; }
    Circle(double _r=0.0) : center(0.0, 0.0), radius(_r) { ; }

    const Point & GetCenter() const { return center; }
    double GetRadius() const { return radius; }

    Circle & SetCenter(const Point & new_center) { center = new_center; return *this; }
    Circle & SetRadius(int new_radius) { radius = new_radius; return *this; }

    bool HasOverlap(const Circle & other) const {
      const double min_dist = radius + other.radius;
      return center.SquareDistance(other.center) < min_dist * min_dist;
    }
  };
};

#endif
