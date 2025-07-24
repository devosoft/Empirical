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
    Point total_shift;      // Build up of shift not yet acted upon.
    Point total_abs_shift;  // Total absolute-value of shifts (to calculate pressure)

    emp::vector<size_t> link_ids;  // Ids of bodies that are attached to this one

  public:
    PhysicsBody(Circle circle = Circle{1.0}, Color color = Palette::BLACK)
      : SurfaceBody(circle, color), target_radius(circle.GetRadius()) {}

    // Accessors
    [[nodiscard]] double GetStartTime() const { return start_time; }
    [[nodiscard]] const Point & GetVelocity() const { return velocity; }
    [[nodiscard]] Angle GetOrientation() const { return orientation; }
    [[nodiscard]] double GetMass() const { return mass; }
    [[nodiscard]] double GetTargetRadius() const { return target_radius; }
    [[nodiscard]] const emp::vector<size_t> & GetLinkIDs() const { return link_ids; }

    [[nodiscard]] Point & GetVelocity() { return velocity; }

    void SetStartTime(double in) { start_time = in; }
    void SetVelocity(Point in) { velocity = in; }
    void SetOrientation(Angle in) { orientation = in; }
    void SetMass(double in) { mass = in; }
    void SetTargetRadius(double in) { target_radius = in; }
    void SetLinkIDs(const emp::vector<size_t> & in) { link_ids = in; }

    // Reset this body to be reused.
    void Deactivate() {
      start_time = std::numeric_limits<double>::max();
      velocity.Set(0.0, 0.0);
      link_ids.resize(0);
      SurfaceBody::Deactivate();
    }

    void ProcessShift(Point in) {
      total_shift += in;
      total_abs_shift += in.Abs();
    }

    // Where will this body be post shift?
    Point2D CalcShiftPos() const { return perimeter.GetCenter() + total_shift; }

    double CalcPressure() const {
      const Point contested_shift = total_abs_shift - total_shift.Abs();
      return contested_shift.SquareMagnitude();
    }

    void FinalizePosition() {
      perimeter += total_shift;
      total_shift.Reset();
      total_abs_shift.Reset();
    }

    // Other orientation and position controls...
    void RotateDegrees(double degrees) { 
      orientation.RotateDegrees(degrees);
    }

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
      DEBUG_STACK();
      emp_assert(!HasLink(id), "Do not add a link to a body multiple times.", id);
      emp_assert(id != GetID(), "Cannot link a body to itself.");
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

    void ProcessVelocity(double friction) {
      if (velocity.AtOrigin()) return; // No velocity; nothing to update. //@CAO remove branch?
      ProcessShift(velocity);
      const double speed = velocity.Magnitude();
      velocity *= (speed > friction) ? (1.0 - friction/speed) : 0.0;
    }

  }; // End of Physics2D::PhysicsBody class

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_GEOMETRY_PHYSICS_BODY_HPP_GUARD
