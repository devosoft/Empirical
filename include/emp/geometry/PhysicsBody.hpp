/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/geometry/Physics2D.hpp
 * @brief Physics2D - handles movement and collisions in a simple 2D world.
 *
 */

#pragma once

#ifndef INCLUDE_EMP_GEOMETRY_PHYSICS_BODY_HPP_GUARD
#define INCLUDE_EMP_GEOMETRY_PHYSICS_BODY_HPP_GUARD

#include <ranges>

#include "Angle.hpp"
#include "Point2D.hpp"
#include "Surface.hpp"
#include "SurfaceBody.hpp"

namespace emp {

  // Extra information to track to manage physics of each body.
  class PhysicsBody : public SurfaceBody {
  private:
    double start_time = 0;  // When was body added?
    Point velocity;         // How fast is body moving?
    Angle orientation;      // Which way is body facing?
    double mass = 1.0;      // "Weight" of this body (@CAO not used yet..)
    double target_radius;   // For growing/shrinking

    // Update calculations
    Point shift;            // How should this body be updated to minimize overlap.
    Point total_shift;      // Build up of shift not yet acted upon.
    Point total_abs_shift;  // Total absolute-value of shifts (to calculate pressure)
    double pressure = 0.0;  // Current pressure on this body.

    emp::vector<size_t> link_ids;  // Ids of bodies that are attached to this one

  public:
    PhysicsBody(Circle circle = Circle{1.0}, Color color = Palette::BLACK)
      : SurfaceBody(circle, color), target_radius(circle.GetRadius()) {}

    // Accessors
    [[nodiscard]] double GetStartTime() const { return start_time; }
    [[nodiscard]] Point GetVelocity() const { return velocity; }
    [[nodiscard]] Angle GetOrientation() const { return orientation; }
    [[nodiscard]] double GetMass() const { return mass; }
    [[nodiscard]] double GetTargetRadius() const { return target_radius; }
    [[nodiscard]] Point GetShift() const { return shift; }
    [[nodiscard]] Point GetTotalShift() const { return total_shift; }
    [[nodiscard]] Point GetTotalAbsShift() const { return total_abs_shift; }
    [[nodiscard]] double GetPressure() const { return pressure; }
    [[nodiscard]] const emp::vector<size_t> & GetLinkIDs() const { return link_ids; }

    void SetStartTime(double in) { start_time = in; }
    void SetVelocity(Point in) { velocity = in; }
    void SetOrientation(Angle in) { orientation = in; }
    void SetMass(double in) { mass = in; }
    void SetTargetRadius(double in) { target_radius = in; }
    void SetShift(Point in) { shift = in; }
    void SetTotalShift(Point in) { total_shift = in; }
    void SetTotalAbsShift(Point in) { total_abs_shift = in; }
    void SetPressure(double in) { pressure = in; }
    void SetLinkIDs(const emp::vector<size_t> & in) { link_ids = in; }

    // Other orientation and position controls...
    void RotateDegrees(double degrees) { orientation.RotateDegrees(degrees); }

    // Other velocity controls...
    void NegateVelocityX() { velocity.SetX(-velocity.X()); }
    void NegateVelocityY() { velocity.SetY(-velocity.Y()); }
    void IncSpeed(const Point & offset) { velocity += offset; }
    void IncSpeed(double val=1.0) { velocity += orientation.GetPoint() * val; }
    void DecSpeed(double val=1.0) { velocity -= orientation.GetPoint() * val; }

    // Manage Links
    [[nodiscard]] bool HasLink(size_t id) const { return std::ranges::find(link_ids, id) != link_ids.end(); }
    [[nodiscard]] size_t GetLinkCount() const { return link_ids.size(); }
    void AddLink(size_t id) {
      emp_assert(!HasLink(id), "Do not add a link to a body multiple times.", id);
      link_ids.push_back(id);
    }
    void RemoveLink(size_t id) {
      emp_assert(HasLink(id), "Do not try to remove a link not currently used.", id);
      link_ids.erase(std::find(link_ids.begin(), link_ids.end(), id));
    }

    void UpdateSize(double max_size_change) {
      double change_needed = target_radius - GetRadius();
      if (change_needed != 0.0) {
        change_needed = std::clamp(change_needed, -max_size_change, max_size_change);
        SetRadius(GetRadius() + change_needed);
      }
    }

    void UpdatePosition(double friction) {
      if (velocity.AtOrigin()) return; // No velocity; nothing to update.
      MoveBy(velocity);
      const double speed = velocity.Magnitude();

      // If body is close to stopping stop it!
      if (friction > speed) { SetVelocity({0.0, 0.0}); }

      // Otherwise slow it down proportionately in the x and y directions.
      else {
        const double friction_slowdown = 1.0 - friction / speed;
        SetVelocity(GetVelocity() * friction_slowdown);
      }
    }

  }; // End of Physics2D::PhysicsBody class

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_GEOMETRY_PHYSICS_BODY_HPP_GUARD
