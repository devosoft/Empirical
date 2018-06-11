//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2018.
//  Released under the MIT Software license; see doc/LICENSE
//  Previously named Surface2D.h
//
//
//  This file defines a templated class to represent a 2D suface capable of maintaining data
//  about which 2D bodies are currently on that surface and rapidly identifying if they are
//  overlapping.
//
//  BODY_TYPE is the class that represents the body geometry.
//
//  To work, the BODY_TYPE must have:
//  - GetCenter() which returns a Point indicating the center of the organism.
//  - GetRadius() which returns a distance under which you want to perfor a more detailed
//                check as to whether two bodies actually overlap.


#ifndef EMP_SURFACE_H
#define EMP_SURFACE_H

#include <functional>

#include "../base/Ptr.h"

#include "Angle2D.h"
#include "Point2D.h"

namespace emp {

  template <typename BODY_TYPE>
  class Surface {
  public:
    using body_t = BODY_TYPE;
  protected:
    const Point max_pos;                // Lower-left corner of the surface.
    emp::vector<Ptr<body_t>> body_set;  // Set of all bodies on surface

    // Data tracking the current bodies on this surface.
    bool data_active;                   // Are we trying to keep data up-to-date?
    double max_radius;                  // Largest radius of any body.
    size_t max_count;                   // How many bodies have the max radius?

    // Update active data associated with a single body.
    void UpdateBody(Ptr<body_t> body) {
      const double cur_radius = body->GetRadius();
      if (cur_radius > max_radius) {
        max_radius = cur_radius;      // Record the new radius.
        max_count = 1;                // New radius has only this body thus far.
        data_active = false;          // May need to rebuild sectors, so deactivate data.
      }
      else if (cur_radius == max_radius) { max_count++; }
    }

    // Cleanup all of the data and mark the data as active.
    void Activate() {
      if (data_active) return; // Already active!

      // Determine the largest radius.
      max_radius = 0.0; max_count = 0;
      for (Ptr<body_t> body : body_set) UpdateBody(body);

      // Figure out the actual number of sectors to use (currently no more than 1024).
      const double max_diameter = max_radius * 2.0;
      emp_assert(max_diameter < max_pos.GetX());  // Surface must be bigger than biggest body
      emp_assert(max_diameter < max_pos.GetY());
      const size_t num_cols = std::min(max_pos.GetX()/max_diameter, 32.0);
      const size_t num_rows = std::min(max_pos.GetY()/max_diameter, 32.0);
      const size_t max_col = num_cols-1;
      const size_t max_row = num_rows-1;

      const size_t num_sectors = num_cols * num_rows;
      const double sector_width = max_pos.GetX() / (double) num_cols;
      const double sector_height = max_pos.GetY() / (double) num_rows;

      emp::vector< emp::vector<Ptr<body_t>> > sector_set(num_sectors);

      data_active = true;
    }
  public:
    Surface(Point _max) : max_pos(_max), body_set(), max_radius(0.0) { ; }
    ~Surface() { Clear(); }

    double GetWidth() const { return max_pos.GetX(); }
    double GetHeight() const { return max_pos.GetY(); }
    const Point & GetMaxPosition() const { return max_pos; }

    /// Add a single body.
    Surface & AddBody(Ptr<body_t> new_body) {
      body_set.push_back(new_body);     // Add body to master list
      return *this;
    }

    /// Remove all bodies from the surface.
    Surface & Clear() {
      data_active = false;
      body_set.resize(0);
      return *this;
    }





    // The following function will test pairs of collisions and run the passed-in function
    // on pairs of objects that *may* collide.

    void TestCollisions(std::function<bool(BODY_TYPE &, BODY_TYPE &)> collide_fun) {
      emp_assert(collide_fun);

      // Find the size of the largest body to determine minimum sector size.
      double max_radius = 0.0;
      for (auto body : body_set) {
        if (body->GetRadius() > max_radius) max_radius = body->GetRadius();
      }

      // Figure out the actual number of sectors to use (currently no more than 1024).
      const int num_cols = std::min<int>((int)(max_pos.GetX() / (max_radius * 2.0)), 32);
      const int num_rows = std::min<int>((int)(max_pos.GetY() / (max_radius * 2.0)), 32);
      const int max_col = num_cols-1;
      const int max_row = num_rows-1;

      const int num_sectors = num_cols * num_rows;
      const double sector_width = max_pos.GetX() / (double) num_cols;
      const double sector_height = max_pos.GetY() / (double) num_rows;

      emp::vector< emp::vector<Ptr<BODY_TYPE>> > sector_set(num_sectors);



      int hit_count = 0;
      int test_count = 0;

      // Loop through all of the bodies on this surface placing them in sectors and testing for
      // collisions with other bodies already in nearby sectors.
      for (auto body : body_set) {
        emp_assert(body);
        // Determine which sector the current body is in.
        const int cur_col = emp::ToRange<int>((int)(body->GetCenter().GetX()/sector_width), 0, max_col);
        const int cur_row = emp::ToRange<int>((int)(body->GetCenter().GetY()/sector_height), 0, max_row);

        // See if this body may collide with any of the bodies previously put into sectors.
        for (int i = std::max(0, cur_col-1); i <= std::min(cur_col+1, num_cols-1); i++) {
          for (int j = std::max(0, cur_row-1); j <= std::min(cur_row+1, num_rows-1); j++) {
            const int sector_id = i + num_cols * j;
            if (sector_set[sector_id].size() == 0) continue;

            for (auto body2 : sector_set[sector_id]) {
              test_count++;
              if (collide_fun(*body, *body2)) hit_count++;
            }

          }
        }

        // Add this body to the current sector for future tests to compare with.
        const int cur_sector = cur_col + cur_row * num_cols;
        emp_assert(cur_sector < (int) sector_set.size());

        sector_set[cur_sector].push_back(body);
      }

      // Make sure all bodies are in a legal position on the surface.
      for (Ptr<BODY_TYPE> cur_body : body_set) {
        cur_body->FinalizePosition(max_pos);
      }
    }

  };


}

#endif
