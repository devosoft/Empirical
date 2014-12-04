#ifndef EMP_SURFACE_2D_H
#define EMP_SURFACE_2D_H

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file defines a templated class to represent a 2D suface capable of maintaining data
//  about which 2D bodies are currently on that surface and rapidly identifying if they are 
//  overlapping.
//
//  BODY_TYPE is the class that represents the body geometry.
//  BODY_INFO represents the internal infomation about the body, including the controller.
//  BASE_TYPE indicates if the physics should be calculated as integer or floating point.

#include "Body2D.h"
#include <functional>

namespace emp {

  template <typename BODY_TYPE, typename BODY_INFO, typename BASE_TYPE> class Surface2D {
  private:
    const Point<BASE_TYPE> max_pos;     // Lower-left corner of the surface.
    std::vector<BODY_TYPE *> body_set;  // Set of all bodies on surface

  public:
    Surface2D(BASE_TYPE _width, BASE_TYPE _height) 
      : max_pos(_width, _height)
    {
    }
    ~Surface2D() { ; }

    const Point<BASE_TYPE> & GetMaxPosition() const { return max_pos; }
    const std::vector<BODY_TYPE *> & GetBodySet() const { return body_set; }

    Surface2D<BODY_TYPE, BODY_INFO, BASE_TYPE> & AddBody(BODY_TYPE * new_body) {
      body_set.push_back(new_body);                         // Add body to master list
      return *this;
    }


    // The following function will test pairs of collisions and run the passed-in function on pairs of
    // objects that *may* collide.

    void TestCollisions(std::function<bool(BODY_TYPE &, BODY_TYPE &)> collide_fun) {
      int hit_count = 0;
      int test_count = 0;
      
      const int cols = 32;
      const int rows = 32;
      const int num_sectors = rows * cols;
      const int sector_width = max_pos.GetX() / cols;
      const int sector_height = max_pos.GetY() / rows;

      std::vector< std::vector<BODY_TYPE *> > sector_set(num_sectors);

      // Loop through all of the bodies on this surface.
      for (auto body : body_set) {
        // Determine which sector the current body is in.
        const int cur_col = body->GetCenter().GetX() / sector_width;
        const int cur_row = body->GetCenter().GetY() / sector_height;

        // See if this body may collide with any of the bodies previously put into sectors.
        for (int i = std::max(0, cur_col-1); i <= std::min(cur_col+1, cols-1); i++) {
          for (int j = std::max(0, cur_row-1); j <= std::min(cur_row+1, rows-1); j++) {
            const int sector_id = i + cols * j;
            if (sector_set[sector_id].size() == 0) continue;

            for (auto body2 : sector_set[sector_id]) {
              test_count++;
              if (collide_fun(*body, *body2)) hit_count++;
                  
              // @CAO -- tag comparisons!
              if (body->GetColorID() == 1) body2->SetColorID(2);
              else if (body2->GetColorID() == 1) body->SetColorID(2);
            }            

          }
        }

        // Add this body to the current sector for future tests to compare with.
        const int cur_sector = cur_col + cur_row * cols;
        sector_set[cur_sector].push_back(body);
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
      std::cout << counter << " : " << hit_count << " / " << test_count << std::endl;

      // Make sure all bodies are in a legal position on the surface.
      for (BODY_TYPE * cur_body : body_set) {
        cur_body->FinalizePosition();
        cur_body->AdjustPosition(max_pos);
      }
    }

  };


};

#endif
