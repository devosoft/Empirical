#ifndef EMP_BODY_2D_H
#define EMP_BODY_2D_H

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file defines templated classes to represent bodies that exist on a 2D surface.
//  Each class should be able to:
//   * Maintain a pointer to information about the full organism associated with this body.
//   * Maintain a pointer to which sector this body is currently located in.
//   * provide a circular perimeter of the body (for phase1 of collision detection)
//   * Provide body an anchor point and center point of the body (typically the same)
//
//  Currently, the only type of body we have is:
//
//    CircleBody2D - One individual circular object in the 2D world.
//

#include "Circle2D.h"

namespace emp {

  template <typename BODY_TYPE, typename BODY_INFO, typename BASE_TYPE> class Sector2D;

  template <typename BODY_INFO, typename BASE_TYPE> class CircleBody2D {
  private:
    Circle<BASE_TYPE> perimeter;     // Includes position and size.
    BASE_TYPE target_radius;         // For growing/shrinking
    Angle orientation;               // Which way is body facing?
    BODY_INFO * info;                // External information about individual
    Sector2D<CircleBody2D<BODY_INFO, BASE_TYPE>, BODY_INFO, BASE_TYPE> * sector;  // Track location
    Point<BASE_TYPE> velocity;       // Speed and direction of movement
    BASE_TYPE mass;                  // "Weight" of this object

  public:
    CircleBody2D(const Circle<BASE_TYPE> & _p, BODY_INFO * _i) : perimeter(_p), info(_i) { ; }
    ~CircleBody2D() { ; }

    const Circle<BASE_TYPE> & GetPerimeter() const { return perimeter; }
    const Point<BASE_TYPE> & GetAnchor() const { return perimeter.GetCenter(); }
    const Point<BASE_TYPE> & GetCenter() const { return perimeter.GetCenter(); }
    BASE_TYPE GetRadius() const { return perimeter.GetRadius(); }
    BASE_TYPE GetTargetRadius() const { return target_radius; }
    const Angle & GetOrientation() const { return orientation; }
    BODY_INFO * GetInfo() { return info; }
    Sector2D<CircleBody2D<BODY_INFO, BASE_TYPE>, BODY_INFO, BASE_TYPE> * GetSector() { return sector; }
    const Point<BASE_TYPE> & GetVelocity() const { return velocity; }
    BASE_TYPE GetMass() const { return mass; }

    CircleBody2D<BODY_INFO, BASE_TYPE> & SetPosition(const Point<BASE_TYPE> & new_pos) {
      perimeter.SetCenter(new_pos); 
      return *this;
    }
    CircleBody2D<BODY_INFO, BASE_TYPE> &
    SetSector(Sector2D<CircleBody2D<BODY_INFO, BASE_TYPE>, BODY_INFO, BASE_TYPE> * new_sector) {
      sector = new_sector;
      return *this;
    }

    CircleBody2D<BODY_INFO, BASE_TYPE> & BodyUpdate(BASE_TYPE grow_factor=1) {
      if ((int) grow_radius > (int) radius) radius += grow_factor;
      else if ((int) grow_radius < (int) radius) radius -= grow_factor;
      return *this;
    }

    CircleBody2D<BODY_INFO, BASE_TYPE> & ProcessStep(BASE_TYPE friction) {
      if (velocity.NonZero()) {
        perimeter.Translate(velocity);
        const double velocity_mag = velocity.Magnitude();
        if (velocity_mag > friction) { velocity.ToOrigin(); }
        else { velocity *= 1.0 - friction / velocity_mag; }
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
    
  };

};

#endif
