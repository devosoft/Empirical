#ifndef EMP_PHYSICS_2D_H
#define EMP_PHYSICS_2D_H

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  Physics2D - handles movement and collissions in a simple 2D world.
//

#include <vector>
#include <unordered_set>

#include "Surface2D.h"

namespace emp {

  template <typename BODY_TYPE, typename BODY_INFO, typename BASE_TYPE> class Physics2D {
  private:
    Surface2D<BODY_TYPE, BODY_INFO, BASE_TYPE> surface;    // Contains bodies that can collide.
    Surface2D<BODY_TYPE, BODY_INFO, BASE_TYPE> background; // Contains bodies that can't collide.

  public:
    Physics2D(BASE_TYPE width, BASE_TYPE height, BASE_TYPE max_org_diameter=100) 
      : surface(width, height, width / max_org_diameter, height / max_org_diameter)
      , background(width, height, width / max_org_diameter, height / max_org_diameter)
    { ; }
    ~Physics2D() { ; }

    Physics2D & AddBody(BODY_TYPE * in_body) { surface.AddBody(in_body); return *this; }
    Physics2D & AddBackground(BODY_TYPE * in_body) { background.AddBody(in_body); return *this; }

    void Update_DoMovement() {
      auto body_set = surface.GetBodySet();
      for (BODY_TYPE * cur_body : body_set) {
        cur_body->BodyUpdate();   // Let a body change size or shape, as needed.
        cur_body->ProcessStep();  // Update position and velocity.
      }
    }

    void Update_DoCollisions() {
      auto body_set = surface.GetBodySet();

      // @CAO Run through all pairs of bodies that might collide and test to see if they did.
      for (auto body_it1 = body_set.begin(); body_it1 != body_set.end(); body_it1++) {
        for (auto body_it2 = body_it1; ++body_it2 != body_set.end();) {
          (*body_it1)->CollisionTest(*(*body_it2));
        }
      }

      // Make sure all bodies are in a legal position on the surface.
      for (BODY_TYPE * cur_body : body_set) {
        cur_body->FinalizePosition();
        cur_body->AdjustPosition(surface.GetMaxPosition());
      }
    }

    void Update() {
      Update_DoMovement();
      Update_DoCollisions();
    }

    const std::unordered_set<BODY_TYPE *> & GetBodySet() const {
      return surface.GetBodySet();
    }
    const std::unordered_set<BODY_TYPE *> & GetBackgroundSet() const {
      return background.GetBodySet();
    }
  };

};

#endif
