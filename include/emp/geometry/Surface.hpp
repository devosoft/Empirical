/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2021 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/geometry/Surface.hpp
 * @brief A 2D surface that maintains data about 2D bodies, identifying if they overlap.
 *
 * Developer Notes:
 *  - Should add enums to control boundary conditions (INFINITE, TOROIDAL, BOUNDED)
 *  - Incorporate physics as plug-in modules?
 */

#pragma once

#ifndef INCLUDE_EMP_GEOMETRY_SURFACE_HPP_GUARD
#define INCLUDE_EMP_GEOMETRY_SURFACE_HPP_GUARD

#include <algorithm>
#include <concepts>
#include <functional>
#include <stddef.h>

#include "../datastructs/vector_utils.hpp"
#include "../math/constants.hpp"
#include "../tools/GridSize.hpp"
#include "../web/Color.hpp"

#include "Box2D.hpp"
#include "Circle2D.hpp"
#include "Point2D.hpp"
#include "SurfaceBody.hpp"

namespace emp {

  template <typename BODY_T>
    requires std::is_base_of_v<SurfaceBody, BODY_T>
  class Surface {
  public:
    static constexpr size_t NO_ID = MAX_SIZE_T;
    using overlap_fun_t = std::function<void(size_t, size_t)>;

    class Sector {
    public:
      enum Direction { UP=0, UP_RIGHT, RIGHT, DOWN_RIGHT, DOWN, DOWN_LEFT, LEFT, UP_LEFT, SIZE };

    private:
      size_t sector_id = NO_ID;
      emp::vector<size_t> body_ids = {};

      Box2D area = {};      // Specific portion of surface covered.
      Box2D mid_area = {};  // Internal area that cannot be touched by neighbors

      // Track neighbor sector IDs (clockwise from 12): U, UR, R, DR, D, DL, L, UL
      emp::array<size_t, Direction::SIZE> neighbor_ids =
        {NO_ID, NO_ID, NO_ID, NO_ID, NO_ID, NO_ID, NO_ID, NO_ID};

    public:
      [[nodiscard]] size_t GetID() const { return sector_id; }
      [[nodiscard]] const emp::vector<size_t> & GetBodyIDs() const { return body_ids; }
      [[nodiscard]] size_t NumBodies() const { return body_ids.size(); }

      [[nodiscard]] bool HasUp() const { return neighbor_ids[UP] != NO_ID; }
      [[nodiscard]] bool HasDown() const { return neighbor_ids[DOWN] != NO_ID; }
      [[nodiscard]] bool HasLeft() const { return neighbor_ids[LEFT] != NO_ID; }
      [[nodiscard]] bool HasRight() const { return neighbor_ids[RIGHT] != NO_ID; }

      [[nodiscard]] bool HasUL() const { return neighbor_ids[UP_LEFT] != NO_ID; }
      [[nodiscard]] bool HasUR() const { return neighbor_ids[UP_RIGHT] != NO_ID; }
      [[nodiscard]] bool HasDL() const { return neighbor_ids[DOWN_LEFT] != NO_ID; }
      [[nodiscard]] bool HasDR() const { return neighbor_ids[DOWN_RIGHT] != NO_ID; }

      [[nodiscard]] size_t GetUp() const { return neighbor_ids[UP]; }
      [[nodiscard]] size_t GetDown() const { return neighbor_ids[DOWN]; }
      [[nodiscard]] size_t GetLeft() const { return neighbor_ids[LEFT]; }
      [[nodiscard]] size_t GetRight() const { return neighbor_ids[RIGHT]; }

      [[nodiscard]] size_t GetUL() const { return neighbor_ids[UP_LEFT]; }
      [[nodiscard]] size_t GetUR() const { return neighbor_ids[UP_RIGHT]; }
      [[nodiscard]] size_t GetDL() const { return neighbor_ids[DOWN_LEFT]; }
      [[nodiscard]] size_t GetDR() const { return neighbor_ids[DOWN_RIGHT]; }

      void Setup(size_t in_sector_id, emp::array<size_t, 8> in_neighbor_ids,
                 Box2D in_area, Box2D in_mid_area) {
        body_ids.resize(0);
        sector_id = in_sector_id;
        neighbor_ids = in_neighbor_ids;
        area = in_area;
        mid_area = in_mid_area;
      }

      void Insert(size_t body_id) { body_ids.push_back(body_id); }

      void Remove(size_t body_id) {
        auto it = std::find(body_ids.begin(), body_ids.end(), body_id);
        emp_assert(it != body_ids.end());  // Make sure we found the id!
        *it = std::move(body_ids.back());  // Move the last id into place.
        body_ids.pop_back();
      }
    };

  protected:
    const Size2D surface_size;     ///< Lower-left corner of the surface.
    emp::vector<BODY_T> body_set;  ///< Set of all bodies on surface
    emp::vector<size_t> open_ids;  ///< Set of body_set ids ready for re-use.
    overlap_fun_t overlap_fun;     ///< Function to trigger when overlaps occur.

    // Data tracking the current bodies on this surface using SECTORS.
    bool data_active = false;  ///< Are we trying to keep measurements up-to-date?
    double max_radius = 0.0;   ///< Largest radius of any body.
    size_t max_count = 0;      ///< How many instances of max_radius? (rescan when 0)

    static constexpr uint32_t MAX_SECTOR_ROWS = 32;
    static constexpr uint32_t MAX_SECTOR_COLS = 32;
    static constexpr uint32_t MAX_SECTORS = MAX_SECTOR_ROWS * MAX_SECTOR_COLS;

    GridSize grid_size;                       ///< How are sectors organized?
    Size2D sector_size;                       ///< How big is each sector?
    uint32_t num_sectors = 0;                 ///< Current number of sectors used.
    emp::array<Sector, MAX_SECTORS> sectors;  ///< Info about the actual sectors.
    bool wrap = false;                        ///< Should edges wrap around?

    emp::vector<size_t> out_ids;  ///< Reusable vector to return ID sets by const reference.

    // Make sure there are num_sectors sectors and remove all bodies from existing ones.
    void InitSectors() {
      num_sectors = grid_size.NumCells();
      emp::array<size_t, 8> neighbor_ids;
      Size2D offset_size{4*max_radius, 4*max_radius};
      Size2D mid_size = sector_size - offset_size;

      for (size_t i = 0; i < num_sectors; ++i) {
        GridPos pos = grid_size.FromIndex(i); // Where is this sector on the grid?
        neighbor_ids[Sector::UP]         = grid_size.ToIndex(grid_size.PosUp(pos, wrap));
        neighbor_ids[Sector::UP_RIGHT]   = grid_size.ToIndex(grid_size.PosUR(pos, wrap));
        neighbor_ids[Sector::RIGHT]      = grid_size.ToIndex(grid_size.PosRight(pos, wrap));
        neighbor_ids[Sector::DOWN_RIGHT] = grid_size.ToIndex(grid_size.PosDR(pos, wrap));
        neighbor_ids[Sector::DOWN]       = grid_size.ToIndex(grid_size.PosDown(pos, wrap));
        neighbor_ids[Sector::DOWN_LEFT]  = grid_size.ToIndex(grid_size.PosDL(pos, wrap));
        neighbor_ids[Sector::LEFT]       = grid_size.ToIndex(grid_size.PosLeft(pos, wrap));
        neighbor_ids[Sector::UP_LEFT]    = grid_size.ToIndex(grid_size.PosUL(pos, wrap));

        Box2D area{sector_size*pos, sector_size};
        Box2D mid_area = {area.GetUL() + offset_size/2, mid_size};

        sectors[i].Setup(i, neighbor_ids, area, mid_area);
      }
    }

    /// Get (or make) an available ID for a new body.
    size_t ReserveBodyID() {
      if (open_ids.size()) {  // If we have an available ID, use it.
        size_t id = open_ids.back();
        open_ids.pop_back();
        return id;
      }
      else {
        size_t id = body_set.size();
        body_set.resize(body_set.size()+1);
        return id;
      }
    }

    // Keep track of the largest body size found.
    // Note: Uses watermarking, so largest body will never shrink, even if removed
    //       unless the user explicitly calls RefreshBodySize()
    void TestBodySize(BODY_T & body) {
      const double cur_radius = body.GetPerimeter().GetRadius();
      if (cur_radius > max_radius) {
        max_radius  = cur_radius;  // Record the new radius.
        data_active = false;       // May need to rebuild sectors, so deactivate data.
      }
    }

    // Clear out the watermarked body size and update the current largest.
    void RefreshBodySize() {
      max_radius = 0.0;
      for (BODY_T & body : body_set) {
        if (body.IsActive()) { TestBodySize(body); }
      }
    }

    // Determine which sector a point is in.
    Sector & FindSector(Point point) {
      emp_assert(point.IsNonNegative() && surface_size.Contains(point));
      GridPos sector_pos = (point / sector_size).ToGridPos();
      return sectors[grid_size.ToIndex(sector_pos)];
    }

    // Determine which sectos a body is in.
    Sector & FindSector(BODY_T & body) { return FindSector(body.GetPerimeter().GetCenter()); }

    // Place an active body into a sector.
    void PlaceBody(BODY_T & body) {
      emp_assert(body.IsActive());  // Only place active bodies.
      FindSector(body).Insert(body.GetID());
    }

    // Cleanup all of the data and mark the sector as active.
    void Activate() {
      if (data_active) { return; } // Already active!
      data_active = true;

      // Figure out the actual number of sectors to use.
      const double max_diameter = max_radius * 2.0;
      emp_assert(max_diameter < surface_size.Width() && max_diameter < surface_size.Height());
      grid_size = (surface_size / max_diameter).ToGridSize();
      grid_size.BoundUpper({32, 32});  // Max sectors is 1024.
      sector_size = surface_size / grid_size;

      InitSectors();  // Now that we know the sizes, we can initialize sectors.

      for (BODY_T & body : body_set) {  // Put active bodies into sectors
        if (body.IsActive()) PlaceBody(body);
       } 
    }

  public:
    /// Create a surface providing maximum size.
    Surface(Size2D surface_size) : surface_size(surface_size) {}

    ~Surface() { Clear(); }

    [[nodiscard]] const Size2D & GetSize() const { return surface_size; }

    [[nodiscard]] BODY_T & GetBody(size_t id) {
      emp_assert(body_set[id].IsActive());
      return body_set[id];      
    }

    [[nodiscard]] const BODY_T & GetBody(size_t id) const {
      emp_assert(body_set[id].IsActive());
      return body_set[id];      
    }

    [[nodiscard]] emp::vector<BODY_T> & GetBodySet() { return body_set; }
    [[nodiscard]] const emp::vector<BODY_T> & GetBodySet() const { return body_set; }

    /// Does an id represent an active body on a surface?
    [[nodiscard]] bool IsActive(size_t id) const { return body_set[id].IsActive(); }

    [[nodiscard]] const Circle & GetPerimeter(size_t id) const {
      return GetBody(id).GetPerimeter();
    }

    [[nodiscard]] const Color & GetColor(size_t id) const { return GetBody(id).GetColor(); }

    void MoveTo(size_t id, Point pos) {
      if (wrap) pos = pos.Wrap(surface_size);
      emp_assert(surface_size.Contains(pos));

      // If data is active, update sectors.
      if (data_active) {
        Sector & old_sector = FindSector(GetBody(id));
        Sector & new_sector = FindSector(pos);
        if (old_sector.GetID() != new_sector.GetID()) {
          old_sector.Remove(id);
          new_sector.Insert(id);
        }
      }

      GetBody(id).MoveTo(pos);
    }

    void MoveBy(size_t id, Point translation) {
      MoveTo(id, body_set[id].GetPerimeter().GetCenter() + translation);
    }

    void SetRadius(size_t id, double _in) {
      emp_assert(body_set[id].IsActive());
      BODY_T & body = body_set[id];
      body.SetRadius(_in);
      TestBodySize(body);
    }

    void SetColorID(size_t id, size_t _in) {
      emp_assert(body_set[id].IsActive());
      body_set[id].SetColorID(_in);
    }

    void RemoveBody(size_t id) {
      emp_assert(body_set[id].IsActive());
      body_set[id].Deactivate();               // Deactivate this body (so we know not to use it)
      open_ids.push_back(id);                  // Mark this position as open for a new body.
      if (data_active) {                       // If we are tracking data right now...
        FindSector(body_set[id]).Remove(id);   // ...remove this body from its sector.
      }
    }

    /// Add a single body to surface; return its unique ID.
    auto & AddBody(Circle circle, emp::Color color) {
      emp_assert(surface_size.Contains(circle.GetCenter()), surface_size, circle.GetCenter());

      const size_t id = ReserveBodyID();
      auto & body = body_set[id];
      body = BODY_T{circle, color};
      body.Activate(id);

      TestBodySize(body);                    // Track largest body seen.
      if (data_active) { PlaceBody(body); }  // Add new body to a sector, if tracking.
      return body;
    }

    /// Remove all bodies from the surface.
    Surface & Clear() {
      data_active = false;
      body_set.clear();
      max_radius  = 0.0;
      num_sectors = 0;
      return *this;
    }

    /// Add a function to call in the case of overlaps (using an std::function object)
    void SetOverlapFun(const overlap_fun_t & in_overlap_fun) { overlap_fun = in_overlap_fun; }

    /// Determine if two bodies overlap.
    static inline bool TestOverlap(const BODY_T & body1, const BODY_T & body2) {
      const Point xy_dist       = body1.GetPerimeter().GetCenter() - body2.GetPerimeter().GetCenter();
      const double sqr_dist     = xy_dist.SquareMagnitude();
      const double total_radius = body1.GetRadius() + body2.GetRadius();
      const double sqr_radius   = total_radius * total_radius;
      return (sqr_dist < sqr_radius);
    }

    bool TestOverlap(size_t id1, size_t id2) {
      return TestOverlap(body_set[id1], body_set[id2]);
    }

    /// Update the list of overlaps in an internal sector.
    /// If add_on is false, it will add on to the list of overlaps already found.
    const emp::vector<size_t> & FindOverlaps(const BODY_T & body, const Sector & sector,
                                             size_t start_pos = 0, bool add_on = false) {
      emp_assert(body.IsActive());
      if (!add_on) out_ids.clear();
      const auto & body_ids = sector.GetBodyIDs();
      for (size_t pos = start_pos; pos < body_ids.size(); ++pos) {
        size_t test_id = body_ids[pos];
        emp_assert(body_set[test_id].IsActive());
        if (body.GetID() == test_id) { continue; }  // Don't match with self!
        if (TestOverlap(body, body_set[test_id])) { out_ids.push_back(test_id); }
      }
      return out_ids;
    }

    /// Find pairs of bodies that overlap, and run passed-in function on those IDs.
    void TriggerOverlaps() {
      Activate();  // Set up data structures, if needed.

      // Loop through all of the sectors to identify collisions.
      for (size_t sector_id = 0; sector_id < num_sectors; ++sector_id) {
        const Sector & sector = sectors[sector_id];
        const auto & body_ids = sector.GetBodyIDs();

        // Loop through all bodies in this sector
        for (size_t pos=0; pos < body_ids.size(); ++pos) {
          const BODY_T & body = body_set[body_ids[pos]];
          out_ids.clear();  // Clear the set of IDs that could overlap with this body.
          FindOverlaps(body, sector, pos+1, true);
          if (sector.HasLeft()) FindOverlaps(body, sectors[sector.GetLeft()]);
          if (sector.HasUL()) FindOverlaps(body, sectors[sector.GetUL()]);
          if (sector.HasUp()) FindOverlaps(body, sectors[sector.GetUp()]);
          if (sector.HasUR()) FindOverlaps(body, sectors[sector.GetUR()]);
          // ...other four directions will be handled when those sectors are processed.

          // Trigger overlap function for each one found.
          for (size_t overlap_id : out_ids) {
            overlap_fun(body.GetID(), overlap_id);
          }
        }
      }

      // Make sure all bodies are in a legal position on the surface.
      // @CAO: Need to move to physics!
      // for (Ptr<BODY_TYPE> cur_body : body_set) {
      //   if (body_set[id].IsActive() == false) continue;
      //   cur_body->FinalizePosition(max_pos);
      // }
    }

    /// Determine if there are any overlaps with a single provided body (that may or may not be on surface).
    const emp::vector<size_t> & FindOverlaps(BODY_T & body) {
      Activate();  // Make sure data structures are setup.

      Sector & sector = FindSector(body);
      out_ids.clear();  // Prepare the return variable.

      // Compare against bodies in its own sector.
      FindOverlaps(body, sector);

      // Compare against bodies in other sectors....
      if (sector.HasUp())    { FindOverlaps(body, sectors[sector.GetUp()]); }
      if (sector.HasUL())    { FindOverlaps(body, sectors[sector.GetUL()]); }
      if (sector.HasUR())    { FindOverlaps(body, sectors[sector.GetUR()]); }
      if (sector.HasLeft())  { FindOverlaps(body, sectors[sector.GetLeft()]); }
      if (sector.HasRight()) { FindOverlaps(body, sectors[sector.GetRight()]); }
      if (sector.HasDL())    { FindOverlaps(body, sectors[sector.GetDL()]); }
      if (sector.HasDown())  { FindOverlaps(body, sectors[sector.GetDown()]); }
      if (sector.HasDR())    { FindOverlaps(body, sectors[sector.GetDR()]); }

      return out_ids;
    }

    const emp::vector<size_t> & FindOverlaps(size_t body_id) {
      emp_assert(body_set[body_id].IsActive());
      return FindOverlaps(body_set[body_id]);
    }

    // Find overlaps using a distance from a point.
    void FindOverlaps(Circle circle) {
      BODY_T tmp_body(NO_ID, circle);
      FindOverlaps(tmp_body);
    }
  };

  using BasicSurface = Surface<SurfaceBody>;

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_GEOMETRY_SURFACE_HPP_GUARD
