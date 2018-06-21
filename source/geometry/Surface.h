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
//  Developer Notes:
//  * Sould add enums to control boundary conditions (INFINITE, TOROIDAL, BOUNDED)
//  * Incorporate physics?  Can have various plug-in modules.
//  * Use TypeTracker to allow variable types of bodies.

#ifndef EMP_SURFACE2_H
#define EMP_SURFACE2_H

#include <algorithm>
#include <functional>

#include "../base/Ptr.h"
#include "../tools/vector_utils.h"

#include "Angle2D.h"
#include "Point2D.h"

namespace emp {

  template <typename BODY_TYPE>
  class Surface {
  public:
    using body_t = BODY_TYPE;

    struct BodyInfo {
      Ptr<body_t> body_ptr;  // Pointer to the bodies
      size_t id;             // Position in body_set to find body info
      Point center;          // Center position of this body
      double radius;         // Size of this body
      size_t color;          // Color of this body

      BodyInfo(Ptr<body_t> _ptr, size_t _id, Point _center, double _radius, size_t _color=0)
        : body_ptr(_ptr), id(_id), center(_center), radius(_radius), color(_color) { ; }
      BodyInfo(size_t _id, Point _center, double _radius)
        : BodyInfo(nullptr, _id, _center, _radius) { ; }
    };

    using sector_t = emp::vector<size_t>;
    using overlap_fun_t = std::function<void(body_t &, body_t &)>;

  protected:
    const Point max_pos;             // Lower-left corner of the surface.
    emp::vector<BodyInfo> body_set;  // Set of all bodies on surface

    // Data tracking the current bodies on this surface using sectors.
    bool data_active;                // Are we trying to keep data up-to-date?
    double max_radius;               // Largest radius of any body.
    size_t num_cols;                 // How many cols of sectors are there?
    size_t num_rows;                 // How many rows of sectors are there?
    size_t num_sectors;              // How many total sectors are there?
    double sector_width;             // How wide is each sector?
    double sector_height;            // How tall is each sector?
    emp::vector<sector_t> sectors;   // Which bodies are in each sector?

    // Make sure there are num_sectors sectors and remove all bodies from existing ones.
    void InitSectors() {
      size_t min_size = std::min(num_sectors, sectors.size());
      sectors.resize(num_sectors);
      for (size_t i = 0; i < min_size; i++) sectors[i].clear();
    }

    // Keep track of the largest body size found.
    // Note: Uses watermarking, so largest body will never shrink, even if removed
    //       unless the user explicitly calls RefreshBodySize()
    inline void TestBodySize(BodyInfo & body) {
      const double cur_radius = body.radius;
      if (cur_radius > max_radius) {
        max_radius = cur_radius;      // Record the new radius.
        data_active = false;          // May need to rebuild sectors, so deactivate data.
      }
    }

    // Clear out the watermarked body size and update the current largest.
    inline void RefreshBodySize() {
      max_radius = 0.0;
      for (BodyInfo & body : body_set) TestBodySize(body);      
    }

    // Determine which sector a body should be in.
    size_t FindSector(Point point) {
      // Where is the body?
      const double body_x = point.GetX();
      const double body_y = point.GetY();

      // Make sure the body is in the sectors.
      emp_assert(body_x >= 0.0 && body_x < max_pos.GetX());
      emp_assert(body_y >= 0.0 && body_y < max_pos.GetY());

      // Determine which sector the body is in.
      const double sector_x = body_x / sector_width;
      const double sector_y = body_y / sector_height;

      const size_t cur_col = (size_t) sector_x;
      const size_t cur_row = (size_t) sector_y;
      const size_t cur_sector = cur_col + cur_row * num_cols;

      emp_assert(cur_sector < sectors.size());
      return cur_sector;
    }

    // Place an active body into a sector.
    void PlaceBody(BodyInfo & body) {
      size_t cur_sector = FindSector(body.center);
      sectors[cur_sector].push_back(body.id);
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

      InitSectors();   // Now that we know the sizes, we can initialize sectors.

      // Put all of the bodies into sectors
      for (BodyInfo & body : body_set) PlaceBody(body);

      data_active = true;
    }
  public:
    Surface(Point _max) : max_pos(_max), body_set(), max_radius(0.0) { ; }
    ~Surface() { Clear(); }

    double GetWidth() const { return max_pos.GetX(); }
    double GetHeight() const { return max_pos.GetY(); }
    const Point & GetMaxPosition() const { return max_pos; }
    const emp::vector<BodyInfo> & GetBodySet() const { return body_set; }

    Ptr<body_t> GetPtr(size_t id) const { return body_set[id].body_ptr; }
    Point GetCenter(size_t id) const { return body_set[id].center; }
    double GetRadius(size_t id) const { return body_set[id].radius; }
    size_t GetColor(size_t id) const { return body_set[id].color; }

    void SetPtr(size_t id, Ptr<body_t> _in) { body_set[id].body_ptr = _in; }
    void SetCenter(size_t id, Point _in) {
      // If not active, just move the body.
      if (data_active == false) body_set[id].center = _in;
      // Otherwise need to update data.
      else {
        const size_t old_sector_id = FindSector(body_set[id].center);
        const size_t new_sector_id = FindSector(_in);
        body_set[id].center = _in;
        if (old_sector_id != new_sector_id) {
          emp::RemoveValue(sectors[old_sector_id], id); // Remove from old sector.
          sectors[new_sector_id].push_back(id);         // Insert into new sector.
        }
      }
    }
    void Translate(size_t id, Point translation) {
      SetCenter(id, body_set[id].center + translation);
    }

    void SetRadius(size_t id, double _in) {
      BodyInfo & body = body_set[id];
      body.radius = _in;
      TestBodySize(body);
    }
    void ScaleRadius(size_t id, double scale) {
      BodyInfo & body = body_set[id];
      body.radius *= scale;
      TestBodySize(body);
    }
    void SetColor(size_t id, size_t _in) { body_set[id].color = _in; }
    
    void RemoveBody(size_t id) {
      // @CAO Change id in body_set to nullptr; record cell id for re-use
      if (data_active) {
        const size_t sector_id = FindSector(body_set[id].center);
        emp::RemoveValue(sectors[sector_id], id);
      }
    }

    /// Add a single body; return its unique ID.
    size_t AddBody(Ptr<body_t> _body, Point _center, double _radius, size_t _color=0) {
      size_t id = body_set.size();        // Figure out the ID for this body
      BodyInfo info = { _body, id, _center, _radius, _color };

      body_set.emplace_back(info);        // Add body to master list
      TestBodySize(info);                 // Keep track of largest body seen.
      if (data_active) PlaceBody(info);   // Add new body to a sector (if active).
      return id;
    }

    /// Remove all bodies from the surface.
    Surface & Clear() {
      data_active = false;
      body_set.clear();
      max_radius = 0.0;
      num_sectors = 0;
      return *this;
    }

    // Determine if two bodies overlap.
    static inline bool TestOverlap(const BodyInfo & body1, const BodyInfo & body2) {
      const Point xy_dist = body1.center - body2.center;
      const double sqr_dist = xy_dist.SquareMagnitude();
      const double total_radius = body1.radius + body2.radius;
      const double sqr_radius = total_radius * total_radius;
      return (sqr_dist < sqr_radius);
    }

    // Determine if a body overlaps with any others in a specified sector.
    inline void FindSectorOverlaps(BodyInfo & body1, size_t sector_id,
                                   const overlap_fun_t & overlap_fun,
                                   size_t start_id=0) {
      auto & sector = sectors[sector_id];
      for (size_t body2_id = start_id; body2_id < sector.size(); body2_id++){
        BodyInfo & body2 = body_set[sector[body2_id]];
        if (TestOverlap(body1, body2)) overlap_fun(*body1.body_ptr, *body2.body_ptr);
      }
    }

    // The following function will test all relevant pairs of bodies and run the passed-in
    // function on those objects that overlap.

    void FindOverlaps(const overlap_fun_t & overlap_fun) {
      emp_assert(overlap_fun);

      Activate();  // Make sure data structures are setup.

      // Loop through all of the sectors to identify collisions.
      for (size_t sector_id = 0; sector_id < num_sectors; sector_id++) {
       sector_t & cur_sector = sectors[sector_id];

        const size_t sector_col = sector_id % num_cols;
        const size_t sector_row = sector_id / num_cols;
        const bool left_ok = (sector_col > 0);
        const bool up_ok = (sector_row > 0);
        const bool ul_ok = left_ok && up_ok;
        const bool ur_ok = up_ok && (sector_col < num_cols - 1);
        const size_t ul_sector_id = sector_id - num_cols - 1;

        // Loop through all bodies in this sector
        for (size_t body_id = 0; body_id < cur_sector.size(); body_id++) {
          BodyInfo & body = body_set[cur_sector[body_id]];

          // Compare against the bodies after this one in the current sector.
          FindSectorOverlaps(body, sector_id, overlap_fun, body_id+1);      // Test remaining bodies here.
          if (ul_ok) FindSectorOverlaps(body, ul_sector_id, overlap_fun);   // Test bodies to upper-left.
          if (up_ok) FindSectorOverlaps(body, ul_sector_id+1, overlap_fun); // Test bodies above
          if (ur_ok) FindSectorOverlaps(body, ul_sector_id+2, overlap_fun); // Test bodies to upper-right.
          if (left_ok) FindSectorOverlaps(body, sector_id-1, overlap_fun);  // Test bodies to left.
        }
      }

      // Make sure all bodies are in a legal position on the surface.
      // @CAO: Need to move to physics!
      // for (Ptr<BODY_TYPE> cur_body : body_set) {
      //   cur_body->FinalizePosition(max_pos);
      // }
    }

    /// Determine if there are any overlaps with a provided body (that may or may not be on surface).
    void FindOverlap(const BodyInfo & body, const overlap_fun_t & overlap_fun) {
      const size_t sector_id = FindSector(body.center);
      const size_t sector_col = sector_id % num_cols;
      const size_t sector_row = sector_id / num_cols;
      auto & cur_sector = sectors[sector_id];

      // Compare against bodies in its own sector.
      for (size_t body2_id : cur_sector) {
        BodyInfo & body2 = body_set[body2_id];
        if (body == body2) continue; // Don't match with self!
        if (TestOverlap(body, body2)) overlap_fun(*body.body_ptr, *body2.body_ptr);
      }

      const bool left_ok  = (sector_col > 0);
      const bool right_ok = (sector_col < num_cols - 1);
      const bool up_ok    = (sector_row > 0);
      const bool down_ok  = (sector_row < num_rows - 1);

      const size_t up_sector_id = sector_id - num_cols;
      const size_t down_sector_id = sector_id + num_cols;

      // Compare against bodies in other sectors....
      if (up_ok)               FindSectorOverlaps(body, up_sector_id,     overlap_fun);
      if (up_ok && left_ok)    FindSectorOverlaps(body, up_sector_id-1,   overlap_fun);
      if (up_ok && right_ok)   FindSectorOverlaps(body, up_sector_id+1,   overlap_fun);
      if (left_ok)             FindSectorOverlaps(body, sector_id-1,      overlap_fun);
      if (right_ok)            FindSectorOverlaps(body, sector_id+1,      overlap_fun);
      if (down_ok)             FindSectorOverlaps(body, down_sector_id,   overlap_fun); 
      if (down_ok && left_ok)  FindSectorOverlaps(body, down_sector_id-1, overlap_fun);
      if (down_ok && right_ok) FindSectorOverlaps(body, down_sector_id+1, overlap_fun);
    }

    // Find overlaps using a distance from a point.
    void FindOverlap(Point center, double radius, const overlap_fun_t & overlap_fun) {
      BodyInfo tmp_body(center, radius);
      FindOverlap(tmp_body, overlap_fun);
    }

  };


}

#endif
