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
  class Physics2D : public emp::Surface<BODY_T>{
  private:
    using surface_t = emp::Surface<BODY_T>;
    using surface_t::body_set;
    using surface_t::GetSize;

    const double max_diameter = 20.0;  // Size limit for bodies.
    const double max_pressure = 3.0;   // Pressure limit for bodies.
    double time = 0;                   // Current time point
    bool detach_on_birth = true;       // Do bodies detach from parents at birth?

  public:
    static constexpr size_t NO_ID = surface_t::NO_ID;
    using body_type = BODY_T;

    Physics2D(double width, double height, double max_diameter = 20, bool detach = true)
      : surface_t({width, height})
      , max_diameter(max_diameter)
      , detach_on_birth(detach) {}

    ~Physics2D() = default;

    // Main accessors
    [[nodiscard]] double GetMaxDiameter() const { return max_diameter; }
    [[nodiscard]] bool GetDetach() const { return detach_on_birth; }

    void SetDetach(bool _in) { detach_on_birth = _in; }

    [[nodiscard]] double GetStartTime(size_t id) const { return body_set[id].start_time; }
    [[nodiscard]] const Point & GetVelocity(size_t id) const { return body_set[id].velocity; }
    [[nodiscard]] Angle GetOrientation(size_t id) const { return body_set[id].orientation; }
    [[nodiscard]] double GetMass(size_t id) const { return body_set[id].mass; }
    [[nodiscard]] double GetTargetRadius(size_t id) const { return body_set[id].target_radius; }

    auto & AddBody(Circle in_body, Color color = Palette::RED, size_t link_id = NO_ID) {
      auto & new_body = surface_t::AddBody(in_body, color);
      new_body.SetStartTime(time);
      if (link_id != NO_ID) AddLink(link_id, new_body.GetID());
      return new_body;
    }

    // Search through all active bodies to find the oldest.
    [[nodiscard]] size_t FindOldest() const {
      size_t oldest_id = MAX_SIZE_T;
      size_t oldest_birth_time = MAX_SIZE_T;

      // Search through all bodies for the oldest.
      for (const auto & body : body_set) {
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
      surface_t::RemoveBody(body.GetID());
    }

    void RemoveBody(size_t id) { RemoveBody(body_set[id]); }

    void RemoveOldest() {
      size_t oldest_id = FindOldest();
      if (oldest_id != MAX_SIZE_T) { RemoveBody(oldest_id); }
    }

    // Test if org with id1 is linked to org with id2.
    bool TestLinked(size_t id1, size_t id2) {
      return body_set[id1].HasLink(id2);
    }

    void AddLink(size_t id1, size_t id2) {
      emp_assert(!TestLinked(id1, id2), "Should not link same bodies twice.", id1, id2);
      body_set[id1].AddLink(id2);
      body_set[id2].AddLink(id1);
    }

    void RemoveLink(size_t id1, size_t id2) {
      emp_assert(TestLinked(id1, id2), "Must make sure link exists before removing it.", id1, id2);
      emp_assert(TestLinked(id2, id1), "Must make sure link exists before removing it.", id1, id2);
      body_set[id1].RemoveLink(id2);
      body_set[id2].RemoveLink(id1);
    }

    bool TestPairCollision(size_t id1, size_t id2) {
      // If bodies are not overlapping OR are linked, they do no collide.
      if (!surface_t::TestOverlap(id1, id2) || TestLinked(id1, id2)) { return false; }

      auto & body1 = body_set[id1];
      auto & body2 = body_set[id2];

      // Don't allow bodies to be directly on top of each other.
      if (body1.GetCenter() == body2.GetCenter()) { body1.MoveBy({0.0, 0.01}); }

      const double collide_dist = body1.GetRadius() + body2.GetRadius();
      const Point cur_offset = body1.GetCenter() - body2.GetCenter();
      const double cur_dist = cur_offset.Magnitude();
      const double shift_fract = (collide_dist / cur_dist - 1.0) / 2.0;
      const Point shift_offset = cur_offset * shift_fract;

      // Re-adjust position to remove overlap.
      body1.MoveBy(shift_offset);
      body2.MoveBy(-shift_offset);

      // @CAO if we have inelastic collisions, we just take the weighted average of velocities
      // and let them move together.

      // Assume elastic: Re-adjust velocity to reflect bounce.
      Point v1 = body_set[id1].GetVelocity();
      Point v2 = body_set[id2].GetVelocity();

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
      // Handle initial movement and size change of bodies
      UpdateBodies(0.25, 0.0125);

      // Deal with collisions between bodies.
      surface_t::TriggerOverlaps([this](size_t b1, size_t b2) {
        return this->TestPairCollision(b1, b2);
      });

      // Deal with edges.
      if (surface_t::wrap) { WrapBodies(); }
      else { // Bound off edges, if needed.
        surface_t::TriggerOffsides(
          [this](size_t id){  // X < 0
            auto & body = body_set[id];
            emp_assert(body.IsActive());
            Point2D & center = body.GetCenter();
            center.SetX(-center.X()); // Reflect back into range.
            if (body.GetVelocity().X() < 0) body.NegateVelocityX();
          },
          [this](size_t id){  // X > MAX_X
            auto & body = body_set[id];
            Point2D & center = body.GetCenter();
            double overshoot = center.X() - GetSize().X();
            center.SetX(GetSize().X() - overshoot); // Reflect back into range.
            if (body.GetVelocity().X() > 0) body.NegateVelocityX();
          },
          [this](size_t id){  // Y < 0
            auto & body = body_set[id];
            Point2D & center = body.GetCenter();
            center.SetY(-center.Y()); // Reflect back into range.
            if (body.GetVelocity().Y() < 0) body.NegateVelocityY();
          },
          [this](size_t id){  // Y > MAX_Y
            auto & body = body_set[id];
            Point2D & center = body.GetCenter();
            double overshoot = center.Y() - GetSize().Y();
            center.SetY(GetSize().Y() - overshoot); // Reflect back into range.
            if (body.GetVelocity().Y() > 0) body.NegateVelocityY();
          }
        );
      }

      // Finalize body positions and remove those with too high pressure.
      for (auto & body : body_set) {
        if (!body.IsActive()) continue;

        // If pressure too high, burst this cell!
        if (body.GetPressure() > max_pressure) { RemoveBody(body); }
      }
    }

    // Change size and position of bodies, initially ignoring other bodies on surface.
    void UpdateBodies(double max_size_change = 1.0, double friction = 0.0) {
      for (auto & body : body_set) {
        body.UpdateSize(max_size_change);
        body.UpdatePosition(friction);
      }
    }
  };

  /// Default Physics is 2D with no extra details for the body.
  using Physics = Physics2D<PhysicsBody>;

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_GEOMETRY_PHYSICS2D_HPP_GUARD

// Local settings for Empecable file checker.
// empecable_words: rel
