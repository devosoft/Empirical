#ifndef EMP_PHYSICS_2D_H
#define EMP_PHYSICS_2D_H

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  Physics2D - handles movement and collissions in a simple 2D world.
//
//  Sector2D - One section of a Physics world; shapes are recorded in each sector they occupy.
//

#include <vector>
#include <unordered_set>

#include "Surface2D.h"

namespace emp {

  template <typename BODY_TYPE, typename BODY_INFO> class Physics2D {
  private:
    Surface2D<BODY_TYPE, BODY_INFO> passive_surface; // Contains bodies only external acted upon
    Surface2D<BODY_TYPE, BODY_INFO> active_surface;  // Contains bodies that can initate actions

  public:
    Physics2D(double width, double height, double max_org_diameter=1.0) 
      : passive_surface(width, height, width / max_org_diameter, height / max_org_diameter)
      , active_surface(width, height, width / max_org_diameter, height / max_org_diameter)
    { ; }
    ~Physics2D() { ; }

    Physics2D & AddActiveBody(BODY_TYPE * in_body) { active_surface.AddBody(in_body); return *this; }
  };

};

#endif
