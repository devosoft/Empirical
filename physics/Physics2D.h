#ifndef EMP_PHYSICS_2D_H
#define EMP_PHYSICS_2D_H

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  Physics2D - handles movement and collissions in a simple 2D world.
//
//  Sector2D - One section of a Physics world; shapes are recorded in each sector they occupy.
//
//  CircleBody2D - One individual circular object in the 2D world.
//


#include <vector>
#include <set>

#include "../tools/Circle.h"

namespace emp {

  template <typename BODY_TYPE, typename BODY_INFO> class Sector2D;

  template <typename BODY_INFO> class CircleBody2D {
  private:
    Circle perimeter;
    BODY_INFO * info;
    Sector2D<CircleBody2D<BODY_INFO>, BODY_INFO> * sector;  // What sector is this body in?
  public:
    Body2D(const Circle & _p, BODY_INFO * _i) : perimeter(_p), info(_i) { ; }
    ~Body2D() { ; }

    const Circle & GetPerimeter() const { return perimeter; }
    const Point & GetAnchor() const { return perimeter.GetCenter(); }
    const Point & GetCenter() const { return perimeter.GetCenter(); }
    BODY_INFO * GetInfo() { return info; }
    Sector2D<CircleBody2D<BODY_INFO>, BODY_INFO> * GetSector() { return sector; }

    auto & MoveTo(const cPoint & new_pos) { perimeter.SetCenter(new_pos); return *this; }
  };

  template <typename BODY_TYPE, typename BODY_INFO> class Sector2D {
  private:
  public:
  };

  class Physics2D {
  private:
  public:
  };

};

#endif
