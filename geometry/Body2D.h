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

#include "Angle2D.h"
#include "Circle2D.h"

namespace emp {

  template <typename BODY_INFO, typename BASE_TYPE> class CircleBody2D {
  private:
    Circle<BASE_TYPE> perimeter;     // Includes position and size.
    Angle orientation;               // Which way is body facing?
    BASE_TYPE target_radius;         // For growing/shrinking
    BODY_INFO * info;                // External information about individual
    Point<BASE_TYPE> velocity;       // Speed and direction of movement
    BASE_TYPE mass;                  // "Weight" of this object
    int color_id;                    // Which color should this body appear?

    // @CAO Technically, we should allow any number of links.
    CircleBody2D * pair_link;        // Is this body linked to another (typically part of reproduction)
    BASE_TYPE pair_dist;             // How far away should the linked body be kept?

    Point<BASE_TYPE> shift;          // How should this body be updated to minimize overlap.

  public:
    CircleBody2D(const Circle<BASE_TYPE> & _p, BODY_INFO * _i)
      : perimeter(_p), target_radius(-1), info(_i), mass(1), color_id(0)
      , pair_link(NULL), pair_dist(0) { ; }
    ~CircleBody2D() { ; }

    const Circle<BASE_TYPE> & GetPerimeter() const { return perimeter; }
    const Point<BASE_TYPE> & GetAnchor() const { return perimeter.GetCenter(); }
    const Point<BASE_TYPE> & GetCenter() const { return perimeter.GetCenter(); }
    BASE_TYPE GetRadius() const { return perimeter.GetRadius(); }
    const Angle & GetOrientation() const { return orientation; }
    BASE_TYPE GetTargetRadius() const { return target_radius; }
    BODY_INFO * GetInfo() { return info; }
    const Point<BASE_TYPE> & GetVelocity() const { return velocity; }
    BASE_TYPE GetMass() const { return mass; }
    int GetColorID() const { return color_id; }

    // Creating, testing, and unlinking other organisms (used for gestation & reproduction)
    bool IsLinked(CircleBody2D * link_test) { return pair_link == link_test; }
    BASE_TYPE GetLinkDist() { return pair_dist; }
    CircleBody2D * BuildOffspring() {
      pair_link = new CircleBody2D(perimeter, new BODY_INFO(*info));
      pair_dist = 0;
      return pair_link;
    }
    void BreakLink(CircleBody2D * old_link) {
      assert(pair_link == old_link);
      pair_link = NULL;
      pair_dist = 0;
    }

    CircleBody2D<BODY_INFO, BASE_TYPE> & SetPosition(const Point<BASE_TYPE> & new_pos) {
      perimeter.SetCenter(new_pos); 
      return *this;
    }
    CircleBody2D<BODY_INFO, BASE_TYPE> & SetRadius(BASE_TYPE new_radius) {
      perimeter.SetRadius(new_radius); 
      return *this;
    }
    CircleBody2D<BODY_INFO, BASE_TYPE> &
    SetTargetRadius(BASE_TYPE _target) { target_radius = _target; return *this; }
    CircleBody2D<BODY_INFO, BASE_TYPE> &
    SetVelocity(BASE_TYPE _x, BASE_TYPE _y) { velocity.Set(_x, _y); return *this; }
    CircleBody2D<BODY_INFO, BASE_TYPE> &
    SetVelocity(const Point<BASE_TYPE> & in_vel) { velocity = in_vel; return *this; }
    CircleBody2D<BODY_INFO, BASE_TYPE> &
    SetColorID(int in_id) { color_id = in_id; return *this; }

    CircleBody2D<BODY_INFO, BASE_TYPE> &
    AddShift(const Point<BASE_TYPE> & inc_val) { shift += inc_val; return *this; }

    CircleBody2D<BODY_INFO, BASE_TYPE> &
    TurnLeft(int steps=1) { orientation.RotateDegrees(45); return *this; }
    CircleBody2D<BODY_INFO, BASE_TYPE> &
    TurnRight(int steps=1) { orientation.RotateDegrees(-45); return *this; }
    CircleBody2D<BODY_INFO, BASE_TYPE> &
    IncSpeed(double steps=1.0) {
      velocity += Point<BASE_TYPE>(orientation.Sin(), orientation.Cos());
      return *this;
    }
    CircleBody2D<BODY_INFO, BASE_TYPE> &
    DecSpeed(double steps=1.0) { return *this; }

    // If a body is not at its target radius, grow it or shrink it, as needed.
    CircleBody2D<BODY_INFO, BASE_TYPE> & BodyUpdate(BASE_TYPE grow_factor=1) {
      if (target_radius == -1) return *this;
      if ((int) target_radius > (int) GetRadius()) SetRadius(GetRadius() + grow_factor);
      else if ((int) target_radius < (int) GetRadius()) SetRadius(GetRadius() - grow_factor);
      else target_radius = -1;  // We're at the target size, so stop!
      return *this;
    }

    // Move this body by its velocity and reduce velocity based on friction.
    CircleBody2D<BODY_INFO, BASE_TYPE> & ProcessStep(BASE_TYPE friction=0) {
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


    // Force a circle to be within a bounding box.
    CircleBody2D<BODY_INFO, BASE_TYPE> & AdjustPosition(const Point<BASE_TYPE> & max_coords) {
      const BASE_TYPE max_x = max_coords.GetX() - GetRadius();
      const BASE_TYPE max_y = max_coords.GetY() - GetRadius();

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
    
    // After all collisions are accounted for, we should shift this object into its final place.
    void FinalizePosition()
    {
      perimeter.Translate(shift);
      shift.ToOrigin();
    }

  };
};

#endif
