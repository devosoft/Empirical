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
  protected:
    using surface_t = emp::Surface<BODY_T>;
    using surface_t::body_set;
    using surface_t::surface_size;

    const double pressure_limit = 3.0;  // Pressure limit for bodies.
    double time = 0;                    // Current time point
    bool detach_on_birth = true;        // Do bodies detach from parents at birth?

  public:
    using body_type = BODY_T;

    using surface_t::Contains;
    using surface_t::ForEachBody;
    using surface_t::GetSize;

    Physics2D(Size2D size, bool detach=true) : surface_t(size), detach_on_birth(detach) {}

    ~Physics2D() = default;

    // Main accessors
    [[nodiscard]] double GetPressureLimit() const { return pressure_limit; }
    [[nodiscard]] bool GetDetach() const { return detach_on_birth; }

    void SetDetach(bool _in) { detach_on_birth = _in; }

    [[nodiscard]] double GetStartTime(size_t id) const { return body_set[id].start_time; }
    [[nodiscard]] const Point & GetVelocity(size_t id) const { return body_set[id].velocity; }
    [[nodiscard]] Angle GetOrientation(size_t id) const { return body_set[id].orientation; }
    [[nodiscard]] double GetMass(size_t id) const { return body_set[id].mass; }
    [[nodiscard]] double GetTargetRadius(size_t id) const { return body_set[id].target_radius; }

    size_t AddBody(Circle in_body, Color color = Palette::RED) {
      size_t id = surface_t::AddBody(in_body, color);
      surface_t::GetBody(id).SetStartTime(time);
      return id;
    }

    /// Add a body with a specified link to an existing body.
    size_t AddBody(Circle in_body, Color color, size_t link_id) {
      size_t id = AddBody(in_body, color);
      AddLink(link_id, id);
      return id;
    }

    void RemoveBody(body_type & body) {
      emp_assert(body.IsActive());                   // Can only remove active bodies.
      for (size_t link_id : body.GetLinkIDs()) {     // Step through all attached bodies...
        body_set[link_id].RemoveLink(body.GetID());  // ...and remove link to this one.
      }
      surface_t::RemoveBody(body.GetID());
    }

    void RemoveBody(size_t id) {
      RemoveBody(body_set[id]);
    }

    // Search through all active bodies to find the oldest.
    [[nodiscard]] size_t FindOldest() const {
      // Find index of lowest start time.
      return emp::FindIndex(body_set, [](const BODY_T & a, const BODY_T & b) {
        return a.GetStartTime() < b.GetStartTime();
      });
    }

    void RemoveOldest() {
      if (surface_t::NumBodies() > 0) { RemoveBody(FindOldest()); }
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
      emp_assert(TestLinked(id2, id1), "Link to be removed is not bi-directional!", id1, id2);
      body_set[id1].RemoveLink(id2);
      body_set[id2].RemoveLink(id1);
    }

    bool ProcessPairCollision(BODY_T & body1, BODY_T & body2) {
      emp_assert(body1.IsActive() && body2.IsActive());

      // If bodies are not overlapping OR are linked, they do not collide.
      if (!surface_t::TestOverlap(body1, body2) || body1.HasLink(body2.GetID())) { return false; }

      // Don't allow bodies to be perfectly on top of each other (will ALMOST never happen)
      if (body1.GetCenter() == body2.GetCenter()) { body1.MoveBy({0.0, 0.01}); }

      // @CAO Some of these recalculate from TestOverlap() above...
      const double collide_dist = body1.GetRadius() + body2.GetRadius();
      const Point cur_offset    = body1.GetCenter() - body2.GetCenter();
      const double cur_dist     = cur_offset.Magnitude();
      const double shift_fract  = (collide_dist / cur_dist - 1.0) / 2.0;
      const Point shift_offset  = cur_offset * shift_fract;

      // Re-adjust position to remove overlap.
      body1.ProcessShift(shift_offset);
      body2.ProcessShift(-shift_offset);

      // @CAO if we had inelastic collisions, we would take the weighted average of velocities
      // and let them move together.

      // Assume elastic: Re-adjust velocity to reflect bounce.
      Point & v1 = body1.GetVelocity();
      Point & v2 = body2.GetVelocity();

      if (cur_offset.GetX() == 0) { // Same X; bounce vertically.
        std::swap(v1.Y(), v2.Y());
      } else if (cur_offset.GetY() == 0) {  // Same Y; bounce horizontally.
        std::swap(v1.X(), v2.X());
      } else {
        const Point normal = cur_offset / cur_dist;    // Normalized direction
        const Point tangent{-normal.Y(), normal.X()};  // Perpendicular to normal

        v1 = tangent * v1.Dot(tangent) + normal * v2.Dot(normal);
        v2 = tangent * v2.Dot(tangent) + normal * v1.Dot(normal);
      }

      return true;
    }

    void Update_Bodies() {
      ForEachBody([this, max_size_change=0.25, friction=0.0125](BODY_T & body){
        (void) this;
        emp_assert(Contains(body.GetCenter()));  // Bodies must be at valid positions.
        body.UpdateSize(max_size_change);
        body.ProcessVelocity(friction);
        emp_assert(Contains(body.GetCenter()));  // Bodies must be at valid positions.
      });
    }

    void Update_BodyCollisions() {
      surface_t::TriggerOverlaps([this](size_t b1, size_t b2) {
        return this->ProcessPairCollision(body_set[b1], body_set[b2]);
      });
    }

    void Update_EdgeCollisions() {
      if (surface_t::wrap) { surface_t::WrapBodies(); }
      else { // Otherwise reflect bodies, as needed.
        ForEachBody([this](BODY_T & body){
          emp_assert(Contains(body.GetCenter()));  // Bodies must start at valid positions.

          // If a shift will put a body in an invalid position, fix it!

          const Point2D shift_pos = body.CalcShiftPos();
          const double radius = body.GetRadius();
          Point2D reflect_adjust;

          if (shift_pos.X() < radius) {
            const double overshoot = radius - shift_pos.X();
            reflect_adjust.SetX(2*overshoot);
            if (body.GetVelocity().X() < 0.0) body.NegateVelocityX();
          }
          else if (shift_pos.X() + radius >= surface_size.Width()) {
            const double overshoot = shift_pos.X() + radius - surface_size.Width();
            reflect_adjust.SetX(-2 * overshoot - 0.000001);
            if (body.GetVelocity().X() > 0.0) body.NegateVelocityX();
          }
          if (shift_pos.Y() < radius) {
            const double overshoot = radius - shift_pos.Y();
            reflect_adjust.SetY(2*overshoot);
            if (body.GetVelocity().Y() < 0.0) body.NegateVelocityY();
          }
          else if (shift_pos.Y() + radius >= surface_size.Height()) {
            const double overshoot = shift_pos.Y() + radius - surface_size.Height();
            reflect_adjust.SetY(-2 * overshoot - 0.000001);
            if (body.GetVelocity().Y() > 0.0) body.NegateVelocityY();
          }
          if (!reflect_adjust.AtOrigin()) { body.ProcessShift(reflect_adjust); }

          emp_assert(Contains(body.CalcShiftPos()), body.CalcShiftPos()); // Shift pos should now be valid.
        });
      }
    }

    void Update_FinalizePositions() {
      ForEachBody([this](BODY_T & body){
        // If pressure is too high, burst this cell!
        if (body.CalcPressure() > pressure_limit) { RemoveBody(body); }

        // Otherwise, finalize the position.
        else { surface_t::FinalizePosition(body); }
      });
    }

    void Update() {
      Update_Bodies();
      Update_BodyCollisions();
      Update_EdgeCollisions();
      Update_FinalizePositions();
    }
  };

  /// Default Physics is 2D with no extra details for the body.
  using Physics = Physics2D<PhysicsBody>;

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_GEOMETRY_PHYSICS2D_HPP_GUARD

// Local settings for Empecable file checker.
// empecable_words: rel
