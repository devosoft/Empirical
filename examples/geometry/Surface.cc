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

class TestBody {
private:
  emp::Point center;
  double radius;
public:
  TestBody(emp::Point _center={0,0}, double _radius=1.0) : center(_center), radius(_radius) { ; }
  ~TestBody() { ; }

  emp::Point GetCenter() const { return center; }
  double GetRadius() const { return radius; }

  void SetCenter(emp::Point _c) { center = _c; }
  void SetRadius(double _r) { radius = _r; }
  void Set(emp::Point _c, double _r) { center = _c; radius = _r; }
};

int main()
{
  emp::Random random;
  emp::Surface<TestBody> surface({1000, 1000});
  emp::vector<TestBody> bodies(10);
  
  for (auto & body : bodies) {
    body.Set({random.GetDouble(1000.0), random.GetDouble(1000.0)}, random.GetDouble(10.0, 100.0));
    surface.AddBody(&body);
  }
}
