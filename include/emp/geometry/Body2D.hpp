/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2016-2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/geometry/Body2D.hpp
 * @brief This file defines classes to represent circular bodies that exist on a 2D surface.
 *
 * Development notes:
 * - Right now all bodies are circular, but additional internal detail should be allowed
 * - consider alternative data structures for links (knowing that there will never be more than ~6)
 */

#pragma once

#ifndef INCLUDE_EMP_GEOMETRY_BODY2D_HPP_GUARD
#define INCLUDE_EMP_GEOMETRY_BODY2D_HPP_GUARD

#include <cstdint>
#include <stddef.h>

#include "../base/assert.hpp"
#include "../base/notify.hpp"
#include "../base/Ptr.hpp"
#include "../base/vector.hpp"
#include "../debug/mem_track.hpp"

#include "Angle.hpp"
#include "Circle2D.hpp"

namespace emp {

  class Body2D {
  protected:
    // Bodies can be linked in several ways.
    // MULTICELL -> Joined together with no extra meaning
    // REPRODUCTION -> "from" is gestating "to"
    // ATTACK -> "from" is trying to eat "to"
    // PARASITE -> "from" is stealing resources from "to"
    enum class LinkType { MULTICELL, REPRODUCTION, ATTACK, PARASITE };

    struct BodyLink {
      LinkType type      = LinkType::MULTICELL;  // MULTICELL, REPRODUCTION, ATTACK, PARASITE
      Ptr<Body2D> from   = nullptr;              // Initiator of link (e.g., parent, attacker)
      Ptr<Body2D> to     = nullptr;              // Target of link (e.g., offspring, prey/host)
      double cur_dist    = 0.0;                  // Current distance apart of bodies
      double target_dist = 0.0;                  // Goal distance apart (e.g., if growing)

      constexpr BodyLink(LinkType t, Ptr<Body2D> _frm, Ptr<Body2D> _to, double cur = 0, double target = 0)
        : type(t), from(_frm), to(_to), cur_dist(cur), target_dist(target) {}
      constexpr BodyLink(const BodyLink &) = default;
      constexpr BodyLink(BodyLink &&) = default;
      constexpr ~BodyLink() = default;
    };

    // Body information
    double target_radius;     // For growing/shrinking
    double birth_time = 0.0;  // When was this body created / born?
    Angle orientation;        // Which way is body facing?
    Point velocity;           // Speed and direction of movement
    double mass       = 1.0;  // "Weight" of this body (@CAO not used yet..)
    size_t color_id = 0;      // Which color should this body appear?
    int repro_count   = 0;    // Number of offspring currently being produced.

    // Ongoing calculations
    Point shift;            // How should this body be updated to minimize overlap.
    Point total_shift;      // Build up of shift not yet acted upon.
    Point total_abs_shift;  // Total absolute-value of shifts (to calculate pressure)
    double pressure = 0.0;  // Current pressure on this body.

    // Information about other bodies that this one is linked to.
    emp::vector<Ptr<BodyLink>> from_links;  // Active links initiated by body
    emp::vector<Ptr<BodyLink>> to_links;    // Active links targeting body

    bool detach_on_divide = true;  // Should offspring detach (or stay linked to parent)
  public:
    Body2D() {}
    Body2D(Body2D &&) = default;

    ~Body2D() {
      // Remove any remaining links from this body.
      while (from_links.size()) { RemoveLink(from_links[0]); }
      while (to_links.size()) { RemoveLink(to_links[0]); }
    }

    [[nodiscard]] constexpr double GetBirthTime() const { return birth_time; }
    [[nodiscard]] constexpr const Point & GetVelocity() const { return velocity; }
    [[nodiscard]] constexpr const Angle & GetOrientation() const { return orientation; }
    [[nodiscard]] constexpr double GetMass() const { return mass; }

    [[nodiscard]] constexpr bool IsReproducing() const { return repro_count; }
    [[nodiscard]] constexpr int GetReproCount() const { return repro_count; }
    [[nodiscard]] constexpr Point GetShift() const { return shift; }
    [[nodiscard]] constexpr double GetPressure() const { return pressure; }
    [[nodiscard]] constexpr bool GetDetachOnDivide() const { return detach_on_divide; }

    [[nodiscard]] constexpr double GetTargetRadius() const { return target_radius; }

    constexpr void SetTargetRadius(double t) { target_radius = t; }

    constexpr void SetBirthTime(double in) { birth_time = in; }
    constexpr void SetOrientation(Angle in) { orientation = in; }
    constexpr void SetVelocity(Point in) { velocity = in; }
    constexpr void SetMass(double in) { mass = in; }
    constexpr void SetColorID(size_t in) { color_id = in; }

    // Other orientation controls...
    constexpr void TurnLeft(int steps = 1) { orientation.RotateDegrees(steps * 45); }
    constexpr void TurnRight(int steps = 1) { orientation.RotateDegrees(steps * -45); }
    constexpr void RotateDegrees(double degrees) { orientation.RotateDegrees(degrees); }

    // Other velocity controls...
    constexpr void IncSpeed(const Point & offset) { velocity += offset; }
    constexpr void IncSpeed() { velocity += orientation.GetPoint(); }
    constexpr void DecSpeed() { velocity -= orientation.GetPoint(); }
    constexpr void SetVelocity(double x, double y) { velocity.Set(x, y); }

    // Shift to apply next update.
    constexpr void AddShift(const Point & s) {
      shift += s;
      total_abs_shift += s.Abs();
    }

    // Controls about replication
    constexpr void SetDetachOnDivide(bool in = true) { detach_on_divide = in; }

    // Creating, testing, and unlinking other organisms
    [[nodiscard]] constexpr bool IsLinkedFrom(const Body2D & link_org) const {
      for (auto cur_link : from_links) {
        if (cur_link->to == &link_org) { return true; }
      }
      return false;
    }

    [[nodiscard]] constexpr bool IsLinkedTo(const Body2D & link_org) const {
      return link_org.IsLinkedFrom(*this);
    }

    [[nodiscard]] constexpr bool IsLinked(const Body2D & link_org) const {
      return IsLinkedFrom(link_org) || IsLinkedTo(link_org);
    }

    [[nodiscard]] constexpr size_t GetLinkCount() const { return from_links.size() + to_links.size(); }

    constexpr void AddLink(LinkType type, Body2D & link_org, double cur_dist, double target_dist) {
      emp_assert(!IsLinked(link_org));  // Don't link twice!

      // Build connections in both directions.
      auto new_link = NewPtr<BodyLink>(type, this, &link_org, cur_dist, target_dist);
      from_links.push_back(new_link);
      link_org.to_links.push_back(new_link);
    }

    constexpr void RemoveLink(Ptr<BodyLink> link) {
      // We should always initiate link removal from the FROM side.
      if (link->to == ToPtr(this)) {
        link->from->RemoveLink(link);
        return;
      }

      // Find and remove the associated FROM link from this body.
      for (size_t i = 0; i < from_links.size(); i++) {
        if (from_links[i]->to == link->to) {
          from_links[i] = from_links.back();
          from_links.pop_back();
          break;
        }
      }

      // Find and remove the TO link from the attached body.
      const size_t to_size = link->to->to_links.size();
      for (size_t i = 0; i < to_size; i++) {
        if (link->to->to_links[i]->from == ToPtr(this)) {
          auto & other_links = link->to->to_links;
          other_links[i]     = other_links.back();
          other_links.pop_back();
          break;
        }
      }

      link.Delete();
    }

    [[nodiscard]] constexpr const BodyLink & FindLink(const Body2D & link_org) const {
      emp_assert(IsLinked(link_org));
      for (auto link : from_links) {
        if (link->to == &link_org) { return *link; }
      }
      return link_org.FindLink(*this);
    }

    [[nodiscard]] constexpr BodyLink & FindLink(Body2D & link_org) {
      emp_assert(IsLinked(link_org));
      for (auto link : from_links) {
        if (link->to == ToPtr(&link_org)) { return *link; }
      }
      return link_org.FindLink(*this);
    }

    [[nodiscard]] constexpr double GetLinkDist(const Body2D & link_org) const {
      emp_assert(IsLinked(link_org));
      return FindLink(link_org).cur_dist;
    }

    [[nodiscard]] constexpr double GetTargetLinkDist(const Body2D & link_org) const {
      emp_assert(IsLinked(link_org));
      return FindLink(link_org).target_dist;
    }

    constexpr void ShiftLinkDist(Body2D & link_org, double change) {
      auto & link = FindLink(link_org);
      link.cur_dist += change;
    }

    [[nodiscard]] constexpr Ptr<Body2D> BuildOffspring(Point offset) {
      // Offspring cannot be right on top of parent.
      emp_assert(offset.GetX() != 0 || offset.GetY() != 0);

      // Create the offspring as a paired link.
      auto offspring = NewPtr<Body2D>(perimeter);
      AddLink(LinkType::REPRODUCTION, *offspring, offset.Magnitude(), perimeter.GetRadius() * 2.0);
      offspring->MoveBy(offset);
      repro_count++;

      return offspring;
    }

    // If a body is not at its target radius, grow it or shrink it, as needed.
    constexpr void BodyUpdate(double change_factor = 1) {
      // Test if this body needs to grow or shrink.
      if ((int) target_radius > (int) GetRadius()) {
        SetRadius(GetRadius() + change_factor);
      } else if ((int) target_radius < (int) GetRadius()) {
        SetRadius(GetRadius() - change_factor);
      }

      // Test if the link distance for this body needs to be updated
      for (size_t i = 0; i < from_links.size(); i++) {
        auto link = from_links[i];
        if (link->cur_dist == link->target_dist) {
          continue;  // No adjustment needed.
        }

        // If we're within the change_factor, just set pair_dist to target.
        if (std::abs(link->cur_dist - link->target_dist) <= change_factor) {
          link->cur_dist = link->target_dist;
          // IF this organism was gestating, finish the reproduction.
          if (link->type == LinkType::REPRODUCTION) {
            emp_assert(repro_count > 0);
            repro_count--;
            if (detach_on_divide) {  // Flag link for removal!
              RemoveLink(link);      // Remove the link.
              i--;                   // Check this position again.
            }
          }
        } else {
          if (link->cur_dist < link->target_dist) {
            link->cur_dist += change_factor;
          } else {
            link->cur_dist -= change_factor;
          }
        }
      }
    }

    // Move this body by its velocity and reduce velocity based on friction.
    constexpr void ProcessStep(double friction = 0) {
      if (velocity.IsNonZero()) {
        perimeter += velocity;
        const double velocity_mag = velocity.Magnitude();

        // If body is close to stopping stop it!
        if (friction > velocity_mag) {
          velocity.Set(0.0, 0.0);
        }

        // Otherwise slow it down proportionately in the x and y directions.
        else {
          velocity *= 1.0 - ((double) friction) / ((double) velocity_mag);
        }
      }
    }

    // Determine where the circle will end up and force it to be within a bounding box.
    constexpr void FinalizePosition(const Point & max_coords) {
      const double max_x = max_coords.GetX() - GetRadius();
      const double max_y = max_coords.GetY() - GetRadius();

      // Update the calculation for pressure.

      // Act on the accumulated shifts only when they add up enough.
      total_shift += shift;
      if (total_shift.SquareMagnitude() > 0.25) {
        perimeter += total_shift;
        total_shift.Set(0.0, 0.0);
      }
      pressure = (total_abs_shift - shift.Abs()).SquareMagnitude();
      shift.Set(0.0, 0.0);  // Clear out the shift for the next round.
      total_abs_shift.Set(0.0, 0.0);

      // If this body is linked to another, enforce the distance between them.
      for (auto link : from_links) {
        if (GetCenter() == link->to->GetCenter()) {
          // If two organisms are on top of each other... shift one.
          MoveBy(Point(0.01, 0.01));
        }

        // Figure out how much each organism should move so that they will be properly spaced.
        const double start_dist  = GetCenter().Distance(link->to->GetCenter());
        const double link_dist   = link->cur_dist;
        const double frac_change = (1.0 - ((double) link_dist) / ((double) start_dist)) / 2.0;

        Point dist_move = (GetCenter() - link->to->GetCenter()) * frac_change;

        perimeter += -dist_move;
        link->to->perimeter += dist_move;
      }

      // Adjust the organism so it stays within the bounding box of the world.
      if (GetCenter().GetX() < GetRadius()) {
        perimeter.SetCenterX(GetRadius());  // Put back in range...
        velocity = velocity.NegateX();      // Bounce off left side.
      } else if (GetCenter().GetX() > max_x) {
        perimeter.SetCenterX(max_x);    // Put back in range...
        velocity = velocity.NegateX();  // Bounce off right side.
      }

      if (GetCenter().GetY() < GetRadius()) {
        perimeter.SetCenterY(GetRadius());  // Put back in range...
        velocity = velocity.NegateY();      // Bounce off top.
      } else if (GetCenter().GetY() > max_y) {
        perimeter.SetCenterY(max_y);    // Put back in range...
        velocity = velocity.NegateY();  // Bounce off bottom.
      }
    }

    // Check to make sure there are no obvious issues with this object.
    constexpr bool OK() {
      for (auto link : from_links) {
        (void) link;
        emp_assert(link->cur_dist >= 0);     // Distances cannot be negative.
        emp_assert(link->target_dist >= 0);  // Distances cannot be negative.
      }

      return true;
    }
  };
}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_GEOMETRY_BODY2D_HPP_GUARD

// Local settings for Empecable file checker.
// empecable_words: frm
