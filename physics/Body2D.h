#ifndef EMP_BODY_2D_H
#define EMP_BODY_2D_H

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file defines templated classes to represent bodies that can exist on a 2D surface.
//  Each class should be able to:
//   * Maintain a pointer to information about the full organism associated with this body.
//   * Maintain a pointer to which sector this body is currently located in.
//   * provide a circular perimeter of the body (for phase1 of collision detection)
//   * Provide body an anchor point and center point of the body (typically the same)
//
//  Currently, the only type of body we have is:
//
//    CircleBody2D - One individual circular object in the 2D world.
//

#include "../tools/Circle.h"

namespace emp {

  template <typename BODY_TYPE, typename BODY_INFO> class Sector2D;

  template <typename BODY_INFO> class CircleBody2D {
  private:
    Circle perimeter;
    BODY_INFO * info;
    Sector2D<CircleBody2D<BODY_INFO>, BODY_INFO> * sector;  // What sector is this body in?
  public:
    CircleBody2D(const Circle & _p, BODY_INFO * _i) : perimeter(_p), info(_i) { ; }
    ~CircleBody2D() { ; }

    const Circle & GetPerimeter() const { return perimeter; }
    const Point & GetAnchor() const { return perimeter.GetCenter(); }
    const Point & GetCenter() const { return perimeter.GetCenter(); }
    BODY_INFO * GetInfo() { return info; }
    Sector2D<CircleBody2D<BODY_INFO>, BODY_INFO> * GetSector() { return sector; }

    CircleBody2D<BODY_INFO> & MoveTo(const Point & new_pos) { perimeter.SetCenter(new_pos); return *this; }
  };

};

#endif
