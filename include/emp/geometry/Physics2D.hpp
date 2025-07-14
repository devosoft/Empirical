/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2021-2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/geometry/Physics2D.hpp
 * @brief Physics2D - handles movement and collisions in a simple 2D world.
 *
 */

#pragma once

#ifndef INCLUDE_EMP_GEOMETRY_PHYSICS2D_HPP_GUARD
#define INCLUDE_EMP_GEOMETRY_PHYSICS2D_HPP_GUARD

#include <functional>
#include <stddef.h>
#include <unordered_set>

#include "../base/vector.hpp"

#include "Angle.hpp"
#include "PhysicsBody.hpp"
#include "Surface.hpp"

namespace emp {

  template <typename BODY_T>
    requires std::is_base_of_v<PhysicsBody, BODY_T>
  class Physics2D {
  private:
    using surface_t = emp::Surface<BODY_T>;
    surface_t surface;                 // Track current position of bodies.
    const double max_diameter = 20.0;  // Size limit for bodies.
    double time = 0;                   // Current time point
    bool detach_on_birth = true;       // Do bodies detach from parents at birth?

  public:
    static constexpr size_t NO_ID = surface_t::NO_ID;
    using body_type = BODY_T;

    Physics2D(double width, double height, double max_diameter = 20, bool detach = true)
      : surface({width, height})
      , max_diameter(max_diameter)
      , detach_on_birth(detach) {}

    ~Physics2D() = default;

    // Main accessors
    [[nodiscard]] const auto & GetSurface() const { return surface; }
    [[nodiscard]] double GetMaxDiameter() const { return max_diameter; }
    [[nodiscard]] bool GetDetach() const { return detach_on_birth; }

    void SetDetach(bool _in) { detach_on_birth = _in; }

    // Body getters
    [[nodiscard]] auto & GetBody(size_t id) { return surface.GetBody(id); }
    [[nodiscard]] const auto & GetBody(size_t id) const { return surface.GetBody(id); }
    [[nodiscard]] emp::vector<body_type> & GetBodySet() { return surface.GetBodySet(); }
    [[nodiscard]] const emp::vector<body_type> & GetConstBodySet() const {
      return surface.GetConstBodySet();
    }

    [[nodiscard]] double GetStartTime(size_t id) const { return GetBody(id).start_time; }
    [[nodiscard]] const Point & GetVelocity(size_t id) const { return GetBody(id).velocity; }
    [[nodiscard]] Angle GetOrientation(size_t id) const { return GetBody(id).orientation; }
    [[nodiscard]] double GetMass(size_t id) const { return GetBody(id).mass; }
    [[nodiscard]] double GetTargetRadius(size_t id) const { return GetBody(id).target_radius; }

    auto & AddBody(Circle in_body, Color color = Palette::RED, size_t link_id = NO_ID) {
      auto & new_body = surface.AddBody(in_body, color);
      new_body.SetStartTime(time);
      if (link_id != NO_ID) AddLink(link_id, new_body.GetID());
      return new_body;
    }

    void Clear() { surface.Clear(); }

    // Search through all active bodies to find the oldest.
    [[nodiscard]] size_t FindOldest() const {
      size_t oldest_id = MAX_SIZE_T;
      size_t oldest_birth_time = MAX_SIZE_T;

      // Search through all bodies for the oldest.
      for (const auto & body : surface.GetBodySet()) {
        if (body.IsActive() && body.GetStartTime() < oldest_birth_time) {
          oldest_id = body.GetID();
          oldest_birth_time = body.GetStartTime();
        }
      }

      return oldest_id;
    }

    void RemoveBody(body_type & body) {
      for (size_t link_id : body.GetLinkIDs()) {
        RemoveLink(body.GetID(), link_id);
      }
      surface.RemoveBody(body.GetID());
    }

    void RemoveBody(size_t id) { RemoveBody(GetBody(id)); }

    void RemoveOldest() {
      size_t oldest_id = FindOldest();
      if (oldest_id != MAX_SIZE_T) { RemoveBody(oldest_id); }
    }

    // Test if org with id1 is linked to org with id2.
    bool TestLinked(size_t id1, size_t id2) {
      return GetBody(id1).HasLink(id2);
    }

    void AddLink(size_t id1, size_t id2) {
      emp_assert(!TestLinked(id1, id2), "Should not link same bodies twice.", id1, id2);
      GetBody(id1).AddLink(id2);
      GetBody(id2).AddLink(id1);
    }

    void RemoveLink(size_t id1, size_t id2) {
      emp_assert(TestLinked(id1, id2), "Must make sure link exists before removing it.", id1, id2);
      emp_assert(TestLinked(id2, id1), "Must make sure link exists before removing it.", id1, id2);
      GetBody(id1).RemoveLink(id2);
      GetBody(id2).RemoveLink(id1);
    }

    bool TestPairCollision(size_t id1, size_t id2) {
      // If bodies are not overlapping OR are linked, they do no collide.
      if (!surface.TestOverlap(id1, id2) || TestLinked(id1, id2)) { return false; }

      auto & body1 = GetBody(id1);
      auto & body2 = GetBody(id2);

      // Don't allow bodies to be directly on top of each other.
      if (body1.GetCenter() == body2.GetCenter()) { body1.MoveBy({0.0, 0.01}); }

      const double contact_dist = body1.GetRadius() + body2.GetRadius();
      const Point cur_offset = body1.GetCenter() - body2.GetCenter();
      const double cur_dist = cur_offset.Magnitude();
      const double shift_fract = (contact_dist / cur_dist - 1.0) / 2.0;
      const Point shift_offset = cur_offset * shift_fract;

      // Re-adjust position to remove overlap.
      body1.MoveBy(shift_offset);
      body2.MoveBy(-shift_offset);

      // @CAO if we have inelastic collisions, we just take the weighted average of velocities
      // and let them move together.

      // Assume elastic: Re-adjust velocity to reflect bounce.
      Point v1 = GetBody(id1).GetVelocity();
      Point v2 = GetBody(id2).GetVelocity();

      if (cur_offset.GetX() == 0) { // Same X; bounce vertically.
        std::swap(v1.Y(), v2.Y());
      } else if (cur_offset.GetY() == 0) {  // Same Y; bounce horizontally.
        std::swap(v1.X(), v2.X());
      } else {
        Point normal = cur_offset / cur_dist;    // Normalized direction
        Point tangent{-normal.Y(), normal.X()};  // Perpendicular to normal

        const double v1n = v1.Dot(normal);
        const double v2n = v2.Dot(normal);

        const double v1t = v1.Dot(tangent);
        const double v2t = v2.Dot(tangent);

        v1 = tangent * v1t + normal * v2n;
        v2 = tangent * v2t + normal * v1n;
        
        // const Point rel_velocity(v2 - v1);
        // double normal_a = cur_offset.Y() / cur_offset.X();
        // double x1 = (rel_velocity.X() + normal_a * rel_velocity.Y()) / (normal_a * normal_a + 1);
        // double y1 = normal_a * x1;
        // double x2 = rel_velocity.X() - x1;
        // double y2 = -(1 / normal_a) * v2.X();

        // v2 = v1 + Point(x2, y2);
        // v1 = v1 + Point(x1, y1);
      }

      body1.SetVelocity(v1);
      body2.SetVelocity(v2);

      return true;
    }

    void Update() {
      // Handle movement of bodies

      auto & body_set = surface.GetBodySet();

      for (auto & cur_body : body_set) {
        UpdateBody(cur_body, 0.25, 0.0125); // Update body size and velocity
      }

      // Handle collisions
      surface.SetOverlapFun([this](size_t b1, size_t b2) {
        return this->TestPairCollision(b1, b2);
      });
      surface.TriggerOverlaps();

      // Determine which bodies we should remove due to high pressure.
      for (auto & body : body_set) {
        // If pressure too high, burst this cell!
        if (body.GetPressure() > 3.0) { RemoveBody(body); }  // @CAO Arbitrary pressure threshold!
      }

    }

    // Physics-related functionality.
    // @CAO: Make these easy to override.
    void UpdateBody(body_type & body, double max_size_change = 1.0, double friction = 0.0) {
      body.UpdateSize(max_size_change);
      body.UpdatePosition(friction);
    }
  };

  /// Default Physics is 2D with no extra details for the body.
  using Physics = Physics2D<PhysicsBody>;

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_GEOMETRY_PHYSICS2D_HPP_GUARD

// Local settings for Empecable file checker.
// empecable_words: rel
