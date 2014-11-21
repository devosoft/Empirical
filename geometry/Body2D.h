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

#include "Angle2D.h"
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

    Point<BASE_TYPE> shift;          // How should this body be updated to minimize overlap.

  public:
    CircleBody2D(const Circle<BASE_TYPE> & _p, BODY_INFO * _i)
      : perimeter(_p), target_radius(-1), info(_i) { ; }
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
    CircleBody2D<BODY_INFO, BASE_TYPE> & SetRadius(BASE_TYPE new_radius) {
      perimeter.SetRadius(new_radius); 
      return *this;
    }
    CircleBody2D<BODY_INFO, BASE_TYPE> &
    SetSector(Sector2D<CircleBody2D<BODY_INFO, BASE_TYPE>, BODY_INFO, BASE_TYPE> * new_sector) {
      sector = new_sector;
      return *this;
    }
    CircleBody2D<BODY_INFO, BASE_TYPE> &
    SetVelocity(BASE_TYPE _x, BASE_TYPE _y) { velocity.Set(_x, _y); return *this; }

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
    
    bool CollisionTest(CircleBody2D<BODY_INFO, BASE_TYPE> & object2)
    {
      const Point<BASE_TYPE> dist = GetCenter() - object2.GetCenter();
      const BASE_TYPE sq_pair_dist = dist.SquareMagnitude();
      const BASE_TYPE radius_sum = GetRadius() + object2.GetRadius();
      const BASE_TYPE sq_min_dist = radius_sum * radius_sum;

      // std::cerr << "Object 1: " << GetCenter() << " ; radius=" << GetRadius() << std::endl;
      // std::cerr << "Object 2: " << object2.GetCenter() << " ; radius=" << object2.GetRadius() << std::endl;
      // std::cerr << "dist: " << dist << std::endl;

  
      // If there was no collision, return.
      if (sq_pair_dist >= sq_min_dist) { return false; }

      // @CAO If objects can phase or explode, identify that here.
      
      // Re-adjust position to remove overlap.
      const double true_dist = sqrt(sq_pair_dist);
      const double overlap_frac = ((double) radius_sum) / true_dist - 1.0;
      const Point<BASE_TYPE> cur_shift = dist * (overlap_frac / 2.0);
      shift += cur_shift;
      object2.shift -= cur_shift;

      // @CAO if we have inelastic collisions, we just take the weighted average of velocites
      // and let the move together.
      

      // Assume elastic: Re-adjust velocity to reflect bounce.
      const Point<BASE_TYPE> rel_velocity(object2.velocity - velocity);
  
      double x1, x2, y1, y2;
  
      if (dist.GetX() == 0) {
        y1 = rel_velocity.GetY();
        x1 = 0; x2 = 0; y2 = 0;
      }
      else if (dist.GetY() == 0) {
        x1 = rel_velocity.GetX();
        y1 = 0; x2 = 0; y2 = 0;
      }
      else {
        double normal_a = dist.GetY() / dist.GetX();
        x1 = ( rel_velocity.GetX() + normal_a * rel_velocity.GetY() )
          / ( normal_a * normal_a + 1 );
        y1 = normal_a * x1;
        x2 = rel_velocity.GetX() - x1;
        y2 = - (1 / normal_a) * x2;
      }
  
      object2.velocity = velocity + Point<BASE_TYPE>(x2, y2);
      velocity = velocity + Point<BASE_TYPE>(x1, y1);

      return true;
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
