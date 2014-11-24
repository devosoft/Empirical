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
    Physics2D(BASE_TYPE width, BASE_TYPE height, BASE_TYPE max_org_diameter=20) 
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
      const auto & body_set = surface.GetBodySet();
      int hit_count = 0;
      
      const int cols = 20;
      const int rows = 20;
      const int num_sectors = rows * cols;
      const int max_x = 600;
      const int max_y = 600;
      const int sector_width = max_x / cols;
      const int sector_height = max_y / rows;
      // const int max_diameter = 20;

      std::vector< std::unordered_set<BODY_TYPE *> > sector_set(num_sectors);

      for (auto body : body_set) {
        const int cur_col = body->GetCenter().GetX() / sector_width;
        const int cur_row = body->GetCenter().GetY() / sector_height;
        const int cur_sector = cur_col + cur_row * cols;

        for (int i = std::max(0, cur_col-1); i <= std::min(cur_col+1, cols-1); i++) {
          for (int j = std::max(0, cur_row-1); j <= std::min(cur_row+1, rows-1); j++) {
            const int sector_id = i + cols * j;

            for (auto body2 : sector_set[sector_id]) {
              if (body->CollisionTest(*body2)) hit_count++;
            }            

          }
        }

        sector_set[cur_sector].insert(body);
      }
      
      /*
      // @CAO Run through all pairs of bodies that might collide and test to see if they did.
      for (auto body_it1 = body_set.begin(); body_it1 != body_set.end(); body_it1++) {
        for (auto body_it2 = body_it1; ++body_it2 != body_set.end();) {
          if ((*body_it1)->CollisionTest(*(*body_it2))) hit_count++;
        }
      }
      */
      
      static int counter = 0;
      counter++;
      std::cout << counter << " : " << hit_count << std::endl;

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
