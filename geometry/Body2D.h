//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
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
//
//  Development notes:
//  * Links should probably be shared by both bodies.
//  * If we are going to have a lot of links, we may want a better data structure than vector.


#ifndef EMP_BODY_2D_H
#define EMP_BODY_2D_H

#include "../tools/assert.h"
#include "../tools/alert.h"
#include "../tools/mem_track.h"
#include "../tools/vector.h"

#include "Angle2D.h"
#include "Circle2D.h"

namespace emp {

  template <typename BODY_TYPE, typename BASE_TYPE=double>
  struct BodyLink {
    BODY_TYPE * other;
    BASE_TYPE cur_dist;     // How far are bodies currently being kept apart?
    BASE_TYPE target_dist;  // How far should the be moved to before splitting?

    BodyLink() : other(nullptr), cur_dist(0), target_dist(0) { ; }
    BodyLink(BODY_TYPE * o, BASE_TYPE cur=0, BASE_TYPE target=0)
      : other(o), cur_dist(cur), target_dist(target) { ; }
    ~BodyLink() { ; }
  };
  
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
    int repro_count;              // Number of offspring currently being produced.
    
    // Information about other bodies that this one is linked to.
    emp::vector< BodyLink<CircleBody2D,BASE_TYPE> > links;
    // emp::vector<CircleBody2D *> links;
    // BASE_TYPE pair_dist;               // How far away should the linked bodies be kept?
    // BASE_TYPE target_pair_dist;        // How far out should a pair get before splitting?

    Point<BASE_TYPE> shift;           // How should this body be updated to minimize overlap.
    Point<BASE_TYPE> cum_shift;       // Build up of shift not yet acted upon.
    Point<BASE_TYPE> total_abs_shift; // Total absolute-value of shifts (to calculate pressure)
    double pressure;                  // Current pressure on this body.

  public:
    CircleBody2D(const Circle<BASE_TYPE> & _p, BRAIN_TYPE * _b = nullptr)
      : perimeter(_p), target_radius(_p.GetRadius()), brain(_b), mass(1), color_id(0)
      , birth_time(0), repro_count(0), pressure(0)
    {
      EMP_TRACK_CONSTRUCT(CircleBody2D);
    }
    ~CircleBody2D() {
      // If this body is paired with another one, remove the pairing.
      if (links.size()) {
        for (auto & link : links) link.other->RemoveLink(*this, false);
        links.resize(0);
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

    // @CAO Links are possible without reproducing; should come up with a better way to track.
    bool IsReproducing() const {
      // return (links.size()) && (GetRadius() != target_radius);
      return repro_count;
    }

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
      for (auto & cur_link : links) if (cur_link.other == &link_org) return true;
      return false;
    }

    CircleBody2D & AddLink(CircleBody2D & link_org, BASE_TYPE cur_dist, BASE_TYPE target_dist) {
      emp_assert(!IsLinked(link_org));  // Don't link twice!

      links.emplace_back(&link_org, cur_dist, target_dist);       // Connect to the linked org.
      link_org.links.emplace_back(this, cur_dist, target_dist);   // Build the connection back.
      
      return *this;
    }

    CircleBody2D & RemoveLink(CircleBody2D & link_org, bool remove_link_back=true) {
      emp_assert(IsLinked(link_org));   // Make sure link exists!

      // Find the link and remove it.
      for (int i = 0; i < (int) links.size(); i++) {
        if (links[i].other == &link_org) {
          links[i] = links.back();
          links.pop_back();
          break;
        }
      }

      // Remove link in other direction (unless we don't need to).
      if (remove_link_back) link_org.RemoveLink(*this, false);

      return *this;
    }

    const BodyLink<CircleBody2D,BASE_TYPE> & FindLink(const CircleBody2D & link_org) const {
      emp_assert(IsLinked(link_org));
      for (auto & link : links) if ( link.other == &link_org) return link;
      return links[0]; // Should never get here!
    }
    
    BodyLink<CircleBody2D,BASE_TYPE> & FindLink(CircleBody2D & link_org)  {
      emp_assert(IsLinked(link_org));
      for (auto & link : links) if ( link.other == &link_org) return link;
      return links[0]; // Should never get here!
    }
    
    BASE_TYPE GetLinkDist(const CircleBody2D & link_org) const {
      emp_assert(!IsLinked(link_org));
      return FindLink(link_org).cur_dist;
    }
    BASE_TYPE GetTargetLinkDist(const CircleBody2D & link_org) const {
      emp_assert(!IsLinked(link_org));
      return FindLink(link_org).target_dist;
    }
    void ShiftLinkDist(CircleBody2D & link_org, BASE_TYPE change) {
      auto & link = FindLink(link_org);
      auto & olink = link.FindLink(*this);
      
      link.cur_dist += change;
      olink.cur_dist = link.cur_dist;
    }

    CircleBody2D * BuildOffspring(emp::Point<BASE_TYPE> offset) {
      // Offspring cannot be right on top of parent.
      emp_assert(offset.GetX() != 0 || offset.GetY() != 0);

      // Create the offspring as a paired link.
      auto * offspring = new CircleBody2D(perimeter, brain ? new BRAIN_TYPE(*brain) : nullptr);
      AddLink(*offspring, offset.Magnitude(), perimeter.GetRadius()*2.0);
      offspring->Translate(offset);

      return offspring;
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
      for (auto & link : links) {
        if (link.cur_dist != link.target_dist) {
          // If we're within the change_factor, just set pair_dist to target.
          if (std::abs(link.cur_dist - link.target_dist) <= change_factor) {
            link.cur_dist = link.target_dist;

            // @CAO Should remove the link!!
          }
          else {
            // @CAO because of the previous check, do we really need to case to int here??
            if ((int) link.cur_dist < (int) link.target_dist) link.cur_dist += change_factor;
            else link.cur_dist -= change_factor;
          }
        }
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
      for (auto & link : links) {
        emp_assert(link.other->IsLinked(*this));

        if (GetAnchor() == link.other->GetAnchor()) {
          // If two organisms are on top of each other... shift one.
          Translate(emp::Point<BASE_TYPE>(0.01, 0.01));
        }
        
        // Figure out how much each oragnism should move so that they will be properly spaced.
        const BASE_TYPE start_dist = GetAnchor().Distance(link.other->GetAnchor());
        const BASE_TYPE link_dist = GetLinkDist(*(link.other));
        const double frac_change = (1.0 - ((double) link_dist) / ((double) start_dist)) / 2.0;
        
        emp::Point<BASE_TYPE> dist_move = (GetAnchor() - link.other->GetAnchor()) * frac_change;
        
        perimeter.Translate(-dist_move);
        link.other->perimeter.Translate(dist_move);
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
      for (auto & link : links) {
        emp_assert(link.other->IsLinked(*this)); // Make sure pairing is reciprical.
        emp_assert(link.cur_dist >= 0);          // Distances cannot be negative.
        emp_assert(link.target_dist >= 0);       // Distances cannot be negative.
      }

      return true;
    }

  };
};

#endif
