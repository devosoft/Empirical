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
#include <cmath>
#include <functional>
#include <stddef.h>
#include <type_traits>

#include "../base/array.hpp"
#include "../datastructs/vector_utils.hpp"
#include "../io/io_utils.hpp"
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
  private:
    static constexpr size_t NO_ID = MAX_SIZE_T;

    class Sector {
    public:
      using neighbors_t = emp::array<size_t, GridDir::SIZE>;
    private:
      size_t sector_id = NO_ID;
      emp::vector<size_t> body_ids = {};
      Box2D area = {};      // Specific portion of surface covered.

      // Track neighbor sector IDs (clockwise from UP): U, UR, R, DR, D, DL, L, UL
      neighbors_t neighbor_ids = {NO_ID, NO_ID, NO_ID, NO_ID, NO_ID, NO_ID, NO_ID, NO_ID};

    public:
      void Serialize(SerialPod & pod) { pod(body_ids); }

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

      void Setup(size_t in_sector_id, const neighbors_t & in_neighbor_ids, Box2D in_area) {
        DEBUG_STACK();
        body_ids.resize(0);  // Clear out any bodies previously in sector.
        sector_id = in_sector_id;
        neighbor_ids = in_neighbor_ids;
        area = in_area;
      }

      [[nodiscard]] bool HasID(size_t body_id) const {
        DEBUG_STACK();
        return std::find(body_ids.begin(), body_ids.end(), body_id) != body_ids.end();
      }

      void Insert(size_t body_id) {
        DEBUG_STACK();
        emp_assert(!HasID(body_id), body_id);
        body_ids.push_back(body_id);
      }

      void Remove(size_t body_id) {
        DEBUG_STACK();
        emp_assert(HasID(body_id), body_id);
        auto it = std::find(body_ids.begin(), body_ids.end(), body_id);
        *it = body_ids.back();  // Copy the last ID into place.
        body_ids.pop_back();    // Remove the last ID.
      }
    };

  protected:
    Size2D surface_size;          ///< Lower-left corner of the surface.
    emp::vector<BODY_T> body_set;       ///< Set of all bodies on surface
    EMP_DEBUG(int body_version = 1000;) // Changes to body_set may invalidate body references.

    emp::vector<size_t> open_ids;  ///< Set of body_set IDs ready for re-use.

    // Make sure that we know if sectors need to be updated.
    double max_radius = 0.0;   ///< Largest radius of any body.

    static constexpr GridSize MAX_GRID_SIZE{32, 32};
    static constexpr uint32_t MAX_SECTORS = MAX_GRID_SIZE.NumCells();

    GridSize grid_size;                       ///< How are sectors organized?
    Size2D sector_size;                       ///< How big is each sector?
    emp::array<Sector, MAX_SECTORS> sectors;  ///< Info about the actual sectors.
    bool loaded_state_valid = true;
    bool wrap = false;                        ///< Should edges wrap around?

    emp::vector<size_t> out_ids;  ///< Reusable vector to return ID sets by const reference.

    // === Helper Functions ===

    GridSize CalculateGridSize(double radius) const {
      if (radius == 0) return GridSize{1, 1};
      // Clamp before converting to integer: small bodies can imply more than 2^32 sectors.
      return GridSize{
        size_t(std::min(double(MAX_GRID_SIZE.NumRows()), surface_size.Height() / (2 * radius))),
        size_t(std::min(double(MAX_GRID_SIZE.NumCols()), surface_size.Width() / (2 * radius)))};
    }

    // Determine how many sectors there should be and rebuild existing ones, as needed.
    void ResetSectors() {
      DEBUG_STACK();
      // Figure out the actual number of sectors to use.
      emp_assert(max_radius * 2 < surface_size.Width() && max_radius * 2 < surface_size.Height());
      grid_size = CalculateGridSize(max_radius);
      sector_size = surface_size / grid_size;

      typename Sector::neighbors_t neighbor_ids;

      for (size_t sector_id = 0; sector_id < grid_size.NumCells(); ++sector_id) {
        const GridPos pos = grid_size.FromIndex(sector_id); // Where is this sector on the grid?
        for (GridDir dir : Grid::DIR_SET) {
          neighbor_ids[dir] = grid_size.ToIndex(grid_size.PosDir(pos, dir, wrap));
        }

        Box2D area{sector_size*pos, sector_size};
        sectors[sector_id].Setup(sector_id, neighbor_ids, area);
      }

      // Put active bodies into sectors
      ForEachBody([this](BODY_T & body){ PlaceBody(body); });
    }

    /// Get (or make) an available ID for a new body.
    BODY_T & ReserveBody(const emp::Circle & circle, emp::Color color) {
      DEBUG_STACK();
      EMP_DEBUG(++body_version);
      size_t body_id;
      if (open_ids.size()) {  // If we have an available ID, use it.
        body_id = open_ids.back();
        open_ids.pop_back();
      } else {
        body_id = body_set.size();
        body_set.resize(body_set.size()+1);
      }

      body_set[body_id] = BODY_T{circle, color};             // Build the new body
      EMP_DEBUG(body_set[body_id].SetVersion(body_version)); // Algin its version with surface
      body_set[body_id].Activate(body_id);                   // Assign its ID.
      return body_set[body_id];
    }

    // Keep track of the largest body size found.
    // Note: Uses watermarking, so largest body will never shrink, even if removed
    //       unless the user explicitly calls RefreshBodySize()
    void TestBodySize(const double radius) {
      DEBUG_STACK();
      emp_assert(radius > 0.0);
      if (radius > max_radius) {
        max_radius = radius;
        const GridSize next_grid = CalculateGridSize(radius);
        if (next_grid != grid_size) ResetSectors();
      }
    }

    // Clear out the watermarked body size and update the current largest.
    void RefreshBodySize() {
      DEBUG_STACK();
      max_radius = 0.0;
      ForEachBody([this](BODY_T & body){ TestBodySize(body.GetRadius()); });
    }

    // Determine the index of a sector a point is in.
    size_t FindSectorID(Point point) {
      DEBUG_STACK();
      emp_assert(Contains(point), point);
      GridPos sector_pos = (point / sector_size).ToGridPos();
      return grid_size.ToIndex(sector_pos);
    }

    // Determine which sector a point is in.
    Sector & FindSector(Point point) {
      DEBUG_STACK();
      return sectors[FindSectorID(point)];
    }

    // Determine which sector a body is in.
    Sector & FindSector(BODY_T & body) { return FindSector(body.GetCenter()); }

    // Place an active body into a sector.
    void PlaceBody(BODY_T & body) {
      DEBUG_STACK();
      emp_assert(body.IsActive());  // Only place active bodies.
      FindSector(body).Insert(body.GetID());
    }

    #ifndef NDEBUG
    void UpdateBodyVersions() const {
      DEBUG_STACK();
      for (const auto & body : body_set) { body.SetVersion(body_version); }
    }
    #endif

  public:
    /// Create a surface providing maximum size.
    Surface(Size2D surface_size) : surface_size(surface_size) { ResetSectors(); }

    ~Surface() { Clear(); }

    /// Save authoritative state, including sector order and the reusable-ID stack.
    /// BODY_T extensions should serialize their base and their own payload.
    void Serialize(SerialPod & pod) {
      double width = surface_size.Width(), height = surface_size.Height();
      geometry_detail::SerializeReal(pod, width);
      geometry_detail::SerializeReal(pod, height);
      geometry_detail::SerializeReal(pod, max_radius);
      pod(wrap, body_set, open_ids);
      if (pod.IsLoad()) {
        surface_size = Size2D{width, height};
        EMP_DEBUG(++body_version; UpdateBodyVersions());
        loaded_state_valid = BodiesOK();
        if (!loaded_state_valid) return;
        ResetSectors();
      }
      for (size_t id = 0; id < grid_size.NumCells(); ++id) pod(sectors[id]);
      if (pod.IsLoad()) loaded_state_valid = LoadedStateOK();
    }

    [[nodiscard]] bool BodiesOK() const {
      if (!std::isfinite(surface_size.Width()) || !std::isfinite(surface_size.Height())
          || surface_size.Width() <= 0 || surface_size.Height() <= 0
          || !std::isfinite(max_radius) || max_radius < 0
          || 2 * max_radius >= std::min(surface_size.Width(), surface_size.Height())) return false;
      emp::vector<bool> open(body_set.size(), false);
      for (size_t id : open_ids) {
        if (id >= body_set.size() || open[id]) return false;
        open[id] = true;
      }
      for (size_t id = 0; id < body_set.size(); ++id) {
        const auto & body = GetBody(id);
        if (body.IsActive() == open[id]) return false;
        if (!body.IsActive()) continue;
        if (body.GetID() != id || !std::isfinite(body.X()) || !std::isfinite(body.Y())
            || !Contains(body.GetCenter()) || !std::isfinite(body.GetRadius())
            || body.GetRadius() <= 0 || body.GetRadius() > max_radius) return false;
      }
      return true;
    }

    [[nodiscard]] bool LoadedStateOK() const {
      if (!loaded_state_valid || !BodiesOK()) return false;
      emp::vector<bool> seen(body_set.size(), false);
      for (size_t sector = 0; sector < grid_size.NumCells(); ++sector) {
        for (size_t id : sectors[sector].GetBodyIDs()) {
          if (id >= body_set.size() || seen[id] || !IsActive(id)) return false;
          const auto pos = (GetBody(id).GetCenter() / sector_size).ToGridPos();
          if (grid_size.ToIndex(pos) != sector) return false;
          seen[id] = true;
        }
      }
      return std::count(seen.begin(), seen.end(), true) == static_cast<ptrdiff_t>(NumBodies());
    }

    void SetWrap(bool value) {
      if (wrap == value) return;
      wrap = value;
      if (max_radius > 0) ResetSectors();
    }

    /// Minimum-image displacement on a torus; ordinary displacement otherwise.
    [[nodiscard]] Point GetDisplacement(Point from, Point to) const {
      Point delta = from - to;
      if (wrap) {
        if (delta.X() > surface_size.Width() / 2) delta.X() -= surface_size.Width();
        else if (delta.X() < -surface_size.Width() / 2) delta.X() += surface_size.Width();
        if (delta.Y() > surface_size.Height() / 2) delta.Y() -= surface_size.Height();
        else if (delta.Y() < -surface_size.Height() / 2) delta.Y() += surface_size.Height();
      }
      return delta;
    }

    [[nodiscard]] const Size2D & GetSize() const { return surface_size; }
    [[nodiscard]] const GridSize & GetGridSize() const { return grid_size; }
    [[nodiscard]] bool GetWrap() const { return wrap; }
    [[nodiscard]] size_t NumBodies() const { return body_set.size() - open_ids.size(); }

    [[nodiscard]] size_t NumOpen() const { return open_ids.size(); } // DEBUG!

    [[nodiscard]] BODY_T & GetBody(size_t id) {
      DEBUG_STACK();
      EMP_DEBUG(body_set[id].SetVersion(body_version));
      return body_set[id];      
    }

    [[nodiscard]] const BODY_T & GetBody(size_t id) const {
      DEBUG_STACK();
      EMP_DEBUG(body_set[id].SetVersion(body_version));
      return body_set[id];      
    }

    [[nodiscard]] emp::vector<BODY_T> & GetBodySet() {
      EMP_DEBUG(UpdateBodyVersions());
      return body_set;
    }
    [[nodiscard]] const emp::vector<BODY_T> & GetBodySet() const {
      EMP_DEBUG(UpdateBodyVersions());
      return body_set;
    }

    /// Get the first active body you can find.
    [[nodiscard]] BODY_T & GetActiveBody() {
      DEBUG_STACK();
      emp_assert(NumBodies() > 0);  // Function requires at least one active body to exist.
      size_t out_id = 0;
      EMP_DEBUG(UpdateBodyVersions());
      while (out_id < body_set.size() && !body_set[out_id].IsActive()) ++out_id;
      EMP_DEBUG(body_set[out_id].SetVersion(body_version));
      return body_set[out_id];
    }

    /// Does an id represent an active body on a surface?
    [[nodiscard]] bool IsActive(size_t id) const {
      DEBUG_STACK();
      return GetBody(id).IsActive();
    }

    [[nodiscard]] const Circle & GetPerimeter(size_t id) const {
      DEBUG_STACK();
      return GetBody(id).GetPerimeter();
    }

    [[nodiscard]] const Color & GetColor(size_t id) const {
      DEBUG_STACK();
      return GetBody(id).GetColor();
    }

    /// Run a provided function on each active body.
    template <typename FUN_T>
    void ForEachBody(FUN_T && fun) {
      DEBUG_STACK();
      // Use an index-based for loop in case body_set memory changes during loop.
      for (size_t body_id = 0; body_id < body_set.size(); ++body_id) {
        auto & body = GetBody(body_id);
        if (body.IsActive()) fun(body);
      }
    }

    [[nodiscard]] bool Contains(const Point2D & point) const {
      DEBUG_STACK();
      return surface_size.Contains(point);
    }

    void MoveTo(size_t id, Point pos) {
      DEBUG_STACK();
      if (wrap) pos = pos.Wrap(surface_size);
      emp_assert(Contains(pos));

      // If data is active, update sectors.
      Sector & old_sector = FindSector(GetBody(id));
      Sector & new_sector = FindSector(pos);
      if (old_sector.GetID() != new_sector.GetID()) {
        old_sector.Remove(id);
        new_sector.Insert(id);
      }

      GetBody(id).MoveTo(pos);
    }

    void MoveBy(size_t id, Point translation) {
      DEBUG_STACK();
      MoveTo(id, GetBody(id).GetCenter() + translation);
    }

    void FinalizePosition(BODY_T & body) {
      DEBUG_STACK();
      size_t start_sector = FindSectorID(body.GetCenter());
      body.FinalizePosition();
      // Wrap only after pressure is measured, so crossing a seam adds no fictitious pressure.
      if (wrap) body.MoveTo(body.GetCenter().Wrap(surface_size));
      size_t end_sector = FindSectorID(body.GetCenter());
      if (start_sector != end_sector) {
        sectors[start_sector].Remove(body.GetID());
        sectors[end_sector].Insert(body.GetID());
      }
    }

    void SetRadius(size_t id, double radius) {
      DEBUG_STACK();
      emp_assert(GetBody(id).IsActive());
      GetBody(id).SetRadius(radius);
      TestBodySize(radius);
    }

    void SetColor(size_t id, Color in) {
      DEBUG_STACK();
      emp_assert(GetBody(id).IsActive());
      GetBody(id).SetColor(in);
    }

    void RemoveBody(size_t id) {
      DEBUG_STACK();
      BODY_T & body = GetBody(id);
      emp_assert(body.IsActive());    // Ensure that this body is available to be removed.
      emp_assert(Contains(body.GetCenter()));
      FindSector(body).Remove(id);  // ...remove this body from its sector.
      body.Deactivate();              // Deactivate this body (so we know not to use it)
      open_ids.push_back(id);         // Mark this position as open for a new body.
    }

    /// Add a single body to surface; return its unique ID.
    size_t AddBody(Circle circle, emp::Color color) {
      DEBUG_STACK();
      emp_assert(Contains(circle.GetCenter()), surface_size, circle.GetCenter());
      TestBodySize(circle.GetRadius()); // Update sector sizes, if needed.

      auto & body = ReserveBody(circle, color);

      PlaceBody(body);              // Add new body to a sector.
      emp_assert(body.IsActive());  // If everything went well, body is now active.
      return body.GetID();
    }

    /// Remove all bodies from the surface.
    Surface & Clear() {
      DEBUG_STACK();
      EMP_DEBUG(++body_version);
      body_set.resize(0);
      open_ids.resize(0);
      max_radius = 0.0;
      loaded_state_valid = true;
      grid_size = GridSize{};
      ResetSectors();
      return *this;
    }

    /// Determine if two bodies overlap.
    bool TestOverlap(const BODY_T & body1, const BODY_T & body2) const {
      DEBUG_STACK();
      emp_assert(body1.IsActive() && body2.IsActive());
      const Point xy_dist       = GetDisplacement(body1.GetCenter(), body2.GetCenter());
      const double sqr_dist     = xy_dist.SquareMagnitude();
      const double total_radius = body1.GetRadius() + body2.GetRadius();
      const double sqr_radius   = total_radius * total_radius;
      return (sqr_dist < sqr_radius);
    }

    bool TestOverlap(size_t id1, size_t id2) {
      DEBUG_STACK();
      return TestOverlap(GetBody(id1), GetBody(id2));
    }

    /// Update the list of overlaps in an internal sector.
    // NOTE: Will always add on to out_ids, so manually clear it first if needed.
    const emp::vector<size_t> & FindOverlaps(const BODY_T & body, const Sector & sector,
                                             size_t start_pos = 0) {
      DEBUG_STACK();
      emp_assert(body.IsActive());
      const auto & body_ids = sector.GetBodyIDs();
      for (size_t pos = start_pos; pos < body_ids.size(); ++pos) {
        size_t test_id = body_ids[pos];
        emp_assert(GetBody(test_id).IsActive());
        if (body.GetID() == test_id) { continue; }  // Don't match with self!
        if (TestOverlap(body, GetBody(test_id))) { out_ids.push_back(test_id); }
      }
      return out_ids;
    }

    /// Find pairs of bodies that overlap, and run passed-in function on those IDs.
    template <typename OVERLAP_FUN_T>
    void TriggerOverlaps(OVERLAP_FUN_T && overlap_fun) {
      DEBUG_STACK();

      // Loop through all of the sectors to identify collisions.
      const size_t num_sectors = grid_size.NumCells();
      for (size_t sector_id = 0; sector_id < num_sectors; ++sector_id) {
        const Sector & sector = sectors[sector_id];
        const emp::vector<size_t> & body_ids = sector.GetBodyIDs();

        // Loop through all bodies in this sector.
        // Note, we only test four of the eight directions since the other four will be tested
        // by neighbors.
        for (size_t pos=0; pos < body_ids.size(); ++pos) {
          const BODY_T & body = GetBody(body_ids[pos]);

          out_ids.clear();  // Clear the set of IDs that could overlap with this body.
          FindOverlaps(body, sector, pos+1);
          if (grid_size.NumCols() > 2 && grid_size.NumRows() > 2) {
            if (sector.HasLeft()) FindOverlaps(body, sectors[sector.GetLeft()]);
            if (sector.HasUL()) FindOverlaps(body, sectors[sector.GetUL()]);
            if (sector.HasUp()) FindOverlaps(body, sectors[sector.GetUp()]);
            if (sector.HasUR()) FindOverlaps(body, sectors[sector.GetUR()]);
          } else {
            // Small wrapped grids repeat neighbors. Visit each sector pair exactly once.
            emp::array<size_t, 8> visited{};
            size_t count = 0;
            for (GridDir dir : Grid::DIR_SET) {
              const size_t other = grid_size.ToIndex(grid_size.PosDir(
                grid_size.FromIndex(sector_id), dir, wrap));
              if (other == NO_ID || other <= sector_id
                  || std::find(visited.begin(), visited.begin() + count, other)
                     != visited.begin() + count) continue;
              visited[count++] = other;
              FindOverlaps(body, sectors[other]);
            }
          }
          // ...other four directions will be handled when those sectors are processed.

          // Trigger overlap function for each one found.
          for (size_t overlap_id : out_ids) {
            overlap_fun(body.GetID(), overlap_id);
          }
        }
      }
    }

    void WrapBodies() {
      DEBUG_STACK();
      ForEachBody([this](BODY_T & body){
        body.MoveTo(body.GetCenter().Wrap(surface_size));
      });
      if (max_radius > 0) ResetSectors();
    }

    template <typename LOW_X_T, typename HIGH_X_T, typename LOW_Y_T, typename HIGH_Y_T>
    void TriggerOffsides(LOW_X_T low_x_fun, HIGH_X_T high_x_fun, LOW_Y_T low_y_fun, HIGH_Y_T high_y_fun) {
      DEBUG_STACK();
      emp_assert(wrap == false);
      // Check for high or low x.
      for (size_t row_id = 0; row_id < grid_size.NumRows(); ++row_id) {
        const auto & left_sector = sectors[row_id * grid_size.NumCols()];
        for (size_t id : left_sector.GetBodyIDs()) {
          if (GetBody(id).X() < 0.0) low_x_fun(id);
        }

        const auto & right_sector = sectors[(row_id+1) * grid_size.NumCols() - 1];
        for (size_t id : right_sector.GetBodyIDs()) {
          if (GetBody(id).X() > surface_size.Width()) high_x_fun(id);
        }
      }

      for (size_t col_id = 0; col_id < grid_size.NumCols(); ++col_id) {
        const auto & top_sector = sectors[col_id];
        for (size_t id : top_sector.GetBodyIDs()) {
          if (GetBody(id).Y() < 0.0) low_y_fun(id);
        }

        const auto & bottom_sector = sectors[(grid_size.NumRows()-1) * grid_size.NumCols() + col_id];
        for (size_t id : bottom_sector.GetBodyIDs()) {
          if (GetBody(id).Y() > surface_size.Height()) high_y_fun(id);
        }
      }
    }

    /// Determine if there are any overlaps with a single provided body (that may or may not be on surface).
    const emp::vector<size_t> & FindOverlaps(BODY_T & body) {
      DEBUG_STACK();

      Sector & sector = FindSector(body);
      out_ids.clear();  // Prepare the return variable.

      // Compare against bodies in its own sector.
      out_ids.clear();  // Clear the set of IDs that could overlap with this body.
      FindOverlaps(body, sector);

      // On a narrow torus, several directions can refer to the same sector.
      emp::array<size_t, 8> visited{};
      size_t count = 0;
      for (GridDir dir : Grid::DIR_SET) {
        const size_t other = grid_size.ToIndex(grid_size.PosDir(
          grid_size.FromIndex(sector.GetID()), dir, wrap));
        if (other == NO_ID || other == sector.GetID()
            || std::find(visited.begin(), visited.begin() + count, other)
               != visited.begin() + count) continue;
        visited[count++] = other;
        FindOverlaps(body, sectors[other]);
      }

      return out_ids;
    }

    const emp::vector<size_t> & FindOverlaps(size_t body_id) {
      DEBUG_STACK();
      emp_assert(GetBody(body_id).IsActive());
      return FindOverlaps(GetBody(body_id));
    }

    // Find overlaps using a distance from a point.
    void FindOverlaps(Circle circle) {
      DEBUG_STACK();
      BODY_T tmp_body(NO_ID, circle);
      FindOverlaps(tmp_body);
    }
  };

  using BasicSurface = Surface<SurfaceBody>;

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_GEOMETRY_SURFACE_HPP_GUARD
