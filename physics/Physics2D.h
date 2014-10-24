#ifndef EMP_PHYSICS_2D_H
#define EMP_PHYSICS_2D_H

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  Physics2D - handles movement and collissions in a simple 2D world.
//
//  Sector2D - One section of a Physics world; shapes are recorded in each sector they occupy.
//
//  Body2D - One individual object in the 2D world.
//


#include <vector>
#include <set>

#include "../tools/Circle.h"

namespace emp {

  template <typename BODY_INFO> class Body2D {
  private:
    Circle perimeter;
    BODY_INFO * info;
  public:
    Body2D(const Circle & _p, BODY_INFO * _i) : perimeter(_p), info(_i) { ; }
    ~Body2D() { ; }

    const Circle & GetPerimeter() const { return perimeter; }
    BODY_INFO * GetInfo() { return info; }
  }

  class Sector2D {
  private:
  public:
  }

  class Physics2D {
  private:
  public:
  };

};

#endif
