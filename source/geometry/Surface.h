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

#include <algorithm>
#include <functional>

#include "../base/Ptr.h"

#include "Angle2D.h"
#include "Point2D.h"

namespace emp {

  template <typename BODY_TYPE>
  class Surface {
  public:
    using body_t = BODY_TYPE;
    using body_set_t = emp::vector<Ptr<body_t>>;
    using overlap_fun_t = std::function<void(body_t &, body_t &)>;
  protected:
    const Point max_pos;     // Lower-left corner of the surface.
    body_set_t body_set;     // Set of all bodies on surface

    // Data tracking the current bodies on this surface.
    bool data_active;                 // Are we trying to keep data up-to-date?
    double max_radius;                // Largest radius of any body.
    size_t num_cols;                  // How many cols of sectors are there?
    size_t num_rows;                  // How many rows of sectors are there?
    size_t num_sectors;               // How many total sectors are there?
    double sector_width;              // How wide is each sector?
    double sector_height;             // How tall is each sector?
    emp::vector<body_set_t> sectors;  // Which bodies are in each sector?

    // Make sure there are num_sectors sectors and remove all bodies from existing ones.
    void SetupSectors() {
      size_t min_size = std::min(num_sectors, sectors.size());
      sectors.resize(num_sectors);
      for (size_t i = 0; i < min_size; i++) sectors[i].resize(0);
    }

    // Keep track of the largest body size found.
    // Note: Uses watermarking, so largest body will never shrink, even if removed
    //       unless the user explicitly calls RefreshBodySize()
    inline void TestBodySize(Ptr<body_t> body) {
      const double cur_radius = body->GetRadius();
      if (cur_radius > max_radius) {
        max_radius = cur_radius;      // Record the new radius.
        data_active = false;          // May need to rebuild sectors, so deactivate data.
      }
    }

    // Clear out the watermarked body size and update the current largest.
    inline void RefreshBodySize() {
      max_radius = 0.0;
      for (Ptr<body_t> body : body_set) TestBodySize(body);      
    }

    // Place an active body into a sector.
    inline void PlaceBody(Ptr<body_t> body) {
      // Where is the current body?
      const double body_x = body->GetCenter().GetX();
      const double body_y = body->GetCenter().GetY();

      // Make sure the current body is in the sectors.
      emp_assert(body_x >= 0.0 && body_x < max_pos.GetX());
      emp_assert(body_y >= 0.0 && body_y < max_pos.GetY());

      // Determine which sector the current body is in.
      const double sector_x = body_x / sector_width;
      const double sector_y = body_y / sector_height;

      const size_t cur_col = (size_t) sector_x;
      const size_t cur_row = (size_t) sector_y;
      const size_t cur_sector = cur_col + cur_row * num_cols;

      emp_assert(cur_sector < sectors.size());
      sectors[cur_sector].push_back(body);
    }

    // Cleanup all of the data and mark the data as active.
    void Activate() {
      if (data_active) return; // Already active!

      // Figure out the actual number of sectors to use (currently no more than 1024).
      const double max_diameter = max_radius * 2.0;
      emp_assert(max_diameter < max_pos.GetX());  // Surface must be bigger than biggest body
      emp_assert(max_diameter < max_pos.GetY());
      num_cols = (size_t) std::min(max_pos.GetX()/max_diameter, 32.0);
      num_rows = (size_t) std::min(max_pos.GetY()/max_diameter, 32.0);

      num_sectors = num_cols * num_rows;
      sector_width = max_pos.GetX() / (double) num_cols;
      sector_height = max_pos.GetY() / (double) num_rows;

      SetupSectors();   // Now that we know the sizes, we can initialize sectors.

      // Put all of the bodies into sectors
      for (Ptr<body_t> body : body_set) PlaceBody(body);

      data_active = true;
    }
  public:
    Surface(Point _max) : max_pos(_max), body_set(), max_radius(0.0) { ; }
    ~Surface() { Clear(); }

    double GetWidth() const { return max_pos.GetX(); }
    double GetHeight() const { return max_pos.GetY(); }
    const Point & GetMaxPosition() const { return max_pos; }
    const body_set_t & GetBodySet() const { return body_set; }

    /// Add a single body.
    Surface & AddBody(Ptr<body_t> new_body) {
      body_set.push_back(new_body);          // Add body to master list
      TestBodySize(new_body);                // Keep track of largest body seen.
      if (data_active) PlaceBody(new_body);  // Add new body to a sector (if active).
      return *this;
    }

    /// Remove all bodies from the surface.
    Surface & Clear() {
      data_active = false;
      body_set.resize(0);
      sectors.resize(0);
      max_radius = 0.0;
      num_sectors = 0;
      return *this;
    }

    // Determine if two bodies overlap.
    static inline bool TestOverlap(body_t & body1, body_t & body2) {
      const Point xy_dist = body1.GetCenter() - body2.GetCenter();
      const double sqr_dist = xy_dist.SquareMagnitude();
      const double total_radius = body1.GetRadius() + body2.GetRadius();
      const double sqr_radius = total_radius * total_radius;
      return (sqr_dist < sqr_radius);
    }

    // Determine if a body overlaps with any others in a specified sector.
    inline void FindSectorOverlaps(body_t & body1, size_t sector_id,
                                   const overlap_fun_t & overlap_fun) {
      auto & sector = sectors[sector_id];
      for (size_t body2_id = 0; body2_id < sector.size(); body2_id++){
        body_t & body2 = *sector[body2_id];
        if (TestOverlap(body1, body2)) overlap_fun(body1, body2);
      }
    }

    // The following function will test all relevant pairs of bodies and run the passed-in
    // function on those objects that overlap.

    void FindOverlaps(const overlap_fun_t & overlap_fun) {
      emp_assert(overlap_fun);

      Activate();  // Make sure data structures are setup.

      // Loop through all of the sectors to identify collisions.
      for (size_t sector_id = 0; sector_id < num_sectors; sector_id++) {
        auto & cur_sector = sectors[sector_id];

        const size_t sector_col = sector_id % num_cols;
        const size_t sector_row = sector_id / num_cols;
        const bool left_ok = (sector_col > 0);
        const bool up_ok = (sector_row > 0);
        const bool ul_ok = left_ok && up_ok;
        const bool ur_ok = up_ok && (sector_col < num_cols - 1);
        const size_t ul_sector_id = sector_id - num_cols - 1;

        // Loop through all bodies in this sector
        for (size_t body1_id = 0; body1_id < cur_sector.size(); body1_id++) {
          auto & body1 = *cur_sector[body1_id];

          // Compare against the bodies before this one in this sector.
          for (size_t body2_id = 0; body2_id < body1_id; body2_id++) {
            auto & body2 = *cur_sector[body2_id];
            if (TestOverlap(body1, body2)) overlap_fun(body1, body2);
          }

          if (ul_ok) FindSectorOverlaps(body1, ul_sector_id, overlap_fun);   // Test bodies to upper-left.
          if (up_ok) FindSectorOverlaps(body1, ul_sector_id+1, overlap_fun); // Test bodies above
          if (ur_ok) FindSectorOverlaps(body1, ul_sector_id+2, overlap_fun); // Test bodies to upper-right.
          if (left_ok) FindSectorOverlaps(body1, sector_id-1, overlap_fun);  // Test bodies to left.
        }
      }

      // Make sure all bodies are in a legal position on the surface.
      // @CAO: Need to move to physics!
      // for (Ptr<BODY_TYPE> cur_body : body_set) {
      //   cur_body->FinalizePosition(max_pos);
      // }
    }

  };


}

#endif
