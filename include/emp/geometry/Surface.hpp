/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2021-2025 Michigan State University
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
#include <type_traits>

#include "../base/array.hpp"
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

    class Sector {
    private:
      size_t sector_id = NO_ID;
      emp::vector<size_t> body_ids = {};
      Box2D area = {};      // Specific portion of surface covered.

      // Track neighbor sector IDs (clockwise from UP): U, UR, R, DR, D, DL, L, UL
      emp::array<size_t, GridDir::SIZE> neighbor_ids =
        {NO_ID, NO_ID, NO_ID, NO_ID, NO_ID, NO_ID, NO_ID, NO_ID};

    public:
      [[nodiscard]] size_t GetID() const { return sector_id; }
      [[nodiscard]] const emp::vector<size_t> & GetBodyIDs() const { return body_ids; }
      [[nodiscard]] size_t NumBodies() const { return body_ids.size(); }

      [[nodiscard]] bool HasUp() const { return neighbor_ids[GridDir::UP] != NO_ID; }
      [[nodiscard]] bool HasDown() const { return neighbor_ids[GridDir::DOWN] != NO_ID; }
      [[nodiscard]] bool HasLeft() const { return neighbor_ids[GridDir::LEFT] != NO_ID; }
      [[nodiscard]] bool HasRight() const { return neighbor_ids[GridDir::RIGHT] != NO_ID; }

      [[nodiscard]] bool HasUL() const { return neighbor_ids[GridDir::UP_LEFT] != NO_ID; }
      [[nodiscard]] bool HasUR() const { return neighbor_ids[GridDir::UP_RIGHT] != NO_ID; }
      [[nodiscard]] bool HasDL() const { return neighbor_ids[GridDir::DOWN_LEFT] != NO_ID; }
      [[nodiscard]] bool HasDR() const { return neighbor_ids[GridDir::DOWN_RIGHT] != NO_ID; }

      [[nodiscard]] size_t GetUp() const { return neighbor_ids[GridDir::UP]; }
      [[nodiscard]] size_t GetDown() const { return neighbor_ids[GridDir::DOWN]; }
      [[nodiscard]] size_t GetLeft() const { return neighbor_ids[GridDir::LEFT]; }
      [[nodiscard]] size_t GetRight() const { return neighbor_ids[GridDir::RIGHT]; }

      [[nodiscard]] size_t GetUL() const { return neighbor_ids[GridDir::UP_LEFT]; }
      [[nodiscard]] size_t GetUR() const { return neighbor_ids[GridDir::UP_RIGHT]; }
      [[nodiscard]] size_t GetDL() const { return neighbor_ids[GridDir::DOWN_LEFT]; }
      [[nodiscard]] size_t GetDR() const { return neighbor_ids[GridDir::DOWN_RIGHT]; }

      void Setup(size_t in_sector_id, emp::array<size_t, 8> in_neighbor_ids, Box2D in_area) {
        body_ids.resize(0);
        sector_id = in_sector_id;
        neighbor_ids = in_neighbor_ids;
        area = in_area;
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

    // Data tracking the current bodies on this surface using SECTORS.
    bool data_active = false;  ///< Are we trying to keep measurements up-to-date?
    double max_radius = 0.0;   ///< Largest radius of any body.

    static constexpr uint32_t MAX_SECTOR_ROWS = 32;
    static constexpr uint32_t MAX_SECTOR_COLS = 32;
    static constexpr uint32_t MAX_SECTORS = MAX_SECTOR_ROWS * MAX_SECTOR_COLS;

    GridSize grid_size;                       ///< How are sectors organized?
    Size2D sector_size;                       ///< How big is each sector?
    uint32_t num_sectors = 0;                 ///< Current number of sectors used.
    emp::array<Sector, MAX_SECTORS> sectors;  ///< Info about the actual sectors.
    bool wrap = false;                        ///< Should edges wrap around?

    emp::vector<size_t> out_ids;  ///< Reusable vector to return ID sets by const reference.

    // === Helper Functions ===

    // Run a function on each active body.
    template <typename FUN_T>
    void ForEachBody(FUN_T && fun) {
      for (auto & body : body_set) {
        if (body.IsActive()) fun(body);
      }
    }

    // Make sure there are num_sectors sectors and remove all bodies from existing ones.
    void InitSectors() {
      num_sectors = grid_size.NumCells();
      emp::array<size_t, 8> neighbor_ids;

      for (size_t i = 0; i < num_sectors; ++i) {
        GridPos pos = grid_size.FromIndex(i); // Where is this sector on the grid?
        neighbor_ids[GridDir::UP]         = grid_size.ToIndex(grid_size.PosUp(pos, wrap));
        neighbor_ids[GridDir::UP_RIGHT]   = grid_size.ToIndex(grid_size.PosUR(pos, wrap));
        neighbor_ids[GridDir::RIGHT]      = grid_size.ToIndex(grid_size.PosRight(pos, wrap));
        neighbor_ids[GridDir::DOWN_RIGHT] = grid_size.ToIndex(grid_size.PosDR(pos, wrap));
        neighbor_ids[GridDir::DOWN]       = grid_size.ToIndex(grid_size.PosDown(pos, wrap));
        neighbor_ids[GridDir::DOWN_LEFT]  = grid_size.ToIndex(grid_size.PosDL(pos, wrap));
        neighbor_ids[GridDir::LEFT]       = grid_size.ToIndex(grid_size.PosLeft(pos, wrap));
        neighbor_ids[GridDir::UP_LEFT]    = grid_size.ToIndex(grid_size.PosUL(pos, wrap));

        Box2D area{sector_size*pos, sector_size};
        sectors[i].Setup(i, neighbor_ids, area);
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
      ForEachBody([this](BODY_T & body){ TestBodySize(body); });
    }

    // Determine the index of a sector a point is in.
    size_t FindSectorID(Point point) {
      emp_assert(Contains(point), point);
      GridPos sector_pos = (point / sector_size).ToGridPos();
      return grid_size.ToIndex(sector_pos);
    }

    // Determine which sector a point is in.
    Sector & FindSector(Point point) { return sectors[FindSectorID(point)]; }

    // Determine which sector a body is in.
    Sector & FindSector(BODY_T & body) { return FindSector(body.GetCenter()); }

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

      // Put active bodies into sectors
      ForEachBody([this](BODY_T & body){ PlaceBody(body); });
    }

  public:
    /// Create a surface providing maximum size.
    Surface(Size2D surface_size) : surface_size(surface_size) {}

    ~Surface() { Clear(); }

    [[nodiscard]] const Size2D & GetSize() const { return surface_size; }
    [[nodiscard]] bool GetWrap() const { return wrap; }
    [[nodiscard]] size_t NumBodies() const { return body_set.size() - open_ids.size(); }

    [[nodiscard]] size_t NumOpen() const { return open_ids.size(); } // DEBUG!

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

    /// Get the first active body you can find.
    [[nodiscard]] BODY_T & GetActiveBody() {
      size_t out_id = 0;
      while (out_id < body_set.size() && body_set[out_id].IsActive() == false) ++out_id;
      emp_assert(out_id < body_set.size());
      return body_set[out_id];
    }

    /// Does an id represent an active body on a surface?
    [[nodiscard]] bool IsActive(size_t id) const { return body_set[id].IsActive(); }

    [[nodiscard]] const Circle & GetPerimeter(size_t id) const {
      return GetBody(id).GetPerimeter();
    }

    [[nodiscard]] const Color & GetColor(size_t id) const { return GetBody(id).GetColor(); }

    [[nodiscard]] bool Contains(const Point2D & point) const {
      return surface_size.Contains(point);
    }

    void MoveTo(size_t id, Point pos) {
      if (wrap) pos = pos.Wrap(surface_size);
      emp_assert(Contains(pos));

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
      MoveTo(id, body_set[id].GetCenter() + translation);
    }

    void FinalizePosition(BODY_T & body) {
      size_t start_sector = FindSectorID(body.GetCenter());
      body.FinalizePosition();
      size_t end_sector = FindSectorID(body.GetCenter());
      if (start_sector != end_sector) {
        sectors[start_sector].Remove(body.GetID());
        sectors[end_sector].Insert(body.GetID());
      }
    }

    void SetRadius(size_t id, double _in) {
      emp_assert(body_set[id].IsActive());
      BODY_T & body = body_set[id];
      body.SetRadius(_in);
      TestBodySize(body);
    }

    void SetColor(size_t id, Color in) { body_set[id].SetColor(in); }

    void RemoveBody(size_t id) {
      BODY_T & body = body_set[id];
      emp_assert(body.IsActive());    // Ensure that this body is available to be removed.
      emp_assert(Contains(body.GetCenter()));
      if (data_active) {              // If we are tracking data right now...
        FindSector(body).Remove(id);  // ...remove this body from its sector.
      }
      body.Deactivate();              // Deactivate this body (so we know not to use it)
      open_ids.push_back(id);         // Mark this position as open for a new body.
    }

    /// Add a single body to surface; return its unique ID.
    auto & AddBody(Circle circle, emp::Color color) {
      emp_assert(Contains(circle.GetCenter()), surface_size, circle.GetCenter());

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
      body_set.resize(0);
      open_ids.resize(0);
      max_radius  = 0.0;
      num_sectors = 0;
      return *this;
    }

    /// Determine if two bodies overlap.
    static bool TestOverlap(const BODY_T & body1, const BODY_T & body2) {
      emp_assert(body1.IsActive() && body2.IsActive());
      const Point xy_dist       = body1.GetCenter() - body2.GetCenter();
      const double sqr_dist     = xy_dist.SquareMagnitude();
      const double total_radius = body1.GetRadius() + body2.GetRadius();
      const double sqr_radius   = total_radius * total_radius;
      return (sqr_dist < sqr_radius);
    }

    bool TestOverlap(size_t id1, size_t id2) {
      return TestOverlap(body_set[id1], body_set[id2]);
    }

    /// Update the list of overlaps in an internal sector.
    /// If add_on is false, it will reset the list of overlaps already found.
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
    template <typename OVERLAP_FUN_T>
    void TriggerOverlaps(OVERLAP_FUN_T && overlap_fun) {
      Activate();  // Set up data structures, if needed.

      // Loop through all of the sectors to identify collisions.
      for (size_t sector_id = 0; sector_id < num_sectors; ++sector_id) {
        const Sector & sector = sectors[sector_id];
        const auto & body_ids = sector.GetBodyIDs();

        // Loop through all bodies in this sector.
        // Note, we only test four of the eight directions since the other four will be tested
        // by neighbors.
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
    }

    void WrapBodies() {
      ForEachBody([this](BODY_T & body){
        if (body.X() < 0.0) body.X() += surface_size.X();
        else if (body.X() >= surface_size.X()) body.X() -= surface_size.X();
        if (body.Y() < 0.0) body.Y() += surface_size.Y();
        else if (body.Y() >= surface_size.Y()) body.Y() -= surface_size.Y();
      });
    }

    template <typename LOW_X_T, typename HIGH_X_T, typename LOW_Y_T, typename HIGH_Y_T>
    void TriggerOffsides(LOW_X_T low_x_fun, HIGH_X_T high_x_fun, LOW_Y_T low_y_fun, HIGH_Y_T high_y_fun) {
      emp_assert(wrap == false);
      // Check for high or low x.
      for (size_t row_id = 0; row_id < grid_size.NumRows(); ++row_id) {
        const auto & left_sector = sectors[row_id * grid_size.NumCols()];
        for (size_t id : left_sector.GetBodyIDs()) {
          if (body_set[id].X() < 0.0) low_x_fun(id);
        }

        const auto & right_sector = sectors[(row_id+1) * grid_size.NumCols() - 1];
        for (size_t id : right_sector.GetBodyIDs()) {
          if (body_set[id].X() > surface_size.Width()) high_x_fun(id);
        }
      }

      for (size_t col_id = 0; col_id < grid_size.NumCols(); ++col_id) {
        const auto & top_sector = sectors[col_id];
        for (size_t id : top_sector.GetBodyIDs()) {
          if (body_set[id].Y() < 0.0) low_y_fun(id);
        }

        const auto & bottom_sector = sectors[(grid_size.NumRows()-1) * grid_size.NumCols() + col_id];
        for (size_t id : bottom_sector.GetBodyIDs()) {
          if (body_set[id].Y() > surface_size.Height()) high_y_fun(id);
        }
      }
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
