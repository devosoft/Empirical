// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

#ifndef EMP_BODY_2D_H
#define EMP_BODY_2D_H

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file defines templated classes to represent bodies that exist on a 2D surface.
//  Each class should be able to:
//   * Maintain a pointer to information about the full organism associated with this body.
//   * provide a circular perimeter of the body (for phase1 of collision detection)
//   * Provide body an anchor point and center point of the body (typically the same)
//
//  Currently, the only type of body we have is:
//
//    CircleBody2D - One individual circular object in the 2D world.
//

#include "../tools/assert.h"
#include "../tools/alert.h"
#include "../tools/mem_track.h"

#include "Angle2D.h"
#include "Circle2D.h"

namespace emp {

  template <typename BRAIN_TYPE, typename BASE_TYPE=double>
  class CircleBody2D {
  private:
    Circle<BASE_TYPE> perimeter;  // Includes position and size.
    Angle orientation;            // Which way is body facing?
    BASE_TYPE target_radius;      // For growing/shrinking
    BRAIN_TYPE * brain;           // Controller for individual
    Point<BASE_TYPE> velocity;    // Speed and direction of movement
    BASE_TYPE mass;               // "Weight" of this object (@CAO not used yet..)
    uint32_t color_id;            // Which color should this body appear?
    double birth_time;            // At what time point was this organism born?

    // @CAO Technically, we should allow any number of links.
    CircleBody2D * pair_link;     // Is this body physically linked to another?
    BASE_TYPE pair_dist;          // How far away should the linked body be kept?
    BASE_TYPE target_pair_dist;   // How far out should the pair get before splitting?

    Point<BASE_TYPE> shift;           // How should this body be updated to minimize overlap.
    Point<BASE_TYPE> cum_shift;       // Build up of shift not yet acted upon.
    Point<BASE_TYPE> total_abs_shift; // Total absolute-value of shifts (to calculate pressure)
    double pressure;                  // Current pressure on this body.

  public:
    CircleBody2D(const Circle<BASE_TYPE> & _p, BRAIN_TYPE * _b = nullptr)
      : perimeter(_p), target_radius(_p.GetRadius()), brain(_b), mass(1), color_id(0)
      , birth_time(0)
      , pair_link(nullptr), pair_dist(0), target_pair_dist(0), pressure(0)
    {
      EMP_TRACK_CONSTRUCT(CircleBody2D);
    }
    ~CircleBody2D() {
      // If this body is paired with another one, remove the pairing.
      if (pair_link) {
        emp_assert(pair_link->pair_link == this);
        pair_link->pair_link = nullptr;
      }
      if (brain) delete brain;
      EMP_TRACK_DESTRUCT(CircleBody2D);
    }

    const Circle<BASE_TYPE> & GetPerimeter() const { return perimeter; }
    const Point<BASE_TYPE> & GetAnchor() const { return perimeter.GetCenter(); }
    const Point<BASE_TYPE> & GetCenter() const { return perimeter.GetCenter(); }
    BASE_TYPE GetRadius() const { return perimeter.GetRadius(); }
    const Angle & GetOrientation() const { return orientation; }
    BASE_TYPE GetTargetRadius() const { return target_radius; }
    BRAIN_TYPE * GetBrain() { return brain; }
    const Point<BASE_TYPE> & GetVelocity() const { return velocity; }
    BASE_TYPE GetMass() const { return mass; }
    uint32_t GetColorID() const { return color_id; }
    double GetBirthTime() const { return birth_time; }

    Point<BASE_TYPE> GetShift() const { return shift; }
    double GetPressure() const { return pressure; }

    bool IsReproducing() const { return (pair_link != nullptr) && (GetRadius() != target_radius); }

    CircleBody2D & SetPosition(const Point<BASE_TYPE> & new_pos) {
      //if (perimeter.GetCenter().SquareDistance(new_pos) > 2.0)
        perimeter.SetCenter(new_pos);
      return *this;
    }
    CircleBody2D & SetRadius(BASE_TYPE new_radius) {
      perimeter.SetRadius(new_radius); 
      return *this;
    }
    CircleBody2D & SetTargetRadius(BASE_TYPE t) { target_radius = t; return *this; }
    CircleBody2D & SetVelocity(BASE_TYPE x, BASE_TYPE y) { velocity.Set(x, y); return *this; }
    CircleBody2D & SetVelocity(const Point<BASE_TYPE> & v) { velocity = v; return *this; }
    CircleBody2D & SetColorID(uint32_t in_id) { color_id = in_id; return *this; }
    CircleBody2D & SetBirthTime(double in_time) { birth_time = in_time; return *this; }

    // Shift at end of next update.
    CircleBody2D & AddShift(const Point<BASE_TYPE> & inc_val) {
      shift += inc_val;
      total_abs_shift += inc_val.Abs();
      return *this;
    }

    // Translate immediately.
    CircleBody2D & Translate(const Point<BASE_TYPE> & inc_val) {
      perimeter.Translate(inc_val);
      return *this;
    }

    // Creating, testing, and unlinking other organisms (used for gestation & reproduction)
    bool IsLinked(const CircleBody2D & link_org) const {
      return pair_link == &link_org;
    }
    BASE_TYPE GetLinkDist(const CircleBody2D & link_org) const {
      if (!IsLinked(link_org)) return -1;
      return pair_dist;
    }
    BASE_TYPE GetTargetLinkDist(const CircleBody2D & link_org) const {
      if (!IsLinked(link_org)) return -1;
      return target_pair_dist;
    }
    void ShiftLinkDist(CircleBody2D & link_org, BASE_TYPE change) {
      pair_dist += change;
      if (pair_link) pair_link->pair_dist = pair_dist;
    }

    CircleBody2D * BuildOffspring(emp::Point<BASE_TYPE> offset) {
      // emp::Alert("Building Offspring at offset ", offset.GetX(), ',', offset.GetY());

      emp_assert(offset.GetX() != 0 || offset.GetY() != 0);
      if (pair_link) {   // If this body is already paired with another, break that link!
        emp_assert(pair_link->pair_link == this);
        pair_link->pair_link = nullptr;
      }
      // Create the offspring as a paired link.
      pair_link = new CircleBody2D(perimeter, brain ? new BRAIN_TYPE(*brain) : nullptr);
      pair_link->pair_link = this;
      pair_link->Translate(offset);

      // Setup distances with offspring...
      pair_dist = offset.Magnitude();
      pair_link->pair_dist = pair_dist;
      target_pair_dist = perimeter.GetRadius() * 2.0;
      pair_link->target_pair_dist = target_pair_dist;

      return pair_link;
    }

    void BreakLink(CircleBody2D * old_link) {
      emp_assert(pair_link == old_link);
      emp_assert(old_link->pair_link == this);
      pair_link = nullptr;
      old_link->pair_link = nullptr;
      pair_dist = 0;
    }

    CircleBody2D<BRAIN_TYPE, BASE_TYPE> &
    TurnLeft(int steps=1) { orientation.RotateDegrees(45); return *this; }
    CircleBody2D<BRAIN_TYPE, BASE_TYPE> &
    TurnRight(int steps=1) { orientation.RotateDegrees(-45); return *this; }
    CircleBody2D<BRAIN_TYPE, BASE_TYPE> &
    IncSpeed(double steps=1.0) {
      velocity += Point<BASE_TYPE>(orientation.Sin(), orientation.Cos());
      return *this;
    }
    CircleBody2D<BRAIN_TYPE, BASE_TYPE> &
    DecSpeed(double steps=1.0) { return *this; }

    // If a body is not at its target radius, grow it or shrink it, as needed.
    CircleBody2D<BRAIN_TYPE, BASE_TYPE> & BodyUpdate(BASE_TYPE change_factor=1) {
      // Test if this body needs to grow or shrink.
      if ((int) target_radius > (int) GetRadius()) SetRadius(GetRadius() + change_factor);
      else if ((int) target_radius < (int) GetRadius()) SetRadius(GetRadius() - change_factor);

      // Test if the link distance for this body needs to be updated
      if (pair_link && pair_dist != target_pair_dist) {
        // If we're within the change_factor, just set pair_dist to target.
        if (std::abs(pair_dist - target_pair_dist) <= change_factor) {
          pair_dist = target_pair_dist;
          // @CAO, for now, break the link!
          pair_link->pair_link = nullptr;
          pair_link = nullptr;
        }
        else {
          if ((int) pair_dist < (int) target_pair_dist) pair_dist += change_factor;
          else pair_dist -= change_factor;
        }
        pair_link->pair_dist = pair_dist;
      }

      return *this;
    }

    // Move this body by its velocity and reduce velocity based on friction.
    CircleBody2D<BRAIN_TYPE, BASE_TYPE> & ProcessStep(BASE_TYPE friction=0) {
      if (velocity.NonZero()) {
        perimeter.Translate(velocity);
        const double velocity_mag = velocity.Magnitude();

        // If body is close to stopping stop it!
        if (friction > velocity_mag) { velocity.ToOrigin(); }

        // Otherwise slow it down proportionately in the x and y directions.
        else { velocity *= 1.0 - ((double) friction) / ((double) velocity_mag); }
      }   
      return *this;
    }


    // Determine where the circle will end up and force it to be within a bounding box.
    CircleBody2D<BRAIN_TYPE, BASE_TYPE> & FinalizePosition(const Point<BASE_TYPE> & max_coords) {
      const BASE_TYPE max_x = max_coords.GetX() - GetRadius();
      const BASE_TYPE max_y = max_coords.GetY() - GetRadius();

      // Update the caclulcation for pressure.

      // Act on the accumulated shifts only when they add up enough.
      cum_shift += shift;
      if (cum_shift.SquareMagnitude() > 0.25) {
        perimeter.Translate(cum_shift);
        cum_shift.ToOrigin();
      }
      pressure = (total_abs_shift - shift.Abs()).SquareMagnitude();
      shift.ToOrigin();              // Clear out the shift for the next round.
      total_abs_shift.ToOrigin();
        
      // If this body is linked to another, enforce the distance between them.
      if (pair_link != nullptr) {
        emp_assert(pair_link->pair_link == this);

        if (GetAnchor() == pair_link->GetAnchor()) {
          // If two organisms are on top of each other... shift one.
          Translate(emp::Point<BASE_TYPE>(0.01, 0.01));
        }
        
        // Figure out how much each oragnism should move so that they will be properly spaced.
        const BASE_TYPE start_dist = GetAnchor().Distance(pair_link->GetAnchor());
        const BASE_TYPE link_dist = GetLinkDist(*pair_link);
        const double frac_change = (1.0 - ((double) link_dist) / ((double) start_dist)) / 2.0;
        
        emp::Point<BASE_TYPE> dist_move = (GetAnchor() - pair_link->GetAnchor()) * frac_change;
        
        perimeter.Translate(-dist_move);
        pair_link->perimeter.Translate(dist_move);
      }
      
      // Adjust the organism so it stays within the bounding box of the world.
      if (GetCenter().GetX() < GetRadius()) { 
        perimeter.SetCenterX(GetRadius());     // Put back in range...
        velocity.NegateX();                    // Bounce off left side.
      } else if (GetCenter().GetX() > max_x) {
        perimeter.SetCenterX(max_x);           // Put back in range...
        velocity.NegateX();                    // Bounce off right side.
      }

      if (GetCenter().GetY() < GetRadius()) { 
        perimeter.SetCenterY(GetRadius());     // Put back in range...
        velocity.NegateY();                    // Bounce off top.
      } else if (GetCenter().GetY() > max_y) {
        perimeter.SetCenterY(max_y);           // Put back in range...
        velocity.NegateY();                    // Bounce off bottom.
      }

      return *this;
    }
    
    // Check to make sure there are no obvious issues with this object.
    bool OK() {
      if (pair_link) {
        emp_assert(pair_link->pair_link == this); // Make sure pairing is reciprical.
        emp_assert(pair_dist >= 0);               // Distances cannot be negative
        emp_assert(target_pair_dist >= 0);        // Distances cannot be negative
      }
      return true;
    }

  };
};

#endif
