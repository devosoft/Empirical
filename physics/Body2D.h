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

  template <typename BODY_TYPE, typename BODY_INFO, typename BASE_TYPE> class Sector2D;

  template <typename BODY_INFO, typename BASE_TYPE> class CircleBody2D {
  private:
    Circle<BASE_TYPE> perimeter;     // Includes position and size.
    BODY_INFO * info;                // External information about individual
    Sector2D<CircleBody2D<BODY_INFO, BASE_TYPE>, BODY_INFO, BASE_TYPE> * sector;  // Track location
  public:
    CircleBody2D(const Circle<BASE_TYPE> & _p, BODY_INFO * _i) : perimeter(_p), info(_i) { ; }
    ~CircleBody2D() { ; }

    const Circle<BASE_TYPE> & GetPerimeter() const { return perimeter; }
    const Point<BASE_TYPE> & GetAnchor() const { return perimeter.GetCenter(); }
    const Point<BASE_TYPE> & GetCenter() const { return perimeter.GetCenter(); }
    BODY_INFO * GetInfo() { return info; }
    Sector2D<CircleBody2D<BODY_INFO, BASE_TYPE>, BODY_INFO, BASE_TYPE> * GetSector() { return sector; }

    CircleBody2D<BODY_INFO, BASE_TYPE> & MoveTo(const Point<BASE_TYPE> & new_pos) {
      perimeter.SetCenter(new_pos); 
      return *this;
    }
    CircleBody2D<BODY_INFO, BASE_TYPE> &
    SetSector(Sector2D<CircleBody2D<BODY_INFO, BASE_TYPE>, BODY_INFO, BASE_TYPE> * new_sector) {
      sector = new_sector;
      return *this;
    }
  };

};

#endif
