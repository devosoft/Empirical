//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Physics2D - handles movement and collissions in a simple 2D world.


#ifndef EMP_PHYSICS_2D_H
#define EMP_PHYSICS_2D_H

#include <unordered_set>
#include <functional>

#include "../base/Ptr.hpp"
#include "../base/vector.hpp"

using namespace std::placeholders;

#include "Surface2D.hpp"

namespace emp {

  template <typename BODY_TYPE> class Physics2D {
  private:
    using Surface_t = Surface2D<BODY_TYPE>;

    Surface_t surface;    // Bodies that can collide.
    Surface_t background; // Bodies that can't collide.

    bool detach_on_birth; // Should bodies detach from their parents when born?

  public:
    Physics2D(double width, double height, double max_org_diameter=20, bool detach=true)
      : surface(width, height)
      , background(width, height)
      , detach_on_birth(detach)
    { ; }
    ~Physics2D() { ; }

    const Surface_t & GetSurface() const { return surface; }
    const Surface_t & GetBackground() const { return background; }
    bool GetDetach() const { return detach_on_birth; }

    Physics2D & SetDetach(bool _in) {
      detach_on_birth = _in;
      // Set all current bodies to new detach setting.
      auto & body_set = surface.GetBodySet();
      for (auto cur_body : body_set) { cur_body->SetDetachOnDivide(_in); }
      return *this;
    }

    Physics2D & AddBody(Ptr<BODY_TYPE> in_body) { surface.AddBody(in_body); return *this; }
    Physics2D & AddBackground(Ptr<BODY_TYPE> in_body) { background.AddBody(in_body); return *this; }

    Physics2D & Clear() { surface.Clear(); background.Clear(); return *this; }

    Physics2D & KillOldest() {
      auto & body_set = surface.GetBodySet();
      if (body_set.size() == 0) return *this;

      size_t oldest_id = 0;

      for (size_t i = 1; i < body_set.size(); i++) {
        if (body_set[i]->GetBirthTime() < body_set[oldest_id]->GetBirthTime()) {
          oldest_id = i;
        }
      }

      // Now kill it!
      body_set[oldest_id].Delete();
      body_set[oldest_id] = body_set.back();
      body_set.resize(body_set.size() - 1);

      return *this;
    }

    bool TestPairCollision(BODY_TYPE & body1, BODY_TYPE & body2) {
      if (body1.IsLinked(body2)) return false;  // Linked bodies can overlap.

      const Point dist = body1.GetCenter() - body2.GetCenter();
      const double sq_pair_dist = dist.SquareMagnitude();
      const double radius_sum = body1.GetRadius() + body2.GetRadius();
      const double sq_min_dist = radius_sum * radius_sum;

      // If there was no collision, return false.
      if (sq_pair_dist >= sq_min_dist) { return false; }

      if (sq_pair_dist == 0.0) {
        // If the shapes are on top of each other, we have a problem.  Shift one!
        body2.Translate(Point(0.01, 0.01));
      }

      // @CAO If objects can phase or explode, identify that here.

      // Re-adjust position to remove overlap.
      const double true_dist = sqrt(sq_pair_dist);
      const double overlap_dist = ((double) radius_sum) - true_dist;
      const double overlap_frac = overlap_dist / true_dist;
      const Point cur_shift = dist * (overlap_frac / 2.0);
      body1.AddShift(cur_shift);
      body2.AddShift(-cur_shift);

      // @CAO if we have inelastic collisions, we just take the weighted average of velocites
      // and let the move together.

      // Assume elastic: Re-adjust velocity to reflect bounce.
      double x1, y1, x2, y2;

      if (dist.GetX() == 0) {
        x1 = body1.GetVelocity().GetX();  y1 = body2.GetVelocity().GetY();
        x2 = body2.GetVelocity().GetX();  y2 = body1.GetVelocity().GetY();

        body1.SetVelocity(Point(x1, y1));
        body2.SetVelocity(Point(x2, y2));
      }
      else if (dist.GetY() == 0) {
        x1 = body2.GetVelocity().GetX();  y1 = body1.GetVelocity().GetY();
        x2 = body1.GetVelocity().GetX();  y2 = body2.GetVelocity().GetY();

        body1.SetVelocity(Point(x1, y1));
        body2.SetVelocity(Point(x2, y2));
      }
      else {
        const Point rel_velocity(body2.GetVelocity() - body1.GetVelocity());
        double normal_a = dist.GetY() / dist.GetX();
        x1 = ( rel_velocity.GetX() + normal_a * rel_velocity.GetY() )
          / ( normal_a * normal_a + 1 );
        y1 = normal_a * x1;
        x2 = rel_velocity.GetX() - x1;
        y2 = - (1 / normal_a) * x2;

        body2.SetVelocity(body1.GetVelocity() + Point(x2, y2));
        body1.SetVelocity(body1.GetVelocity() + Point(x1, y1));
      }


      return true;
    }

    void Update() {
      // Handle movement of bodies

      auto & body_set = surface.GetBodySet();

      for (auto cur_body : body_set) {
        cur_body->BodyUpdate(0.25);       // Let a body change size or shape, as needed.
        cur_body->ProcessStep(0.0125);    // Update position and velocity.
      }

      // Handle collisions
      auto collide_fun =
        [this](BODY_TYPE & b1, BODY_TYPE & b2){ return this->TestPairCollision(b1,b2); };
      surface.TestCollisions(collide_fun);

      // Determine which bodies we should remove.
      size_t cur_id = 0;
      while (cur_id < body_set.size()) {
        emp_assert(body_set[cur_id] != nullptr);
        const double cur_pressure = body_set[cur_id]->GetPressure();

        // @CAO Arbitrary pressure threshold!
        if (cur_pressure > 3.0) {                // If pressure too high, burst this cell!
          body_set[cur_id].Delete();             // Delete the burst cell.
          if (cur_id < body_set.size() - 1) {    // If we are not at the end of the body set...
            body_set[cur_id] = body_set.back();  // ...move last cell to popped position.
          }
          body_set.pop_back();                   // Remove the last element now that it was moved away.
        }
        else cur_id++;
      }
    }

    // Access to bodies
    emp::vector<Ptr<BODY_TYPE>> & GetBodySet() {
      return surface.GetBodySet();
    }
    emp::vector<Ptr<BODY_TYPE>> & GetBackgroundSet() {
      return background.GetBodySet();
    }

    // Access to bodies in a const physics...
    const emp::vector<Ptr<BODY_TYPE>> & GetConstBodySet() const {
      return surface.GetConstBodySet();
    }
    const emp::vector<Ptr<BODY_TYPE>> & GetConstBackgroundSet() const {
      return background.GetConstBodySet();
    }
  };

}

#endif
