//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::Random

#include <iostream>
#include "geometry/Point2D.h"
#include "geometry/Surface.h"
#include "tools/Random.h"
#include "tools/random_utils.h"
#include "tools/string_utils.h"

class TestBody {
protected:
  emp::Point center;
  double radius;
  size_t id;
public:
  TestBody(emp::Point _center={0,0}, double _radius=1.0) : center(_center), radius(_radius), id(0) { ; }
  ~TestBody() { ; }

  emp::Point GetCenter() const { return center; }
  double GetRadius() const { return radius; }
  size_t GetID() const { return id; }

  void SetCenter(emp::Point _c) { center = _c; }
  void SetRadius(double _r) { radius = _r; }
  void Set(emp::Point _c, double _r) { center = _c; radius = _r; }
  void SetID(size_t _id) { id = _id; }

  std::string AsString() const {
    size_t x = (size_t) (center.GetX() + 0.5);
    size_t y = (size_t) (center.GetY() + 0.5);
    return emp::to_string("(", x, ",", y, "):", (size_t) radius);
  }
};

void PrintOverlap(const TestBody & body1, const TestBody & body2) {
  std::cout << "Overlap " << body1.GetID() << " and " << body2.GetID() << ": "
            << body1.AsString() << " and " << body2.AsString()
            << "  dist=" << body1.GetCenter().Distance(body2.GetCenter())
            << std::endl;
}

int main()
{
  emp::Random random;
  emp::Surface<TestBody> surface({1000, 1000});
  emp::vector<TestBody> bodies(20);

  size_t id = 0;  
  for (auto & body : bodies) {
    body.Set({random.GetDouble(1000.0), random.GetDouble(1000.0)}, random.GetDouble(10.0, 100.0));
    body.SetID(id++);
    surface.AddBody(&body);
  }

  std::cout << "---- surface results ----" << std::endl;
  surface.FindOverlaps(PrintOverlap);

  std::cout << "\n---- brute force ----" << std::endl;
  for (size_t id1 = 1; id1 < bodies.size(); id1++) {
    auto & body1 = bodies[id1];
    for (size_t id2 = 0; id2 < id1; id2++) {
      auto & body2 = bodies[id2];
      double radius_sum = body1.GetRadius() + body2.GetRadius();
      if (body1.GetCenter().Distance(body2.GetCenter()) < radius_sum) {
        PrintOverlap(body1, body2);
      }
    }
  }

  std::cout << std::endl;
}
